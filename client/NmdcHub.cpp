/* 
 * Copyright (C) 2001-2003 Jacek Sieka, j_s@telia.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "stdinc.h"
#include "DCPlusPlus.h"

#include "NmdcHub.h"

#include "ResourceManager.h"
#include "ClientManager.h"
#include "SearchManager.h"
#include "ShareManager.h"
#include "CryptoManager.h"
#include "ConnectionManager.h"

#include "Socket.h"
#include "UserCommand.h"
#include "StringTokenizer.h"


NmdcHub::NmdcHub(const string& aHubURL) : Client(aHubURL, '|'), supportFlags(0),  
	adapter(this), state(STATE_CONNECT),
	lastActivity(GET_TICK()), 
	reconnect(true), lastUpdate(0)
{
	TimerManager::getInstance()->addListener(this);

};

NmdcHub::~NmdcHub() throw() {
	TimerManager::getInstance()->removeListener(this);
	Speaker<NmdcHubListener>::removeListeners();

	Lock l(cs);
	clearUsers();
};

void NmdcHub::connect() {
	setRegistered(false);
	reconnect = true;
	supportFlags = 0;
	lastMyInfo.clear();

	if(socket->isConnected()) {
		disconnect();
	}

	state = STATE_LOCK;

	if(getPort() == 0) {
		setPort(411);
	}
	socket->connect(getAddress(), getPort());
}

void NmdcHub::refreshUserList(bool unknownOnly /* = false */) {
	Lock l(cs);
	if(unknownOnly) {
		for(User::NickIter i = users.begin(); i != users.end(); ++i) {
			if(i->second->getConnection().empty()) {
				getInfo(i->second);
			}
		}
	} else {
		clearUsers();
		getNickList();
	}
}

void NmdcHub::clearUsers() {
	for(User::NickIter i = users.begin(); i != users.end(); ++i) {
		ClientManager::getInstance()->putUserOffline(i->second);		
	}
	users.clear();
}

void NmdcHub::onLine(const string& aLine) throw() {
	lastActivity = GET_TICK();

	if(aLine.length() == 0)
		return;
	
	if(aLine[0] != '$') {
		// Check if we're being banned...
		if(state != STATE_CONNECTED) {
			if(Util::findSubString(aLine, "banned") != string::npos) {
				reconnect = false;
			}
		}
		Speaker<NmdcHubListener>::fire(NmdcHubListener::Message(), this, Util::validateMessage(aLine, true));
		return;
	}

	string cmd;
	string param;
	string::size_type x;
	
	if( (x = aLine.find(' ')) == string::npos) {
		cmd = aLine;
	} else {
		cmd = aLine.substr(0, x);
		param = aLine.substr(x+1);
	}

	if(cmd == "$Search") {
		if(state != STATE_CONNECTED) {
			return;
		}
		string::size_type i = 0;
		string::size_type j = param.find(' ', i);
		if(j == string::npos || i == j)
			return;
		string seeker = param.substr(i, j-i);

		// Filter own searches
		if(SETTING(CONNECTION_TYPE) == SettingsManager::CONNECTION_ACTIVE) {
			if(seeker == (getLocalIp() + ":" + Util::toString(SETTING(IN_PORT)))) {
				return;
			}
		} else {
			// Hub:seeker
			if(Util::stricmp(seeker.c_str() + 4, getNick().c_str()) == 0) {
				return;
			}
		}

		i = j + 1;
		
		{
			Lock l(cs);
			u_int32_t tick = GET_TICK();

			seekers.push_back(make_pair(seeker, tick));

			// First, check if it's a flooder
			FloodIter fi;
			for(fi = flooders.begin(); fi != flooders.end(); ++fi) {
				if(fi->first == seeker) {
					return;
				}
			}

			int count = 0;
			for(fi = seekers.begin(); fi != seekers.end(); ++fi) {
				if(fi->first == seeker)
					count++;

				if(count > 7) {
					if(seeker.find("Hub:") != string::npos)
						Speaker<NmdcHubListener>::fire(NmdcHubListener::SearchFlood(), this, seeker.substr(4));
					else
						Speaker<NmdcHubListener>::fire(NmdcHubListener::SearchFlood(), this, seeker + STRING(NICK_UNKNOWN));

					flooders.push_back(make_pair(seeker, tick));
					return;
				}
			}
		}

		int a;
		if(param[i] == 'F') {
			a = SearchManager::SIZE_DONTCARE;
		} else if(param[i+2] == 'F') {
			a = SearchManager::SIZE_ATLEAST;
		} else {
			a = SearchManager::SIZE_ATMOST;
		}
		i += 4;
		j = param.find('?', i);
		if(j == string::npos || i == j)
			return;
		string size = param.substr(i, j-i);
		i = j + 1;
		j = param.find('?', i);
		if(j == string::npos || i == j)
			return;
		int type = Util::toInt(param.substr(i, j-i)) - 1;
		i = j + 1;
		param = param.substr(i);

		if(param.size() > 0) {
			Speaker<NmdcHubListener>::fire(NmdcHubListener::Search(), this, seeker, a, Util::toInt64(size), type, param);
			
			if(seeker.compare(0, 4, "Hub:") == 0) {
				User::Ptr u;
				{
					Lock l(cs);
					User::NickIter ni = users.find(seeker.substr(4));
					if(ni != users.end() && !ni->second->isSet(User::PASSIVE)) {
						u = ni->second;
						u->setFlag(User::PASSIVE);
					}
				}

				if(u) {
					updated(u);
				}
			}
		}
	} else if(cmd == "$MyINFO") {
		string::size_type i, j;
		i = 5;
		string nick;
		j = param.find(' ', i);
		if( (j == string::npos) || (j == i) )
			return;
		nick = param.substr(i, j-i);
		i = j + 1;
		User::Ptr u;
		dcassert(nick.size() > 0);

		{
			Lock l(cs);
			User::NickIter ni = users.find(nick);
			if(ni == users.end()) {
				u = users[nick] = ClientManager::getInstance()->getUser(nick, this);
			} else {
				u  = ni->second;
			}
		}
		j = param.find('$', i);
		if(j == string::npos)
			return;
		string tmpDesc = Util::validateMessage(param.substr(i, j-i), true);
		// Look for a tag...
		if(tmpDesc.size() > 0 && tmpDesc[tmpDesc.size()-1] == '>') {
			x = tmpDesc.rfind('<');
			if(x != string::npos) {
				// Hm, we have something...
				u->setTag(tmpDesc.substr(x));
				tmpDesc.erase(x);
			} else {
				u->setTag(Util::emptyString);
			}
		} else {
			u->setTag(Util::emptyString);
		}
		u->setDescription(tmpDesc);
		i = j + 3;
		j = param.find('$', i);
		if(j == string::npos)
			return;
		u->setConnection(param.substr(i, j-i-1));
		i = j + 1;
		j = param.find('$', i);
		if(j == string::npos)
			return;
		u->setEmail(Util::validateMessage(param.substr(i, j-i), true));
		i = j + 1;
		j = param.find('$', i);
		if(j == string::npos)
			return;
		u->setBytesShared(param.substr(i, j-i));

		Speaker<NmdcHubListener>::fire(NmdcHubListener::MyInfo(), this, u);
	} else if(cmd == "$Quit") {
		if(!param.empty()) {
			User::Ptr u;
			{
				Lock l(cs);
				User::NickIter i = users.find(param);
				if(i == users.end()) {
					dcdebug("C::onLine Quitting user %s not found\n", param.c_str());
					return;
				}
				
				u = i->second;
				users.erase(i);
			}
			
			Speaker<NmdcHubListener>::fire(NmdcHubListener::Quit(), this, u);
			ClientManager::getInstance()->putUserOffline(u, true);
		}
	} else if(cmd == "$ConnectToMe") {
		if(state != STATE_CONNECTED) {
			return;
		}
		string::size_type i = param.find(' ');
		string::size_type j;
		if( (i == string::npos) || ((i + 1) >= param.size()) ) {
			return;
		}
		i++;
		j = param.find(':', i);
		if(j == string::npos) {
			return;
		}
		string server = param.substr(i, j-i);
		if(j+1 >= param.size()) {
			return;
		}
		string port = param.substr(j+1);
		ConnectionManager::getInstance()->connect(server, (short)Util::toInt(port), getNick()); 
		Speaker<NmdcHubListener>::fire(NmdcHubListener::ConnectToMe(), this, server, (short)Util::toInt(port));

	} else if(cmd == "$RevConnectToMe") {
		if(state != STATE_CONNECTED) {
			return;
		}
		User::Ptr u;
		bool up = false;
		{
			Lock l(cs);
			string::size_type j = param.find(' ');
			if(j == string::npos) {
				return;
			}

			User::NickIter i = users.find(param.substr(0, j));
			if(i == users.end()) {
				return;
			}

			u = i->second;
			if(!u->isSet(User::PASSIVE)) {
				u->setFlag(User::PASSIVE);
				up = true;
			}
		}

		if(u) {
			if(SETTING(CONNECTION_TYPE) == SettingsManager::CONNECTION_ACTIVE) {
				connectToMe(u);
				Speaker<NmdcHubListener>::fire(NmdcHubListener::RevConnectToMe(), this, u);
			} else {
				// Notify the user that we're passive too...
				if(up)
					revConnectToMe(u);
			}

			if(up)
				updated(u);
		}
	} else if(cmd == "$SR") {
		SearchManager::getInstance()->onSearchResult(aLine);
	} else if(cmd == "$HubName") {
		name = param;
		Speaker<NmdcHubListener>::fire(NmdcHubListener::HubName(), this);
	} else if(cmd == "$Supports") {
		StringTokenizer st(param, ' ');
		StringList& sl = st.getTokens();
		for(StringIter i = sl.begin(); i != sl.end(); ++i) {
			if(*i == "UserCommand") {
				supportFlags |= SUPPORTS_USERCOMMAND;
			} else if(*i == "NoGetINFO") {
				supportFlags |= SUPPORTS_NOGETINFO;
			} else if(*i == "UserIP2") {
				supportFlags |= SUPPORTS_USERIP2;
			}
		}
		Speaker<NmdcHubListener>::fire(NmdcHubListener::Supports(), this, sl);
	} else if(cmd == "$UserCommand") {
		string::size_type i = string::npos;
		string::size_type j = param.find(' ');
		if(j == string::npos)
			return;

		int type = Util::toInt(param.substr(0, j));
		i = j+1;
 		if(type == UserCommand::TYPE_SEPARATOR || type == UserCommand::TYPE_CLEAR) {
			int ctx = Util::toInt(param.substr(i));
			Speaker<NmdcHubListener>::fire(NmdcHubListener::UserCommand(), this, type, ctx, Util::emptyString, Util::emptyString);
		} else if(type == UserCommand::TYPE_RAW || type == UserCommand::TYPE_RAW_ONCE) {
			j = param.find(' ', i);
			if(j == string::npos)
				return;
			int ctx = Util::toInt(param.substr(i));
			i = j+1;
			j = param.find('$');
			if(j == string::npos)
				return;
			string name = param.substr(i, j-i);
			i = j+1;
			string command = param.substr(i, param.length() - i);
			Speaker<NmdcHubListener>::fire(NmdcHubListener::UserCommand(), this, type, ctx, Util::validateMessage(name, true, false), Util::validateMessage(command, true, false));
		}
	} else if(cmd == "$Lock") {
		if(state != STATE_LOCK) {
			return;
		}
		state = STATE_HELLO;

		if(!param.empty()) {
			string::size_type j = param.find(" Pk=");
			string lock, pk;
			if( j != string::npos ) {
				lock = param.substr(0, j);
				pk = param.substr(j + 4);
			} else {
				// Workaround for faulty linux hubs...
				j = param.find(" ");
				if(j != string::npos)
					lock = param.substr(0, j);
				else
					lock = param;
			}

			if(CryptoManager::getInstance()->isExtended(lock)) {
				StringList feat;
				feat.push_back("UserCommand");
				feat.push_back("NoGetINFO");
				feat.push_back("NoHello");
				feat.push_back("UserIP2");
				feat.push_back("TTHSearch");

				if(BOOLSETTING(COMPRESS_TRANSFERS))
					feat.push_back("GetZBlock");
				supports(feat);
			}

			key(CryptoManager::getInstance()->makeKey(lock));
			validateNick(getNick());

			Speaker<NmdcHubListener>::fire(NmdcHubListener::CLock(), this, lock, pk);	
		}
	} else if(cmd == "$Hello") {
		if(!param.empty()) {
			User::Ptr u = ClientManager::getInstance()->getUser(param, this);
			{
				Lock l(cs);
				users[param] = u;
			}

			if(getNick() == param) {
				setMe(u);

				u->setFlag(User::DCPLUSPLUS);
				if(SETTING(CONNECTION_TYPE) != SettingsManager::CONNECTION_ACTIVE)
					u->setFlag(User::PASSIVE);
				else
					u->unsetFlag(User::PASSIVE);
			}

			if(state == STATE_HELLO) {
				state = STATE_CONNECTED;
				updateCounts(false);

				version();
				getNickList();
				myInfo();
			}

			Speaker<NmdcHubListener>::fire(NmdcHubListener::Hello(), this, u);
		}
	} else if(cmd == "$ForceMove") {
		disconnect();
		Speaker<NmdcHubListener>::fire(NmdcHubListener::Redirect(), this, param);
	} else if(cmd == "$HubIsFull") {
		Speaker<NmdcHubListener>::fire(NmdcHubListener::HubFull(), this);
	} else if(cmd == "$ValidateDenide") {		// Mind the spelling...
		disconnect();
		Speaker<NmdcHubListener>::fire(NmdcHubListener::ValidateDenied(), this);
	} else if(cmd == "$UserIP") {
		if(!param.empty()) {
			User::List v;
			StringTokenizer t(param, "$$");
			StringList& l = t.getTokens();
			for(StringIter it = l.begin(); it != l.end(); ++it) {
				string::size_type j = 0;
				if((j = it->find(' ')) == string::npos)
					continue;
				if((j+1) == it->length())
					continue;
				v.push_back(ClientManager::getInstance()->getUser(it->substr(0, j), this));
				v.back()->setIp(it->substr(j+1));
			}

			Speaker<NmdcHubListener>::fire(NmdcHubListener::UserIp(), this, v);
		}
	} else if(cmd == "$NickList") {
		if(!param.empty()) {
			User::List v;
			StringTokenizer t(param, "$$");
			StringList& sl = t.getTokens();

			for(StringIter it = sl.begin(); it != sl.end(); ++it) {
				v.push_back(ClientManager::getInstance()->getUser(*it, this));
			}

			{
				Lock l(cs);
				for(User::Iter it2 = v.begin(); it2 != v.end(); ++it2) {
					users[(*it2)->getNick()] = *it2;
				}
			}
			
			if(!(getSupportFlags() & SUPPORTS_NOGETINFO)) {
				string tmp;
				// Let's assume 10 characters per nick...
				tmp.reserve(v.size() * (11 + 10 + getNick().length())); 
				for(User::List::const_iterator i = v.begin(); i != v.end(); ++i) {
					tmp += "$GetINFO ";
					tmp += (*i)->getNick();
					tmp += ' ';
					tmp += getNick(); 
					tmp += '|';
				}
				if(!tmp.empty()) {
					send(tmp);
				}
			} 

			Speaker<NmdcHubListener>::fire(NmdcHubListener::NickList(), this, v);
		}
	} else if(cmd == "$OpList") {
		if(!param.empty()) {
			User::List v;
			StringTokenizer t(param, "$$");
			StringList& sl = t.getTokens();
			for(StringIter it = sl.begin(); it != sl.end(); ++it) {
				v.push_back(ClientManager::getInstance()->getUser(*it, this));
				v.back()->setFlag(User::OP);
			}

			{
				Lock l(cs);
				for(User::Iter it2 = v.begin(); it2 != v.end(); ++it2) {
					users[(*it2)->getNick()] = *it2;
				}
			}
			Speaker<NmdcHubListener>::fire(NmdcHubListener::OpList(), this, v);
			updateCounts(false);
			// Special...to avoid op's complaining that their count is not correctly
			// updated when they log in (they'll be counted as registered first...)
			myInfo();
		}
	} else if(cmd == "$To:") {
		string::size_type i = param.find("From:");
		if(i != string::npos) {
			i+=6;
			string::size_type j = param.find("$");
			if(j != string::npos) {
				string from = param.substr(i, j - 1 - i);
				if(from.size() > 0 && param.size() > (j + 1)) {
					Speaker<NmdcHubListener>::fire(NmdcHubListener::PrivateMessage(), this, ClientManager::getInstance()->getUser(from, this, false), Util::validateMessage(param.substr(j + 1), true));
				}
			}
		}
	} else if(cmd == "$GetPass") {
		setRegistered(true);
		Speaker<NmdcHubListener>::fire(NmdcHubListener::GetPassword(), this);
	} else if(cmd == "$BadPass") {
		Speaker<NmdcHubListener>::fire(NmdcHubListener::BadPassword(), this);
	} else if(cmd == "$LogedIn") {
		Speaker<NmdcHubListener>::fire(NmdcHubListener::LoggedIn(), this);
	} else {
		dcassert(cmd[0] == '$');
		dcdebug("NmdcHub::onLine Unknown command %s\n", aLine.c_str());
	} 
}

void NmdcHub::myInfo() {
	checkstate();
	
	dcdebug("MyInfo %s...\n", getNick().c_str());
	lastCounts = counts;
	
	string tmp1 = ";**\x1fU9";
	string tmp2 = "+L9";
	string tmp3 = "+G9";
	string tmp4 = "+R9";
	string tmp5 = "+N9";
	string::size_type i;
	
	for(i = 0; i < 6; i++) {
		tmp1[i]++;
	}
	for(i = 0; i < 3; i++) {
		tmp2[i]++; tmp3[i]++; tmp4[i]++; tmp5[i]++;
	}
	char modeChar = '?';
	if(SETTING(CONNECTION_TYPE) == SettingsManager::CONNECTION_ACTIVE)
		modeChar = 'A';
	else if(SETTING(CONNECTION_TYPE) == SettingsManager::CONNECTION_PASSIVE)
		modeChar = 'P';
	else if(SETTING(CONNECTION_TYPE) == SettingsManager::CONNECTION_SOCKS5)
		modeChar = '5';
	
	string uMin = (SETTING(MIN_UPLOAD_SPEED) == 0) ? Util::emptyString : tmp5 + Util::toString(SETTING(MIN_UPLOAD_SPEED));
	string minf = 
		"$MyINFO $ALL " + Util::validateNick(getNick()) + " " + Util::validateMessage(getDescription(), false) + 
		tmp1 + VERSIONSTRING + tmp2 + modeChar + tmp3 + getCounts() + tmp4 + Util::toString(SETTING(SLOTS)) + uMin + 
		">$ $" + SETTING(CONNECTION) + "\x01$" + Util::validateMessage(SETTING(EMAIL), false) + '$' + 
		ShareManager::getInstance()->getShareSizeString() + "$|";
	if(minf != lastMyInfo) {
		send(minf);
		lastMyInfo = minf;
	}
}

void NmdcHub::disconnect() throw() {	
	state = STATE_CONNECT;
	socket->disconnect();
	{ 
		Lock l(cs);
		clearUsers();
	}
}

void NmdcHub::search(int aSizeType, int64_t aSize, int aFileType, const string& aString){
	checkstate(); 
	char* buf;
	char c1 = (aSizeType == SearchManager::SIZE_DONTCARE) ? 'F' : 'T';
	char c2 = (aSizeType == SearchManager::SIZE_ATLEAST) ? 'F' : 'T';
	string tmp = aString;
	string::size_type i;
	while((i = tmp.find(' ')) != string::npos) {
		tmp[i] = '$';
	}
	int chars = 0;
	if(SETTING(CONNECTION_TYPE) == SettingsManager::CONNECTION_ACTIVE) {
		string x = getLocalIp();
		buf = new char[x.length() + aString.length() + 64];
		chars = sprintf(buf, "$Search %s:%d %c?%c?%s?%d?%s|", x.c_str(), SETTING(IN_PORT), c1, c2, Util::toString(aSize).c_str(), aFileType+1, tmp.c_str());
	} else {
		buf = new char[getNick().length() + aString.length() + 64];
		chars = sprintf(buf, "$Search Hub:%s %c?%c?%s?%d?%s|", getNick().c_str(), c1, c2, Util::toString(aSize).c_str(), aFileType+1, tmp.c_str());
	}
	send(buf, chars);
	delete[] buf;
}

void NmdcHub::kick(const User::Ptr& aUser, const string& aMsg) {
	checkstate(); 
	dcdebug("NmdcHub::kick\n");
	static const char str[] = 
		"$To: %s From: %s $<%s> You are being kicked because: %s|<%s> %s is kicking %s because: %s|";
	string msg2 = Util::validateMessage(aMsg, false);
	
	char* tmp = new char[sizeof(str) + 2*aUser->getNick().length() + 2*msg2.length() + 4*getNick().length()];
	const char* u = aUser->getNick().c_str();
	const char* n = getNick().c_str();
	const char* m = msg2.c_str();
	sprintf(tmp, str, u, n, n, m, n, n, u, m);
	send(tmp);
	delete[] tmp;
	
	// Short, short break to allow the message to reach the NmdcHub...
	Thread::sleep(200);
	send("$Kick " + aUser->getNick() + "|");
}

void NmdcHub::kick(const User* aUser, const string& aMsg) {
	checkstate(); 
	dcdebug("NmdcHub::kick\n");
	
	static const char str[] = 
		"$To: %s From: %s $<%s> You are being kicked because: %s|<%s> %s is kicking %s because: %s|";
	string msg2 = Util::validateMessage(aMsg, false);
	
	char* tmp = new char[sizeof(str) + 2*aUser->getNick().length() + 2*msg2.length() + 4*getNick().length()];
	const char* u = aUser->getNick().c_str();
	const char* n = getNick().c_str();
	const char* m = msg2.c_str();
	sprintf(tmp, str, u, n, n, m, n, n, u, m);
	send(tmp);
	delete[] tmp;
	
	// Short, short break to allow the message to reach the NmdcHub...
	Thread::sleep(100);
	send("$Kick " + aUser->getNick() + "|");
}

// TimerManagerListener
void NmdcHub::on(TimerManagerListener::Second, u_int32_t aTick) throw() {
	if(socket && (lastActivity + (120+Util::rand(0, 60)) * 1000) < aTick) {
		// Nothing's happened for ~120 seconds, check if we're connected, if not, try to connect...
		lastActivity = aTick;
		// Try to send something for the fun of it...
		if(isConnected()) {
			dcdebug("Testing writing...\n");
			socket->write("|", 1);
		} else {
			// Try to reconnect...
			if(reconnect && !getAddress().empty())
				connect();
		}
	}
	{
		Lock l(cs);
		
		while(!seekers.empty() && seekers.front().second + (5 * 1000) < aTick) {
			seekers.pop_front();
		}
		
		while(!flooders.empty() && flooders.front().second + (120 * 1000) < aTick) {
			flooders.pop_front();
		}
	}
}

// BufferedSocketListener
void NmdcHub::on(BufferedSocketListener::Failed, const string& aLine) throw() {
	{
		Lock l(cs);
		clearUsers();
	}
	if(state == STATE_CONNECTED)
		state = STATE_CONNECT;
	Speaker<NmdcHubListener>::fire(NmdcHubListener::Failed(), this, aLine); 
}

/**
 * @file
 * $Id: NmdcHub.cpp,v 1.8 2004/07/16 09:53:46 arnetheduck Exp $
 */


/* 
 * Copyright (C) 2001-2005 Jacek Sieka, arnetheduck on gmail point com
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
}

NmdcHub::~NmdcHub() throw() {
	TimerManager::getInstance()->removeListener(this);
	Speaker<NmdcHubListener>::removeListeners();

	clearUsers();
}

void NmdcHub::connect() {
	setRegistered(false);
	setReconnDelay(120 + Util::rand(0, 60));
	reconnect = true;
	supportFlags = 0;
	lastMyInfoA.clear();
 	lastMyInfoB.clear();
	lastUpdate = 0;

	if(socket->isConnected()) {
		disconnect();
	}

	reloadSettings();

	state = STATE_LOCK;

	if(getPort() == 0) {
		setPort(411);
	}
	socket->connect(getAddress(), getPort());
}

void NmdcHub::connect(const OnlineUser& aUser) {
	checkstate(); 
	dcdebug("NmdcHub::connectToMe %s\n", aUser.getIdentity().getNick().c_str());
	if(SETTING(CONNECTION_TYPE) == SettingsManager::CONNECTION_ACTIVE) {
		send("$ConnectToMe " + toNmdc(aUser.getIdentity().getNick()) + " " + getLocalIp() + ":" + Util::toString(SETTING(IN_PORT)) + "|");
	} else {
		send("$RevConnectToMe " + toNmdc(getNick()) + " " + toNmdc(aUser.getIdentity().getNick())  + "|");
	}
}

int64_t NmdcHub::getAvailable() const {
	Lock l(cs);
	int64_t x = 0;
	for(NickMap::const_iterator i = users.begin(); i != users.end(); ++i) {
		x+=i->second->getIdentity().getBytesShared();
	}
	return x;
}

void NmdcHub::refreshUserList(bool unknownOnly /* = false */) {
	if(unknownOnly) {
		Lock l(cs);
		for(NickIter i = users.begin(); i != users.end(); ++i) {
			if(!i->second->getIdentity().isSet(Identity::GOT_INF)) {
				getInfo(*i->second);
			}
		}
	} else {
		clearUsers();
		getNickList();
	}
}

OnlineUser& NmdcHub::getUser(const string& aNick) {
	OnlineUser* u = NULL;
	{
		Lock l(cs);

		NickIter i = users.find(aNick);
		if(i != users.end())
			return *i->second;

		User::Ptr p = ClientManager::getInstance()->getUser(aNick, getHubURL());

		u = users.insert(make_pair(aNick, new OnlineUser(p, *this))).first->second;
		u->getIdentity().setNick(aNick);
	}

	ClientManager::getInstance()->putOnline(*u);
	return *u;
}

OnlineUser* NmdcHub::findUser(const string& aNick) {
	Lock l(cs);
	NickIter i = users.find(aNick);
	return i == users.end() ? NULL : i->second;
}

void NmdcHub::putUser(const string& aNick) {
	Lock l(cs);
	NickIter i = users.find(aNick);
	if(i == users.end())
		return;
	delete i->second;
	users.erase(i);
}

void NmdcHub::clearUsers() {
	Lock l(cs);
	for(NickIter i = users.begin(); i != users.end(); ++i) {
		ClientManager::getInstance()->putOffline(*i->second);
		delete i->second;
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
		Speaker<NmdcHubListener>::fire(NmdcHubListener::Message(), this, Util::validateMessage(fromNmdc(aLine), true));
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
		
		string seeker = fromNmdc(param.substr(i, j-i));

		// Filter own searches
		if(SETTING(CONNECTION_TYPE) == SettingsManager::CONNECTION_ACTIVE) {
			if(seeker == (getLocalIp() + ":" + Util::toString(SETTING(UDP_PORT)))) {
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
					if(seeker.compare(0, 4, "Hub:") == 0)
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
			if(seeker.compare(0, 4, "Hub:") == 0) {
				OnlineUser* u = findUser(seeker.substr(4));

				if(u == NULL) {
					return;
				}

				if(!u->getUser()->isSet(User::PASSIVE)) {
					u->getUser()->setFlag(User::PASSIVE);
					updated(*u);
				}
			}

			Speaker<NmdcHubListener>::fire(NmdcHubListener::Search(), this, seeker, a, Util::toInt64(size), type, fromNmdc(param));
		}
	} else if(cmd == "$MyINFO") {
		string::size_type i, j;
		i = 5;
		j = param.find(' ', i);
		if( (j == string::npos) || (j == i) )
			return;
		string nick = fromNmdc(param.substr(i, j-i));
		
		if(nick.empty())
			return;

		i = j + 1;
		
		OnlineUser& u = getUser(nick);

		j = param.find('$', i);
		if(j == string::npos)
			return;

		string tmpDesc = Util::validateMessage(fromNmdc(param.substr(i, j-i)), true);
		// Look for a tag...
		if(tmpDesc.size() > 0 && tmpDesc[tmpDesc.size()-1] == '>') {
			x = tmpDesc.rfind('<');
			if(x != string::npos) {
				// Hm, we have something...disassemble it...
				/// @todo u->setTag(tmpDesc.substr(x));
				tmpDesc.erase(x);
			} else {
				/// @todo u->setTag(Util::emptyString);
			}
		} else {
			/// @todo u->setTag(Util::emptyString);
		}
		u.getIdentity().setDescription(tmpDesc);

		i = j + 3;
		j = param.find('$', i);
		if(j == string::npos)
			return;

		/// @todo u->setConnection(fromNmdc(param.substr(i, j-i-1)));
		i = j + 1;
		j = param.find('$', i);

		if(j == string::npos)
			return;

		u.getIdentity().setEmail(Util::validateMessage(fromNmdc(param.substr(i, j-i)), true));

		i = j + 1;
		j = param.find('$', i);
		if(j == string::npos)
			return;
		u.getIdentity().setBytesShared(param.substr(i, j-i));

		Speaker<NmdcHubListener>::fire(NmdcHubListener::MyInfo(), this, u);
	} else if(cmd == "$Quit") {
		if(!param.empty()) {
			string nick = fromNmdc(param);
			OnlineUser* u = findUser(nick);
			if(u == NULL)
				return;

			ClientManager::getInstance()->putOffline(*u);
			Speaker<NmdcHubListener>::fire(NmdcHubListener::Quit(), this, *u);

			/** @todo fix removing user */
			{
				Lock l(cs);
				NickIter i = users.find(nick);
				dcassert(i != users.end());
				
				users.erase(i);
			}
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
		string server = fromNmdc(param.substr(i, j-i));
		if(j+1 >= param.size()) {
			return;
		}
		string port = param.substr(j+1);
		ConnectionManager::getInstance()->nmdcConnect(server, (short)Util::toInt(port), getNick()); 
		Speaker<NmdcHubListener>::fire(NmdcHubListener::ConnectToMe(), this, server, (short)Util::toInt(port));
	} else if(cmd == "$RevConnectToMe") {
		if(state != STATE_CONNECTED) {
			return;
		}

		string::size_type j = param.find(' ');
		if(j == string::npos) {
			return;
		}

		OnlineUser* u = findUser(fromNmdc(param.substr(0, j)));
		if(u == NULL)
			return;

		if(SETTING(CONNECTION_TYPE) == SettingsManager::CONNECTION_ACTIVE) {
			connectToMe(*u);
			Speaker<NmdcHubListener>::fire(NmdcHubListener::RevConnectToMe(), this, *u);
		} else {
			if(!u->getUser()->isSet(User::PASSIVE)) {
				u->getUser()->setFlag(User::PASSIVE);
				// Notify the user that we're passive too...
				revConnectToMe(*u);
				updated(*u);

				return;
			}
		}
	} else if(cmd == "$SR") {
		SearchManager::getInstance()->onSearchResult(aLine);
	} else if(cmd == "$HubName") {
		name = fromNmdc(param);
		Speaker<NmdcHubListener>::fire(NmdcHubListener::HubName(), this);
	} else if(cmd == "$Supports") {
		StringTokenizer<string> st(param, ' ');
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
		string::size_type i = 0;
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
			string name = fromNmdc(param.substr(i, j-i));
			i = j+1;
			string command = fromNmdc(param.substr(i, param.length() - i));
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
			string nick = fromNmdc(param);
			OnlineUser& u = getUser(nick);

			if(getNick() == nick) {
				u.getUser()->setFlag(User::DCPLUSPLUS);
				if(SETTING(CONNECTION_TYPE) != SettingsManager::CONNECTION_ACTIVE)
					u.getUser()->setFlag(User::PASSIVE);
				else
					u.getUser()->unsetFlag(User::PASSIVE);
			}

			if(state == STATE_HELLO) {
				state = STATE_CONNECTED;
				updateCounts(false);

				version();
				getNickList();
				myInfo(true);
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
			OnlineUser::List v;
			StringTokenizer<string> t(fromNmdc(param), "$$");
			StringList& l = t.getTokens();
			for(StringIter it = l.begin(); it != l.end(); ++it) {
				string::size_type j = 0;
				if((j = it->find(' ')) == string::npos)
					continue;
				if((j+1) == it->length())
					continue;

				OnlineUser* u = findUser(it->substr(0, j));
				
				if(u == NULL)
					continue;

				u->getIdentity().setIp(it->substr(j+1));
				v.push_back(u);
			}

			Speaker<NmdcHubListener>::fire(NmdcHubListener::UserIp(), this, v);
		}
	} else if(cmd == "$NickList") {
		if(!param.empty()) {
			OnlineUser::List v;
			StringTokenizer<string> t(fromNmdc(param), "$$");
			StringList& sl = t.getTokens();

			for(StringIter it = sl.begin(); it != sl.end(); ++it) {
				if(it->empty())
					continue;
				
				v.push_back(&getUser(*it));
			}

			if(!(getSupportFlags() & SUPPORTS_NOGETINFO)) {
				string tmp;
				// Let's assume 10 characters per nick...
				tmp.reserve(v.size() * (11 + 10 + getNick().length())); 
				string n = ' ' +  toNmdc(getNick()) + '|';
				for(OnlineUser::List::const_iterator i = v.begin(); i != v.end(); ++i) {
					tmp += "$GetINFO ";
					tmp += toNmdc((*i)->getIdentity().getNick());
					tmp += n;
				}
				if(!tmp.empty()) {
					send(tmp);
				}
			} 

			Speaker<NmdcHubListener>::fire(NmdcHubListener::NickList(), this, v);
		}
	} else if(cmd == "$OpList") {
		if(!param.empty()) {
			OnlineUser::List v;
			StringTokenizer<string> t(fromNmdc(param), "$$");
			StringList& sl = t.getTokens();
			for(StringIter it = sl.begin(); it != sl.end(); ++it) {
				if(it->empty())
					continue;
				v.push_back(&getUser(*it));
				v.back()->getUser()->setFlag(User::OP);
			}

			Speaker<NmdcHubListener>::fire(NmdcHubListener::OpList(), this, v);
			updateCounts(false);

			// Special...to avoid op's complaining that their count is not correctly
			// updated when they log in (they'll be counted as registered first...)
			myInfo(false);
		}
	} else if(cmd == "$To:") {
		string::size_type i = param.find("From:");
		if(i != string::npos) {
			i+=6;
			string::size_type j = param.find("$");
			if(j != string::npos) {
				string from = fromNmdc(param.substr(i, j - 1 - i));
				if(from.size() > 0 && param.size() > (j + 1)) {
					/// @todo Speaker<NmdcHubListener>::fire(NmdcHubListener::PrivateMessage(), this, ClientManager::getInstance()->getUser(from, this, false), Util::validateMessage(fromNmdc(param.substr(j + 1)), true));
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

string NmdcHub::checkNick(const string& aNick) {
	string tmp = aNick;
	string::size_type i = 0;
	while( (i = tmp.find_first_of("|$ ", i)) != string::npos) {
		tmp[i++]='_';
	}
	return tmp;
}

string NmdcHub::getHubURL() {
	return getAddressPort();
}

void NmdcHub::myInfo(bool alwaysSend) {
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
	string myInfoA = 
		"$MyINFO $ALL " + toNmdc(checkNick(getNick())) + " " + toNmdc(Util::validateMessage(getDescription(), false)) + 
		tmp1 + VERSIONSTRING + tmp2 + modeChar + tmp3 + getCounts() + tmp4 + Util::toString(SETTING(SLOTS)) + uMin + 
		">$ $" + SETTING(CONNECTION) + "\x01$" + toNmdc(Util::validateMessage(SETTING(EMAIL), false)) + '$'; 
	string myInfoB = ShareManager::getInstance()->getShareSizeString() + "$|";
 	
 	if(lastMyInfoA != myInfoA || alwaysSend || (lastMyInfoB != myInfoB && lastUpdate + 15*60*1000 < GET_TICK()) ){
 		send(myInfoA + myInfoB);
 		lastMyInfoA = myInfoA;
 		lastMyInfoB = myInfoB;
 		lastUpdate = GET_TICK();
	}
}

void NmdcHub::disconnect() throw() {	
	state = STATE_CONNECT;
	Client::disconnect();
	clearUsers();
}

void NmdcHub::search(int aSizeType, int64_t aSize, int aFileType, const string& aString, const string&) {
	checkstate(); 
	AutoArray<char> buf((char*)NULL);
	char c1 = (aSizeType == SearchManager::SIZE_DONTCARE) ? 'F' : 'T';
	char c2 = (aSizeType == SearchManager::SIZE_ATLEAST) ? 'F' : 'T';
	string tmp = Util::validateMessage(toNmdc((aFileType == SearchManager::TYPE_TTH) ? "TTH:" + aString : aString), false);
	string::size_type i;
	while((i = tmp.find(' ')) != string::npos) {
		tmp[i] = '$';
	}
	int chars = 0;
	if(SETTING(CONNECTION_TYPE) == SettingsManager::CONNECTION_ACTIVE) {
		string x = getLocalIp();
		buf = new char[x.length() + aString.length() + 64];
		chars = sprintf(buf, "$Search %s:%d %c?%c?%s?%d?%s|", x.c_str(), SETTING(UDP_PORT), c1, c2, Util::toString(aSize).c_str(), aFileType+1, tmp.c_str());
	} else {
		buf = new char[getNick().length() + aString.length() + 64];
		chars = sprintf(buf, "$Search Hub:%s %c?%c?%s?%d?%s|", toNmdc(getNick()).c_str(), c1, c2, Util::toString(aSize).c_str(), aFileType+1, tmp.c_str());
	}
	send(buf, chars);
}

// TimerManagerListener
void NmdcHub::on(TimerManagerListener::Second, u_int32_t aTick) throw() {
	if(socket && (lastActivity + getReconnDelay() * 1000) < aTick) {
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
	clearUsers();

	if(state == STATE_CONNECTED)
		state = STATE_CONNECT;

	Speaker<NmdcHubListener>::fire(NmdcHubListener::Failed(), this, aLine); 
}

/**
 * @file
 * $Id: NmdcHub.cpp,v 1.34 2005/04/12 23:24:11 arnetheduck Exp $
 */


/*
 * Copyright (C) 2001-2006 Jacek Sieka, arnetheduck on gmail point com
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

NmdcHub::NmdcHub(const string& aHubURL) : Client(aHubURL, '|', false), supportFlags(0), state(STATE_CONNECT),
	reconnect(true), lastUpdate(0)
{
	TimerManager::getInstance()->addListener(this);
}

NmdcHub::~NmdcHub() throw() {
	TimerManager::getInstance()->removeListener(this);
	clearUsers();
}

void NmdcHub::connect() {
	reconnect = true;
	supportFlags = 0;
	lastMyInfoA.clear();
 	lastMyInfoB.clear();
	lastUpdate = 0;

	state = STATE_LOCK;

	Client::connect();
}

#define checkstate() if(state != STATE_CONNECTED) return

void NmdcHub::connect(const OnlineUser& aUser) {
	checkstate(); 
	dcdebug("NmdcHub::connect %s\n", aUser.getIdentity().getNick().c_str());
	if(ClientManager::getInstance()->isActive()) {
		connectToMe(aUser);
	} else {
		revConnectToMe(aUser);
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

OnlineUser& NmdcHub::getUser(const string& aNick) {
	OnlineUser* u = NULL;
	{
		Lock l(cs);

		NickIter i = users.find(aNick);
		if(i != users.end())
			return *i->second;
	}

	User::Ptr p;
	if(aNick == getMyNick()) {
		p = ClientManager::getInstance()->getMe();
		getMyIdentity().setUser(p);
		getMyIdentity().setHubUrl(getHubUrl());
	} else {
		p = ClientManager::getInstance()->getUser(aNick, getHubUrl());
		// In NMDC, everyone's a bot until they show a good myinfo
		p->setFlag(User::BOT);
	}

	{
		Lock l(cs);
		u = users.insert(make_pair(aNick, new OnlineUser(p, *this, 0))).first->second;
		u->getIdentity().setNick(aNick);
	}

	ClientManager::getInstance()->putOnline(*u);
	return *u;
}

void NmdcHub::supports(const StringList& feat) { 
	string x;
	for(StringList::const_iterator i = feat.begin(); i != feat.end(); ++i) {
		x+= *i + ' ';
	}
	send("$Supports " + x + '|');
}

OnlineUser* NmdcHub::findUser(const string& aNick) {
	Lock l(cs);
	NickIter i = users.find(aNick);
	return i == users.end() ? NULL : i->second;
}

void NmdcHub::putUser(const string& aNick) {
	OnlineUser* u = NULL;
	{
		Lock l(cs);
		NickIter i = users.find(aNick);
		if(i == users.end())
			return;
		u = i->second;
		users.erase(i);
	}
	ClientManager::getInstance()->putOffline(*u);
	delete u;
}

void NmdcHub::clearUsers() {
	NickMap u2;

	{
		Lock l(cs);
		u2 = users;
		users.clear();
	}

	for(NickIter i = u2.begin(); i != u2.end(); ++i) {
		ClientManager::getInstance()->putOffline(*i->second);
		delete i->second;
	}
}

void NmdcHub::updateFromTag(Identity& id, const string& tag) {
	StringTokenizer<string> tok(tag, ',');
	for(StringIter i = tok.getTokens().begin(); i != tok.getTokens().end(); ++i) {
		if(i->length() < 2)
			continue;

		if(i->compare(0, 2, "H:") == 0) {
			StringTokenizer<string> t(i->substr(2), '/');
			if(t.getTokens().size() != 3)
				continue;
			id.set("HN", t.getTokens()[0]);
			id.set("HR", t.getTokens()[1]);
			id.set("HO", t.getTokens()[2]);
		} else if(i->compare(0, 2, "S:") == 0) {
			id.set("SL", i->substr(2));
		} else if(i->find("V:") != string::npos) {
			string::size_type j = i->find("V:");
			i->erase(i->begin() + j, i->begin() + j + 2);
			id.set("VE", *i);
		} else if(i->compare(0, 2, "M:") == 0) {
			if(i->size() == 3) {
				if((*i)[2] == 'A')
					id.getUser()->unsetFlag(User::PASSIVE);
				else
					id.getUser()->setFlag(User::PASSIVE);
			}
		}
	}
	/// @todo Think about this
	id.set("TA", '<' + tag + '>');
}

void NmdcHub::onLine(const string& aLine) throw() {
	updateActivity();

	if(aLine.length() == 0)
		return;
	
	if(aLine[0] != '$') {
		// Check if we're being banned...
		if(state != STATE_CONNECTED) {
			if(Util::findSubString(aLine, "banned") != string::npos) {
				reconnect = false;
			}
		}
		string line = fromNmdc(aLine);
		// @todo Decrypt who the message is from...
		fire(ClientListener::StatusMessage(), this, Util::validateMessage(fromNmdc(aLine), true));
		return;
	}

	string cmd;
	string param;
	string::size_type x;
	
	if( (x = aLine.find(' ')) == string::npos) {
		cmd = aLine;
	} else {
		cmd = aLine.substr(0, x);
		param = fromNmdc(aLine.substr(x+1));
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
		if(ClientManager::getInstance()->isActive()) {
			if(seeker == (ClientManager::getInstance()->getCachedIp() + ":" + Util::toString(SearchManager::getInstance()->getPort()))) {
				return;
			}
		} else {
			// Hub:seeker
			if(Util::stricmp(seeker.c_str() + 4, getMyNick().c_str()) == 0) {
				return;
			}
		}

		i = j + 1;
		
		{
			Lock l(cs);
			u_int32_t tick = GET_TICK();

			seekers.push_back(make_pair(seeker, tick));

			// First, check if it's a flooder
			for(FloodIter fi = flooders.begin(); fi != flooders.end(); ++fi) {
				if(fi->first == seeker) {
					return;
				}
			}

			int count = 0;
			for(FloodIter fi = seekers.begin(); fi != seekers.end(); ++fi) {
				if(fi->first == seeker)
					count++;

				if(count > 7) {
					if(seeker.compare(0, 4, "Hub:") == 0)
						fire(ClientListener::SearchFlood(), this, seeker.substr(4));
					else
						fire(ClientListener::SearchFlood(), this, seeker + STRING(NICK_UNKNOWN));

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

			fire(ClientListener::NmdcSearch(), this, seeker, a, Util::toInt64(size), type, param);
		}
	} else if(cmd == "$MyINFO") {
		string::size_type i, j;
		i = 5;
		j = param.find(' ', i);
		if( (j == string::npos) || (j == i) )
			return;
		string nick = param.substr(i, j-i);
		
		if(nick.empty())
			return;

		i = j + 1;
		
		OnlineUser& u = getUser(nick);

		u.getUser()->unsetFlag(User::BOT);

		j = param.find('$', i);
		if(j == string::npos)
			return;

		string tmpDesc = Util::validateMessage(param.substr(i, j-i), true);
		// Look for a tag...
		if(tmpDesc.size() > 0 && tmpDesc[tmpDesc.size()-1] == '>') {
			x = tmpDesc.rfind('<');
			if(x != string::npos) {
				// Hm, we have something...disassemble it...
				updateFromTag(u.getIdentity(), tmpDesc.substr(x + 1, tmpDesc.length() - x - 2));
				tmpDesc.erase(x);
			}
		}
		u.getIdentity().setDescription(tmpDesc);

		i = j + 3;
		j = param.find('$', i);
		if(j == string::npos)
			return;

		u.getIdentity().setConnection(param.substr(i, j-i-1));
		i = j + 1;
		j = param.find('$', i);

		if(j == string::npos)
			return;

		u.getIdentity().setEmail(Util::validateMessage(param.substr(i, j-i), true));

		i = j + 1;
		j = param.find('$', i);
		if(j == string::npos)
			return;
		u.getIdentity().setBytesShared(param.substr(i, j-i));

		if(u.getUser() == getMyIdentity().getUser())
			setMyIdentity(u.getIdentity());
		
		fire(ClientListener::UserUpdated(), this, u);
	} else if(cmd == "$Quit") {
		if(!param.empty()) {
			string nick = param;
			OnlineUser* u = findUser(nick);
			if(u == NULL)
				return;

			fire(ClientListener::UserRemoved(), this, *u);

			putUser(nick);
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
		ConnectionManager::getInstance()->nmdcConnect(server, (short)Util::toInt(port), getMyNick(), getHubUrl()); 
	} else if(cmd == "$RevConnectToMe") {
		if(state != STATE_CONNECTED) {
			return;
		}

		string::size_type j = param.find(' ');
		if(j == string::npos) {
			return;
		}

		OnlineUser* u = findUser(param.substr(0, j));
		if(u == NULL)
			return;

		if(ClientManager::getInstance()->isActive()) {
			connectToMe(*u);
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
		// Hack - first word goes to hub name, rest to description
		string::size_type i = param.find(' ');
		if(i == string::npos) {
			getHubIdentity().setNick(param);
			getHubIdentity().setDescription(Util::emptyString);			
		} else {
			getHubIdentity().setNick(param.substr(0, i));
			getHubIdentity().setDescription(param.substr(i+1));
		}
		fire(ClientListener::HubUpdated(), this);
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
	} else if(cmd == "$UserCommand") {
		string::size_type i = 0;
		string::size_type j = param.find(' ');
		if(j == string::npos)
			return;

		int type = Util::toInt(param.substr(0, j));
		i = j+1;
 		if(type == UserCommand::TYPE_SEPARATOR || type == UserCommand::TYPE_CLEAR) {
			int ctx = Util::toInt(param.substr(i));
			fire(ClientListener::UserCommand(), this, type, ctx, Util::emptyString, Util::emptyString);
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
			fire(ClientListener::UserCommand(), this, type, ctx, Util::validateMessage(name, true, false), Util::validateMessage(command, true, false));
		}
	} else if(cmd == "$Lock") {
		if(state != STATE_LOCK) {
			return;
		}
		state = STATE_HELLO;

		// Param must not be fromNmdc'd...
		param = aLine.substr(6);

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
				feat.push_back("ZPipe0");

				if(BOOLSETTING(COMPRESS_TRANSFERS))
					feat.push_back("GetZBlock");
				supports(feat);
			}

			key(CryptoManager::getInstance()->makeKey(lock));
			validateNick(getMyNick());
		}
	} else if(cmd == "$Hello") {
		if(!param.empty()) {
			OnlineUser& u = getUser(param);

			if(getMyNick() == param) {
				u.getUser()->setFlag(User::DCPLUSPLUS);
				if(ClientManager::getInstance()->isActive())
					u.getUser()->unsetFlag(User::PASSIVE);
				else
					u.getUser()->setFlag(User::PASSIVE);
			}

			if(state == STATE_HELLO) {
				state = STATE_CONNECTED;
				updateCounts(false);

				version();
				getNickList();
				myInfo(true);
			}

			fire(ClientListener::UserUpdated(), this, u);
		}
	} else if(cmd == "$ForceMove") {
		socket->disconnect(false);
		fire(ClientListener::Redirect(), this, param);
	} else if(cmd == "$HubIsFull") {
		fire(ClientListener::HubFull(), this);
	} else if(cmd == "$ValidateDenide") {		// Mind the spelling...
		socket->disconnect(false);
		fire(ClientListener::NickTaken(), this);
	} else if(cmd == "$UserIP") {
		if(!param.empty()) {
			OnlineUser::List v;
			StringTokenizer<string> t(param, "$$");
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

			fire(ClientListener::UsersUpdated(), this, v);
		}
	} else if(cmd == "$NickList") {
		if(!param.empty()) {
			OnlineUser::List v;
			StringTokenizer<string> t(param, "$$");
			StringList& sl = t.getTokens();

			for(StringIter it = sl.begin(); it != sl.end(); ++it) {
				if(it->empty())
					continue;
				
				v.push_back(&getUser(*it));
			}

			if(!(getSupportFlags() & SUPPORTS_NOGETINFO)) {
				string tmp;
				// Let's assume 10 characters per nick...
				tmp.reserve(v.size() * (11 + 10 + getMyNick().length())); 
				string n = ' ' +  toNmdc(getMyNick()) + '|';
				for(OnlineUser::List::const_iterator i = v.begin(); i != v.end(); ++i) {
					tmp += "$GetINFO ";
					tmp += toNmdc((*i)->getIdentity().getNick());
					tmp += n;
				}
				if(!tmp.empty()) {
					send(tmp);
				}
			} 

			fire(ClientListener::UsersUpdated(), this, v);
		}
	} else if(cmd == "$OpList") {
		if(!param.empty()) {
			OnlineUser::List v;
			StringTokenizer<string> t(param, "$$");
			StringList& sl = t.getTokens();
			for(StringIter it = sl.begin(); it != sl.end(); ++it) {
				if(it->empty())
					continue;
				OnlineUser& ou = getUser(*it);
				ou.getIdentity().setOp(true);
				if(*it == getMyNick())
					getMyIdentity().setOp(true);
				v.push_back(&ou);
			}

			fire(ClientListener::UsersUpdated(), this, v);
			updateCounts(false);

			// Special...to avoid op's complaining that their count is not correctly
			// updated when they log in (they'll be counted as registered first...)
			myInfo(false);
		}
	} else if(cmd == "$To:") {
		string::size_type i = param.find("From:");
		if(i == string::npos)
			return;

		i+=6;
		string::size_type j = param.find('$', i);
		if(j == string::npos)
			return;

		string rtNick = param.substr(i, j - 1 - i);
		if(rtNick.empty())
			return;
		i = j + 1;

		if(param.size() < i + 3 || param[i] != '<')
			return;

		j = param.find('>', i);
		if(j == string::npos)
			return;

		string fromNick = param.substr(i+1, j-i-1);
		if(fromNick.empty())
			return;

        OnlineUser* replyTo = findUser(rtNick);
		OnlineUser* from = findUser(fromNick);
		OnlineUser* to = findUser(getMyNick());

		if(replyTo == NULL || from == NULL || to == NULL) {
			fire(ClientListener::StatusMessage(), this, Util::validateMessage(param.substr(i), true));
		} else {
			string msg = param.substr(j + 2);
			fire(ClientListener::PrivateMessage(), this, *from, *to, *replyTo, Util::validateMessage(param.substr(j + 2), true));
		}
	} else if(cmd == "$GetPass") {
		setRegistered(true);
		fire(ClientListener::GetPassword(), this);
	} else if(cmd == "$BadPass") {
		fire(ClientListener::BadPassword(), this);
	} else if(cmd == "$ZOn") {
		socket->setMode (BufferedSocket::MODE_ZPIPE);
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

void NmdcHub::connectToMe(const OnlineUser& aUser) {
	checkstate(); 
	dcdebug("NmdcHub::connectToMe %s\n", aUser.getIdentity().getNick().c_str());
	ConnectionManager::getInstance()->nmdcExpect(aUser.getIdentity().getNick(), getMyNick(), getHubUrl());
	send("$ConnectToMe " + toNmdc(aUser.getIdentity().getNick()) + " " + getLocalIp() + ":" + Util::toString(ConnectionManager::getInstance()->getPort()) + "|");
}

void NmdcHub::revConnectToMe(const OnlineUser& aUser) {
	checkstate(); 
	dcdebug("NmdcHub::revConnectToMe %s\n", aUser.getIdentity().getNick().c_str());
	send("$RevConnectToMe " + toNmdc(getMyNick()) + " " + toNmdc(aUser.getIdentity().getNick()) + "|");
}

void NmdcHub::hubMessage(const string& aMessage) { 
	checkstate(); 
	send(toNmdc( "<" + getMyNick() + "> " + Util::validateMessage(aMessage, false) + "|" ) ); 
}

void NmdcHub::myInfo(bool alwaysSend) {
	checkstate();
	
	reloadSettings(false);

	dcdebug("MyInfo %s...\n", getMyNick().c_str());
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
	if(SETTING(OUTGOING_CONNECTIONS) == SettingsManager::OUTGOING_SOCKS5)
		modeChar = '5';
	else if(ClientManager::getInstance()->isActive())
		modeChar = 'A';
	else 
		modeChar = 'P';
	
	string uMin = (SETTING(MIN_UPLOAD_SPEED) == 0) ? Util::emptyString : tmp5 + Util::toString(SETTING(MIN_UPLOAD_SPEED));
	string myInfoA = 
		"$MyINFO $ALL " + toNmdc(checkNick(getMyNick())) + " " + toNmdc(Util::validateMessage(getMyIdentity().getDescription(), false)) + 
		tmp1 + VERSIONSTRING + tmp2 + modeChar + tmp3 + getCounts() + tmp4 + Util::toString(SETTING(SLOTS)) + uMin + 
		">$ $" + SETTING(UPLOAD_SPEED) + "\x01$" + toNmdc(Util::validateMessage(SETTING(EMAIL), false)) + '$'; 
	string myInfoB = ShareManager::getInstance()->getShareSizeString() + "$|";
 	
 	if(lastMyInfoA != myInfoA || alwaysSend || (lastMyInfoB != myInfoB && lastUpdate + 15*60*1000 < GET_TICK()) ){
 		send(myInfoA + myInfoB);
 		lastMyInfoA = myInfoA;
 		lastMyInfoB = myInfoB;
 		lastUpdate = GET_TICK();
	}
}

void NmdcHub::disconnect(bool graceless) throw() {	
	Client::disconnect(graceless);
	state = STATE_CONNECT;
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
	if(ClientManager::getInstance()->isActive()) {
		string x = ClientManager::getInstance()->getCachedIp();
		buf = new char[x.length() + aString.length() + 64];
		chars = sprintf(buf, "$Search %s:%d %c?%c?%s?%d?%s|", x.c_str(), (int)SearchManager::getInstance()->getPort(), c1, c2, Util::toString(aSize).c_str(), aFileType+1, tmp.c_str());
	} else {
		buf = new char[getMyNick().length() + aString.length() + 64];
		chars = sprintf(buf, "$Search Hub:%s %c?%c?%s?%d?%s|", toNmdc(getMyNick()).c_str(), c1, c2, Util::toString(aSize).c_str(), aFileType+1, tmp.c_str());
	}
	send(buf, chars);
}

void NmdcHub::privateMessage(const OnlineUser& aUser, const string& aMessage) { 
	checkstate();

	send("$To: " + toNmdc(aUser.getIdentity().getNick()) + " From: " + toNmdc(getMyNick()) + " $" + toNmdc(Util::validateMessage("<" + getMyNick() + "> " + aMessage, false)) + "|");
	// Emulate a returning message...
	Lock l(cs);
	NickIter i = users.find(getMyNick());
	if(i != users.end())
		fire(ClientListener::PrivateMessage(), this, *i->second, aUser, *i->second, aMessage);
}

// TimerManagerListener
void NmdcHub::on(TimerManagerListener::Second, u_int32_t aTick) throw() {
	if(socket && (getLastActivity() + getReconnDelay() * 1000) < aTick) {
		// Nothing's happened for ~120 seconds, check if we're connected, if not, try to connect...
		if(isConnected()) {
			// Try to send something for the fun of it...
			dcdebug("Testing writing...\n");
			send("|", 1);
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

	fire(ClientListener::Failed(), this, aLine); 
}

/* 
 * Copyright (C) 2001 Jacek Sieka, j_s@telia.com
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

#include "Socket.h"
#include "Client.h"
#include "ClientManager.h"
#include "SearchManager.h"

long Client::hubs = 0;

Client::~Client() throw() {
	TimerManager::getInstance()->removeListener(this);
	removeListeners();

	if(socket) {
		socket->removeListener(this);
		BufferedSocket::putSocket(socket);
		socket = NULL;
	}

	for(User::NickIter i = users.begin(); i != users.end(); ++i) {
		ClientManager::getInstance()->putUserOffline(i->second);
	}

	if(counted)
		Thread::safeDec(&hubs);
};

void Client::connect(const string& aServer) {
	string tmp;
	port = 411;
	Util::decodeUrl(aServer, server, port, tmp);

	connect();
}

void Client::connect() {
	registered = false;

	if(!socket) {
		socket = BufferedSocket::getSocket('|');
	}
	
	if(socket->isConnected()) {
		disconnect();
	}

	state = STATE_LOCK;

	socket->addListener(this);
	fire(ClientListener::CONNECTING, this);
	socket->connect(server, port);
}

void Client::onLine(const string& aLine) throw() {
	lastActivity = GET_TICK();

	if(aLine.length() == 0)
		return;
	
	if(aLine[0] != '$') {
		fire(ClientListener::MESSAGE, this, aLine);
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
		i = j + 1;
		
		{
			Lock l(cs);
			u_int32_t tick = GET_TICK();

			seekers.push_back(make_pair(seeker, tick));

			// First, check if it's a flooder
			FloodIter i;
			for(i = flooders.begin(); i != flooders.end(); ++i) {
				if(i->first == seeker) {
					return;
				}
			}

			int count = 0;
			for(i = seekers.begin(); i != seekers.end(); ++i) {
				if(i->first == seeker)
					count++;

				if(count > 7) {
					if(seeker.find("Hub:") != string::npos)
						fire(ClientListener::SEARCH_FLOOD, this, seeker.substr(4));
					else
						fire(ClientListener::SEARCH_FLOOD, this, seeker + STRING(NICK_UNKNOWN));

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
			fire(ClientListener::SEARCH, this, seeker, a, size, type, param);
			
			if(seeker.find("Hub:") != string::npos) {
				User::Ptr u;
				{
					Lock l(cs);
					User::NickIter i = users.find(seeker.substr(4));
					if(i != users.end() && !i->second->isSet(User::PASSIVE)) {
						u = i->second;
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
				u = ClientManager::getInstance()->getUser(nick, this);
				users[nick] = u;
			} else {
				u  = ni->second;
			}
		}
		j = param.find('$', i);
		if(j == string::npos)
			return;
		u->setDescription(param.substr(i, j-i));
		i = j + 3;
		j = param.find('$', i);
		if(j == string::npos)
			return;
		u->setConnection(param.substr(i, j-i-1));
		i = j + 1;
		j = param.find('$', i);
		if(j == string::npos)
			return;
		u->setEmail(param.substr(i, j-i));
		i = j + 1;
		j = param.find('$', i);
		if(j == string::npos)
			return;
		u->setBytesShared(param.substr(i, j-i));
		
		fire(ClientListener::MY_INFO, this, u);
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
				users.erase(param);
			}
			
			fire(ClientListener::QUIT, this, u);
			ClientManager::getInstance()->putUserOffline(u);
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
		fire(ClientListener::CONNECT_TO_ME, this, server, param.substr(j+1));

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
				fire(ClientListener::REV_CONNECT_TO_ME, this, u);
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
		fire(ClientListener::HUB_NAME, this);
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
			fire(ClientListener::C_LOCK, this, lock, pk);	
		}
	} else if(cmd == "$Hello") {
		if(!param.empty()) {
			User::Ptr& u = ClientManager::getInstance()->getUser(param, this);
			{
				Lock l(cs);
				users[param] = u;
			}
			
			if(u->getNick() == getNick()) {
				if(state != STATE_HELLO) {
					return;
				}
				state = STATE_CONNECTED;
				if(registered) {
					if(counted) {
						Thread::safeDec(&hubs);
						counted = false;
					}
				} else {
					if(!counted) {
						Thread::safeInc(&hubs);
						counted = true;
					}
				}

				u->setFlag(User::DCPLUSPLUS);
				if(SETTING(CONNECTION_TYPE) == SettingsManager::CONNECTION_PASSIVE)
					u->setFlag(User::PASSIVE);
			}
			
			fire(ClientListener::HELLO, this, u);
		}
	} else if(cmd == "$ForceMove") {
		disconnect();
		fire(ClientListener::FORCE_MOVE, this, param);
	} else if(cmd == "$HubIsFull") {
		fire(ClientListener::HUB_FULL, this);
	} else if(cmd == "$ValidateDenide") {		// Mind the spelling...
		fire(ClientListener::VALIDATE_DENIED, this);
	} else if(cmd == "$NickList") {
		if(!param.empty()) {
			User::List v;

			string::size_type j, k = 0;
			{
				Lock l(cs);
				while( (j=param.find("$$", k)) != string::npos) {
					if(j == k)
						continue;
					string nick = param.substr(k, j-k);
					User::Ptr& u = ClientManager::getInstance()->getUser(nick, this);

					users[nick] = u;

					v.push_back(u);
					k = j + 2;
				}
			}
			
			fire(ClientListener::NICK_LIST, this, v);
		}
	} else if(cmd == "$OpList") {
		if(!param.empty()) {
			User::List v;
			string::size_type j, k;
			k = 0;
			{
				Lock l(cs);
				while( (j=param.find("$$", k)) != string::npos) {
					if(j == k)
						continue;
					string nick = param.substr(k, j-k);
					User::Ptr& u = ClientManager::getInstance()->getUser(nick, this);
					users[nick] = u;
					u->setFlag(User::OP);

					v.push_back(u);
					k = j + 2;
				}
			}
			fire(ClientListener::OP_LIST, this, v);
		}
	} else if(cmd == "$To:") {
		string::size_type i = param.find("From:");
		if(i != string::npos) {
			i+=6;
			string::size_type j = param.find("$");
			if(j != string::npos) {
				string from = param.substr(i, j - 1 - i);
				if(from.size() > 0 && param.size() > (j + 1)) {
					fire(ClientListener::PRIVATE_MESSAGE, this, ClientManager::getInstance()->getUser(from, this, false), param.substr(j + 1));
				}
			}
		}
	} else if(cmd == "$GetPass") {
		registered = true;
		fire(ClientListener::GET_PASSWORD, this);
	} else if(cmd == "$BadPass") {
		fire(ClientListener::BAD_PASSWORD, this);
	} else if(cmd == "$LogedIn") {
		fire(ClientListener::LOGGED_IN, this);
	} else {
		dcassert(cmd[0] == '$');
		dcdebug("Client::onLine Unknown command %s\n", aLine.c_str());
	} 
}

void Client::disconnect() throw() {	
	state = STATE_CONNECT;
	socket->disconnect();
	{ 
		Lock l(cs);
		
		for(User::NickIter i = users.begin(); i != users.end(); ++i) {
			ClientManager::getInstance()->putUserOffline(i->second);
		}
		users.clear();
	}
}

void Client::search(int aSizeType, int64_t aSize, int aFileType, const string& aString){
	checkstate(); 
	char* buf;
	char c1 = (aSizeType == SearchManager::SIZE_DONTCARE) ? 'F' : 'T';
	char c2 = (aSizeType == SearchManager::SIZE_ATLEAST) ? 'F' : 'T';
	string tmp = aString;
	string::size_type i;
	while((i = tmp.find(' ')) != string::npos) {
		tmp[i] = '$';
	}
	if(SETTING(CONNECTION_TYPE) == SettingsManager::CONNECTION_ACTIVE) {
		string x = Socket::resolve(SETTING(SERVER));
		buf = new char[x.length() + aString.length() + 64];
		sprintf(buf, "$Search %s:%d %c?%c?%s?%d?%s|", x.c_str(), SETTING(PORT), c1, c2, Util::toString(aSize).c_str(), aFileType+1, tmp.c_str());
	} else {
		buf = new char[getNick().length() + aString.length() + 64];
		sprintf(buf, "$Search Hub:%s %c?%c?%s?%d?%s|", getNick().c_str(), c1, c2, Util::toString(aSize).c_str(), aFileType+1, tmp.c_str());
	}
	send(buf);
	delete[] buf;
}

/**
 * @file Client.cpp
 * $Id: Client.cpp,v 1.44 2002/05/18 11:20:36 arnetheduck Exp $
 */


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

#include "stdafx.h"
#include "DCPlusPlus.h"

#include "Socket.h"
#include "Client.h"
#include "CriticalSection.h"

void Client::connect(const string& aServer, short aPort) {
	
	if(socket.isConnected()) {
		disconnect();
	}

	server = aServer;
	port = aPort;
	fire(ClientListener::CONNECTING, this);
	socket.addListener(this);
	socket.connect(aServer, aPort);
}

/**
 * Reader thread. Read data from socket and calls getLine whenever there's data to be read.
 * Finishes whenever p->stopEvent is signaled
 * @param p Pointer to the Clientent that started the thread
 */

void Client::onLine(const string& aLine) throw() {
	lastActivity = TimerManager::getTick();

	if(aLine.length() == 0)
		return;
	
	string cmd;
	string param;
	int x;
	
	if( (x = aLine.find(' ')) == string::npos) {
		cmd = aLine;
	} else {
		cmd = aLine.substr(0, x);
		param = aLine.substr(x+1);
	}
	
	if(cmd == "$Search") {
		int i = 0;
		int j = param.find(' ', i);
		string seeker = param.substr(i, j-i);
		i = j + 1;
		int a;
		if(param[i] == 'F') {
			a = SearchManager::SIZE_DONTCARE;
		} else if(param[2] == 'F') {
			a = SearchManager::SIZE_ATLEAST;
		} else {
			a = SearchManager::SIZE_ATMOST;
		}
		i += 4;
		j = param.find('?', i);
		string size = param.substr(i, j-i);
		i = j + 1;
		j = param.find('?', i);
		int type = Util::toInt(param.substr(i, j-i));
		i = j + 1;
		param = param.substr(i);
		
		if(param.size() > 0)
			fire(ClientListener::SEARCH, this, seeker, a, size, type, param);
	} else if(cmd == "$MyINFO") {
		int i, j;
		i = 5;
		string nick;
		j = param.find(' ', i);
		nick = param.substr(i, j-i);
		i = j + 1;
		User::Ptr u;

		{
			Lock l(cs);

			User::NickIter it = users.find(nick);
			if(it == users.end()) {
				u = new User(nick, User::ONLINE);
				u->setClient(this);
				users[nick] = u;
			} else {
				u = it->second;
			}
		}

		j = param.find('$', i);
		u->setDescription(param.substr(i, j-i));
		i = j + 3;
		j = param.find('$', i);
		u->setConnection(param.substr(i, j-i-1));
		i = j + 1;
		j = param.find('$', i);
		u->setEmail(param.substr(i, j-i));
		i = j + 1;
		j = param.find('$', i);
		u->setBytesShared(param.substr(i, j-i));
		
		fire(ClientListener::MY_INFO, this, u);
		
	} else if(cmd == "$Quit") {
		cs.enter();
		User::NickIter i = users.find(param);
		if(i != users.end()) {
			User::Ptr u = i->second;
			users.erase(i);
			cs.leave();
			u->unsetFlag(User::ONLINE);
			fire(ClientListener::QUIT, this, u);
		} else {
			cs.leave();
		}
		
	} else if(cmd == "$ConnectToMe") {
		param = param.substr(param.find(' ') + 1);
		string server = param.substr(0, param.find(':'));
		fire(ClientListener::CONNECT_TO_ME, this, server, param.substr(param.find(':')+1));
	} else if(cmd == "$RevConnectToMe") {
		if(SETTING(CONNECTION_TYPE) == SettingsManager::CONNECTION_ACTIVE) {
			cs.enter();
			User::NickIter i = users.find(param.substr(0, param.find(' ')));
			if(i != users.end()) {
				cs.leave();
				fire(ClientListener::REV_CONNECT_TO_ME, this, i->second);
			} else {
				cs.leave();
			}
		}
	} else if(cmd == "$SR") {
		SearchManager::getInstance()->onSearchResult(aLine);
	} else if(cmd == "$HubName") {
		name = param;
		fire(ClientListener::HUB_NAME, this);
	} else if(cmd == "$Lock") {
	
		string lock = param.substr(0, param.find(' '));
		string pk = param.substr(param.find(' ') + 4);
		fire(ClientListener::LOCK, this, lock, pk);	
	} else if(cmd == "$Hello") {
		User::Ptr u;

		{
			Lock l(cs);

			User::NickIter i = users.find(param);
			if(i == users.end()) {
				u = new User(param, User::ONLINE);
				u->setClient(this);
				users[param] = u;
			} else {
				u = i->second;
			}
			
			if(u->getNick() == getNick())
				u->setFlag(User::DCPLUSPLUS);
		}

		fire(ClientListener::HELLO, this, u);
	} else if(cmd == "$ForceMove") {
		fire(ClientListener::FORCE_MOVE, this, param);
	} else if(cmd == "$HubIsFull") {
		fire(ClientListener::HUB_FULL, this);
	} else if(cmd == "$ValidateDenide") {
		fire(ClientListener::VALIDATE_DENIED, this);
	} else if(cmd == "$NickList") {
		StringList v;
		int j, k = 0;
		while( (j=param.find("$$", k)) != string::npos) {
			string nick = param.substr(k, j-k);
			cs.enter();
			if(users.find(nick) == users.end()) {
				User::Ptr u = new User(nick, User::ONLINE);
				u->setClient(this);
				users[nick] = u;
				cs.leave();
				fire(ClientListener::MY_INFO, this, u);
			} else {
				cs.leave();
			}
			v.push_back(nick);
			k = j + 2;
		}
		
		fire(ClientListener::NICK_LIST, this, v);
		
	} else if(cmd == "$OpList") {
		StringList v;
		int j, k;
		k = 0;
		while( (j=param.find("$$", k)) != string::npos) {
			string nick = param.substr(k, j-k);
			{
				Lock l(cs);
				User::NickIter i = users.find(nick);
				if( i == users.end()) {
					User::Ptr u = new User(nick, User::OP | User::ONLINE);
					u->setClient(this);
					users[nick] = u;
				} else {
					i->second->setFlag(User::OP);
				}
			}

			fire(ClientListener::MY_INFO, this, users[nick]);
			v.push_back(nick);
			k = j + 2;
		}
		fire(ClientListener::OP_LIST, this, v);
	} else if(cmd == "$To:") {
		int i = param.find("From:");
		if(i != -1) {
			i+=6;
			int j = param.find("$");
			string from = param.substr(i, j - 1 - i);
			if(from.size() > 0 && param.size() > (j + 1)) {
				User::Ptr& user = getUser(from);
				if(user) {
					fire(ClientListener::PRIVATE_MESSAGE, this, user, param.substr(j + 1));
				} else {
					fire(ClientListener::PRIVATE_MESSAGE, this, from, param.substr(j + 1));
				}
			}
		}
	} else if(cmd == "$GetPass") {
		fire(ClientListener::GET_PASSWORD, this);
	} else if(cmd == "$BadPass") {
		fire(ClientListener::BAD_PASSWORD, this);
	} else if(cmd == "$LogedIn") {
		fire(ClientListener::LOGGED_IN, this);
	} else if(cmd[0] == '$') {
		fire(ClientListener::UNKNOWN, this, aLine);
	} else {
		fire(ClientListener::MESSAGE, this, aLine);
	}
}


/**
 * @file Client.cpp
 * $Id: Client.cpp,v 1.20 2002/01/20 22:54:46 arnetheduck Exp $
 * @if LOG
 * $Log: Client.cpp,v $
 * Revision 1.20  2002/01/20 22:54:46  arnetheduck
 * Bugfixes to 0.131 mainly...
 *
 * Revision 1.19  2002/01/17 23:35:59  arnetheduck
 * Reworked threading once more, now it actually seems stable. Also made
 * sure that noone tries to access client objects that have been deleted
 * as well as some other minor updates
 *
 * Revision 1.18  2002/01/13 22:50:47  arnetheduck
 * Time for 0.12, added favorites, a bunch of new icons and lot's of other stuff
 *
 * Revision 1.17  2002/01/11 14:52:56  arnetheduck
 * Huge changes in the listener code, replaced most of it with templates,
 * also moved the getinstance stuff for the managers to a template
 *
 * Revision 1.16  2002/01/10 12:33:14  arnetheduck
 * Various fixes
 *
 * Revision 1.15  2002/01/08 00:24:10  arnetheduck
 * Last bugs fixed before 0.11
 *
 * Revision 1.14  2002/01/06 11:13:07  arnetheduck
 * Last fixes before 0.10
 *
 * Revision 1.13  2001/12/30 15:03:44  arnetheduck
 * Added framework to handle incoming searches
 *
 * Revision 1.12  2001/12/21 20:21:17  arnetheduck
 * Private messaging added, and a lot of other updates as well...
 *
 * Revision 1.11  2001/12/19 23:07:59  arnetheduck
 * Added directory downloading from the directory tree (although it hasn't been
 * tested at all) and password support.
 *
 * Revision 1.10  2001/12/16 19:47:48  arnetheduck
 * Reworked downloading and user handling some, and changed some small UI things
 *
 * Revision 1.9  2001/12/15 17:01:06  arnetheduck
 * Passive mode searching as well as some searching code added
 *
 * Revision 1.8  2001/12/13 19:21:57  arnetheduck
 * A lot of work done almost everywhere, mainly towards a friendlier UI
 * and less bugs...time to release 0.06...
 *
 * Revision 1.7  2001/12/12 00:06:04  arnetheduck
 * Updated the public hub listings, fixed some minor transfer bugs, reworked the
 * sockets to use only one thread (instead of an extra thread for sending files),
 * and fixed a major bug in the client command decoding (still have to fix this
 * one for the userconnections...)
 *
 * Revision 1.6  2001/12/08 14:25:49  arnetheduck
 * More bugs removed...did my first search as well...
 *
 * Revision 1.5  2001/12/07 20:03:02  arnetheduck
 * More work done towards application stability
 *
 * Revision 1.4  2001/12/04 21:50:34  arnetheduck
 * Work done towards application stability...still a lot to do though...
 * a bit more and it's time for a new release.
 *
 * Revision 1.3  2001/12/01 17:15:03  arnetheduck
 * Added a crappy version of huffman encoding, and some other minor changes...
 *
 * Revision 1.2  2001/11/29 19:10:54  arnetheduck
 * Refactored down/uploading and some other things completely.
 * Also added download indicators and download resuming, along
 * with some other stuff.
 *
 * Revision 1.1  2001/11/27 22:10:08  arnetheduck
 * Renamed DCClient* to Client*
 *
 * Revision 1.5  2001/11/26 23:40:36  arnetheduck
 * Downloads!! Now downloads are possible, although the implementation is
 * likely to change in the future...more UI work (splitters...) and some bug
 * fixes. Only user file listings are downloadable, but at least it's something...
 *
 * Revision 1.4  2001/11/25 22:06:25  arnetheduck
 * Finally downloading is working! There are now a few quirks and bugs to be fixed
 * but what the heck....!
 *
 * Revision 1.3  2001/11/24 10:34:02  arnetheduck
 * Updated to use BufferedSocket instead of handling threads by itself.
 *
 * Revision 1.2  2001/11/22 19:47:42  arnetheduck
 * A simple XML parser. Doesn't have all the features, but works good enough for
 * the configuration file.
 *
 * Revision 1.1.1.1  2001/11/21 17:33:20  arnetheduck
 * Inital release
 *
 * @endif
 */


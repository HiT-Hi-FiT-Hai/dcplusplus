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
#include "ClientManager.h"
#include "SearchManager.h"

long Client::hubs = 0;

void Client::connect(const string& aServer) {
	
	string tmp;
	port = 411;
	Util::decodeUrl(aServer, server, port, tmp);

	if(socket.isConnected()) {
		disconnect(false);
	}

	fire(ClientListener::CONNECTING, this);
	socket.addListener(this);
	socket.connect(server, port);
}

void Client::connect(const string& aServer, short aPort) {
	
	if(socket.isConnected()) {
		disconnect(false);
	}

	server = aServer;
	port = aPort;
	
	fire(ClientListener::CONNECTING, this);
	socket.addListener(this);
	socket.connect(server, port);
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
	
	if(aLine[0] != '$') {
		fire(ClientListener::MESSAGE, this, aLine);
		return;
	}

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
		if(j == string::npos)
			return;
		string seeker = param.substr(i, j-i);
		i = j + 1;
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
		if(j == string::npos)
			return;
		string size = param.substr(i, j-i);
		i = j + 1;
		j = param.find('?', i);
		if(j == string::npos)
			return;
		int type = Util::toInt(param.substr(i, j-i)) - 1;
		i = j + 1;
		param = param.substr(i);
		bool spam = false;
		{
			Lock l(cs);
			FloodIter s = searchFlood.find(seeker);
			if(s != searchFlood.end()) {
				if(++s->second > 5) {
					// We have a search spammer!!!
					if(seeker.find("Hub:") != string::npos)
						fire(ClientListener::SEARCH_FLOOD, this, seeker.substr(4));
					else
						fire(ClientListener::SEARCH_FLOOD, this, seeker + " (Nick unknown)");
					spam = true;
				}
			} else {
				searchFlood[seeker] = 1;
			}
		}

		if(!spam && param.size() > 0) {
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
		int i, j;
		i = 5;
		string nick;
		j = param.find(' ', i);
		if(j == string::npos)
			return;
		nick = param.substr(i, j-i);
		i = j + 1;
		User::Ptr u = ClientManager::getInstance()->getUser(nick, this);

		{
			Lock l(cs);
			users[nick] = u;
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

	} else if(cmd == "$ConnectToMe") {
		param = param.substr(param.find(' ') + 1);
		string server = param.substr(0, param.find(':'));
		fire(ClientListener::CONNECT_TO_ME, this, server, param.substr(param.find(':')+1));
	} else if(cmd == "$RevConnectToMe") {
		User::Ptr u;
		bool up = false;
		{
			Lock l(cs);
			User::NickIter i = users.find(param.substr(0, param.find(' ')));
			if(i != users.end()) {
				u = i->second;
				if(!u->isSet(User::PASSIVE)) {
					u->setFlag(User::PASSIVE);
					up = true;
				}
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
		int j = param.find(" Pk=");
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
		fire(ClientListener::LOCK, this, lock, pk);	
	} else if(cmd == "$Hello") {
		
		User::Ptr u = ClientManager::getInstance()->getUser(param, this);
		{
			Lock l(cs);
			users[param] = u;
		}

		if(u->getNick() == getNick()) {
			u->setFlag(User::DCPLUSPLUS);
			if(SETTING(CONNECTION_TYPE) == SettingsManager::CONNECTION_PASSIVE)
				u->setFlag(User::PASSIVE);
		}
		
		fire(ClientListener::HELLO, this, u);
	} else if(cmd == "$ForceMove") {
		fire(ClientListener::FORCE_MOVE, this, param);
	} else if(cmd == "$HubIsFull") {
		fire(ClientListener::HUB_FULL, this);
	} else if(cmd == "$ValidateDenide") {
		fire(ClientListener::VALIDATE_DENIED, this);
	} else if(cmd == "$NickList") {
		User::List v;

		int j, k = 0;
		while( (j=param.find("$$", k)) != string::npos) {
			string nick = param.substr(k, j-k);
			User::Ptr u = ClientManager::getInstance()->getUser(nick, this);
			users[param] = u;
			fire(ClientListener::MY_INFO, this, u);

			v.push_back(u);
			k = j + 2;
		}
		
		fire(ClientListener::NICK_LIST, this, v);
		
	} else if(cmd == "$OpList") {
		User::List v;
		int j, k;
		k = 0;
		while( (j=param.find("$$", k)) != string::npos) {
			string nick = param.substr(k, j-k);
			User::Ptr u = ClientManager::getInstance()->getUser(nick, this);
			{
				Lock l(cs);
				users[nick] = u;
			}
			u->setFlag(User::OP);
			fire(ClientListener::MY_INFO, this, u);
			
			v.push_back(u);
			k = j + 2;
		}
		fire(ClientListener::OP_LIST, this, v);
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
		if(counted) {
			InterlockedDecrement(&hubs);
			counted = false;
		}
		fire(ClientListener::GET_PASSWORD, this);
	} else if(cmd == "$BadPass") {
		fire(ClientListener::BAD_PASSWORD, this);
	} else if(cmd == "$LogedIn") {
		fire(ClientListener::LOGGED_IN, this);
	} else {
		dcassert(cmd[0] == '$');
		fire(ClientListener::UNKNOWN, this, aLine);
	} 
}

void Client::disconnect(bool rl /* = true */) throw() {	
	if(rl)
		socket.removeListener(this);
	
	socket.disconnect();
	
	{ 
		Lock l(cs);
		
		for(User::NickIter i = users.begin(); i != users.end(); ++i) {
			ClientManager::getInstance()->putUserOffline(i->second);
		}
		users.clear();
	}
}

void Client::search(int aSizeType, LONGLONG aSize, int aFileType, const string& aString){
	char* buf;
	char c1 = (aSizeType == SearchManager::SIZE_DONTCARE) ? 'F' : 'T';
	char c2 = (aSizeType == SearchManager::SIZE_ATLEAST) ? 'F' : 'T';
	string tmp = aString;
	string::size_type i;
	while((i = tmp.find(' ')) != string::npos) {
		tmp.replace(i, 1, 1, '$');
	}
	if(SETTING(CONNECTION_TYPE) == SettingsManager::CONNECTION_ACTIVE) {
		buf = new char[SETTING(SERVER).length() + aString.length() + 64];
		sprintf(buf, "$Search %s:%d %c?%c?%I64d?%d?%s|", SETTING(SERVER).c_str(), SETTING(PORT), c1, c2, aSize, aFileType+1, tmp.c_str());
	} else {
		buf = new char[getNick().length() + aString.length() + 64];
		sprintf(buf, "$Search Hub:%s %c?%c?%I64d?%d?%s|", getNick().c_str(), c1, c2, aSize, aFileType+1, tmp.c_str());
	}
	send(buf);
	delete buf;
}

/**
 * @file Client.cpp
 * $Id: Client.cpp,v 1.36 2002/03/26 09:17:59 arnetheduck Exp $
 * @if LOG
 * $Log: Client.cpp,v $
 * Revision 1.36  2002/03/26 09:17:59  arnetheduck
 * New UsersFrame
 *
 * Revision 1.35  2002/03/19 00:41:37  arnetheduck
 * 0.162, hub counting and cpu bug
 *
 * Revision 1.34  2002/03/14 16:17:35  arnetheduck
 * Oops, file buffering bug
 *
 * Revision 1.33  2002/03/13 23:06:07  arnetheduck
 * New info sent in the description part of myinfo...
 *
 * Revision 1.32  2002/03/13 20:35:25  arnetheduck
 * Release canditate...internationalization done as far as 0.155 is concerned...
 * Also started using mirrors of the public hub lists
 *
 * Revision 1.31  2002/03/10 22:41:08  arnetheduck
 * Working on internationalization...
 *
 * Revision 1.30  2002/03/04 23:52:30  arnetheduck
 * Updates and bugfixes, new user handling almost finished...
 *
 * Revision 1.29  2002/02/28 00:10:47  arnetheduck
 * Some fixes to the new user model
 *
 * Revision 1.28  2002/02/27 12:02:09  arnetheduck
 * Completely new user handling, wonder how it turns out...
 *
 * Revision 1.27  2002/02/25 15:39:28  arnetheduck
 * Release 0.154, lot of things fixed...
 *
 * Revision 1.26  2002/02/18 23:48:32  arnetheduck
 * New prerelease, bugs fixed and features added...
 *
 * Revision 1.25  2002/02/09 18:13:51  arnetheduck
 * Fixed level 4 warnings and started using new stl
 *
 * Revision 1.24  2002/02/02 17:21:27  arnetheduck
 * Fixed search bugs and some other things...
 *
 * Revision 1.23  2002/01/26 16:34:00  arnetheduck
 * Colors dialog added, as well as some other options
 *
 * Revision 1.22  2002/01/26 12:06:39  arnetheduck
 * Sm�saker
 *
 * Revision 1.21  2002/01/22 00:10:37  arnetheduck
 * Version 0.132, removed extra slots feature for nm dc users...and some bug
 * fixes...
 *
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


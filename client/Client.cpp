/* 
 * Copyright (C) 2001 Jacek Sieka, jacek@creatio.se
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

Client::List Client::clientList;

ClientListener::List Client::staticListeners;
CriticalSection Client::staticListenersCS;

void Client::connect(const string& aServer, short aPort) {
	if(socket.isConnected()) {
		disconnect();
	}
	server = aServer;
	port = aPort;
	fireConnecting();
	socket.addListener(this);
	socket.connect(aServer, aPort);
}

/**
 * Reader thread. Read data from socket and calls getLine whenever there's data to be read.
 * Finishes whenever p->stopEvent is signaled
 * @param p Pointer to the Clientent that started the thread
 */

void Client::onLine(const string& aLine) {
	if(aLine.length() == 0) {
//		dcassert(0); // should never happen...but it does...
	} else if(aLine.find("$Search") != string::npos) {
		string tmp = aLine.substr(8);
		string seeker = tmp.substr(0, tmp.find(' '));
		tmp = tmp.substr(tmp.find(' ') + 1);
		int a;
		if(tmp[0] == 'F') {
			a = SEARCH_PLAIN;
		} else if(tmp[2] == 'F') {
			a = SEARCH_ATLEAST;
		} else {
			a = SEARCH_ATMOST;
		}
		tmp=tmp.substr(4);
		string size = tmp.substr(0, tmp.find('?'));
		tmp = tmp.substr(tmp.find('?')+1);
		int type = atoi(tmp.substr(0, tmp.find('?')).c_str());
		tmp = tmp.substr(tmp.find('?')+1);
		fireSearch(seeker, a, size, type, tmp);
	} else if(aLine.find("$ConnectToMe") != string::npos) {
		string tmp = aLine.substr(13);
		tmp = tmp.substr(tmp.find(' ') + 1);
		string server = tmp.substr(0, tmp.find(':'));
		fireConnectToMe(server, tmp.substr(tmp.find(':')+1));
	} else if(aLine.find("$RevConnectToMe") != string::npos) {
		if(Settings::getConnectionType() == Settings::CONNECTION_ACTIVE) {
			string tmp = aLine.substr(16);
			User::NickIter i = users.find(tmp.substr(0, tmp.find(' ')));
			if(i != users.end()) {
				fireRevConnectToMe(i->second);
			}
		}
	} else if(aLine.find("$HubName") != string::npos) {
		name = aLine.substr(9);
		fireHubName();
	} else if(aLine.find("$Lock")!=string::npos) {
	
		string lock = aLine.substr(6);
		lock = lock.substr(0, lock.find(' '));
		string pk = lock.substr(lock.find(' ') + 4);
		fireLock(lock, pk);	
	} else if(aLine.find("$Hello") != string::npos) {
		string nick = aLine.substr(7);
		User* u;
		User::NickIter i = users.find(nick);
		if(i == users.end()) {
			u = new User(nick);
			u->setClient(this);
			users[nick] = u;
		} else {
			u = i->second;
		}
		fireHello(u);
	} else if(aLine.find("$ForceMove") != string::npos) {
		fireForceMove(aLine.substr(11));
	} else if(aLine.find("$HubIsFull") != string::npos) {
		fireHubFull();
	} else if(aLine.find("$MyINFO $ALL") != string::npos) {
		string nick;

		string tmp = aLine.substr(13);
		nick = tmp.substr(0, tmp.find(' '));
		tmp = tmp.substr(tmp.find(' ')+1);
		User* u;
		if(users.find(nick) == users.end()) {
			u = new User(nick);
			u->setClient(this);
			users[nick] = u;
		} else {
			u = users[nick];
		}

		u->setDescription(tmp.substr(0, tmp.find('$')));
		tmp = tmp.substr(tmp.find('$')+3);
		u->setConnection(tmp.substr(0, tmp.find('$')-1));
		tmp = tmp.substr(tmp.find('$')+1);
		u->setEmail(tmp.substr(0, tmp.find('$')));
		tmp = tmp.substr(tmp.find('$')+1);
		u->setBytesShared(tmp.substr(0, tmp.find('$')));
		
		fireMyInfo(u);
		
	} else if(aLine.find("$Quit") != string::npos) {
		string nick = aLine.substr(6);
		if(users.find(nick) != users.end()) {
			User* u = users[nick];
			
			fireQuit(u);
			delete u;
			users.erase(nick);
		}
		
	} else if(aLine.find("$ValidateDenide") != string::npos) {
		fireValidateDenied();
	} else if(aLine.find("$NickList") != string::npos) {
		StringList v;
		int j;
		string tmp = aLine.substr(10);
		while( (j=tmp.find("$$")) != string::npos) {
			string nick = tmp.substr(0, j);
			
			if(users.find(nick) == users.end()) {
				User* u = new User(nick);
				u->setClient(this);
				users[nick] = u;
			}

			v.push_back(nick);
			tmp = tmp.substr(j+2);
		}
		
		fireNickList(v);
		
	} else if(aLine.find("$OpList") != string::npos) {
		StringList v;
		int j;
		string tmp = aLine.substr(8);
		while( (j=tmp.find("$$")) != string::npos) {
			string nick = tmp.substr(0, j);
			if(users.find(nick) == users.end()) {
				User* u = new User(nick, User::FLAG_OP);
				u->setClient(this);
				users[nick] = u;
			}
			users[nick]->setFlag(User::FLAG_OP);
			v.push_back(nick);
			tmp = tmp.substr(j+2);
		}
		fireOpList(v);
	} else if(aLine.find("$To") != string::npos) {
		string tmp = aLine.substr(aLine.find("From:") + 6);
		string nick = tmp.substr(0, tmp.find("$") - 1);
		tmp = tmp.substr(tmp.find("$") + 1);
		firePrivateMessage(nick, tmp);
	} else if(aLine.find("$") != string::npos) {
		fireUnknown(aLine);
	} else {
		fireMessage(aLine);
	}
}


/**
 * @file Client.cpp
 * $Id: Client.cpp,v 1.5 2001/12/07 20:03:02 arnetheduck Exp $
 * @if LOG
 * $Log: Client.cpp,v $
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


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
#include "DCClient.h"

DCClient::List DCClient::clientList;

void DCClient::connect(const string& aServer, short aPort) {
	if(socket.isConnected()) {
		disconnect();
	}
	fireConnecting(aServer);
	server = aServer;
	port = aPort;
	socket.addListener(this);
	socket.connect(aServer, aPort);
}

/**
 * Reader thread. Read data from socket and calls getLine whenever there's data to be read.
 * Finishes whenever p->stopEvent is signaled
 * @param p Pointer to the DCClientent that started the thread
 */

void DCClient::onLine(const string& aLine) {
	if(aLine.length() == 0) {
//		dcassert(0); // should never happen
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
	} else if(aLine.find("$HubName") != string::npos) {
		fireHubName(aLine.substr(9));
	} else if(aLine.find("$Lock")!=string::npos) {
	
		string lock = aLine.substr(6);
		lock = lock.substr(0, lock.find(' '));
		string pk = lock.substr(lock.find(' ') + 4);
		fireLock(lock, pk);	
	} else if(aLine.find("$Hello") != string::npos) {
		fireHello(aLine.substr(7));
		users.push_back(aLine.substr(7));
	} else if(aLine.find("$ForceMove") != string::npos) {
		fireForceMove(aLine.substr(11));
	} else if(aLine.find("$HubIsFull") != string::npos) {
		fireHubFull();
	} else if(aLine.find("$MyINFO $ALL") != string::npos) {
		string nick, description, speed, email, bytes;

		string tmp = aLine.substr(13);
		nick = tmp.substr(0, tmp.find(' '));
		tmp = tmp.substr(tmp.find(' ')+1);
		description = tmp.substr(0, tmp.find('$'));
		tmp = tmp.substr(tmp.find('$')+3);
		speed = tmp.substr(0, tmp.find('$')-1);
		tmp = tmp.substr(tmp.find('$')+1);
		email = tmp.substr(0, tmp.find('$'));
		tmp = tmp.substr(tmp.find('$')+1);
		bytes = tmp.substr(0, tmp.find('$'));
		
		fireMyInfo(nick, description, speed, email, bytes);
		
	} else if(aLine.find("$Quit") != string::npos) {
		string nick = aLine.substr(6);
		fireQuit(nick);
		for(StringIter i = users.begin(); i != users.end(); ++i) {
			if(*i == nick) {
				users.erase(i);
			}
		}
	} else if(aLine.find("$ValidateDenide") != string::npos) {
		fireValidateDenied();
	} else if(aLine.find("$NickList") != string::npos) {
		StringList v;
		int j;
		string tmp = aLine.substr(10);
		while( (j=tmp.find("$$")) != string::npos) {
			v.push_back(tmp.substr(0, j));
			tmp = tmp.substr(j+2);
		}
		users.insert(users.end(), v.begin(), v.end());
		fireNickList(v);
		
	} else if(aLine.find("$OpList") != string::npos) {
		StringList v;
		int j;
		string tmp = aLine.substr(8);
		while( (j=tmp.find("$$")) != string::npos) {
			v.push_back(tmp.substr(0, j));
			tmp = tmp.substr(j+2);
		}
		users.insert(users.end(), v.begin(), v.end());
		fireOpList(v);
	} else if(aLine.find("$To") != string::npos) {
		string tmp = aLine.substr(aLine.find("From:") + 6);
		string nick = tmp.substr(0, tmp.find("$") - 1);
		tmp = tmp.substr(tmp.find("$") + 1);
		firePrivateMessage(nick, tmp);
	} else if(aLine.find("$ConnectToMe") != string::npos) {
		string tmp = aLine.substr(13);
		tmp = tmp.substr(tmp.find(' ') + 1);
		string server = tmp.substr(0, tmp.find(':'));
		fireConnectToMe(server, tmp.substr(tmp.find(':')+1));
	} else if(aLine.find("$RevConnectToMe") != string::npos) {
		string tmp = aLine.substr(16);
		fireRevConnectToMe(tmp.substr(tmp.find(' ')+1));
	} else if(aLine.find("$") != string::npos) {
		fireUnknown(aLine);
	} else {
		fireMessage(aLine);
	}
}


/**
 * @file DCClient.cpp
 * $Id: DCClient.cpp,v 1.4 2001/11/25 22:06:25 arnetheduck Exp $
 * @if LOG
 * $Log: DCClient.cpp,v $
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


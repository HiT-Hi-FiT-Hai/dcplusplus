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

void DCClient::connect(const string& aServer, short aPort) {
	if(socket.isConnected()) {
		disconnect();
	}
	fireConnecting(aServer);
	server = aServer;
	port = aPort;
	// Start the reader thread
	ATLASSERT(readerThread == NULL);
	startReader();
}

static const int BUFSIZE = 1024;

/**
 * Reader thread. Read data from socket and calls getLine whenever there's data to be read.
 * Finishes whenever p->stopEvent is signaled
 * @param p Pointer to the DCClientent that started the thread
 */

DWORD WINAPI DCClient::reader(void* p) {
	ATLASSERT(p);
	
	DCClient* client = (DCClient*) p;
	
	HANDLE h[2];
	
	EnterCriticalSection(&client->stopCS);

	h[0] = client->stopEvent;
	
	try {
		client->socket.connect(client->server, client->port);
		h[1] = client->socket.getReadEvent();
	} catch (SocketException e) {
		client->fireConnectionFailed(e.getError());
		LeaveCriticalSection(&client->stopCS);
		return 0;
	}
	LeaveCriticalSection(&client->stopCS);
	
	string line = "";
	BYTE* buf = new BYTE[BUFSIZE];
		
	while(WaitForMultipleObjects(2, h, FALSE, INFINITE) == WAIT_OBJECT_0 + 1) {
		EnterCriticalSection(&client->stopCS);
		try {
			dcdebug("Available bytes: %d\n", client->socket.getAvailable());
			int i = client->socket.read(buf, BUFSIZE);

			string l((char*)buf, i);
			line = line + l;
			while(line.find('|') != string::npos) {
				
				string tmp = line.substr(0, line.find('|'));
				line = line.substr(line.find('|')+1);
				
				client->gotLine(tmp);
			}
		} catch(SocketException e) {
			client->fireConnectionFailed(e.getError());
			LeaveCriticalSection(&client->stopCS);
			return 0;
		}
		LeaveCriticalSection(&client->stopCS);
	}
	return 0;
}

void DCClient::gotLine(const string& aLine) {
	if(aLine.length() == 0) {
		// Do Nothing
	} else if(aLine.find("$HubName") != string::npos) {
		fireHubName(aLine.substr(9));
	} else if(aLine.find("$Lock")!=string::npos) {
	
		string lock = aLine.substr(6);
		lock = lock.substr(0, lock.find(' '));
		string pk = lock.substr(lock.find(' ') + 4);
		fireLock(lock, pk);	
	} else if(aLine.find("$Hello") != string::npos) {
		fireHello(aLine.substr(7));
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
		fireQuit(aLine.substr(6));
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
		fireNickList(v);
		
	} else if(aLine.find("$OpList") != string::npos) {
		StringList v;
		int j;
		string tmp = aLine.substr(8);
		while( (j=tmp.find("$$")) != string::npos) {
			v.push_back(tmp.substr(0, j));
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

string DCClient::keySubst(string aKey, int n) {
	BYTE* temp = new BYTE[aKey.length() + n * 10];
	
	int j=0;
	
	for(int i = 0; i<aKey.length(); i++) {
		if(isExtra(aKey[i])) {
			temp[j++] = '/'; temp[j++] = '%'; temp[j++] = 'D';
			temp[j++] = 'C'; temp[j++] = 'N';
			switch(aKey[i]) {
			case 0: temp[j++] = '0'; temp[j++] = '0'; temp[j++] = '0'; break;
			case 5: temp[j++] = '0'; temp[j++] = '0'; temp[j++] = '5'; break;
			case 36: temp[j++] = '0'; temp[j++] = '3'; temp[j++] = '6'; break;
			case 96: temp[j++] = '0'; temp[j++] = '9'; temp[j++] = '6'; break;
			case 124: temp[j++] = '1'; temp[j++] = '2'; temp[j++] = '4'; break;
			case 126: temp[j++] = '1'; temp[j++] = '2'; temp[j++] = '6'; break;
			}
			temp[j++] = '%'; temp[j++] = '/';
		} else {
			temp[j++] = aKey[i];
		}
	}
	string tmp((char*)temp, j);
	delete temp;
	return tmp;
}

string DCClient::makeKey(const string& lock) {
	BYTE* temp = new BYTE[lock.length()];
	int v1, v2, v3, v4, v5, v6;
	int extra=0;
	
	v1 = lock[0];
	v2 = v1^5;
	v3 = v2 / 0x10;
	v4 = v2 * 0x10;
	v5 = v4 % 0x100;
	v6 = v3 | v5;
	
	temp[0] = (BYTE)v6;
	
	for(int i = 1; i<lock.length(); i++) {
		v1 = lock[i];
		v2 = v1^lock[i-1];
		v3 = v2 / 0x10;
		v4 = v2 * 0x10;
		v5 = v4 % 0x100;
		v6 = v3 | v5;
		temp[i] = (BYTE)v6;
		if(isExtra(temp[i]))
			extra++;
	}
	
	temp[0] = (BYTE)(temp[0] ^ temp[lock.length()-1]);
	
	if(isExtra(temp[0])) {
		extra++;
	}

	string tmp((char*)temp, i);
	delete temp;
	return keySubst(tmp, extra);
}

/**
 * @file DCClient.cpp
 * $Id: DCClient.cpp,v 1.1 2001/11/21 17:33:20 arnetheduck Exp $
 * @if LOG
 * $Log: DCClient.cpp,v $
 * Revision 1.1  2001/11/21 17:33:20  arnetheduck
 * Initial revision
 *
 * @endif
 */


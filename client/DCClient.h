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

#if !defined(AFX_DCCLIENT_H__089CBD05_4833_4E30_9A57_BB636231D78E__INCLUDED_)
#define AFX_DCCLIENT_H__089CBD05_4833_4E30_9A57_BB636231D78E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ClientListener.h"
#include "Socket.h"

class DCClient  
{
public:
	enum {
		SEARCH_PLAIN,
		SEARCH_ATLEAST,
		SEARCH_ATMOST
	};
	typedef DCClient* Ptr;

	DCClient() : readerThread(NULL), stopEvent(NULL) { };
	virtual ~DCClient() {
		stopReader();
	};
	
	void addListener(ClientListener::Ptr aListener) {
		listeners.push_back(aListener);
	}

	void removeListener(ClientListener::Ptr aListener) {
		for(ClientListener::Iter i = listeners.begin(); i != listeners.end(); ++i) {
			if(*i == aListener) {
				listeners.erase(i);
				break;
			}
		}
	}
	
	void removeListeners() {
		listeners.clear();
	}

	void disconnect() {		
		stopReader();
		socket.disconnect();
	}

	void validateNick(const string& aNick) {
		dcdebug("validateNick %s\n", aNick.c_str());
		send("$ValidateNick " + aNick + "|");
	}
	
	void key(const string& aKey) {
		dcdebug("key xxx\n");
		send("$Key " + aKey + "|");
	}
	
	void version(const string& aVersion) {
		dcdebug("version %s\n", aVersion.c_str());
		send("$Version " + aVersion + "|");
	}
	
	void getNickList() {
		dcdebug("getNickList\n");
		send("$GetNickList|");
	}
	
	void sendMessage(const string& aMessage) {
		dcdebug("sendMessage ...\n");
		int i;
		string tmp = aMessage;
		while( (i = tmp.find_first_of("|$")) != string::npos) {
			tmp.erase(i, 1);
		}
		send("<" + Settings::getNick() + "> " + tmp + "|");
	}
	void getInfo(const string& aUser) {
		dcdebug("GetInfo %s\n", aUser.c_str());
		send("$GetINFO " + aUser + " " + Settings::getNick() + "|");
	}
	
	void myInfo(const string& aNick, const string& aDescription, const string& aSpeed, const string& aEmail, const string& aBytesShared) {
		dcdebug("MyInfo %s...\n", aNick.c_str());
		send("$MyINFO $ALL " + aNick + " " + aDescription+ " $ $" + aSpeed + "$" + aEmail + "$" + aBytesShared + "$|");
	}
	
	void connect(const string& aServer, short aPort = 411);
	string makeKey(const string& aLock);

protected:
	ClientListener::List listeners;
	string server;
	short port;
	Socket socket;
	
	HANDLE stopEvent;
	CRITICAL_SECTION stopCS;
	HANDLE readerThread;

	void gotLine(const string& aLine);
	static string keySubst(string aKey, int n);
	static DWORD WINAPI reader(void* p);
	
	void startReader() {
		DWORD threadId;
		InitializeCriticalSection(&stopCS);
		stopEvent=CreateEvent(NULL, FALSE, FALSE, NULL);
		readerThread=CreateThread(NULL, 0, &reader, this, 0, &threadId);
	}

	void stopReader() {
		if(readerThread != NULL) {
			EnterCriticalSection(&stopCS);
			SetEvent(stopEvent);
			LeaveCriticalSection(&stopCS);
			
			if(WaitForSingleObject(readerThread, 1000) == WAIT_TIMEOUT) {
				MessageBox(NULL, _T("Unable to stop reader thread!!!"), _T("Internal error"), MB_OK | MB_ICONERROR);
			}
			
			readerThread = NULL;
			CloseHandle(stopEvent);
			stopEvent = NULL;
			DeleteCriticalSection(&stopCS);
		}
	}

	void send(const string& a) {
		socket.write(a);
	}
	
	static boolean isExtra(BYTE b) {
		if(b == 0 || b==5 || b==124 || b==96 || b==126 || b==36)
			return true;
		else
			return false;
	}
	
	void fireConnecting(const string& aServer) {
		dcdebug("fireConnecting\n");
		for(ClientListener::Iter i=listeners.begin(); i != listeners.end(); ++i) {
			(*i)->onConnecting(aServer);
		}
	}
	void fireSearch(const string& aSeeker, int aSearchType, const string& aSize, int aFileType, const string& aString) {
		dcdebug("fireConnecting\n");
		for(ClientListener::Iter i=listeners.begin(); i != listeners.end(); ++i) {
			(*i)->onSearch(aSeeker, aSearchType, aSize, aFileType, aString);
		}
	}
	void fireHubName(const string& aName) {
		dcdebug("fireHubName %s\n", aName.c_str());
		for(ClientListener::Iter i=listeners.begin(); i != listeners.end(); ++i) {
			(*i)->onHubName(aName);
		}
	}
	void fireLock(const string& aLock, const string& aPk) {
		dcdebug("fireLock %s\n", aLock.c_str());
		for(ClientListener::Iter i=listeners.begin(); i != listeners.end(); ++i) {
			(*i)->onLock(aLock, aPk);
		}
	}
	void firePrivateMessage(const string& aFrom, const string& aMessage) {
		dcdebug("firePM %s ...\n", aFrom.c_str());
		for(ClientListener::Iter i=listeners.begin(); i != listeners.end(); ++i) {
			(*i)->onPrivateMessage(aFrom, aMessage);
		}
	}
	void fireHello(const string& aNick) {
		dcdebug("fireHello %s\n", aNick.c_str());
		for(ClientListener::Iter i=listeners.begin(); i != listeners.end(); ++i) {
			(*i)->onHello(aNick);
		}
	}
	void fireForceMove(const string& aServer) {
		dcdebug("fireForceMove %s\n", aServer.c_str());
		for(ClientListener::Iter i=listeners.begin(); i != listeners.end(); ++i) {
			(*i)->onForceMove(aServer);
		}
	}
	void fireHubFull() {
		dcdebug("fireHubFull\n");
		for(ClientListener::Iter i=listeners.begin(); i != listeners.end(); ++i) {
			(*i)->onHubFull();
		}
	}
	void fireMyInfo(const string& aNick, const string& aDesc, const string& aSpeed, const string& aEmail, const string& aBytes) {
		dcdebug("fireMyInfo %s...\n", aNick.c_str());
		for(ClientListener::Iter i=listeners.begin(); i != listeners.end(); ++i) {
			(*i)->onMyInfo(aNick, aDesc, aSpeed, aEmail, aBytes);
		}
	}
	void fireValidateDenied() {
		dcdebug("fireValidateDenied\n");
		for(ClientListener::Iter i=listeners.begin(); i != listeners.end(); ++i) {
			(*i)->onValidateDenied();
		}
	}
	void fireQuit(const string& aName) {
		dcdebug("fireQuit %s\n", aName.c_str());
		for(ClientListener::Iter i=listeners.begin(); i != listeners.end(); ++i) {
			(*i)->onQuit(aName);
		}
	}
	void fireNickList(StringList& aList) {
		dcdebug("fireNickList ... \n");
		for(ClientListener::Iter i=listeners.begin(); i != listeners.end(); ++i) {
			(*i)->onNickList(aList);
		}
	}
	void fireOpList(StringList& aList) {
		dcdebug("fireOpList ... \n");
		for(ClientListener::Iter i=listeners.begin(); i != listeners.end(); ++i) {
			(*i)->onOpList(aList);
		}
	}
	void fireUnknown(const string& aString) {
		dcdebug("fireUnknown %s\n", aString.c_str());
		for(ClientListener::Iter i=listeners.begin(); i != listeners.end(); ++i) {
			(*i)->onUnknown(aString);
		}
	}
	void fireMessage(const string& aMessage) {
		dcdebug("fireMessage %s\n", aMessage.c_str());
		for(ClientListener::Iter i=listeners.begin(); i != listeners.end(); ++i) {
			(*i)->onMessage(aMessage);
		}
	}
	void fireConnectionFailed(const string& aMessage) {
		dcdebug("fireConnectionFailed %s\n", aMessage.c_str());
		for(ClientListener::Iter i=listeners.begin(); i != listeners.end(); ++i) {
			(*i)->onConnectionFailed(aMessage);
		}
	}
};


#endif // !defined(AFX_DCCLIENT_H__089CBD05_4833_4E30_9A57_BB636231D78E__INCLUDED_)

/**
 * @file DCClient.h
 * $Id: DCClient.h,v 1.2 2001/11/22 19:47:42 arnetheduck Exp $
 * @if LOG
 * $Log: DCClient.h,v $
 * Revision 1.2  2001/11/22 19:47:42  arnetheduck
 * A simple XML parser. Doesn't have all the features, but works good enough for
 * the configuration file.
 *
 * Revision 1.1.1.1  2001/11/21 17:33:20  arnetheduck
 * Inital release
 *
 * @endif
 */


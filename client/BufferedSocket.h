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

#if !defined(AFX_BUFFEREDSOCKET_H__0760BAF6_91F5_481F_BFF7_7CA192EE44CC__INCLUDED_)
#define AFX_BUFFEREDSOCKET_H__0760BAF6_91F5_481F_BFF7_7CA192EE44CC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Socket.h"

class BufferedSocketListener {
public:
	typedef BufferedSocketListener* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;
	
	virtual void onLine(const string& aLine) { };
	virtual void onError(const string& aReason) { };
};

class BufferedSocket : public Socket  
{
	BufferedSocketListener::List listeners;
	string server;
	short port;
	
	char separator;

	HANDLE stopEvent;
	HANDLE readerThread;
	static DWORD WINAPI reader(void* p);

	void startReader() {
		DWORD threadId;
		stopReader();

		stopEvent=CreateEvent(NULL, FALSE, FALSE, NULL);
		readerThread=CreateThread(NULL, 0, &reader, this, 0, &threadId);
	}
	
	void stopReader() {
		if(readerThread != NULL) {
			SetEvent(stopEvent);
			
			if(WaitForSingleObject(readerThread, 1000) == WAIT_TIMEOUT) {
				MessageBox(NULL, _T("Unable to stop reader thread!!!"), _T("Internal error"), MB_OK | MB_ICONERROR);
			}
			
			readerThread = NULL;
			CloseHandle(stopEvent);
			stopEvent = NULL;
		}
	}

	void fireLine(const string& aLine) {
//		dcdebug("fireGotLine %s\n", aLine.c_str());
		for(BufferedSocketListener::Iter i=listeners.begin(); i != listeners.end(); ++i) {
			(*i)->onLine(aLine);
		}
	}
	void fireError(const string& aLine) {
		//		dcdebug("fireGotLine %s\n", aLine.c_str());
		for(BufferedSocketListener::Iter i=listeners.begin(); i != listeners.end(); ++i) {
			(*i)->onError(aLine);
		}
	}
public:
	void addListener(BufferedSocketListener::Ptr aListener) {
		listeners.push_back(aListener);
	}
	void removeListener(BufferedSocketListener::Ptr aListener) {
		for(BufferedSocketListener::Iter i = listeners.begin(); i != listeners.end(); ++i) {
			if(*i == aListener) {
				listeners.erase(i);
				break;
			}
		}
	}
	void removeListeners() {
		listeners.clear();
	}

	virtual void connect(const string& aServer, short aPort) {
		server = aServer;
		port = aPort;
		startReader();
	}
	
	char getSeparator() { return separator; };
	void setSeparator(char aSeparator) { separator = aSeparator; };

	BufferedSocket(char aSeparator = 0x0a) : separator(aSeparator), readerThread(NULL), stopEvent(NULL) {

	};

	virtual ~BufferedSocket() {
		stopReader();
	}
};

#endif // !defined(AFX_BUFFEREDSOCKET_H__0760BAF6_91F5_481F_BFF7_7CA192EE44CC__INCLUDED_)

/**
 * @file BufferedSocket.h
 * $Id: BufferedSocket.h,v 1.1 2001/11/24 10:39:00 arnetheduck Exp $
 * @if LOG
 * $Log: BufferedSocket.h,v $
 * Revision 1.1  2001/11/24 10:39:00  arnetheduck
 * New BufferedSocket creates reader threads and reports inbound data through a listener.
 *
 * @endif
 */
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

#if !defined(AFX_SERVERSOCKET_H__789A5170_2834_4B7B_9E44_A22566439C9F__INCLUDED_)
#define AFX_SERVERSOCKET_H__789A5170_2834_4B7B_9E44_A22566439C9F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class ServerSocketListener {
public:
	typedef ServerSocketListener* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;

	virtual void onIncomingConnection() { };
};

#include "Socket.h"

class ServerSocket  
{
public:
	void addListener(ServerSocketListener::Ptr aListener) {
		listeners.push_back(aListener);
	}
	
	void removeListener(ServerSocketListener::Ptr aListener) {
		for(ServerSocketListener::Iter i = listeners.begin(); i != listeners.end(); ++i) {
			if(*i == aListener) {
				listeners.erase(i);
				break;
			}
		}
	}
	
	void removeListeners() {
		listeners.clear();
	}
	
	void waitForConnections(short aPort) throw(SocketException);

	ServerSocket() : sock(NULL), waiterEvent(NULL), waiterThread(NULL), sockEvent(NULL) {
	};

	virtual ~ServerSocket() {
		stopWaiter();
	}
	
	HANDLE getReadEvent() throw(SocketException) {
		if(sockEvent == NULL) {
			sockEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
			if(sockEvent == NULL)
				throw SocketException(WSAGetLastError());
			
			checksockerr(WSAEventSelect(sock, sockEvent, FD_ACCEPT | FD_READ | FD_CLOSE));
		}
		return sockEvent;
	}
	
	SOCKET getSocket() const {
		return sock;
	}
	
private:
	HANDLE sockEvent;
	SOCKET sock;
	HANDLE waiterEvent;
	HANDLE waiterThread;
	static DWORD WINAPI waiter(void* p);

	ServerSocketListener::List listeners;

	void startWaiter() {
		DWORD threadId;
		stopWaiter();
		
		waiterEvent=CreateEvent(NULL, FALSE, FALSE, NULL);
		waiterThread=CreateThread(NULL, 0, &waiter, this, 0, &threadId);
	}
	
	void stopWaiter() {
		if(waiterThread != NULL) {
			SetEvent(waiterEvent);
			
			if(WaitForSingleObject(waiterThread, 1000) == WAIT_TIMEOUT) {
				MessageBox(NULL, _T("Unable to stop waiter thread!!!"), _T("Internal error"), MB_OK | MB_ICONERROR);
			}
			
			waiterThread = NULL;
			CloseHandle(waiterEvent);
			waiterEvent = NULL;
		}
	}

	void fireIncomingConnection() {
		dcdebug("ServerSocket::fireIncomingConnection\n");
		for(ServerSocketListener::Iter i=listeners.begin(); i != listeners.end(); ++i) {
			(*i)->onIncomingConnection();
		}
	}
};

#endif // !defined(AFX_SERVERSOCKET_H__789A5170_2834_4B7B_9E44_A22566439C9F__INCLUDED_)

/**
 * @file ServerSocket.h
 * $Id: ServerSocket.h,v 1.1 2001/11/25 22:06:25 arnetheduck Exp $
 * @if LOG
 * $Log: ServerSocket.h,v $
 * Revision 1.1  2001/11/25 22:06:25  arnetheduck
 * Finally downloading is working! There are now a few quirks and bugs to be fixed
 * but what the heck....!
 *
 * @endif
 */

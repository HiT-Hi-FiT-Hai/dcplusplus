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

#if !defined(AFX_SERVERSOCKET_H__789A5170_2834_4B7B_9E44_A22566439C9F__INCLUDED_)
#define AFX_SERVERSOCKET_H__789A5170_2834_4B7B_9E44_A22566439C9F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Util.h"

class ServerSocketListener {
public:
	typedef ServerSocketListener* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;
	enum Types {
		INCOMING_CONNECTION
	};
	virtual void onAction(Types) { };
};

#include "Socket.h"
#include "Util.h"

class ServerSocket : public Speaker<ServerSocketListener>  
{
public:
	void waitForConnections(short aPort) throw(SocketException);

	ServerSocket() : sock(NULL), waiterEvent(NULL), waiterThread(NULL), sockEvent(NULL) {
	};
	void disconnect() {
		stopWaiter();
		closesocket(sock);
		CloseHandle(sockEvent);
		sockEvent = NULL;
		sock = NULL;
	}
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

	void startWaiter() {
		DWORD threadId;
		stopWaiter();
		
		waiterEvent=CreateEvent(NULL, FALSE, FALSE, NULL);
		waiterThread=CreateThread(NULL, 0, &waiter, this, 0, &threadId);
	}
	
	void stopWaiter() {
		if(waiterThread != NULL) {
			SetEvent(waiterEvent);
			
			if(WaitForSingleObject(waiterThread, 2000) == WAIT_TIMEOUT) {
				MessageBox(NULL, _T("Unable to stop waiter thread!!!"), _T("Internal error"), MB_OK | MB_ICONERROR);
			}
			
			waiterThread = NULL;
			CloseHandle(waiterEvent);
			waiterEvent = NULL;
		}
	}
};

#endif // !defined(AFX_SERVERSOCKET_H__789A5170_2834_4B7B_9E44_A22566439C9F__INCLUDED_)

/**
 * @file ServerSocket.h
 * $Id: ServerSocket.h,v 1.8 2002/02/09 18:13:51 arnetheduck Exp $
 * @if LOG
 * $Log: ServerSocket.h,v $
 * Revision 1.8  2002/02/09 18:13:51  arnetheduck
 * Fixed level 4 warnings and started using new stl
 *
 * Revision 1.7  2002/01/20 22:54:46  arnetheduck
 * Bugfixes to 0.131 mainly...
 *
 * Revision 1.6  2002/01/11 14:52:57  arnetheduck
 * Huge changes in the listener code, replaced most of it with templates,
 * also moved the getinstance stuff for the managers to a template
 *
 * Revision 1.5  2001/12/13 19:21:57  arnetheduck
 * A lot of work done almost everywhere, mainly towards a friendlier UI
 * and less bugs...time to release 0.06...
 *
 * Revision 1.4  2001/12/02 23:47:35  arnetheduck
 * Added the framework for uploading and file sharing...although there's something strange about
 * the file lists...my client takes them, but not the original...
 *
 * Revision 1.3  2001/12/02 11:16:47  arnetheduck
 * Optimised hub listing, removed a few bugs and leaks, and added a few small
 * things...downloads are now working, time to start writing the sharing
 * code...
 *
 * Revision 1.2  2001/11/26 23:40:36  arnetheduck
 * Downloads!! Now downloads are possible, although the implementation is
 * likely to change in the future...more UI work (splitters...) and some bug
 * fixes. Only user file listings are downloadable, but at least it's something...
 *
 * Revision 1.1  2001/11/25 22:06:25  arnetheduck
 * Finally downloading is working! There are now a few quirks and bugs to be fixed
 * but what the heck....!
 *
 * @endif
 */


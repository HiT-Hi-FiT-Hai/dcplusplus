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
#include "ServerSocket.h"

#define MAX_CONNECTIONS 10

void ServerSocket::waitForConnections(short aPort) throw(SocketException) {
	if(sock)
		closesocket(sock);

	SOCKADDR_IN tcpaddr;
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	
	tcpaddr.sin_family = AF_INET;
	tcpaddr.sin_port = htons(aPort);
	tcpaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	checksockerr(bind(sock, (SOCKADDR *)&tcpaddr, sizeof(tcpaddr)));
	checksockerr(listen(sock, MAX_CONNECTIONS));
	
	startWaiter();
}

DWORD WINAPI ServerSocket::waiter(void* p) {
	ServerSocket* s = (ServerSocket*) p;
	
	HANDLE wait[2];
	wait[0] = s->waiterEvent;
	wait[1] = s->getReadEvent();
	dcdebug("Waiting for incoming connections...\n");
	while(WaitForMultipleObjects(2, wait, FALSE, INFINITE) == WAIT_OBJECT_0 + 1) {
		// Make an extra check that there really is something to accept...
		fd_set fd;
		FD_ZERO(&fd);
		FD_SET(s->sock, &fd);
		TIMEVAL t = { 0, 0 };
		select(1, &fd, NULL, NULL, &t);
		if(FD_ISSET(s->sock, &fd))
			s->fire(ServerSocketListener::INCOMING_CONNECTION);
	}
	dcdebug("Stopped waiting for incoming connections...\n");
	return 0;
}

void ServerSocket::startWaiter() {
	DWORD threadId;
	stopWaiter();
	
	waiterEvent=CreateEvent(NULL, FALSE, FALSE, NULL);
	waiterThread=CreateThread(NULL, 0, &waiter, this, 0, &threadId);
}

void ServerSocket::stopWaiter() {
	if(waiterThread != NULL) {
		SetEvent(waiterEvent);
		
		if(WaitForSingleObject(waiterThread, 3000) == WAIT_TIMEOUT) {
			MessageBox(NULL, _T("Unable to stop waiter thread!!!"), _T("Internal error"), MB_OK | MB_ICONERROR);
		}
		CloseHandle(waiterThread);
		waiterThread = NULL;
		CloseHandle(waiterEvent);
		waiterEvent = NULL;
	}
}

/**
 * @file ServerSocket.cpp
 * $Id: ServerSocket.cpp,v 1.7 2002/03/10 22:41:08 arnetheduck Exp $
 * @if LOG
 * $Log: ServerSocket.cpp,v $
 * Revision 1.7  2002/03/10 22:41:08  arnetheduck
 * Working on internationalization...
 *
 * Revision 1.6  2002/03/04 23:52:31  arnetheduck
 * Updates and bugfixes, new user handling almost finished...
 *
 * Revision 1.5  2002/02/26 23:25:22  arnetheduck
 * Minor updates and fixes
 *
 * Revision 1.4  2002/01/20 22:54:46  arnetheduck
 * Bugfixes to 0.131 mainly...
 *
 * Revision 1.3  2002/01/11 14:52:57  arnetheduck
 * Huge changes in the listener code, replaced most of it with templates,
 * also moved the getinstance stuff for the managers to a template
 *
 * Revision 1.2  2001/12/02 11:16:47  arnetheduck
 * Optimised hub listing, removed a few bugs and leaks, and added a few small
 * things...downloads are now working, time to start writing the sharing
 * code...
 *
 * Revision 1.1  2001/11/25 22:06:25  arnetheduck
 * Finally downloading is working! There are now a few quirks and bugs to be fixed
 * but what the heck....!
 *
 * @endif
 */


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
		s->fireIncomingConnection();
	}
	dcdebug("Stopped waiting for incoming connections...\n");
	s->waiterThread = NULL;
	return 0;
}

/**
 * @file ServerSocket.cpp
 * $Id: ServerSocket.cpp,v 1.2 2001/12/02 11:16:47 arnetheduck Exp $
 * @if LOG
 * $Log: ServerSocket.cpp,v $
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


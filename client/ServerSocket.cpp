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

#include "stdinc.h"
#include "DCPlusPlus.h"

#include "ServerSocket.h"

#define MAX_CONNECTIONS 20

void ServerSocket::waitForConnections(short aPort) throw(SocketException) {
	disconnect();
	
	sockaddr_in tcpaddr;
    checksocket(sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
	
	tcpaddr.sin_family = AF_INET;
	tcpaddr.sin_port = htons(aPort);
	tcpaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	checksockerr(bind(sock, (sockaddr *)&tcpaddr, sizeof(tcpaddr)));
	checksockerr(listen(sock, MAX_CONNECTIONS));
	
	start();
}

int ServerSocket::run() {
	fd_set fd;
	FD_ZERO(&fd);
	FD_SET(sock, &fd);

	dcdebug("Waiting for incoming connections...\n");
	while(select(1, &fd, NULL, NULL, NULL) != -1) {
		if(stop) {
			return 0;
		}

		fire(ServerSocketListener::INCOMING_CONNECTION);
	}
	dcdebug("Stopped waiting for incoming connections...\n");
	
	return 0;
}

/**
 * @file ServerSocket.cpp
 * $Id: ServerSocket.cpp,v 1.10 2002/04/22 13:58:14 arnetheduck Exp $
 */


/*
 * Copyright (C) 2001-2005 Jacek Sieka, arnetheduck on gmail point com
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

#if !defined(SERVER_SOCKET_H)
#define SERVER_SOCKET_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Socket.h"
#include "Speaker.h"

class ServerSocketListener {
public:
	template<int I>	struct X { enum { TYPE = I };  };

	typedef X<0> IncomingConnection;
	virtual void on(IncomingConnection) throw() = 0;
};

class ServerSocket : public Speaker<ServerSocketListener> {
public:
	void waitForConnections(short aPort) throw(SocketException);
	ServerSocket() : sock(INVALID_SOCKET) { };

	virtual ~ServerSocket() throw() {
		disconnect();
	}
	
	void disconnect() throw() {
		if(sock != INVALID_SOCKET) {
			closesocket(sock);
			sock = INVALID_SOCKET;
		}
	}

	socket_t getSocket() const { return sock; }

	/** This is called by windows whenever an "FD_ACCEPT" is sent...doesn't work with unix... */
	void incoming() {
		fire(ServerSocketListener::IncomingConnection());
	}
	
private:
	ServerSocket(const ServerSocket&);
	ServerSocket& operator=(const ServerSocket&);

	socket_t sock;
};

#endif // !defined(SERVER_SOCKET_H)

/**
 * @file
 * $Id: ServerSocket.h,v 1.25 2005/04/24 08:13:12 arnetheduck Exp $
 */

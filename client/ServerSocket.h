/* 
 * Copyright (C) 2001-2003 Jacek Sieka, j_s@telia.com
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

#include "Socket.h"
#include "Speaker.h"

class ServerSocketListener {
public:
	template<int I>	struct X { static const int TYPE = I; };

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
 
#endif // !defined(AFX_SERVERSOCKET_H__789A5170_2834_4B7B_9E44_A22566439C9F__INCLUDED_)

/**
 * @file
 * $Id: ServerSocket.h,v 1.21 2004/04/18 12:51:14 arnetheduck Exp $
 */


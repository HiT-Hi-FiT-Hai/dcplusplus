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

#define checkconnected() if(!connected) throw SocketException("Not connected")

#define BUFSIZE 4096
#ifdef _DEBUG

SocketException::SocketException(int aError) {
	error = "SocketException: " + errorToString(aError);
	dcdebug("Thrown: %s\n", error.c_str());
}

#else // _DEBUG

SocketException::SocketException(int aError) {
	error = errorToString(aError);
}

#endif

Socket::Stats Socket::stats = { 0, 0, 0, 0 };

string SocketException::errorToString(int aError) {
	switch(aError) {
	case EWOULDBLOCK:
		return "Operation would block execution";
	case EACCES:
		return "Permission denied";
	case EADDRINUSE:
		return "Address already in use";
	case EADDRNOTAVAIL:
		return "Address is not available.";
	case EALREADY:
		return "Non-blocking operation still in progress";
	case ECONNREFUSED:
		return "Connection refused by target machine";
	case ETIMEDOUT:
		return "Connection timeout";
	case EHOSTUNREACH:
		return "Host unreachable";
	case ESHUTDOWN:
		return "Socket has been shut down";
	case ECONNABORTED:
		return "Connection closed";
	case ECONNRESET:
		return "Connection reset by server";
	case ENOTSOCK:
		return "Socket error";
	default:
		char tmp[128];
		sprintf(tmp, "Unknown error: 0x%x", aError);
		return tmp;
	}
}

Socket::Socket() throw(SocketException) : event(NULL), connected(false), sock(-1) {
}

Socket::Socket(const string& ip, const string& port) throw(SocketException) : event(NULL), connected(false), sock(-1) {
	connect(ip, port);	
}

Socket::Socket(const string& ip, short port) throw(SocketException) : event(NULL), connected(false), sock(-1) {
	connect(ip, port);	
}

/**
 * Binds an UDP socket to a certain port.
 */
void Socket::bind(short aPort) throw (SocketException){
	dcassert(type == TYPE_UDP);
	dcassert(!isConnected());

	SOCKADDR_IN sock_addr;
		
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port = htons(aPort);
	sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    checksockerr(::bind(sock, (SOCKADDR *)&sock_addr, sizeof(sock_addr)));

	connected = true;
}

void Socket::accept(const ServerSocket& aSocket) throw(SocketException){
	create();
	dcassert(type == TYPE_TCP);
	dcassert(!isConnected());
	checksockerr(sock=::accept(aSocket.getSocket(), NULL, NULL));
	connected = true;
}

/**
 * Connects a socket to an address/ip, closing any other connections made with
 * this instance.
 * @param ip Server IP, in xxx.xxx.xxx.xxx format.
 * @param port Server port.
 * @throw SocketException If any connection error occurs.
 */
void Socket::connect(const string& ip, const string& port) throw(SocketException) {
	connect(ip, atoi(port.c_str()));
}

void Socket::connect(const string& aip, short port) throw(SocketException) {
	SOCKADDR_IN  serv_addr;
	hostent* host;

	dcassert(!isConnected());
	
	if(sock == -1) {
		create();
	}

	memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_port = htons(port);
	serv_addr.sin_family = AF_INET;
	
	serv_addr.sin_addr.s_addr = inet_addr(aip.c_str());

    if (serv_addr.sin_addr.s_addr == INADDR_NONE) {   /* server address is a name or invalid */
        host = gethostbyname(aip.c_str());
        if (host == NULL) {
            throw SocketException("Unknown address");
        }
        serv_addr.sin_addr.s_addr = *((DWORD*)host->h_addr);
		
    }

	setIp(inet_ntoa(serv_addr.sin_addr));
//	dcdebug("Server %s = %s\n", aip.c_str(), getIp().c_str());
	
    if(::connect(sock,(sockaddr*)&serv_addr,sizeof(serv_addr)) == SOCKET_ERROR) {
		// EWOULDBLOCK is ok, the attempt is still being made, and FD_CONNECT will be signaled...
		if(errno != EWOULDBLOCK) {
			checksockerr(SOCKET_ERROR);
		}
	} else {
		connected = true;
	}
}

/**
 * Reads zero to aBufLen characters from this socket, 
 * @param aBuffer A buffer to store the data in.
 * @param aBufLen Size of the buffer.
 * @return Number of bytes read, 0 if disconnected and -1 if the call would block.
 * @throw SocketException On any failure.
 */
int Socket::read(void* aBuffer, int aBufLen) throw(SocketException) {
	checkconnected();
	int len = 0;
	if(type == TYPE_TCP) {
		checkrecv(len=::recv(sock, (char*)aBuffer, aBufLen, 0));
	} else if(type == TYPE_UDP) {
		checkrecv(len=::recvfrom(sock, (char*)aBuffer, aBufLen, 0, NULL, NULL));
	}
	stats.down += len;
	stats.totalDown += len;
	return len;
}

/**
 * Sends data.
 * @todo Fix the blocking stuff!!! This is really ugly...
 * @param aData The string to send
 * @throw SocketExcpetion Send failed.
 */
void Socket::write(const char* aBuffer, int aLen) throw(SocketException) {
	checkconnected();
//	dcdebug("Writing %db: %.100s\n", aLen, aBuffer);
	dcassert(aLen > 0);
	int sendSize = aLen;
	while(aLen) {
		int i = ::send(sock, aBuffer, sendSize, 0);
		if(i == SOCKET_ERROR) {
			if(errno == EWOULDBLOCK) {
				Sleep(10);
			} else if(errno == ENOBUFS) {
				Sleep(10);
				if(sendSize > 32) {
					sendSize /= 2;
					dcdebug("Reducing send window size to %d\n", sendSize);
				} else {
					throw SocketException("Ran out of buffer space");
				}
			} else {
				checksockerr(SOCKET_ERROR);
			}
		} else {
			aLen-=i;
			stats.up += i;
			stats.totalUp += i;
			
		}
	}
}


/**
 * @file Socket.cpp
 * $Id: Socket.cpp,v 1.19 2002/02/07 17:25:28 arnetheduck Exp $
 * @if LOG
 * $Log: Socket.cpp,v $
 * Revision 1.19  2002/02/07 17:25:28  arnetheduck
 * many bugs fixed, time for 0.152 I think
 *
 * Revision 1.18  2002/02/06 12:29:06  arnetheduck
 * New Buffered socket handling with asynchronous sending (asynchronous everything really...)
 *
 * Revision 1.17  2002/02/02 17:21:27  arnetheduck
 * Fixed search bugs and some other things...
 *
 * Revision 1.16  2002/01/20 22:54:46  arnetheduck
 * Bugfixes to 0.131 mainly...
 *
 * Revision 1.15  2002/01/17 23:35:59  arnetheduck
 * Reworked threading once more, now it actually seems stable. Also made
 * sure that noone tries to access client objects that have been deleted
 * as well as some other minor updates
 *
 * Revision 1.14  2002/01/06 21:55:20  arnetheduck
 * Some minor bugs fixed, but there remains one strange thing, the reconnect
 * button doesn't work...
 *
 * Revision 1.13  2002/01/06 00:14:54  arnetheduck
 * Incoming searches almost done, just need some testing...
 *
 * Revision 1.12  2001/12/10 10:48:40  arnetheduck
 * Ahh, finally found one bug that's been annoying me for days...=) the connections
 * in the pool were not reset correctly before being put back for later use...
 *
 * Revision 1.11  2001/12/08 20:59:26  arnetheduck
 * Fixing bugs...
 *
 * Revision 1.10  2001/12/08 14:25:49  arnetheduck
 * More bugs removed...did my first search as well...
 *
 * Revision 1.9  2001/12/07 20:03:25  arnetheduck
 * More work done towards application stability
 *
 * Revision 1.8  2001/12/05 19:40:13  arnetheduck
 * More bugfixes.
 *
 * Revision 1.7  2001/12/05 14:27:35  arnetheduck
 * Premature disconnection bugs removed.
 *
 * Revision 1.6  2001/12/04 21:50:34  arnetheduck
 * Work done towards application stability...still a lot to do though...
 * a bit more and it's time for a new release.
 *
 * Revision 1.5  2001/12/02 11:16:47  arnetheduck
 * Optimised hub listing, removed a few bugs and leaks, and added a few small
 * things...downloads are now working, time to start writing the sharing
 * code...
 *
 * Revision 1.4  2001/11/26 23:40:36  arnetheduck
 * Downloads!! Now downloads are possible, although the implementation is
 * likely to change in the future...more UI work (splitters...) and some bug
 * fixes. Only user file listings are downloadable, but at least it's something...
 *
 * Revision 1.3  2001/11/25 22:06:25  arnetheduck
 * Finally downloading is working! There are now a few quirks and bugs to be fixed
 * but what the heck....!
 *
 * Revision 1.2  2001/11/24 10:39:00  arnetheduck
 * New BufferedSocket creates reader threads and reports inbound data through a listener.
 *
 * Revision 1.1.1.1  2001/11/21 17:33:20  arnetheduck
 * Inital release
 *
 * @endif
 */


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

#include "Socket.h"
#include "ServerSocket.h"

#define checkconnected() if(!isConnected()) throw SocketException(STRING(NOT_CONNECTED))

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
		return STRING(OPERATION_WOULD_BLOCK_EXECUTION);
	case EACCES:
		return STRING(PERMISSION_DENIED);
	case EADDRINUSE:
		return STRING(ADDRESS_ALREADY_IN_USE);
	case EADDRNOTAVAIL:
		return STRING(ADDRESS_NOT_AVAILABLE);
	case EALREADY:
		return STRING(NON_BLOCKING_OPERATION);
	case ECONNREFUSED:
		return STRING(CONNECTION_REFUSED);
	case ETIMEDOUT:
		return STRING(CONNECTION_TIMEOUT);
	case EHOSTUNREACH:
		return STRING(HOST_UNREACHABLE);
	case ESHUTDOWN:
		return STRING(SOCKET_SHUT_DOWN);
	case ECONNABORTED:
		return STRING(CONNECTION_CLOSED);
	case ECONNRESET:
		return STRING(CONNECTION_RESET);
	case ENOTSOCK:
		return STRING(NOT_SOCKET);
	case ENOTCONN:
		return STRING(NOT_CONNECTED);
	default:
		{
			char tmp[64];
			sprintf(tmp, CSTRING(UNKNOWN_ERROR), aError);
			return tmp;
		}
	}
}

/**
 * Binds an UDP socket to a certain local port.
 */
void Socket::bind(short aPort) throw (SocketException){
	dcassert(type == TYPE_UDP);

	sockaddr_in sock_addr;
		
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port = htons(aPort);
	sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    checksockerr(::bind(sock, (sockaddr *)&sock_addr, sizeof(sock_addr)));
	connected = true;
}

void Socket::accept(const ServerSocket& aSocket) throw(SocketException){
	if(sock != INVALID_SOCKET) {
		Socket::disconnect();
	}
	type = TYPE_TCP;
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
void Socket::connect(const string& aip, short port) throw(SocketException) {
	sockaddr_in  serv_addr;
	hostent* host;

	if(sock == INVALID_SOCKET) {
		create();
	}

	memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_port = htons(port);
	serv_addr.sin_family = AF_INET;
	
	serv_addr.sin_addr.s_addr = inet_addr(aip.c_str());

    if (serv_addr.sin_addr.s_addr == INADDR_NONE) {   /* server address is a name or invalid */
        host = gethostbyname(aip.c_str());
        if (host == NULL) {
            throw SocketException(STRING(UNKNOWN_ADDRESS));
        }
        serv_addr.sin_addr.s_addr = *((u_int32_t*)host->h_addr);
    }

	setIp(inet_ntoa(serv_addr.sin_addr));
//	dcdebug("Server %s = %s\n", aip.c_str(), getIp().c_str());
	
	bool blocking = true;
    if(::connect(sock,(sockaddr*)&serv_addr,sizeof(serv_addr)) == SOCKET_ERROR) {
		// EWOULDBLOCK is ok, the attempt is still being made, and FD_CONNECT will be signaled...
		if(errno != EWOULDBLOCK) {
			checksockerr(SOCKET_ERROR);
		} else {
			blocking = true;
		}
	}
	if(blocking)
		connected = true;
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
 * Sends data, will block until all data has been sent or an exception occurs
 * @param aBuffer Buffer with data
 * @param aLen Data length
 * @throw SocketExcpetion Send failed.
 */
void Socket::write(const char* aBuffer, int aLen) throw(SocketException) {
	checkconnected();
//	dcdebug("Writing %db: %.100s\n", aLen, aBuffer);
	dcassert(aLen > 0);
	int pos = 0;
	int sendSize = min(aLen, 8192);

	bool blockAgain = false;

	while(pos < aLen) {
		int i = ::send(sock, aBuffer+pos, min(aLen-pos, sendSize), 0);
		if(i == SOCKET_ERROR) {
			if(errno == EWOULDBLOCK) {
				if(blockAgain) {
					// Uhm, two blocks in a row...try making the send window smaller...
					if(sendSize >= 256) {
						sendSize /= 2;
						dcdebug("Reducing send window size to %d\n", sendSize);
					} else {
						Thread::sleep(10);
					}
					blockAgain = false;
				} else {
					blockAgain = true;
				}

				timeval t;

				// 2 seconds...
				t.tv_sec = 2;
				t.tv_usec = 0;
				fd_set wfd;
				FD_ZERO(&wfd);
				FD_SET(sock, &wfd);
				// Wait until something happens with the socket...
				checksockerr(select(sock+1, NULL, &wfd, NULL, &t));
			} else if(errno == ENOBUFS) {
				if(sendSize > 32) {
					sendSize /= 2;
					dcdebug("Reducing send window size to %d\n", sendSize);
				} else {
					throw SocketException(STRING(OUT_OF_BUFFER_SPACE));
				}
			} else {
				checksockerr(SOCKET_ERROR);
			}
		} else {
			dcassert(i != 0);
			pos+=i;

			stats.up += i;
			stats.totalUp += i;
			blockAgain = false;
		}
	}
}

/**
 * Blocks until timeout is reached or data is available on the socket
 * @return True if there's data, false if there was a timeout
 * @throw SocketException Select fails
 */
bool Socket::waitForData(u_int32_t millis) throw(SocketException) {
	timeval tv;
	fd_set fd;

	tv.tv_sec = millis/1000;
	tv.tv_usec = (millis%1000)*1000; 

	FD_ZERO(&fd);
	FD_SET(sock, &fd);
	int x = select(sock+1, &fd, NULL, NULL, &tv);
	checksockerr(x);
	return FD_ISSET(sock, &fd) > 0;
}

/**
 * Blocks until timeout is reached or data is available on the socket
 * @return True if the socket connected successfully, false if there was a timeout
 * @throw SocketException Select or the connection attempt failed
 */
bool Socket::waitForConnect(u_int32_t millis) throw(SocketException) {
	timeval tv;
	fd_set wfd, efd;

	tv.tv_sec = millis/1000;
	tv.tv_usec = (millis%1000)*1000; 

	FD_ZERO(&wfd);
	FD_ZERO(&efd);

	FD_SET(sock, &wfd);
	FD_SET(sock, &efd);
	int x = select(sock+1, NULL, &wfd, &efd, &tv);
	checksockerr(x);

	if(FD_ISSET(sock, &wfd) || FD_ISSET(sock, &efd)) {
		int y = 0;
		socklen_t z = sizeof(y);
		x = getsockopt(sock, SOL_SOCKET, SO_ERROR, (char*)&y, &z);
		checksockerr(x);
		if(y != 0)
			throw SocketException(y);
		// No errors! We're connected (?)...
		return true;
	}
	return false;
}

/**
 * @file Socket.cpp
 * $Id: Socket.cpp,v 1.38 2002/05/18 11:20:37 arnetheduck Exp $
 */


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

#ifndef _SOCKET_H
#define _SOCKET_H

#include "Exception.h"

#ifdef WIN32

// Berkely constants converted to the windows equivs...
#	define EWOULDBLOCK             WSAEWOULDBLOCK
#	define EINPROGRESS             WSAEINPROGRESS
#	define EALREADY                WSAEALREADY
#	define ENOTSOCK                WSAENOTSOCK
#	define EDESTADDRREQ            WSAEDESTADDRREQ
#	define EMSGSIZE                WSAEMSGSIZE
#	define EPROTOTYPE              WSAEPROTOTYPE
#	define ENOPROTOOPT             WSAENOPROTOOPT
#	define EPROTONOSUPPORT         WSAEPROTONOSUPPORT
#	define ESOCKTNOSUPPORT         WSAESOCKTNOSUPPORT
#	define EOPNOTSUPP              WSAEOPNOTSUPP
#	define EPFNOSUPPORT            WSAEPFNOSUPPORT
#	define EAFNOSUPPORT            WSAEAFNOSUPPORT
#	define EADDRINUSE              WSAEADDRINUSE
#	define EADDRNOTAVAIL           WSAEADDRNOTAVAIL
#	define ENETDOWN                WSAENETDOWN
#	define ENETUNREACH             WSAENETUNREACH
#	define ENETRESET               WSAENETRESET
#	define ECONNABORTED            WSAECONNABORTED
#	define ECONNRESET              WSAECONNRESET
#	define ENOBUFS                 WSAENOBUFS
#	define EISCONN                 WSAEISCONN
#	define ENOTCONN                WSAENOTCONN
#	define ESHUTDOWN               WSAESHUTDOWN
#	define ETOOMANYREFS            WSAETOOMANYREFS
#	define ETIMEDOUT               WSAETIMEDOUT
#	define ECONNREFUSED            WSAECONNREFUSED
#	define ELOOP                   WSAELOOP
#	ifdef ENAMETOOLONG
#		undef ENAMETOOLONG
#	endif
#	define ENAMETOOLONG            WSAENAMETOOLONG
#	define EHOSTDOWN               WSAEHOSTDOWN
#	define EHOSTUNREACH            WSAEHOSTUNREACH
#	ifdef ENOTEMPTY
#		undef ENOTEMPTY
#	endif
#	define ENOTEMPTY               WSAENOTEMPTY
#	define EPROCLIM                WSAEPROCLIM
#	define EUSERS                  WSAEUSERS
#	define EDQUOT                  WSAEDQUOT
#	define ESTALE                  WSAESTALE
#	define EREMOTE                 WSAEREMOTE
#	ifdef EACCES
#		undef EACCES
#	endif
#	define EACCES					WSAEACCES
#	ifdef errno
#		undef errno
#	endif
#	define errno WSAGetLastError()
#	define checksocket(x) if((x) == INVALID_SOCKET) { int a = WSAGetLastError(); Socket::disconnect(); throw SocketException(a); }
#	define checkrecv(x) if((x) == SOCKET_ERROR) { int a = WSAGetLastError(); if(a == EWOULDBLOCK) return -1; else { Socket::disconnect(); throw SocketException(a); } }
#	define checksockerr(x) if((x) == SOCKET_ERROR) { int a = WSAGetLastError(); Socket::disconnect(); throw SocketException(a); }
typedef int socklen_t;
#else
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>

typedef int SOCKET;
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
#	define closesocket(x) close(x)
#	define ioctlsocket(a, b, c) ioctl(a, b, c)
#	define checksocket(x) if((x) < 0) { Socket::disconnect(); throw SocketException(errno); }
#	define checkrecv(x) if((x) == SOCKET_ERROR) { Socket::disconnect(); throw SocketException(errno); }
#	define checksockerr(x) if((x) == SOCKET_ERROR) { Socket::disconnect(); throw SocketException(errno); }
#endif

class SocketException : public Exception {
public:
#ifdef _DEBUG
	SocketException(const string& aError) : Exception("SocketException: " + aError) { };
#else //_DEBUG
	SocketException(const string& aError) : Exception(aError) { };
#endif // _DEBUG
	
	SocketException(int aError);
	virtual ~SocketException() { };
private:
	string errorToString(int aError);
};

class ServerSocket;

class Socket
{
public:
	Socket::Socket() throw(SocketException) : sock(INVALID_SOCKET), connected(false) { }
	
	Socket::Socket(const string& ip, const string& port) throw(SocketException) : sock(INVALID_SOCKET), connected(false) {
		connect(ip, port);	
	}
	
	Socket::Socket(const string& ip, short port) throw(SocketException) : sock(INVALID_SOCKET), connected(false) {
		connect(ip, port);	
	}
	
	virtual ~Socket() {
		Socket::disconnect();
	};

	virtual void bind(short aPort) throw(SocketException);
	
	virtual void connect(const string& ip, short port) throw(SocketException);
	void Socket::connect(const string& ip, const string& port) throw(SocketException) {
		connect(ip, (short)Util::toInt(port));
	}
	
	virtual void disconnect() {
		if(sock != INVALID_SOCKET) {
			closesocket(sock);
		}
		connected = false;

		sock = INVALID_SOCKET;
	}
	enum {
		TYPE_TCP = 0,
		TYPE_UDP = 1
	};

	void create(int aType = TYPE_TCP) throw(SocketException) {
		if(sock != INVALID_SOCKET)
			Socket::disconnect();

		switch(aType) {
		case TYPE_TCP:
			checksocket(sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
			break;
		case TYPE_UDP:
			checksocket(sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP));
			break;
		default:
			dcassert(0);
		}
		type = aType;
	}

	int getAvailable() {
		u_int32_t i = 0;
		ioctlsocket(sock, FIONREAD, &i);
		return i;
	}

	virtual void accept(const ServerSocket& aSocket) throw(SocketException);
	virtual void write(const char* buffer, int len) throw(SocketException);
	virtual void write(const string& aData) throw(SocketException) {
		write(aData.data(), aData.length());
	}

	bool isConnected() { return connected; };

	bool waitForData(u_int32_t millis) throw(SocketException);
	bool waitForConnect(u_int32_t millis) throw(SocketException);

#ifdef WIN32
	void setBlocking(bool block) throw(SocketException) {
		u_long b = block ? 0 : 1;
		ioctlsocket(sock, FIONBIO, &b);
	}
#else
	void setBlocking(bool block) throw(SocketException) {
	}
#endif

	int read(void* aBuffer, int aBufLen) throw(SocketException); 
	
	string getLocalIp() {
		sockaddr_in sock_addr;
		socklen_t len = sizeof(sock_addr);
		if(getsockname(sock, (sockaddr*)&sock_addr, &len) == 0) {
			return inet_ntoa(sock_addr.sin_addr);
		}
		return Util::emptyString;
	}
	static void resetStats() { stats.up = stats.down = 0; };
	static u_int32_t getDown() { return stats.down; };
	static u_int32_t getUp() { return stats.up; };
	static u_int64_t getTotalDown() { return stats.totalDown; };
	static u_int64_t getTotalUp() { return stats.totalUp; };

	GETSETREF(string, ip, Ip);
protected:
	SOCKET sock;
	bool connected;

private:
	Socket(const Socket&) {
		// Copies not allowed
	}
	int type;

	class Stats {
	public:
		u_int32_t down;
		u_int32_t up;
		int64_t totalDown;
		int64_t totalUp;
	};
	static Stats stats;
};

#endif // _SOCKET_H

/**
 * @file Socket.h
 * $Id: Socket.h,v 1.34 2002/05/12 21:54:08 arnetheduck Exp $
 */


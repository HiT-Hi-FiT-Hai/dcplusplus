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

#ifndef SOCKET_H
#define SOCKET_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Util.h"
#include "Exception.h"

#ifdef _WIN32
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
#	define errno ::WSAGetLastError()
#	define checksocket(x) if((x) == INVALID_SOCKET) { int a = WSAGetLastError(); Socket::disconnect(); throw SocketException(a); }
#	define checkrecv(x) if((x) == SOCKET_ERROR) { int a = WSAGetLastError(); if(a == EWOULDBLOCK) return -1; else { Socket::disconnect(); throw SocketException(a); } }
#	define checksockerr(x) if((x) == SOCKET_ERROR) { int a = WSAGetLastError(); Socket::disconnect(); throw SocketException(a); }

typedef int socklen_t;
typedef SOCKET socket_t;

#else
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <fcntl.h>

typedef int socket_t;
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
	SocketException(const string& aError) throw() : Exception("SocketException: " + aError) { }
#else //_DEBUG
	SocketException(const string& aError) throw() : Exception(aError) { }
#endif // _DEBUG
	
	SocketException(int aError) throw();
	virtual ~SocketException() throw() { }
private:
	static string errorToString(int aError) throw();
};

class ServerSocket;

class Socket
{
public:
	enum {
		WAIT_NONE = 0x00,
		WAIT_CONNECT = 0x01,
		WAIT_READ = 0x02,
		WAIT_WRITE = 0x04
	};

	enum {
		TYPE_TCP,
		TYPE_UDP
	};

	Socket::Socket() throw(SocketException) : noproxy(false), sock(INVALID_SOCKET), connected(false) { }
	Socket::Socket(const string& aIp, const string& aPort) throw(SocketException) : noproxy(false), sock(INVALID_SOCKET), connected(false) { connect(aIp, aPort); };
	Socket::Socket(const string& aIp, short aPort) throw(SocketException) : noproxy(false), sock(INVALID_SOCKET), connected(false) { connect(aIp, aPort); };
	virtual ~Socket() { Socket::disconnect(); };

	virtual void create(int aType = TYPE_TCP) throw(SocketException);
	virtual void bind(short aPort) throw(SocketException);
	virtual void connect(const string& aIp, short aPort) throw(SocketException);
	void connect(const string& aIp, const string& aPort) throw(SocketException) { connect(aIp, (short)Util::toInt(aPort)); };
	virtual void accept(const ServerSocket& aSocket) throw(SocketException);
	virtual void write(const char* aBuffer, size_t aLen) throw(SocketException);
	void write(const string& aData) throw(SocketException) { write(aData.data(), aData.length()); };
	virtual void writeTo(const string& aIp, short aPort, const char* aBuffer, size_t aLen) throw(SocketException);
	void writeTo(const string& aIp, short aPort, const string& aData) throw(SocketException) { writeTo(aIp, aPort, aData.data(), aData.length()); };
	virtual void disconnect() throw();

	int read(void* aBuffer, int aBufLen) throw(SocketException);
	int read(void* aBuffer, int aBufLen, string &aIP) throw(SocketException);
	int readFull(void* aBuffer, int aBufLen) throw(SocketException);
	
	int wait(u_int32_t millis, int waitFor) throw(SocketException);
	bool isConnected() { return connected; };
	
	static string resolve(const string& aDns);
	static int64_t getTotalDown() { return stats.totalDown; };
	static int64_t getTotalUp() { return stats.totalUp; };
	
	int getAvailable() {
		u_int32_t i = 0;
		ioctlsocket(sock, FIONREAD, &i);
		return i;
	}

#ifdef _WIN32
	void setBlocking(bool block) throw() {
		u_long b = block ? 0 : 1;
		ioctlsocket(sock, FIONBIO, &b);
	}
#else
	void setBlocking(bool block) throw() {
		int flags = fcntl(sock, F_GETFL, 0); 
		if(block) {
			fcntl(sock, F_SETFL, flags | O_NONBLOCK); 
		} else {
			fcntl(sock, F_SETFL, flags & (~O_NONBLOCK));
		}
	}
#endif
	
	string getLocalIp() throw() {
		if(sock == INVALID_SOCKET)
			return Util::emptyString;

		sockaddr_in sock_addr;
		socklen_t len = sizeof(sock_addr);
		if(getsockname(sock, (sockaddr*)&sock_addr, &len) == 0) {
			return inet_ntoa(sock_addr.sin_addr);
		}
		return Util::emptyString;
	}

	/** When socks settings are updated, this has to be called... */
	static void socksUpdated();

	GETSET(string, ip, Ip);
	GETSET(bool, noproxy, Noproxy);
protected:
	socket_t sock;
	int type;
	bool connected;

	static string udpServer;
	static short udpPort;

private:
	Socket(const Socket&);
	Socket& operator=(const Socket&);

	class Stats {
	public:
		int64_t totalDown;
		int64_t totalUp;
	};
	static Stats stats;
};

#endif // _SOCKET_H

/**
 * @file
 * $Id: Socket.h,v 1.57 2005/01/05 19:30:25 arnetheduck Exp $
 */

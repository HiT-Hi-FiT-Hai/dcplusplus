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

#	ifdef errno
#		undef errno
#	endif
#	define errno WSAGetLastError()
#	define checksocket(x) if((x) == INVALID_SOCKET) { throw SocketException(WSAGetLastError()); }
#	define checksend(x, len) if((x) != len) { throw SocketException(WSAGetLastError()); }
#	define checkrecv(x) if((x) == SOCKET_ERROR) { if(WSAGetLastError() == EWOULDBLOCK) return 0; else throw SocketException(WSAGetLastError()); }
#	define checksockerr(x) if((x) == SOCKET_ERROR) { throw SocketException(WSAGetLastError()); }
#else
#	define closesocket(x) close(x)
#	define checksocket(x) if((x) < 0) { throw SocketException(errno); }
#	define checkconnect(x) if((x) == -1) { throw SocketException(errno); }
#	define checksend(x, len) if((x) != len) { throw SocketException(errno); }
#	define checkrecv(x) if((x) == -1) { throw SocketException(errno); }
#endif


class SocketException : public Exception {
public:
	SocketException(const string& aError) : Exception(aError, "SocketException: ") { };
	SocketException(int aError);
	virtual ~SocketException() { };
private:
	string errorToString(int aError);
};

STANDARD_EXCEPTION(TimeOutException);

class ServerSocket;

class Socket
{
public:
	Socket();
	Socket(const string& ip, const string& port) throw(SocketException);
	Socket(const string& ip, short port) throw(SocketException);
	virtual ~Socket() {
		closesocket(sock);
	};
	
	virtual void connect(const string& ip, short port) throw(SocketException);
	virtual void connect(const string& ip, const string& port) throw(SocketException);
	
	virtual void disconnect() {
		closesocket(sock);
		connected = false;
	}

	int getAvailable() {
		DWORD i;
		ioctlsocket(sock, FIONREAD, &i);
		return i;
	}
	virtual void accept(const ServerSocket& aSocket);
	virtual void write(const char* buffer, int len) throw(SocketException);
	virtual void write(const string& aData) throw(SocketException); 

	boolean isConnected() {
		return connected;
	}

	/**
	 * Returns a handle to an event that fires whenever there is data available in the read buffer.
	 * @return An event to be user with WaitForSingleObject och MultiObjects
	 * @todo This is pretty windows-specific...put it someplace else...
	 */
	HANDLE getReadEvent() throw(SocketException) {
		if(readEvent == NULL) {
			readEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
			if(readEvent == NULL)
				throw SocketException(WSAGetLastError());
			
			checksockerr(WSAEventSelect(sock, readEvent, FD_ACCEPT | FD_READ | FD_CLOSE));
		}
		return readEvent;
	}
		
	int read(void* aBuffer, int aBufLen) throw(SocketException); 
	void writeLine(const string& aData) throw(SocketException);
	virtual string readLine(int aTimeOut = -1, char separator = 0x0a) throw(SocketException, TimeOutException);
protected:
	Socket(const Socket& aSocket) {
		// Copies not allowed
	}
	HANDLE readEvent;
	int sock;
	bool connected;
	string buffer;	

};

#endif // _SOCKET_H

/**
 * @file Socket.h
 * $Id: Socket.h,v 1.3 2001/11/25 22:06:25 arnetheduck Exp $
 * @if LOG
 * $Log: Socket.h,v $
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


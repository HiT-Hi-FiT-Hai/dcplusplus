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

#	ifdef errno
#		undef errno
#	endif
#	define errno WSAGetLastError()
#	define checksocket(x) if((x) == INVALID_SOCKET) { throw SocketException(WSAGetLastError()); }
#	define checksend(x, len) if((x) != len) { throw SocketException(WSAGetLastError()); }
#	define checkrecv(x) if((x) == SOCKET_ERROR) { if(WSAGetLastError() == EWOULDBLOCK) return -1; else throw SocketException(WSAGetLastError()); }
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

STANDARD_EXCEPTION(TimeOutException);

class ServerSocket;

class Socket
{
public:
	Socket();
	Socket(const string& ip, const string& port) throw(SocketException);
	Socket(const string& ip, short port) throw(SocketException);
	virtual ~Socket() {
		disconnect();
		
	};
	virtual void bind(short aPort);
	
	virtual void connect(const string& ip, short port) throw(SocketException);
	virtual void connect(const string& ip, const string& port) throw(SocketException);
	
	virtual void disconnect() {
		connected = false;
		closesocket(sock);
		sock = -1;
		if(event) {
			CloseHandle(event);
			event = NULL;
		}
	}
	enum {
		TYPE_TCP = 0,
		TYPE_UDP = 1
	};
	void create(int aType = TYPE_TCP) throw(SocketException) {
		if(sock != -1)
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
		DWORD i = 0;
		ioctlsocket(sock, FIONREAD, &i);
		return i;
	}
	virtual void accept(const ServerSocket& aSocket);
	virtual void write(const char* buffer, int len) throw(SocketException);
	virtual void write(const string& aData) throw(SocketException) {
		write(aData.data(), aData.length());
	}

	bool isConnected() {
		return connected;
	}
	void setConnected() {
		connected = true;
	}
	/**
	 * Returns a handle to an event that fires whenever there is data available in the read buffer.
	 * Note; The socket will automatically be put in non-blocking mode after returning from this 
	 * function, and there's no way back!
	 * @return An event to be user with WaitForSingleObject och MultiObjects
	 * @todo This is pretty windows-specific...put it someplace else...
	 */
	HANDLE getEvent() throw(SocketException) {
		if(event == NULL) {
			event = CreateEvent(NULL, FALSE, FALSE, NULL);
			if(event == NULL)
				throw SocketException(WSAGetLastError());
			
			checksockerr(WSAEventSelect(sock, event, FD_CONNECT | FD_ACCEPT | FD_READ | FD_WRITE | FD_CLOSE));
		}
		return event;
	}
		
	int read(void* aBuffer, int aBufLen) throw(SocketException); 
	
	string getLocalIp() {
		SOCKADDR_IN sock_addr;
		int len = sizeof(sock_addr);
		if(getsockname(sock, (sockaddr*)&sock_addr, &len) == 0) {
			return inet_ntoa(sock_addr.sin_addr);
		}
		return "";
	}
	static void resetStats() { stats.up = stats.down = 0; };
	static DWORD getDown() { return stats.down; };
	static DWORD getUp() { return stats.up; };
	static LONGLONG getTotalDown() { return stats.totalDown; };
	static LONGLONG getTotalUp() { return stats.totalUp; };
	int sock;

	GETSETREF(string, ip, Ip);
private:
	Socket(const Socket&) {
		// Copies not allowed
	}
	int type;
	HANDLE event;
	bool connected;

	class Stats {
	public:
		DWORD down;
		DWORD up;
		LONGLONG totalDown;
		LONGLONG totalUp;
	};
	static Stats stats;
};

#endif // _SOCKET_H

/**
 * @file Socket.h
 * $Id: Socket.h,v 1.20 2002/02/09 18:13:51 arnetheduck Exp $
 * @if LOG
 * $Log: Socket.h,v $
 * Revision 1.20  2002/02/09 18:13:51  arnetheduck
 * Fixed level 4 warnings and started using new stl
 *
 * Revision 1.19  2002/02/07 22:12:22  arnetheduck
 * Last fixes before 0.152
 *
 * Revision 1.18  2002/02/07 17:25:28  arnetheduck
 * many bugs fixed, time for 0.152 I think
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
 * Revision 1.14  2002/01/11 14:52:57  arnetheduck
 * Huge changes in the listener code, replaced most of it with templates,
 * also moved the getinstance stuff for the managers to a template
 *
 * Revision 1.13  2002/01/06 21:55:20  arnetheduck
 * Some minor bugs fixed, but there remains one strange thing, the reconnect
 * button doesn't work...
 *
 * Revision 1.12  2001/12/13 19:21:57  arnetheduck
 * A lot of work done almost everywhere, mainly towards a friendlier UI
 * and less bugs...time to release 0.06...
 *
 * Revision 1.11  2001/12/10 10:48:40  arnetheduck
 * Ahh, finally found one bug that's been annoying me for days...=) the connections
 * in the pool were not reset correctly before being put back for later use...
 *
 * Revision 1.10  2001/12/08 14:25:49  arnetheduck
 * More bugs removed...did my first search as well...
 *
 * Revision 1.9  2001/12/07 20:03:26  arnetheduck
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


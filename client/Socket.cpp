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

#define checkconnected() if(!connected) throw SocketException("Not connected")

#define BUFSIZE 4096
SocketException::SocketException(int aError) {
	error = errorToString(aError);
}

string SocketException::errorToString(int aError) {
	switch(aError) {
	case EWOULDBLOCK:
		return "Operation would block execution.";
	case EACCES:
		return "Permission denied.";
	case EADDRINUSE:
		return "Address already in use.";
	case EADDRNOTAVAIL:
		return "Address is not available.";
	case EALREADY:
		return "Non-blocking operation still in progress.";
	case ECONNREFUSED:
		return "Connection refused by target machine.";
	case ETIMEDOUT:
		return "Connection timeout.";
	case EHOSTUNREACH:
		return "Host unreachable.";
	case ESHUTDOWN:
		return "Socket has been shut down.";
	case ECONNABORTED:
		return "Connection aborted.";
	case ECONNRESET:
		return "Connection reset by server.";
		
	default:
		char tmp[1024];
		sprintf(tmp, "Unknown error: 0x%x", aError);
		return tmp;
	}
}

Socket::Socket() : readEvent(NULL), connected(false), sock(-1) {
	buffer = "";
	buffer.reserve(256);
}

Socket::Socket(const string& ip, const string& port) throw(SocketException) : readEvent(NULL), connected(false), sock(-1) {
	buffer = "";
	buffer.reserve(256);
	connect(ip, port);	
}

Socket::Socket(const string& ip, short port) throw(SocketException) : readEvent(NULL), connected(false), sock(-1) {
	buffer = "";
	buffer.reserve(256);
	connect(ip, port);	
}

void Socket::accept(const ServerSocket& aSocket) throw(SocketException){
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

void Socket::connect(const string& ip, short port) throw(SocketException) {
	
	SOCKADDR_IN  serv_addr;
	hostent* host;

	if(sock != -1 || connected) {
		closesocket(sock);
	}

	checksocket(sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
	
	memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_port = htons(port);
	serv_addr.sin_family = AF_INET;
	
    if (isalpha(ip[0])) {   /* server address is a name */
        host = gethostbyname(ip.c_str());
        if (host == NULL) {
            throw SocketException("Unknown address");
        }
        serv_addr.sin_addr.s_addr = *((DWORD*)host->h_addr);
		dcdebug("Server %s = %s\n", ip.c_str(), inet_ntoa(serv_addr.sin_addr));
		
    } else { /* Convert nnn.nnn address to a usable one */
        serv_addr.sin_addr.s_addr = inet_addr(ip.c_str());
    } 
	
    checksockerr(::connect(sock,(sockaddr*)&serv_addr,sizeof(serv_addr))); 
	
	connected = true;

}

/**
 * Reads zero to aBufLen characters from this socket, 
 * @param aBuffer A buffer to store the data in.
 * @param aBufLen Size of the buffer.
 * @return Number of bytes read.
 * @throw SocketException On any failure.
 */
int Socket::read(void* aBuffer, int aBufLen) throw(SocketException) {
	checkconnected();
	int len = 0;
	checkrecv(len=::recv(sock, (char*)aBuffer, aBufLen, 0));
	return len;
}

/**
 * Sends data, throwing an error if all data is not sent (note; an error may be thrown
 * even if some data has been sent).
 * @todo Fix the blocking stuff!!! This is really ugly...and slows things down dramatically...maybe an internal buffer?
 * @param aData The string to send
 * @throw SocketExcpetion Send failed.
 */
void Socket::write(const char* aBuffer, int aLen) throw(SocketException) {
	checkconnected();
resend:
	if(::send(sock, aBuffer, aLen, 0) == SOCKET_ERROR) {
		if(errno == EWOULDBLOCK) {
			Sleep(10);
			goto resend;
		}
	}
}

void Socket::write(const string& aData) throw(SocketException) {
	write(aData.c_str(), aData.length());
}

/**
 * Sends data, throwing an error if all data is not sent. (note; an error may be thrown
 * even if some data has been sent). A LF(0x0a) is appended to the string sent.
 * @param aData The string to send
 * @throw SocketExcpetion Send failed.
 */
void Socket::writeLine(const string& aData) throw(SocketException) {
	checkconnected();

	string temp = aData + (char)0x0a;
	write(temp.c_str(), temp.length());
}

/**
 * Reads a line of input, waiting until there is one.
 * @param aTimeOut Timeout in seconds
 * @throw SocketException Read error or connection lost.
 * @throw TimeOutExecption No data received befor timeout.
 */
string Socket::readLine(int aTimeOut, char aSeparator) throw(SocketException, TimeOutException) {
	char buf[BUFSIZE];
	int len;
	
	if( !connected ) {
		throw SocketException("Not connected");
	}

	while( buffer.find_first_of(aSeparator, 0) == string::npos) {
		
		// Wait for data
		timeval tv;
		tv.tv_sec = aTimeOut;
		tv.tv_usec = 0;

		fd_set set;
		FD_ZERO(&set);
		FD_SET(sock, &set);
		
		select(sock + 1, &set, NULL, NULL, (aTimeOut == -1)?NULL : &tv);
		if(! FD_ISSET(sock, &set) ) {
			throw TimeOutException("No data received within timeout");
		}
		
		checkrecv(len = ::recv(sock, buf, BUFSIZE, 0));
		if(len == 0) {
			// We've lost our connection!!
			connected = false;
			throw SocketException("Connection lost");

		}
		buffer.append(buf, len);
	}

	int i;
	string temp(buffer, 0, (i=buffer.find_first_of(aSeparator, 0)));
	buffer.erase(0, i + 1);
	return temp;
}

/**
 * @file Socket.cpp
 * $Id: Socket.cpp,v 1.4 2001/11/26 23:40:36 arnetheduck Exp $
 * @if LOG
 * $Log: Socket.cpp,v $
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


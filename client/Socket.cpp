/* 
 * Copyright (C) 2001-2004 Jacek Sieka, j_s at telia com
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

#include "SettingsManager.h"
#include "ResourceManager.h"

#include "ServerSocket.h"

string Socket::udpServer;
short Socket::udpPort;

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

Socket::Stats Socket::stats = { 0, 0 };

string Socket::getRemoteIp() const {
	sockaddr_in sock_addr_rem = { 0 };
	if(type == TYPE_TCP) {
		socklen_t len = sizeof(sock_addr_rem);
		if(getpeername(sock, (sockaddr*)&sock_addr_rem, &len) == SOCKET_ERROR)
			return Util::emptyString;
	}

	return string(inet_ntoa(sock_addr_rem.sin_addr));	// + ":" + string(Util::toString((sock_addr_rem.sin_port >> 8) | (sock_addr_rem.sin_port << 8 & 0xffff)));
}

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
	case ENETUNREACH:
		return STRING(NETWORK_UNREACHABLE);
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
#ifdef _WIN32
	// Make sure we disable any inherited windows message things for this socket.
	::WSAAsyncSelect(sock, NULL, 0, 0);
#endif
	setBlocking(true);
	connected = true;
	
}

/**
 * Connects a socket to an address/ip, closing any other connections made with
 * this instance.
 * @param aAddr Server address, in dns or xxx.xxx.xxx.xxx format.
 * @param aPort Server port.
 * @throw SocketException If any connection error occurs.
 */
void Socket::connect(const string& aAddr, short aPort) throw(SocketException) {
	sockaddr_in  serv_addr;
	hostent* host;

	if(sock == INVALID_SOCKET) {
		create();
	}

	memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_port = htons(aPort);
	serv_addr.sin_family = AF_INET;
	
	serv_addr.sin_addr.s_addr = inet_addr(aAddr.c_str());

    if (serv_addr.sin_addr.s_addr == INADDR_NONE) {   /* server address is a name or invalid */
        host = gethostbyname(aAddr.c_str());
        if (host == NULL) {
            throw SocketException(STRING(UNKNOWN_ADDRESS));
        }
        serv_addr.sin_addr.s_addr = *((u_int32_t*)host->h_addr);
    }

	setIp(inet_ntoa(serv_addr.sin_addr));
	
    if(::connect(sock,(sockaddr*)&serv_addr,sizeof(serv_addr)) == SOCKET_ERROR) {
		// EWOULDBLOCK is ok, the attempt is still being made, and FD_CONNECT will be signaled...
		if(errno != EWOULDBLOCK) {
			checksockerr(SOCKET_ERROR);
		}
	}
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
	stats.totalDown += len;
	return len;
}

/**
 * Reads zero to aBufLen characters from this socket, 
 * @param aBuffer A buffer to store the data in.
 * @param aBufLen Size of the buffer.
 * @param aIP Remote IP address
 * @return Number of bytes read, 0 if disconnected and -1 if the call would block.
 * @throw SocketException On any failure.
 */
int Socket::read(void* aBuffer, int aBufLen, string &aIP) throw(SocketException) {
	checkconnected();
	int len = 0;

	sockaddr_in remote_addr = { 0 };
	socklen_t addr_length = sizeof(remote_addr);

	checkrecv(len=::recvfrom(sock, (char*)aBuffer, aBufLen, 0, (sockaddr*)&remote_addr, &addr_length)); //
	aIP = string(inet_ntoa(remote_addr.sin_addr));

	stats.totalDown += len;
	return len;
}

/**
 * Reads data until aBufLen bytes have been read or an error occurs.
 * On error, an unspecified amount of bytes might have already been read...
 */
int Socket::readFull(void* aBuffer, int aBufLen) throw(SocketException) {
	int i = 0;
	int j;
	while(i < aBufLen) {
		if((j = read(((char*)aBuffer) + i, aBufLen - i)) <= 0) {
			return j;
		}
		i += j;
	}
	return i;
}

/**
 * Sends data, will block until all data has been sent or an exception occurs
 * @param aBuffer Buffer with data
 * @param aLen Data length
 * @throw SocketExcpetion Send failed.
 */
void Socket::write(const char* aBuffer, size_t aLen) throw(SocketException) {
	checkconnected();
//	dcdebug("Writing %db: %.100s\n", aLen, aBuffer);
        
        if(aLen == 0){
                return;
        }

	size_t pos = 0;
	size_t sendSize = min(aLen, (size_t)64 * 1024);

	bool blockAgain = false;

	while(pos < aLen) {
		int i = ::send(sock, aBuffer+pos, (int)min(aLen-pos, sendSize), 0);
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
				wait(2000, WAIT_WRITE);

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

			stats.totalUp += i;
			blockAgain = false;
		}
	}
}

/**
* Sends data, will block until all data has been sent or an exception occurs
* @param aBuffer Buffer with data
* @param aLen Data length
* @throw SocketExcpetion Send failed.
*/
void Socket::writeTo(const string& ip, short port, const char* aBuffer, size_t aLen) throw(SocketException) {
	if(sock == INVALID_SOCKET) {
		create(TYPE_UDP);
	}
	dcassert(type == TYPE_UDP);
	//	dcdebug("Writing %db: %.100s\n", aLen, aBuffer);
	dcassert(aLen > 0);
	dcassert(aLen < 1450);
	dcassert(sock != INVALID_SOCKET);

	sockaddr_in  serv_addr;
	hostent* host;

	if(ip.empty() || port == 0) {
		throw SocketException(STRING(ADDRESS_NOT_AVAILABLE));
	}

	memset(&serv_addr, 0, sizeof(serv_addr));

	if(SETTING(CONNECTION_TYPE) == SettingsManager::CONNECTION_SOCKS5 && !noproxy) {

		if(udpServer.empty() || udpPort == 0) {
			throw SocketException(STRING(SOCKS_SETUP_ERROR));
		}

		serv_addr.sin_port = htons(udpPort);
		serv_addr.sin_family = AF_INET;
		
		serv_addr.sin_addr.s_addr = inet_addr(udpServer.c_str());
		
		string s = BOOLSETTING(SOCKS_RESOLVE) ? resolve(ip) : ip;

		// Alrite, let's get on with it...
		AutoArray<u_int8_t> connStr(10 + s.length() + aLen);
		connStr[0] = 0;		// Reserved
		connStr[1] = 0;		// Reserved
		connStr[2] = 0;		// Fragment number, 0 always in our case...
		
		int connLen;
		if(BOOLSETTING(SOCKS_RESOLVE)) {
			
			u_int8_t slen =(u_int8_t)(s.length() & 0xff);
			connStr[3] = 3;		// Address type: domain name
			connStr[4] = slen;
			strncpy((char*)(u_int8_t*)connStr + 5, s.c_str(), slen);
			*((u_int16_t*)(&connStr[5 + slen])) = htons(port);
			connLen = 7 + slen;
		} else {
			connStr[3] = 1;		// Address type: IPv4;
			*((long*)(&connStr[4])) = inet_addr(s.c_str());
			*((u_int16_t*)(&connStr[8])) = htons(port);	
			connLen = 10;
		}

		memcpy(((u_int8_t*)connStr) + connLen, aBuffer, aLen);

		int i = ::sendto(sock, (char*)(u_int8_t*)connStr, connLen + (int)aLen, 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
		checksockerr(i);
		
		stats.totalUp += i;
	} else {
		serv_addr.sin_port = htons(port);
		serv_addr.sin_family = AF_INET;
		
		serv_addr.sin_addr.s_addr = inet_addr(ip.c_str());
		
		if (serv_addr.sin_addr.s_addr == INADDR_NONE) {   /* server address is a name or invalid */
			host = gethostbyname(ip.c_str());
			if (host == NULL) {
				throw SocketException(STRING(UNKNOWN_ADDRESS));
			}
			serv_addr.sin_addr.s_addr = *((u_int32_t*)host->h_addr);
		}
		
		int i = ::sendto(sock, aBuffer, (int)aLen, 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
		checksockerr(i);
		
		stats.totalUp += i;
	}
}

/**
 * Blocks until timeout is reached one of the specified conditions have been fulfilled
 * @param waitFor WAIT_*** flags that set what we're waiting for, set to the combination of flags that
 *				  triggered the wait stop on return (==WAIT_NONE on timeout)
 * @return WAIT_*** ored together of the current state.
 * @throw SocketException Select or the connection attempt failed.
 */
int Socket::wait(u_int32_t millis, int waitFor) throw(SocketException) {
	timeval tv;
	fd_set rfd, wfd, efd;
	fd_set *rfdp = NULL, *wfdp = NULL;
	tv.tv_sec = millis/1000;
	tv.tv_usec = (millis%1000)*1000; 

	if(waitFor & WAIT_CONNECT) {
		dcassert(!(waitFor & WAIT_READ) && !(waitFor & WAIT_WRITE));

		FD_ZERO(&wfd);
		FD_ZERO(&efd);

		FD_SET(sock, &wfd);
		FD_SET(sock, &efd);
		checksockerr(select((int)sock+1, NULL, &wfd, &efd, &tv));

		if(FD_ISSET(sock, &wfd) || FD_ISSET(sock, &efd)) {
			int y = 0;
			socklen_t z = sizeof(y);
			checksockerr(getsockopt(sock, SOL_SOCKET, SO_ERROR, (char*)&y, &z));

			if(y != 0)
				throw SocketException(y);
			// No errors! We're connected (?)...
			return WAIT_CONNECT;
		}
		return 0;
	}

	if(waitFor & WAIT_READ) {
		dcassert(!(waitFor & WAIT_CONNECT));
		rfdp = &rfd;
		FD_ZERO(rfdp);
		FD_SET(sock, rfdp);
	}
	if(waitFor & WAIT_WRITE) {
		dcassert(!(waitFor & WAIT_CONNECT));
		wfdp = &wfd;
		FD_ZERO(wfdp);
		FD_SET(sock, wfdp);
	}
	waitFor = WAIT_NONE;
	checksockerr(select((int)sock+1, rfdp, wfdp, NULL, &tv));

	if(rfdp && FD_ISSET(sock, rfdp)) {
		waitFor |= WAIT_READ;
	}
	if(wfdp && FD_ISSET(sock, wfdp)) {
		waitFor |= WAIT_WRITE;
	}

	return waitFor;
}

string Socket::resolve(const string& aDns) {
	sockaddr_in sock_addr;

	memset(&sock_addr, 0, sizeof(sock_addr));
	sock_addr.sin_port = 0;
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_addr.s_addr = inet_addr(aDns.c_str());

	if (sock_addr.sin_addr.s_addr == INADDR_NONE) {   /* server address is a name or invalid */
		hostent* host;
		host = gethostbyname(aDns.c_str());
		if (host == NULL) {
			return Util::emptyString;
		}
		sock_addr.sin_addr.s_addr = *((u_int32_t*)host->h_addr);
		return inet_ntoa(sock_addr.sin_addr);
	} else {
		return aDns;
	}
}

void Socket::socksUpdated() {
	udpServer.clear();
	udpPort = 0;
	
	if(SETTING(CONNECTION_TYPE) == SettingsManager::CONNECTION_SOCKS5) {
		try {
			Socket s(SETTING(SOCKS_SERVER), (short)SETTING(SOCKS_PORT));
			
			if(SETTING(SOCKS_USER).empty() && SETTING(SOCKS_PASSWORD).empty()) {
				// No username and pw, easier...=)
				char connStr[3];
				connStr[0] = 5;			// SOCKSv5
				connStr[1] = 1;			// 1 method
				connStr[2] = 0;			// Method 0: No auth...
				
				s.write(connStr, 3);
				
				if(s.readFull(connStr, 2) <= 0)
					return;
				
				if(connStr[1] != 0) {
					return;
				}				
			} else {
				// We try the username and password auth type (no, we don't support gssapi)
				u_int8_t ulen = (u_int8_t)(SETTING(SOCKS_USER).length() & 0xff);
				u_int8_t plen = (u_int8_t)(SETTING(SOCKS_PASSWORD).length() & 0xff);
				AutoArray<u_int8_t> connStr(3 + ulen + plen);
				
				connStr[0] = 5;			// SOCKSv5
				connStr[1] = 1;			// 1 method
				connStr[2] = 2;			// Method 2: Name/Password...
				s.write((char*)(u_int8_t*)connStr, 3);
				if(s.readFull((char*)(u_int8_t*)connStr, 2) <= 0)
					return;

				if(connStr[1] != 2) {
					return;
				}				
				// Now we send the username / pw...
				connStr[0] = 1;
				connStr[1] = ulen;
				strncpy((char*)(u_int8_t*)connStr + 2, SETTING(SOCKS_USER).c_str(), ulen);
				connStr[2 + ulen] = plen;
				strncpy((char*)(u_int8_t*)connStr + 3 + ulen, SETTING(SOCKS_PASSWORD).c_str(), plen);
				s.write((char*)(u_int8_t*)connStr, 3 + plen + ulen);
				
				if(s.readFull((char*)(u_int8_t*)connStr, 2) <= 0) {
					return;
				}
				
				if(connStr[1] != 0) {
					return;
				}
				
			}
			// Alrite, let's get on with it...
			char connStr[10];
			connStr[0] = 5;			// SOCKSv5
			connStr[1] = 3;			// UDP Associate
			connStr[2] = 0;			// Reserved
			connStr[3] = 1;			// Address type: IPv4;
			*((long*)(&connStr[4])) = 0;		// No specific outgoing UDP address
			*((u_int16_t*)(&connStr[8])) = 0;	// No specific port...
			
			s.write(connStr, 10);
			
			// We assume we'll get a ipv4 address back...therefore, 10 bytes...if not, things
			// will break, but hey...noone's perfect (and I'm tired...)...
			if(s.readFull(connStr, 10) <= 0) {
				return;
			}

			if(connStr[0] != 5 || connStr[1] != 0) {
				return;
			}

			udpPort = (short)ntohs(*((u_int16_t*)(&connStr[8])));

			sockaddr_in  serv_addr;
			
			memset(&serv_addr, 0, sizeof(serv_addr));
			serv_addr.sin_port = htons(udpPort);
			serv_addr.sin_family = AF_INET;
			
			serv_addr.sin_addr.s_addr = *((long*)(&connStr[4]));
			udpServer = inet_ntoa(serv_addr.sin_addr);
		} catch(const SocketException&) {
			dcdebug("Socket: Failed to register with socks server\n");
			// ...
		}
	}
}

/**
 * @file
 * $Id: Socket.cpp,v 1.58 2004/09/09 09:27:36 arnetheduck Exp $
 */


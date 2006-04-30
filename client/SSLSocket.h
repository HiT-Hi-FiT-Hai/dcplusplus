/*
 * Copyright (C) 2001-2006 Jacek Sieka, arnetheduck on gmail point com
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

#if !defined(SSLSOCKET_H)
#define SSLSOCKET_H

#include "Socket.h"
#include "Singleton.h"

class SSLSocket;

namespace yaSSL {
	class SSL;
	class SSL_CTX;
	struct DH;
}

using namespace yaSSL;

class SSLSocketFactory : public Singleton<SSLSocketFactory> {
public:
	SSLSocketFactory();
	virtual ~SSLSocketFactory();

	SSLSocket* getClientSocket() throw(SocketException);
	SSLSocket* getServerSocket() throw(SocketException);

	void loadCertificates() throw();
	bool hasCerts() const { return certsLoaded; }
private:
	SSL_CTX* clientContext;
	SSL_CTX* serverContext;
	DH* dh;
	bool certsLoaded;
};

class SSLSocket : public Socket {
public:
	virtual ~SSLSocket() throw() {}

	virtual void accept(const Socket& listeningSocket) throw(SocketException);
	virtual void connect(const string& aIp, short aPort) throw(SocketException);
	virtual int read(void* aBuffer, int aBufLen) throw(SocketException);
	virtual int write(const void* aBuffer, int aLen) throw(SocketException);
	virtual int wait(u_int32_t millis, int waitFor) throw(SocketException);
	virtual void shutdown() throw();
	virtual void close() throw();
private:
	friend class SSLSocketFactory;

	SSLSocket(SSL_CTX* context) throw(SocketException);
	SSLSocket(const SSLSocket&);
	SSLSocket& operator=(const SSLSocket&);

	SSL_CTX* ctx;
	SSL* ssl;

	int checkSSL(int ret) throw(SocketException);
};

#endif // SSLSOCKET_H

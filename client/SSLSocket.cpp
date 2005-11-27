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

#include "stdinc.h"
#include "DCPlusPlus.h"

#include "SSLSocket.h"
#include <openssl/ssl.h>

SSLSocketFactory::SSLSocketFactory() : clientContext(SSL_CTX_new(TLSv1_client_method())), serverContext(SSL_CTX_new(TLSv1_server_method())) {
	SSL_CTX_set_verify(serverContext, SSL_VERIFY_NONE, 0);
	SSL_CTX_set_verify(clientContext, SSL_VERIFY_NONE, 0);
}

SSLSocketFactory::~SSLSocketFactory() {
	SSL_CTX_free(serverContext);
	SSL_CTX_free(clientContext);
	serverContext = clientContext = 0;
}

SSLSocket* SSLSocketFactory::getClientSocket() {
	return new SSLSocket(clientContext);
}
SSLSocket* SSLSocketFactory::getServerSocket() {
	return new SSLSocket(serverContext);
}

SSLSocket::SSLSocket(SSL_CTX* context) {
	ssl = SSL_new(context);
}

void SSLSocket::connect(const string& aIp, short aPort) throw(SocketException) {
	Socket::connect(aIp, aPort);
	
	SSL_set_fd(ssl, sock);
	SSL_connect(ssl);
}

void SSLSocket::accept(const Socket& listeningSocket) throw(SocketException) {
	Socket::accept(listeningSocket);
	checkSSL(SSL_accept(ssl));
}
int SSLSocket::read(void* aBuffer, int aBufLen) throw(SocketException) {
	return checkSSL(SSL_read(ssl, aBuffer, aBufLen));
}

int SSLSocket::write(const void* aBuffer, int aLen) throw(SocketException) {
	int ret = SSL_write(ssl, aBuffer, aLen);
	return ret;
}

int SSLSocket::checkSSL(int ret) {
	if(ret <= 0) {
		int err = SSL_get_error(ssl, ret);
		switch(SSL_get_error(ssl, ret)) {
			case SSL_ERROR_WANT_READ:	// Fallthrough
			case SSL_ERROR_WANT_WRITE:
				return -1;
			default:
				throw SocketException("SSL Error: " + Util::toString(ret) + ", " + Util::toString(err)); // @todo Translate
		}
	}
	return ret;
}

void SSLSocket::shutdown() throw() {
	SSL_shutdown(ssl);
}

void SSLSocket::close() throw() {
	if(ssl) {
		SSL_free(ssl);
		ssl = 0;
	}
	Socket::close();
}

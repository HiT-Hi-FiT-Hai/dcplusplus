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

SSLSocketFactory::SSLSocketFactory() : clientContext(SSL_CTX_new(TLSv1_client_method())), serverContext(SSL_CTX_new(TLSv1_server_method())), dh(0) {
	SSL_CTX_set_verify(serverContext, SSL_VERIFY_NONE, 0);
	SSL_CTX_use_certificate_file(serverContext, (Util::getAppPath() + "\\..\\yassl\\certs\\server-cert.pem").c_str(), SSL_FILETYPE_PEM);
	SSL_CTX_use_PrivateKey_file(serverContext, (Util::getAppPath() + "\\..\\yassl\\certs\\server-key.pem").c_str(), SSL_FILETYPE_PEM);
	SSL_CTX_set_verify(clientContext, SSL_VERIFY_NONE, 0);

	static unsigned char dh512_p[] =
	{
		0xDA,0x58,0x3C,0x16,0xD9,0x85,0x22,0x89,0xD0,0xE4,0xAF,0x75,
		0x6F,0x4C,0xCA,0x92,0xDD,0x4B,0xE5,0x33,0xB8,0x04,0xFB,0x0F,
		0xED,0x94,0xEF,0x9C,0x8A,0x44,0x03,0xED,0x57,0x46,0x50,0xD3,
		0x69,0x99,0xDB,0x29,0xD7,0x76,0x27,0x6B,0xA2,0xD3,0xD4,0x12,
		0xE2,0x18,0xF4,0xDD,0x1E,0x08,0x4C,0xF6,0xD8,0x00,0x3E,0x7C,
		0x47,0x74,0xE8,0x33,
	};

	static unsigned char dh512_g[] =
	{
		0x02,
	};

	dh = DH_new();
	if(dh) {
		dh->p = BN_bin2bn(dh512_p, sizeof(dh512_p), 0);
		dh->g = BN_bin2bn(dh512_g, sizeof(dh512_g), 0);

		if (!dh->p || !dh->g) {
			DH_free(dh);
			dh = 0;
		} else {
			SSL_CTX_set_tmp_dh(serverContext, dh);
		}
	}
}

SSLSocketFactory::~SSLSocketFactory() {
	if(serverContext)
		SSL_CTX_free(serverContext);
	if(clientContext)
		SSL_CTX_free(clientContext);
	if(dh)
		DH_free(dh);
}

SSLSocket* SSLSocketFactory::getClientSocket() throw(SocketException) {
	return new SSLSocket(clientContext);
}
SSLSocket* SSLSocketFactory::getServerSocket() throw(SocketException) {
	return new SSLSocket(serverContext);
}

SSLSocket::SSLSocket(SSL_CTX* context) throw(SocketException) : ctx(context), ssl(0) {
}

void SSLSocket::connect(const string& aIp, short aPort) throw(SocketException) {
	Socket::connect(aIp, aPort);
	
	if(ssl)
		SSL_free(ssl);

	ssl = SSL_new(ctx);
	if(!ssl)
		checkSSL(-1);

	checkSSL(SSL_set_fd(ssl, sock));
	checkSSL(SSL_connect(ssl));
}

void SSLSocket::accept(const Socket& listeningSocket) throw(SocketException) {
	Socket::accept(listeningSocket);

	checkSSL(SSL_set_fd(ssl, sock));
	checkSSL(SSL_accept(ssl));
}

int SSLSocket::read(void* aBuffer, int aBufLen) throw(SocketException) {
	int len = checkSSL(SSL_read(ssl, aBuffer, aBufLen));

	if(len > 0) {
		stats.totalDown += len;
		dcdebug("In(s): %.*s\n", len, (char*)aBuffer);
	}
	return len;

}

int SSLSocket::write(const void* aBuffer, int aLen) throw(SocketException) {
	int ret = checkSSL(SSL_write(ssl, aBuffer, aLen));
	if(ret > 0) {
		stats.totalUp += ret;
		dcdebug("Out(s): %.*s\n", ret, (char*)aBuffer);
	}
	return ret;
}

int SSLSocket::checkSSL(int ret) throw(SocketException) {
	if(ret <= 0) {
		int err = SSL_get_error(ssl, ret);
		switch(SSL_get_error(ssl, ret)) {
			case SSL_ERROR_WANT_READ:	// Fallthrough
			case SSL_ERROR_WANT_WRITE:
				return -1;
			default:
				{
					if(ssl) {
						SSL_free(ssl);
						ssl = 0;
					}
					// @todo replace 80 with MAX_ERROR_SZ or whatever's appropriate for yaSSL in some nice way...
					char errbuf[80];
					throw SocketException(string("SSL Error: ") + ERR_error_string(err, errbuf) + " (" + Util::toString(ret) + ", " + Util::toString(err) + ")"); // @todo Translate
				}
		}
	}
	return ret;
}

void SSLSocket::shutdown() throw() {
	if(ssl)
		SSL_shutdown(ssl);

	Socket::shutdown();
}

void SSLSocket::close() throw() {
	if(ssl) {
		SSL_free(ssl);
		ssl = 0;
	}
	Socket::close();
}

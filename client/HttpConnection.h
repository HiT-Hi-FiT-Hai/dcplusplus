/* 
 * Copyright (C) 2001-2003 Jacek Sieka, j_s@telia.com
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

#if !defined(AFX_HTTPCONNECTION_H__47AE2649_8D90_4C38_B048_69B3C26B3954__INCLUDED_)
#define AFX_HTTPCONNECTION_H__47AE2649_8D90_4C38_B048_69B3C26B3954__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BufferedSocket.h"

class HttpConnection;

class HttpConnectionListener {
public:
	template<int I>	struct X { enum { TYPE = I };  };

	typedef X<0> Data;
	typedef X<1> Failed;
	typedef X<2> Complete;
	typedef X<3> Redirected;
	typedef X<4> TypeNormal;
	typedef X<5> TypeBZ2;

	virtual void on(Data, HttpConnection*, u_int8_t*, size_t) throw() { }
	virtual void on(Failed, HttpConnection*, const string&) throw() { }
	virtual void on(Complete, HttpConnection*, const string&) throw() { }
	virtual void on(Redirected, HttpConnection*, const string&) throw() { }
	virtual void on(TypeNormal, HttpConnection*) throw() { }
	virtual void on(TypeBZ2, HttpConnection*) throw() { }
};

class HttpConnection : BufferedSocketListener, public Speaker<HttpConnectionListener>
{
public:
	void downloadFile(const string& aUrl);
	HttpConnection() : ok(false), port(80), size(-1), moved302(false), socket(NULL) { };
	virtual ~HttpConnection() { 
		if(socket) {
			socket->removeListener(this); 
			BufferedSocket::putSocket(socket);
		}
	}

private:

	HttpConnection(const HttpConnection&);
	HttpConnection& operator=(const HttpConnection&);

	string currentUrl;
	string file;
	string server;
	bool ok;
	short port;
	int64_t size;
	bool moved302;

	BufferedSocket* socket;

	// BufferedSocketListener
	virtual void on(Connected) throw();
	virtual void on(Line, const string&) throw();
	virtual void on(Data, u_int8_t*, size_t) throw();
	virtual void on(ModeChange) throw();
	virtual void on(Failed, const string&) throw();

	void onConnected(); 
	void onLine(const string& aLine);
	
};

#endif // !defined(AFX_HTTPCONNECTION_H__47AE2649_8D90_4C38_B048_69B3C26B3954__INCLUDED_)

/**
 * @file
 * $Id: HttpConnection.h,v 1.19 2004/04/24 09:40:58 arnetheduck Exp $
 */


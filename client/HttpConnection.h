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

#if !defined(AFX_HTTPCONNECTION_H__47AE2649_8D90_4C38_B048_69B3C26B3954__INCLUDED_)
#define AFX_HTTPCONNECTION_H__47AE2649_8D90_4C38_B048_69B3C26B3954__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BufferedSocket.h"
#include "Util.h"

class HttpConnection;

class HttpConnectionListener {
public:
	typedef HttpConnectionListener* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;
	enum Types {
		DATA,
		FAILED,
		COMPLETE
	};

	virtual void onAction(Types, HttpConnection*) { };	
	virtual void onAction(Types, HttpConnection*, const string&) { };
	virtual void onAction(Types, HttpConnection*, const u_int8_t*, int) { };
};

class HttpConnection : BufferedSocketListener, public Speaker<HttpConnectionListener>
{
public:
	void downloadFile(const string& aUrl);
	HttpConnection() : ok(false), port(80), size(-1), socket(NULL) { };
	virtual ~HttpConnection() { 
		if(socket) {
			socket->removeListener(this); 
			BufferedSocket::putSocket(socket);
		}
	}

private:

	HttpConnection(const HttpConnection&) { dcassert(0); };

	string file;
	string server;
	bool ok;
	short port;
	int64_t size;
	
	BufferedSocket* socket;

	// BufferedSocketListener
	virtual void onAction(BufferedSocketListener::Types type) {
		switch(type) {
		case BufferedSocketListener::CONNECTED:
			onConnected(); break;
		}
	}
	virtual void onAction(BufferedSocketListener::Types type, const string& aLine) {
		switch(type) {
		case BufferedSocketListener::LINE:
			onLine(aLine); break;
		case BufferedSocketListener::FAILED:
			socket->removeListener(this);
			fire(HttpConnectionListener::FAILED, this, aLine); break;
		}
	}
	virtual void onAction(BufferedSocketListener::Types type, int /*mode*/) {
		switch(type) {
		case BufferedSocketListener::MODE_CHANGE:
			socket->removeListener(this);
			socket->disconnect();
			fire(HttpConnectionListener::COMPLETE, this); 
			break;
		default:
			dcassert(0);
		}
	}
	virtual void onAction(BufferedSocketListener::Types type, const u_int8_t* aBuf, int aLen) {
		switch(type) {
		case BufferedSocketListener::DATA:
			fire(HttpConnectionListener::DATA, this, aBuf, aLen); break;
		default:
			dcassert(0);
		}
	}

	void onConnected(); 
	void onLine(const string& aLine);

};

#endif // !defined(AFX_HTTPCONNECTION_H__47AE2649_8D90_4C38_B048_69B3C26B3954__INCLUDED_)

/**
 * @file HttpConnection.h
 * $Id: HttpConnection.h,v 1.10 2002/05/03 18:53:02 arnetheduck Exp $
 */


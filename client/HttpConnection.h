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
	virtual void onAction(Types, HttpConnection*, const BYTE*, int) { };
};

class HttpConnection : BufferedSocketListener, public Speaker<HttpConnectionListener>
{
public:
	void downloadFile(const string& aUrl);
	HttpConnection() : ok(false), port(false), size(-1) { 	socket.addListener(this); };
	virtual ~HttpConnection() { socket.removeListener(this); };

private:

	HttpConnection(const HttpConnection&) { dcassert(0); };

	string file;
	string server;
	short port;
	LONGLONG size;
	
	bool ok;
	BufferedSocket socket;

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
			fire(HttpConnectionListener::FAILED, this, aLine); break;
		}
	}
	virtual void onAction(BufferedSocketListener::Types type, int /*mode*/) {
		switch(type) {
		case BufferedSocketListener::MODE_CHANGE:
			fire(HttpConnectionListener::COMPLETE, this); socket.disconnect(); break;
		default:
			dcassert(0);
		}
	}
	virtual void onAction(BufferedSocketListener::Types type, const BYTE* aBuf, int aLen) {
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
 * $Id: HttpConnection.h,v 1.7 2002/02/09 18:13:51 arnetheduck Exp $
 * @if LOG
 * $Log: HttpConnection.h,v $
 * Revision 1.7  2002/02/09 18:13:51  arnetheduck
 * Fixed level 4 warnings and started using new stl
 *
 * Revision 1.6  2002/01/20 22:54:46  arnetheduck
 * Bugfixes to 0.131 mainly...
 *
 * Revision 1.5  2002/01/11 14:52:57  arnetheduck
 * Huge changes in the listener code, replaced most of it with templates,
 * also moved the getinstance stuff for the managers to a template
 *
 * Revision 1.4  2002/01/05 10:13:39  arnetheduck
 * Automatic version detection and some other updates
 *
 * Revision 1.3  2001/12/15 17:01:06  arnetheduck
 * Passive mode searching as well as some searching code added
 *
 * Revision 1.2  2001/12/07 20:03:07  arnetheduck
 * More work done towards application stability
 *
 * Revision 1.1.1.1  2001/11/21 17:33:20  arnetheduck
 * Inital release
 *
 * @endif
 */


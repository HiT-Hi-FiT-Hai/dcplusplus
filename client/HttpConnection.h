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

	virtual void onHttpData(HttpConnection* aConn, const BYTE* aBuf, int aLen) { };
	virtual void onHttpError(HttpConnection* aConn, const string& aError) { };
	virtual void onHttpComplete(HttpConnection* aConn) { };	
};

class HttpConnection : BufferedSocketListener, public Speaker<HttpConnectionListener>
{
public:
	void downloadFile(const string& aUrl);
	HttpConnection() : ok(false), port(false), size(-1) { 	socket.addListener(this); };
	virtual ~HttpConnection() { socket.removeListener(this); };

private:

	virtual void onConnected();
	virtual void onLine(const string& aLine);
	virtual void onData(const BYTE* aBuf, int aLen) { fireData(aBuf, aLen); }
	virtual void onError(const string& aReason) { fireError(aReason); }
	virtual void onModeChange(int newMode) { fireComplete(); socket.disconnect(); }
	
	string file;
	string server;
	short port;
	LONGLONG size;
	
	bool ok;
	BufferedSocket socket;
	
	void fireData(const BYTE* aBuf, int aLen) {
		listenerCS.enter();
		dcdebug("HttpConnection::fireData %d\n", aLen);
		HttpConnectionListener::List tmp = listeners;
		listenerCS.leave();
		for(HttpConnectionListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onHttpData(this, aBuf, aLen);
		}
	}
	void fireError(const string& aError) {
		listenerCS.enter();
		dcdebug("HttpConnection::fireError %s\n", aError.c_str());
		HttpConnectionListener::List tmp = listeners;
		listenerCS.leave();
		for(HttpConnectionListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onHttpError(this, aError);
		}
	}
	void fireComplete() {
		listenerCS.enter();
		dcdebug("HttpConnection::fireComplete\n");
		HttpConnectionListener::List tmp = listeners;
		listenerCS.leave();
		for(HttpConnectionListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onHttpComplete(this);
		}
	}
};

#endif // !defined(AFX_HTTPCONNECTION_H__47AE2649_8D90_4C38_B048_69B3C26B3954__INCLUDED_)

/**
 * @file HttpConnection.h
 * $Id: HttpConnection.h,v 1.4 2002/01/05 10:13:39 arnetheduck Exp $
 * @if LOG
 * $Log: HttpConnection.h,v $
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


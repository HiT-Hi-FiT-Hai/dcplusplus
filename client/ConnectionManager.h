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

#if !defined(AFX_ConnectionManager_H__675A2F66_AFE6_4A15_8386_6B6FD579D5FF__INCLUDED_)
#define AFX_ConnectionManager_H__675A2F66_AFE6_4A15_8386_6B6FD579D5FF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ServerSocket.h"
#include "UserConnection.h"
#include "User.h"
#include "CriticalSection.h"

class ConnectionManager : public UserConnectionListener, ServerSocketListener
{
public:
	static void newInstance() {
		if(instance)
			delete instance;

		instance = new ConnectionManager();
	}
	static ConnectionManager* getInstance() {
		dcassert(instance);
		return instance;
	}
	static void deleteInstance() {
		if(instance)
			delete instance;
		instance = NULL;
	}

	int getDownloadConnection(User* aUser);
	void putDownloadConnection(UserConnection* aSource) {
		aSource->disconnect();
		downloaderCS.enter();
		int j = downloaders.size();
		for(UserConnection::NickIter i = downloaders.begin(); i != downloaders.end(); ++i) {
			// Can't search by user, he/she might have disconnected...
			if(i->second == aSource) {
				downloaders.erase(i);
				break;
			}
		}
		downloaderCS.leave();		
		// Pool it for later usage...
		pool.push_back(aSource);
	}
	void putUploadConnection(UserConnection* aSource) {
		aSource->disconnect();
		uploaderCS.enter();
		for(UserConnection::NickIter i = uploaders.begin(); i != uploaders.end(); ++i) {
			// Can't search by user, he/she might have disconnected...
			if(i->second == aSource) {
				uploaders.erase(i);
				break;
			}
		}
		uploaderCS.leave();
		pool.push_back(aSource);
	}
	void connect(const string& aServer, short aPort);

	/**
	 * Set this ConnectionManager to listen at a different port.
	 * @todo Multiple ports and something clever when the port is taken.
	 */
	void setPort(short aPort) {
		try {
			socket.disconnect();
			socket.waitForConnections(aPort);
		} catch(SocketException e) {
			// Doh!...
		}
	}

	virtual void onIncomingConnection();
	virtual void onMyNick(UserConnection* aSource, const string& aNick);
	virtual void onLock(UserConnection* aSource, const string& aLock, const string& aPk);
	virtual void onConnected(UserConnection* aSource);
	virtual void onKey(UserConnection* aSource, const string& aKey);
private:
	User::NickMap pendingDown;
	CriticalSection downloaderCS;
	UserConnection::NickMap downloaders;
	CriticalSection uploaderCS;
	UserConnection::NickMap uploaders;
	
	UserConnection::List pool;

	/**
	 * Returns an unused connection, either from the pool or a brand new fresh one.
	 */
	UserConnection* getConnection() {
		UserConnection* uc;
		if(pool.size() == 0) {
			uc = new UserConnection();
			uc->addListener(this);
		} else {
			uc = pool.back();
			pool.pop_back();
		}
		return uc;
	}
	/**
	 * Put a connection back into the pool.
	 */
	void putConnection(UserConnection* aConn) {
		aConn->disconnect();
		pool.push_back(aConn);
	}
	static ConnectionManager* instance;
	/**
	 * @todo Something clever when the port is busy.
	 */
	ConnectionManager() {
		try {
			socket.addListener(this);
			socket.waitForConnections(atoi(Settings::getPort().c_str()));
		} catch(SocketException e) {
			// Doh!
		}
	};

	~ConnectionManager() {
		socket.removeListener(this);
		// Time to empty the pool...
		for(UserConnection::Iter i = pool.begin(); i != pool.end(); ++i) {
			delete *i;
		}
	}

	ServerSocket socket;
};

#endif // !defined(AFX_ConnectionManager_H__675A2F66_AFE6_4A15_8386_6B6FD579D5FF__INCLUDED_)

/**
 * @file IncomingManger.h
 * $Id: ConnectionManager.h,v 1.6 2001/12/04 21:50:34 arnetheduck Exp $
 * @if LOG
 * $Log: ConnectionManager.h,v $
 * Revision 1.6  2001/12/04 21:50:34  arnetheduck
 * Work done towards application stability...still a lot to do though...
 * a bit more and it's time for a new release.
 *
 * Revision 1.5  2001/12/03 20:52:19  arnetheduck
 * Blah! Finally, the listings are working...one line of code missing (of course),
 * but more than 2 hours of search...hate that kind of bugs...=(...some other
 * things spiffed up as well...
 *
 * Revision 1.4  2001/12/02 11:16:46  arnetheduck
 * Optimised hub listing, removed a few bugs and leaks, and added a few small
 * things...downloads are now working, time to start writing the sharing
 * code...
 *
 * Revision 1.3  2001/12/01 17:15:03  arnetheduck
 * Added a crappy version of huffman encoding, and some other minor changes...
 *
 * Revision 1.2  2001/11/29 19:10:54  arnetheduck
 * Refactored down/uploading and some other things completely.
 * Also added download indicators and download resuming, along
 * with some other stuff.
 *
 * Revision 1.1  2001/11/27 20:29:37  arnetheduck
 * Renamed from ConnectionManager
 *
 * Revision 1.2  2001/11/26 23:40:36  arnetheduck
 * Downloads!! Now downloads are possible, although the implementation is
 * likely to change in the future...more UI work (splitters...) and some bug
 * fixes. Only user file listings are downloadable, but at least it's something...
 *
 * Revision 1.1  2001/11/25 22:06:25  arnetheduck
 * Finally downloading is working! There are now a few quirks and bugs to be fixed
 * but what the heck....!
 *
 * @endif
 */

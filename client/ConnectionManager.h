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

	int getDownloadConnection(User* aUser, bool aRemind = false);
	
	void putDownloadConnection(UserConnection* aSource) {
		cs.enter();
		for(UserConnection::NickIter i = downloaders.begin(); i != downloaders.end(); ++i) {
			// Can't search by user, he/she might have disconnected...
			if(i->second == aSource) {
				downloaders.erase(i);
				break;
			}
		}
		cs.leave();		
		// Pool it for later usage...
		putConnection(aSource);
	}
	void putUploadConnection(UserConnection* aSource) {
		cs.enter();
		for(UserConnection::NickIter i = uploaders.begin(); i != uploaders.end(); ++i) {
			// Can't search by user, he/she might have disconnected...
			if(i->second == aSource) {
				uploaders.erase(i);
				break;
			}
		}
		cs.leave();
		putConnection(aSource);
	}
	void connect(const string& aServer, short aPort);

	/**
	 * Set this ConnectionManager to listen at a different port.
	 */
	void setPort(short aPort) throw(SocketException) {
		socket.disconnect();
		socket.waitForConnections(aPort);
	}

private:

	/** Main critical section for the connection manager */
	CriticalSection cs;

	User::NickMap pendingDown;
	UserConnection::NickMap downloaders;
	UserConnection::NickMap uploaders;
	UserConnection::List pool;

	virtual void onIncomingConnection();
	virtual void onMyNick(UserConnection* aSource, const string& aNick);
	virtual void onLock(UserConnection* aSource, const string& aLock, const string& aPk);
	virtual void onConnected(UserConnection* aSource);
	virtual void onKey(UserConnection* aSource, const string& aKey);
	virtual void onError(UserConnection* aSource, const string& aError);
	
	/**
	 * Returns an unused connection, either from the pool or a brand new fresh one.
	 */
	UserConnection* getConnection() {
		UserConnection* uc;
		cs.enter();
		// We want to keep a few connections in the pool, so that they have time to stop...
		if(pool.size() <= 5) {
			uc = new UserConnection();
			uc->addListener(this);
		} else {
			uc = pool.front();
			pool.pop_front();
			dcdebug("ConnectionManager::getConnection %p, %d listeners\n", uc, uc->listeners.size());
			uc->addListener(this);
		}
		cs.leave();
		return uc;
	}
	/**
	 * Put a connection back into the pool. Note; As it seems, this function might be called multiple times
	 * for the same connection, once when the reader dies, and once when some other thread tries to recover...
	 */
	void putConnection(UserConnection* aConn) {
		aConn->reset();
		cs.enter();
		bool found = false;
		for(UserConnection::Iter i = pool.begin(); i != pool.end(); ++i) {
			if(*i == aConn) {
				found = true;
				break;
			}
		}
		if(!found)
			pool.push_back(aConn);
		cs.leave();
	}
	static ConnectionManager* instance;

	ConnectionManager() {
		socket.addListener(this);
	};

	~ConnectionManager() {
		socket.removeListener(this);
		// Time to empty the pool...
		for(UserConnection::Iter i = pool.begin(); i != pool.end(); ++i) {
			dcdebug("Deleting connection %p\n", *i);
			delete *i;
		}
	}

	ServerSocket socket;
};

#endif // !defined(AFX_ConnectionManager_H__675A2F66_AFE6_4A15_8386_6B6FD579D5FF__INCLUDED_)

/**
 * @file IncomingManger.h
 * $Id: ConnectionManager.h,v 1.12 2001/12/13 19:21:57 arnetheduck Exp $
 * @if LOG
 * $Log: ConnectionManager.h,v $
 * Revision 1.12  2001/12/13 19:21:57  arnetheduck
 * A lot of work done almost everywhere, mainly towards a friendlier UI
 * and less bugs...time to release 0.06...
 *
 * Revision 1.11  2001/12/11 01:10:29  arnetheduck
 * More bugfixes...I really have to change the bufferedsocket so that it only
 * uses one thread...or maybe even multiple sockets/thread...
 *
 * Revision 1.10  2001/12/10 10:48:40  arnetheduck
 * Ahh, finally found one bug that's been annoying me for days...=) the connections
 * in the pool were not reset correctly before being put back for later use...
 *
 * Revision 1.9  2001/12/08 20:59:26  arnetheduck
 * Fixing bugs...
 *
 * Revision 1.8  2001/12/08 14:25:49  arnetheduck
 * More bugs removed...did my first search as well...
 *
 * Revision 1.7  2001/12/07 20:03:05  arnetheduck
 * More work done towards application stability
 *
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

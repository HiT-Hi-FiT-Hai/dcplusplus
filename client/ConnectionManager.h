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

#if !defined(AFX_ConnectionManager_H__675A2F66_AFE6_4A15_8386_6B6FD579D5FF__INCLUDED_)
#define AFX_ConnectionManager_H__675A2F66_AFE6_4A15_8386_6B6FD579D5FF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ServerSocket.h"
#include "UserConnection.h"
#include "User.h"
#include "CriticalSection.h"
#include "TimerManager.h"
#include "Util.h"

class ConnectionQueueItem {
public:
	typedef ConnectionQueueItem* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;
	typedef map<Ptr, DWORD> TimeMap;
	typedef TimeMap::iterator TimeIter;
	typedef map<UserConnection*, Ptr> QueueMap;
	typedef QueueMap::iterator QueueIter;

	enum Status {
		CONNECTING,
		WAITING
	};

	ConnectionQueueItem(const User::Ptr& aUser) : connection(NULL), user(aUser), status(CONNECTING) { };
	
	void setUser(const User::Ptr& aUser) { user = aUser; };
	User::Ptr& getUser() { return user; };
	
	GETSET(Status, status, Status);
	GETSET(UserConnection*, connection, Connection);
private:
	User::Ptr user;
};

class ConnectionManagerListener {
public:
	typedef ConnectionManagerListener* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;
	
	enum Types {
		ADDED,
		CONNECTED,
		REMOVED,
		FAILED,
		STATUS_CHANGED
	};
	virtual void onAction(Types type, ConnectionQueueItem* aItem) { };
	virtual void onAction(Types type, ConnectionQueueItem* aItem, const string& aLine) { };
};


class ConnectionManager : public Speaker<ConnectionManagerListener>, public UserConnectionListener, ServerSocketListener, TimerManagerListener, public Singleton<ConnectionManager>
{
public:

	ConnectionQueueItem* getQueueItem(UserConnection* aConn) {
		if(connections.find(aConn) != connections.end()) {
			return connections[aConn];
		}
		dcassert(0);
		return NULL;
	}
	
	void removeConnection(ConnectionQueueItem* aCqi);
	
	void getDownloadConnection(const User::Ptr& aUser);
	void putDownloadConnection(UserConnection* aSource, bool reuse = false);
	void retryDownload(ConnectionQueueItem* aCqi);
	void putUploadConnection(UserConnection* aSource) {
		putConnection(aSource);
	}
	void connect(const string& aServer, short aPort, const string& aNick);
	void updateUser(UserConnection* aConn);
	
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

	ConnectionQueueItem::TimeMap pendingDown;
	UserConnection::List pendingDelete;
	ConnectionQueueItem::QueueMap downPool;
	ConnectionQueueItem::QueueMap connections;
	ConnectionQueueItem::List pendingAdd;

	ServerSocket socket;

	friend class Singleton<ConnectionManager>;
	ConnectionManager() {
		TimerManager::getInstance()->addListener(this);
		socket.addListener(this);
	};
	
	~ConnectionManager() {
		TimerManager::getInstance()->removeListener(this);
		socket.removeListener(this);
		// Time to empty the pool...
		for(UserConnection::Iter i = pendingDelete.begin(); i != pendingDelete.end(); ++i) {
			dcdebug("Deleting connection %p\n", *i);
			delete *i;
		}
	}
	// ServerSocketListener
	virtual void onAction(ServerSocketListener::Types type) {
		switch(type) {
		case ServerSocketListener::INCOMING_CONNECTION:
			onIncomingConnection();
		}
	}
	void onIncomingConnection() throw();

	// UserConnectionListener
	virtual void onAction(UserConnectionListener::Types type, UserConnection* conn) {
		switch(type) {
		case UserConnectionListener::CONNECTED:
			onConnected(conn); break;
		}
	}
	virtual void onAction(UserConnectionListener::Types type, UserConnection* conn, const string& line) {
		switch(type) {
		case UserConnectionListener::MY_NICK:
			onMyNick(conn, line); break;
		case UserConnectionListener::KEY:
			onKey(conn, line); break;
		case UserConnectionListener::FAILED:
			onFailed(conn, line); break;
		}
	}
	virtual void onAction(UserConnectionListener::Types type, UserConnection* conn, const string& line1, const string& line2) {
		switch(type) {
		case UserConnectionListener::LOCK:
			onLock(conn, line1, line2); break;
		case UserConnectionListener::DIRECTION:
			onDirection(conn, line1, line2); break;
		}
	}
	void onMyNick(UserConnection* aSource, const string& aNick) throw();
	void onLock(UserConnection* aSource, const string& aLock, const string& aPk) throw();
	void onDirection(UserConnection* aSource, const string& dir, const string& num) throw();
	void onConnected(UserConnection* aSource) throw();
	void onKey(UserConnection* aSource, const string& aKey) throw();
	void onFailed(UserConnection* aSource, const string& aError) throw();
	
	// TimerManagerListener
	virtual void onAction(TimerManagerListener::Types type, DWORD aTick);
	
	/**
	 * Returns an connection, either from the pool or a brand new fresh one.
	 */
	UserConnection* getConnection() {

		UserConnection* uc = new UserConnection();
		uc->addListener(this);
		return uc;
	}
	/**
	 * Put a connection back into the pool. There are a few reasons for pooling user connections, the
	 * most notable one is that it is possible that the connection's own socket thread will call this function,
	 * and therefore, it cannot be removed at once. And since we have to store the connections on in a pool
	 * we might as well reuse them instead of always deleting and creating new ones...
	 */
	void putConnection(UserConnection* aConn) {
		TimerManager::getInstance()->removeListener(aConn);
		aConn->removeListeners();
		ConnectionQueueItem* cqi = NULL;
		{
			Lock l(cs);
			
			ConnectionQueueItem::QueueIter i = connections.find(aConn);
			if(i != connections.end()) {
				cqi = i->second;
				connections.erase(i);
			}
			if(find(pendingDelete.begin(), pendingDelete.end(), aConn) == pendingDelete.end()) {
				pendingDelete.push_back(aConn);
			} else {
				dcdebug("ConnectionManager::putConnection %p put back twice\n", aConn);
			}
		}
		if(cqi) {
			fire(ConnectionManagerListener::REMOVED, cqi);
			delete cqi;
		}
	}
};

#endif // !defined(AFX_ConnectionManager_H__675A2F66_AFE6_4A15_8386_6B6FD579D5FF__INCLUDED_)

/**
 * @file IncomingManger.h
 * $Id: ConnectionManager.h,v 1.24 2002/02/01 02:00:25 arnetheduck Exp $
 * @if LOG
 * $Log: ConnectionManager.h,v $
 * Revision 1.24  2002/02/01 02:00:25  arnetheduck
 * A lot of work done on the new queue manager, hopefully this should reduce
 * the number of crashes...
 *
 * Revision 1.23  2002/01/20 22:54:46  arnetheduck
 * Bugfixes to 0.131 mainly...
 *
 * Revision 1.22  2002/01/17 23:35:59  arnetheduck
 * Reworked threading once more, now it actually seems stable. Also made
 * sure that noone tries to access client objects that have been deleted
 * as well as some other minor updates
 *
 * Revision 1.21  2002/01/14 22:19:43  arnetheduck
 * Commiting minor bugfixes
 *
 * Revision 1.20  2002/01/13 22:50:47  arnetheduck
 * Time for 0.12, added favorites, a bunch of new icons and lot's of other stuff
 *
 * Revision 1.19  2002/01/11 14:52:56  arnetheduck
 * Huge changes in the listener code, replaced most of it with templates,
 * also moved the getinstance stuff for the managers to a template
 *
 * Revision 1.18  2002/01/08 00:24:10  arnetheduck
 * Last bugs fixed before 0.11
 *
 * Revision 1.17  2002/01/05 10:13:39  arnetheduck
 * Automatic version detection and some other updates
 *
 * Revision 1.16  2002/01/02 16:12:32  arnetheduck
 * Added code for multiple download sources
 *
 * Revision 1.15  2001/12/21 20:21:17  arnetheduck
 * Private messaging added, and a lot of other updates as well...
 *
 * Revision 1.14  2001/12/16 19:47:48  arnetheduck
 * Reworked downloading and user handling some, and changed some small UI things
 *
 * Revision 1.13  2001/12/15 17:01:06  arnetheduck
 * Passive mode searching as well as some searching code added
 *
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

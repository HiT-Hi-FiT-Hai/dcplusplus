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
#include "Thread.h"

class ConnectionQueueItem {
public:
	typedef ConnectionQueueItem* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;
	typedef map<Ptr, u_int32_t> TimeMap;
	typedef TimeMap::iterator TimeIter;
	typedef map<UserConnection*, Ptr> QueueMap;
	typedef QueueMap::iterator QueueIter;

	enum Status {
		CONNECTING,
		IDLE,
		WAITING
	};

	ConnectionQueueItem(const User::Ptr& aUser) : status(CONNECTING), connection(NULL), user(aUser)  { };
	
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
	virtual void onAction(Types, ConnectionQueueItem*) { };
	virtual void onAction(Types, ConnectionQueueItem*, const string&) { };
};

class ConnectionManager : public Speaker<ConnectionManagerListener>, public UserConnectionListener, ServerSocketListener, TimerManagerListener, public Singleton<ConnectionManager>
{
public:

	ConnectionQueueItem* getQueueItem(UserConnection* aConn) {
		dcassert(connections.find(aConn) != connections.end());
		return connections[aConn];
	}

	void connect(const string& aServer, short aPort, const string& aNick);
	void abortDownloadConnection(const User::Ptr& aUser);
	void getDownloadConnection(const User::Ptr& aUser);
	void putDownloadConnection(UserConnection* aSource, bool reuse = false);
	void putUploadConnection(UserConnection* aSource) { putConnection(aSource); };
	
	void removeConnection(ConnectionQueueItem* aCqi);
	
	void disconnectAll() {
		socket.removeListener(this);
		socket.disconnect();
		Lock l(cs);
		for(UserConnection::Iter j = userConnections.begin(); j != userConnections.end(); ++j) {
			(*j)->disconnect();
		}
	}		
	
	/**
	 * Set this ConnectionManager to listen at a different port.
	 */
	void setPort(short aPort) throw(SocketException) {
		socket.waitForConnections(aPort);
	}

private:

	/** Main critical section for the connection manager */
	CriticalSection cs;

	ConnectionQueueItem::TimeMap pendingDown;
	ConnectionQueueItem::List downPool;
	ConnectionQueueItem::QueueMap connections;
	ConnectionQueueItem::List pendingAdd;
	UserConnection::List pendingDelete;
	UserConnection::List userConnections;

	ServerSocket socket;
	StringList features;

	friend class Singleton<ConnectionManager>;
	ConnectionManager() {
		TimerManager::getInstance()->addListener(this);
		socket.addListener(this);

		features.push_back("BZList");
	};
	
	virtual ~ConnectionManager() {
		TimerManager::getInstance()->removeListener(this);

		socket.removeListener(this);
		socket.disconnect();
		
		{
			Lock l(cs);
			for(UserConnection::Iter j = userConnections.begin(); j != userConnections.end(); ++j) {
				(*j)->disconnect();
			}
		}
		
		while(true) {
			{
				Lock l(cs);
				if(userConnections.empty())
					break;
			}
			Thread::sleep(100);			
		}
	}
	
	/**
	 * Returns a connection, either from the pool or a brand new fresh one.
	 */
	UserConnection* getConnection() {
		UserConnection* uc = new UserConnection();
		uc->addListener(this);
		{
			Lock l(cs);
			userConnections.push_back(uc);
		}
		return uc;
	}
	void putConnection(UserConnection* aConn);

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
		case UserConnectionListener::C_LOCK:
			onLock(conn, line1, line2); break;
		case UserConnectionListener::DIRECTION:
			onDirection(conn, line1, line2); break;
		}
	}
	// UserConnectionListener
	virtual void onAction(UserConnectionListener::Types type, UserConnection* conn, const StringList& feat) {
		switch(type) {
		case UserConnectionListener::SUPPORTS:
			{
				for(StringList::const_iterator i = feat.begin(); i != feat.end(); ++i) {
					if(*i == "BZList")
						conn->setFlag(UserConnection::FLAG_SUPPORTS_BZLIST);
				}
			}
			break;
		}
	}
	
	void onMyNick(UserConnection* aSource, const string& aNick) throw();
	void onLock(UserConnection* aSource, const string& aLock, const string& aPk) throw();
	void onDirection(UserConnection* aSource, const string& dir, const string& num) throw();
	void onConnected(UserConnection* aSource) throw();
	void onKey(UserConnection* aSource, const string& aKey) throw();
	void onFailed(UserConnection* aSource, const string& aError) throw();
	
	// TimerManagerListener
	virtual void onAction(TimerManagerListener::Types type, u_int32_t aTick) {
		switch(type) {
		case TimerManagerListener::SECOND: onTimerSecond(aTick); break;
		case TimerManagerListener::MINUTE: onTimerMinute(aTick); break;
		}
	}
	
	void onTimerSecond(u_int32_t aTick);
	void onTimerMinute(u_int32_t aTick);

};

#endif // !defined(AFX_ConnectionManager_H__675A2F66_AFE6_4A15_8386_6B6FD579D5FF__INCLUDED_)

/**
 * @file IncomingManger.h
 * $Id: ConnectionManager.h,v 1.37 2002/05/12 21:54:07 arnetheduck Exp $
 */

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

#if !defined(AFX_ConnectionManager_H__675A2F66_AFE6_4A15_8386_6B6FD579D5FF__INCLUDED_)
#define AFX_ConnectionManager_H__675A2F66_AFE6_4A15_8386_6B6FD579D5FF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TimerManager.h"

#include "ServerSocket.h"
#include "UserConnection.h"
#include "User.h"
#include "CriticalSection.h"
#include "Singleton.h"
#include "Util.h"

#include "ConnectionManagerListener.h"

class ConnectionQueueItem {
public:
	typedef ConnectionQueueItem* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;
	
	enum State {
		CONNECTING,					// In pendingDown, recently sent request to connect
		WAITING,					// In pendingDown, waiting to send request to connect
		NO_DOWNLOAD_SLOTS,			// In pendingDown, but not needed right now
		IDLE,						// In the download pool
	};

	ConnectionQueueItem(const User::Ptr& aUser) : state(WAITING), connection(NULL), lastAttempt(0), user(aUser) { };
	
	User::Ptr& getUser() { return user; };
	
	GETSET(State, state, State);
	GETSET(UserConnection*, connection, Connection);
	GETSET(u_int32_t, lastAttempt, LastAttempt);
private:
	User::Ptr user;
};
// Comparing with a user...
inline bool operator==(ConnectionQueueItem::Ptr ptr, const User::Ptr& aUser) { return ptr->getUser() == aUser; };

class ConnectionManager : public Speaker<ConnectionManagerListener>, 
	public UserConnectionListener, ServerSocketListener, TimerManagerListener, 
	public Singleton<ConnectionManager>
{
public:
	void nmdcConnect(const string& aServer, short aPort, const string& aNick);
	void adcConnect(const string& aServer, short aPort, const string& aToken);
	void getDownloadConnection(const User::Ptr& aUser);
	void putDownloadConnection(UserConnection* aSource, bool reuse = false, bool ntd = false);
	void putUploadConnection(UserConnection* aSource, bool ntd);
	
	void removeConnection(const User::Ptr& aUser, int isDownload);
	void shutdown();	
	/**
	 * Set this ConnectionManager to listen at a different port.
	 */
	void setPort(short aPort) throw(SocketException) {
		port = aPort;
		socket.waitForConnections(aPort);
	}
	void disconnect() throw() {
		socket.disconnect();
	}
	unsigned short getPort() {
		return port;
	}

	// Ugly trick to use windows messages...
	ServerSocket& getServerSocket() {
		return socket;
	}

private:
	CriticalSection cs;
	short port;

	/** Pending connections, i e users we're trying to connect to */
	ConnectionQueueItem::List pendingDown;
	/** Download connection pool, pool of active connections to be used for downloading */
	ConnectionQueueItem::List downPool;
	/** Connections that are currently being used by the Up/DownloadManager */
	ConnectionQueueItem::List active;

	User::List pendingAdd;
	UserConnection::List pendingDelete;
	/** All active connections */
	UserConnection::List userConnections;

	ServerSocket socket;
	StringList features;
	StringList adcFeatures;

	u_int32_t floodCounter;

	bool shuttingDown;

	friend class Singleton<ConnectionManager>;
	ConnectionManager();

	virtual ~ConnectionManager() { shutdown(); };
	
	UserConnection* getConnection(bool aNmdc) throw(SocketException) {
		UserConnection* uc = new UserConnection();
		uc->addListener(this);
		{
			Lock l(cs);
			userConnections.push_back(uc);
		}
		if(aNmdc)
			uc->setFlag(UserConnection::FLAG_NMDC);
		return uc;
	}
	void putConnection(UserConnection* aConn);

	// ServerSocketListener
	virtual void on(ServerSocketListener::IncomingConnection) throw();

	// UserConnectionListener
	virtual void on(Connected, UserConnection*) throw();
	virtual void on(Failed, UserConnection*, const string&) throw();
	virtual void on(CLock, UserConnection*, const string&, const string&) throw();
	virtual void on(Key, UserConnection*, const string&) throw();
	virtual void on(Direction, UserConnection*, const string&, const string&) throw();
	virtual void on(MyNick, UserConnection*, const string&) throw();
	virtual void on(Supports, UserConnection*, const StringList&) throw();

	virtual void on(AdcCommand::SUP, UserConnection*, const AdcCommand&) throw();
	virtual void on(AdcCommand::INF, UserConnection*, const AdcCommand&) throw();
	virtual void on(AdcCommand::NTD, UserConnection*, const AdcCommand&) throw();
	virtual void on(AdcCommand::STA, UserConnection*, const AdcCommand&) throw();

	// TimerManagerListener
	virtual void on(TimerManagerListener::Second, u_int32_t aTick) throw();	
	virtual void on(TimerManagerListener::Minute, u_int32_t aTick) throw();	

};

#endif // !defined(AFX_ConnectionManager_H__675A2F66_AFE6_4A15_8386_6B6FD579D5FF__INCLUDED_)

/**
 * @file
 * $Id: ConnectionManager.h,v 1.64 2005/01/05 19:30:24 arnetheduck Exp $
 */

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
		downloaders.erase(downloaders.find(aSource->user->getNick()));
		//delete aSource;
	}

	void connect(const string& aServer, short aPort);

	void setPort(short aPort) {
		socket.disconnect();
		socket.waitForConnections(aPort);
	}

	virtual void onIncomingConnection();
	virtual void onMyNick(UserConnection* aSource, const string& aNick);
	virtual void onLock(UserConnection* aSource, const string& aLock, const string& aPk);
	virtual void onConnected(UserConnection* aSource);
	virtual void onKey(UserConnection* aSource, const string& aKey);
private:
	User::NickMap pendingDown;
	User::NickMap pendingUp;
	UserConnection::NickMap downloaders;
	UserConnection::NickMap uploaders;
	
	void addConnection(UserConnection::Ptr conn) {
		conn->addListener(this);
		connections.push_back(conn);
	}
	void removeConnection(UserConnection::Ptr aConn) {
		for(UserConnection::Iter i = connections.begin(); i != connections.end(); ++i) {
			if(*i == aConn) {
				aConn->removeListener(this);
				connections.erase(i);
				return;
			}
		}
	}
	void removeConnections() {
		for(UserConnection::Iter i = connections.begin(); i != connections.end(); ++i) {
			(*i)->removeListener(this);
			connections.erase(i);
		}
	}

	UserConnection::List connections;
	static ConnectionManager* instance;
	ConnectionManager() {
		socket.addListener(this);
		socket.waitForConnections(atoi(Settings::getPort().c_str()));
	};

	~ConnectionManager() {
		socket.removeListener(this);
		removeConnections();
	}

	ServerSocket socket;
};

#endif // !defined(AFX_ConnectionManager_H__675A2F66_AFE6_4A15_8386_6B6FD579D5FF__INCLUDED_)

/**
 * @file IncomingManger.h
 * $Id: ConnectionManager.h,v 1.2 2001/11/29 19:10:54 arnetheduck Exp $
 * @if LOG
 * $Log: ConnectionManager.h,v $
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

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

#if !defined(AFX_USERCONNECTION_H__52BFD1A0_9924_4C07_BAFA_FB9682884841__INCLUDED_)
#define AFX_USERCONNECTION_H__52BFD1A0_9924_4C07_BAFA_FB9682884841__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BufferedSocket.h"
#include "CriticalSection.h"

class UserConnection;

class UserConnectionListener {
public:
	typedef UserConnectionListener* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;

	virtual void onConnecting(UserConnection* aSource, const string& aServer) { };
	virtual void onConnected(UserConnection* aSource) { };
	virtual void onConnectionError(UserConnection* aSource, const string& aReason) { };
	virtual void onData(UserConnection* aSource, BYTE* aBuf, int aLen) { };
	virtual void onError(UserConnection* aSource, const string& aError) { };
	virtual void onMyNick(UserConnection* aSource, const string& aNick) { };
	virtual void onLock(UserConnection* aSource, const string& aLock, const string& aPk) { };
	virtual void onKey(UserConnection* aSource, const string& aKey) { };
	virtual void onDirection(UserConnection* aSource, const string& aDirection, const string& aNumber) { };
	virtual void onGet(UserConnection* aSource, const string& aFile, LONGLONG aResumeFrom) { };
	virtual void onFileLength(UserConnection* aSource, const string& aFileLength) { };
	virtual void onSend(UserConnection* aSource) { };
	virtual void onDisconnected(UserConnection* aSource) { };
	virtual void onGetListLen(UserConnection* aSource) { };
	virtual void onModeChange(UserConnection* aSource, int aNewMode) { };
};

class ServerSocket;

class UserConnection : public BufferedSocketListener
{
public:
	typedef UserConnection* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;

	enum {	MODE_COMMAND = BufferedSocket::MODE_LINE,
		MODE_DATA = BufferedSocket::MODE_DATA
	};

	void addListener(UserConnectionListener::Ptr aListener) {
		listenerCS.enter();
		listeners.push_back(aListener);
		listenerCS.leave();
	}
	
	void removeListener(UserConnectionListener::Ptr aListener) {
		listenerCS.enter();
		for(UserConnectionListener::Iter i = listeners.begin(); i != listeners.end(); ++i) {
			if(*i == aListener) {
				listeners.erase(i);
				break;
			}
		}
		listenerCS.leave();
	}
	
	void removeListeners() {
		listenerCS.enter();
		listeners.clear();
		listenerCS.leave();
	}
	
	virtual void onConnected() { fireConnected(); };
	virtual void onLine(const string& aLine);
	virtual void onError(const string& aError) { fireConnectionError(aError); };
	virtual void onModeChange(int aNewMode) { fireModeChange(aNewMode); };
	virtual void onData(BYTE *aBuf, int aLen) { fireData(aBuf, aLen); };

	void myNick(const string& aNick) { send("$MyNick " + aNick + "|"); sentNick = true; }
	void lock(const string& aLock, const string& aPk) { send ("$Lock " + aLock + " Pk=" + aPk + "|"); sentLock = true; }
	void key(const string& aKey) { send("$Key " + aKey + "|"); }
	void direction(const string& aDirection, const string& aNumber) { send("$Direction " + aDirection + " " + aNumber + "|"); }
	
	void get(const string& aFile, LONGLONG aResume) { 
		char buf[24];
		_i64toa(aResume+1, buf, 10);
		send("$Get " + aFile + "$" + string(buf)+"|");
	}
	
	void fileLength(const string& aLength) { send("$FileLength " + aLength + "|"); }
	void startSend() { send("$Send|"); }
	void error(const string& aError) { send("$Error " + aError + "|"); };
	void listLen(const string& aLength) { send("$ListLen " + aLength + "|"); };
	void maxedOut() { send("$MaxedOut|"); };
	boolean hasSentNick() { return sentNick; };
	boolean hasSentLock() { return sentLock; };
	UserConnection() : socket('|'), sentNick(false), sentLock(false) { 
		socket.addListener(this);
	};
	
	void setDataMode(LONGLONG aBytes) { socket.setDataMode(aBytes); }

	const string& getNick() { return nick; };

	void connect(const string& aServer, short aPort = 412);
	void accept(const ServerSocket& aSocket);
	void waitForConnection(short aPort = 412);
	void disconnect() {
		socket.removeListener(this);
		socket.disconnect();
		fireDisconnected();
	}
	virtual ~UserConnection() {
	};

private:
	boolean sentNick;
	boolean sentLock;
	string server;
	short port;
	BufferedSocket socket;
	string nick;
	
	UserConnectionListener::List listeners;
	CriticalSection listenerCS;

	void send(const string& aString) {
		socket.write(aString);
	}
	void fireConnected() {
		listenerCS.enter();
		dcdebug("UserConnection::fireConnected\n");
		UserConnectionListener::List tmp = listeners;
		for(UserConnectionListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onConnected(this);
		}
		listenerCS.leave();
	}
	void fireConnecting(const string& aServer) {
		listenerCS.enter();
		dcdebug("UserConnection::fireConnecting %s\n", aServer.c_str());
		UserConnectionListener::List tmp = listeners;
		for(UserConnectionListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onConnecting(this, aServer);
		}
		listenerCS.leave();
	}
	void fireConnectionError(const string& aError) {
		listenerCS.enter();
		dcdebug("UserConnection::fireConnectionError %s\n", aError.c_str());
		UserConnectionListener::List tmp = listeners;
		for(UserConnectionListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onConnectionError(this, aError);
		}
		listenerCS.leave();
	}
	void fireData(BYTE* aData, int aLen) {
		listenerCS.enter();
		dcdebug("UserConnection::fireData %d\n", aLen);
		UserConnectionListener::List tmp = listeners;
		for(UserConnectionListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onData(this, aData, aLen);
		}
		listenerCS.leave();
	}
	void fireDirection(const string& aDirection, const string& aNumber) {
		listenerCS.enter();
		dcdebug("UserConnection::fireDirection %s\n", aDirection.c_str());
		UserConnectionListener::List tmp = listeners;
		for(UserConnectionListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onDirection(this, aDirection, aNumber);
		}
		listenerCS.leave();
	}
	void fireDisconnected() {
		listenerCS.enter();
		dcdebug("UserConnection::fireDisconnected\n");
		UserConnectionListener::List tmp = listeners;
		for(UserConnectionListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onDisconnected(this);
		}
		listenerCS.leave();
	}
	void fireError(const string& aError) {
		listenerCS.enter();
		dcdebug("UserConnection::fireError %s\n", aError.c_str());
		UserConnectionListener::List tmp = listeners;
		for(UserConnectionListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onError(this, aError);
		}
		listenerCS.leave();
	}
	void fireFileLength(const string& aLength) {
		listenerCS.enter();
		dcdebug("UserConnection::fireFileLength %s\n", aLength.c_str());
		UserConnectionListener::List tmp = listeners;
		for(UserConnectionListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onFileLength(this, aLength);
		}
		listenerCS.leave();
	}
	void fireGet(const string& aFile, LONGLONG aResume) {
		listenerCS.enter();
		dcdebug("UserConnection::fireGet %s\n", aFile.c_str());
		UserConnectionListener::List tmp = listeners;
		for(UserConnectionListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onGet(this, aFile, aResume);
		}
		listenerCS.leave();
	}
	void fireKey(const string& aKey) {
		listenerCS.enter();
		dcdebug("UserConnection::fireKey\n");
		UserConnectionListener::List tmp = listeners;
		for(UserConnectionListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onKey(this, aKey);
		}
		listenerCS.leave();
	}
	void fireGetListLen() {
		listenerCS.enter();
		dcdebug("fireGetListLen\n");
		UserConnectionListener::List tmp = listeners;
		for(UserConnectionListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onGetListLen(this);
		}
		listenerCS.leave();
	}
	void fireLock(const string& aLock, const string& aPk) {
		listenerCS.enter();
		dcdebug("UserConnection::fireLock\n");
		UserConnectionListener::List tmp = listeners;
		for(UserConnectionListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onLock(this, aLock, aPk);
		}
		listenerCS.leave();
	}
	void fireModeChange(int aNewMode) {
		listenerCS.enter();
		dcdebug("UserConnection::fireModeChange\n");
		UserConnectionListener::List tmp = listeners;
		for(UserConnectionListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onModeChange(this, aNewMode);
		}
		listenerCS.leave();
	}
	void fireMyNick(const string& aNick) {
		listenerCS.enter();
		dcdebug("UserConnection::fireMyNick %s\n", aNick.c_str());
		UserConnectionListener::List tmp = listeners;
		for(UserConnectionListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onMyNick(this, aNick);
		}
		listenerCS.leave();
	}
	void fireSend() {
		listenerCS.enter();
		dcdebug("UserConnection::fireSend\n");
		UserConnectionListener::List tmp = listeners;
		for(UserConnectionListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onSend(this);
		}
		listenerCS.leave();
	}
};

#endif // !defined(AFX_USERCONNECTION_H__52BFD1A0_9924_4C07_BAFA_FB9682884841__INCLUDED_)

/**
 * @file UserConnection.h
 * $Id: UserConnection.h,v 1.1 2001/11/25 22:06:25 arnetheduck Exp $
 * @if LOG
 * $Log: UserConnection.h,v $
 * Revision 1.1  2001/11/25 22:06:25  arnetheduck
 * Finally downloading is working! There are now a few quirks and bugs to be fixed
 * but what the heck....!
 *
 * @endif
 */

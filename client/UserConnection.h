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

#if !defined(AFX_USERCONNECTION_H__52BFD1A0_9924_4C07_BAFA_FB9682884841__INCLUDED_)
#define AFX_USERCONNECTION_H__52BFD1A0_9924_4C07_BAFA_FB9682884841__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BufferedSocket.h"
#include "CriticalSection.h"
#include "File.h"
#include "TimerManager.h"
#include "User.h"
#include "Util.h"

class UserConnection;

class UserConnectionListener {
public:
	typedef UserConnectionListener* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;
	enum Types {
		BYTES_SENT,
		CONNECTING,
		CONNECTED,
		DATA,
		FAILED,
		LOCK,
		KEY,
		DIRECTION,
		GET,
		FILE_LENGTH,
		SEND,
		GET_LIST_LENGTH,
		MAXED_OUT,
		MODE_CHANGE,
		MY_NICK,
		TRANSMIT_DONE
	};

	virtual void onAction(Types, UserConnection*) { };
	virtual void onAction(Types, UserConnection*, u_int32_t) { };
	virtual void onAction(Types, UserConnection*, const string&) { };
	virtual void onAction(Types, UserConnection*, const u_int8_t*, int) { };
	virtual void onAction(Types, UserConnection*, const string&, const string&) { };
	virtual void onAction(Types, UserConnection*, const string&, int64_t) { };
	virtual void onAction(Types, UserConnection*, int) { };
};

class ConnectionQueueItem;

class Transfer {
public:
	int64_t getPos() { return pos; };
	void setPos(int64_t aPos) { pos = aPos; };
	void setPos(int64_t aPos, bool aUpdate) { 
		pos = aPos;
		if(aUpdate) {
			file->setPos(aPos);
		}
	};
	void addPos(int64_t aPos) { pos += aPos; last+=aPos; total+=aPos; };
	
	int64_t getTotal() { return total; };
	void resetTotal() { total = 0; };
	
	int64_t getSize() { return size; };
	void setSize(int64_t aSize) { size = aSize; };
	void setSize(const string& aSize) { setSize(Util::toInt64(aSize)); };

	int64_t getAverageSpeed() {
		int64_t dif = (int64_t)(GET_TICK() - getStart());
		return (dif > 0) ? (getTotal() * (int64_t)1000 / dif) : 0;
	}

	int64_t getSecondsLeft() {
		int64_t avg = getAverageSpeed();
		return (avg > 0) ? ((getSize() - getPos()) / avg) : 0;
	}

	Transfer() : file(NULL), userConnection(NULL), start(0), last(0), total(0), 
		pos(-1), size(-1) { };
	virtual ~Transfer() { dcassert(userConnection == NULL); dcassert(file == NULL); };

	GETSET(File*, file, File);
	GETSET(UserConnection*, userConnection, UserConnection);
	GETSET(u_int32_t, start, Start);
private:
	int64_t last;
	int64_t total;
	int64_t pos;
	int64_t size;

};

class ServerSocket;
class Upload;
class Download;

class UserConnection : public Speaker<UserConnectionListener>, private BufferedSocketListener, public Flags
{
public:
	friend class ConnectionManager;
	
	typedef UserConnection* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;
	
	enum Modes {	
		MODE_COMMAND = BufferedSocket::MODE_LINE,
		MODE_DATA = BufferedSocket::MODE_DATA
	};

	enum Flags {
		FLAG_UPLOAD = 0x01,
		FLAG_DOWNLOAD = FLAG_UPLOAD << 1,
		FLAG_INCOMING = FLAG_DOWNLOAD << 1,
		FLAG_HASSLOT = FLAG_INCOMING << 1,
		FLAG_HASEXTRASLOT = FLAG_HASSLOT << 1,
		FLAG_INVALIDKEY = FLAG_HASEXTRASLOT << 1
	};
	
	enum States {
		// ConnectionManager
		STATE_UNCONNECTED,
		STATE_CONNECT,
		STATE_NICK,
		STATE_LOCK,
		STATE_KEY,
		// UploadManager
		STATE_GET,
		STATE_SEND,
		STATE_DONE,
		// DownloadManager
		STATE_FILELENGTH
	};

	void myNick(const string& aNick) { send("$MyNick " + aNick + "|"); }
	void lock(const string& aLock, const string& aPk) { send ("$Lock " + aLock + " Pk=" + aPk + "|"); }
	void key(const string& aKey) { send("$Key " + aKey + "|"); }
	void direction(const string& aDirection, const string& aNumber) { send("$Direction " + aDirection + " " + aNumber + "|"); }
	void get(const string& aFile, int64_t aResume) { send("$Get " + aFile + "$" + Util::toString(aResume + 1) + "|"); };
	void fileLength(const string& aLength) { send("$FileLength " + aLength + "|"); }
	void startSend() { send("$Send|"); }
	void error(const string& aError) { send("$Error " + aError + "|"); };
	void listLen(const string& aLength) { send("$ListLen " + aLength + "|"); };
	void maxedOut() { send("$MaxedOut|"); };

	void setDataMode(int64_t aBytes) { dcassert(socket); socket->setDataMode(aBytes); }

	void UserConnection::connect(const string& aServer, short aPort) throw(SocketException) { 
		if(socket == NULL) {
			socket = BufferedSocket::getSocket('|');
			socket->addListener(this);
		}
		
		socket->connect(aServer, aPort);
	}
	
	void UserConnection::accept(const ServerSocket& aServer) throw(SocketException) {
		if(socket != NULL) {
			socket->removeListener(this);
			BufferedSocket::putSocket(socket);
			socket = NULL;
		}
		socket = BufferedSocket::accept(aServer, '|', this);
	}
	
	void disconnect() { dcassert(socket); socket->disconnect(); };
	void transmitFile(File* f) { socket->transmitFile(f); };

	const string& getDirectionString() {
		dcassert(isSet(FLAG_UPLOAD) ^ isSet(FLAG_DOWNLOAD));
		return isSet(FLAG_UPLOAD) ? UPLOAD : DOWNLOAD;
	}

	User::Ptr& getUser() { return user; };

	GETSET(ConnectionQueueItem*, cqi, CQI);
	GETSET(States, state, State);
	GETSET(u_int32_t, lastActivity, LastActivity);
	GETSETREF(string, nick, Nick);
	
	Download* getDownload() { dcassert(isSet(FLAG_DOWNLOAD)); return download; };
	void setDownload(Download* d) { dcassert(isSet(FLAG_DOWNLOAD)); download = d; };
	Upload* getUpload() { dcassert(isSet(FLAG_UPLOAD)); return upload; };
	void setUpload(Upload* u) { dcassert(isSet(FLAG_UPLOAD)); upload = u; };

private:
	BufferedSocket* socket;
	User::Ptr user;
	
	static const string UPLOAD, DOWNLOAD;
	
	union {
		Download* download;
		Upload* upload;
	};

	void setUser(const User::Ptr& aUser) {
		user = aUser;
	};

	void onLine(const string& aLine) throw();
	
	void send(const string& aString) {
		lastActivity = GET_TICK();
		socket->write(aString);
	}
	
	// We only want ConnectionManager to create this...
	UserConnection() : state(STATE_UNCONNECTED), lastActivity(0), socket(NULL), download(NULL) { };
	UserConnection(const UserConnection&) { dcassert(0); };

	virtual ~UserConnection() {
		if(socket != NULL) {
			socket->removeListener(this);
			BufferedSocket::putSocket(socket);
		}
		removeListeners();
	};

	// BufferedSocketListener
	virtual void onAction(BufferedSocketListener::Types type) {
		lastActivity = GET_TICK();
		switch(type) {
		case BufferedSocketListener::CONNECTED:
			fire(UserConnectionListener::CONNECTED, this);
			break;
		case BufferedSocketListener::TRANSMIT_DONE:
			fire(UserConnectionListener::TRANSMIT_DONE, this); break;
		}
	}
	virtual void onAction(BufferedSocketListener::Types type, u_int32_t bytes) {
		lastActivity = GET_TICK();
		switch(type) {
		case BufferedSocketListener::BYTES_SENT:
			fire(UserConnectionListener::BYTES_SENT, this, bytes); break;
		default:
			dcassert(0);
		}
	}
	virtual void onAction(BufferedSocketListener::Types type, const string& aLine) {
		lastActivity = GET_TICK();
		switch(type) {
		case BufferedSocketListener::LINE:
			onLine(aLine); break;
		case BufferedSocketListener::FAILED:
			setState(STATE_UNCONNECTED);
			fire(UserConnectionListener::FAILED, this, aLine); break;
		default:
			dcassert(0);
		}
	}
	virtual void onAction(BufferedSocketListener::Types type, int mode) {
		lastActivity = GET_TICK();
		switch(type) {
		case BufferedSocketListener::MODE_CHANGE:
			fire(UserConnectionListener::MODE_CHANGE, this, mode); break;
		default:
			dcassert(0);
		}
	}
	virtual void onAction(BufferedSocketListener::Types type, const u_int8_t* buf, int len) {
		lastActivity = GET_TICK();
		switch(type) {
		case BufferedSocketListener::DATA:
			fire(UserConnectionListener::DATA, this, buf, len); break;
		default:
			dcassert(0);
		}
	}

};

#endif // !defined(AFX_USERCONNECTION_H__52BFD1A0_9924_4C07_BAFA_FB9682884841__INCLUDED_)

/**
 * @file UserConnection.h
 * $Id: UserConnection.h,v 1.41 2002/04/13 12:57:23 arnetheduck Exp $
 */

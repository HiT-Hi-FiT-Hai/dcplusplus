/* 
 * Copyright (C) 2001-2003 Jacek Sieka, j_s@telia.com
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
		CONNECTED,
		DATA,
		FAILED,
		C_LOCK,
		KEY,
		DIRECTION,
		GET,
		GET_ZBLOCK,
		SENDING,
		FILE_LENGTH,
		SEND,
		GET_LIST_LENGTH,
		MAXED_OUT,
		MODE_CHANGE,
		MY_NICK,
		TRANSMIT_DONE,
		SUPPORTS,
		FILE_NOT_AVAILABLE
	};

	virtual void onAction(Types, UserConnection*) throw() { };							// GET_LIST_LENGTH, SEND, MAXED_OUT, CONNECTED, TRANSMIT_DONE
	virtual void onAction(Types, UserConnection*, u_int32_t) throw() { };				// BYTES_SENT
	virtual void onAction(Types, UserConnection*, const string&) throw() { };			// MY_NICK, FAILED, FILE_LENGTH, KEY, SUPPORTS
	virtual void onAction(Types, UserConnection*, const u_int8_t*, int) throw() { };	// DATA
	virtual void onAction(Types, UserConnection*, const string&, const string&) throw() { };	// DIRECTION, LOCK
	virtual void onAction(Types, UserConnection*, const string&, int64_t) throw() { };	// GET
	virtual void onAction(Types, UserConnection*, const string&, int64_t, int64_t) throw() { };	// GET_BZ_BLOCK
	virtual void onAction(Types, UserConnection*, int) throw() { };						// MODE_CHANGE
	virtual void onAction(Types, UserConnection*, const StringList&) throw() { };		// SUPPORTS
};

class ConnectionQueueItem;

class Transfer {
public:
	Transfer() : file(NULL), userConnection(NULL), start(0), lastTick(GET_TICK()), runningAverage(0), 
		last(0), total(0), pos(-1), size(-1) { };
	virtual ~Transfer() { dcassert(userConnection == NULL); dcassert(file == NULL); };
	
	int64_t getPos() { return pos; };
	void setPos(int64_t aPos) { pos = aPos; };
	void setPos(int64_t aPos, bool aUpdate) { 
		pos = aPos;
		if(aUpdate) {
			file->setPos(aPos);
		}
	};

	void addPos(int64_t aPos) {
		pos += aPos; last+=aPos; total+=aPos; 
	};
	
	void updateRunningAverage() {
		u_int32_t tick = GET_TICK();
		if(tick > lastTick) {
			int64_t diff = (int64_t)(tick - lastTick);
			if(runningAverage == 0) {
				runningAverage = last * 1000 / diff;
			} else if( (tick - getStart()) < 30000) {
				runningAverage = getAverageSpeed();
			} else {
				runningAverage = ( (runningAverage * ((int64_t)30000 - diff) ) + (last*diff)) / (int64_t)30000;
			}
			last = 0;
		}
		lastTick = tick;
	}

	int64_t getTotal() { return total; };
	void resetTotal() { total = 0; };
	
	int64_t getSize() { return size; };
	void setSize(int64_t aSize) { size = aSize; };
	void setSize(const string& aSize) { setSize(Util::toInt64(aSize)); };

	int64_t getAverageSpeed() {
		int64_t diff = (int64_t)(GET_TICK() - getStart());
		return (diff > 0) ? (getTotal() * (int64_t)1000 / diff) : 0;
	}

	int64_t getSecondsLeft() {
		updateRunningAverage();
		int64_t avg = getRunningAverage();
		return (avg > 0) ? ((getSize() - getPos()) / avg) : 0;
	}

	GETSET(File*, file, File);
	GETSET(UserConnection*, userConnection, UserConnection);
	GETSET(u_int32_t, start, Start);
	GETSET(u_int32_t, lastTick, LastTick);
	GETSET(int64_t, runningAverage, RunningAverage);
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
		FLAG_INVALIDKEY = FLAG_HASEXTRASLOT << 1,
		FLAG_SUPPORTS_BZLIST = FLAG_INVALIDKEY << 1,
		FLAG_SUPPORTS_GETZBLOCK = FLAG_SUPPORTS_BZLIST << 1
	};
	
	enum States {
		// ConnectionManager
		STATE_UNCONNECTED,
		STATE_CONNECT,
		STATE_NICK,
		STATE_LOCK,
		STATE_DIRECTION,
		STATE_KEY,
		// UploadManager
		STATE_GET,
		STATE_SEND,
		STATE_DONE,
		// DownloadManager
		STATE_FILELENGTH
	};

	int getNumber() { return (((u_int32_t)this)>>2) & 0x7fff; };

	void myNick(const string& aNick) { send("$MyNick " + aNick + '|'); }
	void lock(const string& aLock, const string& aPk) { send ("$Lock " + aLock + " Pk=" + aPk + '|'); }
	void key(const string& aKey) { send("$Key " + aKey + '|'); }
	void direction(const string& aDirection, int aNumber) { send("$Direction " + aDirection + " " + Util::toString(aNumber) + '|'); }
	void get(const string& aFile, int64_t aResume) { send("$Get " + aFile + "$" + Util::toString(aResume + 1) + '|'); };
	void getZBlock(const string& aFile, int64_t aResume, int64_t aBytes) { send("$GetZBlock " + Util::toString(aResume) + ' ' + Util::toString(aBytes) + ' ' + aFile + '|'); };
	void fileLength(const string& aLength) { send("$FileLength " + aLength + '|'); }
	void startSend() { send("$Send|"); }
	void sending() { send("$Sending|"); };
	void error(const string& aError) { send("$Error " + aError + '|'); };
	void listLen(const string& aLength) { send("$ListLen " + aLength + '|'); };
	void maxedOut() { send("$MaxedOut|"); };
	void supports(const StringList& feat) { 
		string x;
		for(StringList::const_iterator i = feat.begin(); i != feat.end(); ++i) {
			x+= *i + ' ';
		}
		send("$Supports " + x + '|');
	}
	void setDataMode(int64_t aBytes = -1) { dcassert(socket); socket->setDataMode(aBytes); }
	void setLineMode() { dcassert(socket); socket->setLineMode(); };

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
	
	void disconnect() { if(socket) socket->disconnect(); };
	void transmitFile(File* f, int64_t size, bool comp = false) { 
		socket->transmitFile(f, size, comp); 
	};

	const string& getDirectionString() {
		dcassert(isSet(FLAG_UPLOAD) ^ isSet(FLAG_DOWNLOAD));
		return isSet(FLAG_UPLOAD) ? UPLOAD : DOWNLOAD;
	}

	User::Ptr& getUser() { return user; };

	Download* getDownload() { dcassert(isSet(FLAG_DOWNLOAD)); return download; };
	void setDownload(Download* d) { dcassert(isSet(FLAG_DOWNLOAD)); download = d; };
	Upload* getUpload() { dcassert(isSet(FLAG_UPLOAD)); return upload; };
	void setUpload(Upload* u) { dcassert(isSet(FLAG_UPLOAD)); upload = u; };

	GETSET(ConnectionQueueItem*, cqi, CQI);
	GETSET(States, state, State);
	GETSET(u_int32_t, lastActivity, LastActivity);
	GETSETREF(string, nick, Nick);
	
private:
	BufferedSocket* socket;
	User::Ptr user;
	
	static const string UPLOAD, DOWNLOAD;
	
	union {
		Download* download;
		Upload* upload;
	};

	// We only want ConnectionManager to create this...
	UserConnection() : cqi(NULL), state(STATE_UNCONNECTED), lastActivity(0), socket(NULL), download(NULL) { };

	virtual ~UserConnection() {
		if(socket != NULL) {
			socket->removeListener(this);
			BufferedSocket::putSocket(socket);
		}
		removeListeners();
	};
	friend struct DeleteFunction<UserConnection*>;

	void setUser(const User::Ptr& aUser) {
		user = aUser;
	};

	void onLine(const string& aLine) throw();
	
	void send(const string& aString) {
		lastActivity = GET_TICK();
		socket->write(aString);
	}

	// BufferedSocketListener
	virtual void onAction(BufferedSocketListener::Types type) throw();
	virtual void onAction(BufferedSocketListener::Types type, u_int32_t bytes) throw();
	virtual void onAction(BufferedSocketListener::Types type, const string& aLine) throw();
	virtual void onAction(BufferedSocketListener::Types type, int mode) throw();
	virtual void onAction(BufferedSocketListener::Types type, const u_int8_t* buf, int len) throw();

};

#endif // !defined(AFX_USERCONNECTION_H__52BFD1A0_9924_4C07_BAFA_FB9682884841__INCLUDED_)

/**
 * @file UserConnection.h
 * $Id: UserConnection.h,v 1.52 2003/03/13 13:31:40 arnetheduck Exp $
 */


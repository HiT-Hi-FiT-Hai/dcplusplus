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

#include "TimerManager.h"

#include "BufferedSocket.h"
#include "CriticalSection.h"
#include "File.h"
#include "User.h"
#include "AdcCommand.h"

class UserConnection;

class UserConnectionListener {
public:
	template<int I>	struct X { enum { TYPE = I };  };

	typedef X<0> BytesSent;
	typedef X<1> Connected;
	typedef X<2> Data;
	typedef X<3> Failed;
	typedef X<4> CLock;
	typedef X<5> Key;
	typedef X<6> Direction;
	typedef X<7> Get;
	typedef X<8> GetBlock;
	typedef X<9> GetZBlock;
	typedef X<10> Sending;
	typedef X<11> FileLength;
	typedef X<12> Send;
	typedef X<13> GetListLength;
	typedef X<14> MaxedOut;
	typedef X<15> ModeChange;
	typedef X<16> MyNick;
	typedef X<17> TransmitDone;
	typedef X<18> Supports;
	typedef X<19> FileNotAvailable;
	typedef X<20> ADCGet;
	typedef X<21> ADCSnd;
	typedef X<22> ADCSta;

	virtual void on(BytesSent, UserConnection*, size_t, size_t) throw() { }
	virtual void on(Connected, UserConnection*) throw() { }
	virtual void on(Data, UserConnection*, const u_int8_t*, size_t) throw() { }
	virtual void on(Failed, UserConnection*, const string&) throw() { }
	virtual void on(CLock, UserConnection*, const string&, const string&) throw() { }
	virtual void on(Key, UserConnection*, const string&) throw() { }
	virtual void on(Direction, UserConnection*, const string&, const string&) throw() { }
	virtual void on(Get, UserConnection*, const string&, int64_t) throw() { }
	virtual void on(GetBlock, UserConnection*, const string&, int64_t, int64_t) throw() { }
	virtual void on(GetZBlock, UserConnection*, const string&, int64_t, int64_t) throw() { }
	virtual void on(Sending, UserConnection*, int64_t) throw() { }
	virtual void on(FileLength, UserConnection*, int64_t) throw() { }
	virtual void on(Send, UserConnection*) throw() { }
	virtual void on(GetListLength, UserConnection*) throw() { }
	virtual void on(MaxedOut, UserConnection*) throw() { }
	virtual void on(ModeChange, UserConnection*) throw() { }
	virtual void on(MyNick, UserConnection*, const string&) throw() { }
	virtual void on(TransmitDone, UserConnection*) throw() { }
	virtual void on(Supports, UserConnection*, const StringList&) throw() { }
	virtual void on(FileNotAvailable, UserConnection*) throw() { }
	virtual void on(Command::GET, UserConnection*, const Command&) throw() { }
	virtual void on(Command::SND, UserConnection*, const Command&) throw() { }
	virtual void on(Command::STA, UserConnection*, const Command&) throw() { }
};

class ConnectionQueueItem;

class Transfer {
public:
	Transfer() : userConnection(NULL), start(0), lastTick(GET_TICK()), runningAverage(0), 
		last(0), total(0), actual(0), pos(-1), size(-1) { };
	virtual ~Transfer() { dcassert(userConnection == NULL); };
	
	int64_t getPos() { return pos; };
	void setPos(int64_t aPos) { pos = aPos; };

	void addPos(int64_t aBytes) { pos += aBytes; total+=aBytes; };
	void addActual(int64_t aBytes) { actual += aBytes; };

	enum { AVG_PERIOD = 30000 };
	void updateRunningAverage();

	int64_t getTotal() { return total; };
	int64_t getActual() { return actual; };
	
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

	GETSET(UserConnection*, userConnection, UserConnection);
	GETSET(u_int32_t, start, Start);
	GETSET(u_int32_t, lastTick, LastTick);
	GETSET(int64_t, runningAverage, RunningAverage);
private:
	/** Bytes on last avg update */
	int64_t last;
	/** Total effective bytes transfered this session */
	int64_t total;
	/** Total actual bytes transfered this session (compression?) */
	int64_t actual;
	/** Write position in file */
	int64_t pos;
	/** Target size of this transfer */
	int64_t size;

};

class ServerSocket;
class Upload;
class Download;

class UserConnection : public Speaker<UserConnectionListener>, 
	private BufferedSocketListener, public Flags, private CommandHandler<UserConnection>
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
		FLAG_SUPPORTS_GETZBLOCK = FLAG_SUPPORTS_BZLIST << 1,
		FLAG_SUPPORTS_MINISLOTS = FLAG_SUPPORTS_GETZBLOCK << 1,
		FLAG_SUPPORTS_GETTESTZBLOCK = FLAG_SUPPORTS_MINISLOTS << 1,
		FLAG_SUPPORTS_XML_BZLIST = FLAG_SUPPORTS_GETTESTZBLOCK << 1,
		FLAG_SUPPORTS_XGET = FLAG_SUPPORTS_XML_BZLIST << 1,
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
	void getZBlock(const string& aFile, int64_t aResume, int64_t aBytes, bool utf8) { send((isSet(FLAG_SUPPORTS_GETZBLOCK) ? (utf8 ? "$UGetZBlock " : "$GetZBlock ") : "$GetTestZBlock ") + Util::toString(aResume) + ' ' + Util::toString(aBytes) + ' ' + aFile + '|'); };
	void getBlock(const string& aFile, int64_t aResume, int64_t aBytes, bool utf8) { send((utf8 ? "$UGetBlock " : "$GetBlock ") + Util::toString(aResume) + ' ' + Util::toString(aBytes) + ' ' + aFile + '|'); }
	void fileLength(const string& aLength) { send("$FileLength " + aLength + '|'); }
	void startSend() { send("$Send|"); }
	void sending(int64_t bytes) { send(bytes == -1 ? "$Sending|" : "$Sending " + Util::toString(bytes) + "|"); };
	void error(const string& aError) { send("$Error " + aError + '|'); };
	void listLen(const string& aLength) { send("$ListLen " + aLength + '|'); };
	void maxedOut() { send("$MaxedOut|"); };
	void fileNotAvail() { send("$Error File Not Available|"); }

	void send(const Command& c) {
		send(c.toString(true));
	}
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
		socket->connect(aServer, aPort);
	}
	
	void UserConnection::accept(const ServerSocket& aServer) throw(SocketException) {
		socket->accept(aServer);
	}
	
	void disconnect() { if(socket) socket->disconnect(); };
	void transmitFile(InputStream* f) { 
		socket->transmitFile(f); 
	};

	const string& getDirectionString() {
		dcassert(isSet(FLAG_UPLOAD) ^ isSet(FLAG_DOWNLOAD));
		return isSet(FLAG_UPLOAD) ? UPLOAD : DOWNLOAD;
	}

	User::Ptr& getUser() { return user; };

	string getRemoteIp() const { return socket->getRemoteIp(); }
	Download* getDownload() { dcassert(isSet(FLAG_DOWNLOAD)); return download; };
	void setDownload(Download* d) { dcassert(isSet(FLAG_DOWNLOAD)); download = d; };
	Upload* getUpload() { dcassert(isSet(FLAG_UPLOAD)); return upload; };
	void setUpload(Upload* u) { dcassert(isSet(FLAG_UPLOAD)); upload = u; };

	void handle(Command::GET t, const Command& c) {
		fire(t, this, c);
	}
	void handle(Command::SND t, const Command& c) {
		fire(t, this, c);
	}
	void handle(Command::STA t, const Command& c) {
		fire(t, this, c);
	}
	template<typename T>
	void handle(T , Command& ) {
	}
	GETSET(ConnectionQueueItem*, cqi, CQI);
	GETSET(States, state, State);
	GETSET(u_int32_t, lastActivity, LastActivity);
	GETSET(string, nick, Nick);
	
private:
	BufferedSocket* socket;
	User::Ptr user;
	
	static const string UPLOAD, DOWNLOAD;
	
	union {
		Download* download;
		Upload* upload;
	};

	// We only want ConnectionManager to create this...
	UserConnection() throw(SocketException) : cqi(NULL), state(STATE_UNCONNECTED), lastActivity(0), 
		socket(BufferedSocket::getSocket('|')), download(NULL) { 
		
		socket->addListener(this);
	};

	virtual ~UserConnection() {
		socket->removeListener(this);
		removeListeners();
		BufferedSocket::putSocket(socket);
	};
	friend struct DeleteFunction<UserConnection*>;

	UserConnection(const UserConnection&);
	UserConnection& operator=(const UserConnection&);

	void setUser(const User::Ptr& aUser) {
		user = aUser;
	};

	void onLine(const string& aLine) throw();
	
	void send(const string& aString) {
		lastActivity = GET_TICK();
		socket->write(aString);
	}

	virtual void on(Connected) throw() {
        lastActivity = GET_TICK();
        fire(UserConnectionListener::Connected(), this); 
    }
	virtual void on(Line, const string&) throw();
	virtual void on(Data, u_int8_t* data, size_t len) throw() { 
        lastActivity = GET_TICK(); 
        fire(UserConnectionListener::Data(), this, data, len); 
    }
	virtual void on(BytesSent, size_t bytes, size_t actual) throw() { 
        lastActivity = GET_TICK();
        fire(UserConnectionListener::BytesSent(), this, bytes, actual); 
    }
	virtual void on(ModeChange) throw() { 
        lastActivity = GET_TICK(); 
        fire(UserConnectionListener::ModeChange(), this); 
    }
	virtual void on(TransmitDone) throw() { fire(UserConnectionListener::TransmitDone(), this); }
	virtual void on(Failed, const string&) throw();
};

#endif // !defined(AFX_USERCONNECTION_H__52BFD1A0_9924_4C07_BAFA_FB9682884841__INCLUDED_)

/**
 * @file
 * $Id: UserConnection.h,v 1.71 2004/04/24 20:56:27 arnetheduck Exp $
 */


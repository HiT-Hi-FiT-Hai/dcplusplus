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
	File* getFile() { return file; };
	void setFile(File* aFile) { 
		if(file != NULL) {
			delete file;
		}
		file = aFile;
	}

	int64_t getPos() { return pos; };
	void setPos(u_int64_t aPos) { pos = aPos; };
	void setPos(u_int64_t aPos, bool aUpdate) { 
		pos = aPos;
		if(aUpdate) {
			file->setPos(aPos);
		}
	};
	void addPos(u_int64_t aPos) { pos += aPos; last+=aPos; total+=aPos; };
	
	int64_t getTotal() { return total; };
	void resetTotal() { total = 0; };
	
	int64_t getSize() { return size; };
	void setSize(u_int64_t aSize) { size = aSize; };
	void setSize(const string& aSize) { setSize(Util::toInt64(aSize)); };

	int64_t getAverageSpeed() {
		int64_t dif = (int64_t)(GET_TICK() - getStart());
		if(dif > 0) {
			return (int) (getTotal() * (int64_t)1000 / dif);
		} else {
			return 0;
		}
	}

	int64_t getSecondsLeft() {
		int64_t avg = getAverageSpeed();
		if(avg > 0)
			return (getSize() - getPos()) / avg;
		else
			return 0;
	}

	Transfer(ConnectionQueueItem* aQI) : cqi(aQI), start(0), last(0), total(0), 
		file(NULL), pos(-1), size(-1) { };
	~Transfer() { if(file) delete file; };

	GETSET(ConnectionQueueItem*, cqi, CQI);
	GETSET(u_int32_t, start, Start);
private:
	int64_t last;
	int64_t total;
	
	File* file;
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
 * $Id: UserConnection.h,v 1.40 2002/04/09 18:43:28 arnetheduck Exp $
 * @if LOG
 * $Log: UserConnection.h,v $
 * Revision 1.40  2002/04/09 18:43:28  arnetheduck
 * Major code reorganization, to ease maintenance and future port...
 *
 * Revision 1.39  2002/04/03 23:20:35  arnetheduck
 * ...
 *
 * Revision 1.38  2002/03/07 19:07:52  arnetheduck
 * Minor fixes + started code review
 *
 * Revision 1.37  2002/03/04 23:52:31  arnetheduck
 * Updates and bugfixes, new user handling almost finished...
 *
 * Revision 1.36  2002/02/27 12:02:09  arnetheduck
 * Completely new user handling, wonder how it turns out...
 *
 * Revision 1.35  2002/02/25 15:39:29  arnetheduck
 * Release 0.154, lot of things fixed...
 *
 * Revision 1.34  2002/02/18 23:48:32  arnetheduck
 * New prerelease, bugs fixed and features added...
 *
 * Revision 1.33  2002/02/12 00:35:37  arnetheduck
 * 0.153
 *
 * Revision 1.32  2002/02/09 18:13:51  arnetheduck
 * Fixed level 4 warnings and started using new stl
 *
 * Revision 1.31  2002/02/07 17:25:28  arnetheduck
 * many bugs fixed, time for 0.152 I think
 *
 * Revision 1.30  2002/02/01 02:00:47  arnetheduck
 * A lot of work done on the new queue manager, hopefully this should reduce
 * the number of crashes...
 *
 * Revision 1.29  2002/01/20 22:54:46  arnetheduck
 * Bugfixes to 0.131 mainly...
 *
 * Revision 1.28  2002/01/19 13:09:10  arnetheduck
 * Added a file class to hide ugly file code...and fixed a small resume bug (I think...)
 *
 * Revision 1.27  2002/01/17 23:35:59  arnetheduck
 * Reworked threading once more, now it actually seems stable. Also made
 * sure that noone tries to access client objects that have been deleted
 * as well as some other minor updates
 *
 * Revision 1.26  2002/01/14 22:19:43  arnetheduck
 * Commiting minor bugfixes
 *
 * Revision 1.25  2002/01/13 22:50:48  arnetheduck
 * Time for 0.12, added favorites, a bunch of new icons and lot's of other stuff
 *
 * Revision 1.24  2002/01/11 14:52:57  arnetheduck
 * Huge changes in the listener code, replaced most of it with templates,
 * also moved the getinstance stuff for the managers to a template
 *
 * Revision 1.23  2002/01/07 23:05:48  arnetheduck
 * Resume rollback implemented
 *
 * Revision 1.22  2002/01/06 21:55:20  arnetheduck
 * Some minor bugs fixed, but there remains one strange thing, the reconnect
 * button doesn't work...
 *
 * Revision 1.21  2002/01/05 10:13:40  arnetheduck
 * Automatic version detection and some other updates
 *
 * Revision 1.20  2002/01/02 16:12:33  arnetheduck
 * Added code for multiple download sources
 *
 * Revision 1.19  2001/12/30 15:03:45  arnetheduck
 * Added framework to handle incoming searches
 *
 * Revision 1.18  2001/12/29 13:47:14  arnetheduck
 * Fixing bugs and UI work
 *
 * Revision 1.17  2001/12/21 20:21:17  arnetheduck
 * Private messaging added, and a lot of other updates as well...
 *
 * Revision 1.16  2001/12/16 19:47:48  arnetheduck
 * Reworked downloading and user handling some, and changed some small UI things
 *
 * Revision 1.15  2001/12/15 17:01:06  arnetheduck
 * Passive mode searching as well as some searching code added
 *
 * Revision 1.14  2001/12/13 19:21:57  arnetheduck
 * A lot of work done almost everywhere, mainly towards a friendlier UI
 * and less bugs...time to release 0.06...
 *
 * Revision 1.13  2001/12/10 10:48:40  arnetheduck
 * Ahh, finally found one bug that's been annoying me for days...=) the connections
 * in the pool were not reset correctly before being put back for later use...
 *
 * Revision 1.12  2001/12/08 20:59:26  arnetheduck
 * Fixing bugs...
 *
 * Revision 1.11  2001/12/08 14:25:49  arnetheduck
 * More bugs removed...did my first search as well...
 *
 * Revision 1.10  2001/12/07 20:03:28  arnetheduck
 * More work done towards application stability
 *
 * Revision 1.9  2001/12/05 14:27:35  arnetheduck
 * Premature disconnection bugs removed.
 *
 * Revision 1.8  2001/12/04 21:50:34  arnetheduck
 * Work done towards application stability...still a lot to do though...
 * a bit more and it's time for a new release.
 *
 * Revision 1.7  2001/12/03 20:52:19  arnetheduck
 * Blah! Finally, the listings are working...one line of code missing (of course),
 * but more than 2 hours of search...hate that kind of bugs...=(...some other
 * things spiffed up as well...
 *
 * Revision 1.6  2001/12/02 23:47:35  arnetheduck
 * Added the framework for uploading and file sharing...although there's something strange about
 * the file lists...my client takes them, but not the original...
 *
 * Revision 1.5  2001/12/02 11:16:47  arnetheduck
 * Optimised hub listing, removed a few bugs and leaks, and added a few small
 * things...downloads are now working, time to start writing the sharing
 * code...
 *
 * Revision 1.4  2001/12/01 17:15:03  arnetheduck
 * Added a crappy version of huffman encoding, and some other minor changes...
 *
 * Revision 1.3  2001/11/29 19:10:55  arnetheduck
 * Refactored down/uploading and some other things completely.
 * Also added download indicators and download resuming, along
 * with some other stuff.
 *
 * Revision 1.2  2001/11/26 23:40:37  arnetheduck
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

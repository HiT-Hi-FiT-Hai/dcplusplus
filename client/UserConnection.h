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
#include "TimerManager.h"
#include "User.h"

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
	virtual void onAction(Types, UserConnection*, DWORD) { };
	virtual void onAction(Types, UserConnection*, const string&) { };
	virtual void onAction(Types, UserConnection*, const BYTE*, int) { };
	virtual void onAction(Types, UserConnection*, const string&, const string&) { };
	virtual void onAction(Types, UserConnection*, const string&, LONGLONG) { };
	virtual void onAction(Types, UserConnection*, int) { };
};

class Transfer {
public:
	HANDLE getFile() { return file; };
	void setFile(HANDLE aFile, bool aUpdate = false) { 
		file = aFile;
		if(aUpdate) {
			DWORD high =0;
			size = (LONGLONG) GetFileSize(aFile, &high) | (((LONGLONG)high) << 32);
		}
	}

	LONGLONG getPos() { return pos; };
	void setPos(LONGLONG aPos) { pos = aPos; };
	void setPos(LONGLONG aPos, bool aUpdate) { 
		pos = aPos;
		if(aUpdate) {
			long high = pos >> 32;
			SetFilePointer(file, (DWORD)pos, &high, FILE_BEGIN);
		}
	};
	void addPos(LONGLONG aPos) { pos += aPos; last+=aPos; total+=aPos; };
	
	LONGLONG getTotal() { return total; };
	void resetTotal() { total = 0; };
	
	DWORD getStart() { return start; };
	void setStart(DWORD aStart) { start = aStart; };

	LONGLONG getSize() { return size; };
	void setSize(LONGLONG aSize) { size = aSize; };
	void setSize(const string& aSize) { setSize(_atoi64(aSize.c_str())); };

	Transfer() : total(0), start(0), last(0), pos(-1), size(-1), file(NULL) { };
	~Transfer() { if(file) CloseHandle(file); };
private:
	DWORD start;
	DWORD last;
	LONGLONG total;
	
	HANDLE file;
	LONGLONG pos;
	LONGLONG size;

};
class ServerSocket;

class UserConnection : public Speaker<UserConnectionListener>, private BufferedSocketListener, TimerManagerListener
{
public:
	friend class ConnectionManager;
	
	typedef UserConnection* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;
	typedef map<string, Ptr> NickMap;
	typedef NickMap::iterator NickIter;
	
	enum {	
		MODE_COMMAND = BufferedSocket::MODE_LINE,
		MODE_DATA = BufferedSocket::MODE_DATA
	};

	enum {
		CONNECTING,
		LOGIN,
		BUSY,
		FREE
	};

	enum {
		FLAG_UPLOAD = 0x01,
		FLAG_DOWNLOAD = 0x02,
		FLAG_INCOMING = 0x04
	};

	void myNick(const string& aNick) { send("$MyNick " + aNick + "|"); }
	void lock(const string& aLock, const string& aPk) { send ("$Lock " + aLock + " Pk=" + aPk + "|"); }
	void key(const string& aKey) { send("$Key " + aKey + "|"); }
	void direction(const string& aDirection, const string& aNumber) { send("$Direction " + aDirection + " " + aNumber + "|"); }
	
	void get(const string& aFile, LONGLONG aResume) { 
		char buf[512];
		sprintf(buf, "$Get %s$%I64d|", aFile.c_str(), aResume+1);
		send(buf);
	}
	
	void fileLength(const string& aLength) { send("$FileLength " + aLength + "|"); }
	void startSend() { send("$Send|"); }
	void error(const string& aError) { send("$Error " + aError + "|"); };
	void listLen(const string& aLength) { send("$ListLen " + aLength + "|"); };
	void maxedOut() { send("$MaxedOut|"); };

	void setDataMode(LONGLONG aBytes) { socket.setDataMode(aBytes); }

	void connect(const string& aServer, short aPort = 412);
	void accept(const ServerSocket& aSocket);
	void waitForConnection(short aPort = 412);
	void disconnect() {
		socket.disconnect();
	}
	
	void transmitFile(HANDLE f) {
		socket.transmitFile(f);
	}

	const string& getDirectionString() {
		dcassert(flags & (FLAG_UPLOAD | FLAG_DOWNLOAD));
		return (flags & UserConnection::FLAG_UPLOAD) ? UPLOAD : DOWNLOAD;
	}
	const string& getNick() {
		if(nick.empty()) {
			return SETTING(NICK);
		} else {
			return nick;
		}
	}
	void setNick(const string& aNick) {
		nick = aNick;
	}
	
	User::Ptr& getUser() { return user; };

private:
	string nick;
	string server;
	short port;
	BufferedSocket socket;
	
	int state;
	int flags;
	DWORD lastActivity;
	User::Ptr user;
	
	static const string UPLOAD, DOWNLOAD;
	
	void setUser(const User::Ptr& aUser) {
		user = aUser;
	}


	void reset() {
		dcdebug("UserConnection(%p)::reset\n", this );

		TimerManager::getInstance()->removeListener(this);
		removeListeners();
		disconnect();
		user = User::nuser;
		flags = 0;
		state = LOGIN;
		server = "";
		port = 0;
		lastActivity = 0;
	}

	// We only want ConnectionManager to create this...
	UserConnection() : socket('|'), user(NULL), state(LOGIN), flags(0), port(0), lastActivity(0) { 
		socket.addListener(this);
	};
	virtual ~UserConnection() {
		dcdebug("UserConnection destroyer\n", this );
	};

	// BufferedSocketListener
	virtual void onAction(BufferedSocketListener::Types type) {
		switch(type) {
		case BufferedSocketListener::CONNECTED:
			TimerManager::getInstance()->addListener(this); 
			lastActivity = TimerManager::getTick();
			fire(UserConnectionListener::CONNECTED, this);
			break;
		case BufferedSocketListener::TRANSMIT_DONE:
			fire(UserConnectionListener::TRANSMIT_DONE, this); break;
		}
	}
	virtual void onAction(BufferedSocketListener::Types type, DWORD bytes) {
		switch(type) {
		case BufferedSocketListener::BYTES_SENT:
			lastActivity = TimerManager::getTick(); fire(UserConnectionListener::BYTES_SENT, this, bytes); break;
		default:
			dcassert(0);
		}
	}
	virtual void onAction(BufferedSocketListener::Types type, const string& aLine) {
		switch(type) {
		case BufferedSocketListener::LINE:
			onLine(aLine); break;
		case BufferedSocketListener::FAILED:
			fire(UserConnectionListener::FAILED, this, aLine); break;
		default:
			dcassert(0);
		}
	}
	virtual void onAction(BufferedSocketListener::Types type, int mode) {
		switch(type) {
		case BufferedSocketListener::MODE_CHANGE:
			fire(UserConnectionListener::MODE_CHANGE, this, mode); break;
		default:
			dcassert(0);
		}
	}
	virtual void onAction(BufferedSocketListener::Types type, const BYTE* buf, int len) {
		switch(type) {
		case BufferedSocketListener::DATA:
			lastActivity = TimerManager::getTick(); fire(UserConnectionListener::DATA, this, buf, len); break;
		default:
			dcassert(0);
		}
	}

	void onLine(const string& aLine) throw();

	// TimerManagerListener
	virtual void onAction(TimerManagerListener::Types type, DWORD aTick) {
		if(type == TimerManagerListener::SECOND) {
			if((lastActivity + 180 * 1000) < aTick) {
				// Nothing's happened for 180 seconds, fire error...
				dcdebug("UserConnection::onTimerSecond Connection timeout\n");
				fire(UserConnectionListener::FAILED, this, "Connection Timeout");
				lastActivity = aTick;
			}
		}
	}

	void send(const string& aString) {
		lastActivity = TimerManager::getTick();
		try {
			socket.write(aString);
		} catch(SocketException e) {
			fire(UserConnectionListener::FAILED, this, e.getError());
		}
	}
};

#endif // !defined(AFX_USERCONNECTION_H__52BFD1A0_9924_4C07_BAFA_FB9682884841__INCLUDED_)

/**
 * @file UserConnection.h
 * $Id: UserConnection.h,v 1.26 2002/01/14 22:19:43 arnetheduck Exp $
 * @if LOG
 * $Log: UserConnection.h,v $
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

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
class User;

class UserConnectionListener {
public:
	typedef UserConnectionListener* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;

	virtual void onBytesSent(UserConnection* aSource, DWORD aBytes) { };
	virtual void onConnecting(UserConnection* aSource, const string& aServer) { };
	virtual void onConnected(UserConnection* aSource) { };
	virtual void onData(UserConnection* aSource, BYTE* aBuf, int aLen) { };
	virtual void onError(UserConnection* aSource, const string& aError) { };
	virtual void onLock(UserConnection* aSource, const string& aLock, const string& aPk) { };
	virtual void onKey(UserConnection* aSource, const string& aKey) { };
	virtual void onDirection(UserConnection* aSource, const string& aDirection, const string& aNumber) { };
	virtual void onGet(UserConnection* aSource, const string& aFile, LONGLONG aResumeFrom) { };
	virtual void onFileLength(UserConnection* aSource, const string& aFileLength) { };
	virtual void onSend(UserConnection* aSource) { };
	virtual void onGetListLen(UserConnection* aSource) { };
	virtual void onMaxedOut(UserConnection* aSource) { };
	virtual void onModeChange(UserConnection* aSource, int aNewMode) { };
	virtual void onMyNick(UserConnection* aSource, const string& aNick) { };
	virtual void onTransmitDone(UserConnection* aSource) { };
};

class Transfer {
public:
	const string& getFileName() { return fileName; };
	void setFileName(const string& aName) { fileName = aName; };

	HANDLE getFile() { return file; };
	void setFile(HANDLE aFile, bool aUpdate = false) { 
		file = aFile;
		if(aUpdate) {
			DWORD high;
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
	void addPos(LONGLONG aPos) { pos += aPos; };
	
	LONGLONG getSize() { return size; };
	void setSize(LONGLONG aSize) { size = aSize; };
	void setSize(const string& aSize) { setSize(_atoi64(aSize.c_str())); };
	User* getUser() { return user; };
	void setUser(User* aUser) { user = aUser; };

	Transfer() : pos(-1), size(-1), file(NULL) { };
	~Transfer() { if(file) CloseHandle(file); };
private:
	User* user;
	string fileName;
	HANDLE file;
	LONGLONG pos;
	LONGLONG size;

};
class ServerSocket;

class UserConnection : public Speaker<UserConnectionListener>, public BufferedSocketListener
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
	};

	virtual void onBytesSent(DWORD aBytes) { fireBytesSent(aBytes); };
	virtual void onConnected() { fireConnected(); };
	virtual void onLine(const string& aLine);
	virtual void onError(const string& aError) { fireError(aError); };
	virtual void onModeChange(int aNewMode) { fireModeChange(aNewMode); };
	virtual void onData(BYTE *aBuf, int aLen) { fireData(aBuf, aLen); };
	virtual void onTransmitDone() {
		fireTransmitDone();
	};
	
	void myNick(const string& aNick) { send("$MyNick " + aNick + "|"); }
	void lock(const string& aLock, const string& aPk) { send ("$Lock " + aLock + " Pk=" + aPk + "|"); }
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

	User* getUser() { return user; };

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
private:
	string server;
	short port;
	BufferedSocket socket;
	User* user;
	int state;
	int flags;
	
	// We only want ConnectionManager to create this...
	UserConnection() : socket('|'), user(NULL), state(LOGIN), flags(0) { 
		socket.addListener(this);
	};
	virtual ~UserConnection() {
		dcdebug("UserConnection destroyer\n");
	};
	
	void send(const string& aString) {
		socket.write(aString);
	}
	void fireBytesSent(DWORD aBytes) {
		listenerCS.enter();
	//	dcdebug("UserConnection::fireBytesSent\n");
		UserConnectionListener::List tmp = listeners;
		listenerCS.leave();
		for(UserConnectionListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onBytesSent(this, aBytes);
		}
	}
	void fireConnected() {
		listenerCS.enter();
		dcdebug("UserConnection::fireConnected\n");
		UserConnectionListener::List tmp = listeners;
		listenerCS.leave();
		for(UserConnectionListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onConnected(this);
		}
	}
	void fireConnecting(const string& aServer) {
		listenerCS.enter();
		dcdebug("UserConnection::fireConnecting %s\n", aServer.c_str());
		UserConnectionListener::List tmp = listeners;
		listenerCS.leave();
		for(UserConnectionListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onConnecting(this, aServer);
		}
	}
	void fireData(BYTE* aData, int aLen) {
		listenerCS.enter();
//		dcdebug("UserConnection::fireData %d\n", aLen);
		UserConnectionListener::List tmp = listeners;
		listenerCS.leave();
		for(UserConnectionListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onData(this, aData, aLen);
		}
	}
	void fireDirection(const string& aDirection, const string& aNumber) {
		listenerCS.enter();
		dcdebug("UserConnection::fireDirection %s\n", aDirection.c_str());
		UserConnectionListener::List tmp = listeners;
		listenerCS.leave();
		for(UserConnectionListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onDirection(this, aDirection, aNumber);
		}
	}
	void fireError(const string& aError) {
		listenerCS.enter();
		dcdebug("UserConnection::fireError %s\n", aError.c_str());
		UserConnectionListener::List tmp = listeners;
		listenerCS.leave();
		for(UserConnectionListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onError(this, aError);
		}
	}
	void fireFileLength(const string& aLength) {
		listenerCS.enter();
		dcdebug("UserConnection::fireFileLength %s\n", aLength.c_str());
		UserConnectionListener::List tmp = listeners;
		listenerCS.leave();
		for(UserConnectionListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onFileLength(this, aLength);
		}
	}
	void fireGet(const string& aFile, LONGLONG aResume) {
		listenerCS.enter();
		dcdebug("UserConnection::fireGet %s\n", aFile.c_str());
		UserConnectionListener::List tmp = listeners;
		listenerCS.leave();
		for(UserConnectionListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onGet(this, aFile, aResume);
		}
	}
	void fireKey(const string& aKey) {
		listenerCS.enter();
		dcdebug("UserConnection::fireKey\n");
		UserConnectionListener::List tmp = listeners;
		listenerCS.leave();
		for(UserConnectionListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onKey(this, aKey);
		}
	}
	void fireGetListLen() {
		listenerCS.enter();
		dcdebug("fireGetListLen\n");
		UserConnectionListener::List tmp = listeners;
		listenerCS.leave();
		for(UserConnectionListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onGetListLen(this);
		}
	}
	void fireLock(const string& aLock, const string& aPk) {
		listenerCS.enter();
		dcdebug("UserConnection::fireLock\n");
		UserConnectionListener::List tmp = listeners;
		listenerCS.leave();
		for(UserConnectionListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onLock(this, aLock, aPk);
		}
	}
	void fireMaxedOut() {
		listenerCS.enter();
		dcdebug("UserConnection::fireMaxedOut\n");
		UserConnectionListener::List tmp = listeners;
		listenerCS.leave();
		for(UserConnectionListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onMaxedOut(this);
		}
	}
	void fireModeChange(int aNewMode) {
		listenerCS.enter();
		dcdebug("UserConnection::fireModeChange\n");
		UserConnectionListener::List tmp = listeners;
		listenerCS.leave();
		for(UserConnectionListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onModeChange(this, aNewMode);
		}
	}
	void fireMyNick(const string& aNick) {
		listenerCS.enter();
		dcdebug("UserConnection::fireMyNick %s\n", aNick.c_str());
		UserConnectionListener::List tmp = listeners;
		listenerCS.leave();
		for(UserConnectionListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onMyNick(this, aNick);
		}
	}
	void fireSend() {
		listenerCS.enter();
		dcdebug("UserConnection::fireSend\n");
		UserConnectionListener::List tmp = listeners;
		listenerCS.leave();
		for(UserConnectionListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onSend(this);
		}
	}
	void fireTransmitDone() {
		listenerCS.enter();
		dcdebug("UserConnection::fireTransmitDone\n");
		UserConnectionListener::List tmp = listeners;
		listenerCS.leave();
		for(UserConnectionListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onTransmitDone(this);
		}
	}
};

#endif // !defined(AFX_USERCONNECTION_H__52BFD1A0_9924_4C07_BAFA_FB9682884841__INCLUDED_)

/**
 * @file UserConnection.h
 * $Id: UserConnection.h,v 1.10 2001/12/07 20:03:28 arnetheduck Exp $
 * @if LOG
 * $Log: UserConnection.h,v $
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

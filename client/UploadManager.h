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

#if !defined(AFX_UPLOADMANAGER_H__B0C67119_3445_4208_B5AA_938D4A019703__INCLUDED_)
#define AFX_UPLOADMANAGER_H__B0C67119_3445_4208_B5AA_938D4A019703__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "UserConnection.h"
#include "ConnectionManager.h"
#include "ShareManager.h"
#include "Util.h"

class Upload : public Transfer {
public:
	typedef Upload* Ptr;
	typedef map<UserConnection::Ptr, Ptr> Map;
	typedef Map::iterator MapIter;
	
	const string& getFileName() { return fileName; };
	void setFileName(const string& aName) { fileName = aName; };

	void setUser(User::Ptr& aUser) { user = aUser; }
	User::Ptr& getUser() { return user; };
	
private:
	string fileName;
	User::Ptr user;	

};

class UploadManagerListener {
public:
	typedef UploadManagerListener* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;
	
	enum Types {
		COMPLETE,
		FAILED,
		STARTING,
		TICK
	};

	virtual void onAction(Types type, Upload* aUpload) { };
	virtual void onAction(Types type, Upload* aUpload, const string& aReason) { };

};

class UploadManager : private UserConnectionListener, public Speaker<UploadManagerListener>, private TimerManagerListener, public Singleton<UploadManager>
{
public:
	void removeUpload(Upload* aUpload) {
		cs.enter();
		for(Upload::MapIter i = uploads.begin(); i != uploads.end(); ++i) {
			if(i->second == aUpload) {
				removeConnection(i->first);
				uploads.erase(i);
				fire(UploadManagerListener::FAILED, aUpload, "Aborted");
				delete aUpload;
				break;
			}
		}
		cs.leave();
	}

	int getConnections() { cs.enter(); int sz = connections.size(); cs.leave(); return sz; };
	
	void addConnection(UserConnection::Ptr conn) {
		conn->addListener(this);
		connections.push_back(conn);
	}

	void removeConnection(UserConnection::Ptr aConn) {
		cs.enter();
		for(UserConnection::Iter i = connections.begin(); i != connections.end(); ++i) {
			if(*i == aConn) {
				aConn->removeListener(this);
				connections.erase(i);
				ConnectionManager::getInstance()->putUploadConnection(aConn);
				break;
			}
		}
		cs.leave();
	}

	void removeConnections() {
		cs.enter();
		
		for(UserConnection::Iter i = connections.begin(); i != connections.end(); ++i) {
			(*i)->removeListener(this);
			ConnectionManager::getInstance()->putUploadConnection(*i);
		}
		connections.clear();
		cs.leave();
	}
	
private:
	UserConnection::List connections;
	Upload::Map uploads;
	CriticalSection cs;
	
	friend class Singleton<UploadManager>;
	UploadManager() { 
		TimerManager::getInstance()->addListener(this);
	};
	~UploadManager() {
		TimerManager::getInstance()->removeListener(this);
		UserConnection::List tmp = connections;
		cs.enter();
		for(Upload::MapIter j = uploads.begin(); j != uploads.end(); ++j) {
			delete j->second;
		}
		cs.leave();

		removeConnections();
	}

	// TimerManagerListener
	virtual void onTimerSecond(DWORD aTick) {
		for(Upload::MapIter i = uploads.begin(); i != uploads.end(); ++i) {
			fire(UploadManagerListener::TICK, i->second);
		}
	}
	
	// UserConnectionListener
	virtual void onAction(UserConnectionListener::Types type, UserConnection* conn) {
		switch(type) {
		case UserConnectionListener::TRANSMIT_DONE:
			onTransmitDone(conn); break;
		case UserConnectionListener::SEND:
			onSend(conn); break;
		case UserConnectionListener::GET_LIST_LENGTH:
			conn->listLen(ShareManager::getInstance()->getListLenString()); break;
		}
	}
	virtual void onAction(UserConnectionListener::Types type, UserConnection* conn, DWORD bytes) {
		switch(type) {
		case UserConnectionListener::BYTES_SENT:
			onBytesSent(conn, bytes); break;
		}
	}
	virtual void onAction(UserConnectionListener::Types type, UserConnection* conn, const string& line) {
		switch(type) {
		case UserConnectionListener::FAILED:
			onFailed(conn, line); break;
		}
	}
	virtual void onAction(UserConnectionListener::Types type, UserConnection* conn, const string& line, LONGLONG resume) {
		switch(type) {
		case UserConnectionListener::GET:
			onGet(conn, line, resume); break;
		}
	}
	
	void onBytesSent(UserConnection* aSource, DWORD aBytes);
	void onFailed(UserConnection* aSource, const string& aError);
	void onTransmitDone(UserConnection* aSource);
	void onGet(UserConnection* aSource, const string& aFile, LONGLONG aResume);
	void onSend(UserConnection* aSource);
	
};

#endif // !defined(AFX_UPLOADMANAGER_H__B0C67119_3445_4208_B5AA_938D4A019703__INCLUDED_)

/**
 * @file UploadManger.h
 * $Id: UploadManager.h,v 1.25 2002/01/11 14:52:57 arnetheduck Exp $
 * @if LOG
 * $Log: UploadManager.h,v $
 * Revision 1.25  2002/01/11 14:52:57  arnetheduck
 * Huge changes in the listener code, replaced most of it with templates,
 * also moved the getinstance stuff for the managers to a template
 *
 * Revision 1.24  2002/01/06 00:14:54  arnetheduck
 * Incoming searches almost done, just need some testing...
 *
 * Revision 1.23  2002/01/05 19:06:09  arnetheduck
 * Added user list images, fixed bugs and made things more effective
 *
 * Revision 1.21  2002/01/05 10:13:40  arnetheduck
 * Automatic version detection and some other updates
 *
 * Revision 1.20  2002/01/02 16:12:33  arnetheduck
 * Added code for multiple download sources
 *
 * Revision 1.19  2001/12/29 13:47:14  arnetheduck
 * Fixing bugs and UI work
 *
 * Revision 1.18  2001/12/27 12:05:00  arnetheduck
 * Added flat tabs, fixed sorting and a StringTokenizer bug
 *
 * Revision 1.17  2001/12/21 20:21:17  arnetheduck
 * Private messaging added, and a lot of other updates as well...
 *
 * Revision 1.16  2001/12/16 19:47:48  arnetheduck
 * Reworked downloading and user handling some, and changed some small UI things
 *
 * Revision 1.15  2001/12/13 19:21:57  arnetheduck
 * A lot of work done almost everywhere, mainly towards a friendlier UI
 * and less bugs...time to release 0.06...
 *
 * Revision 1.14  2001/12/11 01:10:29  arnetheduck
 * More bugfixes...I really have to change the bufferedsocket so that it only
 * uses one thread...or maybe even multiple sockets/thread...
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
 * Revision 1.10  2001/12/07 20:03:26  arnetheduck
 * More work done towards application stability
 *
 * Revision 1.9  2001/12/05 19:40:13  arnetheduck
 * More bugfixes.
 *
 * Revision 1.8  2001/12/05 14:27:35  arnetheduck
 * Premature disconnection bugs removed.
 *
 * Revision 1.7  2001/12/04 21:50:34  arnetheduck
 * Work done towards application stability...still a lot to do though...
 * a bit more and it's time for a new release.
 *
 * Revision 1.6  2001/12/03 20:52:19  arnetheduck
 * Blah! Finally, the listings are working...one line of code missing (of course),
 * but more than 2 hours of search...hate that kind of bugs...=(...some other
 * things spiffed up as well...
 *
 * Revision 1.5  2001/12/02 23:47:35  arnetheduck
 * Added the framework for uploading and file sharing...although there's something strange about
 * the file lists...my client takes them, but not the original...
 *
 * Revision 1.4  2001/12/01 17:15:03  arnetheduck
 * Added a crappy version of huffman encoding, and some other minor changes...
 *
 * Revision 1.3  2001/11/29 19:10:55  arnetheduck
 * Refactored down/uploading and some other things completely.
 * Also added download indicators and download resuming, along
 * with some other stuff.
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

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

#if !defined(AFX_DOWNLOADMANAGER_H__D6409156_58C2_44E9_B63C_B58C884E36A3__INCLUDED_)
#define AFX_DOWNLOADMANAGER_H__D6409156_58C2_44E9_B63C_B58C884E36A3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Exception.h"
#include "UserConnection.h"
#include "Util.h"
#include "TimerManager.h"

class SimpleXML;

STANDARD_EXCEPTION(DownloadException);

class Download : public Transfer {
public:
	class Source {
	public:
		typedef Source* Ptr;
		typedef vector<Ptr> List;
		typedef List::iterator Iter;

		Source(const string& aNick, const string& aFileName, const string& aPath, const User::Ptr& aUser = User::nuser) : nick(aNick),
			fileName(aFileName), path(aPath), user(aUser) { };

		User::Ptr& getUser() { return user; };
		void setUser(const User::Ptr& aUser) {
			user = aUser;
		}

		GETSETREF(string, fileName, FileName);
		GETSETREF(string, nick, Nick);
		GETSETREF(string, path, Path);
		
	private:
		User::Ptr user;
	};

	typedef Download* Ptr;
	typedef list<Ptr> List;
	typedef List::iterator Iter;
	typedef map<UserConnection::Ptr, Ptr> Map;
	typedef Map::iterator MapIter;
	typedef map<User::Ptr, Ptr> UserMap;
	typedef UserMap::iterator UserIter;
	
	Download() : flags(0), currentSource(NULL), rollbackBuffer(NULL) { }
	~Download() {
		if(rollbackBuffer)
			delete rollbackBuffer;

		for(Source::Iter i = sources.begin(); i != sources.end(); ++i) {
			delete *i;
		}
	}

	enum {
		USER_LIST = 0x01,
		RUNNING = 0x02,
		RESUME = 0x04,
		ROLLBACK = 0x08
	};

	BYTE* getRollbackBuffer() { return rollbackBuffer; };
	void setRollbackBuffer(int aSize) { 
		if(rollbackBuffer) 
			delete rollbackBuffer;

		if(aSize > 0) {
			rollbackBuffer = new BYTE[aSize];
		} else {
			rollbackBuffer = NULL;
		}
		rollbackSize = aSize;
	}

	int getRollbackSize() { return rollbackSize; };

	const string& getTarget() { return target; };
	void setTarget(const string& aTarget) { target = aTarget; };

	Source::Ptr addSource(const string& aNick, const string& aFileName, const string& aPath) {
		Source::Ptr s = new Source(aNick, aFileName, aPath);
		sources.push_back(s);
		return s;
	}
	Source::Ptr addSource(const User::Ptr& aUser, const string& aFileName, const string& aPath) {
		Source::Ptr s = new Source(aUser->getNick(), aFileName, aPath, aUser);
		sources.push_back(s);
		return s;
	}

	void removeSource(Source* aSource) {
		for(Source::Iter i = sources.begin(); i != sources.end(); ++i) {
			if(*i == aSource) {
				sources.erase(i);
				break;
			}
		}
	}
	
	Source::Ptr getSource(const User::Ptr& aUser) {
		// First, search by user...or nick...
		for(Source::List::const_iterator i = sources.begin(); i != sources.end(); ++i) {
			if((*i)->getUser() == aUser)
				return *i;
		}

		for(Source::List::const_iterator j = sources.begin(); j != sources.end(); ++j) {
			if( !((*j)->getUser() && (*j)->getUser()->isOnline()) && ((*j)->getNick() == aUser->getNick())) {
				(*j)->setUser(aUser);
				return *j;
			}
		}
		
		return NULL;
	}
	
	bool isSource(const User::Ptr& aUser) const {
		for(Source::List::const_iterator i = sources.begin(); i != sources.end(); ++i) {
			if((*i)->getUser() == aUser)
				return true;
		}
		for(Source::List::const_iterator j = sources.begin(); j != sources.end(); ++j) {
			if( !((*j)->getUser() && (*j)->getUser()->isOnline()) && ((*j)->getNick() == aUser->getNick())) {
				(*j)->setUser(aUser);
				return true;
			}
		}
		return false;
	}

	bool isSource(const string& aUser) const {
		for(Source::List::const_iterator j = sources.begin(); j != sources.end(); ++j) {
			if((*j)->getNick() == aUser) {
				return true;
			}
		}
		return false;
	}
	
	Source::List& getSources() { return sources; };

	Source* getCurrentSource() { return currentSource; };
	void setCurrentSource(Source* aSource) { currentSource = aSource; };
	
	bool isSet(int aFlag) { return (flags & aFlag) > 0; };
	void setFlag(int aFlag) { flags |= aFlag; };
	void unsetFlag(int aFlag) { flags &= ~aFlag; };
private:

	int flags;
	string target;
	Source* currentSource;
	
	BYTE* rollbackBuffer;
	int rollbackSize;
	
	Source::List sources;
};

class DownloadManagerListener {
public:
	typedef DownloadManagerListener* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;

	enum Types {
		ADDED,
		COMPLETE,
		CONNECTING,
		FAILED,
		REMOVED,
		SOURCE_ADDED,
		SOURCE_REMOVED,
		SOURCE_UPDATED,
		STARTING,
		TICK
	};

	virtual void onAction(Types type, Download* aDownload) { };
	virtual void onAction(Types type, Download* aDownload, Download::Source* aSource) { };
	virtual void onAction(Types type, Download* aDownload, const string& aReason) { };
	
};

class DownloadManager : public Speaker<DownloadManagerListener>, private UserConnectionListener, private TimerManagerListener, public Singleton<DownloadManager>
{
public:
	void download(const string& aFile, const string& aSize, const User::Ptr& aUser, const string& aDestination, bool aResume = true) throw(DownloadException) {
		download(aFile, aSize.length() > 0 ? Util::toInt64(aSize.c_str()) : -1, aUser, aDestination, aResume);
	}
	void download(const string& aFile, LONGLONG aSize, const User::Ptr& aUser, const string& aDestination, bool aResume = true) throw(DownloadException);

	void download(const string& aFile, const string& aSize, const string& aUser, const string& aDestination, bool aResume = true) throw(DownloadException) {
		download(aFile, aSize.length() > 0 ? Util::toInt64(aSize.c_str()) : -1, aUser, aDestination, aResume);
	}
	void download(const string& aFile, LONGLONG aSize, const string& aUser, const string& aDestination, bool aResume = true) throw(DownloadException);
	void downloadList(const User::Ptr& aUser) throw(DownloadException);
	void downloadList(const string& aUser) throw(DownloadException);
	void connectFailed(const User::Ptr& aUser);
	
	void removeDownload(Download* aDownload);

	void addConnection(UserConnection::Ptr conn) {
		conn->addListener(this);
		
		{
			Lock l(cs);
			waiting.erase(conn->getUser());
			connections.push_back(conn);
		}

		checkDownloads(conn);
	}
	StringList getTargetsBySize(LONGLONG aSize) {
		Lock l(cs);
		StringList sl;

		for(Download::Iter i = queue.begin(); i != queue.end(); ++i) {
			if((*i)->getSize() == aSize) {
				sl.push_back((*i)->getTarget());
			}
		}
		return sl;
	}
	void removeSource(Download* aDownload, Download::Source::Ptr aSource);

	void removeConnection(UserConnection::Ptr aConn, bool reuse = false);
	void removeConnections(); 

	void load(SimpleXML* aXml);
	void save(SimpleXML* aXml);
private:

	void failDownload(UserConnection* c, Download* d, const string& aReason) {
		{
			Lock l(cs);
			running.erase(c);
		}
		
		d->unsetFlag(Download::RUNNING);
		d->resetTotal();
		
		d->setFile(NULL);
		
		fire(DownloadManagerListener::FAILED, d, aReason);
		removeSource(d, d->getCurrentSource());
		
		d->setCurrentSource(NULL);

		removeConnection(c);
	}

	friend class Singleton<DownloadManager>;
	DownloadManager() { 
		TimerManager::getInstance()->addListener(this);
	};
	virtual ~DownloadManager() {
		TimerManager::getInstance()->removeListener(this);
		removeConnections();
		for(StringIter i = userLists.begin(); i!= userLists.end(); ++i) {
			DeleteFile(i->c_str());
		}
	};
	
	CriticalSection cs;

	Download::List queue;
	map<User::Ptr, DWORD> waiting;
	Download::Map running;
	
	UserConnection::List connections;
	StringList userLists;
	
	Download* getNextDownload(const User::Ptr& aUser) {
		Lock l(cs);
		Download* d = NULL;
		for(Download::Iter i = queue.begin(); i != queue.end(); ++i) {
			if((*i)->isSource(aUser) ) {
				if( !(*i)->isSet(Download::RUNNING) ) {
					if((*i)->getSize() < 16*1024) {
						d = *i;
						break;
					}

					if(d == NULL) {
						d = *i;
					}
				}
			}
		}
		return d;
	}

	void checkDownloads(UserConnection* aConn);
	
	// UserConnectionListener
	virtual void onAction(UserConnectionListener::Types type, UserConnection* conn) {
		switch(type) {
		case UserConnectionListener::MAXED_OUT:
			onMaxedOut(conn); break;
		}
	}
	virtual void onAction(UserConnectionListener::Types type, UserConnection* conn, const string& line) {
		switch(type) {
		case UserConnectionListener::FILE_LENGTH:
			onFileLength(conn, line); break;
		case UserConnectionListener::FAILED:
			onFailed(conn, line); break;
		}
	}
	virtual void onAction(UserConnectionListener::Types type, UserConnection* conn, const BYTE* data, int len) {
		switch(type) {
		case UserConnectionListener::DATA:
			onData(conn, data, len); break;
		}
	}

	virtual void onAction(UserConnectionListener::Types type, UserConnection* conn, int mode) {
		switch(type) {
		case UserConnectionListener::MODE_CHANGE:
			onModeChange(conn, mode); break;
		}
	}
	
	void onFailed(UserConnection* aSource, const string& aError);
	void onData(UserConnection* aSource, const BYTE* aData, int aLen);
	void onFileLength(UserConnection* aSource, const string& aFileLength);
	void onMaxedOut(UserConnection* aSource);
	void onModeChange(UserConnection* aSource, int aNewMode);
	
	// TimerManagerListener
	virtual void onAction(TimerManagerListener::Types type, DWORD aTick) {
		switch(type) {
		case TimerManagerListener::SECOND:
			onTimerSecond(aTick); break;
		case TimerManagerListener::MINUTE:
			onTimerMinute(aTick); break;
		default:
			dcassert(0);			
		}
	}
	void onTimerSecond(DWORD aTick);
	void onTimerMinute(DWORD aTick);

};

#endif // !defined(AFX_DOWNLOADMANAGER_H__D6409156_58C2_44E9_B63C_B58C884E36A3__INCLUDED_)

/**
 * @file DownloadManger.h
 * $Id: DownloadManager.h,v 1.28 2002/01/19 13:09:10 arnetheduck Exp $
 * @if LOG
 * $Log: DownloadManager.h,v $
 * Revision 1.28  2002/01/19 13:09:10  arnetheduck
 * Added a file class to hide ugly file code...and fixed a small resume bug (I think...)
 *
 * Revision 1.27  2002/01/18 17:41:43  arnetheduck
 * Reworked many right button menus, adding op commands and making more easy to use
 *
 * Revision 1.26  2002/01/17 23:35:59  arnetheduck
 * Reworked threading once more, now it actually seems stable. Also made
 * sure that noone tries to access client objects that have been deleted
 * as well as some other minor updates
 *
 * Revision 1.25  2002/01/14 22:19:43  arnetheduck
 * Commiting minor bugfixes
 *
 * Revision 1.24  2002/01/11 14:52:57  arnetheduck
 * Huge changes in the listener code, replaced most of it with templates,
 * also moved the getinstance stuff for the managers to a template
 *
 * Revision 1.23  2002/01/07 23:05:48  arnetheduck
 * Resume rollback implemented
 *
 * Revision 1.22  2002/01/05 19:06:09  arnetheduck
 * Added user list images, fixed bugs and made things more effective
 *
 * Revision 1.20  2002/01/05 10:13:39  arnetheduck
 * Automatic version detection and some other updates
 *
 * Revision 1.19  2002/01/02 16:12:32  arnetheduck
 * Added code for multiple download sources
 *
 * Revision 1.18  2001/12/30 15:03:45  arnetheduck
 * Added framework to handle incoming searches
 *
 * Revision 1.17  2001/12/29 13:47:14  arnetheduck
 * Fixing bugs and UI work
 *
 * Revision 1.16  2001/12/21 20:21:17  arnetheduck
 * Private messaging added, and a lot of other updates as well...
 *
 * Revision 1.15  2001/12/19 23:07:59  arnetheduck
 * Added directory downloading from the directory tree (although it hasn't been
 * tested at all) and password support.
 *
 * Revision 1.14  2001/12/18 12:32:18  arnetheduck
 * Stability fixes
 *
 * Revision 1.13  2001/12/16 19:47:48  arnetheduck
 * Reworked downloading and user handling some, and changed some small UI things
 *
 * Revision 1.12  2001/12/15 17:01:06  arnetheduck
 * Passive mode searching as well as some searching code added
 *
 * Revision 1.11  2001/12/13 19:21:57  arnetheduck
 * A lot of work done almost everywhere, mainly towards a friendlier UI
 * and less bugs...time to release 0.06...
 *
 * Revision 1.10  2001/12/11 01:10:29  arnetheduck
 * More bugfixes...I really have to change the bufferedsocket so that it only
 * uses one thread...or maybe even multiple sockets/thread...
 *
 * Revision 1.9  2001/12/10 10:48:40  arnetheduck
 * Ahh, finally found one bug that's been annoying me for days...=) the connections
 * in the pool were not reset correctly before being put back for later use...
 *
 * Revision 1.8  2001/12/07 20:03:06  arnetheduck
 * More work done towards application stability
 *
 * Revision 1.7  2001/12/05 14:27:35  arnetheduck
 * Premature disconnection bugs removed.
 *
 * Revision 1.6  2001/12/04 21:50:34  arnetheduck
 * Work done towards application stability...still a lot to do though...
 * a bit more and it's time for a new release.
 *
 * Revision 1.5  2001/12/02 23:47:35  arnetheduck
 * Added the framework for uploading and file sharing...although there's something strange about
 * the file lists...my client takes them, but not the original...
 *
 * Revision 1.4  2001/12/02 11:16:46  arnetheduck
 * Optimised hub listing, removed a few bugs and leaks, and added a few small
 * things...downloads are now working, time to start writing the sharing
 * code...
 *
 * Revision 1.3  2001/11/29 19:10:54  arnetheduck
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

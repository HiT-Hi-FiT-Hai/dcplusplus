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

#include "UserConnection.h"
#include "CryptoManager.h"
#include "Util.h"
#include "TimerManager.h"

class User;

class Download : public Transfer {
public:
	typedef Download* Ptr;
	typedef list<Ptr> List;
	typedef List::iterator Iter;
	typedef map<UserConnection::Ptr, Ptr> Map;
	typedef Map::iterator MapIter;
	
	Download() : lastTry(0), resume(false) { }

	bool getResume() { return resume; };
	void setResume(bool aResume) { resume = aResume; };

	string getTarget() { 
		if(target.length() == 0) {
			return Settings::getDownloadDirectory() + getFileName();
		} else {
			return target;
		}
	};

	void setTarget(const string& aTarget) { target = aTarget; };

	const string& getLastNick() { return lastNick; };
	const string& getLastPath() { return lastPath; };
	void setLast(const string& aNick, const string& aPath) { lastNick = aNick; lastPath = aPath; };

	DWORD getLastTry() { return lastTry; };
	void setLastTry(DWORD aTime) { lastTry = aTime; };

private:
	bool resume;
	string target;
	DWORD lastTry;
	
	string lastNick;
	string lastPath;
};

class DownloadManagerListener {
public:
	typedef DownloadManagerListener* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;
	
	virtual void onDownloadAdded(Download* aDownload) { };
	virtual void onDownloadComplete(Download* aDownload) { };
	virtual void onDownloadConnecting(Download* aDownload) { };
	virtual void onDownloadFailed(Download* aDownload, const string& aReason) { };
	virtual void onDownloadStarting(Download* aDownload) { };
	virtual void onDownloadTick(Download* aDownload) { };
};

class DownloadManager : public Speaker<DownloadManagerListener>, private UserConnectionListener, private TimerManagerListener
{
public:
	void download(const string& aFile, const string& aSize, User* aUser, const string& aDestination, bool aResume = true) {
		download(aFile, aSize.length() > 0 ? _atoi64(aSize.c_str()) : -1, aUser, aDestination, aResume);
	}
	void download(const string& aFile, LONGLONG aSize, User* aUser, const string& aDestination, bool aResume = true);

	void download(const string& aFile, const string& aSize, const string& aUser, const string& aDestination, bool aResume = true) {
		download(aFile, aSize.length() > 0 ? _atoi64(aSize.c_str()) : -1, aUser, aDestination, aResume);
	}
	void download(const string& aFile, LONGLONG aSize, const string& aUser, const string& aDestination, bool aResume = true);
	void downloadList(User* aUser);
	void downloadList(const string& aUser);
	void connectFailed(const string& aUser);
	
	void removeDownload(Download* aDownload);

	static DownloadManager* getInstance() {
		dcassert(instance);
		return instance;
	}
	
	static void newInstance() {
		if(instance)
			delete instance;
		
		instance = new DownloadManager();
		// Add ourselves to the Timer Manager
		TimerManager::getInstance()->addListener(instance);
	}

	static void deleteInstance() {
		if(instance)
			delete instance;
		instance = NULL;
	}
	
	void addConnection(UserConnection::Ptr conn) {
		conn->addListener(this);
		
		cs.enter();
		connections.push_back(conn);
		cs.leave();

		checkDownloads(conn);
	}
	
	void removeConnection(UserConnection::Ptr aConn);
	void removeConnections(); 
private:

	CriticalSection cs;

	Download::List queue;
	Download::Map running;
	
	map<User*, DWORD> lastConnection;
	
	static DownloadManager* instance;
	
	UserConnection::List connections;
	StringList userLists;
	
	void checkDownloads(UserConnection* aConn);
	
	// UserConnectionListener
	virtual void onError(UserConnection* aSource, const string& aError);
	virtual void onData(UserConnection* aSource, const BYTE* aData, int aLen);
	virtual void onFileLength(UserConnection* aSource, const string& aFileLength);
	virtual void onMaxedOut(UserConnection* aSource);
	virtual void onModeChange(UserConnection* aSource, int aNewMode);
	
	// TimerManagerListener
	virtual void onTimerSecond(DWORD aTick);
	
	void fireAdded(Download::Ptr aPtr) {
		listenerCS.enter();
		DownloadManagerListener::List tmp = listeners;
		listenerCS.leave();
		//dcdebug("DownloadManager::fireAdded %p\n", aPtr);
		for(DownloadManagerListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onDownloadAdded(aPtr);
		}
	}
	void fireComplete(Download::Ptr aPtr) {
		listenerCS.enter();
		DownloadManagerListener::List tmp = listeners;
		listenerCS.leave();
		//dcdebug("DownloadManager::fireComplete %p\n", aPtr);
		for(DownloadManagerListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onDownloadComplete(aPtr);
		}
	}
	void fireConnecting(Download::Ptr aPtr) {
		listenerCS.enter();
		DownloadManagerListener::List tmp = listeners;
		listenerCS.leave();
		//dcdebug("DownloadManager:.fireConnecting %p\n", aPtr);
		for(DownloadManagerListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onDownloadConnecting(aPtr);
		}
	}
	void fireFailed(Download::Ptr aPtr, const string& aReason) {
		listenerCS.enter();
		DownloadManagerListener::List tmp = listeners;
		listenerCS.leave();
		//dcdebug("DownloadManager::fireFailed %p\n", aPtr);
		for(DownloadManagerListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onDownloadFailed(aPtr, aReason);
		}
	}
	void fireStarting(Download::Ptr aPtr) {
		listenerCS.enter();
		DownloadManagerListener::List tmp = listeners;
		listenerCS.leave();
		//dcdebug("DownloadManager::fireStarting %p\n", aPtr);
		for(DownloadManagerListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onDownloadStarting(aPtr);
		}
	}
	void fireTick(Download::Ptr aPtr) {
		listenerCS.enter();
		DownloadManagerListener::List tmp = listeners;
		listenerCS.leave();
		//		dcdebug("fireGotLine %s\n", aLine.c_str());
		for(DownloadManagerListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onDownloadTick(aPtr);
		}
	}
	

	DownloadManager() { };
	virtual ~DownloadManager() {
		removeConnections();
		for(StringIter i = userLists.begin(); i!= userLists.end(); ++i) {
			DeleteFile(i->c_str());
		}
	};
};

#endif // !defined(AFX_DOWNLOADMANAGER_H__D6409156_58C2_44E9_B63C_B58C884E36A3__INCLUDED_)

/**
 * @file DownloadManger.h
 * $Id: DownloadManager.h,v 1.12 2001/12/15 17:01:06 arnetheduck Exp $
 * @if LOG
 * $Log: DownloadManager.h,v $
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

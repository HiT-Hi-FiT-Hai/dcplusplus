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

class Download {
public:
	typedef Download* Ptr;
	typedef list<Ptr> List;
	typedef List::iterator Iter;
	typedef map<UserConnection::Ptr, Ptr> Map;
	typedef Map::iterator MapIter;
	
	Download() : size(-1), pos(-1), hFile(NULL), lastTry(0) { }
	
	string fileName;
	LONGLONG size;
	LONGLONG pos;
	string targetFileName;
	User* user;
	bool resume;
	DWORD lastTry;
	
	HANDLE hFile;
	
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

class DownloadManager : public Speaker<DownloadManagerListener>, public UserConnectionListener, public TimerManagerListener
{
public:

	virtual void onDisconnected(UserConnection* aSource) {
		removeConnection(aSource);
	}
	
	virtual void onData(UserConnection* aSource, BYTE* aData, int aLen);
	virtual void onFileLength(UserConnection* aSource, const string& aFileLength);
	virtual void onMaxedOut(UserConnection* aSource);
	virtual void onModeChange(UserConnection* aSource, int aNewMode);

	virtual void onDirection(UserConnection* aSource, const string& aDirection, const string& aNumber) {
		dcassert(aDirection == "Upload");
	}
	
	virtual void onTimerSecond(DWORD aTick);

	void download(const string& aFile, const string& aSize, User* aUser, const string& aDestination, bool aResume = true) {
		download(aFile, aSize.length() > 0 ? _atoi64(aSize.c_str()) : -1, aUser, aDestination, aResume);
	}
	void download(const string& aFile, LONGLONG aSize, User* aUser, const string& aDestination, bool aResume = true);
	
	void checkDownloads(UserConnection* aConn);

	static DownloadManager* getInstance() {
		dcassert(instance);
		return instance;
	}
	
	static void newInstance() {
		if(instance)
			delete instance;
		
		instance = new DownloadManager();
	}

	static void deleteInstance() {
		if(instance)
			delete instance;
		instance = NULL;
	}
	
	void addConnection(UserConnection::Ptr conn) {
		conn->addListener(this);
		connections.push_back(conn);
		checkDownloads(conn);
	}
	
	void removeConnection(UserConnection::Ptr aConn) {
		for(UserConnection::Iter i = connections.begin(); i != connections.end(); ++i) {
			if(*i == aConn) {
				aConn->removeListener(this);
				connections.erase(i);
				return;
			}
		}
	}
	
	void removeConnections() {
		for(UserConnection::Iter i = connections.begin(); i != connections.end(); ++i) {
			(*i)->removeListener(this);
			connections.erase(i);
		}
	}

private:

	Download::List queue;
	Download::Map running;
	
	static DownloadManager* instance;
	StringList expectedNicks;
	
	UserConnection::List connections;
	
	void fireAdded(Download::Ptr aPtr) {
		listenerCS.enter();
		DownloadManagerListener::List tmp = listeners;
		listenerCS.leave();
		//		dcdebug("fireGotLine %s\n", aLine.c_str());
		for(DownloadManagerListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onDownloadAdded(aPtr);
		}
	}
	void fireComplete(Download::Ptr aPtr) {
		listenerCS.enter();
		DownloadManagerListener::List tmp = listeners;
		listenerCS.leave();
		//		dcdebug("fireGotLine %s\n", aLine.c_str());
		for(DownloadManagerListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onDownloadComplete(aPtr);
		}
	}
	void fireConnecting(Download::Ptr aPtr) {
		listenerCS.enter();
		DownloadManagerListener::List tmp = listeners;
		listenerCS.leave();
		//		dcdebug("fireGotLine %s\n", aLine.c_str());
		for(DownloadManagerListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onDownloadConnecting(aPtr);
		}
	}
	void fireFailed(Download::Ptr aPtr, const string& aReason) {
		listenerCS.enter();
		DownloadManagerListener::List tmp = listeners;
		listenerCS.leave();
		//		dcdebug("fireGotLine %s\n", aLine.c_str());
		for(DownloadManagerListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onDownloadFailed(aPtr, aReason);
		}
	}
	void fireStarting(Download::Ptr aPtr) {
		listenerCS.enter();
		DownloadManagerListener::List tmp = listeners;
		listenerCS.leave();
		//		dcdebug("fireGotLine %s\n", aLine.c_str());
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
	};
};

#endif // !defined(AFX_DOWNLOADMANAGER_H__D6409156_58C2_44E9_B63C_B58C884E36A3__INCLUDED_)

/**
 * @file DownloadManger.h
 * $Id: DownloadManager.h,v 1.5 2001/12/02 23:47:35 arnetheduck Exp $
 * @if LOG
 * $Log: DownloadManager.h,v $
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

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
#include "CriticalSection.h"

class Download {
public:
	typedef Download* Ptr;
	typedef list<Ptr> List;
	typedef List::iterator Iter;
	typedef map<UserConnection::Ptr, Ptr> Map;
	typedef Map::iterator MapIter;
	
	Download() : size(-1), pos(-1), hFile(NULL) { }
	
	string fileName;
	LONGLONG size;
	LONGLONG pos;
	string destination;
	
	HANDLE hFile;
	
	string lastUser;
	string lastPath;
};

class DownloadManagerListener {
public:
	typedef DownloadManagerListener* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;
	
	virtual void onDownloadComplete(Download::Ptr aDownload) { };
	virtual void onDownloadFailed(Download::Ptr aDownload, const string& aReason) { };
	virtual void onDownloadStarting(Download::Ptr aDownload) { };
	virtual void onDownloadTick(Download::Ptr aDownload) { }
};

class DownloadManager : public UserConnectionListener
{
public:

	virtual void onDisconnected(UserConnection* aSource) {
		removeConnection(aSource);
	}
	
	virtual void onLock(UserConnection* aSource, const string& aLock, const string& aPk);
	virtual void onKey(UserConnection* aSource, const string& aKey);
	virtual void onData(UserConnection* aSource, BYTE* aData, int aLen);
	virtual void onFileLength(UserConnection* aSource, const string& aFileLength);
	virtual void onMaxedOut(UserConnection* aSource) { aSource->disconnect(); fireDownloadFailed(running[aSource], "No slots available"); };
	virtual void onModeChange(UserConnection* aSource, int aNewMode);

	virtual void onDirection(UserConnection* aSource, const string& aDirection, const string& aNumber) {
		dcassert(aDirection == "Upload");
	}
	
	void download(const string& aFile, const string& aUser, const string& aDestination);
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
		removeNick(conn->getNick());
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

	void addListener(DownloadManagerListener::Ptr aListener) {
		listenerCS.enter();
		listeners.push_back(aListener);
		listenerCS.leave();
	}
	
	void removeListener(DownloadManagerListener::Ptr aListener) {
		listenerCS.enter();
		for(DownloadManagerListener::Iter i = listeners.begin(); i != listeners.end(); ++i) {
			if(*i == aListener) {
				listeners.erase(i);
				break;
			}
		}
		listenerCS.leave();
	}
	
	void removeListeners() {
		listenerCS.enter();
		listeners.clear();
		listenerCS.leave();
	}

	bool isExpected(const string& aNick) {
		for(StringIter i = expectedNicks.begin(); i != expectedNicks.end(); i++) {
			if(*i == aNick)
				return true;
		}
		return false;
	}

	void addNick(const string& aNick) {
		for(StringIter i = expectedNicks.begin(); i != expectedNicks.end(); ++i) {
			if(*i == aNick) {
				return;
			}
		}
		expectedNicks.push_back(aNick);
	}
	
	void removeNick(const string& aNick) {
		for(StringIter i = expectedNicks.begin(); i != expectedNicks.end(); ++i) {
			if(*i == aNick) {
				expectedNicks.erase(i);
				return;
			}
		}
	}
private:

	Download::List queue;
	Download::Map running;
	
	static DownloadManager* instance;
	StringList expectedNicks;
	
	DownloadManagerListener::List listeners;
	CriticalSection listenerCS;
	
	UserConnection::List connections;

	void fireDownloadComplete(Download::Ptr aPtr) {
		listenerCS.enter();
		DownloadManagerListener::List tmp = listeners;
		//		dcdebug("fireGotLine %s\n", aLine.c_str());
		for(DownloadManagerListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onDownloadComplete(aPtr);
		}
		listenerCS.leave();
	}
	void fireDownloadFailed(Download::Ptr aPtr, const string& aReason) {
		listenerCS.enter();
		DownloadManagerListener::List tmp = listeners;
		//		dcdebug("fireGotLine %s\n", aLine.c_str());
		for(DownloadManagerListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onDownloadFailed(aPtr, aReason);
		}
		listenerCS.leave();
	}
	void fireDownloadStarting(Download::Ptr aPtr) {
		listenerCS.enter();
		DownloadManagerListener::List tmp = listeners;
		//		dcdebug("fireGotLine %s\n", aLine.c_str());
		for(DownloadManagerListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onDownloadStarting(aPtr);
		}
		listenerCS.leave();
	}
	void fireDownloadTick(Download::Ptr aPtr) {
		listenerCS.enter();
		DownloadManagerListener::List tmp = listeners;
		//		dcdebug("fireGotLine %s\n", aLine.c_str());
		for(DownloadManagerListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onDownloadTick(aPtr);
		}
		listenerCS.leave();
	}
	

	DownloadManager() { };
	virtual ~DownloadManager() {
	};
};

#endif // !defined(AFX_DOWNLOADMANAGER_H__D6409156_58C2_44E9_B63C_B58C884E36A3__INCLUDED_)

/**
 * @file DownloadManger.h
 * $Id: DownloadManager.h,v 1.2 2001/11/26 23:40:36 arnetheduck Exp $
 * @if LOG
 * $Log: DownloadManager.h,v $
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

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
	
};

class UploadManagerListener {
public:
	typedef UploadManagerListener* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;
	
	virtual void onUploadComplete(Upload* aUpload) { };
	virtual void onUploadFailed(Upload* aUpload, const string& aReason) { };
	virtual void onUploadStarting(Upload* aUpload) { };
	virtual void onUploadTick(Upload* aUpload) { };
};

class UploadManager : public UserConnectionListener, public Speaker<UploadManagerListener>
{
public:
	virtual void onBytesSent(UserConnection* aSource, DWORD aBytes) {
		Upload* u;
		uploadCS.enter();
		Upload::MapIter i = uploads.find(aSource);
		if(i == uploads.end()) {
			// Something strange happened?
			dcdebug("onBytesSent: Upload not found???\n");
			uploadCS.leave();
			removeConnection(aSource);
			return;
		}
		u = i->second;
		uploadCS.leave();
		u->addPos(aBytes);
		fireTick(u);
	}

	virtual void onError(UserConnection* aSource, const string& aError) {
		Upload* u;
		aSource->disconnect();
		uploadCS.enter();
		Upload::MapIter i = uploads.find(aSource);
		if(i != uploads.end()) {
			u = i->second;
			fireFailed(u, aError);
			dcdebug("onError: Removing upload\n");
			uploads.erase(i);
			delete u;
		}
		uploadCS.leave();
		removeConnection(aSource);
	}

	/**
	 * Transfer finished, release the Upload and wait for the next command.
	 */
	virtual void onTransmitDone(UserConnection* aSource) {
		Upload * u;
		uploadCS.enter();
		Upload::MapIter i = uploads.find(aSource);
		if(i == uploads.end()) {
			// Something strange happened?
			dcdebug("onTransmitDone: Upload not found???\n");
			
			uploadCS.leave();
			removeConnection(aSource);
			return;
		}
		u = i->second;
		fireComplete(u);
		dcdebug("onTransmitDone: Removing upload\n");
		uploads.erase(i);
		uploadCS.leave();
		delete u;

	}

	virtual void onGet(UserConnection* aSource, const string& aFile, LONGLONG aResume) {
		Upload* u;
		HANDLE h;
		uploadCS.enter();

		try {
			if(uploads.size() >= Settings::getSlots()) {
				aSource->maxedOut();
				removeConnection(aSource);
				uploadCS.leave();
				return;
			}
			string file = ShareManager::getInstance()->translateFileName(aFile);
			Upload::MapIter i = uploads.find(aSource);
			if(i != uploads.end()) {
				// This is bad!
				
				dcdebug("UploadManager::onGet Unexpected command\n");				
				fireFailed(i->second, "Unexpected command");
				delete i->second;
				dcdebug("onGet: Removing upload\n");
				uploads.erase(i);
				uploadCS.leave();
				removeConnection(aSource);
				return;
			} 
			h = CreateFile(file.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
			if(h == INVALID_HANDLE_VALUE) {
				aSource->error("File Not Available");
				uploadCS.leave();
				return;
			}

			u = new Upload();
			u->setFile(h, true);
			u->setPos(aResume, true);
			u->setFileName(aFile);
			u->setUser(aSource->getUser());
			
			char buf[24];
			aSource->fileLength(_i64toa(u->getSize(), buf, 10));
			uploads[aSource] = u;

		} catch (ShareException e) {
			aSource->error("File Not Available");
		} catch(SocketException e) {
			dcdebug("UploadManager::onGet caught: %s\n", e.getError().c_str());
		}

		uploadCS.leave();
	}
	
	virtual void onSend(UserConnection* aSource) {
		Upload* u;
		uploadCS.enter();
		Upload::MapIter i = uploads.find(aSource);
		if(i==uploads.end()) {
			// Huh? Where did this come from?
			removeConnection(aSource);
			uploadCS.leave();
			return;
		}

		u = i->second;
		try {
			aSource->transmitFile(u->getFile());
			fireStarting(u);
		} catch(Exception e) {
			dcdebug("UploadManager::onGet caught: %s\n", e.getError().c_str());
			dcdebug("onSend: Removing upload\n");
			uploads.erase(i);
			delete u;
			removeConnection(aSource);
		}
		uploadCS.leave();
	}
	
	virtual void onGetListLen(UserConnection* aSource) {
		aSource->listLen(ShareManager::getInstance()->getListLenString());
	}

	void addConnection(UserConnection::Ptr conn) {
		conn->addListener(this);
		connections.push_back(conn);
	}

	void removeConnection(UserConnection::Ptr aConn) {
		for(UserConnection::Iter i = connections.begin(); i != connections.end(); ++i) {
			if(*i == aConn) {
				aConn->removeListener(this);
				connections.erase(i);
				ConnectionManager::getInstance()->putUploadConnection(aConn);
				break;
			}
		}
	}

	void removeConnections() {
		UserConnection::Iter i = connections.begin();
		while( i != connections.end()) {
			(*i)->removeListener(this);
			i = connections.erase(i);
			ConnectionManager::getInstance()->putUploadConnection(*i);
		}
	}
	
	static UploadManager* getInstance() {
		dcassert(instance);
		return instance;
	}
	static void newInstance() {
		if(instance)
			delete instance;

		instance = new UploadManager();
	}
	static void deleteInstance() {
		delete instance;
		instance = NULL;
	}
private:
	static UploadManager* instance;
	UserConnection::List connections;
	Upload::Map uploads;
	CriticalSection uploadCS;
	
	UploadManager() { };
	~UploadManager() {
		UserConnection::List tmp = connections;
		uploadCS.enter();
		for(Upload::MapIter j = uploads.begin(); j != uploads.end(); ++j) {
			delete j->second;
		}
		uploadCS.leave();

		removeConnections();
	}

	void fireComplete(Upload::Ptr aPtr) {
		listenerCS.enter();
		UploadManagerListener::List tmp = listeners;
		listenerCS.leave();
		//		dcdebug("fireGotLine %s\n", aLine.c_str());
		for(UploadManagerListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onUploadComplete(aPtr);
		}
	}
	void fireFailed(Upload::Ptr aPtr, const string& aReason) {
		listenerCS.enter();
		UploadManagerListener::List tmp = listeners;
		listenerCS.leave();
		//		dcdebug("fireGotLine %s\n", aLine.c_str());
		for(UploadManagerListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onUploadFailed(aPtr, aReason);
		}
	}
	void fireStarting(Upload::Ptr aPtr) {
		listenerCS.enter();
		UploadManagerListener::List tmp = listeners;
		listenerCS.leave();
		//		dcdebug("fireGotLine %s\n", aLine.c_str());
		for(UploadManagerListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onUploadStarting(aPtr);
		}
	}
	void fireTick(Upload::Ptr aPtr) {
		listenerCS.enter();
		UploadManagerListener::List tmp = listeners;
		listenerCS.leave();
		//		dcdebug("fireGotLine %s\n", aLine.c_str());
		for(UploadManagerListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onUploadTick(aPtr);
		}
	}
};

#endif // !defined(AFX_UPLOADMANAGER_H__B0C67119_3445_4208_B5AA_938D4A019703__INCLUDED_)

/**
 * @file UploadManger.h
 * $Id: UploadManager.h,v 1.12 2001/12/08 20:59:26 arnetheduck Exp $
 * @if LOG
 * $Log: UploadManager.h,v $
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

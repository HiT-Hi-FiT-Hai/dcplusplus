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

class DownloadManagerListener {
	typedef DownloadManagerListener* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;

	onDownloadComplete(const string& aFile);
};

class DownloadManager : public UserConnectionListener
{
public:

	virtual void onDisconnected(UserConnection* aSource) {
		removeConnection(aSource);
	}
	
	virtual void onLock(UserConnection* aSource, const string& aLock, const string& aPk) {
		if(!aSource->hasSentNick())
			aSource->myNick(Settings::getNick());
		if(!aSource->hasSentLock())
			aSource->lock(CryptoManager::getLock(), CryptoManager::getPk());
		
		aSource->direction("Download", "666");
		aSource->key(CryptoManager::makeKey(aLock));
	}
	
	virtual void onKey(UserConnection* aSource, const string& aKey) {
		dcassert(running.find(aSource) != running.end());
		aSource->get(running[aSource]->lastPath + running[aSource]->fileName, 0);
	}
	virtual void onData(UserConnection* aSource, BYTE* aData, int aLen) {
		dcassert(running.find(aSource) != running.end());
		DWORD len;
		Download* d = running[aSource];
		
		WriteFile(d->hFile, aData, aLen, &len, NULL);
		d->pos += len;
	}
	virtual void onFileLength(UserConnection* aSource, const string& aFileLength) {
		dcassert(running.find(aSource) != running.end());
		Download* d = running[aSource];
		d->size = _atoi64(aFileLength.c_str());
		d->pos = 0;
		aSource->setDataMode(d->size);
		aSource->startSend();
	}

	/**
	 * Download finished!
	 */
	virtual void onModeChange(UserConnection* aSource, int aNewMode) {
		Download::MapIter i = running.find(aSource);
		
		Download::Ptr p = i->second;
		running.erase(i);

		CloseHandle(p->hFile);
		if(p->pos != p->size)
			dcdebug("Download incomplete??? : ");

		dcdebug("Download finished: %s to %s, size %i64d\n", p->fileName.c_str(), p->destination.c_str(), p->size);
		delete p;

		checkDownloads(aSource);

	}
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

	boolean isExpected(const string& aNick) {
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

	Download::List queue;
	Download::Map running;
	
	
	static DownloadManager* instance;
	StringList expectedNicks;

	UserConnection::List connections;
		
	DownloadManager() { };
	virtual ~DownloadManager() {
	};
};

#endif // !defined(AFX_DOWNLOADMANAGER_H__D6409156_58C2_44E9_B63C_B58C884E36A3__INCLUDED_)

/**
 * @file DownloadManger.h
 * $Id: DownloadManager.h,v 1.1 2001/11/25 22:06:25 arnetheduck Exp $
 * @if LOG
 * $Log: DownloadManager.h,v $
 * Revision 1.1  2001/11/25 22:06:25  arnetheduck
 * Finally downloading is working! There are now a few quirks and bugs to be fixed
 * but what the heck....!
 *
 * @endif
 */

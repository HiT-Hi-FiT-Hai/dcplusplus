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
#include "CryptoManager.h"

class UploadManager : public UserConnectionListener
{
public:
	virtual void onDisconnected(UserConnection* aSource) {
		removeConnection(aSource);
	}

	virtual void onLock(UserConnection* aSource, const string& aLock, const string& aPk) {
		if(!aSource->hasSentNick())
			aSource->myNick(Settings::getNick());
		if(!aSource->hasSentLock())
			aSource->lock(CryptoManager::getInstance()->getLock(), CryptoManager::getInstance()->getPk());
		
		aSource->direction("Upload", "666");
		aSource->key(CryptoManager::getInstance()->makeKey(aLock));
	}
	
	virtual void onDirection(UserConnection* aSource, const string& aDirection, const string& aNumber) {
		dcassert(aDirection == "Download");
	}
	
	virtual void onGet(UserConnection* aSource, const string& aFile, LONGLONG aResume) {
		aSource->maxedOut();
	}
	
	virtual void onGetListLen(UserConnection* aSource) {
		aSource->listLen("0");
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
				delete aConn;
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
	
	UploadManager() { };
	~UploadManager() {
		UserConnection::List tmp = connections;
		
		for(UserConnection::Iter i = tmp.begin(); i != tmp.end(); ++i) {
			(*i)->disconnect();
		}
	}
};

#endif // !defined(AFX_UPLOADMANAGER_H__B0C67119_3445_4208_B5AA_938D4A019703__INCLUDED_)

/**
 * @file UploadManger.h
 * $Id: UploadManager.h,v 1.2 2001/11/26 23:40:36 arnetheduck Exp $
 * @if LOG
 * $Log: UploadManager.h,v $
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

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

#include "stdafx.h"
#include "DCPlusPlus.h"

#include "ConnectionManager.h"
#include "DownloadManager.h"
#include "UploadManager.h"

#include "UserConnection.h"
#include "CryptoManager.h"
#include "Client.h"

ConnectionManager* ConnectionManager::instance = NULL;

/**
 * Request a connection for downloading.
 * DownloadConnection::addConnection will be called as soon as the connection is ready
 * for downloading.
 * @param aUser The user to connect to.
 * @return The state of the connection sequence (see UserConnection)
 */
int ConnectionManager::getDownloadConnection(User* aUser) {
	UserConnection::NickIter i = downloaders.find(aUser->getNick());
	if(i != downloaders.end()) {
		if(i->second->state == UserConnection::FREE) {
			DownloadManager::getInstance()->addConnection(i->second);
		}
		return i->second->state;
	}

	if(Settings::getConnectionType() == Settings::CONNECTION_ACTIVE) {
		aUser->getClient()->connectToMe(aUser);
	} else {
		aUser->getClient()->revConnectToMe(aUser);
	}

	pendingDown[aUser->getNick()] = aUser;
	return UserConnection::CONNECTING;
}

/**
 * Someone's connecting, accept the connection and wait for identification...
 */
void ConnectionManager::onIncomingConnection() {
	UserConnection::Ptr uc;

	if(pool.size() == 0) {
		uc = new UserConnection();
		uc->addListener(this);
	} else {
		uc = pool.back();
		pool.pop_back();
	}
	try { 
		uc->accept(socket);
	} catch(Exception e) {
		dcdebug("Error creating connection\n");
		delete uc;
	}
}

void ConnectionManager::onMyNick(UserConnection* aSource, const string& aNick) {
	User::NickIter i = pendingDown.find(aNick);

	if(i != pendingDown.end()) {
		if(Settings::getConnectionType() == Settings::CONNECTION_ACTIVE) {
			// We sent a CTM and got an answer, now the other fellow sent his nick.
			// Record it, send own nick and wait for the lock...
			aSource->myNick(Settings::getNick());
			aSource->lock(CryptoManager::getInstance()->getLock(), CryptoManager::getInstance()->getPk());
		} 	
		// In a passive connection, we sent our nick first, and then waited for the
		// other fellow to send his...

		aSource->user = i->second;
		downloaders[aNick] = aSource;
		pendingDown.erase(i);
		aSource->direction("Download", "666");
	} else {
		// We didn't order it so it must be an uploading connection...
			aSource->user = i->second;
			aSource->direction("Upload", "666");
			aSource->flags |= UserConnection::FLAG_UPLOAD;
			uploaders[aNick] = aSource;
	} 
}

void ConnectionManager::onLock(UserConnection* aSource, const string& aLock, const string& aPk) {
	aSource->key(CryptoManager::getInstance()->makeKey(aLock));

	if(aSource->flags & UserConnection::FLAG_UPLOAD) {
		// Pass it to the UploadManager
		UploadManager::getInstance()->addConnection(aSource);
	} else if(Settings::getConnectionType() == Settings::CONNECTION_PASSIVE) {
		// We're done, send this connection to the downloadmanager.
		DownloadManager::getInstance()->addConnection(aSource);
	}
}

void ConnectionManager::onKey(UserConnection* aSource, const string& aKey) {
	if(!(aSource->flags & UserConnection::FLAG_UPLOAD) && Settings::getConnectionType() == Settings::CONNECTION_ACTIVE) {
		// We're done, send this connection to the downloadmanager.
		DownloadManager::getInstance()->addConnection(aSource);
	}
}

void ConnectionManager::onConnected(UserConnection* aSource) {
	aSource->myNick(Settings::getNick());
	aSource->lock(CryptoManager::getInstance()->getLock(), CryptoManager::getInstance()->getPk());
}

void ConnectionManager::connect(const string& aServer, short aPort) {
	UserConnection* c = new UserConnection();
	addConnection(c);
	c->connect(aServer, aPort);
}

/**
 * @file IncomingManger.cpp
 * $Id: ConnectionManager.cpp,v 1.4 2001/12/02 11:16:46 arnetheduck Exp $
 * @if LOG
 * $Log: ConnectionManager.cpp,v $
 * Revision 1.4  2001/12/02 11:16:46  arnetheduck
 * Optimised hub listing, removed a few bugs and leaks, and added a few small
 * things...downloads are now working, time to start writing the sharing
 * code...
 *
 * Revision 1.3  2001/12/01 17:15:03  arnetheduck
 * Added a crappy version of huffman encoding, and some other minor changes...
 *
 * Revision 1.2  2001/11/29 19:10:54  arnetheduck
 * Refactored down/uploading and some other things completely.
 * Also added download indicators and download resuming, along
 * with some other stuff.
 *
 * Revision 1.1  2001/11/27 20:29:37  arnetheduck
 * Renamed from ConnectionManager
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

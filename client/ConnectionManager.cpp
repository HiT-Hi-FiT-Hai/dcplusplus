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
 * @return The state of the connection sequence (UserConnection::CONNECTING if a connection is being made, otherwise
 * there's probably already a connection in progress).
 */
int ConnectionManager::getDownloadConnection(User* aUser) {
	downloaderCS.enter();
	UserConnection::NickIter i = downloaders.find(aUser->getNick());

	if(i != downloaders.end()) {
		UserConnection* u = i->second;
		if(u->state == UserConnection::FREE) {
			// Good, we found a free connection, use it!
			u->state = UserConnection::BUSY;
			DownloadManager::getInstance()->addConnection(i->second);
			downloaderCS.leave();		
			
			return UserConnection::CONNECTING;
		}
		// Already connected to this user...		
		downloaderCS.leave();		
		return UserConnection::BUSY;

	} else if(pendingDown.find(aUser->getNick()) != pendingDown.end()) {
		// Already trying to connect to this user...
		downloaderCS.leave();		
		return UserConnection::BUSY;
	}

	downloaderCS.leave();		
	
	// Alright, set up a new connection attempt.
	try {
		if(Settings::getConnectionType() == Settings::CONNECTION_ACTIVE) {
		} else {
			aUser->getClient()->revConnectToMe(aUser);
		}
	} catch(Exception e) {
		dcdebug("ConnectionManager::getDownloadConnection: Caught: %s\n", e.getError().c_str());
		return UserConnection::BUSY;
	}
	
	// Add to the list of pending downloads...
	pendingDown[aUser->getNick()] = aUser;
	return UserConnection::CONNECTING;
}

/**
 * Someone's connecting, accept the connection and wait for identification...
 * It's always the other fellow that starts sending if he made the connection.
 */
void ConnectionManager::onIncomingConnection() {
	UserConnection* uc = getConnection();

	try { 
		uc->accept(socket);
		uc->state = UserConnection::LOGIN;
	} catch(Exception e) {
		dcdebug("Error creating connection\n");
		uc->disconnect();
		pool.push_back(uc);
	}
}

/**
 * Nick received. If it's a downloader, fine, otherwise it must be an uploader.
 */
void ConnectionManager::onMyNick(UserConnection* aSource, const string& aNick) {
	User::NickIter i = pendingDown.find(aNick);

	if(i != pendingDown.end()) {
		if(Settings::getConnectionType() == Settings::CONNECTION_ACTIVE) {
			// We sent a CTM and got an answer, now the other fellow sent his nick.
			// Record it, send own nick and wait for the lock...
			try {
				aSource->myNick(Settings::getNick());
				aSource->lock(CryptoManager::getInstance()->getLock(), CryptoManager::getInstance()->getPk());
			} catch(Exception e) {
				putConnection(aSource);
			}
		} 	
		// In a passive connection, we sent our nick first, and then waited for the
		// other fellow to send his...

		aSource->user = i->second;
		downloaderCS.enter();
		downloaders[aNick] = aSource;
		downloaderCS.leave();

		pendingDown.erase(i);
		try {
			aSource->direction("Download", "666");
		} catch(Exception e) {
			putConnection(aSource);
		}
	
	} else {
		// We didn't order it so it must be an uploading connection...
		// Make sure we know who it is, i e that he/she is connected...
		aSource->user = Client::findUser(aNick);
		if(aSource->user == NULL) {
			// Duuuh...this is bad...abort, abort, abort! (Disconnect and replace the connection on the pool)
			putConnection(aSource);
		}
		
		try {
			aSource->direction("Upload", "666");
		} catch(Exception e) {
			putConnection(aSource);
		}
	
		aSource->flags |= UserConnection::FLAG_UPLOAD;
		uploaderCS.enter();
		uploaders[aNick] = aSource;
		uploaderCS.leave();
	} 
}

void ConnectionManager::onLock(UserConnection* aSource, const string& aLock, const string& aPk) {

	try {
		aSource->key(CryptoManager::getInstance()->makeKey(aLock));
	} catch(Exception e) {
		putConnection(aSource);
	}
	
	if(aSource->flags & UserConnection::FLAG_UPLOAD) {
		// Pass it to the UploadManager
		aSource->state = UserConnection::BUSY;
		UploadManager::getInstance()->addConnection(aSource);
	} else if(Settings::getConnectionType() == Settings::CONNECTION_PASSIVE) {
		// We're done, send this connection to the downloadmanager.
		aSource->state = UserConnection::BUSY;
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
	try {
		aSource->myNick(Settings::getNick());
		aSource->lock(CryptoManager::getInstance()->getLock(), CryptoManager::getInstance()->getPk());
	} catch(Exception e) {
		putConnection(aSource);
	}
}

void ConnectionManager::connect(const string& aServer, short aPort) {
	UserConnection* c = getConnection();
	c->state = UserConnection::LOGIN;

	try { 
		c->connect(aServer, aPort);
	} catch(Exception e) {
		putConnection(c);
	}
}

/**
 * @file IncomingManger.cpp
 * $Id: ConnectionManager.cpp,v 1.5 2001/12/04 21:50:34 arnetheduck Exp $
 * @if LOG
 * $Log: ConnectionManager.cpp,v $
 * Revision 1.5  2001/12/04 21:50:34  arnetheduck
 * Work done towards application stability...still a lot to do though...
 * a bit more and it's time for a new release.
 *
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

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
#include "ClientManager.h"

ConnectionManager* ConnectionManager::instance = NULL;

/**
 * Request a connection for downloading.
 * DownloadConnection::addConnection will be called as soon as the connection is ready
 * for downloading.
 * @param aUser The user to connect to.
 * @return The state of the connection sequence (UserConnection::CONNECTING if a connection is being made, otherwise
 * there's probably already a connection in progress).
 */
int ConnectionManager::getDownloadConnection(const User::Ptr& aUser) {
	cs.enter();

	if( pendingDown.find(aUser) != pendingDown.end() ) {
		cs.leave();
		return UserConnection::BUSY;
	}

	// Check the downloader's pool
	for(UserConnection::Iter i = downPool.begin(); i != downPool.end(); ++i) {
		if((*i)->getUser() == aUser) {
			// Bingo!
			UserConnection* u = *i;
			dcdebug("ConnectionManager::getDownloadConnection Found connection to %s in active pool\n", u->getUser()->getNick().c_str());
			downPool.erase(i);
			cs.leave();
			DownloadManager::getInstance()->addConnection(u);
			return UserConnection::FREE;
		}
	}
	// Alright, set up a new connection attempt.
	try {
		if(SETTING(CONNECTION_TYPE) == SettingsManager::CONNECTION_ACTIVE) {
			aUser->getClient()->connectToMe(aUser);
		} else {
			aUser->getClient()->revConnectToMe(aUser);
		}
	} catch(Exception e) {
		dcdebug("ConnectionManager::getDownloadConnection: Caught: %s\n", e.getError().c_str());
		cs.leave();
		
		return UserConnection::BUSY;
	}
	
	// Add to the list of pending downloads...
	pendingDown[aUser] = TimerManager::getTick();
	cs.leave();
	return UserConnection::CONNECTING;
}

void ConnectionManager::putDownloadConnection(UserConnection* aSource, bool reuse /* = false */) {
	cs.enter();
	UserConnection::Iter i = find(downloaders.begin(), downloaders.end(), aSource);
	if(i != downloaders.end()) {
		downloaders.erase(i);
	}
	// Pool it for later usage...
	if(reuse) {
		if(find(downPool.begin(), downPool.end(), aSource) == downPool.end()) {
			dcdebug("ConnectionManager::putDownloadConnection Pooing reusable connection to %s\n", aSource->getUser()->getNick().c_str());
			
			aSource->addListener(this);
			downPool.push_back(aSource);
		}
		cs.leave();
	} else {
		cs.leave();		
		putConnection(aSource);
	}
}

void ConnectionManager::onAction(TimerManagerListener::Types type, DWORD aTick) {
	if(type == TimerManagerListener::SECOND) {
		cs.enter();
		map<User::Ptr, DWORD>::iterator i = pendingDown.begin();
		
		while(i != pendingDown.end()) {
			if((i->second + 30*1000) < aTick) {
				// Haven't connected for a long, long time...
				DownloadManager::getInstance()->connectFailed(i->first);
				
				i = pendingDown.erase(i);
			} else {
				++i;
			}
		}
		cs.leave();
	}
}

/**
 * Someone's connecting, accept the connection and wait for identification...
 * It's always the other fellow that starts sending if he made the connection.
 */
void ConnectionManager::onIncomingConnection() throw() {
	UserConnection* uc = getConnection();

	try { 
		uc->accept(socket);
		uc->flags |= UserConnection::FLAG_INCOMING;
		uc->state = UserConnection::LOGIN;

		//		uc->myNick(SETTING(NICK));
		//		uc->lock(CryptoManager::getInstance()->getLock(), CryptoManager::getInstance()->getPk());
	} catch(Exception e) {
		dcdebug("ConnectionManager::OnIncomingConnection caught: %s\n", e.getError().c_str());
		putConnection(uc);
	}
}

/**
 * Nick received. If it's a downloader, fine, otherwise it must be an uploader.
 */
void ConnectionManager::onMyNick(UserConnection* aSource, const string& aNick) throw() {
	dcdebug("ConnectionManager::onMyNick %s\n", aNick.c_str());
	cs.enter();

	for(map<User::Ptr, DWORD>::iterator i = pendingDown.begin(); i != pendingDown.end(); ++i) {
		if(i->first->getNick() == aNick) {
			aSource->user = i->first;
			break;
		}
	}

	if(aSource->user) {
		if(aSource->user->getClient()) {
			aSource->myNick(aSource->user->getClient()->getNick());
		} else {
			aSource->myNick(SETTING(NICK));
		}
		aSource->lock(CryptoManager::getInstance()->getLock(), CryptoManager::getInstance()->getPk());
		
		aSource->flags |= UserConnection::FLAG_DOWNLOAD;
		
		downloaders.push_back(aSource);
		if(i != pendingDown.end()) {
			pendingDown.erase(i);
		}

	} else {
		// We didn't order it so it must be an uploading connection...
		// Make sure we know who it is, i e that he/she is connected...
		aSource->user = ClientManager::getInstance()->findUser(aNick);
		if(!aSource->user) {
			putConnection(aSource);
			cs.leave();
			return;
		}
		
		if(aSource->user->getClient()) {
			aSource->myNick(aSource->user->getClient()->getNick());
		} else {
			aSource->myNick(SETTING(NICK));
		}
		aSource->lock(CryptoManager::getInstance()->getLock(), CryptoManager::getInstance()->getPk());

		aSource->flags |= UserConnection::FLAG_UPLOAD;

		uploaders.push_back(aSource);
	} 
	cs.leave();
}

void ConnectionManager::onLock(UserConnection* aSource, const string& aLock, const string& aPk) throw() {
	try {
		if(aLock == CryptoManager::getInstance()->getLock()) {
			// Alright, we have an extended protocol, set a user flag for this user and refresh his info...
			if( (aPk.find("DCPLUSPLUS") != string::npos) && aSource->getUser()) {
				aSource->getUser()->setFlag(User::DCPLUSPLUS);

				if(aSource->getUser()->getClient()) {
					aSource->getUser()->getClient()->getInfo(aSource->getUser());
				}
			}
		}
		aSource->direction(aSource->getDirectionString(), "666");
		aSource->key(CryptoManager::getInstance()->makeKey(aLock));
	} catch(SocketException e) {
		dcdebug("ConnectionManager::onLock caught: %s\n", e.getError().c_str());
		putConnection(aSource);
		return;
	}
				
}

void ConnectionManager::onKey(UserConnection* aSource, const string& aKey) throw() {
	// We don't want any messages while the Up/DownloadManagers are working...
	aSource->removeListener(this);
	aSource->state = UserConnection::BUSY;
	
	if(aSource->flags & UserConnection::FLAG_DOWNLOAD) {
		dcdebug("ConnectionManager::onKey, leaving to downloadmanager\n");
		DownloadManager::getInstance()->addConnection(aSource);
	} else {
		dcassert(aSource->flags & UserConnection::FLAG_UPLOAD);
		UploadManager::getInstance()->addConnection(aSource);
	}
}

void ConnectionManager::onConnected(UserConnection* aSource) throw() {
	try {
		aSource->myNick(aSource->getNick());
		aSource->lock(CryptoManager::getInstance()->getLock(), CryptoManager::getInstance()->getPk());
	} catch(Exception e) {
		dcdebug("ConnectionManager::onConnected caught: %s\n", e.getError().c_str());
		putConnection(aSource);
	}
}

void ConnectionManager::connect(const string& aServer, short aPort, const string& aNick) {
	UserConnection* c = getConnection();
	c->setNick(aNick);
	c->state = UserConnection::LOGIN;

	try { 
		c->connect(aServer, aPort);
	} catch(Exception e) {
		dcdebug("ConnectionManager::connect caught: %s\n", e.getError().c_str());
		putConnection(c);
	}
}

void ConnectionManager::onFailed(UserConnection* aSource, const string& aError) throw() {
	if(aSource->flags & UserConnection::FLAG_DOWNLOAD) {
		cs.enter();
		UserConnection::Iter i = find(downPool.begin(), downPool.end(), aSource);
		if(i != downPool.end()) {
			dcdebug("ConnectionManager::onError Removing connection to %s from active pool\n", aSource->getUser()->getNick().c_str());
			downPool.erase(i);
		}
		cs.leave();
		putDownloadConnection(aSource);
	} else if(aSource->flags & UserConnection::FLAG_UPLOAD) {
		putUploadConnection(aSource);
	} else {
		putConnection(aSource);
	}
}

void ConnectionManager::updateUser(UserConnection* aConn) {
	dcassert(!aConn->getUser()->isOnline());
	
	User::Ptr& p = ClientManager::getInstance()->findUser(aConn->getUser()->getNick());
	if(p) {
		aConn->setUser(p);
	}
	
}

/**
 * @file IncomingManger.cpp
 * $Id: ConnectionManager.cpp,v 1.20 2002/01/14 22:19:43 arnetheduck Exp $
 * @if LOG
 * $Log: ConnectionManager.cpp,v $
 * Revision 1.20  2002/01/14 22:19:43  arnetheduck
 * Commiting minor bugfixes
 *
 * Revision 1.19  2002/01/14 01:56:33  arnetheduck
 * All done for 0.12
 *
 * Revision 1.18  2002/01/13 22:50:47  arnetheduck
 * Time for 0.12, added favorites, a bunch of new icons and lot's of other stuff
 *
 * Revision 1.17  2002/01/11 14:52:56  arnetheduck
 * Huge changes in the listener code, replaced most of it with templates,
 * also moved the getinstance stuff for the managers to a template
 *
 * Revision 1.16  2002/01/07 20:17:59  arnetheduck
 * Finally fixed the reconnect bug that's been annoying me for a whole day...
 * Hopefully the app works better in w95 now too...
 *
 * Revision 1.15  2002/01/05 10:13:39  arnetheduck
 * Automatic version detection and some other updates
 *
 * Revision 1.14  2002/01/02 16:12:32  arnetheduck
 * Added code for multiple download sources
 *
 * Revision 1.13  2001/12/21 20:21:17  arnetheduck
 * Private messaging added, and a lot of other updates as well...
 *
 * Revision 1.12  2001/12/16 19:47:48  arnetheduck
 * Reworked downloading and user handling some, and changed some small UI things
 *
 * Revision 1.11  2001/12/15 17:01:06  arnetheduck
 * Passive mode searching as well as some searching code added
 *
 * Revision 1.10  2001/12/13 19:21:57  arnetheduck
 * A lot of work done almost everywhere, mainly towards a friendlier UI
 * and less bugs...time to release 0.06...
 *
 * Revision 1.9  2001/12/10 10:48:40  arnetheduck
 * Ahh, finally found one bug that's been annoying me for days...=) the connections
 * in the pool were not reset correctly before being put back for later use...
 *
 * Revision 1.8  2001/12/08 20:59:26  arnetheduck
 * Fixing bugs...
 *
 * Revision 1.7  2001/12/07 20:03:04  arnetheduck
 * More work done towards application stability
 *
 * Revision 1.6  2001/12/05 14:27:35  arnetheduck
 * Premature disconnection bugs removed.
 *
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

/* 
 * Copyright (C) 2001 Jacek Sieka, j_s@telia.com
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
 */
void ConnectionManager::getDownloadConnection(const User::Ptr& aUser) {
	dcassert((bool)aUser);
	ConnectionQueueItem* cqi = NULL;
	{
		Lock l(cs);

		for(ConnectionQueueItem::QueueIter j = downPool.begin(); j != downPool.end(); ++j) {
			if(j->first->getUser() == aUser) {
				UserConnection* uc = j->first;
				uc->setStatus(UserConnection::BUSY);
				pendingAdd.push_back(j->second);
				downPool.erase(j);
				return;
			}
		}
		
		for(ConnectionQueueItem::TimeIter i = pendingDown.begin(); i != pendingDown.end(); ++i) {
			if(i->first->getUser() == aUser) {
				return;
			}
		}

		for(ConnectionQueueItem::QueueIter k = connections.begin(); k != connections.end(); ++k) {
			if(k->first->getUser() == aUser && k->first->isSet(UserConnection::FLAG_DOWNLOAD)) {
				return;
			}
		}
		
		// Add it to the pending...
		cqi = new ConnectionQueueItem(aUser);
		
		pendingDown[cqi] = 0;

		fire(ConnectionManagerListener::ADDED, cqi);
	}
}

void ConnectionManager::putDownloadConnection(UserConnection* aSource, bool reuse /* = false */) {
	// Pool it for later usage...
	if(reuse) {
		aSource->addListener(this);
		{
			Lock l(cs);
			dcassert(downPool.find(aSource) == downPool.end());
			dcassert(connections.find(aSource) != connections.end());
			
			ConnectionQueueItem::QueueIter i = connections.find(aSource);
			aSource->setStatus(UserConnection::IDLE);
			downPool[aSource] = i->second;
			aSource->setLastActivity(TimerManager::getTick());
		}
		dcdebug("ConnectionManager::putDownloadConnection Pooing reusable connection %p to %s\n", aSource, aSource->getUser()->getNick().c_str());
		
	} else {
		if(aSource->getUser()) {
			if(QueueManager::getInstance()->hasDownload(aSource->getUser())) {
				aSource->removeListeners();
				aSource->disconnect();
				Lock l(cs);
				
				ConnectionQueueItem::QueueIter i = connections.find(aSource);
				if(i != connections.end()) {
					i->second->setConnection(NULL);
					i->second->setStatus(ConnectionQueueItem::WAITING);
					dcassert(pendingDown.find(i->second) == pendingDown.end());
					pendingDown[i->second] = TimerManager::getTick();
					connections.erase(i);
				}

				dcassert(find(pendingDelete.begin(), pendingDelete.end(), aSource) == pendingDelete.end());
				dcassert(find(userConnections.begin(), userConnections.end(), aSource) != userConnections.end());
				userConnections.erase(find(userConnections.begin(), userConnections.end(), aSource));
				pendingDelete.push_back(aSource);
				return;
			}
			
		}
		putConnection(aSource);
	}
}

void ConnectionManager::putConnection(UserConnection* aConn) {
	aConn->removeListeners();
	aConn->disconnect();
	ConnectionQueueItem* cqi = NULL;
	{
		Lock l(cs);
		
		ConnectionQueueItem::QueueIter i = connections.find(aConn);
		if(i != connections.end()) {
			cqi = i->second;
			connections.erase(i);
		}
		dcassert(find(pendingDelete.begin(), pendingDelete.end(), aConn) == pendingDelete.end());
		dcassert(find(userConnections.begin(), userConnections.end(), aConn) != userConnections.end());
		userConnections.erase(find(userConnections.begin(), userConnections.end(), aConn));
		pendingDelete.push_back(aConn);
	}
	if(cqi) {
		fire(ConnectionManagerListener::REMOVED, cqi);
		delete cqi;
	}
}

void ConnectionManager::onTimerSecond(DWORD aTick) {
	UserConnection::List removed;
	ConnectionQueueItem::List add;
	ConnectionQueueItem::List remove;
	ConnectionQueueItem::List failPassive;
	ConnectionQueueItem::List connecting;
	int attempts = 0;

	{
		Lock l(cs);
		add = pendingAdd;
		pendingAdd.clear();

		removed = pendingDelete;
		pendingDelete.clear();

		ConnectionQueueItem::TimeIter i = pendingDown.begin();
		
		while(i != pendingDown.end()) {
			ConnectionQueueItem* cqi = i->first;

			if(!cqi->getUser()->isOnline()) {
				// Not online anymore...remove him from the pending...
				cqi->setUser(ClientManager::getInstance()->findUser(cqi->getUser()->getNick()));
				if(!cqi->getUser()) {
					pendingDown.erase(i++);
					remove.push_back(cqi);
				} else {
					++i;
				}
				continue;
			}

			if( ((i->second + 60*1000) < aTick) ) {
				if((attempts <= 3) ) {
					// Nothing's happened for 60 seconds, try again...
					if(!QueueManager::getInstance()->hasDownload(cqi->getUser())) {
						pendingDown.erase(i++);
						// i = pendingDown.erase(i);	// This works with MSVC++ STL 
						remove.push_back(cqi);
						continue;
					}
					i->second = aTick;

					if(cqi->getUser()->isSet(User::PASSIVE) && SETTING(CONNECTION_TYPE) == SettingsManager::CONNECTION_PASSIVE) {
						failPassive.push_back(cqi);
					} else {
						cqi->getUser()->connect();
						connecting.push_back(cqi);
						attempts++;
						cqi->setStatus(ConnectionQueueItem::CONNECTING);
					}
				}
			} else if(((i->second + 50*1000) < aTick) && cqi->getStatus() == ConnectionQueueItem::CONNECTING) {
				fire(ConnectionManagerListener::FAILED, cqi, "Connection Timeout");
				cqi->setStatus(ConnectionQueueItem::WAITING);
			}

			++i;
		}
	}

	for(UserConnection::Iter j = removed.begin(); j != removed.end(); ++j) {
		dcdebug("UserConnection %p deleted\n", *j);
		delete *j;
	}

	for(ConnectionQueueItem::Iter k = add.begin(); k != add.end(); ++k) {
		fire(ConnectionManagerListener::STATUS_CHANGED, *k);
		DownloadManager::getInstance()->addConnection((*k)->getConnection());
	}

	for(ConnectionQueueItem::Iter l = remove.begin(); l != remove.end(); ++l) {
		fire(ConnectionManagerListener::REMOVED, *l);
		delete *l;
	}
	for(ConnectionQueueItem::Iter m = failPassive.begin(); m != failPassive.end(); ++m) {
		fire(ConnectionManagerListener::FAILED, *m, "Can't connect to passive user while in passive mode");
	}
	for(ConnectionQueueItem::Iter n = connecting.begin(); n != connecting.end(); ++n) {
		fire(ConnectionManagerListener::STATUS_CHANGED, *n);
	}
	
}

void ConnectionManager::onTimerMinute(DWORD aTick) {
	Lock l(cs);
	for(UserConnection::Iter j = userConnections.begin(); j != userConnections.end(); ++j) {
		if(((*j)->getLastActivity() + 180*1000) < aTick) {
			(*j)->disconnect();
		}
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
		uc->setFlag(UserConnection::FLAG_INCOMING);
		uc->setStatus(UserConnection::CONNECTING);
		uc->setState(UserConnection::STATE_NICK);
		
	} catch(Exception e) {
		dcdebug("ConnectionManager::OnIncomingConnection caught: %s\n", e.getError().c_str());
		putConnection(uc);
	}
}

/**
 * Nick received. If it's a downloader, fine, otherwise it must be an uploader.
 */
void ConnectionManager::onMyNick(UserConnection* aSource, const string& aNick) throw() {

	if(aSource->getState() != UserConnection::STATE_NICK) {
		// Already got this once, ignore...
		dcdebug("CM::onMyNick %p sent nick twice\n", aSource);
		return;
	}

	dcassert(aNick.size() > 0);
	dcdebug("ConnectionManager::onMyNick %p, %s\n", aSource, aNick.c_str());
	dcassert(!aSource->getUser());
	
	ConnectionQueueItem* cqi = NULL;
	{
		Lock l(cs);
		
		for(ConnectionQueueItem::TimeIter i = pendingDown.begin(); i != pendingDown.end(); ++i) {
			cqi = i->first;
			if(cqi->getUser()->getNick() == aNick) {
				aSource->setUser(cqi->getUser());
				cqi->setConnection(aSource);
				pendingDown.erase(i);
				connections[aSource] = cqi;
				break;
			}
		}
	}

	if( !aSource->isSet(UserConnection::FLAG_UPLOAD) && aSource->user) {
		aSource->setFlag(UserConnection::FLAG_DOWNLOAD);
		fire(ConnectionManagerListener::CONNECTED, cqi);
	} else {

		// We didn't order it so it must be an uploading connection...
		// Make sure we know who it is, i e that he/she is connected...
		aSource->user = ClientManager::getInstance()->findUser(aNick);
		if(!aSource->user) {
			dcdebug("CM::onMyNick Incoming connection from unknown user %s\n", aNick.c_str());
			putConnection(aSource);
			return;
		}
		aSource->setFlag(UserConnection::FLAG_UPLOAD);
		cqi = new ConnectionQueueItem(aSource->getUser());
		cqi->setConnection(aSource);
		{
			Lock l(cs);
			connections[aSource] = cqi;
		}
		fire(ConnectionManagerListener::ADDED, cqi);
		fire(ConnectionManagerListener::CONNECTED, cqi);
	} 

	if( aSource->isSet(UserConnection::FLAG_INCOMING) ) {
		aSource->myNick(aSource->getUser()->getClientNick()); 
		aSource->lock(CryptoManager::getInstance()->getLock(), CryptoManager::getInstance()->getPk());
	}

	aSource->setState(UserConnection::STATE_LOCK);
}

void ConnectionManager::onLock(UserConnection* aSource, const string& aLock, const string& aPk) throw() {
	if(aSource->getState() != UserConnection::STATE_LOCK) {
		dcdebug("CM::onLock %p received lock twice, ignoring\n", aSource);
		return;
	}

	if(aLock == CryptoManager::getInstance()->getLock()) {
		// Alright, we have an extended protocol, set a user flag for this user and refresh his info...
		if( (aPk.find("DCPLUSPLUS") != string::npos) && aSource->getUser()) {
			aSource->getUser()->setFlag(User::DCPLUSPLUS);
			User::updated(aSource->getUser());
		}
	}
	aSource->setState(UserConnection::STATE_KEY);
	aSource->direction(aSource->getDirectionString(), "666");
	aSource->key(CryptoManager::getInstance()->makeKey(aLock));
}

void ConnectionManager::onKey(UserConnection* aSource, const string&/* aKey*/) throw() {
	if(aSource->getState() != UserConnection::STATE_KEY) {
		dcdebug("CM::onKey Bad state, ignoring");
		return;
	}
	// We don't want any messages while the Up/DownloadManagers are working...
	aSource->removeListener(this);
	aSource->setStatus(UserConnection::BUSY);
	ConnectionQueueItem cqi = NULL;
	{
		Lock l(cs);
		ConnectionQueueItem::QueueIter i = connections.find(aSource);
		if(i == connections.end()) {
			dcdebug("Connection not found in connections QueueMap\n");
			putConnection(aSource);
			return;
		}
	}
	
	if(aSource->isSet(UserConnection::FLAG_DOWNLOAD)) {
		if(aSource->isSet(UserConnection::FLAG_UPLOAD)) {
			// Oops, not good...
			dcdebug("ConnectionManager::onKey, bad download connection\n");
			
			putDownloadConnection(aSource);
			return;
		}
		if(!aSource->getUser()) {
			// We still don't know who this is!!!
			putDownloadConnection(aSource);
			return;
		} else {
			dcdebug("ConnectionManager::onKey, leaving to downloadmanager\n");
			DownloadManager::getInstance()->addConnection(aSource);
		}
	} else {
		dcassert(aSource->isSet(UserConnection::FLAG_UPLOAD));

		if(aSource->isSet(UserConnection::FLAG_DOWNLOAD)) {
			// Oops, not good...
			dcdebug("ConnectionManager::onKey, bad upload connection\n");
			putDownloadConnection(aSource);
			return;
		}
		
		dcdebug("ConnectionManager::onKey, leaving to uploadmanager\n");
		if(!aSource->getUser()) {
			// We still don't know who this is!!!
			putDownloadConnection(aSource);
		} else {
			UploadManager::getInstance()->addConnection(aSource);
		}
	}
}

void ConnectionManager::onConnected(UserConnection* aSource) throw() {
	dcassert(aSource->getState() == UserConnection::STATE_CONNECT);
	aSource->myNick(aSource->getNick());
	aSource->lock(CryptoManager::getInstance()->getLock(), CryptoManager::getInstance()->getPk());
	aSource->setState(UserConnection::STATE_NICK);
}

void ConnectionManager::connect(const string& aServer, short aPort, const string& aNick) {
	UserConnection* c = getConnection();
	c->setNick(aNick);
	c->setStatus(UserConnection::CONNECTING);
	c->setState(UserConnection::STATE_CONNECT);
	c->connect(aServer, aPort);

}

void ConnectionManager::onFailed(UserConnection* aSource, const string& /*aError*/) throw() {
	if(aSource->isSet(UserConnection::FLAG_DOWNLOAD)) {
		{
			Lock l(cs);
			
			ConnectionQueueItem::QueueIter i = downPool.find(aSource);
			if(i != downPool.end()) {
				dcdebug("ConnectionManager::onError Removing connection %p to %s from active pool\n", aSource, aSource->getUser()->getNick().c_str());
				downPool.erase(i);
			}
		}
		putDownloadConnection(aSource);
	} else if(aSource->isSet(UserConnection::FLAG_UPLOAD)) {
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

void ConnectionManager::onDirection(UserConnection* aSource, const string& dir, const string& /*num*/) throw() {
	if(dir == "Upload") {
		aSource->setFlag(UserConnection::FLAG_DOWNLOAD);
	} else {
		dcassert(dir == "Download");
		aSource->setFlag(UserConnection::FLAG_UPLOAD);
	}
}

void ConnectionManager::removeConnection(ConnectionQueueItem* aCqi) {

	bool found = false;
	{
		Lock l(cs);

		if(pendingDown.find(aCqi) != pendingDown.end()) {
			pendingDown.erase(aCqi);
			found = true;
			fire(ConnectionManagerListener::REMOVED, aCqi);
			delete aCqi;
		} else {
			ConnectionQueueItem::QueueIter i;
			for(i = connections.begin(); i != connections.end(); ++i) {
				if(i->second == aCqi)
					break;
			}

			if(i!=connections.end()) {
				found = true;
				dcassert(aCqi->getConnection());
				aCqi->getConnection()->disconnect();
			} else {
				dcdebug("ConnectionManager::removeConnection: %p not found\n", aCqi);
			}
		}
	}
}

/**
 * @file IncomingManger.cpp
 * $Id: ConnectionManager.cpp,v 1.32 2002/02/26 23:25:22 arnetheduck Exp $
 * @if LOG
 * $Log: ConnectionManager.cpp,v $
 * Revision 1.32  2002/02/26 23:25:22  arnetheduck
 * Minor updates and fixes
 *
 * Revision 1.31  2002/02/25 15:39:28  arnetheduck
 * Release 0.154, lot of things fixed...
 *
 * Revision 1.30  2002/02/18 23:48:32  arnetheduck
 * New prerelease, bugs fixed and features added...
 *
 * Revision 1.29  2002/02/12 00:35:37  arnetheduck
 * 0.153
 *
 * Revision 1.28  2002/02/09 18:13:51  arnetheduck
 * Fixed level 4 warnings and started using new stl
 *
 * Revision 1.27  2002/02/07 17:25:28  arnetheduck
 * many bugs fixed, time for 0.152 I think
 *
 * Revision 1.26  2002/02/04 01:10:29  arnetheduck
 * Release 0.151...a lot of things fixed
 *
 * Revision 1.25  2002/02/03 01:06:56  arnetheduck
 * More bugfixes and some minor changes
 *
 * Revision 1.24  2002/02/01 02:00:24  arnetheduck
 * A lot of work done on the new queue manager, hopefully this should reduce
 * the number of crashes...
 *
 * Revision 1.23  2002/01/26 16:34:00  arnetheduck
 * Colors dialog added, as well as some other options
 *
 * Revision 1.22  2002/01/20 22:54:46  arnetheduck
 * Bugfixes to 0.131 mainly...
 *
 * Revision 1.21  2002/01/17 23:35:59  arnetheduck
 * Reworked threading once more, now it actually seems stable. Also made
 * sure that noone tries to access client objects that have been deleted
 * as well as some other minor updates
 *
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

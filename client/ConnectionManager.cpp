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

#include "stdinc.h"
#include "DCPlusPlus.h"

#include "ConnectionManager.h"
#include "DownloadManager.h"
#include "UploadManager.h"

#include "UserConnection.h"
#include "CryptoManager.h"
#include "ClientManager.h"
#include "QueueManager.h"

#include "ResourceManager.h"

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

		for(ConnectionQueueItem::Iter j = downPool.begin(); j != downPool.end(); ++j) {
			if((*j)->getUser() == aUser) {
				dcassert((*j)->getConnection());
				
				pendingAdd.push_back(*j);
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
			
			ConnectionQueueItem::QueueIter i = connections.find(aSource);
			dcassert(i != connections.end());
			downPool.push_back(i->second);
			aSource->setLastActivity(GET_TICK());
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
					pendingDown[i->second] = GET_TICK();
					connections.erase(i);
				}

				dcassert(find(userConnections.begin(), userConnections.end(), aSource) != userConnections.end());
				userConnections.erase(find(userConnections.begin(), userConnections.end(), aSource));
				delete aSource;
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
	ConnectionQueueItem::List add;
	ConnectionQueueItem::List remove;
	ConnectionQueueItem::List failPassive;
	ConnectionQueueItem::List connecting;

	int attempts = 0;

	{
		Lock l(cs);
		add = pendingAdd;
		pendingAdd.clear();

		{
			for(UserConnection::Iter i = pendingDelete.begin(); i != pendingDelete.end(); ++i) {
				delete *i;
			}
		}
		pendingDelete.clear();

		ConnectionQueueItem::TimeIter i = pendingDown.begin();
		bool startDown = true;
		
		if( ((SETTING(DOWNLOAD_SLOTS) != 0) && DownloadManager::getInstance()->getDownloads() >= SETTING(DOWNLOAD_SLOTS)) ||
			((SETTING(MAX_DOWNLOAD_SPEED) != 0 && DownloadManager::getInstance()->getAverageSpeed() >= (SETTING(MAX_DOWNLOAD_SPEED)*1024)) ) ) {
			startDown = false;
		}
		
		while(i != pendingDown.end()) {
			ConnectionQueueItem* cqi = i->first;
			dcassert(cqi->getUser());

			if(!cqi->getUser()->isOnline()) {
				// Not online anymore...remove him from the pending...
				pendingDown.erase(i++);
				remove.push_back(cqi);
				continue;
			}

			if( ((i->second + 60*1000) < aTick) ) {

				if(startDown) {
					if( attempts <= 3 ) {
						// Nothing's happened for 60 seconds, try again...
						if(!QueueManager::getInstance()->hasDownload(cqi->getUser())) {
							pendingDown.erase(i++);
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
				} else {
					fire(ConnectionManagerListener::FAILED, cqi, STRING(ALL_DOWNLOAD_SLOTS_TAKEN));
				}
			} else if(((i->second + 50*1000) < aTick) && cqi->getStatus() == ConnectionQueueItem::CONNECTING) {
				fire(ConnectionManagerListener::FAILED, cqi, STRING(CONNECTION_TIMEOUT));
				cqi->setStatus(ConnectionQueueItem::WAITING);
			}

			++i;
		}
	}

	for(ConnectionQueueItem::Iter k = add.begin(); k != add.end(); ++k) {
		fire(ConnectionManagerListener::STATUS_CHANGED, *k);
		(*k)->getConnection()->removeListener(this);
		DownloadManager::getInstance()->addConnection((*k)->getConnection());
	}

	for(ConnectionQueueItem::Iter l = remove.begin(); l != remove.end(); ++l) {
		fire(ConnectionManagerListener::REMOVED, *l);
		delete *l;
	}
	for(ConnectionQueueItem::Iter m = failPassive.begin(); m != failPassive.end(); ++m) {
		fire(ConnectionManagerListener::FAILED, *m, STRING(CANT_CONNECT_IN_PASSIVE_MODE));
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
		uc->setState(UserConnection::STATE_NICK);
	} catch(Exception e) {
		dcdebug("ConnectionManager::OnIncomingConnection caught: %s\n", e.getError().c_str());
		putConnection(uc);
	}
}

void ConnectionManager::connect(const string& aServer, short aPort, const string& aNick) {
	UserConnection* c = getConnection();
	c->setNick(aNick);
	c->setState(UserConnection::STATE_CONNECT);
	c->connect(aServer, aPort);
}

void ConnectionManager::onConnected(UserConnection* aSource) throw() {
	dcassert(aSource->getState() == UserConnection::STATE_CONNECT);
	aSource->myNick(aSource->getNick());
	aSource->lock(CryptoManager::getInstance()->getLock(), CryptoManager::getInstance()->getPk());
	aSource->setState(UserConnection::STATE_NICK);
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
		if(!ClientManager::getInstance()->isOnline(aNick)) {
			dcdebug("CM::onMyNick Incoming connection from unknown user %s\n", aNick.c_str());
			putConnection(aSource);
			return;
		}

		aSource->setUser(ClientManager::getInstance()->getUser(aNick));
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
	
	if( CryptoManager::getInstance()->isExtended(aLock) ) {
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

void ConnectionManager::onDirection(UserConnection* aSource, const string& dir, const string& /*num*/) throw() {
	if(dir == "Upload") {
		aSource->setFlag(UserConnection::FLAG_DOWNLOAD);
	} else {
		aSource->setFlag(UserConnection::FLAG_UPLOAD);
	}
}

void ConnectionManager::onFailed(UserConnection* aSource, const string& /*aError*/) throw() {
	if(aSource->isSet(UserConnection::FLAG_DOWNLOAD)) {
		{
			Lock l(cs);
			
			for(ConnectionQueueItem::Iter i = downPool.begin(); i != downPool.end(); ++i) {
				dcassert((*i)->getConnection());
				if((*i)->getConnection() == aSource) {
					dcdebug("ConnectionManager::onError Removing connection %p to %s from active pool\n", aSource, aSource->getUser()->getNick().c_str());
					downPool.erase(i);
					break;
				}
			}
		}

		putDownloadConnection(aSource);
	} else if(aSource->isSet(UserConnection::FLAG_UPLOAD)) {
		putUploadConnection(aSource);
	} else {
		putConnection(aSource);
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
 * $Id: ConnectionManager.cpp,v 1.39 2002/04/09 18:43:27 arnetheduck Exp $
 */

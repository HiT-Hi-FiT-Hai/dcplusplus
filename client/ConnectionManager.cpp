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

ConnectionManager* Singleton<ConnectionManager>::instance = NULL;

ConnectionManager::~ConnectionManager() {
	TimerManager::getInstance()->removeListener(this);

	socket.removeListener(this);
	socket.disconnect();

	{
		Lock l(cs);
		for(UserConnection::Iter j = userConnections.begin(); j != userConnections.end(); ++j) {
			(*j)->disconnect();
		}
	}

	while(true) {
		{
			Lock l(cs);
			if(userConnections.empty())
				break;
		}
		Thread::sleep(100);			
	}
}

/**
 * Request a connection for downloading.
 * DownloadManager::addConnection will be called as soon as the connection is ready
 * for downloading.
 * @param aUser The user to connect to.
 */
void ConnectionManager::getDownloadConnection(const User::Ptr& aUser) {
	dcassert((bool)aUser);
	ConnectionQueueItem* cqi = NULL;
	{
		Lock l(cs);

        ConnectionQueueItem::Iter j;
		if(find(pendingAdd.begin(), pendingAdd.end(), aUser) != pendingAdd.end())
			return;
		
		if( (j = find(downPool.begin(), downPool.end(), aUser)) != downPool.end()) {
			dcassert((*j)->getConnection());
			pendingAdd.push_back(aUser);
			return;
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
					User::Iter j = find(pendingAdd.begin(), pendingAdd.end(), i->second->getUser());
					if(j != pendingAdd.end()) {
						pendingAdd.erase(j);
					}
					i->second->setConnection(NULL);
					i->second->setStatus(ConnectionQueueItem::WAITING);
					dcassert(pendingDown.find(i->second) == pendingDown.end());
					pendingDown[i->second] = GET_TICK();
					connections.erase(i);
				}

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
		dcassert(find(userConnections.begin(), userConnections.end(), aConn) != userConnections.end());
		userConnections.erase(find(userConnections.begin(), userConnections.end(), aConn));
		pendingDelete.push_back(aConn);
	}
	if(cqi) {
		fire(ConnectionManagerListener::REMOVED, cqi);
		delete cqi;
	}
}

void ConnectionManager::onTimerSecond(u_int32_t aTick) {
	ConnectionQueueItem::List remove;
	ConnectionQueueItem::List failPassive;
	ConnectionQueueItem::List connecting;

	int attempts = 0;

	{
		Lock l(cs);
		{
			for(User::Iter k = pendingAdd.begin(); k != pendingAdd.end(); ++k) {
				ConnectionQueueItem::Iter i = find(downPool.begin(), downPool.end(), *k);
				if(i == downPool.end()) {
					// Hm, connection must have failed before it could be collected...
					getDownloadConnection(*k);
				} else {
					dcassert((*i)->getConnection());
					(*i)->getConnection()->removeListener(this);
					fire(ConnectionManagerListener::STATUS_CHANGED, *i);
					DownloadManager::getInstance()->addConnection((*i)->getConnection());
				}
			}
		}

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
					if( attempts <= 1 ) {
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

void ConnectionManager::onTimerMinute(u_int32_t aTick) {
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
		uc->setFlag(UserConnection::FLAG_INCOMING);
		uc->setState(UserConnection::STATE_NICK);
		uc->setLastActivity(GET_TICK());
		uc->accept(socket);
	} catch(Exception e) {
		dcdebug("ConnectionManager::OnIncomingConnection caught: %s\n", e.getError().c_str());
		putConnection(uc);
	}
}

void ConnectionManager::connect(const string& aServer, short aPort, const string& aNick) {
	if(shuttingDown)
		return;

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

	// First, we try looking in the pending downloads...hopefully it's one of them...
	{
		Lock l(cs);
		for(ConnectionQueueItem::TimeIter i = pendingDown.begin(); i != pendingDown.end(); ++i) {
			if(i->first->getUser()->getNick() == aNick) {
				aSource->setUser(i->first->getUser());
				// Indicate that we're interested in this file...
				aSource->setFlag(UserConnection::FLAG_DOWNLOAD);
			}
		}
	}

	if(!aSource->getUser()) {
		// Make sure we know who it is, i e that he/she is connected...
		if(!ClientManager::getInstance()->isOnline(aNick)) {
			dcdebug("CM::onMyNick Incoming connection from unknown user %s\n", aNick.c_str());
			putConnection(aSource);
			return;
		}

		aSource->setUser(ClientManager::getInstance()->getUser(aNick));
		// We don't need this connection for downloading...make it an upload connection instead...
		aSource->setFlag(UserConnection::FLAG_UPLOAD);
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
		aSource->supports(features);
	}

	aSource->setState(UserConnection::STATE_DIRECTION);
	aSource->direction(aSource->getDirectionString(), aSource->getNumber());
	aSource->key(CryptoManager::getInstance()->makeKey(aLock));
}

void ConnectionManager::onDirection(UserConnection* aSource, const string& dir, const string& num) throw() {
	if(aSource->getState() != UserConnection::STATE_DIRECTION) {
		dcdebug("CM::onDirection %p received direction twice, ignoring\n", aSource);
		return;
	}

	dcassert(aSource->isSet(UserConnection::FLAG_DOWNLOAD) ^ aSource->isSet(UserConnection::FLAG_UPLOAD));
	if(dir == "Upload") {
		// Fine, the other fellow want's to send us data...make sure we really want that...
		if(aSource->isSet(UserConnection::FLAG_UPLOAD)) {
			// Huh? Strange...disconnect...
			putConnection(aSource);
			return;
		}
	} else {
		if(aSource->isSet(UserConnection::FLAG_DOWNLOAD)) {
			int number = Util::toInt(num);
			// Damn, both want to download...the one with the highest number wins...
			if(aSource->getNumber() < number) {
				// Damn! We lost!
				aSource->unsetFlag(UserConnection::FLAG_DOWNLOAD);
				aSource->setFlag(UserConnection::FLAG_UPLOAD);
			}
		}
	}

	dcassert(aSource->isSet(UserConnection::FLAG_DOWNLOAD) ^ aSource->isSet(UserConnection::FLAG_UPLOAD));

	{
		Lock l(cs);
		// Only one connection / user...
		for(ConnectionQueueItem::QueueIter k = connections.begin(); k != connections.end(); ++k) {
			if( (k->first->getUser() == aSource->getUser()) && 
				(aSource->isSet(UserConnection::FLAG_UPLOAD) == k->first->isSet(UserConnection::FLAG_UPLOAD)) ) {
				
				putConnection(aSource);
				return;
			}
		}

		// Now, let's see if we still want this connection...
		// First, we try looking in the pending downloads...hopefully it's one of them...
		if(aSource->isSet(UserConnection::FLAG_DOWNLOAD)) {
			for(ConnectionQueueItem::TimeIter i = pendingDown.begin(); i != pendingDown.end(); ++i) {
				ConnectionQueueItem *cqi = i->first;
				if(cqi->getUser() == aSource->getUser()) {
					cqi->setConnection(aSource);
					aSource->setCQI(cqi);
					pendingDown.erase(i);
					connections[aSource] = cqi;
					aSource->setState(UserConnection::STATE_KEY);
					fire(ConnectionManagerListener::CONNECTED, cqi);
					return;
				}
			}
		} else {
			aSource->setFlag(UserConnection::FLAG_UPLOAD);
			ConnectionQueueItem *cqi = new ConnectionQueueItem(aSource->getUser());
			cqi->setConnection(aSource);
			aSource->setCQI(cqi);
			connections[aSource] = cqi;
			aSource->setState(UserConnection::STATE_KEY);
			fire(ConnectionManagerListener::ADDED, cqi);
			fire(ConnectionManagerListener::CONNECTED, cqi);
			return;
		}
	}
	// Don't want it anymore???
	putConnection(aSource);
}

void ConnectionManager::onKey(UserConnection* aSource, const string&/* aKey*/) throw() {
	if(aSource->getState() != UserConnection::STATE_KEY) {
		dcdebug("CM::onKey Bad state, ignoring");
		return;
	}
	// We don't want any messages while the Up/DownloadManagers are working...
	aSource->removeListener(this);
	dcassert(aSource->getUser());

	if(aSource->isSet(UserConnection::FLAG_DOWNLOAD)) {
		dcdebug("ConnectionManager::onKey, leaving to downloadmanager\n");
		DownloadManager::getInstance()->addConnection(aSource);
	} else {
		dcassert(aSource->isSet(UserConnection::FLAG_UPLOAD));
		dcdebug("ConnectionManager::onKey, leaving to uploadmanager\n");
		UploadManager::getInstance()->addConnection(aSource);
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
	{
		Lock l(cs);

		if(pendingDown.find(aCqi) != pendingDown.end()) {
			pendingDown.erase(aCqi);
			fire(ConnectionManagerListener::REMOVED, aCqi);
			delete aCqi;
		} else {
			ConnectionQueueItem::QueueIter i;
			for(i = connections.begin(); i != connections.end(); ++i) {
				if(i->second == aCqi)
					break;
			}

			if(i!=connections.end()) {
				dcassert(aCqi->getConnection());
				aCqi->getConnection()->disconnect();
			} else {
				dcdebug("ConnectionManager::removeConnection: %p not found\n", aCqi);
			}
		}
	}
}

void ConnectionManager::shutdown() {
	shuttingDown = true;
	socket.removeListener(this);
	socket.disconnect();
	{
		Lock l(cs);
		for(UserConnection::Iter j = userConnections.begin(); j != userConnections.end(); ++j) {
			(*j)->disconnect();
		}
	}
	// Wait until all connections have died out...
	while(true) {
		{
			Lock l(cs);
			if(userConnections.empty()) {
				break;
			}
		}
		Thread::sleep(50);
	}
}		

// ServerSocketListener
void ConnectionManager::onAction(ServerSocketListener::Types type) {
	switch(type) {
	case ServerSocketListener::INCOMING_CONNECTION:
		onIncomingConnection();
	}
}

// UserConnectionListener
void ConnectionManager::onAction(UserConnectionListener::Types type, UserConnection* conn) {
	switch(type) {
	case UserConnectionListener::CONNECTED:
		onConnected(conn); break;
	}
}
void ConnectionManager::onAction(UserConnectionListener::Types type, UserConnection* conn, const string& line) {
	switch(type) {
	case UserConnectionListener::MY_NICK:
		onMyNick(conn, line); break;
	case UserConnectionListener::KEY:
		onKey(conn, line); break;
	case UserConnectionListener::FAILED:
		onFailed(conn, line); break;
	}
}
void ConnectionManager::onAction(UserConnectionListener::Types type, UserConnection* conn, const string& line1, const string& line2) {
	switch(type) {
	case UserConnectionListener::C_LOCK:
		onLock(conn, line1, line2); break;
	case UserConnectionListener::DIRECTION:
		onDirection(conn, line1, line2); break;
	}
}
// UserConnectionListener
void ConnectionManager::onAction(UserConnectionListener::Types type, UserConnection* conn, const StringList& feat) {
	switch(type) {
	case UserConnectionListener::SUPPORTS:
		{
			for(StringList::const_iterator i = feat.begin(); i != feat.end(); ++i) {
				if(*i == "BZList")
					conn->setFlag(UserConnection::FLAG_SUPPORTS_BZLIST);
			}
		}
		break;
	}
}

// TimerManagerListener
void ConnectionManager::onAction(TimerManagerListener::Types type, u_int32_t aTick) {
	switch(type) {
	case TimerManagerListener::SECOND: onTimerSecond(aTick); break;
	case TimerManagerListener::MINUTE: onTimerMinute(aTick); break;
	}
}

/**
 * @file ConnectionManger.cpp
 * $Id: ConnectionManager.cpp,v 1.50 2002/06/02 00:12:44 arnetheduck Exp $
 */

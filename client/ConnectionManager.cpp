/* 
 * Copyright (C) 2001-2004 Jacek Sieka, j_s at telia com
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
#include "CryptoManager.h"
#include "ClientManager.h"
#include "QueueManager.h"

#include "UserConnection.h"

ConnectionManager::ConnectionManager() : floodCounter(0), shuttingDown(false) {
	TimerManager::getInstance()->addListener(this);
	socket.addListener(this);

	features.push_back(UserConnection::FEATURE_MINISLOTS);
	features.push_back(UserConnection::FEATURE_XML_BZLIST);
	features.push_back(UserConnection::FEATURE_ADCGET);
	features.push_back(UserConnection::FEATURE_TTHL);
	features.push_back(UserConnection::FEATURE_TTHF);
};

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

		// Check the download pool
		if(find(downPool.begin(), downPool.end(), aUser) != downPool.end()) {
			if(find(pendingAdd.begin(), pendingAdd.end(), aUser) == pendingAdd.end())
				pendingAdd.push_back(aUser);
			return;
		}
		
		// See if we're already trying to connect
		if(find(pendingDown.begin(), pendingDown.end(), aUser) != pendingDown.end())
			return;

		// Check if we have an active download connection already
		for(ConnectionQueueItem::Iter j = active.begin(); j != active.end(); ++j) {
			dcassert((*j)->getConnection());
			if((*j == aUser) && ((*j)->getConnection()->isSet(UserConnection::FLAG_DOWNLOAD)))
				return;
		}
		
		// Add it to the pending...
		cqi = new ConnectionQueueItem(aUser);
		cqi->setState(ConnectionQueueItem::WAITING);
		pendingDown.push_back(cqi);

		fire(ConnectionManagerListener::Added(), cqi);
	}
}

void ConnectionManager::putDownloadConnection(UserConnection* aSource, bool reuse /* = false */) {
	// Pool it for later usage...
	if(reuse) {
		aSource->addListener(this);
		{
			Lock l(cs);
			aSource->getCQI()->setState(ConnectionQueueItem::IDLE);

			dcassert(find(active.begin(), active.end(), aSource->getCQI()) != active.end());
			active.erase(find(active.begin(), active.end(), aSource->getCQI()));
			
			downPool.push_back(aSource->getCQI());
		}
		dcdebug("ConnectionManager::putDownloadConnection Pooling reusable connection %p to %s\n", aSource, aSource->getUser()->getNick().c_str());
		
	} else {
		if(QueueManager::getInstance()->hasDownload(aSource->getCQI()->getUser())) {
			aSource->removeListeners();
			aSource->disconnect();
			Lock l(cs);

			ConnectionQueueItem* cqi = aSource->getCQI();
			dcassert(cqi);
			
			// Remove the userconnection, don't need it any more
			dcassert(find(userConnections.begin(), userConnections.end(), aSource) != userConnections.end());
			userConnections.erase(find(userConnections.begin(), userConnections.end(), aSource));
			pendingDelete.push_back(aSource);

			cqi->setConnection(NULL);
			cqi->setState(ConnectionQueueItem::WAITING);
			
			dcassert(find(active.begin(), active.end(), aSource->getCQI()) != active.end());
			active.erase(find(active.begin(), active.end(), aSource->getCQI()));
			
			cqi->setLastAttempt(GET_TICK());
			pendingDown.push_back(cqi);
		} else {
			{
				Lock l(cs);
				dcassert(find(active.begin(), active.end(), aSource->getCQI()) != active.end());
				active.erase(find(active.begin(), active.end(), aSource->getCQI()));
			}
			putConnection(aSource);
		}
	}
}

void ConnectionManager::putUploadConnection(UserConnection* aSource) {
	{
		Lock l(cs);
		dcassert(find(active.begin(), active.end(), aSource->getCQI()) != active.end());
		active.erase(find(active.begin(), active.end(), aSource->getCQI()));
	}
	putConnection(aSource);
}

void ConnectionManager::putConnection(UserConnection* aConn) {
	aConn->removeListeners();
	aConn->disconnect();
	ConnectionQueueItem* cqi = NULL;
	{
		Lock l(cs);
		cqi = aConn->getCQI();
		
		dcassert(find(userConnections.begin(), userConnections.end(), aConn) != userConnections.end());
		userConnections.erase(find(userConnections.begin(), userConnections.end(), aConn));
		pendingDelete.push_back(aConn);
	}
	if(cqi) {
		fire(ConnectionManagerListener::Removed(), cqi);
		delete cqi;
	}
}

void ConnectionManager::on(TimerManagerListener::Second, u_int32_t aTick) throw() {
	ConnectionQueueItem::List failPassive;
	ConnectionQueueItem::List connecting;
	ConnectionQueueItem::List removed;
	User::List getDown;
	{
		Lock l(cs);
		{
			for(User::Iter k = pendingAdd.begin(); k != pendingAdd.end(); ++k) {
				ConnectionQueueItem::Iter i = find(downPool.begin(), downPool.end(), *k);
				if(i == downPool.end()) {
					// Hm, connection must have failed before it could be collected...
					getDown.push_back(*k);
				} else {
					ConnectionQueueItem* cqi = *i;
					downPool.erase(i);
					dcassert(find(active.begin(), active.end(), cqi) == active.end());
					active.push_back(cqi);
					dcassert(cqi->getConnection());
					dcassert(cqi->getConnection()->getCQI() == cqi);
					cqi->getConnection()->removeListener(this);
					DownloadManager::getInstance()->addConnection(cqi->getConnection());
				}
			}
			pendingAdd.clear();
		}

		bool tooMany = ((SETTING(DOWNLOAD_SLOTS) != 0) && DownloadManager::getInstance()->getDownloadCount() >= (size_t)SETTING(DOWNLOAD_SLOTS));
		bool tooFast = ((SETTING(MAX_DOWNLOAD_SPEED) != 0 && DownloadManager::getInstance()->getAverageSpeed() >= (SETTING(MAX_DOWNLOAD_SPEED)*1024)));
		
		bool startDown = !tooMany && !tooFast;

		int attempts = 0;
		
		ConnectionQueueItem::Iter i = pendingDown.begin();
		while(i != pendingDown.end()) {
			ConnectionQueueItem* cqi = *i;
			dcassert(cqi->getUser());

			if(!cqi->getUser()->isOnline()) {
				// Not online anymore...remove him from the pending...
				i = pendingDown.erase(i);
				removed.push_back(cqi);
				continue;
			}

			if( ((cqi->getLastAttempt() + 60*1000) < aTick) && (attempts < 2) ) {
				cqi->setLastAttempt(aTick);

				if(!QueueManager::getInstance()->hasDownload(cqi->getUser())) {
					i = pendingDown.erase(i);
					removed.push_back(cqi);
					continue;
				}

				if(cqi->getUser()->isSet(User::PASSIVE) && (SETTING(CONNECTION_TYPE) != SettingsManager::CONNECTION_ACTIVE)) {
					i = pendingDown.erase(i);
					failPassive.push_back(cqi);
					continue;
				}

				// Always start high-priority downloads unless we have 3 more than maxdownslots already...
				if(!startDown) {
					bool extraFull = (SETTING(DOWNLOAD_SLOTS) != 0) && (DownloadManager::getInstance()->getDownloadCount() >= (size_t)(SETTING(DOWNLOAD_SLOTS)+3));
					startDown = !extraFull && QueueManager::getInstance()->hasDownload(cqi->getUser(), QueueItem::HIGHEST);
				}
				if(cqi->getState() == ConnectionQueueItem::WAITING) {
					if(startDown) {
						cqi->setState(ConnectionQueueItem::CONNECTING);
						cqi->getUser()->connect();
						fire(ConnectionManagerListener::StatusChanged(), cqi);
						attempts++;
					} else {
						cqi->setState(ConnectionQueueItem::NO_DOWNLOAD_SLOTS);
						fire(ConnectionManagerListener::Failed(), cqi, STRING(ALL_DOWNLOAD_SLOTS_TAKEN));
					}
				} else if(cqi->getState() == ConnectionQueueItem::NO_DOWNLOAD_SLOTS && startDown) {
					cqi->setState(ConnectionQueueItem::WAITING);
				}
			} else if(((cqi->getLastAttempt() + 50*1000) < aTick) && (cqi->getState() == ConnectionQueueItem::CONNECTING)) {
				fire(ConnectionManagerListener::Failed(), cqi, STRING(CONNECTION_TIMEOUT));
				cqi->setState(ConnectionQueueItem::WAITING);
			}
			++i;
		}
	}

	ConnectionQueueItem::Iter m;
	for(m = removed.begin(); m != removed.end(); ++m) {
		fire(ConnectionManagerListener::Removed(), *m);
		delete *m;
	}
	for(m = failPassive.begin(); m != failPassive.end(); ++m) {
		QueueManager::getInstance()->removeSources((*m)->getUser(), QueueItem::Source::FLAG_PASSIVE);
		fire(ConnectionManagerListener::Removed(), *m);
		delete *m;
	}
	for(User::Iter n = getDown.begin(); n != getDown.end(); ++n) {
		getDownloadConnection(*n);
	}
}

void ConnectionManager::on(TimerManagerListener::Minute, u_int32_t aTick) throw() {	
	Lock l(cs);
	{
		for(UserConnection::Iter j = userConnections.begin(); j != userConnections.end(); ++j) {
			if(((*j)->getLastActivity() + 180*1000) < aTick) {
				(*j)->disconnect();
			}
		}
	}

	for_each(pendingDelete.begin(), pendingDelete.end(), DeleteFunction<UserConnection*>());
	pendingDelete.clear();
}

static const u_int32_t FLOOD_TRIGGER = 10000;
static const u_int32_t FLOOD_ADD = 2000;

/**
 * Someone's connecting, accept the connection and wait for identification...
 * It's always the other fellow that starts sending if he made the connection.
 */
void ConnectionManager::on(ServerSocketListener::IncomingConnection) throw() {
	UserConnection* uc = NULL;
	u_int32_t now = GET_TICK();

	if(now > floodCounter) {
		floodCounter = now + FLOOD_ADD;
	} else {
		if(now + FLOOD_TRIGGER < floodCounter) {
			Socket s;
			try {
				s.accept(socket);
			} catch(const SocketException&) {
				// ...
			}
			dcdebug("Connection flood detected!\n");
			return;
		} else {
			floodCounter += 2000;
		}
	}

	try { 
		uc = getConnection();
		uc->setFlag(UserConnection::FLAG_INCOMING);
		uc->setState(UserConnection::STATE_NICK);
		uc->setLastActivity(GET_TICK());
		uc->accept(socket);
	} catch(const SocketException& e) {
		dcdebug("ConnectionManager::OnIncomingConnection caught: %s\n", e.getError().c_str());
		if(uc)
			putConnection(uc);
	}
}

void ConnectionManager::connect(const string& aServer, short aPort, const string& aNick) {
	if(shuttingDown)
		return;

	UserConnection* uc = NULL;
	try {
		uc = getConnection();
		uc->setNick(aNick);
		uc->setState(UserConnection::STATE_CONNECT);
		uc->connect(aServer, aPort);
	} catch(const SocketException&) {
		if(uc)
			putConnection(uc);
	}
}

void ConnectionManager::on(UserConnectionListener::Connected, UserConnection* aSource) throw() {
	dcassert(aSource->getState() == UserConnection::STATE_CONNECT);
	aSource->myNick(aSource->getNick());
	aSource->lock(CryptoManager::getInstance()->getLock(), CryptoManager::getInstance()->getPk());
	aSource->setState(UserConnection::STATE_NICK);
}

/**
 * Nick received. If it's a downloader, fine, otherwise it must be an uploader.
 */
void ConnectionManager::on(UserConnectionListener::MyNick, UserConnection* aSource, const string& aNick) throw() {

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
		for(ConnectionQueueItem::Iter i = pendingDown.begin(); i != pendingDown.end(); ++i) {
			ConnectionQueueItem* cqi = *i;
			if(cqi->getUser()->getNick() == aNick) {
				aSource->setUser(cqi->getUser());
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

void ConnectionManager::on(UserConnectionListener::CLock, UserConnection* aSource, const string& aLock, const string& aPk) throw() {
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
		StringList defFeatures = features;
		if(BOOLSETTING(COMPRESS_TRANSFERS)) {
			defFeatures.push_back(UserConnection::FEATURE_GET_ZBLOCK);
			defFeatures.push_back(UserConnection::FEATURE_ZLIB_GET);
		}

		aSource->supports(defFeatures);
	}

	aSource->setState(UserConnection::STATE_DIRECTION);
	aSource->direction(aSource->getDirectionString(), aSource->getNumber());
	aSource->key(CryptoManager::getInstance()->makeKey(aLock));
}

void ConnectionManager::on(UserConnectionListener::Direction, UserConnection* aSource, const string& dir, const string& num) throw() {
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
			} else if(aSource->getNumber() == number) {
				putConnection(aSource);
				return;
			}
		}
	}

	dcassert(aSource->isSet(UserConnection::FLAG_DOWNLOAD) ^ aSource->isSet(UserConnection::FLAG_UPLOAD));

	aSource->setState(UserConnection::STATE_KEY);
}

void ConnectionManager::on(UserConnectionListener::Key, UserConnection* aSource, const string&/* aKey*/) throw() {
	if(aSource->getState() != UserConnection::STATE_KEY) {
		dcdebug("CM::onKey Bad state, ignoring");
		return;
	}
	// We don't want any messages while the Up/DownloadManagers are working...
	aSource->removeListener(this);
	dcassert(aSource->getUser());
	{
		Lock l(cs);

		// Only one connection / user & direction...
		for(ConnectionQueueItem::Iter k = active.begin(); k != active.end(); ++k) {
			bool sameDirection = (*k)->getConnection()->isSet(UserConnection::FLAG_UPLOAD) == aSource->isSet(UserConnection::FLAG_UPLOAD);
			if( sameDirection && (*k == aSource->getUser()) ) {
				putConnection(aSource);
				return;
			}
		}
		
		ConnectionQueueItem* cqi = NULL;

		if(aSource->isSet(UserConnection::FLAG_DOWNLOAD)) {
			// See if we have a matching user in the pending connections...
			ConnectionQueueItem::Iter i = find(pendingDown.begin(), pendingDown.end(), aSource->getUser());

			if(i == pendingDown.end()) {
				putConnection(aSource);
				return;
			}
			cqi = *i;
			pendingDown.erase(i);
			cqi->setConnection(aSource);
		} else {
			dcassert(aSource->isSet(UserConnection::FLAG_UPLOAD));
			cqi = new ConnectionQueueItem(aSource->getUser());
			cqi->setConnection(aSource);
			fire(ConnectionManagerListener::Added(), cqi);			
		}

		aSource->setCQI(cqi);

		dcassert(find(active.begin(), active.end(), cqi) == active.end());
		active.push_back(cqi);

		fire(ConnectionManagerListener::Connected(), cqi);
		
		if(aSource->isSet(UserConnection::FLAG_DOWNLOAD)) {
			dcdebug("ConnectionManager::onKey, leaving to downloadmanager\n");
			DownloadManager::getInstance()->addConnection(aSource);
		} else {
			dcassert(aSource->isSet(UserConnection::FLAG_UPLOAD));
			dcdebug("ConnectionManager::onKey, leaving to uploadmanager\n");
			UploadManager::getInstance()->addConnection(aSource);
		}
	}
}

void ConnectionManager::on(UserConnectionListener::Failed, UserConnection* aSource, const string& /*aError*/) throw() {
	if(aSource->isSet(UserConnection::FLAG_DOWNLOAD) && aSource->getCQI()) {
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
	}
	putConnection(aSource);
}

void ConnectionManager::removeConnection(const User::Ptr& aUser, int isDownload) {
	Lock l(cs);
	for(UserConnection::Iter i = userConnections.begin(); i != userConnections.end(); ++i) {
		UserConnection* uc = *i;
		if(uc->getUser() == aUser && uc->isSet(isDownload ? UserConnection::FLAG_DOWNLOAD : UserConnection::FLAG_UPLOAD)) {
			uc->disconnect();
			break;
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

// UserConnectionListener
void ConnectionManager::on(UserConnectionListener::Supports, UserConnection* conn, const StringList& feat) throw() {
	for(StringList::const_iterator i = feat.begin(); i != feat.end(); ++i) {
		if(*i == UserConnection::FEATURE_GET_ZBLOCK)
			conn->setFlag(UserConnection::FLAG_SUPPORTS_GETZBLOCK);
		else if(*i == UserConnection::FEATURE_MINISLOTS)
			conn->setFlag(UserConnection::FLAG_SUPPORTS_MINISLOTS);
		else if(*i == UserConnection::FEATURE_XML_BZLIST)
			conn->setFlag(UserConnection::FLAG_SUPPORTS_XML_BZLIST);
		else if(*i == UserConnection::FEATURE_ADCGET)
			conn->setFlag(UserConnection::FLAG_SUPPORTS_ADCGET);
		else if(*i == UserConnection::FEATURE_ZLIB_GET)
			conn->setFlag(UserConnection::FLAG_SUPPORTS_ZLIB_GET);
		else if(*i == UserConnection::FEATURE_TTHL)
			conn->setFlag(UserConnection::FLAG_SUPPORTS_TTHL);
		else if(*i == UserConnection::FEATURE_TTHF)
			conn->setFlag(UserConnection::FLAG_SUPPORTS_TTHF);
	}
}

/**
 * @file
 * $Id: ConnectionManager.cpp,v 1.79 2004/11/06 12:13:59 arnetheduck Exp $
 */

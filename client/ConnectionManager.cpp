/*
 * Copyright (C) 2001-2005 Jacek Sieka, arnetheduck on gmail point com
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

#include "ResourceManager.h"
#include "DownloadManager.h"
#include "UploadManager.h"
#include "CryptoManager.h"
#include "ClientManager.h"
#include "QueueManager.h"

#include "UserConnection.h"

ConnectionManager::ConnectionManager() : port(0), securePort(0), server(0), secureServer(0), floodCounter(0), shuttingDown(false) {
	TimerManager::getInstance()->addListener(this);

	features.push_back(UserConnection::FEATURE_MINISLOTS);
	features.push_back(UserConnection::FEATURE_XML_BZLIST);
	features.push_back(UserConnection::FEATURE_ADCGET);
	features.push_back(UserConnection::FEATURE_TTHL);
	features.push_back(UserConnection::FEATURE_TTHF);

	adcFeatures.push_back("+BAS0");
}
// @todo clean this up
void ConnectionManager::listen() throw(Exception){
	short lastPort = (short)SETTING(TCP_PORT);
	
	if(lastPort == 0)
		lastPort = (short)Util::rand(1025, 32000);

	short firstPort = lastPort;

	disconnect();

	while(true) {
		try {
			server = new Server(false, lastPort, SETTING(BIND_ADDRESS));
			port = lastPort;
			break;
		} catch(const Exception&) {
			short newPort = (short)((lastPort == 32000) ? 1025 : lastPort + 1);
			if(!SettingsManager::getInstance()->isDefault(SettingsManager::TCP_PORT) || (firstPort == newPort)) {
				throw Exception("Could not find a suitable free port");
			}
			lastPort = newPort;
		}
	}

	lastPort++;
	firstPort = lastPort;

	while(true) {
		try {
			secureServer = new Server(true, lastPort, SETTING(BIND_ADDRESS));
			securePort = lastPort;
			break;
		} catch(const Exception&) {
			short newPort = (short)((lastPort == 32000) ? 1025 : lastPort + 1);
			if(!SettingsManager::getInstance()->isDefault(SettingsManager::TCP_PORT) || (firstPort == newPort)) {
				throw Exception("Could not find a suitable free port");
			}
			lastPort = newPort;
		}
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
	{
		Lock l(cs);
		ConnectionQueueItem::Iter i = find(downloads.begin(), downloads.end(), aUser);
		if(i == downloads.end()) {
			getCQI(aUser, true);
		} else {
			ConnectionQueueItem* cqi = *i;
			if(cqi->getState() == ConnectionQueueItem::IDLE) {
				if(find(pendingAdd.begin(), pendingAdd.end(), aUser) == pendingAdd.end())
					pendingAdd.push_back(aUser);
				return;
			}
		}
	}
}

ConnectionQueueItem* ConnectionManager::getCQI(const User::Ptr& aUser, bool download) {
	ConnectionQueueItem* cqi = new ConnectionQueueItem(aUser, download);
	if(download) {
		dcassert(find(downloads.begin(), downloads.end(), aUser) == downloads.end());
		downloads.push_back(cqi);

	} else {
		dcassert(find(uploads.begin(), uploads.end(), aUser) == uploads.end());
		uploads.push_back(cqi);
	}

	fire(ConnectionManagerListener::Added(), cqi);
	
	return cqi;
}

void ConnectionManager::putCQI(ConnectionQueueItem* cqi) {
	fire(ConnectionManagerListener::Removed(), cqi);
	if(cqi->getDownload()) {
		dcassert(find(downloads.begin(), downloads.end(), cqi) != downloads.end());
		downloads.erase(find(downloads.begin(), downloads.end(), cqi));
	} else {
		dcassert(find(uploads.begin(), uploads.end(), cqi) != uploads.end());
		uploads.erase(find(uploads.begin(), uploads.end(), cqi));
	}
	delete cqi;
}

void ConnectionManager::putDownloadConnection(UserConnection* aSource, bool reuse /* = false */, bool ntd /* = false */) {
	ConnectionQueueItem* cqi = aSource->getCQI();
	dcassert(cqi);

	if(reuse) {
		dcdebug("ConnectionManager::putDownloadConnection Pooling reusable connection %p to %s\n", aSource, aSource->getUser()->getFirstNick().c_str());
		// Pool it for later usage...
		aSource->addListener(this);
		{
			Lock l(cs);
			cqi->setState(ConnectionQueueItem::IDLE);
		}
	} else {
		// Disassociate the two...
		aSource->setCQI(NULL);

		bool hasDown = QueueManager::getInstance()->hasDownload(aSource->getUser());

		{
			Lock l(cs);
			cqi->setConnection(NULL);
			if(hasDown) {
				cqi->setLastAttempt(GET_TICK());
				cqi->setState(ConnectionQueueItem::WAITING);
			} else {
				putCQI(cqi);
			}
		}

		if(ntd) {
			aSource->unsetFlag(UserConnection::FLAG_DOWNLOAD);
			addUploadConnection(aSource);
		} else {
			putConnection(aSource);
		}
	}
}

void ConnectionManager::putUploadConnection(UserConnection* aSource, bool ntd) {
	ConnectionQueueItem* cqi = aSource->getCQI();
	aSource->setCQI(NULL);

	if(ntd) {
		// We should pass it to the download manager...
		aSource->unsetFlag(UserConnection::FLAG_UPLOAD);
		aSource->setFlag(UserConnection::FLAG_DOWNLOAD);
		addDownloadConnection(aSource, false);
	} else {
		putConnection(aSource);
	}

	{
		Lock l(cs);
		putCQI(cqi);
	}
}

UserConnection* ConnectionManager::getConnection(bool aNmdc, bool secure) throw(SocketException) {
	UserConnection* uc = new UserConnection(secure);
	uc->addListener(this);
	{
		Lock l(cs);
		userConnections.push_back(uc);
	}
	if(aNmdc)
		uc->setFlag(UserConnection::FLAG_NMDC);
	return uc;
}

void ConnectionManager::putConnection(UserConnection* aConn) {
	aConn->removeListeners();
	aConn->disconnect();
	{
		Lock l(cs);
		
		dcassert(find(userConnections.begin(), userConnections.end(), aConn) != userConnections.end());
		userConnections.erase(find(userConnections.begin(), userConnections.end(), aConn));
		pendingDelete.push_back(aConn);
	}
}

void ConnectionManager::on(TimerManagerListener::Second, u_int32_t aTick) throw() {
	User::List passiveUsers;
	ConnectionQueueItem::List removed;
	UserConnection::List added;
	UserConnection::List penDel;

	bool tooMany = ((SETTING(DOWNLOAD_SLOTS) != 0) && DownloadManager::getInstance()->getDownloadCount() >= (size_t)SETTING(DOWNLOAD_SLOTS));
	bool tooFast = ((SETTING(MAX_DOWNLOAD_SPEED) != 0 && DownloadManager::getInstance()->getAverageSpeed() >= (SETTING(MAX_DOWNLOAD_SPEED)*1024)));

	{
		Lock l(cs);

		int attempts = 0;

		for(ConnectionQueueItem::Iter i = downloads.begin(); i != downloads.end(); ++i) {
			ConnectionQueueItem* cqi = *i;

			if(cqi->getState() == ConnectionQueueItem::ACTIVE) {
				// Do nothing
			} else if(cqi->getState() == ConnectionQueueItem::IDLE) {
				User::Iter it = find(pendingAdd.begin(), pendingAdd.end(), cqi->getUser());
				if(it != pendingAdd.end()) {
					dcassert(cqi->getConnection());
					dcassert(cqi->getConnection()->getCQI() == cqi);
					cqi->setState(ConnectionQueueItem::ACTIVE);
					cqi->getConnection()->removeListener(this);
					added.push_back(cqi->getConnection());

					pendingAdd.erase(it);
				}
			} else {
				if(cqi->getState() == ConnectionQueueItem::WAITING) {
					UserConnection::List::iterator it = find(pendingDelete.begin(), pendingDelete.end(), cqi->getConnection());
					if(it != pendingDelete.end()) {
						cqi->setConnection(NULL);
						removed.push_back(cqi);
						continue;
					}
				} 
				
				if(!cqi->getUser()->isOnline()) {
					// Not online anymore...remove it from the pending...
					removed.push_back(cqi);
					continue;
				} 
				
				if(cqi->getUser()->isSet(User::PASSIVE) && !ClientManager::getInstance()->isActive()) {
					passiveUsers.push_back(cqi->getUser());
					removed.push_back(cqi);
					continue;
				}

				if( ((cqi->getLastAttempt() + 60*1000) < aTick) && (attempts < 2) ) {
					cqi->setLastAttempt(aTick);

					if(!QueueManager::getInstance()->hasDownload(cqi->getUser())) {
						removed.push_back(cqi);
						continue;
					}

					// Always start high-priority downloads unless we have 3 more than maxdownslots already...
					bool startDown = !tooMany && !tooFast;

					if(!startDown) {
						bool extraFull = (SETTING(DOWNLOAD_SLOTS) != 0) && (DownloadManager::getInstance()->getDownloadCount() >= (size_t)(SETTING(DOWNLOAD_SLOTS)+3));
						startDown = !extraFull && QueueManager::getInstance()->hasDownload(cqi->getUser(), QueueItem::HIGHEST);
					}

					if(cqi->getState() == ConnectionQueueItem::WAITING) {
						if(startDown) {
							cqi->setState(ConnectionQueueItem::CONNECTING);
							ClientManager::getInstance()->connect(cqi->getUser());
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
			}
		}

		pendingAdd.clear();

		for(ConnectionQueueItem::Iter m = removed.begin(); m != removed.end(); ++m) {
			putCQI(*m);
		}

		penDel = pendingDelete;
		pendingDelete.clear();

	}

	for_each(penDel.begin(), penDel.end(), DeleteFunction<UserConnection*>());

	for(User::Iter ui = passiveUsers.begin(); ui != passiveUsers.end(); ++ui) {
		QueueManager::getInstance()->removeSource(*ui, QueueItem::Source::FLAG_PASSIVE);
	}

	for(UserConnection::Iter i = added.begin(); i != added.end(); ++i) {
		DownloadManager::getInstance()->addConnection(*i);
	}
}

void ConnectionManager::on(TimerManagerListener::Minute, u_int32_t aTick) throw() {	
	Lock l(cs);

	for(UserConnection::Iter j = userConnections.begin(); j != userConnections.end(); ++j) {
		if(((*j)->getLastActivity() + 180*1000) < aTick) {
			(*j)->disconnect();
		}
	}
}

static const u_int32_t FLOOD_TRIGGER = 20000;
static const u_int32_t FLOOD_ADD = 2000;

ConnectionManager::Server::Server(bool secure_, short port, const string& ip /* = "0.0.0.0" */) : secure(secure_), die(false) {
	sock.create();
	sock.bind(port, ip);
	sock.listen();

	start();
}


static const u_int32_t POLL_TIMEOUT = 250;

int ConnectionManager::Server::run() throw() {
	while(!die) {
		if(sock.wait(POLL_TIMEOUT, Socket::WAIT_READ) == Socket::WAIT_READ) {
			ConnectionManager::getInstance()->accept(sock, secure);
		}
	}
	return 0;
}

/**
 * Someone's connecting, accept the connection and wait for identification...
 * It's always the other fellow that starts sending if he made the connection.
 */
void ConnectionManager::accept(const Socket& sock, bool secure) throw() {
	UserConnection* uc = NULL;
	u_int32_t now = GET_TICK();

	if(now > floodCounter) {
		floodCounter = now + FLOOD_ADD;
	} else {
		if(now + FLOOD_TRIGGER < floodCounter) {
			Socket s;
			try {
				s.accept(sock);
			} catch(const SocketException&) {
				// ...
			}
			dcdebug("Connection flood detected!\n");
			return;
		} else {
			floodCounter += FLOOD_ADD;
		}
	}

	try { 
		uc = getConnection(false, secure);
		uc->setFlag(UserConnection::FLAG_INCOMING);
		uc->setState(UserConnection::STATE_SUPNICK);
		uc->setLastActivity(GET_TICK());
		uc->accept(sock);
	} catch(const SocketException& e) {
		dcdebug("ConnectionManager::OnIncomingConnection caught: %s\n", e.getError().c_str());
		if(uc)
			putConnection(uc);
	}
}

void ConnectionManager::nmdcConnect(const string& aServer, short aPort, const string& aNick, const string& hubUrl) {
	if(shuttingDown)
		return;

	UserConnection* uc = NULL;
	try {
		uc = getConnection(true, false);
		uc->setToken(aNick);
		uc->setHubUrl(hubUrl);
		uc->setState(UserConnection::STATE_CONNECT);
		uc->setFlag(UserConnection::FLAG_NMDC);
		uc->connect(aServer, aPort);
	} catch(const SocketException&) {
		if(uc)
			putConnection(uc);
	}
}

void ConnectionManager::adcConnect(const OnlineUser& aUser, short aPort, const string& aToken, bool secure) {
	if(shuttingDown)
		return;

	UserConnection* uc = NULL;
	try {
		uc = getConnection(false, secure);
		uc->setToken(aToken);
		uc->setState(UserConnection::STATE_CONNECT);
		if(aUser.getIdentity().isOp()) {
			uc->setFlag(UserConnection::FLAG_OP);
		}
		uc->connect(aUser.getIdentity().getIp(), aPort);
	} catch(const SocketException&) {
		if(uc)
			putConnection(uc);
	}
}

void ConnectionManager::on(AdcCommand::SUP, UserConnection* aSource, const AdcCommand&) throw() {
	if(aSource->getState() != UserConnection::STATE_SUPNICK) {
		// Already got this once, ignore...
		dcdebug("CM::onMyNick %p sent nick twice\n", aSource);
		return;
	}

	if(aSource->isSet(UserConnection::FLAG_INCOMING)) {
		aSource->sup(adcFeatures);
		aSource->inf(false);
	} else {
		aSource->inf(true);
	}
	aSource->setState(UserConnection::STATE_INF);
}

void ConnectionManager::on(AdcCommand::NTD, UserConnection*, const AdcCommand&) throw() {

}

void ConnectionManager::on(AdcCommand::STA, UserConnection*, const AdcCommand&) throw() {
	
}

void ConnectionManager::on(UserConnectionListener::Connected, UserConnection* aSource) throw() {
	dcassert(aSource->getState() == UserConnection::STATE_CONNECT);
	if(aSource->isSet(UserConnection::FLAG_NMDC)) {
		aSource->myNick(aSource->getToken());
		aSource->lock(CryptoManager::getInstance()->getLock(), CryptoManager::getInstance()->getPk());
	} else {
		aSource->sup(adcFeatures);
	}
	aSource->setState(UserConnection::STATE_SUPNICK);
}

void ConnectionManager::on(UserConnectionListener::MyNick, UserConnection* aSource, const string& aNick) throw() {
	if(aSource->getState() != UserConnection::STATE_SUPNICK) {
		// Already got this once, ignore...
		dcdebug("CM::onMyNick %p sent nick twice\n", aSource);
		return;
	}

	dcassert(aNick.size() > 0);
	dcdebug("ConnectionManager::onMyNick %p, %s\n", aSource, aNick.c_str());
	dcassert(!aSource->getUser());

	if(aSource->isSet(UserConnection::FLAG_INCOMING)) {
		// Try to guess where this came from...
		ExpectMap::iterator i = expectedConnections.find(aNick);
		if(i == expectedConnections.end()) {
			dcdebug("Unknown incoming connection from %s\n", aNick.c_str());
			putConnection(aSource);
			return;
		}
        aSource->setToken(i->second.first);	
		aSource->setHubUrl(i->second.second);	
		expectedConnections.erase(i);
	}
	CID cid = ClientManager::getInstance()->makeCid(aNick, aSource->getHubUrl());

	// First, we try looking in the pending downloads...hopefully it's one of them...
	{
		Lock l(cs);
		for(ConnectionQueueItem::Iter i = downloads.begin(); i != downloads.end(); ++i) {
			ConnectionQueueItem* cqi = *i;
			if((cqi->getState() == ConnectionQueueItem::CONNECTING || cqi->getState() == ConnectionQueueItem::WAITING) && cqi->getUser()->getCID() == cid) {
				aSource->setUser(cqi->getUser());
				// Indicate that we're interested in this file...
				aSource->setFlag(UserConnection::FLAG_DOWNLOAD);
				break;
			}
		}
	}

	if(!aSource->getUser()) {
		// Make sure we know who it is, i e that he/she is connected...

		aSource->setUser(ClientManager::getInstance()->findUser(cid));
		if(!aSource->getUser() || !ClientManager::getInstance()->isOnline(aSource->getUser())) {
			dcdebug("CM::onMyNick Incoming connection from unknown user %s\n", aNick.c_str());
			putConnection(aSource);
			return;
		}
		// We don't need this connection for downloading...make it an upload connection instead...
		aSource->setFlag(UserConnection::FLAG_UPLOAD);
	}

	if(ClientManager::getInstance()->isOp(aSource->getUser(), aSource->getHubUrl()))
		aSource->setFlag(UserConnection::FLAG_OP);

	if( aSource->isSet(UserConnection::FLAG_INCOMING) ) {
		aSource->myNick(aSource->getToken()); 
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
		if( (aPk.find("DCPLUSPLUS") != string::npos) && aSource->getUser() && !aSource->getUser()->isSet(User::DCPLUSPLUS)) {
			aSource->getUser()->setFlag(User::DCPLUSPLUS);
			/// @todo User::updated(aSource->getUser());
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

void ConnectionManager::addDownloadConnection(UserConnection* uc, bool sendNTD) {

	dcassert(uc->isSet(UserConnection::FLAG_DOWNLOAD));

	uc->removeListener(this);

	bool addConn = false;
	{
		Lock l(cs);

		ConnectionQueueItem::Iter i = find(downloads.begin(), downloads.end(), uc->getUser());
		if(i != downloads.end()) {
			ConnectionQueueItem* cqi = *i;
			if(cqi->getState() == ConnectionQueueItem::WAITING || cqi->getState() == ConnectionQueueItem::CONNECTING) {
				// Associate the two...
				dcassert(uc->getCQI() == NULL);
				uc->setCQI(cqi);
				dcassert(cqi->getConnection() == NULL);
				cqi->setConnection(uc);
				cqi->setState(ConnectionQueueItem::ACTIVE);

				fire(ConnectionManagerListener::Connected(), cqi);
				
				dcdebug("ConnectionManager::addDownloadConnection, leaving to downloadmanager\n");
				addConn = true;
			}
		}
	}

	if(addConn) {
		DownloadManager::getInstance()->addConnection(uc);
	} else if(sendNTD) {
		uc->ntd();
		uc->unsetFlag(UserConnection::FLAG_DOWNLOAD);
		uc->setFlag(UserConnection::FLAG_UPLOAD);
		addUploadConnection(uc);
	} else {
		putConnection(uc);
	}
}

void ConnectionManager::addUploadConnection(UserConnection* uc) {
	dcassert(uc->isSet(UserConnection::FLAG_UPLOAD));

	uc->removeListener(this);

	bool addConn = false;
	{
		Lock l(cs);

		ConnectionQueueItem::Iter i = find(uploads.begin(), uploads.end(), uc->getUser());
		if(i == uploads.end()) {
			ConnectionQueueItem* cqi = getCQI(uc->getUser(), false);

			uc->setCQI(cqi);
			cqi->setConnection(uc);
			cqi->setState(ConnectionQueueItem::ACTIVE);

			fire(ConnectionManagerListener::Connected(), cqi);

			dcdebug("ConnectionManager::addUploadConnection, leaving to uploadmanager\n");
			addConn = true;
		}
	}

	if(addConn) {
		UploadManager::getInstance()->addConnection(uc);
	} else {
		putConnection(uc);
	}
}

void ConnectionManager::on(UserConnectionListener::Key, UserConnection* aSource, const string&/* aKey*/) throw() {
	if(aSource->getState() != UserConnection::STATE_KEY) {
		dcdebug("CM::onKey Bad state, ignoring");
		return;
	}

	dcassert(aSource->getUser());

	if(aSource->isSet(UserConnection::FLAG_DOWNLOAD)) {
		addDownloadConnection(aSource, false);
	} else {
		addUploadConnection(aSource);
	}
}

void ConnectionManager::on(AdcCommand::INF, UserConnection* aSource, const AdcCommand& cmd) throw() {
	if(aSource->getState() != UserConnection::STATE_INF) {
		// Already got this once, ignore...
		aSource->sta(AdcCommand::SEV_FATAL, AdcCommand::ERROR_PROTOCOL_GENERIC, "Expecting INF");
		dcdebug("CM::onMyNick %p sent nick twice\n", aSource);
		return;
	}

	aSource->setUser(ClientManager::getInstance()->findUser(cmd.getFrom()));

	if(!aSource->getUser()) {
		dcdebug("CM::onINF: User not found");
		aSource->sta(AdcCommand::SEV_FATAL, AdcCommand::ERROR_GENERIC, "User not found");
		putConnection(aSource);
		return;
	}

	if(aSource->isSet(UserConnection::FLAG_INCOMING)) {
		aSource->setFlag(UserConnection::FLAG_DOWNLOAD);
		addDownloadConnection(aSource, true);
	} else {
		aSource->setFlag(UserConnection::FLAG_UPLOAD);
		addUploadConnection(aSource);
	}
}

void ConnectionManager::on(UserConnectionListener::Failed, UserConnection* aSource, const string& /*aError*/) throw() {
	if(aSource->isSet(UserConnection::FLAG_DOWNLOAD) && aSource->getCQI()) {
		{
			Lock l(cs);

			ConnectionQueueItem* cqi = aSource->getCQI();
			dcassert(cqi->getState() == ConnectionQueueItem::IDLE);
			cqi->setState(ConnectionQueueItem::WAITING);
			cqi->setLastAttempt(GET_TICK());
			//cqi->setConnection(NULL);
			aSource->setCQI(NULL);
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
	disconnect();
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
		if(*i == UserConnection::FEATURE_GET_ZBLOCK) {
			conn->setFlag(UserConnection::FLAG_SUPPORTS_GETZBLOCK); 
		} else if(*i == UserConnection::FEATURE_MINISLOTS) {
			conn->setFlag(UserConnection::FLAG_SUPPORTS_MINISLOTS);
		} else if(*i == UserConnection::FEATURE_XML_BZLIST) {
			conn->setFlag(UserConnection::FLAG_SUPPORTS_XML_BZLIST);
		} else if(*i == UserConnection::FEATURE_ADCGET) {
			conn->setFlag(UserConnection::FLAG_SUPPORTS_ADCGET);
		} else if(*i == UserConnection::FEATURE_ZLIB_GET) {
			conn->setFlag(UserConnection::FLAG_SUPPORTS_ZLIB_GET);
		} else if(*i == UserConnection::FEATURE_TTHL) {
			conn->setFlag(UserConnection::FLAG_SUPPORTS_TTHL);
		} else if(*i == UserConnection::FEATURE_TTHF) {
			conn->setFlag(UserConnection::FLAG_SUPPORTS_TTHF); 
			conn->getUser()->setFlag(User::TTH_GET);
		}
	}
}

/**
 * @file
 * $Id: ConnectionManager.cpp,v 1.109 2005/12/03 12:32:36 arnetheduck Exp $
 */

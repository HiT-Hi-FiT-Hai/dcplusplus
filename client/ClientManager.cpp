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

#include "ClientManager.h"
#include "ShareManager.h"
#include "SearchManager.h"

ClientManager* Singleton<ClientManager>::instance = NULL;

Client* ClientManager::getClient() {
	Client* c = new Client();

	{
		Lock l(cs);
		clients.push_back(c);
	}

	c->addListener(this);
	return c;
}

void ClientManager::putClient(Client* aClient) {
	aClient->removeListeners();
	aClient->disconnect();

	{
		Lock l(cs);

		// Either I'm stupid or the msvc7 optimizer is doing something _very_ strange here...
		// STL-port -D_STL_DEBUG complains that .begin() and .end() don't have the same owner (!)
		//		dcassert(find(clients.begin(), clients.end(), aClient) != clients.end());
		//		clients.erase(find(clients.begin(), clients.end(), aClient));
		
		for(Client::Iter i = clients.begin(); i != clients.end(); ++i) {
			if(*i == aClient) {
				clients.erase(i);
				break;
			}
		}
	}
	delete aClient;
}

void ClientManager::onClientHello(Client* aClient, const User::Ptr& aUser) throw() {
	if(aUser->getNick() == aClient->getNick()) {
		aClient->version(SETTING(CLIENTVERSION));
		aClient->getNickList();
		aClient->myInfo(aClient->getNick(), SETTING(DESCRIPTION), SETTING(CONNECTION), SETTING(EMAIL), ShareManager::getInstance()->getShareSizeString());
	} else {
		aClient->getInfo(aUser);
	}
}

void ClientManager::infoUpdated() {
	Lock l(cs);
	for(Client::Iter i = clients.begin(); i != clients.end(); ++i) {
		if((*i)->isConnected()) {
			(*i)->myInfo((*i)->getNick(), SETTING(DESCRIPTION), SETTING(CONNECTION), SETTING(EMAIL), ShareManager::getInstance()->getShareSizeString());
		}
	}
}

void ClientManager::onClientSearch(Client* aClient, const string& aSeeker, int aSearchType, const string& aSize, 
									int aFileType, const string& aString) throw() {
	
	bool search = false;
	if(SETTING(CONNECTION_TYPE) == SettingsManager::CONNECTION_ACTIVE) {
		if(aSeeker.find(SETTING(SERVER)) == string::npos) {
			search = true;
		}
	} else {
		if(aSeeker.find(aClient->getNick()) == string::npos) {
			search = true;
		}
	}
	
	if(search) {
		string::size_type pos = aSeeker.find("Hub:");
		// We don't wan't to answer passive searches if we're in passive mode...
		if(pos != string::npos && SETTING(CONNECTION_TYPE) == SettingsManager::CONNECTION_PASSIVE) {
			return;
		}
		
		SearchResult::List l = ShareManager::getInstance()->search(aString, aSearchType, aSize, aFileType, aClient, (pos == string::npos) ? 10 : 5);
//		dcdebug("Found %d items (%s)\n", l.size(), aString.c_str());
		if(l.size() > 0) {
			if(pos != string::npos) {
				string name = aSeeker.substr(4);
				// Good, we have a passive seeker, those are easier...
				string str;
				char* buf = new char[1024];
				for(SearchResult::Iter i = l.begin(); i != l.end(); ++i) {
					SearchResult* sr = *i;
					sprintf(buf, "$SR %s %s%c%s %d/%d%c%s (%s)%c%s|", aClient->getNick().c_str(), sr->getFile().c_str(), 5,
						Util::toString(sr->getSize()).c_str(), sr->getFreeSlots(), sr->getSlots(), 5, sr->getHubName().c_str(), sr->getHubAddress().c_str(), 5, name.c_str());
					str += buf;
					delete sr;
				}
				delete[] buf;
				
				if(str.size() > 0)
					aClient->searchResults(str);
				
			} else {
				char* buf = new char[1024];

				try {
					Socket s;
					s.create(Socket::TYPE_UDP);
					string ip, file;
					short port = 0;
					Util::decodeUrl(aSeeker, ip, port, file);
					if(port == 0) port = 412;
					s.connect(ip, port);
					for(SearchResult::Iter i = l.begin(); i != l.end(); ++i) {
						SearchResult* sr = *i;
						sprintf(buf, "$SR %s %s%c%s %d/%d%c%s (%s)", aClient->getNick().c_str(), sr->getFile().c_str(), 5,
							Util::toString(sr->getSize()).c_str(), sr->getFreeSlots(), sr->getSlots(), 5, sr->getHubName().c_str(), sr->getHubAddress().c_str());
						s.write(buf, strlen(buf));
					}
				} catch(SocketException /* e */) {
					dcdebug("Search caught error\n");
				}
				delete[] buf;

				for(SearchResult::Iter i = l.begin(); i != l.end(); ++i) {
					delete *i;
				}
			}
		}
	}
}

User::Ptr& ClientManager::getUser(const string& aNick, const string& aHint /* = Util::emptyString */) {
	Lock l(cs);
	dcassert(aNick.size() > 0);
	UserPair p = users.equal_range(aNick);

	if(p.first == p.second) {
		dcdebug("Allocating %d bytes for user %s (#%d)\n", sizeof(User), aNick.c_str(), users.size());
		User::Ptr& u = users.insert(make_pair(aNick, new User(aNick)))->second;
		u->setLastHubIp(aHint);
		return u;
	}

	if(aHint.empty()) {
		// No hint, return the first user with this nick...
		return p.first->second;
	}

	// Since we have a hint, make sure we use it...
	UserIter i;
	for(i = p.first; i != p.second; ++i) {
		if(i->second->getLastHubIp() == aHint) {
			return i->second;
		}
	}

	for(i = p.first; i != p.second; ++i) {
		if(i->second->getLastHubIp().empty()) {
			i->second->setLastHubIp(aHint);
			return i->second;
		}
	}

	User::Ptr& u = users.insert(make_pair(aNick, new User(aNick)))->second;
	u->setLastHubIp(aHint);
	return u;
}

User::Ptr& ClientManager::getUser(const string& aNick, Client* aClient, bool putOnline /* = true */) {
	Lock l(cs);
	dcassert(aNick.size() > 0);
	dcassert(aClient != NULL);
	dcassert(find(clients.begin(), clients.end(), aClient) != clients.end());

	UserPair p = users.equal_range(aNick);
	UserIter i;

	// Check for a user already online
	for(i = p.first; i != p.second; ++i) {
		if(i->second->isClient(aClient)) {
			return i->second;
		}
	}

	// Check for an offline user that was on that hub that we can put online again
	for(i = p.first; i != p.second; ++i) {
		if( (!i->second->isOnline()) && (i->second->getLastHubIp() == aClient->getIp()) ) {
			if(putOnline) {
				i->second->setClient(aClient);
				fire(ClientManagerListener::USER_UPDATED, i->second);
			}
			return i->second;
		}
	}

	// Check for an offline user that was not on another hub
	for(i = p.first; i != p.second; ++i) {
		if( (!i->second->isOnline()) && i->second->getLastHubIp().empty() ) {
			if(putOnline) {
				i->second->setClient(aClient);
				fire(ClientManagerListener::USER_UPDATED, i->second);
			}
			return i->second;
		}
	}
	
	// Create a new user
	i = users.insert(make_pair(aNick, new User(aNick)));
	if(putOnline) {
		i->second->setClient(aClient);
		fire(ClientManagerListener::USER_UPDATED, i->second);
	}
	return i->second;
}

void ClientManager::onTimerMinute(u_int8_t aTick) {
	if(minutes++ >= 5) {
		minutes = 0;
		Lock l(cs);
		UserIter i = users.begin();
		while(i != users.end()) {
			if(i->second->unique()) {
				users.erase(i++);
			} else {
				++i;
			}
		}

		for(Client::Iter j = clients.begin(); j != clients.end(); ++j) {
			if((*j)->lastUpdate + 10 * 1000 < aTick && (*j)->lastHubs != Client::hubs) {
				(*j)->myInfo((*j)->getNick(), SETTING(DESCRIPTION), SETTING(CONNECTION), SETTING(EMAIL), ShareManager::getInstance()->getShareSizeString());
			}
		}
	}
	
}

/**
 * @file ClientManager.cpp
 * $Id: ClientManager.cpp,v 1.23 2002/05/09 15:26:46 arnetheduck Exp $
 */


/* 
 * Copyright (C) 2001-2003 Jacek Sieka, j_s@telia.com
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
#include "CryptoManager.h"
#include "ConnectionManager.h"

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
	aClient->disconnect();
	aClient->removeListeners();

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
	if(aUser->getNick() == aClient->getNick() && aClient->getFirstHello()) {
		aClient->version(SETTING(CLIENTVERSION));
		aClient->getNickList();
		aClient->myInfo();
	} else {
		//aClient->getInfo(aUser);
	}
}

void ClientManager::infoUpdated() {
	Lock l(cs);
	for(Client::Iter i = clients.begin(); i != clients.end(); ++i) {
		if((*i)->isConnected()) {
			(*i)->myInfo();
		}
	}
}

void ClientManager::onClientSearch(Client* aClient, const string& aSeeker, int aSearchType, const string& aSize, 
									int aFileType, const string& aString) throw() {
	
	// Filter own searches
	if(SETTING(CONNECTION_TYPE) == SettingsManager::CONNECTION_ACTIVE) {
		if(aSeeker.find(aClient->getLocalIp()) != string::npos) {
			return;
		}
	} else {
		if(aSeeker.find(aClient->getNick()) != string::npos) {
			return;
		}
	}
	
	string::size_type pos = aSeeker.find("Hub:");
	// We don't wan't to answer passive searches if we're in passive mode...
	if(pos != string::npos && SETTING(CONNECTION_TYPE) != SettingsManager::CONNECTION_ACTIVE) {
		return;
	}
	
	SearchResult::List l = ShareManager::getInstance()->search(aString, aSearchType, aSize, aFileType, aClient, (pos == string::npos) ? 10 : 5);
//		dcdebug("Found %d items (%s)\n", l.size(), aString.c_str());
	if(l.size() > 0) {
		if(pos != string::npos) {
			string name = aSeeker.substr(4);
			// Good, we have a passive seeker, those are easier...
			string str;
			char* buf = new char[4096];
			for(SearchResult::Iter i = l.begin(); i != l.end(); ++i) {
				SearchResult* sr = *i;
				if(sr->getType() == SearchResult::TYPE_FILE) {
					sprintf(buf, "$SR %s %s%c%s %d/%d%c%s (%s)%c%s|", aClient->getNick().c_str(), sr->getFile().c_str(), 5,
						Util::toString(sr->getSize()).c_str(), sr->getFreeSlots(), sr->getSlots(), 5, sr->getHubName().c_str(), sr->getHubAddress().c_str(), 5, name.c_str());
				} else {
					dcassert(sr->getType() == SearchResult::TYPE_DIRECTORY);
					dcassert(sr->getFile().size() > 0);
					string fname = sr->getFile().substr(0, sr->getFile().length() - 1);
					sprintf(buf, "$SR %s %s %d/%d%c%s (%s)%c%s|", aClient->getNick().c_str(), fname.c_str(), 
						sr->getFreeSlots(), sr->getSlots(), 5, sr->getHubName().c_str(), sr->getHubAddress().c_str(), 5, name.c_str());
				}
				str += buf;
				delete sr;
			}
			delete[] buf;
			
			if(str.size() > 0)
				aClient->searchResults(str);
			
		} else {
			char* buf = new char[4096];

			try {
				string ip, file;
				short port = 0;
				Util::decodeUrl(aSeeker, ip, port, file);
				ip = Socket::resolve(ip);
				if(port == 0) port = 412;
				for(SearchResult::Iter i = l.begin(); i != l.end(); ++i) {
					SearchResult* sr = *i;
					if(sr->getType() == SearchResult::TYPE_FILE) {
						sprintf(buf, "$SR %s %s%c%s %d/%d%c%s (%s)", aClient->getNick().c_str(), sr->getFile().c_str(), 5,
							Util::toString(sr->getSize()).c_str(), sr->getFreeSlots(), sr->getSlots(), 5, sr->getHubName().c_str(), sr->getHubAddress().c_str());
					} else {
						dcassert(sr->getType() == SearchResult::TYPE_DIRECTORY);
						dcassert(sr->getFile().size() > 0);
						string fname = sr->getFile().substr(0, sr->getFile().length() - 1);
						sprintf(buf, "$SR %s %s %d/%d%c%s (%s)", aClient->getNick().c_str(), fname.c_str(), 
							sr->getFreeSlots(), sr->getSlots(), 5, sr->getHubName().c_str(), sr->getHubAddress().c_str());
					}
					int len = strlen(buf);
					if(len > 0 && len < 1400)
						s.writeTo(ip, port, buf, len);
				}
			} catch(const SocketException& /* e */) {
				dcdebug("Search caught error\n");
			}
			delete[] buf;

			for_each(l.begin(), l.end(), DeleteFunction<SearchResult*>());
		}
	}
}

User::Ptr ClientManager::getUser(const string& aNick, const string& aHint /* = Util::emptyString */) {
	Lock l(cs);
	dcassert(aNick.size() > 0);
	UserPair p = users.equal_range(aNick);

	if(p.first == p.second) {
		User::Ptr& u = users.insert(make_pair(aNick, new User(aNick)))->second;
		u->setLastHubAddress(aHint);
		return u;
	}

	UserIter i;
	if(aHint.empty()) {
		// No hint, first, try finding an online user...
		for(i = p.first; i != p.second; ++i) {
			if(i->second->isOnline()) {
				return i->second;
			}
		}
		// Blah...return the first one...doesn't matter now...
		return p.first->second;
	}

	// Since we have a hint, make sure we use it...
	for(i = p.first; i != p.second; ++i) {
		if(i->second->getLastHubAddress() == aHint) {
			return i->second;
		}
	}
	// Since old dc++'s didn't return port in $SR's we'll check for port-less hints as well
	string::size_type k = aHint.find(':');
	if(k != string::npos) {
		string hint = aHint.substr(0, k); 
		for(i = p.first; i != p.second; ++i) {
			if(i->second->getLastHubAddress() == hint) {
				return i->second;
			}
		}
	}
	
	// Try to find an online user, higher probablility that it's one of these...
	for(i = p.first; i != p.second; ++i) {
		if(i->second->isOnline()) {
			return i->second;
		}
	}

	return users.insert(make_pair(aNick, new User(aNick)))->second;
}

User::Ptr ClientManager::getUser(const string& aNick, Client* aClient, bool putOnline /* = true */) {
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

	string tmpAddress = (aClient->getPort() == 411) ? aClient->getIp() : aClient->getIp() + ':' + Util::toString(aClient->getPort());
	// Check for an offline user that was on that hub that we can put online again
	for(i = p.first; i != p.second; ++i) {
		if( (!i->second->isOnline()) && (i->second->getLastHubAddress() == tmpAddress) ) {
			if(putOnline) {
				i->second->setClient(aClient);
				fire(ClientManagerListener::USER_UPDATED, i->second);
			}
			return i->second;
		}
	}

	// Check for any offline user
	for(i = p.first; i != p.second; ++i) {
		if( (!i->second->isOnline()) ) {
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

void ClientManager::onTimerMinute(u_int32_t /* aTick */) {
	Lock l(cs);

	// Collect some garbage...
	UserIter i = users.begin();
	while(i != users.end()) {
		if(i->second->unique()) {
			users.erase(i++);
		} else {
			++i;
		}
	}

	for(Client::Iter j = clients.begin(); j != clients.end(); ++j) {
		if((*j)->lastCounts != Client::counts) {
			(*j)->myInfo();
		}
	}
}

// ClientListener
void ClientManager::onAction(ClientListener::Types type, Client* client, const string& line1, const string& line2) throw() {
	switch(type) {
	case ClientListener::C_LOCK:
		client->key(CryptoManager::getInstance()->makeKey(line1));
		client->validateNick(client->getNick());
		break;
	case ClientListener::CONNECT_TO_ME:
		ConnectionManager::getInstance()->connect(line1, (short)Util::toInt(line2), client->getNick()); break;
	default:
		break;
	}
}
void ClientManager::onAction(ClientListener::Types type, Client* client, const User::Ptr& user) throw() {
	switch(type) {
	case ClientListener::HELLO:
		onClientHello(client, user); break;
	case ClientListener::REV_CONNECT_TO_ME:
		if(SETTING(CONNECTION_TYPE) == SettingsManager::CONNECTION_ACTIVE) {
			client->connectToMe(user);
		}
		break;
	default:
		break;		
	}
}
void ClientManager::onAction(ClientListener::Types type, Client* client, const User::List& aList) throw() {
	switch(type) {
	case ClientListener::NICK_LIST:
		{
			string tmp;
			// Let's assume 16 bytes / getinfo...
			tmp.reserve(aList.size() * 16); 
			for(User::List::const_iterator i = aList.begin(); i != aList.end(); ++i) {
				//tmp += "$GetINFO " + (*i)->getNick() + '|';
				client->getInfo(*i);
			}
			//if(!tmp.empty()) {
			//	client->send(tmp);
			//}
		} break;
	case ClientListener::OP_LIST:
		{
			for(User::List::const_iterator i = aList.begin(); i != aList.end(); ++i) {
				if((*i)->getNick() == client->getNick())
					client->setOp(true);
			}
		}
		break;
	default:
		break;
	}
}

void ClientManager::onAction(ClientListener::Types type, Client* aClient, const string& aSeeker, int aSearchType, const string& aSize, 
					  int aFileType, const string& aString) throw() {
	switch(type) {
	case ClientListener::SEARCH:
		fire(ClientManagerListener::INCOMING_SEARCH, aString);
		onClientSearch(aClient, aSeeker, aSearchType, aSize, aFileType, aString);
		break;
	default:
		break;
	}
}

// TimerManagerListener
void ClientManager::onAction(TimerManagerListener::Types type, u_int32_t aTick) throw() {
	if(type == TimerManagerListener::MINUTE) {
		onTimerMinute(aTick);
	}
}

/**
 * @file
 * $Id: ClientManager.cpp,v 1.37 2003/09/22 13:17:22 arnetheduck Exp $
 */

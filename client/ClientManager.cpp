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
#include "HubManager.h"

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
	fire(ClientManagerListener::CLIENT_DISCONNECTED, aClient);
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
	if(aUser == aClient->getMe() && aClient->getFirstHello()) {
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
	
	SearchResult::List l;
	ShareManager::getInstance()->search(l, aString, aSearchType, aSize, aFileType, aClient, (pos == string::npos) ? 10 : 5);
//		dcdebug("Found %d items (%s)\n", l.size(), aString.c_str());
	if(l.size() > 0) {
		if(pos != string::npos) {
			string name = aSeeker.substr(4);
			// Good, we have a passive seeker, those are easier...
			string str;
			for(SearchResult::Iter i = l.begin(); i != l.end(); ++i) {
				SearchResult* sr = *i;
				str += sr->toSR();
				str[str.length()-1] = 5;
				str += name;
				str += '|';

				sr->decRef();
			}
			
			if(str.size() > 0)
				aClient->searchResults(str);
			
		} else {
			try {
				string ip, file;
				short port = 0;
				Util::decodeUrl(aSeeker, ip, port, file);
				ip = Socket::resolve(ip);
				if(port == 0) port = 412;
				for(SearchResult::Iter i = l.begin(); i != l.end(); ++i) {
					SearchResult* sr = *i;
					s.writeTo(ip, port, sr->toSR());
					sr->decRef();
				}
			} catch(const SocketException& /* e */) {
				dcdebug("Search caught error\n");
			}
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

	// Check for an offline user that was on that hub that we can put online again
	for(i = p.first; i != p.second; ++i) {
		if( (!i->second->isOnline()) && 
			((i->second->getLastHubAddress() == aClient->getAddressPort()) || (i->second->getLastHubAddress() == aClient->getIpPort())) )
		{
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

void ClientManager::onClientLock(Client* client, const string& aLock) throw() {
	if(CryptoManager::getInstance()->isExtended(aLock)) {
		StringList feat = features;
		if(BOOLSETTING(HASH_FILES))
			feat.push_back("TTHSearch");
		if(BOOLSETTING(COMPRESS_TRANSFERS))
			feat.push_back("GetZBlock");
		client->supports(feat);
	}
	client->key(CryptoManager::getInstance()->makeKey(aLock));
	client->validateNick(client->getNick());
}

// ClientListener
void ClientManager::onAction(ClientListener::Types type, Client* client) throw() {
	if(type == ClientListener::CONNECTED) {
		fire(ClientManagerListener::CLIENT_CONNECTED, client);
	}
}

void ClientManager::onAction(ClientListener::Types type, Client* client, const string& /*line*/) throw() {
	if(type == ClientListener::FAILED) {
		HubManager::getInstance()->removeUserCommand(client->getAddressPort());
		fire(ClientManagerListener::CLIENT_DISCONNECTED, client);
	} else if(type == ClientListener::HUB_NAME) {
		fire(ClientManagerListener::CLIENT_UPDATED, client);
	}
}

void ClientManager::onAction(ClientListener::Types type, Client* client, int aType, int ctx, const string& name, const string& command) throw() {
	if(type == ClientListener::USER_COMMAND) {
		if(BOOLSETTING(HUB_USER_COMMANDS)) {
			HubManager::getInstance()->addUserCommand(aType, ctx, UserCommand::FLAG_NOSAVE, name, command, client->getAddressPort());
		}
	}
}

void ClientManager::onAction(ClientListener::Types type, Client* client, const string& line1, const string& line2) throw() {
	switch(type) {
	case ClientListener::C_LOCK:
		onClientLock(client, line1);
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
		if(!(client->getSupportFlags() & Client::SUPPORTS_NOGETINFO)) {
			string tmp;
			// Let's assume 10 characters per nick...
			tmp.reserve(aList.size() * (11 + 10 + client->getNick().length())); 
			for(User::List::const_iterator i = aList.begin(); i != aList.end(); ++i) {
				tmp += "$GetINFO ";
				tmp += (*i)->getNick();
				tmp += ' ';
				tmp += client->getNick(); 
				tmp += '|';
			}
			if(!tmp.empty()) {
				client->send(tmp);
			}
		} break;
	case ClientListener::OP_LIST:
		fire(ClientManagerListener::CLIENT_UPDATED, client);
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
 * $Id: ClientManager.cpp,v 1.50 2004/03/12 08:20:59 arnetheduck Exp $
 */

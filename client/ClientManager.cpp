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

#include "AdcHub.h"
#include "NmdcHub.h"

Client* ClientManager::getClient(const string& aHubURL) {
	Client* c;
	if(Util::strnicmp("adc://", aHubURL.c_str(), 6) == 0) {
		c = new AdcHub(aHubURL);
	} else {
		c = new NmdcHub(aHubURL);
	}

	{
		Lock l(cs);
		clients.push_back(c);
	}

	c->addListener(this);
	return c;
}

void ClientManager::putClient(Client* aClient) {
	aClient->disconnect();
	fire(ClientManagerListener::ClientDisconnected(), aClient);
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

void ClientManager::infoUpdated() {
	Lock l(cs);
	for(Client::Iter i = clients.begin(); i != clients.end(); ++i) {
		if((*i)->isConnected()) {
			(*i)->info();
		}
	}
}

void ClientManager::on(NmdcSearch, Client* aClient, const string& aSeeker, int aSearchType, int64_t aSize, 
									int aFileType, const string& aString) throw() 
{
	
	bool isPassive = (aSeeker.compare(0, 4, "Hub:") == 0);

	// We don't wan't to answer passive searches if we're in passive mode...
	if(isPassive && SETTING(CONNECTION_TYPE) != SettingsManager::CONNECTION_ACTIVE) {
		return;
	}
	
	SearchResult::List l;
	ShareManager::getInstance()->search(l, aString, aSearchType, aSize, aFileType, aClient, isPassive ? 5 : 10);
//		dcdebug("Found %d items (%s)\n", l.size(), aString.c_str());
	if(l.size() > 0) {
		if(isPassive) {
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
				aClient->send(str);
			
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
				fire(ClientManagerListener::UserUpdated(), i->second);
			}
			return i->second;
		}
	}

	// Check for any offline user
	for(i = p.first; i != p.second; ++i) {
		if( (!i->second->isOnline()) ) {
			if(putOnline) {
				i->second->setClient(aClient);
				fire(ClientManagerListener::UserUpdated(), i->second);
			}
			return i->second;
		}
	}
	
	// Create a new user
	i = users.insert(make_pair(aNick, new User(aNick)));
	if(putOnline) {
		i->second->setClient(aClient);
		fire(ClientManagerListener::UserUpdated(), i->second);
	}
	return i->second;
}

void ClientManager::putUserOffline(User::Ptr& aUser, bool quitHub /*= false*/) {
	{
		Lock l(cs);
		aUser->setIp(Util::emptyString);
		aUser->unsetFlag(User::PASSIVE);
		aUser->unsetFlag(User::OP);
		aUser->unsetFlag(User::DCPLUSPLUS);
		if(quitHub)
			aUser->setFlag(User::QUIT_HUB);
		aUser->setClient(NULL);
	}
	fire(ClientManagerListener::UserUpdated(), aUser);
}


User::Ptr ClientManager::getUser(const CID& cid, bool createUser) {
	Lock l(cs);
	dcassert(!cid.isZero());
	AdcIter i = adcUsers.find(cid);
	if(i != adcUsers.end())
		return i->second;

	return createUser ? adcUsers.insert(make_pair(cid, new User(cid)))->second : User::Ptr();
}

User::Ptr ClientManager::getUser(const CID& cid, Client* aClient, bool putOnline /* = true */) {
	Lock l(cs);
	dcassert(!cid.isZero());
	dcassert(aClient != NULL && find(clients.begin(), clients.end(), aClient) != clients.end());

	AdcPair p = adcUsers.equal_range(cid);
	for(AdcIter i = p.first; i != p.second; ++i) {
		User::Ptr& u = i->second;
		if(u->isClient(aClient))
			return u;
	}

	if(putOnline) {
		User::Ptr up;
		for(AdcIter i = p.first; i != p.second; ++i) {
			if(!i->second->isOnline()) {
				up = i->second;
			}
		}
		if(!up)
			up = adcUsers.insert(make_pair(cid, new User(cid)))->second;

		up->setClient(aClient);
		fire(ClientManagerListener::UserUpdated(), up);
		return up;
	}

	// Create a new user
	return adcUsers.insert(make_pair(cid, new User(cid)))->second;
}

void ClientManager::on(TimerManagerListener::Minute, u_int32_t /* aTick */) throw() {
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
	AdcIter k = adcUsers.begin();
	while(k != adcUsers.end()) {
		if(k->second->unique()) {
			adcUsers.erase(k++);
		} else {
			++k;
		}
	}

	for(Client::Iter j = clients.begin(); j != clients.end(); ++j) {
		(*j)->info();
	}
}

void ClientManager::on(Failed, Client* client, const string&) throw() { 
	HubManager::getInstance()->removeUserCommand(client->getAddressPort());
	fire(ClientManagerListener::ClientDisconnected(), client);
}

void ClientManager::on(UserCommand, Client* client, int aType, int ctx, const string& name, const string& command) throw() { 
	if(BOOLSETTING(HUB_USER_COMMANDS)) {
		HubManager::getInstance()->addUserCommand(aType, ctx, ::UserCommand::FLAG_NOSAVE, name, command, client->getAddressPort());
	}
}
/*
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
*/

/**
 * @file
 * $Id: ClientManager.cpp,v 1.57 2004/05/03 12:38:04 arnetheduck Exp $
 */

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

#include "ClientManager.h"
#include "ShareManager.h"
#include "SearchManager.h"

ClientManager* ClientManager::instance = NULL;

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
		dcassert(find(clients.begin(), clients.end(), aClient) != clients.end());
		clients.erase(find(clients.begin(), clients.end(), aClient));
		
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
		int pos = aSeeker.find("Hub:");
		// We don't wan't to answer passive searches if we're in passive mode...
		if(pos != string::npos && SETTING(CONNECTION_TYPE) == SettingsManager::CONNECTION_PASSIVE) {
			return;
		}
		
		SearchResult::List l = ShareManager::getInstance()->search(aString, aSearchType, aSize, aFileType, aClient);
//		dcdebug("Found %d items (%s)\n", l.size(), aString.c_str());
		if(l.size() > 0) {
			if(pos != string::npos) {
				string name = aSeeker.substr(4);
				// Good, we have a passive seeker, those are easier...
				string str;
				char* buf = new char[1024];
				for(SearchResult::Iter i = l.begin(); i != l.end(); ++i) {
					SearchResult* sr = *i;
					sprintf(buf, "$SR %s %s%c%I64d %d/%d%c%s (%s)%c%s|", aClient->getNick().c_str(), sr->getFile().c_str(), 5,
						sr->getSize(), sr->getFreeSlots(), sr->getSlots(), 5, sr->getHubName().c_str(), sr->getHubAddress().c_str(), 5, name.c_str());
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
					short port = (short)SETTING(PORT);
					Util::decodeUrl(aSeeker, ip, port, file);
					s.connect(ip, port);
					for(SearchResult::Iter i = l.begin(); i != l.end(); ++i) {
						SearchResult* sr = *i;
						sprintf(buf, "$SR %s %s%c%I64d %d/%d%c%s (%s)", aClient->getNick().c_str(), sr->getFile().c_str(), 5,
							sr->getSize(), sr->getFreeSlots(), sr->getSlots(), 5, sr->getHubName().c_str(), sr->getHubAddress().c_str());
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
		return users.insert(make_pair(aNick, new User(aNick)))->second;
	}

	for(UserIter i = p.first; i != p.second; ++i) {
		if(i->second->getLastHubIp() == aHint) {
			return i->second;
		}
	}

	return p.first->second;
}

User::Ptr& ClientManager::getUser(const string& aNick, Client* aClient, bool putOnline /* = true */) {
	Lock l(cs);
	dcassert(aNick.size() > 0);
	dcassert(aClient != NULL);
	dcassert(find(clients.begin(), clients.end(), aClient) != clients.end());

	UserPair p = users.equal_range(aNick);
	
	// Check for a user already online
	for(UserIter i = p.first; i != p.second; ++i) {
		if(i->second->isClient(aClient)) {
			return i->second;
		}
	}

	// Check for an offline user that was on that hub that we can put online again
	for(UserIter j = p.first; j != p.second; ++j) {
		if( (!j->second->isOnline()) && (j->second->getLastHubIp() == aClient->getIp()) ) {
			if(putOnline) {
				j->second->setClient(aClient);
				fire(ClientManagerListener::USER_UPDATED, j->second);
			}
			return j->second;
		}
	}

	// Check for any offline user that we can put online again
	for(UserIter m = p.first; m != p.second; ++m) {
		if(!m->second->isOnline()) {
			if(putOnline) {
				m->second->setClient(aClient);
				fire(ClientManagerListener::USER_UPDATED, m->second);
			}
			return m->second;
		}
	}
	
	// Create a new user
	UserIter k = users.insert(make_pair(aNick, new User(aNick)));
	if(putOnline) {
		k->second->setClient(aClient);
		fire(ClientManagerListener::USER_UPDATED, k->second);
	}
	return k->second;
}

void ClientManager::onTimerMinute(DWORD aTick) {
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
 * $Id: ClientManager.cpp,v 1.16 2002/04/07 16:08:14 arnetheduck Exp $
 * @if LOG
 * $Log: ClientManager.cpp,v $
 * Revision 1.16  2002/04/07 16:08:14  arnetheduck
 * Fixes and additions
 *
 * Revision 1.15  2002/03/25 22:23:24  arnetheduck
 * Lots of minor updates
 *
 * Revision 1.14  2002/03/13 23:06:07  arnetheduck
 * New info sent in the description part of myinfo...
 *
 * Revision 1.13  2002/03/10 22:41:08  arnetheduck
 * Working on internationalization...
 *
 * Revision 1.12  2002/03/04 23:52:30  arnetheduck
 * Updates and bugfixes, new user handling almost finished...
 *
 * Revision 1.11  2002/02/28 00:10:47  arnetheduck
 * Some fixes to the new user model
 *
 * Revision 1.10  2002/02/27 12:02:09  arnetheduck
 * Completely new user handling, wonder how it turns out...
 *
 * Revision 1.9  2002/02/18 23:48:32  arnetheduck
 * New prerelease, bugs fixed and features added...
 *
 * Revision 1.8  2002/02/12 00:35:37  arnetheduck
 * 0.153
 *
 * Revision 1.7  2002/01/25 00:11:26  arnetheduck
 * New settings dialog and various fixes
 *
 * Revision 1.6  2002/01/20 22:54:46  arnetheduck
 * Bugfixes to 0.131 mainly...
 *
 * Revision 1.5  2002/01/17 23:35:59  arnetheduck
 * Reworked threading once more, now it actually seems stable. Also made
 * sure that noone tries to access client objects that have been deleted
 * as well as some other minor updates
 *
 * Revision 1.4  2002/01/13 22:50:47  arnetheduck
 * Time for 0.12, added favorites, a bunch of new icons and lot's of other stuff
 *
 * Revision 1.3  2002/01/11 14:52:56  arnetheduck
 * Huge changes in the listener code, replaced most of it with templates,
 * also moved the getinstance stuff for the managers to a template
 *
 * Revision 1.2  2002/01/07 20:17:59  arnetheduck
 * Finally fixed the reconnect bug that's been annoying me for a whole day...
 * Hopefully the app works better in w95 now too...
 *
 * Revision 1.1  2001/12/21 18:46:18  arnetheduck
 * Replaces ProtocolHandler with enhanced functionality
 *
 * @endif
 */


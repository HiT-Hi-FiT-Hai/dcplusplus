/* 
 * Copyright (C) 2001 Jacek Sieka, jacek@creatio.se
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
	cs.enter();
	Client* c = new Client();

	c->addListener(this);
	clients.push_back(c);
	cs.leave();
	return c;
}

void ClientManager::putClient(Client* aClient) {
	cs.enter();
	aClient->removeListeners();
	aClient->disconnect();

	for(Client::Iter i = clients.begin(); i != clients.end(); ++i) {
		if( (*i) == aClient) {
			clients.erase(i);
			break;
		}
	}
	aClient->cs.enter();		// Just make sure noone's using it still...
	aClient->cs.leave();	
	delete aClient;
	cs.leave();
}

void ClientManager::onClientHello(Client* aClient, const User::Ptr& aUser) throw() {
	aClient->cs.enter();
	if(aUser->getNick() == Settings::getNick()) {
		aClient->version("1,0091");
		aClient->getNickList();
		aClient->myInfo(Settings::getNick(), Settings::getDescription(), Settings::getConnection(), Settings::getEmail(), ShareManager::getInstance()->getShareSizeString());
	} else {
		aClient->getInfo(aUser);
	}
	aClient->cs.leave();
}

void ClientManager::onClientSearch(Client* aClient, const string& aSeeker, int aSearchType, const string& aSize, 
							int aFileType, const string& aString) throw() {
	
	bool search = false;
	if(Settings::getConnectionType() == Settings::CONNECTION_ACTIVE) {
		if(aSeeker.find(Settings::getServer().empty() ? Util::getLocalIp() : Settings::getServer()) == string::npos) {
			search = true;
		}
	} else {
		if(aSeeker.find(Settings::getNick()) == string::npos) {
			search = true;
		}
	}
	
	if(search) {
		int pos = aSeeker.find("Hub:");
		// We don't wan't to answer passive searches if we're in passive mode...
		if(pos != string::npos && Settings::getConnectionType() == Settings::CONNECTION_PASSIVE) {
			return;
		}
		
		SearchResult::List l = ShareManager::getInstance()->search(aString, aSearchType, aSize, aFileType);
		dcdebug("Found %d items (%s)\n", l.size(), aString.c_str());
		if(pos != string::npos) {
			string name = aSeeker.substr(4);
			// Good, we have a passive seeker, those are easier...
			string str;
			for(SearchResult::Iter i = l.begin(); i != l.end(); ++i) {
				char buf[512];
				SearchResult* sr = *i;
				sprintf(buf, "$SR %s %s%c%I64d %d/%d%c%s (%s)%c%s|", Settings::getNick().c_str(), sr->getFile().c_str(), 5,
					sr->getSize(), sr->getFreeSlots(), sr->getSlots(), 5, sr->getHubName().c_str(), sr->getHubAddress().c_str(), 5, name.c_str());
				str += buf;
				delete sr;
			}
			
			aClient->searchResults(str);
		} else {
			Socket s;
			s.create(Socket::TYPE_UDP);
			string ip, file;
			short port = 412;
			Util::decodeUrl(aSeeker, ip, port, file);
			s.connect(ip, port);
			for(SearchResult::Iter i = l.begin(); i != l.end(); ++i) {
				char buf[512];
				SearchResult* sr = *i;
				sprintf(buf, "$SR %s %s%c%I64d %d/%d%c%s (%s)", Settings::getNick().c_str(), sr->getFile().c_str(), 5,
					sr->getSize(), sr->getFreeSlots(), sr->getSlots(), 5, sr->getHubName().c_str(), sr->getHubAddress().c_str());
				s.write(buf, strlen(buf));
				delete sr;
			}
		}
	}
}

/**
 * @file ClientManager.cpp
 * $Id: ClientManager.cpp,v 1.3 2002/01/11 14:52:56 arnetheduck Exp $
 * @if LOG
 * $Log: ClientManager.cpp,v $
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


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
		aClient->version("1,0091");
		aClient->getNickList();
		aClient->myInfo(aClient->getNick(), SETTING(DESCRIPTION), SETTING(CONNECTION), SETTING(EMAIL), ShareManager::getInstance()->getShareSizeString());
	} else {
		aClient->getInfo(aUser);
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
				for(SearchResult::Iter i = l.begin(); i != l.end(); ++i) {
					char buf[512];
					SearchResult* sr = *i;
					sprintf(buf, "$SR %s %s%c%I64d %d/%d%c%s (%s)%c%s|", aClient->getNick().c_str(), sr->getFile().c_str(), 5,
						sr->getSize(), sr->getFreeSlots(), sr->getSlots(), 5, sr->getHubName().c_str(), sr->getHubAddress().c_str(), 5, name.c_str());
					str += buf;
					delete sr;
				}
				
				if(str.size() > 0)
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
					sprintf(buf, "$SR %s %s%c%I64d %d/%d%c%s (%s)", aClient->getNick().c_str(), sr->getFile().c_str(), 5,
						sr->getSize(), sr->getFreeSlots(), sr->getSlots(), 5, sr->getHubName().c_str(), sr->getHubAddress().c_str());
					s.write(buf, strlen(buf));
					delete sr;
				}
			}
		}
	}
}

/**
 * @file ClientManager.cpp
 * $Id: ClientManager.cpp,v 1.6 2002/01/20 22:54:46 arnetheduck Exp $
 * @if LOG
 * $Log: ClientManager.cpp,v $
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


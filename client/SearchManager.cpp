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

#include "SearchManager.h"
#include "ClientManager.h"

SearchManager* Singleton<SearchManager>::instance = NULL;

void SearchManager::search(const string& aName, int64_t aSize, TypeModes aTypeMode /* = TYPE_ANY */, SizeModes aSizeMode /* = SIZE_ATLEAST */) {
	ClientManager::getInstance()->search(aSizeMode, aSize, aTypeMode, aName);
}

void SearchManager::setPort(short aPort) throw(SocketException) {
	if(socket != NULL) {
		socket->disconnect();
		join();
	} else {
		socket = new Socket();
	}

	socket->create(Socket::TYPE_UDP);
	socket->bind(aPort);
	start();
}

int SearchManager::run() {
	static const int bufsize = 4096;

	u_int8_t* buf = new u_int8_t[bufsize];
	int len;
	try {
		while( (len = socket->read(buf, bufsize)) != 0) {
			onData(buf, len);
		}
	} catch(SocketException e) {
		dcdebug("SearchManager::run Stopped listening: %s\n", e.getError());
		return 1;
	}

	delete buf;
	return 0;
}

void SearchManager::onData(const u_int8_t* buf, int aLen) {
	string x((char*)buf, aLen);
	if(x.find("$SR") != string::npos) {
		SearchResult sr;
		
		string::size_type i, j;
		string nick;
		// Find out if this is a file or directory...skip the directories for now...
		if(x.find('/') > x.find((char)5)) {
			i = 4;
			if( (j = x.find(' ', i)) == string::npos) {
				return;
			}
			nick = x.substr(i, j-i);
			i = j + 1;
			if( (j = x.find((char)5, i)) == string::npos) {
				return;
			}
			sr.setFile(x.substr(i, j-i));
			i = j + 1;
			if( (j = x.find(' ', i)) == string::npos) {
				return;
			}
			sr.setSize(x.substr(i, j-i));
			i = j + 1;
			if( (j = x.find('/', i)) == string::npos) {
				return;
			}
			sr.setFreeSlots(x.substr(i, j-i));
			i = j + 1;
			if( (j = x.find((char)5, i)) == string::npos) {
				return;
			}
			sr.setSlots(x.substr(i, j-i));
			i = j + 1;
			if( (j = x.rfind(" (")) == string::npos) {
				return;
			}
			sr.setHubName(x.substr(i, j-i));
			i = j + 2;
			if( (j = x.rfind(')')) == string::npos) {
				return;
			}
			sr.setHubAddress(x.substr(i, j-i));

			sr.setUser(ClientManager::getInstance()->getUser(nick, sr.getHubAddress()));
			
			fire(SearchManagerListener::SEARCH_RESULT, &sr);
		}
	}
}

/**
 * @file SearchManager.cpp
 * $Id: SearchManager.cpp,v 1.20 2002/05/12 21:54:08 arnetheduck Exp $
 */


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

string SearchResult::getFileName() { 
	if(getType() == TYPE_FILE) 
		return Util::getFileName(getFile()); 

	if(getFile().size() < 2)
		return getFile();

	string::size_type i = getFile().rfind('\\', getFile().length() - 2);
	if(i == string::npos)
		return getFile();

	return getFile().substr(i + 1);
};

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

#define BUFSIZE 8192
int SearchManager::run() {
	
	AutoArray<u_int8_t> buf(BUFSIZE);
	int len;
	try {
		while( (len = socket->read((u_int8_t*)buf, BUFSIZE)) != 0) {
			onData(buf, len);
		}
	} catch(SocketException e) {
		dcdebug("SearchManager::run Stopped listening: %s\n", e.getError().c_str());
		return 1;
	}
	
	return 0;
}

void SearchManager::onData(const u_int8_t* buf, int aLen) {
	string x((char*)buf, aLen);
	if(x.find("$SR") != string::npos) {
		SearchResult sr;
		
		string::size_type i, j;
		string nick;
		// Directories: $SR <nick><0x20><directory><0x20><free slots>/<total slots><0x05><Hubname><0x20>(<Hubip:port>)
		// Files:       $SR <nick><0x20><filename><0x05><filesize><0x20><free slots>/<total slots><0x05><Hubname><0x20>(<Hubip:port>)
		i = 4;
		if( (j = x.find(' ', i)) == string::npos) {
			return;
		}
		nick = x.substr(i, j-i);
		i = j + 1;

		// A file has 2 0x05, a directory only one
		size_t cnt = count(x.begin() + j, x.end(), 0x05);
	
		if(cnt == 1) {
			// We have a directory...find the first space beyond the first 0x05 from the back 
			// (dirs might contain spaces as well...clever protocol, eh?)
			sr.setType(SearchResult::TYPE_DIRECTORY);
			// Get past the hubname that might contain spaces
			if((j = x.rfind(0x05)) == string::npos) {
				return;
			}
			// Find the end of the directory info
			if((j = x.rfind(' ', j-1)) == string::npos) {
				return;
			}
			if(j < i + 1) {
				return;
			}
			sr.setFile(x.substr(i, j-i) + '\\');
		} else if(cnt == 2) {
			sr.setType(SearchResult::TYPE_FILE);

			if( (j = x.find((char)5, i)) == string::npos) {
				return;
			}
			sr.setFile(x.substr(i, j-i));
			i = j + 1;
			if( (j = x.find(' ', i)) == string::npos) {
				return;
			}
			sr.setSize(x.substr(i, j-i));
		}
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

/**
 * @file SearchManager.cpp
 * $Id: SearchManager.cpp,v 1.23 2002/12/28 01:31:49 arnetheduck Exp $
 */


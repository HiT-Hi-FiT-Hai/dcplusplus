/* 
 * Copyright (C) 2001-2004 Jacek Sieka, j_s at telia com
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
#include "UploadManager.h"

#include "ClientManager.h"
#include "AdcCommand.h"

SearchResult::SearchResult(Client* aClient, Types aType, int64_t aSize, const string& aFile, TTHValue* aTTH) :
file(aFile), hubName(aClient->getName()), hubIpPort(aClient->getIpPort()), user(aClient->getMe()), 
size(aSize), type(aType), slots(SETTING(SLOTS)), freeSlots(UploadManager::getInstance()->getFreeSlots()),  
tth(aTTH == NULL ? NULL : new TTHValue(*aTTH)), ref(1) { }

string SearchResult::toSR() const {
	// File:		"$SR %s %s%c%s %d/%d%c%s (%s)|"
	// Directory:	"$SR %s %s %d/%d%c%s (%s)|"
	string tmp;
	tmp.reserve(128);
	tmp.append("$SR ", 4);
	tmp.append(user->getNick());
	tmp.append(1, ' ');
	if(type == TYPE_FILE) {
		tmp.append(file);
		tmp.append(1, '\x05');
		tmp.append(Util::toString(size));
	} else {
		tmp.append(file, 0, file.length() - 1);
	}
	tmp.append(1, ' ');
	tmp.append(Util::toString(freeSlots));
	tmp.append(1, '/');
	tmp.append(Util::toString(slots));
	tmp.append(1, '\x05');
	if(getTTH() == NULL) {
		tmp.append(hubName);
	} else {
		tmp.append("TTH:" + getTTH()->toBase32());
	}
	tmp.append(" (", 2);
	tmp.append(hubIpPort);
	tmp.append(")|", 2);
	return tmp;
}

string SearchResult::toRES() const {
	string tmp;
	tmp.reserve(128);
	tmp.append(user->getCID().toBase32());
	tmp.append(" SI");
	tmp.append(Util::toString(size));
	tmp.append(" SL");
	tmp.append(Util::toString(freeSlots));
	tmp.append(" FN");
	string fn = file;
	string::size_type i = 0;
	while( (i = fn.find('\\', i)) != string::npos ) {
		fn[i] = '/';
	}
	fn.insert(0, "/");

	if(getTTH() != NULL) {
		tmp.append(" TR");
		tmp.append(getTTH()->toBase32());
	}

	tmp.append(1, '\n');
	return tmp;
}

void SearchManager::search(const string& aName, int64_t aSize, TypeModes aTypeMode /* = TYPE_ANY */, SizeModes aSizeMode /* = SIZE_ATLEAST */) {
	ClientManager::getInstance()->search(aSizeMode, aSize, aTypeMode, aName);
}

void SearchManager::search(StringList& who, const string& aName, int64_t aSize /* = 0 */, TypeModes aTypeMode /* = TYPE_ANY */, SizeModes aSizeMode /* = SIZE_ATLEAST */) {
	ClientManager::getInstance()->search(who, aSizeMode, aSize, aTypeMode, aName);
}

string SearchResult::getFileName() const { 
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
	port = aPort;
	if(socket != NULL) {
		disconnect();
	} else {
		socket = new Socket();
	}

	socket->create(Socket::TYPE_UDP);
	socket->bind(aPort);
	start();
}

void SearchManager::disconnect() throw() {
	if(socket != NULL) {
		stop = true;
		socket->disconnect();
		join();
		stop = false;
	}
}

#define BUFSIZE 8192
int SearchManager::run() {
	
	AutoArray<u_int8_t> buf(BUFSIZE);
	int len;

	while(true) {

		string remoteAddr;
		try {
			while( (len = socket->read((u_int8_t*)buf, BUFSIZE, remoteAddr)) != 0) {
				onData(buf, len, remoteAddr);
			}
		} catch(const SocketException& e) {
			dcdebug("SearchManager::run Error: %s\n", e.getError().c_str());
		}
		if(stop) {
			return 0;
		}

		try {
			socket->disconnect();
			socket->create(Socket::TYPE_UDP);
			socket->bind(port);
		} catch(const SocketException& e) {
			// Oops, fatal this time...
			dcdebug("SearchManager::run Stopped listening: %s\n", e.getError().c_str());
			return 1;
		}
	}
	
	return 0;
}

void SearchManager::onData(const u_int8_t* buf, int aLen, const string& address) {
	string x((char*)buf, aLen);
	if(x.compare(0, 4, "$SR ") == 0) {
		string::size_type i, j;
		// Directories: $SR <nick><0x20><directory><0x20><free slots>/<total slots><0x05><Hubname><0x20>(<Hubip:port>)
		// Files:       $SR <nick><0x20><filename><0x05><filesize><0x20><free slots>/<total slots><0x05><Hubname><0x20>(<Hubip:port>)
		i = 4;
		if( (j = x.find(' ', i)) == string::npos) {
			return;
		}
		string nick = x.substr(i, j-i);
		i = j + 1;

		// A file has 2 0x05, a directory only one
		size_t cnt = count(x.begin() + j, x.end(), 0x05);
		
		SearchResult::Types type = SearchResult::TYPE_FILE;
		string file;
		int64_t size = 0;

		if(cnt == 1) {
			// We have a directory...find the first space beyond the first 0x05 from the back 
			// (dirs might contain spaces as well...clever protocol, eh?)
			type = SearchResult::TYPE_DIRECTORY;
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
			file = x.substr(i, j-i) + '\\';
		} else if(cnt == 2) {
			if( (j = x.find((char)5, i)) == string::npos) {
				return;
			}
			file = x.substr(i, j-i);
			i = j + 1;
			if( (j = x.find(' ', i)) == string::npos) {
				return;
			}
			size = Util::toInt64(x.substr(i, j-i));
		}
		i = j + 1;
		
		if( (j = x.find('/', i)) == string::npos) {
			return;
		}
		int freeSlots = Util::toInt(x.substr(i, j-i));
		i = j + 1;
		if( (j = x.find((char)5, i)) == string::npos) {
			return;
		}
		int slots = Util::toInt(x.substr(i, j-i));
		i = j + 1;
		if( (j = x.rfind(" (")) == string::npos) {
			return;
		}
		string hubName = x.substr(i, j-i);
		i = j + 2;
		if( (j = x.rfind(')')) == string::npos) {
			return;
		}
		string hubIpPort = x.substr(i, j-i);
		User::Ptr user = ClientManager::getInstance()->getUser(nick, hubIpPort);

		SearchResult* sr = new SearchResult(user, type, slots, freeSlots, size,
			file, hubName, hubIpPort, address);
		fire(SearchManagerListener::SR(), sr);
		sr->decRef();
	} else if(x.compare(1, 4, "RES ") == 0) {
		Command c(x);
		if(c.getParameters().empty())
			return;

		User::Ptr p = ClientManager::getInstance()->getUser(CID(c.getParameters()[0]), false);
		if(!p)
			return;

		int freeSlots = -1;
		int64_t size = -1;
		string name;

		for(StringIter i = (c.getParameters().begin() + 1); i != c.getParameters().end(); ++i) {
			string& str = *i;
			if(str.compare(0, 2, "FN") == 0) {
				name = str.substr(2);
			} else if(str.compare(0, 2, "SL")) {
				freeSlots = Util::toInt(str.substr(2));
			} else if(str.compare(0, 2, "SI")) {
				size = Util::toInt64(str.substr(2));
			}
		}

		if(!name.empty() && freeSlots != -1 && size != -1) {
			SearchResult::Types type = (name[name.length() - 1] == '/' ? SearchResult::TYPE_DIRECTORY : SearchResult::TYPE_FILE);
			SearchResult* sr = new SearchResult(p, type, 0, freeSlots, size, name, p->getClientName(), "0.0.0.0", NULL);
			fire(SearchManagerListener::SR(), sr);
			sr->decRef();
		}
	}
}

string SearchManager::clean(const string& aSearchString) {
	static const char* badChars = "$|.[]()-_+";
	string::size_type i = aSearchString.find_first_of(badChars);
	if(i == string::npos)
		return aSearchString;

	string tmp = aSearchString;
	// Remove all strange characters from the search string
	do {
		tmp[i] = ' ';
	} while ( (i = tmp.find_first_of(badChars, i)) != string::npos);

	return tmp;
}


/**
 * @file
 * $Id: SearchManager.cpp,v 1.41 2004/09/06 12:32:42 arnetheduck Exp $
 */


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

#include "SearchManager.h"
#include "Client.h"

SearchManager* SearchManager::instance = NULL;

void SearchManager::search(const string& aName, LONGLONG aSize, DWORD aFlags /* = 0 */, int aType /* = 0 */ ) {
	Client::List ls = Client::getList();
	for(Client::Iter i = ls.begin(); i != ls.end(); ++i) {
		try {
			(*i)->search(aType, aSize, aType, aName);
		} catch(Exception e) {
			dcdebug("SearchManager::search caught: %s\n", e.getError().c_str());
		}
	}
}

void SearchManager::onData(BYTE* buf, int aLen) {
	string x((char*)buf, aLen);
	if(x.find("$SR") != string::npos) {
		SearchResult* sr=new SearchResult();
		
		x = x.substr(4);
		sr->setNick(x.substr(0, x.find(' ')));
		x = x.substr(x.find(' ') + 1);
		sr->setFile(x.substr(0, x.find((char)5)));
		x = x.substr(x.find((char)5) + 1);
		sr->setSize(x.substr(0, x.find(' ')));
		x = x.substr(x.find(' ') + 1);
		sr->setFreeSlots(x.substr(0, x.find('/')));
		x = x.substr(x.find('/') + 1);
		sr->setSlots(x.substr(0, x.find((char)5)));
		x = x.substr(x.find((char)5)+1);
		sr->setHubName(x.substr(0, x.rfind(" (")));
		x = x.substr(x.rfind(" (")+2);
		sr->setHubAddress(x.substr(0, x.find(')')));

		fireResult(sr);
		delete sr;
	}
	dcdebug("Search: %s\n", x.c_str());
}

/**
 * @file SearchManager.cpp
 * $Id: SearchManager.cpp,v 1.4 2001/12/13 19:21:57 arnetheduck Exp $
 * @if LOG
 * $Log: SearchManager.cpp,v $
 * Revision 1.4  2001/12/13 19:21:57  arnetheduck
 * A lot of work done almost everywhere, mainly towards a friendlier UI
 * and less bugs...time to release 0.06...
 *
 * Revision 1.3  2001/12/08 20:59:26  arnetheduck
 * Fixing bugs...
 *
 * Revision 1.2  2001/12/08 14:25:49  arnetheduck
 * More bugs removed...did my first search as well...
 *
 * Revision 1.1  2001/12/07 20:04:32  arnetheduck
 * Time to start working on searching...
 *
 * @endif
 */


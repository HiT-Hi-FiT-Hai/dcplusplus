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

#include "SearchManager.h"
#include "ClientManager.h"

SearchManager* SearchManager::instance = NULL;

void SearchManager::search(const string& aName, LONGLONG aSize, DWORD /*aFlags*/ /* = 0 */, int aType /* = 0 */ ) {
	ClientManager::getInstance()->search(aType, aSize, 0, aName);
}

void SearchManager::onData(const BYTE* buf, int aLen) {
	string x((char*)buf, aLen);
	if(x.find("$SR") != string::npos) {
		SearchResult sr;
		
		int i, j;
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
 * $Id: SearchManager.cpp,v 1.16 2002/03/10 22:41:08 arnetheduck Exp $
 * @if LOG
 * $Log: SearchManager.cpp,v $
 * Revision 1.16  2002/03/10 22:41:08  arnetheduck
 * Working on internationalization...
 *
 * Revision 1.15  2002/02/27 12:02:09  arnetheduck
 * Completely new user handling, wonder how it turns out...
 *
 * Revision 1.14  2002/02/09 18:13:51  arnetheduck
 * Fixed level 4 warnings and started using new stl
 *
 * Revision 1.13  2002/01/20 22:54:46  arnetheduck
 * Bugfixes to 0.131 mainly...
 *
 * Revision 1.12  2002/01/15 21:57:53  arnetheduck
 * Hopefully fixed the two annoying bugs...
 *
 * Revision 1.11  2002/01/13 22:50:48  arnetheduck
 * Time for 0.12, added favorites, a bunch of new icons and lot's of other stuff
 *
 * Revision 1.10  2002/01/11 14:52:57  arnetheduck
 * Huge changes in the listener code, replaced most of it with templates,
 * also moved the getinstance stuff for the managers to a template
 *
 * Revision 1.9  2002/01/10 12:33:14  arnetheduck
 * Various fixes
 *
 * Revision 1.8  2002/01/07 23:05:48  arnetheduck
 * Resume rollback implemented
 *
 * Revision 1.7  2001/12/29 13:47:14  arnetheduck
 * Fixing bugs and UI work
 *
 * Revision 1.6  2001/12/21 20:21:17  arnetheduck
 * Private messaging added, and a lot of other updates as well...
 *
 * Revision 1.5  2001/12/15 17:01:06  arnetheduck
 * Passive mode searching as well as some searching code added
 *
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


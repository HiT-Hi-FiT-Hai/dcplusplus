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

#include "HubManager.h"

#include "HttpConnection.h"
#include "StringTokenizer.h"
#include "SimpleXML.h"

HubManager* HubManager::instance = NULL;

void HubManager::onHttpData(const BYTE* aBuf, int aLen) throw() {
	downloadBuf.append((char*)aBuf, aLen);
	string::size_type i;
	
	{
		Lock l(cs);

		while( (i=downloadBuf.find("\r\n")) != string::npos) {
			StringTokenizer tok(downloadBuf.substr(0, i), '|');
			downloadBuf = downloadBuf.substr(i+2);

			StringIter j = tok.getTokens().begin();
			string& name = *j++;
			string& server = *j++;
			string& desc = *j++;
			string& users = *j++;
			publicHubs.push_back(HubEntry(name, server, desc, users));
		}
	}
}

void HubManager::save(SimpleXML* aXml) {
	Lock l(cs);
	aXml->addTag("Favorites");
	aXml->stepIn();
	
	for(FavoriteHubEntry::Iter i = favoriteHubs.begin(); i != favoriteHubs.end(); ++i) {
		aXml->addTag("Favorite");
		aXml->addChildAttrib("Name", (*i)->getName());
		aXml->addChildAttrib("Connect", (*i)->getConnect());
		aXml->addChildAttrib("Description", (*i)->getDescription());
		aXml->addChildAttrib("Nick", (*i)->getNick(false));
		aXml->addChildAttrib("Password", (*i)->getPassword());
		aXml->addChildAttrib("Server", (*i)->getServer());
	}
	aXml->stepOut();
}

void HubManager::load(SimpleXML* aXml) {
	Lock l(cs);
	if(aXml->findChild("Favorites")) {
		aXml->stepIn();
		while(aXml->findChild("Favorite")) {
			FavoriteHubEntry* e = new FavoriteHubEntry();
			e->setName(aXml->getChildAttrib("Name"));
			e->setConnect(aXml->getBoolChildAttrib("Connect"));
			e->setDescription(aXml->getChildAttrib("Description"));
			e->setNick(aXml->getChildAttrib("Nick"));
			e->setPassword(aXml->getChildAttrib("Password"));
			e->setServer(aXml->getChildAttrib("Server"));
			favoriteHubs.push_back(e);
		}
	}
}

/**
 * @file HubManager.cpp
 * $Id: HubManager.cpp,v 1.14 2002/01/20 22:54:46 arnetheduck Exp $
 * @if LOG
 * $Log: HubManager.cpp,v $
 * Revision 1.14  2002/01/20 22:54:46  arnetheduck
 * Bugfixes to 0.131 mainly...
 *
 * Revision 1.13  2002/01/17 23:35:59  arnetheduck
 * Reworked threading once more, now it actually seems stable. Also made
 * sure that noone tries to access client objects that have been deleted
 * as well as some other minor updates
 *
 * Revision 1.12  2002/01/13 22:50:48  arnetheduck
 * Time for 0.12, added favorites, a bunch of new icons and lot's of other stuff
 *
 * Revision 1.11  2002/01/11 14:52:57  arnetheduck
 * Huge changes in the listener code, replaced most of it with templates,
 * also moved the getinstance stuff for the managers to a template
 *
 * Revision 1.10  2002/01/05 19:06:09  arnetheduck
 * Added user list images, fixed bugs and made things more effective
 *
 * Revision 1.8  2002/01/05 10:13:39  arnetheduck
 * Automatic version detection and some other updates
 *
 * Revision 1.7  2001/12/15 17:01:06  arnetheduck
 * Passive mode searching as well as some searching code added
 *
 * Revision 1.6  2001/12/13 19:21:57  arnetheduck
 * A lot of work done almost everywhere, mainly towards a friendlier UI
 * and less bugs...time to release 0.06...
 *
 * Revision 1.5  2001/12/11 01:10:29  arnetheduck
 * More bugfixes...I really have to change the bufferedsocket so that it only
 * uses one thread...or maybe even multiple sockets/thread...
 *
 * Revision 1.4  2001/12/07 20:03:07  arnetheduck
 * More work done towards application stability
 *
 * Revision 1.3  2001/12/02 11:16:46  arnetheduck
 * Optimised hub listing, removed a few bugs and leaks, and added a few small
 * things...downloads are now working, time to start writing the sharing
 * code...
 *
 * Revision 1.2  2001/11/25 22:06:25  arnetheduck
 * Finally downloading is working! There are now a few quirks and bugs to be fixed
 * but what the heck....!
 *
 * Revision 1.1.1.1  2001/11/21 17:33:20  arnetheduck
 * Inital release
 *
 * @endif
 */


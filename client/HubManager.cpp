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

#include "HubManager.h"

#include "HttpConnection.h"
#include "StringTokenizer.h"
#include "SimpleXML.h"
#include "ClientManager.h"
#include "CryptoManager.h"

HubManager* Singleton<HubManager>::instance = NULL;

void HubManager::onHttpFinished() throw() {
	string::size_type i, j;
	string* x;
	string bzlist;

	if(listType == TYPE_BZIP2) {
		CryptoManager::getInstance()->decodeBZ2((u_int8_t*)downloadBuf.data(), downloadBuf.size(), bzlist);
		x = &bzlist;
	} else {
		x = &downloadBuf;
	}

	{
		Lock l(cs);
		i = 0;
		
		while( (i < x->size()) && ((j=x->find("\r\n", i)) != string::npos)) {
			StringTokenizer tok(x->substr(i, j-i), '|');
			i = j + 2;
			if(tok.getTokens().size() < 4)
				continue;

			StringList::const_iterator k = tok.getTokens().begin();
			const string& name = *k++;
			const string& server = *k++;
			const string& desc = *k++;
			const string& users = *k++;
			publicHubs.push_back(HubEntry(name, server, desc, users));
		}
	}
	downloadBuf = Util::emptyString;
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
	aXml->addTag("Users");
	aXml->stepIn();
	for(User::Iter j = users.begin(); j != users.end(); ++j) {
		aXml->addTag("User");
		aXml->addChildAttrib("Nick", (*j)->getNick());
		aXml->addChildAttrib("LastHubIp", (*j)->getLastHubIp());
		aXml->addChildAttrib("LastHubName", (*j)->getLastHubName());
	}

	aXml->stepOut();
}

void HubManager::load(SimpleXML* aXml) {
	Lock l(cs);
	aXml->resetCurrentChild();
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
		aXml->stepOut();
	}
	aXml->resetCurrentChild();
	if(aXml->findChild("Users")) {
		aXml->stepIn();
		while(aXml->findChild("User")) {
			User::Ptr& u = ClientManager::getInstance()->getUser(aXml->getChildAttrib("Nick"), aXml->getChildAttrib("LastHubIp"));
			if(!u->isOnline()) {
				u->setLastHubIp(aXml->getChildAttrib("LastHubIp"));
				u->setLastHubName(aXml->getChildAttrib("LastHubName"));
			}
			addFavoriteUser(u);
		}
		aXml->stepOut();
	}
}

void HubManager::refresh() {
	
	StringList l = StringTokenizer(SETTING(HUBLIST_SERVERS), ';').getTokens();
	const string& server = l[(lastServer) % l.size()];

	fire(HubManagerListener::DOWNLOAD_STARTING, server);
	if(!running) {
		if(!c)
			c = new HttpConnection();
		{
			Lock l(cs);
			publicHubs.clear();
		}
		c->addListener(this);
		c->downloadFile(server);
		if(server.substr(server.size() - 4) == ".bz2") {
			listType = TYPE_BZIP2;
		} else {
			listType = TYPE_NORMAL;
		}
		running = true;

	}
}

/**
 * @file HubManager.cpp
 * $Id: HubManager.cpp,v 1.23 2002/04/28 08:25:50 arnetheduck Exp $
 */


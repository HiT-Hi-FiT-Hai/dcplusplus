/* 
 * Copyright (C) 2001-2003 Jacek Sieka, j_s@telia.com
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

#define FAVORITES_FILE "Favorites.xml"

void HubManager::onHttpFinished() throw() {
	string::size_type i, j;
	string* x;
	string bzlist;

	if(listType == TYPE_BZIP2) {
		try {
			CryptoManager::getInstance()->decodeBZ2((u_int8_t*)downloadBuf.data(), downloadBuf.size(), bzlist);
		} catch(CryptoException) {
			bzlist.clear();
		}
		x = &bzlist;
	} else {
		x = &downloadBuf;
	}

	{
		Lock l(cs);
		publicHubs.clear();
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

void HubManager::save(SimpleXML*) {
	Lock l(cs);

	try {
		SimpleXML xml(8);

		xml.addTag("Favorites");
		xml.stepIn();

		xml.addTag("Hubs");
		xml.stepIn();

		for(FavoriteHubEntry::Iter i = favoriteHubs.begin(); i != favoriteHubs.end(); ++i) {
			xml.addTag("Hub");
			xml.addChildAttrib("Name", (*i)->getName());
			xml.addChildAttrib("Connect", (*i)->getConnect());
			xml.addChildAttrib("Description", (*i)->getDescription());
			xml.addChildAttrib("Nick", (*i)->getNick(false));
			xml.addChildAttrib("Password", (*i)->getPassword());
			xml.addChildAttrib("Server", (*i)->getServer());
			xml.addChildAttrib("UserDescription", (*i)->getUserDescription(false));
		}
		xml.stepOut();
		xml.addTag("Users");
		xml.stepIn();
		for(User::Iter j = users.begin(); j != users.end(); ++j) {
			xml.addTag("User");
			xml.addChildAttrib("Nick", (*j)->getNick());
			xml.addChildAttrib("LastHubIp", (*j)->getLastHubIp());
			xml.addChildAttrib("LastHubName", (*j)->getLastHubName());
		}
		xml.stepOut();
		xml.addTag("Commands");
		xml.stepIn();
		for(UserCommand::Iter k = userCommands.begin(); k != userCommands.end(); ++k) {
			xml.addTag("Command");
			xml.addChildAttrib("Name", k->getName());
			xml.addChildAttrib("Command", k->getCommand());
			xml.addChildAttrib("Hub", k->getHub());
			xml.addChildAttrib("Nick", k->getNick());
		}
		xml.stepOut();

		xml.stepOut();

		string fname = Util::getAppPath() + FAVORITES_FILE;

		BufferedFile f(fname + ".tmp", File::WRITE, File::CREATE | File::TRUNCATE);
		f.write("<?xml version=\"1.0\" encoding=\"windows-1252\"?>\r\n");
		xml.toXML(&f);
		f.close();
		File::deleteFile(fname);
		File::renameFile(fname + ".tmp", fname);

	} catch(Exception e) {
		dcdebug("HubManager::save: %s\n", e.getError().c_str());
	}
}

void HubManager::load() {
	try {
		SimpleXML xml(8);
		xml.fromXML(File(Util::getAppPath() + FAVORITES_FILE, File::READ, File::OPEN).read());
		
		if(xml.findChild("Favorites")) {
			xml.stepIn();
			load(&xml);
			xml.stepOut();
		}
	} catch(Exception e) {
		dcdebug("HubManager::load: %s\n", e.getError().c_str());
	}
}

void HubManager::load(SimpleXML* aXml) {
	Lock l(cs);
	
	// Old names...load for compatibility.
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
			e->setUserDescription(aXml->getChildAttrib("UserDescription"));
			favoriteHubs.push_back(e);
		}
		aXml->stepOut();
	}
	// End old names

	aXml->resetCurrentChild();
	if(aXml->findChild("Hubs")) {
		aXml->stepIn();
		while(aXml->findChild("Hub")) {
			FavoriteHubEntry* e = new FavoriteHubEntry();
			e->setName(aXml->getChildAttrib("Name"));
			e->setConnect(aXml->getBoolChildAttrib("Connect"));
			e->setDescription(aXml->getChildAttrib("Description"));
			e->setNick(aXml->getChildAttrib("Nick"));
			e->setPassword(aXml->getChildAttrib("Password"));
			e->setServer(aXml->getChildAttrib("Server"));
			e->setUserDescription(aXml->getChildAttrib("UserDescription"));
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
	aXml->resetCurrentChild();
	if(aXml->findChild("Commands")) {
		aXml->stepIn();
		while(aXml->findChild("Command")) {
			userCommands.push_back(UserCommand(aXml->getChildAttrib("Name"),
				aXml->getChildAttrib("Command"), aXml->getChildAttrib("Hub"),
				aXml->getChildAttrib("Nick")));
		}
		aXml->stepOut();
	}
}

void HubManager::refresh() {
	StringList l = StringTokenizer(SETTING(HUBLIST_SERVERS), ';').getTokens();
	const string& server = l[(lastServer) % l.size()];
	if(Util::strnicmp(server.c_str(), "http://", 7) != 0) {
		lastServer++;
		return;
	}

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

// HttpConnectionListener
void HubManager::onAction(HttpConnectionListener::Types type, HttpConnection* /*conn*/, const u_int8_t* buf, int len) throw() {
	switch(type) {
	case HttpConnectionListener::DATA:
		downloadBuf.append((char*)buf, len); break;
	default:
		dcassert(0);
	}
}
void HubManager::onAction(HttpConnectionListener::Types type, HttpConnection* /*conn*/, const string& aLine) throw() {
	switch(type) {
	case HttpConnectionListener::FAILED:
		dcassert(c);
		c->removeListener(this);
		lastServer++;
		fire(HubManagerListener::DOWNLOAD_FAILED, aLine);
		running = false;
	}
}
void HubManager::onAction(HttpConnectionListener::Types type, HttpConnection* /*conn*/) throw() {
	switch(type) {
	case HttpConnectionListener::COMPLETE:
		dcassert(c);
		c->removeListener(this);
		onHttpFinished();
		running = false;
		fire(HubManagerListener::DOWNLOAD_FINISHED);
	}
}

void HubManager::onAction(SettingsManagerListener::Types type, SimpleXML* xml) throw() {
	switch(type) {
	case SettingsManagerListener::LOAD: load(xml); load(); break;
	case SettingsManagerListener::SAVE: save(xml); break;
	}
}

/**
 * @file HubManager.cpp
 * $Id: HubManager.cpp,v 1.27 2003/03/13 13:31:23 arnetheduck Exp $
 */

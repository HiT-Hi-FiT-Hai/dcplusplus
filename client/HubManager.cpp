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
#include "UserCommand.h"

HubManager* Singleton<HubManager>::instance = NULL;

#define FAVORITES_FILE "Favorites.xml"

void HubManager::onHttpFinished() throw() {
	string::size_type i, j;
	string* x;
	string bzlist;

	if(listType == TYPE_BZIP2) {
		try {
			CryptoManager::getInstance()->decodeBZ2((u_int8_t*)downloadBuf.data(), downloadBuf.size(), bzlist);
		} catch(const CryptoException&) {
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

void HubManager::save() {
	if(dontSave)
		return;

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
			xml.addChildAttrib("UserDescription", (*i)->getUserDescription());
		}
		xml.stepOut();
		xml.addTag("Users");
		xml.stepIn();
		for(User::Iter j = users.begin(); j != users.end(); ++j) {
			xml.addTag("User");
			xml.addChildAttrib("Nick", (*j)->getNick());
			xml.addChildAttrib("LastHubAddress", (*j)->getLastHubAddress());
			xml.addChildAttrib("LastHubName", (*j)->getLastHubName());
		}
		xml.stepOut();
		xml.addTag("UserCommands");
		xml.stepIn();
		for(UserCommand::Iter k = userCommands.begin(); k != userCommands.end(); ++k) {
			if(!k->isSet(UserCommand::FLAG_NOSAVE)) {
				xml.addTag("UserCommand");
				xml.addChildAttrib("Type", k->getType());
				xml.addChildAttrib("Context", k->getCtx());
				xml.addChildAttrib("Name", k->getName());
				xml.addChildAttrib("Command", k->getCommand());
				xml.addChildAttrib("Hub", k->getHub());
			}
		}
		xml.stepOut();

		xml.stepOut();

		string fname = Util::getAppPath() + FAVORITES_FILE;

		File f(fname + ".tmp", File::WRITE, File::CREATE | File::TRUNCATE);
		f.write("<?xml version=\"1.0\" encoding=\"windows-1252\"?>\r\n");
		f.write(xml.toXML());
		f.close();
		File::deleteFile(fname);
		File::renameFile(fname + ".tmp", fname);

	} catch(const Exception& e) {
		dcdebug("HubManager::save: %s\n", e.getError().c_str());
	}
}

void HubManager::load() {
	
	// Add standard op commands
	static const char kickstr[] = 
		"$To: %[nick] From: %[mynick] $<%[mynick]> You are being kicked because: %[line:Reason]|<%[mynick]> %[mynick] is kicking %[nick] because: %[line:Reason]|$Kick %[nick]|";
	addUserCommand(UserCommand::TYPE_RAW, UserCommand::CONTEXT_CHAT | UserCommand::CONTEXT_HUB, UserCommand::FLAG_NOSAVE, 
		STRING(KICK_USER), kickstr, "op");
	static const char redirstr[] =
		"$OpForceMove $Who:%[nick]$Where:%[line:Target Server]$Msg:%[line:Message]|";
	addUserCommand(UserCommand::TYPE_RAW, UserCommand::CONTEXT_CHAT | UserCommand::CONTEXT_HUB, UserCommand::FLAG_NOSAVE, 
		STRING(REDIRECT_USER), redirstr, "op");

	try {
		SimpleXML xml(8);
		xml.fromXML(File(Util::getAppPath() + FAVORITES_FILE, File::READ, File::OPEN).read());
		
		if(xml.findChild("Favorites")) {
			xml.stepIn();
			load(&xml);
			xml.stepOut();
		}
	} catch(const Exception& e) {
		dcdebug("HubManager::load: %s\n", e.getError().c_str());
	}
}

void HubManager::load(SimpleXML* aXml) {
	dontSave = true;

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
	aXml->resetCurrentChild();
	if(aXml->findChild("Commands")) {
		aXml->stepIn();
		while(aXml->findChild("Command")) {
			const string& name = aXml->getChildAttrib("Name");
			const string& command = aXml->getChildAttrib("Command");
			const string& hub = aXml->getChildAttrib("Hub");
			const string& nick = aXml->getChildAttrib("Nick");
			if(nick.empty()) {
				// Old mainchat style command
				addUserCommand(UserCommand::TYPE_RAW, UserCommand::CONTEXT_CHAT | UserCommand::CONTEXT_SEARCH, 
					0, name, "<%[mynick]> " + command + "|", hub);
			} else {
				addUserCommand(UserCommand::TYPE_RAW, UserCommand::CONTEXT_CHAT | UserCommand::CONTEXT_SEARCH,
					0, name, "$To: " + nick + " From: %[mynick] $" + command + "|", hub);
			}
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
			User::Ptr u = ClientManager::getInstance()->getUser(aXml->getChildAttrib("Nick"), aXml->getChildAttrib("LastHubAddress"));
			if(!u->isOnline()) {
				u->setLastHubAddress(aXml->getChildAttrib("LastHubAddress"));
				u->setLastHubName(aXml->getChildAttrib("LastHubName"));
			}
			addFavoriteUser(u);
		}
		aXml->stepOut();
	}
	aXml->resetCurrentChild();
	if(aXml->findChild("UserCommands")) {
		aXml->stepIn();
		while(aXml->findChild("UserCommand")) {
			addUserCommand(aXml->getIntChildAttrib("Type"), aXml->getIntChildAttrib("Context"),
				0, aXml->getChildAttrib("Name"), aXml->getChildAttrib("Command"), aXml->getChildAttrib("Hub"));
		}
		aXml->stepOut();
	}

	dontSave = false;
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
		running = true;
	}
}

UserCommand::List HubManager::getUserCommands(int ctx, const string& hub, bool op) {
	Lock l(cs);
	UserCommand::List lst;
	for(UserCommand::Iter i = userCommands.begin(); i != userCommands.end(); ++i) {
		UserCommand& uc = *i;
		if(uc.getCtx() & ctx) {
			if( (uc.getHub().empty()) || 
				(op && uc.getHub() == "op") || 
				(Util::stricmp(hub, uc.getHub()) == 0) )
			{
				lst.push_back(*i);
			}
		}
	}
	return lst;
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
	case HttpConnectionListener::COMPLETE:
		dcassert(c);
		c->removeListener(this);
		onHttpFinished();
		running = false;
		fire(HubManagerListener::DOWNLOAD_FINISHED, aLine);
		break;
	case HttpConnectionListener::FAILED:
		dcassert(c);
		c->removeListener(this);
		lastServer++;
		running = false;
		fire(HubManagerListener::DOWNLOAD_FAILED, aLine);
		break;
	case HttpConnectionListener::REDIRECTED:
		fire(HubManagerListener::DOWNLOAD_STARTING, aLine);
		break;
	}
}
void HubManager::onAction(HttpConnectionListener::Types type, HttpConnection* /*conn*/) throw() {
	switch(type) {
	case HttpConnectionListener::SET_DOWNLOAD_TYPE_BZIP2:
		listType = TYPE_BZIP2; break;
	case HttpConnectionListener::SET_DOWNLOAD_TYPE_NORMAL:
		listType = TYPE_NORMAL; break;
	}
}

void HubManager::onAction(SettingsManagerListener::Types type, SimpleXML* xml) throw() {
	if(type == SettingsManagerListener::LOAD) {
		load(xml); 
		load();
	}
}

/**
 * @file
 * $Id: HubManager.cpp,v 1.34 2003/10/22 01:21:02 arnetheduck Exp $
 */

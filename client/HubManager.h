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

#if !defined(AFX_HUBMANAGER_H__75858D5D_F12F_40D0_B127_5DDED226C098__INCLUDED_)
#define AFX_HUBMANAGER_H__75858D5D_F12F_40D0_B127_5DDED226C098__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CriticalSection.h"
#include "HttpConnection.h"
#include "User.h"
#include "SettingsManager.h"

class HubEntry {
public:
	typedef vector<HubEntry> List;
	typedef List::iterator Iter;
	
	HubEntry(const string& aName, const string& aServer, const string& aDescription, const string& aUsers) throw() : 
	name(aName), server(aServer), description(aDescription), users(aUsers) { };
	HubEntry() throw() { };
	HubEntry(const HubEntry& rhs) throw() : name(rhs.name), server(rhs.server), description(rhs.description), users(rhs.users) { }
	virtual ~HubEntry() throw() { };

	GETSETREF(string, name, Name);
	GETSETREF(string, server, Server);
	GETSETREF(string, description, Description);
	GETSETREF(string, users, Users);
};

class FavoriteHubEntry {
public:
	typedef FavoriteHubEntry* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;

	FavoriteHubEntry() throw() : connect(false) { };
	FavoriteHubEntry(const HubEntry& rhs) throw() : name(rhs.getName()), server(rhs.getServer()), description(rhs.getDescription()), connect(false) { };
	FavoriteHubEntry(const FavoriteHubEntry& rhs) throw() : name(rhs.getName()), server(rhs.getServer()), description(rhs.getDescription()), password(rhs.getPassword()), connect(rhs.getConnect()), nick(rhs.nick) { };
	~FavoriteHubEntry() throw() { }	
	
	const string& getNick(bool useDefault = true) const { 
		return (!nick.empty() || !useDefault) ? nick : SETTING(NICK);
	}

	void setNick(const string& aNick) { nick = aNick; };

	const string& getUserDescription(bool useDefault = true) const {
		return (!userdescription.empty() || !useDefault) ? userdescription : SETTING(DESCRIPTION);
	}
	void setUserDescription(const string& aUserDescription) { userdescription = aUserDescription; };
	GETSETREF(string, name, Name);
	GETSETREF(string, server, Server);
	GETSETREF(string, description, Description);
	GETSETREF(string, password, Password);
	GETSET(bool, connect, Connect);
private:
	string nick, userdescription;
};

class UserCommand {
public:
	typedef vector<UserCommand> List;
	typedef List::iterator Iter;

	UserCommand() { };
	UserCommand(const string& aName, const string& aCommand, const string& aHub,
		const string& aNick) throw() : name(aName), command(aCommand), hub(aHub), 
		nick(aNick) { };

	GETSETREF(string, name, Name);
	GETSETREF(string, command, Command);
	GETSETREF(string, hub, Hub);
	GETSETREF(string, nick, Nick);
};

class HubManagerListener {
public:
	typedef HubManagerListener* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;
	enum Types {
		DOWNLOAD_STARTING,
		DOWNLOAD_FAILED,
		DOWNLOAD_FINISHED,
		FAVORITE_ADDED,
		FAVORITE_REMOVED,
		USER_ADDED,
		USER_REMOVED
	};

	virtual void onAction(Types, FavoriteHubEntry*) throw() { };
	virtual void onAction(Types, const string&) throw() { };
	virtual void onAction(Types, const User::Ptr&) throw() { };
	virtual void onAction(Types) throw() { };
};

class SimpleXML;

/**
 * Public hub list, favorites (hub&user). Assumed to be called only by UI thread.
 */
class HubManager : public Speaker<HubManagerListener>, private HttpConnectionListener, public Singleton<HubManager>,
	private SettingsManagerListener
{
public:
	
	void refresh();

	const FavoriteHubEntry::List& getFavoriteHubs() { return favoriteHubs; };

	User::List& getFavoriteUsers() { return users; };
	
	void addFavoriteUser(const User::Ptr& aUser) { 
		if(find(users.begin(), users.end(), aUser) == users.end()) {
			users.push_back(aUser);
			fire(HubManagerListener::USER_ADDED, aUser);
			save();
		}
	}

	void removeFavoriteUser(const User::Ptr& aUser) {
		User::Iter i = find(users.begin(), users.end(), aUser);
		if(i != users.end()) {
			fire(HubManagerListener::USER_REMOVED, aUser);
			users.erase(i);
			save();
		}
	}

	bool isFavoriteUser(const User::Ptr& aUser) {
		return (find(users.begin(), users.end(), aUser) != users.end());
	}
	
	//void addFavorite(const HubEntry& aEntry) { addFavorite(FavoriteHubEntry(aEntry)); };
	void addFavorite(const FavoriteHubEntry& aEntry) {
		FavoriteHubEntry* f;

		FavoriteHubEntry::Iter i = getFavoriteHub(aEntry.getServer());
		if(i != favoriteHubs.end()) {
			return;
		}
		f = new FavoriteHubEntry(aEntry);
		favoriteHubs.push_back(f);
		fire(HubManagerListener::FAVORITE_ADDED, f);
		save();
	}

	void removeFavorite(FavoriteHubEntry* entry) {
		FavoriteHubEntry::Iter i = find(favoriteHubs.begin(), favoriteHubs.end(), entry);
		if(i == favoriteHubs.end()) {
			return;
		}
		
		fire(HubManagerListener::FAVORITE_REMOVED, entry);
		favoriteHubs.erase(i);
		delete entry;
		save();
	}
	
	HubEntry::List getPublicHubs() {
		Lock l(cs);
		return publicHubs;
	}

	UserCommand::List& getUserCommands() {
		return userCommands;
	}

	bool isDownloading() {
		return running;
	}

	void save();
private:
	
	enum {
		TYPE_NORMAL,
		TYPE_BZIP2
	} listType;

	HubEntry::List publicHubs;
	FavoriteHubEntry::List favoriteHubs;
	UserCommand::List userCommands;
	User::List users;

	CriticalSection cs;
	bool running;
	HttpConnection* c;
	int lastServer;

	/** Used during loading to prevent saving. */
	bool dontSave;

	friend class Singleton<HubManager>;
	
	HubManager() : running(false), c(NULL), lastServer(0), dontSave(false) {
		SettingsManager::getInstance()->addListener(this);
	}

	virtual ~HubManager() {
		SettingsManager::getInstance()->addListener(this);
		if(c) {
			c->removeListener(this);
			delete c;
			c = NULL;
		}
		
		for_each(favoriteHubs.begin(), favoriteHubs.end(), DeleteFunction<FavoriteHubEntry*>());
	}
	
	string downloadBuf;
	
	FavoriteHubEntry::Iter getFavoriteHub(const string& aServer) {
		for(FavoriteHubEntry::Iter i = favoriteHubs.begin(); i != favoriteHubs.end(); ++i) {
			if((*i)->getServer() == aServer) {
				return i;
			}
		}
		return favoriteHubs.end();
	}

	// HttpConnectionListener
	virtual void onAction(HttpConnectionListener::Types type, HttpConnection* /*conn*/, const u_int8_t* buf, int len) throw();
	virtual void onAction(HttpConnectionListener::Types type, HttpConnection* /*conn*/, const string& aLine) throw();
	virtual void onAction(HttpConnectionListener::Types type, HttpConnection* /*conn*/) throw();
	
 	void onHttpFinished() throw();

	// SettingsManagerListener
	virtual void onAction(SettingsManagerListener::Types type, SimpleXML*) throw();

	void load(SimpleXML* aXml);
	void load();
	
};

#endif // !defined(AFX_HUBMANAGER_H__75858D5D_F12F_40D0_B127_5DDED226C098__INCLUDED_)

/**
 * @file
 * $Id: HubManager.h,v 1.37 2003/04/15 10:13:54 arnetheduck Exp $
 */


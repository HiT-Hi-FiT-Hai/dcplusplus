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

#if !defined(AFX_HUBMANAGER_H__75858D5D_F12F_40D0_B127_5DDED226C098__INCLUDED_)
#define AFX_HUBMANAGER_H__75858D5D_F12F_40D0_B127_5DDED226C098__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Util.h"
#include "CriticalSection.h"
#include "HttpConnection.h"
#include "TimerManager.h"
#include "User.h"

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
	virtual ~FavoriteHubEntry() throw() { }	
	const string& getNick(bool useDefault = true) const {
		if(nick.size() > 0 || !useDefault) 
			return nick;
		else 
			return SETTING(NICK);
	}

	void setNick(const string& aNick) {
		nick = aNick;
	}

	GETSETREF(string, name, Name);
	GETSETREF(string, server, Server);
	GETSETREF(string, description, Description);
	GETSETREF(string, password, Password);
	GETSET(bool, connect, Connect);
private:
	string nick;
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
		GET_FAVORITE_HUBS,
		FAVORITE_ADDED,
		FAVORITE_REMOVED,
		USER_ADDED,
		USER_REMOVED
	};
	virtual void onAction(Types, const FavoriteHubEntry::List&) { };
	virtual void onAction(Types, FavoriteHubEntry*) { };
	virtual void onAction(Types, const string&) { };
	virtual void onAction(Types, const User::Ptr&) { };
	virtual void onAction(Types) { };
};

class SimpleXML;

class HubManager : public Speaker<HubManagerListener>, private HttpConnectionListener, public Singleton<HubManager>, 
	private TimerManagerListener, private SettingsManagerListener
{
public:
	
	void refresh();
	
	void getFavoriteHubs() {
		Lock l(cs);
		fire(HubManagerListener::GET_FAVORITE_HUBS, favoriteHubs);
	}
	
	User::List getFavoriteUsers() { Lock l(cs); return users; };
	
	void addFavoriteUser(const User::Ptr& aUser) { 
		Lock l(cs);
		if(find(users.begin(), users.end(), aUser) == users.end()) {
			users.push_back(aUser);
			fire(HubManagerListener::USER_ADDED, aUser);
		}
	}

	void removeFavoriteUser(const User::Ptr& aUser) {
		Lock l(cs);
		User::Iter i = find(users.begin(), users.end(), aUser);
		if(i != users.end()) {
			fire(HubManagerListener::USER_REMOVED, aUser);
			users.erase(i);
		}
	}

	bool isFavoriteUser(const User::Ptr& aUser) {
		Lock l(cs);
		return (find(users.begin(), users.end(), aUser) != users.end());
	}
	
	void addFavorite(const HubEntry& aEntry) { addFavorite(FavoriteHubEntry(aEntry)); };
	void addFavorite(const FavoriteHubEntry& aEntry) {
		FavoriteHubEntry* f;

		{
			Lock l(cs);
			FavoriteHubEntry::Iter i = getFavoriteHub(aEntry.getServer());
			if(i != favoriteHubs.end()) {
				return;
			}
			f = new FavoriteHubEntry(aEntry);
			favoriteHubs.push_back(f);
		}
		fire(HubManagerListener::FAVORITE_ADDED, f);
	}

	void removeFavorite(FavoriteHubEntry* entry) {
		{
			Lock l(cs);
			
			FavoriteHubEntry::Iter i = find(favoriteHubs.begin(), favoriteHubs.end(), entry);
			if(i == favoriteHubs.end()) {
				return;
			}
			
			favoriteHubs.erase(i);
		}
		fire(HubManagerListener::FAVORITE_REMOVED, entry);
		delete entry;
	}
	
	HubEntry::List getPublicHubs() {
		Lock l(cs);
		return publicHubs;
	}

	bool isDownloading() {
		return running;
	}
private:
	
	enum {
		TYPE_NORMAL,
		TYPE_BZIP2
	} listType;

	HubEntry::List publicHubs;
	FavoriteHubEntry::List favoriteHubs;
	User::List users;

	CriticalSection cs;
	bool running;
	HttpConnection* c;
	int lastServer;

	friend class Singleton<HubManager>;
	
	HubManager() : running(false), c(NULL), lastServer(0) {
		TimerManager::getInstance()->addListener(this);
		SettingsManager::getInstance()->addListener(this);
	}

	virtual ~HubManager() {
		SettingsManager::getInstance()->removeListener(this);
		TimerManager::getInstance()->removeListener(this);
		if(c) {
			c->removeListener(this);
			delete c;
			c = NULL;
		}
		
		{
			Lock l(cs);
			for(FavoriteHubEntry::Iter i = favoriteHubs.begin(); i != favoriteHubs.end(); ++i) {
				delete *i;
			}
		}
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
	virtual void onAction(HttpConnectionListener::Types type, HttpConnection* /*conn*/, const u_int8_t* buf, int len) {
		switch(type) {
		case HttpConnectionListener::DATA:
			downloadBuf.append((char*)buf, len); break;
		default:
			dcassert(0);
		}
	}
	virtual void onAction(HttpConnectionListener::Types type, HttpConnection* /*conn*/, const string& aLine) {
		switch(type) {
		case HttpConnectionListener::FAILED:
			dcassert(c);
			c->removeListener(this);
			lastServer++;
			fire(HubManagerListener::DOWNLOAD_FAILED, aLine);
			running = false;
		}
	}
	virtual void onAction(HttpConnectionListener::Types type, HttpConnection* /*conn*/) {
		switch(type) {
		case HttpConnectionListener::COMPLETE:
			dcassert(c);
			c->removeListener(this);
			onHttpFinished();
			running = false;
			fire(HubManagerListener::DOWNLOAD_FINISHED);
		}
	}
	
 	void onHttpFinished() throw();

	// TimerManagerListener
	virtual void onAction(TimerManagerListener::Types type, u_int32_t) {
		if(type == TimerManagerListener::MINUTE) {
			if(publicHubs.empty() && !running)
				refresh();
		}
	}

	virtual void onAction(SettingsManagerListener::Types type, SimpleXML* xml) {
		switch(type) {
		case SettingsManagerListener::LOAD: load(xml); break;
		case SettingsManagerListener::SAVE: save(xml); break;
		}
	}
	
	void load(SimpleXML* aXml);
	void save(SimpleXML* aXml);
	
};

#endif // !defined(AFX_HUBMANAGER_H__75858D5D_F12F_40D0_B127_5DDED226C098__INCLUDED_)

/**
 * @file HubManager.h
 * $Id: HubManager.h,v 1.30 2002/05/12 21:54:08 arnetheduck Exp $
 */


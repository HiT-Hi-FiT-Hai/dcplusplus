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
	FavoriteHubEntry(const FavoriteHubEntry& rhs) throw() : name(rhs.getName()), server(rhs.getServer()), description(rhs.getDescription()), nick(rhs.nick), password(rhs.password), connect(rhs.connect) { };
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
		MESSAGE,
		FINISHED,
		GET_PUBLIC_HUBS,
		GET_FAVORITE_HUBS,
		FAVORITE_ADDED,
		FAVORITE_REMOVED
	};
	virtual void onAction(Types, const FavoriteHubEntry::List&) { };
	virtual void onAction(Types, FavoriteHubEntry*) { };
	virtual void onAction(Types, const string&) { };
	virtual void onAction(Types, const HubEntry::List&) { };
	virtual void onAction(Types) { };
};

class SimpleXML;

class HubManager : public Speaker<HubManagerListener>, private HttpConnectionListener, public Singleton<HubManager>, private TimerManagerListener
{
public:
	
	void load(SimpleXML* aXml);
	void save(SimpleXML* aXml);
	
	void getFavoriteHubs() {
		Lock l(cs);
		fire(HubManagerListener::GET_FAVORITE_HUBS, favoriteHubs);
	}
	
	void addFavorite(const HubEntry& aEntry) {
		addFavorite(FavoriteHubEntry(aEntry));
	}

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
	
	void getPublicHubs() {
		Lock l(cs);
		fire(HubManagerListener::GET_PUBLIC_HUBS, publicHubs);
	}

	bool isRunning() {
		return running;
	}
	bool hasDownloaded() {
		return downloaded;
	}
	
 	void refresh();
	
	void reset() {
		if(conn) {
			conn->removeListener(this);
			delete conn;
			conn = NULL;
		}
	}
private:
	
	HubEntry::List publicHubs;
	FavoriteHubEntry::List favoriteHubs;
	
	CriticalSection cs;
	HttpConnection* conn;
	bool running;
	bool downloaded;
	bool pendingConnect;

	int lastServer;

	friend class Singleton<HubManager>;
	
	HubManager() : downloaded(false), conn(NULL), running(false), pendingConnect(false), lastServer(0) {
		TimerManager::getInstance()->addListener(this);
	}

	~HubManager() {
		TimerManager::getInstance()->removeListener(this);
		if(conn) {
			conn->removeListener(this);
			delete conn;
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
	virtual void onAction(HttpConnectionListener::Types type, HttpConnection* /*conn*/, const BYTE* buf, int len) {
		switch(type) {
		case HttpConnectionListener::DATA:
			downloadBuf.append((char*)buf, len); break;
		default:
			dcassert(0);
		}
	}
	virtual void onAction(HttpConnectionListener::Types type, HttpConnection* conn, const string& /*aLine*/) {
		switch(type) {
		case HttpConnectionListener::FAILED:
			conn->removeListener(this);
			running = false;
			fire(HubManagerListener::MESSAGE, "Unable to download public server list. Check your internet connection!");
			pendingConnect = true;
		}
	}
	virtual void onAction(HttpConnectionListener::Types type, HttpConnection* conn) {
		switch(type) {
		case HttpConnectionListener::COMPLETE:
			conn->removeListener(this);
			onHttpFinished();
			running = false;
			downloaded = true;
			fire(HubManagerListener::FINISHED);
		}
	}
	
 	void onHttpFinished() throw();

	// TimerManagerListener
	virtual void onAction(TimerManagerListener::Types type, DWORD) {
		if(pendingConnect && type == TimerManagerListener::MINUTE) {
			pendingConnect = false;
			refresh();
		}
	}
};

#endif // !defined(AFX_HUBMANAGER_H__75858D5D_F12F_40D0_B127_5DDED226C098__INCLUDED_)

/**
 * @file HubManager.h
 * $Id: HubManager.h,v 1.23 2002/03/13 20:35:25 arnetheduck Exp $
 * @if LOG
 * $Log: HubManager.h,v $
 * Revision 1.23  2002/03/13 20:35:25  arnetheduck
 * Release canditate...internationalization done as far as 0.155 is concerned...
 * Also started using mirrors of the public hub lists
 *
 * Revision 1.22  2002/03/07 19:07:52  arnetheduck
 * Minor fixes + started code review
 *
 * Revision 1.21  2002/02/10 12:25:24  arnetheduck
 * New properties for favorites, and some minor performance tuning...
 *
 * Revision 1.20  2002/02/09 18:13:51  arnetheduck
 * Fixed level 4 warnings and started using new stl
 *
 * Revision 1.19  2002/01/25 00:11:26  arnetheduck
 * New settings dialog and various fixes
 *
 * Revision 1.18  2002/01/20 22:54:46  arnetheduck
 * Bugfixes to 0.131 mainly...
 *
 * Revision 1.17  2002/01/17 23:35:59  arnetheduck
 * Reworked threading once more, now it actually seems stable. Also made
 * sure that noone tries to access client objects that have been deleted
 * as well as some other minor updates
 *
 * Revision 1.16  2002/01/13 22:50:48  arnetheduck
 * Time for 0.12, added favorites, a bunch of new icons and lot's of other stuff
 *
 * Revision 1.15  2002/01/11 14:52:57  arnetheduck
 * Huge changes in the listener code, replaced most of it with templates,
 * also moved the getinstance stuff for the managers to a template
 *
 * Revision 1.14  2002/01/10 12:33:14  arnetheduck
 * Various fixes
 *
 * Revision 1.13  2002/01/05 19:06:09  arnetheduck
 * Added user list images, fixed bugs and made things more effective
 *
 * Revision 1.11  2002/01/05 10:13:39  arnetheduck
 * Automatic version detection and some other updates
 *
 * Revision 1.10  2001/12/29 13:47:14  arnetheduck
 * Fixing bugs and UI work
 *
 * Revision 1.9  2001/12/15 17:01:06  arnetheduck
 * Passive mode searching as well as some searching code added
 *
 * Revision 1.8  2001/12/13 19:21:57  arnetheduck
 * A lot of work done almost everywhere, mainly towards a friendlier UI
 * and less bugs...time to release 0.06...
 *
 * Revision 1.7  2001/12/11 01:10:29  arnetheduck
 * More bugfixes...I really have to change the bufferedsocket so that it only
 * uses one thread...or maybe even multiple sockets/thread...
 *
 * Revision 1.6  2001/12/07 20:03:10  arnetheduck
 * More work done towards application stability
 *
 * Revision 1.5  2001/12/02 23:47:35  arnetheduck
 * Added the framework for uploading and file sharing...although there's something strange about
 * the file lists...my client takes them, but not the original...
 *
 * Revision 1.4  2001/12/02 11:16:46  arnetheduck
 * Optimised hub listing, removed a few bugs and leaks, and added a few small
 * things...downloads are now working, time to start writing the sharing
 * code...
 *
 * Revision 1.3  2001/11/26 23:40:36  arnetheduck
 * Downloads!! Now downloads are possible, although the implementation is
 * likely to change in the future...more UI work (splitters...) and some bug
 * fixes. Only user file listings are downloadable, but at least it's something...
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


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

#if !defined(AFX_HUBMANAGER_H__75858D5D_F12F_40D0_B127_5DDED226C098__INCLUDED_)
#define AFX_HUBMANAGER_H__75858D5D_F12F_40D0_B127_5DDED226C098__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SettingsManager.h"

#include "CriticalSection.h"
#include "HttpConnection.h"
#include "User.h"
#include "UserCommand.h"
#include "FavoriteUser.h"
#include "Singleton.h"

class HubEntry {
public:
	typedef vector<HubEntry> List;
	typedef List::iterator Iter;
	
	HubEntry(const string& aName, const string& aServer, const string& aDescription, const string& aUsers) throw() : 
	name(aName), server(aServer), description(aDescription), users(Util::toInt(aUsers)), country(Util::emptyString), 
	rating(Util::emptyString), reliability(0.0), shared(0), minShare(0), minSlots(0), maxHubs(0), maxUsers(0) { };

	HubEntry(const string& aName, const string& aServer, const string& aDescription, const string& aUsers, const string& aCountry,
		const string& aShared, const string& aMinShare, const string& aMinSlots, const string& aMaxHubs, const string& aMaxUsers,
		const string& aReliability, const string& aRating) : name(aName), server(aServer), description(aDescription), country(aCountry), 
		rating(aRating), reliability((float)(Util::toFloat(aReliability) / 100.0)), shared(Util::toInt64(aShared)), minShare(Util::toInt64(aMinShare)),
		users(Util::toInt(aUsers)), minSlots(Util::toInt(aMinSlots)), maxHubs(Util::toInt(aMaxHubs)), maxUsers(Util::toInt(aMaxUsers)) 
	{

	}

	HubEntry() throw() { };
	HubEntry(const HubEntry& rhs) throw() : name(rhs.name), server(rhs.server), description(rhs.description), country(rhs.country), 
		rating(rhs.rating), reliability(rhs.reliability), shared(rhs.shared), minShare(rhs.minShare), users(rhs.users), minSlots(rhs.minSlots),
		maxHubs(rhs.maxHubs), maxUsers(rhs.maxUsers) { }

	~HubEntry() throw() { };

	GETSET(string, name, Name);
	GETSET(string, server, Server);
	GETSET(string, description, Description);
	GETSET(string, country, Country);
	GETSET(string, rating, Rating);
	GETSET(float, reliability, Reliability);
	GETSET(int64_t, shared, Shared);
	GETSET(int64_t, minShare, MinShare);
	GETSET(int, users, Users);
	GETSET(int, minSlots, MinSlots);
	GETSET(int, maxHubs, MaxHubs)
	GETSET(int, maxUsers, MaxUsers);
};

class FavoriteHubEntry {
public:
	typedef FavoriteHubEntry* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;

	FavoriteHubEntry() throw() : connect(false), bottom(0), top(0), left(0), right(0){ };
	FavoriteHubEntry(const HubEntry& rhs) throw() : name(rhs.getName()), server(rhs.getServer()), description(rhs.getDescription()), connect(false), bottom(0), top(0), left(0), right(0){ };
	FavoriteHubEntry(const FavoriteHubEntry& rhs) throw() : userdescription(rhs.userdescription), name(rhs.getName()), server(rhs.getServer()), description(rhs.getDescription()), 
		password(rhs.getPassword()), connect(rhs.getConnect()), bottom(rhs.getBottom()), top(rhs.getTop()), left(rhs.getLeft()), right(rhs.getRight()), nick(rhs.nick){ };
	~FavoriteHubEntry() throw() { }	
	
	const string& getNick(bool useDefault = true) const { 
		return (!nick.empty() || !useDefault) ? nick : SETTING(NICK);
	}

	void setNick(const string& aNick) { nick = aNick; };

	GETSET(string, userdescription, UserDescription);
	GETSET(string, name, Name);
	GETSET(string, server, Server);
	GETSET(string, description, Description);
	GETSET(string, password, Password);
	GETSET(bool, connect, Connect);

	GETSET(u_int16_t, bottom, Bottom);
	GETSET(u_int16_t, top, Top);
	GETSET(u_int16_t, left, Left);
	GETSET(u_int16_t, right, Right);


private:
	string nick;
};

class HubManagerListener {
public:
	template<int I>	struct X { enum { TYPE = I };  };

	typedef X<0> DownloadStarting;
	typedef X<1> DownloadFailed;
	typedef X<2> DownloadFinished;
	typedef X<3> FavoriteAdded;
	typedef X<4> FavoriteRemoved;
	typedef X<5> UserAdded;
	typedef X<6> UserRemoved;

	virtual void on(DownloadStarting, const string&) throw() { }
	virtual void on(DownloadFailed, const string&) throw() { }
	virtual void on(DownloadFinished, const string&) throw() { }
	virtual void on(FavoriteAdded, const FavoriteHubEntry*) throw() { }
	virtual void on(FavoriteRemoved, const FavoriteHubEntry*) throw() { }
	virtual void on(UserAdded, const User::Ptr&) throw() { }
	virtual void on(UserRemoved, const User::Ptr&) throw() { }
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

	FavoriteHubEntry::List& getFavoriteHubs() { return favoriteHubs; };

	User::List& getFavoriteUsers() { return users; };
	
	void addFavoriteUser(User::Ptr& aUser);
	void removeFavoriteUser(User::Ptr& aUser);

	void addFavorite(const FavoriteHubEntry& aEntry);
	void removeFavorite(FavoriteHubEntry* entry);
	
	FavoriteHubEntry* getFavoriteHubEntry(const string& aServer) {
		for(FavoriteHubEntry::Iter i = favoriteHubs.begin(); i != favoriteHubs.end(); ++i) {
			FavoriteHubEntry* hub = *i;
			if(Util::stricmp(hub->getServer(), aServer) == 0) {
				return hub;
			}
		}
		return NULL;
	}

	HubEntry::List getPublicHubs() {
		Lock l(cs);
		return publicHubs;
	}

	UserCommand addUserCommand(int type, int ctx, int flags, const string& name, const string& command, const string& hub) {
		// No dupes, add it...
		Lock l(cs);
		userCommands.push_back(UserCommand(lastId++, type, ctx, flags, name, command, hub));
		UserCommand& uc = userCommands.back();
		if(!uc.isSet(UserCommand::FLAG_NOSAVE)) 
			save();
		return userCommands.back();
	}

	bool getUserCommand(int cid, UserCommand& uc) {
		Lock l(cs);
		for(UserCommand::Iter i = userCommands.begin(); i != userCommands.end(); ++i) {
			if(i->getId() == cid) {
				uc = *i;
				return true;
			}
		}
		return false;
	}

	bool moveUserCommand(int cid, int pos) {
		dcassert(pos == -1 || pos == 1);
		Lock l(cs);
		for(UserCommand::Iter i = userCommands.begin(); i != userCommands.end(); ++i) {
			if(i->getId() == cid) {
				swap(*i, *(i + pos));
				return true;
			}
		}
		return false;
	}

	void updateUserCommand(const UserCommand& uc) {
		bool nosave = true;
		Lock l(cs);
		for(UserCommand::Iter i = userCommands.begin(); i != userCommands.end(); ++i) {
			if(i->getId() == uc.getId()) {
				*i = uc;
				nosave = uc.isSet(UserCommand::FLAG_NOSAVE);
				break;
			}
		}
		if(!nosave)
			save();
	}

	void removeUserCommand(int cid) {
		bool nosave = true;
		Lock l(cs);
		for(UserCommand::Iter i = userCommands.begin(); i != userCommands.end(); ++i) {
			if(i->getId() == cid) {
				nosave = i->isSet(UserCommand::FLAG_NOSAVE);
				userCommands.erase(i);
				break;
			}
		}
		if(!nosave)
			save();
	}
	void removeUserCommand(const string& srv) {
		Lock l(cs);
		for(UserCommand::Iter i = userCommands.begin(); i != userCommands.end(); ) {
			if((i->getHub() == srv) && i->isSet(UserCommand::FLAG_NOSAVE)) {
				i = userCommands.erase(i);
			} else {
				++i;
			}
		}
	}

	void removeHubUserCommands(int ctx, const string& hub) {
		Lock l(cs);
		for(UserCommand::Iter i = userCommands.begin(); i != userCommands.end(); ) {
			if(i->getHub() == hub && i->isSet(UserCommand::FLAG_NOSAVE) && i->getCtx() & ctx) {
				i = userCommands.erase(i);
			} else {
				++i;
			}
		}
	}

	UserCommand::List getUserCommands() { Lock l(cs); return userCommands; };
	UserCommand::List getUserCommands(int ctx, const string& hub, bool op);

	bool isDownloading() { return running; };

	void load();
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
	int lastId;

	/** Used during loading to prevent saving. */
	bool dontSave;

	friend class Singleton<HubManager>;
	
	HubManager() : running(false), c(NULL), lastServer(0), lastId(0), dontSave(false) {
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
			if(Util::stricmp((*i)->getServer(), aServer) == 0) {
				return i;
			}
		}
		return favoriteHubs.end();
	}

	void loadXmlList(const string& xml);

	// HttpConnectionListener
	virtual void on(Data, HttpConnection*, const u_int8_t*, size_t) throw();
	virtual void on(Failed, HttpConnection*, const string&) throw();
	virtual void on(Complete, HttpConnection*, const string&) throw();
	virtual void on(Redirected, HttpConnection*, const string&) throw();
	virtual void on(TypeNormal, HttpConnection*) throw();
	virtual void on(TypeBZ2, HttpConnection*) throw();

	void onHttpFinished() throw();

	// SettingsManagerListener
	virtual void on(SettingsManagerListener::Load, SimpleXML* xml) throw() {
		load(xml);
	}

	void load(SimpleXML* aXml);
	
};

#endif // !defined(AFX_HUBMANAGER_H__75858D5D_F12F_40D0_B127_5DDED226C098__INCLUDED_)

/**
 * @file
 * $Id: HubManager.h,v 1.59 2004/10/26 13:53:58 arnetheduck Exp $
 */


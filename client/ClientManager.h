/* 
 * Copyright (C) 2001-2005 Jacek Sieka, arnetheduck on gmail point com
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

#if !defined(AFX_CLIENTMANAGER_H__8EF173E1_F7DC_40B5_B2F3_F92297701034__INCLUDED_)
#define AFX_CLIENTMANAGER_H__8EF173E1_F7DC_40B5_B2F3_F92297701034__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TimerManager.h"

#include "Client.h"
#include "Singleton.h"
#include "SettingsManager.h"

#include "ClientManagerListener.h"

class ClientManager : public Speaker<ClientManagerListener>, 
	private ClientListener, public Singleton<ClientManager>, 
	private TimerManagerListener, private SettingsManagerListener
{
public:
	Client* getClient(const string& aHubURL);
	void putClient(Client* aClient);

	size_t getUserCount() {
		Lock l(cs);

		size_t c = 0;
		for(Client::Iter i = clients.begin(); i != clients.end(); ++i) {
			c+=(*i)->getUserCount();
		}
		return c;
	}

	int64_t getAvailable() {
		Lock l(cs);
		
		int64_t c = 0;
		for(Client::Iter i = clients.begin(); i != clients.end(); ++i) {
			c+=(*i)->getAvailable();
		}
		return c;
	}

	bool isConnected(const string& aAddress, short port) {
		Lock l(cs);

		for(Client::Iter i = clients.begin(); i != clients.end(); ++i) {
			if(((*i)->getAddress() == aAddress || (*i)->getIp() == aAddress) && (*i)->getPort() == port) {
				return true;
			}
		}
		return false;
	}
	
	void search(int aSizeMode, int64_t aSize, int aFileType, const string& aString, const string& aToken);
	void search(StringList& who, int aSizeMode, int64_t aSize, int aFileType, const string& aString, const string& aToken);
	void infoUpdated();

	User::Ptr getUser(const string& aNick, const string& aHubUrl) throw();
	User::Ptr getLegacyUser(const string& aNick) throw();
	User::Ptr getUser(const CID& cid) throw();

	User::Ptr findUser(const string& aNick, const string& aHubUrl) throw() { return findUser(makeCid(aNick, aHubUrl)); }
	User::Ptr findUser(const CID& cid) throw();

	/** Constructs a synthetic, hopefully unique CID */
	CID makeCid(const string& nick, const string& hubUrl) throw();

	void putOnline(OnlineUser& ou) throw();
	void putOffline(OnlineUser& ou) throw();

	User::Ptr& getMe() { return me; }
	
	void connect(const User::Ptr& p);
	void send(AdcCommand& c);
	void privateMessage(const User::Ptr& p, const string& msg);

	bool isActive() { return SETTING(INCOMING_CONNECTIONS) != SettingsManager::INCOMING_FIREWALL_PASSIVE; }
	
	void lock() throw() { cs.enter(); }
	void unlock() throw() { cs.leave(); }

	Client::List& getClients() { return clients; }

 	void removeClientListener(ClientListener* listener) {
 		Lock l(cs);
 		Client::Iter endIt = clients.end();
 		for(Client::Iter it = clients.begin(); it != endIt; ++it) {
 			Client* client = *it;
 			client->removeListener(listener);
 		}
 	}

private:
	typedef HASH_MAP<string, User::Ptr> LegacyMap;
	typedef LegacyMap::iterator LegacyIter;

	typedef HASH_MAP_X(CID, User::Ptr, CID::Hash, equal_to<CID>, less<CID>) UserMap;
	typedef UserMap::iterator UserIter;

	typedef HASH_MULTIMAP_X(CID, OnlineUser*, CID::Hash, equal_to<CID>, less<CID>) OnlineMap;
	typedef OnlineMap::iterator OnlineIter;
	typedef pair<OnlineIter, OnlineIter> OnlinePair;

	Client::List clients;
	CriticalSection cs;
	
	UserMap users;
	LegacyMap legacyUsers;
	OnlineMap onlineUsers;

	User::Ptr me;
	
	Socket s;

	friend class Singleton<ClientManager>;

	ClientManager() { 
		TimerManager::getInstance()->addListener(this); 
		SettingsManager::getInstance()->addListener(this);
	}

	virtual ~ClientManager() throw() { 
		SettingsManager::getInstance()->removeListener(this);
		TimerManager::getInstance()->removeListener(this); 
	}

	// SettingsManagerListener
	virtual void on(Load, SimpleXML*) throw() {
		me = new User(SETTING(CLIENT_ID));
	}

	// ClientListener
	virtual void on(Connected, Client* c) throw() { fire(ClientManagerListener::ClientConnected(), c); }
	virtual void on(UsersUpdated, Client* c, const User::List&) throw() { fire(ClientManagerListener::ClientUpdated(), c); }
	virtual void on(Failed, Client*, const string&) throw();
	virtual void on(HubUpdated, Client* c) throw() { fire(ClientManagerListener::ClientUpdated(), c); }
	virtual void on(UserCommand, Client*, int, int, const string&, const string&) throw();
	virtual void on(NmdcSearch, Client* aClient, const string& aSeeker, int aSearchType, int64_t aSize, 
		int aFileType, const string& aString) throw();
	virtual void on(AdcSearch, Client* c, const AdcCommand& adc) throw();
	// TimerManagerListener
	virtual void on(TimerManagerListener::Minute, u_int32_t aTick) throw();
};

#endif // !defined(AFX_CLIENTMANAGER_H__8EF173E1_F7DC_40B5_B2F3_F92297701034__INCLUDED_)

/**
 * @file
 * $Id: ClientManager.h,v 1.59 2005/04/23 15:45:32 arnetheduck Exp $
 */


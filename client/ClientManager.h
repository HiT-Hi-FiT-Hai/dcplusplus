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

#if !defined(AFX_CLIENTMANAGER_H__8EF173E1_F7DC_40B5_B2F3_F92297701034__INCLUDED_)
#define AFX_CLIENTMANAGER_H__8EF173E1_F7DC_40B5_B2F3_F92297701034__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TimerManager.h"

#include "Client.h"
#include "Singleton.h"

#include "ClientManagerListener.h"

class ClientManager : public Speaker<ClientManagerListener>, 
	private ClientListener, public Singleton<ClientManager>, 
	private TimerManagerListener
{
public:
	Client* getClient(const string& aHubURL);
	void putClient(Client* aClient);

	int getUserCount() {
		Lock l(cs);

		int c = 0;
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
	
	void search(int aSizeMode, int64_t aSize, int aFileType, const string& aString) {
		Lock l(cs);

		for(Client::Iter i = clients.begin(); i != clients.end(); ++i) {
			if((*i)->isConnected()) {
				(*i)->search(aSizeMode, aSize, aFileType, aString);
			}
		}
	}

	void search(StringList& who, int aSizeMode, int64_t aSize, int aFileType, const string& aString) {
		Lock l(cs);

		for(StringIter it = who.begin(); it != who.end(); ++it) {
			string& client = *it;
			for(Client::Iter j = clients.begin(); j != clients.end(); ++j) {
				Client* c = *j;
				if(c->isConnected() && c->getIpPort() == client) {
					c->search(aSizeMode, aSize, aFileType, aString);

				}
			}
		}
	}

	void infoUpdated();

	User::Ptr getUser(const CID& cid, bool createUser);
	User::Ptr getUser(const CID& cid, Client* aClient, bool putOnline = true);
	User::Ptr getUser(const string& aNick, const string& aHint = Util::emptyString);
	User::Ptr getUser(const string& aNick, Client* aClient, bool putOnline = true);
	
	bool isOnline(const string& aNick) {
		Lock l(cs);
		UserPair i = users.equal_range(aNick);
		for(UserIter j = i.first; j != i.second; ++j) {
			if(j->second->isOnline())
				return true;
		}
		return false;
	}

	/**
	 * A user went offline. Must be called whenever a user quits a hub.
	 * @param quitHub The user went offline because (s)he disconnected from the hub.
	 */
	void putUserOffline(User::Ptr& aUser, bool quitHub = false);
	
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
	typedef HASH_MULTIMAP<string, User::Ptr> UserMap;
	typedef UserMap::iterator UserIter;
	typedef pair<UserIter, UserIter> UserPair;

	typedef HASH_MULTIMAP_X(CID, User::Ptr, CID::Hash, equal_to<CID>, less<CID>) AdcMap;
	typedef AdcMap::iterator AdcIter;
	typedef pair<AdcIter, AdcIter> AdcPair;

	Client::List clients;
	CriticalSection cs;
	
	UserMap users;
	AdcMap adcUsers;

	Socket s;

	friend class Singleton<ClientManager>;
	ClientManager() { 
		TimerManager::getInstance()->addListener(this); 
		if(SETTING(CLIENT_ID).empty())
			SettingsManager::getInstance()->set(SettingsManager::CLIENT_ID, CID::generate().toBase32());
	};

	virtual ~ClientManager() { TimerManager::getInstance()->removeListener(this); };

	// ClientListener
	virtual void on(Connected, Client* c) throw() { fire(ClientManagerListener::ClientConnected(), c); }
	virtual void on(UsersUpdated, Client* c, const User::List&) throw() { fire(ClientManagerListener::ClientUpdated(), c); }
	virtual void on(Failed, Client*, const string&) throw();
	virtual void on(HubUpdated, Client* c) throw() { fire(ClientManagerListener::ClientUpdated(), c); }
	virtual void on(UserCommand, Client*, int, int, const string&, const string&) throw();
	virtual void on(NmdcSearch, Client* aClient, const string& aSeeker, int aSearchType, int64_t aSize, 
		int aFileType, const string& aString) throw();

	// TimerManagerListener
	virtual void on(TimerManagerListener::Minute, u_int32_t aTick) throw();
};

#endif // !defined(AFX_CLIENTMANAGER_H__8EF173E1_F7DC_40B5_B2F3_F92297701034__INCLUDED_)

/**
 * @file
 * $Id: ClientManager.h,v 1.49 2004/09/06 12:32:41 arnetheduck Exp $
 */


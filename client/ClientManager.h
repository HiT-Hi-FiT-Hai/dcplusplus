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

#if !defined(AFX_CLIENTMANAGER_H__8EF173E1_F7DC_40B5_B2F3_F92297701034__INCLUDED_)
#define AFX_CLIENTMANAGER_H__8EF173E1_F7DC_40B5_B2F3_F92297701034__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Client.h"
#include "CryptoManager.h"
#include "ConnectionManager.h"
#include "TimerManager.h"

class ClientManagerListener {
public:
	typedef ClientManagerListener* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;
	
	enum Types {
		USER_UPDATED,
		INCOMING_SEARCH
	};

	virtual void onAction(Types, const User::Ptr&) { };
	virtual void onAction(Types, const string&) { };
};

class ClientManager : public Speaker<ClientManagerListener>, private ClientListener, public Singleton<ClientManager>, private TimerManagerListener
{
public:
	Client* getClient();
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

	bool isConnected(const string& aServer) {
		Lock l(cs);

		for(Client::Iter i = clients.begin(); i != clients.end(); ++i) {
			if((*i)->getServer() == aServer || (*i)->getIp() == aServer) {
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

	void infoUpdated();

	User::Ptr& getUser(const string& aNick, const string& aHint = Util::emptyString);
	User::Ptr& getUser(const string& aNick, Client* aClient, bool putOnline = true);
	
	bool isOnline(const string& aNick) {
		Lock l(cs);
		UserPair i = users.equal_range(aNick);
		for(UserIter j = i.first; j != i.second; ++j) {
			if(j->second->isOnline())
				return true;
		}
		return false;
	}

	void ClientManager::putUserOffline(User::Ptr& aUser) {
		{
			Lock l(cs);
			aUser->unsetFlag(User::PASSIVE);
			aUser->unsetFlag(User::OP);
			aUser->unsetFlag(User::DCPLUSPLUS);
			aUser->setClient(NULL);
		}
		fire(ClientManagerListener::USER_UPDATED, aUser);
	}
	
private:
	typedef HASH_MULTIMAP<string, User::Ptr> UserMap;
	typedef UserMap::iterator UserIter;
	typedef pair<UserIter, UserIter> UserPair;

	Client::List clients;
	CriticalSection cs;
	int minutes;

	UserMap users;
	Socket s;

	friend class Singleton<ClientManager>;
	ClientManager() : minutes(0) { 
		try {
			s.create(Socket::TYPE_UDP); 
		} catch(SocketException e) {
			dcassert(0);
		}
		TimerManager::getInstance()->addListener(this); 
	};

	ClientManager(const ClientManager&) { dcassert(0); };	// No copying...
	virtual ~ClientManager() { TimerManager::getInstance()->removeListener(this); };

	// ClientListener
	virtual void onAction(ClientListener::Types type, Client* client, const string& line1, const string& line2);
	virtual void onAction(ClientListener::Types type, Client* client, const User::Ptr& user);
	virtual void onAction(ClientListener::Types type, Client* client, const User::List& aList);
	virtual void onAction(ClientListener::Types type, Client* client, const string& aSeeker, int aSearchType, const string& aSize, int aFileType, const string& aString);
	
	void onClientHello(Client* aClient, const User::Ptr& aUser) throw();
	void onClientSearch(Client* aClient, const string& aSeeker, int aSearchType, const string& aSize, 
		int aFileType, const string& aString) throw();

	// TimerManagerListener
	void onAction(TimerManagerListener::Types type, u_int8_t aTick);
	void onTimerMinute(u_int8_t aTick);
};

#endif // !defined(AFX_CLIENTMANAGER_H__8EF173E1_F7DC_40B5_B2F3_F92297701034__INCLUDED_)

/**
 * @file ClientManager.h
 * $Id: ClientManager.h,v 1.28 2002/05/30 19:09:33 arnetheduck Exp $
 */


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

#if !defined(AFX_CLIENTMANAGER_H__8EF173E1_F7DC_40B5_B2F3_F92297701034__INCLUDED_)
#define AFX_CLIENTMANAGER_H__8EF173E1_F7DC_40B5_B2F3_F92297701034__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Client.h"
#include "TimerManager.h"

#include "ClientManagerListener.h"

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

	bool isConnected(const string& aServer, short port) {
		Lock l(cs);

		for(Client::Iter i = clients.begin(); i != clients.end(); ++i) {
			if(((*i)->getServer() == aServer || (*i)->getIp() == aServer) && (*i)->getPort() == port) {
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
	void ClientManager::putUserOffline(User::Ptr& aUser, bool quitHub = false) {
		{
			Lock l(cs);
			aUser->unsetFlag(User::PASSIVE);
			aUser->unsetFlag(User::OP);
			aUser->unsetFlag(User::DCPLUSPLUS);
			if(quitHub)
				aUser->setFlag(User::QUIT_HUB);
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
	
	StringList features;
	UserMap users;
	Socket s;

	friend class Singleton<ClientManager>;
	ClientManager() { 
		TimerManager::getInstance()->addListener(this); 

		features.push_back("UserCommand");
	};

	// Dummy...
	ClientManager(const ClientManager&) throw() { dcassert(0); };

	virtual ~ClientManager() { TimerManager::getInstance()->removeListener(this); };

	// ClientListener
	virtual void onAction(ClientListener::Types type, Client* client, const string& line1, const string& line2) throw();
	virtual void onAction(ClientListener::Types type, Client* client, const User::Ptr& user) throw();
	virtual void onAction(ClientListener::Types type, Client* client, const User::List& aList) throw();
	virtual void onAction(ClientListener::Types type, Client* client, const string& aSeeker, int aSearchType, const string& aSize, int aFileType, const string& aString) throw();
	virtual void onAction(ClientListener::Types type, Client* client, int aType, int ctx, const string& name, const string& command) throw();

	void onClientHello(Client* aClient, const User::Ptr& aUser) throw();
	void onClientSearch(Client* aClient, const string& aSeeker, int aSearchType, const string& aSize, 
		int aFileType, const string& aString) throw();
	void onClientLock(Client* aClient, const string& aLock) throw();

	// TimerManagerListener
	void onAction(TimerManagerListener::Types type, u_int32_t aTick) throw();
	void onTimerMinute(u_int32_t aTick);
};

#endif // !defined(AFX_CLIENTMANAGER_H__8EF173E1_F7DC_40B5_B2F3_F92297701034__INCLUDED_)

/**
 * @file
 * $Id: ClientManager.h,v 1.35 2003/10/20 21:04:55 arnetheduck Exp $
 */


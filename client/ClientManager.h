/* 
 * Copyright (C) 2001 Jacek Sieka, jacek@creatio.se
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
#include "ShareManager.h"
#include "ConnectionManager.h"

class ClientManager : private ClientListener  
{
public:

	Client* getClient();
	void putClient(Client* aClient);

	int getTotalUserCount() {
		int c = 0;
		for(Client::Iter i = clients.begin(); i != clients.end(); ++i) {
			c+=(*i)->getUserCount();
		}
		return c;
	}

	User::Ptr& findUser(const string& aNick) {
		dcassert(aNick.length() > 0);
		cs.enter();
		for(Client::Iter i = clients.begin(); i != clients.end(); ++i) {
			User::Ptr& u = (*i)->getUser(aNick);
			if(u) {
				cs.leave();
				return u;
			}
		}
		cs.leave();
		return User::nuser;
	}
	
	bool isConnected(const string& aServer) {
		cs.enter();
		for(Client::Iter i = clients.begin(); i != clients.end(); ++i) {
			if((*i)->getServer() == aServer) {
				cs.leave();
				return true;
			}
		}
		cs.leave();
		return false;
	}
	
	void search(int aSearchType, LONGLONG aSize, int aFileType, const string& aString) {
		cs.enter();
		for(Client::Iter i = clients.begin(); i != clients.end(); ++i) {
			(*i)->cs.enter();
			(*i)->search(aSearchType, aSize, aFileType, aString);
			(*i)->cs.leave();
		}
		cs.leave();
	}

	static void newInstance() {
		if(instance)
			delete instance;
		
		instance = new ClientManager();
	}
	static ClientManager* getInstance() {
		dcassert(instance);
		return instance;
	}
	static void deleteInstance() {
		if(instance)
			delete instance;
		instance = NULL;
	}
	
	
private:
	Client::List clients;

	CriticalSection cs;
	static ClientManager* instance;

	ClientManager() { };
	virtual ~ClientManager() { };

	// ClientListener
	virtual void onClientLock(Client::Ptr aClient, const string& aLock, const string& aPk) {
		aClient->cs.enter();
		aClient->key(CryptoManager::getInstance()->makeKey(aLock));
		aClient->validateNick(Settings::getNick());
		aClient->cs.leave();
	}
	virtual void onClientHello(Client::Ptr aClient, User::Ptr& aUser) {
		aClient->cs.enter();
		if(aUser->getNick() == Settings::getNick()) {
			aClient->version("1,0091");
			aClient->getNickList();
			aClient->myInfo(Settings::getNick(), Settings::getDescription(), Settings::getConnection(), Settings::getEmail(), ShareManager::getInstance()->getShareSizeString());
		} else {
			aClient->getInfo(aUser);
		}
		aClient->cs.leave();
	}
	virtual void onClientNickList(Client::Ptr aClient, StringList& aNicks) {
		aClient->cs.enter();
		for(StringIter i = aNicks.begin(); i != aNicks.end(); ++i) {
			aClient->getInfo(*i);
		}
		aClient->cs.leave();
	}
	
	virtual void onClientOpList(Client::Ptr aClient, StringList& aNicks) {
		aClient->cs.enter();
		for(StringIter i = aNicks.begin(); i != aNicks.end(); ++i) {
			aClient->getInfo(*i);
		}
		aClient->cs.leave();
	}
	
	virtual void onClientConnectToMe(Client::Ptr aClient, const string& aServer, const string& aPort) {
		ConnectionManager::getInstance()->connect(aServer, atoi(aPort.c_str()));
	}
	
	virtual void onClientRevConnectToMe(Client::Ptr aClient, User::Ptr& aUser) {
		aClient->cs.enter();
		aClient->connectToMe(aUser);
		aClient->cs.leave();
	}
	
	
};

#endif // !defined(AFX_CLIENTMANAGER_H__8EF173E1_F7DC_40B5_B2F3_F92297701034__INCLUDED_)

/**
 * @file ClientManager.h
 * $Id: ClientManager.h,v 1.1 2001/12/21 18:46:18 arnetheduck Exp $
 * @if LOG
 * $Log: ClientManager.h,v $
 * Revision 1.1  2001/12/21 18:46:18  arnetheduck
 * Replaces ProtocolHandler with enhanced functionality
 *
 * @endif
 */


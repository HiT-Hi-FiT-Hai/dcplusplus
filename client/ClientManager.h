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
#include "ConnectionManager.h"

class ClientManager : private ClientListener, public Singleton<ClientManager>
{
public:
	Client* getConnectedClient() { 
		Client* ret = NULL;
		
		cs.enter(); 
		if(clients.size() > 0) 
			ret = clients.front(); 
		cs.leave(); 

		return ret; 
	};

	Client* getClient();
	void putClient(Client* aClient);

	int getTotalUserCount() {
		int c = 0;
		for(Client::Iter i = clients.begin(); i != clients.end(); ++i) {
			c+=(*i)->getUserCount();
		}
		return c;
	}

	User::Ptr& findUser(const string& aNick, const string& aHint = "") {
		dcassert(aNick.length() > 0);
		cs.enter();
		if(aHint.size() > 0) {
			for(Client::Iter i = clients.begin(); i != clients.end(); ++i) {
				if((*i)->getServer() == aHint) {
					User::Ptr& u = (*i)->getUser(aNick);
					if(u) {
						cs.leave();
						return u;
					} else {
						break;
					}
				} 
			}
			
		}
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
			if((*i)->isConnected()) {
				(*i)->search(aSearchType, aSize, aFileType, aString);
			}
		}
		cs.leave();
	}

private:
	Client::List clients;
	CriticalSection cs;
	
	friend class Singleton<ClientManager>;
	ClientManager() { };
	virtual ~ClientManager() { };

	// ClientListener
	virtual void onAction(ClientListener::Types type, Client* client, const string& line1, const string& line2) {
		switch(type) {
		case ClientListener::LOCK:
			client->cs.enter();
			client->key(CryptoManager::getInstance()->makeKey(line1));
			client->validateNick(client->getNick());
			client->cs.leave();
			break;
		case ClientListener::CONNECT_TO_ME:
			ConnectionManager::getInstance()->connect(line1, Util::toInt(line2), client->getNick()); break;

		}
	}
	virtual void onAction(ClientListener::Types type, Client* client, const User::Ptr& user) {
		switch(type) {
		case ClientListener::HELLO:
			onClientHello(client, user); break;
		case ClientListener::REV_CONNECT_TO_ME:
			if(SETTING(CONNECTION_TYPE) == SettingsManager::CONNECTION_ACTIVE) {
				client->cs.enter();
				client->connectToMe(user);
				client->cs.leave();
			}
			break;
			
		}
	}
	virtual void onAction(ClientListener::Types type, Client* client, const StringList& aList) {
		switch(type) {
		case ClientListener::NICK_LIST:		// Fall through...
		case ClientListener::OP_LIST:
			client->cs.enter();
			for(StringIterC i = aList.begin(); i != aList.end(); ++i) {
				// Make sure we're indeed connected (if the server resets on the first getInfo, 
				// we'll on trying aNicks.size times...not good...)
				if(!client->isConnected()) {
					break;
				}
				client->getInfo(*i);
			}
			client->cs.leave();
			
		}
	}
	virtual void onAction(ClientListener::Types type, Client* aClient, const string& aSeeker, int aSearchType, const string& aSize, 
		int aFileType, const string& aString) {
		switch(type) {
		case ClientListener::SEARCH:
			onClientSearch(aClient, aSeeker, aSearchType, aSize, aFileType, aString);
		}
	}
		
	
	void onClientHello(Client* aClient, const User::Ptr& aUser) throw();

	void onClientSearch(Client* aClient, const string& aSeeker, int aSearchType, const string& aSize, 
		int aFileType, const string& aString) throw();
	
};

#endif // !defined(AFX_CLIENTMANAGER_H__8EF173E1_F7DC_40B5_B2F3_F92297701034__INCLUDED_)

/**
 * @file ClientManager.h
 * $Id: ClientManager.h,v 1.7 2002/01/13 22:50:47 arnetheduck Exp $
 * @if LOG
 * $Log: ClientManager.h,v $
 * Revision 1.7  2002/01/13 22:50:47  arnetheduck
 * Time for 0.12, added favorites, a bunch of new icons and lot's of other stuff
 *
 * Revision 1.6  2002/01/11 14:52:56  arnetheduck
 * Huge changes in the listener code, replaced most of it with templates,
 * also moved the getinstance stuff for the managers to a template
 *
 * Revision 1.5  2002/01/07 20:17:59  arnetheduck
 * Finally fixed the reconnect bug that's been annoying me for a whole day...
 * Hopefully the app works better in w95 now too...
 *
 * Revision 1.4  2002/01/06 11:13:07  arnetheduck
 * Last fixes before 0.10
 *
 * Revision 1.3  2002/01/06 00:14:54  arnetheduck
 * Incoming searches almost done, just need some testing...
 *
 * Revision 1.2  2001/12/30 15:03:45  arnetheduck
 * Added framework to handle incoming searches
 *
 * Revision 1.1  2001/12/21 18:46:18  arnetheduck
 * Replaces ProtocolHandler with enhanced functionality
 *
 * @endif
 */


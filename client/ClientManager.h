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

class ClientManager : private ClientListener, public Singleton<ClientManager>, private TimerManagerListener
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

	LONGLONG getAvailable() {
		Lock l(cs);
		
		LONGLONG c = 0;
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
	
	void search(int aSearchType, LONGLONG aSize, int aFileType, const string& aString) {
		Lock l(cs);

		for(Client::Iter i = clients.begin(); i != clients.end(); ++i) {
			if((*i)->isConnected()) {
				(*i)->search(aSearchType, aSize, aFileType, aString);
			}
		}
	}

	void infoUpdated();

	User::Ptr& getUser(const string& aNick, const string& aHint = Util::emptyString);
	User::Ptr& getUser(const string& aNick, Client* aClient, bool putOnline = true);
	
	bool isOnline(const string& aNick) {
		Lock l(cs);
		UserIter i = users.find(aNick);
		if(i != users.end()) {
			return i->second->isOnline();
		}
		return false;
	}

	void putUserOffline(User::Ptr& aUser);
private:
	typedef HASH_MULTIMAP<string, User::Ptr> UserMap;
	typedef UserMap::iterator UserIter;
	typedef pair<UserIter, UserIter> UserPair;

	Client::List clients;
	CriticalSection cs;
	int minutes;

	UserMap users;

	friend class Singleton<ClientManager>;
	ClientManager() : minutes(0) { TimerManager::getInstance()->addListener(this); };
	virtual ~ClientManager() { TimerManager::getInstance()->removeListener(this); };

	// ClientListener
	virtual void onAction(ClientListener::Types type, Client* client, const string& line1, const string& line2) {
		switch(type) {
		case ClientListener::LOCK:
			client->key(CryptoManager::getInstance()->makeKey(line1));
			client->validateNick(client->getNick());
			break;
		case ClientListener::CONNECT_TO_ME:
			ConnectionManager::getInstance()->connect(line1, (short)Util::toInt(line2), client->getNick()); break;

		}
	}
	virtual void onAction(ClientListener::Types type, Client* client, const User::Ptr& user) {
		switch(type) {
		case ClientListener::HELLO:
			onClientHello(client, user); break;
		case ClientListener::REV_CONNECT_TO_ME:
			if(SETTING(CONNECTION_TYPE) == SettingsManager::CONNECTION_ACTIVE) {
				client->connectToMe(user);
			}
			break;
			
		}
	}
	virtual void onAction(ClientListener::Types type, Client* client, const User::List& aList) {
		switch(type) {
		case ClientListener::NICK_LIST:		// Fall through...
		case ClientListener::OP_LIST:
			for(User::List::const_iterator i = aList.begin(); i != aList.end(); ++i) {
				// Make sure we're indeed connected (if the server resets on the first getInfo, 
				// we'll on trying aNicks.size times...not good...)
				if(!client->isConnected()) {
					break;
				}
				client->getInfo(*i);
				if(type == OP_LIST) {
					if((*i)->getNick() == client->getNick())
						client->setOp(true);
				}
			}
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

	// TimerManagerListener
	void onAction(TimerManagerListener::Types type, DWORD /*aTick*/) {
		if(type == TimerManagerListener::MINUTE) {
			if(minutes++ >= 5) {
				minutes = 0;
				Lock l(cs);
				UserIter i = users.begin();
				while(i != users.end()) {
					if(i->second->unique()) {
						users.erase(i++);
					} else {
						++i;
					}
				}
			}
		}
	}
};

#endif // !defined(AFX_CLIENTMANAGER_H__8EF173E1_F7DC_40B5_B2F3_F92297701034__INCLUDED_)

/**
 * @file ClientManager.h
 * $Id: ClientManager.h,v 1.17 2002/03/04 23:52:30 arnetheduck Exp $
 * @if LOG
 * $Log: ClientManager.h,v $
 * Revision 1.17  2002/03/04 23:52:30  arnetheduck
 * Updates and bugfixes, new user handling almost finished...
 *
 * Revision 1.16  2002/02/28 00:10:47  arnetheduck
 * Some fixes to the new user model
 *
 * Revision 1.15  2002/02/27 12:02:09  arnetheduck
 * Completely new user handling, wonder how it turns out...
 *
 * Revision 1.14  2002/02/25 15:39:28  arnetheduck
 * Release 0.154, lot of things fixed...
 *
 * Revision 1.13  2002/02/09 18:13:51  arnetheduck
 * Fixed level 4 warnings and started using new stl
 *
 * Revision 1.12  2002/01/26 14:59:22  arnetheduck
 * Fixed disconnect crash
 *
 * Revision 1.11  2002/01/25 00:11:26  arnetheduck
 * New settings dialog and various fixes
 *
 * Revision 1.10  2002/01/20 22:54:46  arnetheduck
 * Bugfixes to 0.131 mainly...
 *
 * Revision 1.9  2002/01/18 17:41:43  arnetheduck
 * Reworked many right button menus, adding op commands and making more easy to use
 *
 * Revision 1.8  2002/01/17 23:35:59  arnetheduck
 * Reworked threading once more, now it actually seems stable. Also made
 * sure that noone tries to access client objects that have been deleted
 * as well as some other minor updates
 *
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


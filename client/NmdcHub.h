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

#if !defined(AFX_NmdcHub_H__089CBD05_4833_4E30_9A57_BB636231D78E__INCLUDED_)
#define AFX_NmdcHub_H__089CBD05_4833_4E30_9A57_BB636231D78E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TimerManager.h"
#include "SettingsManager.h"

#include "ClientManager.h"

#include "BufferedSocket.h"
#include "User.h"
#include "CriticalSection.h"

class NmdcHub;

class NmdcHubListener  
{
public:
	typedef NmdcHubListener* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;
	
	enum Types {
		BAD_PASSWORD,
		CONNECT_TO_ME,
		CONNECTED,
		CONNECTING,
		FAILED,
		FORCE_MOVE,
		GET_PASSWORD,
		HELLO,
		HUB_NAME,
		HUB_FULL,
		SUPPORTS,
		C_LOCK,
		LOGGED_IN,
		MESSAGE,
		MY_INFO,
		NICK_LIST,
		OP_LIST,
		USER_IP,
		PRIVATE_MESSAGE,
		REV_CONNECT_TO_ME,
		SEARCH,
		QUIT,
		UNKNOWN,
		USER_COMMAND,
		VALIDATE_DENIED,
		SEARCH_FLOOD
	};
	
	virtual void onAction(Types, NmdcHub*) throw() { };
	virtual void onAction(Types, NmdcHub*, const string&) throw() { };
	virtual void onAction(Types, NmdcHub*, const string&, const string&) throw() { };
	virtual void onAction(Types, NmdcHub*, const User::Ptr&) throw() { };
	virtual void onAction(Types, NmdcHub*, const User::List&) throw() { };		// USER_IP
	virtual void onAction(Types, NmdcHub*, const User::Ptr&, const string&) throw() { };
	virtual void onAction(Types, NmdcHub*, const string&, int, const string&, int, const string&) throw() { };
	virtual void onAction(Types, NmdcHub*, int, int, const string&, const string&) throw() { }; // USER_COMMAND
	virtual void onAction(Types, NmdcHub*, const StringList&) throw() { };		// SUPPORTS
};

class NmdcHub : public Client, public Speaker<NmdcHubListener>, private TimerManagerListener, private Flags
{
	friend class ClientManager;
public:
	typedef NmdcHub* Ptr;
	typedef list<Ptr> List;
	typedef List::iterator Iter;

	enum SupportFlags {
		SUPPORTS_USERCOMMAND = 0x01,
		SUPPORTS_NOGETINFO = 0x02,
		SUPPORTS_USERIP2 = 0x04,
	};

	User::NickMap& lockUserList() { cs.enter(); return users; };
	void unlockUserList() { cs.leave(); };

	void disconnect() throw();
	void myInfo();
	
	void refreshUserList(bool unknownOnly = false);

#define checkstate() if(state != STATE_CONNECTED) return

	void validateNick(const string& aNick) { send("$ValidateNick " + aNick + "|"); }
	void key(const string& aKey) { send("$Key " + aKey + "|"); };	
	void version(const string& aVersion) { send("$Version " + aVersion + "|"); };
	void getNickList() { checkstate(); send("$GetNickList|"); };
	void password(const string& aPass) { send("$MyPass " + aPass + "|"); };
	void getInfo(User::Ptr aUser) { checkstate(); send("$GetINFO " + aUser->getNick() + " " + getNick() + "|"); };
	void getInfo(User* aUser) {  checkstate(); send("$GetINFO " + aUser->getNick() + " " + getNick() + "|"); };
	void hubMessage(const string& aMessage) { checkstate(); send("<" + getNick() + "> " + Util::validateMessage(aMessage, false) + "|"); }

	void info() { myInfo(); }

	void search(int aSizeType, int64_t aSize, int aFileType, const string& aString);
	
	void connectToMe(const User::Ptr& aUser) {
		checkstate(); 
		dcdebug("NmdcHub::connectToMe %s\n", aUser->getNick().c_str());
		send("$ConnectToMe " + aUser->getNick() + " " + getLocalIp() + ":" + Util::toString(SETTING(IN_PORT)) + "|");
	}
	void connect(const User* aUser) {
		checkstate(); 
		dcdebug("NmdcHub::connectToMe %s\n", aUser->getNick().c_str());
		if(SETTING(CONNECTION_TYPE) == SettingsManager::CONNECTION_ACTIVE) {
			send("$ConnectToMe " + aUser->getNick() + " " + getLocalIp() + ":" + Util::toString(SETTING(IN_PORT)) + "|");
		} else {
			send("$RevConnectToMe " + getNick() + " " + aUser->getNick()  + "|");
		}
	}
	void privateMessage(const User::Ptr& aUser, const string& aMessage) {
		privateMessage(aUser->getNick(), aMessage);
	}
	void privateMessage(const User* aUser, const string& aMessage) {
		privateMessage(aUser->getNick(), aMessage);
	}
	void privateMessage(const string& aNick, const string& aMessage) {
		checkstate(); 
		send("$To: " + aNick + " From: " + getNick() + " $" + Util::validateMessage(aMessage, false) + "|");
	}
	void supports(const StringList& feat) { 
		string x;
		for(StringList::const_iterator i = feat.begin(); i != feat.end(); ++i) {
			x+= *i + ' ';
		}
		send("$Supports " + x + '|');
	}
	void revConnectToMe(const User::Ptr& aUser) {
		checkstate(); 
		dcdebug("NmdcHub::revConnectToMe %s\n", aUser->getNick().c_str());
		send("$RevConnectToMe " + getNick() + " " + aUser->getNick()  + "|");
	}

	void send(const string& a) throw() {
		lastActivity = GET_TICK();
		//dcdebug("Sending %d to %s: %.40s\n", a.size(), getName().c_str(), a.c_str());
		socket->write(a);
	}
	void send(const char* aBuf, int aLen) throw() {
		lastActivity = GET_TICK();
		socket->write(aBuf, aLen);
	}

	void kick(const User::Ptr& aUser, const string& aMsg);
	void kick(const User* aUser, const string& aMsg);

	virtual void ban(const User* user, const string& aMessage, time_t seconds) {
		// Unimplemented...
	}
	void opForceMove(const User::Ptr& aUser, const string& aServer, const string& aMsg) {
		checkstate(); 
		dcdebug("NmdcHub::opForceMove\n");
		send("$OpForceMove $Who:" + aUser->getNick() + "$Where:" + aServer + "$Msg:" + aMsg + "|");
	}

	void redirect(const User* aUser, const string& aServer, const string& aMsg) {
		checkstate(); 
		dcdebug("NmdcHub::opForceMove\n");
		send("$OpForceMove $Who:" + aUser->getNick() + "$Where:" + aServer + "$Msg:" + aMsg + "|");
	}

	int getUserCount() const {
		Lock l(cs);
		return users.size();
	}

	int64_t getAvailable() const {
		Lock l(cs);
		int64_t x = 0;
		for(User::NickMap::const_iterator i = users.begin(); i != users.end(); ++i) {
			x+=i->second->getBytesShared();
		}
		return x;
	}

	const string& getName() const { return name; };
	bool getOp() const { return getMe() ? getMe()->isSet(User::OP) : false; };

	GETSET(int, supportFlags, SupportFlags);
private:

	struct ClientAdapter : public NmdcHubListener {
		ClientAdapter(NmdcHub* aClient) : c(aClient) { aClient->Speaker<NmdcHubListener>::addListener(this); }
		Client* c;

		virtual void onAction(NmdcHubListener::Types type, NmdcHub*) throw() { 
			switch(type) {
				case NmdcHubListener::CONNECTING: c->fire(ClientListener::CONNECTING, c); break;
				case NmdcHubListener::CONNECTED: c->fire(ClientListener::CONNECTED, c); break;
				case NmdcHubListener::BAD_PASSWORD: c->fire(ClientListener::BAD_PASSWORD, c); break;
				case NmdcHubListener::GET_PASSWORD: c->fire(ClientListener::GET_PASSWORD, c); break;
				case NmdcHubListener::HUB_FULL: c->fire(ClientListener::HUB_FULL, c); break;
				case NmdcHubListener::VALIDATE_DENIED: c->fire(ClientListener::NICK_TAKEN, c); break;
				case NmdcHubListener::HUB_NAME: c->fire(ClientListener::HUB_NAME, c); break;
				default: break;
			}
		};

		virtual void onAction(NmdcHubListener::Types type, NmdcHub*, const string& line1) throw() { 
			switch(type) {
				case NmdcHubListener::MESSAGE: c->fire(ClientListener::MESSAGE, c, line1); break;
				case NmdcHubListener::FORCE_MOVE: c->fire(ClientListener::FORCE_MOVE, c, line1); break;
				case NmdcHubListener::FAILED: c->fire(ClientListener::FAILED, c, line1); break;
				case NmdcHubListener::SEARCH_FLOOD: c->fire(ClientListener::SEARCH_FLOOD, c, line1); break;
				default: break;
			}
		};
		virtual void onAction(NmdcHubListener::Types, NmdcHub*, const string&, const string&) throw() { };
		virtual void onAction(NmdcHubListener::Types type, NmdcHub*, const User::Ptr& u) throw() { 
			switch(type) {
				case NmdcHubListener::MY_INFO:	// fallthrough
				case NmdcHubListener::HELLO: c->fire(ClientListener::USER_UPDATED, c, u); break;
				case NmdcHubListener::QUIT: c->fire(ClientListener::USER_REMOVED, c, u); break;
				default: break;
			}
		};
		virtual void onAction(NmdcHubListener::Types type, NmdcHub*, const User::List& u) throw() {
			switch(type) {
				case NmdcHubListener::OP_LIST: c->fire(ClientListener::USERS_UPDATED, c, u); break;
				case NmdcHubListener::NICK_LIST: c->fire(ClientListener::USERS_UPDATED, c, u); break;
				default: break;
			}
		};		// USER_IP
		virtual void onAction(NmdcHubListener::Types type, NmdcHub*, const User::Ptr& u, const string& line1) throw() { 
			switch(type) {
				case NmdcHubListener::PRIVATE_MESSAGE: c->fire(ClientListener::PRIVATE_MESSAGE, c, u, line1);
				default: break;
			}
		};

		virtual void onAction(NmdcHubListener::Types type, NmdcHub*, const string& line1, int a, const string& line2, int b, const string& line3) throw() { 
			switch(type) {
				case NmdcHubListener::SEARCH: c->fire(ClientListener::SEARCH, c, line1, a, line2, b, line3); break;
				default: break;
			}
		};
		virtual void onAction(NmdcHubListener::Types type, NmdcHub*, int a, int b, const string& line1, const string& line2) throw() { 
			switch(type) {
				case NmdcHubListener::USER_COMMAND: c->fire(ClientListener::USER_COMMAND, c, a, b, line1, line2); break;
				default: break;
			}
		}; // USER_COMMAND
		virtual void onAction(NmdcHubListener::Types, NmdcHub*, const StringList&) throw() { };		// SUPPORTS

	} adapter;
	enum States {
		STATE_CONNECT,
		STATE_LOCK,
		STATE_HELLO,
		STATE_CONNECTED
	} state;

	string name;

	u_int32_t lastActivity;

	mutable CriticalSection cs;

	User::NickMap users;

	bool reconnect;
	u_int32_t lastUpdate;
	string lastMyInfo;

	
	typedef list<pair<string, u_int32_t> > FloodMap;
	typedef FloodMap::iterator FloodIter;
	FloodMap seekers;
	FloodMap flooders;

	NmdcHub(const string& aHubURL);	
	virtual ~NmdcHub() throw();

	// Dummy
	NmdcHub(const NmdcHub&);
	NmdcHub& operator=(const NmdcHub&);

	void connect();

	void clearUsers();
	void onLine(const string& aLine) throw();
	
	// TimerManagerListener
	virtual void onAction(TimerManagerListener::Types type, u_int32_t aTick) throw();

	// BufferedSocketListener
	virtual void onAction(BufferedSocketListener::Types type, const string& aLine) throw();
	virtual void onAction(BufferedSocketListener::Types type) throw();

};

#endif // !defined(AFX_NmdcHub_H__089CBD05_4833_4E30_9A57_BB636231D78E__INCLUDED_)

/**
 * @file
 * $Id: NmdcHub.h,v 1.1 2004/04/04 12:11:51 arnetheduck Exp $
 */


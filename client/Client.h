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

#if !defined(AFX_Client_H__089CBD05_4833_4E30_9A57_BB636231D78E__INCLUDED_)
#define AFX_Client_H__089CBD05_4833_4E30_9A57_BB636231D78E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BufferedSocket.h"
#include "User.h"
#include "Util.h"
#include "TimerManager.h"
#include "CriticalSection.h"

class Client;

class ClientListener  
{
public:
	typedef ClientListener* Ptr;
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
		C_LOCK,
		LOGGED_IN,
		MESSAGE,
		MY_INFO,
		NICK_LIST,
		OP_LIST,
		PRIVATE_MESSAGE,
		REV_CONNECT_TO_ME,
		SEARCH,
		QUIT,
		UNKNOWN,
		VALIDATE_DENIED,
		SEARCH_FLOOD
	};
	
	virtual void onAction(Types, Client*) { };
	virtual void onAction(Types, Client*, const string&) { };
	virtual void onAction(Types, Client*, const string&, const string&) { };
	virtual void onAction(Types, Client*, const User::Ptr&) { };
	virtual void onAction(Types, Client*, const User::List&) { };
	virtual void onAction(Types, Client*, const User::Ptr&, const string&) { };
	virtual void onAction(Types, Client*, const string&, int, const string&, int, const string&) { };
};

class Client : public Speaker<ClientListener>, private BufferedSocketListener, private TimerManagerListener
{
	friend class ClientManager;
public:
	typedef Client* Ptr;
	typedef list<Ptr> List;
	typedef List::iterator Iter;

	bool isConnected() { if(!socket) return false; else return socket->isConnected(); };
	void disconnect() throw();

	void refreshUserList();

#define checkstate() if(state != STATE_CONNECTED) return

	void validateNick(const string& aNick) { send("$ValidateNick " + aNick + "|"); }
	void key(const string& aKey) { send("$Key " + aKey + "|"); };	
	void version(const string& aVersion) { send("$Version " + aVersion + "|"); };
	void getNickList() { checkstate(); send("$GetNickList|"); };
	void password(const string& aPass) { send("$MyPass " + aPass + "|"); };
	void getInfo(User::Ptr aUser) { checkstate(); if(getUserInfo()) send("$GetINFO " + aUser->getNick() + " " + getNick() + "|"); };
	void getInfo(User* aUser) {  checkstate(); if(getUserInfo())send("$GetINFO " + aUser->getNick() + " " + getNick() + "|"); };
	void sendMessage(const string& aMessage) { checkstate(); send("<" + getNick() + "> " + Util::validateMessage(aMessage) + "|"); }

	void search(int aSizeType, int64_t aSize, int aFileType, const string& aString);
	void searchResults(const string& aResults) { send(aResults); }
	
	void myInfo(const string& aNick, const string& aDescription, const string& aSpeed, const string& aEmail, const string& aBytesShared) {
		checkstate();

		dcdebug("MyInfo %s...\n", aNick.c_str());
		lastHubs = hubs;
		lastUpdate = GET_TICK();
		string tmp1 = ";**\x1fU9";
		string tmp2 = "+L9";
		string tmp3 = "+G9";
		string tmp4 = "+R9";
		
		string::size_type i;

		for(i = 0; i < tmp1.size(); i++) {
			tmp1[i]++;
		}
		for(i = 0; i < tmp2.size(); i++) {
			tmp2[i]++; tmp3[i]++; tmp4[i]++;
		}
		send("$MyINFO $ALL " + Util::validateNick(aNick) + " " + Util::validateMessage(aDescription) + 
			tmp1 + VERSIONSTRING + tmp2 + ((SETTING(CONNECTION_TYPE) == SettingsManager::CONNECTION_ACTIVE) ? string("A") : string("P")) + 
			tmp3 + Util::toString(lastHubs) + tmp4 + Util::toString(SETTING(SLOTS)) + 
			">$ $" + aSpeed + "\x01$" + Util::validateMessage(aEmail) + '$' + aBytesShared + "$|");
	}

	void connectToMe(const User::Ptr& aUser) {
		checkstate(); 
		dcdebug("Client::connectToMe %s\n", aUser->getNick().c_str());
		send("$ConnectToMe " + aUser->getNick() + " " + Socket::resolve(SETTING(SERVER)) + ":" + Util::toString(SETTING(PORT)) + "|");
	}
	void connectToMe(User* aUser) {
		checkstate(); 
		dcdebug("Client::connectToMe %s\n", aUser->getNick().c_str());
		send("$ConnectToMe " + aUser->getNick() + " " + Socket::resolve(SETTING(SERVER)) + ":" + Util::toString(SETTING(PORT)) + "|");
	}
	void privateMessage(const User::Ptr& aUser, const string& aMessage) {
		checkstate(); 
		send("$To: " + aUser->getNick() + " From: " + getNick() + " $" + Util::validateMessage(aMessage) + "|");
	}
	void privateMessage(User* aUser, const string& aMessage) {
		checkstate(); 
		send("$To: " + aUser->getNick() + " From: " + getNick() + " $" + Util::validateMessage(aMessage) + "|");
	}
	void revConnectToMe(const User::Ptr& aUser) {
		checkstate(); 
		dcdebug("Client::revConnectToMe %s\n", aUser->getNick().c_str());
		send("$RevConnectToMe " + getNick() + " " + aUser->getNick()  + "|");
	}
	void revConnectToMe(User* aUser) {
		checkstate(); 
		dcdebug("Client::revConnectToMe %s\n", aUser->getNick().c_str());
		send("$RevConnectToMe " + getNick() + " " + aUser->getNick()  + "|");
	}
	
	void kick(const User::Ptr& aUser, const string& aMsg);
	void kick(User* aUser, const string& aMsg);
	
	void opForceMove(const User::Ptr& aUser, const string& aServer, const string& aMsg) {
		checkstate(); 
		dcdebug("Client::opForceMove\n");
		send("$OpForceMove $Who:" + aUser->getNick() + "$Where:" + aServer + "$Msg:" + aMsg + "|");
	}

	void connect(const string& aServer);
	
	void updated(User::Ptr& aUser) {
		fire(ClientListener::MY_INFO, this, aUser);
	}

	const string& getName() { return name; };
	const string& getServer() { return server; };

	int getUserCount() throw() {
		Lock l(cs);
		return users.size();
	}

	int64_t getAvailable() throw() {
		Lock l(cs);
		int64_t x = 0;
		for(User::NickIter i = users.begin(); i != users.end(); ++i) {
			x+=i->second->getBytesShared();
		}
		return x;
	}
	const string& getNick() {
		if(nick.empty()) {
			return SETTING(NICK);
		} else {
			return nick;
		}
	}

	void setNick(const string& aNick) {
		nick = aNick;
	}
	
	const string& getIp() {
		dcassert(socket);
		return socket->getIp();
	}
	
	GETSET(bool, userInfo, UserInfo);
	GETSET(bool, op, Op);
	GETSET(bool, registered, Registered);
	GETSETREF(string, defpassword, Password);
private:
	enum States {
		STATE_CONNECT,
		STATE_LOCK,
		STATE_HELLO,
		STATE_CONNECTED
	} state;
	string nick;
	string server;
	short port;
	BufferedSocket* socket;
	string name;
	u_int32_t lastActivity;

	CriticalSection cs;

	User::NickMap users;
	static long hubs;

	int lastHubs;
	bool counted;
	u_int32_t lastUpdate;
	
	typedef deque<pair<string, u_int32_t> > FloodMap;
	typedef FloodMap::iterator FloodIter;
	FloodMap seekers;
	FloodMap flooders;

	Client() : userInfo(true), op(false), registered(false), state(STATE_CONNECT), socket(NULL), lastActivity(GET_TICK()), lastHubs(0), counted(false) {
		TimerManager::getInstance()->addListener(this);
	};
	
	// No copying...
	Client(const Client&) { dcassert(0); };
	virtual ~Client() throw();
	void connect();

	void clearUsers();
	void onLine(const string& aLine) throw();
	
	// TimerManagerListener
	virtual void onAction(TimerManagerListener::Types type, u_int32_t aTick);

	// BufferedSocketListener
	virtual void onAction(BufferedSocketListener::Types type, const string& aLine);
	virtual void onAction(BufferedSocketListener::Types type);

	void send(const string& a) throw() {
		lastActivity = GET_TICK();
		//dcdebug("Sending %d to %s: %.40s\n", a.size(), getName().c_str(), a.c_str());
		socket->write(a);
	}
};


#endif // !defined(AFX_Client_H__089CBD05_4833_4E30_9A57_BB636231D78E__INCLUDED_)

/**
 * @file Client.h
 * $Id: Client.h,v 1.56 2002/05/26 20:28:10 arnetheduck Exp $
 */


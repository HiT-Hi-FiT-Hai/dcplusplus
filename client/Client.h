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

#if !defined(AFX_Client_H__089CBD05_4833_4E30_9A57_BB636231D78E__INCLUDED_)
#define AFX_Client_H__089CBD05_4833_4E30_9A57_BB636231D78E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BufferedSocket.h"
#include "User.h"
#include "TimerManager.h"
#include "CriticalSection.h"
#include "SettingsManager.h"

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
		SUPPORTS,
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
		USER_COMMAND,
		VALIDATE_DENIED,
		SEARCH_FLOOD
	};
	
	virtual void onAction(Types, Client*) throw() { };
	virtual void onAction(Types, Client*, const string&) throw() { };
	virtual void onAction(Types, Client*, const string&, const string&) throw() { };
	virtual void onAction(Types, Client*, const User::Ptr&) throw() { };
	virtual void onAction(Types, Client*, const User::List&) throw() { };
	virtual void onAction(Types, Client*, const User::Ptr&, const string&) throw() { };
	virtual void onAction(Types, Client*, const string&, int, const string&, int, const string&) throw() { };
	virtual void onAction(Types, Client*, int, int, const string&, const string&) throw() { }; // USER_COMMAND
	virtual void onAction(Types, Client*, const StringList&) throw() { };		// SUPPORTS
};

class Client : public Speaker<ClientListener>, private BufferedSocketListener, private TimerManagerListener, private Flags
{
	friend class ClientManager;
public:
	typedef Client* Ptr;
	typedef list<Ptr> List;
	typedef List::iterator Iter;

	enum SupportFlags {
		SUPPORTS_USERCOMMAND = 0x01,
		SUPPORTS_NOGETINFO = 0x02, 
	};

	User::NickMap& lockUserList() throw() { cs.enter(); return users; };
	void unlockUserList() throw() { cs.leave(); };

	bool isConnected() { return socket->isConnected(); };
	void disconnect() throw();
	void myInfo();
	
	void refreshUserList(bool unknownOnly = false);

#define checkstate() if(state != STATE_CONNECTED) return

	void validateNick(const string& aNick) { send("$ValidateNick " + aNick + "|"); }
	void key(const string& aKey) { send("$Key " + aKey + "|"); };	
	void version(const string& aVersion) { send("$Version " + aVersion + "|"); };
	void getNickList() { checkstate(); send("$GetNickList|"); };
	void password(const string& aPass) { send("$MyPass " + aPass + "|"); };
	void getInfo(User::Ptr aUser) { checkstate(); if(getUserInfo()) send("$GetINFO " + aUser->getNick() + " " + getNick() + "|"); };
	void getInfo(User* aUser) {  checkstate(); if(getUserInfo())send("$GetINFO " + aUser->getNick() + " " + getNick() + "|"); };
	void sendMessage(const string& aMessage) { checkstate(); send("<" + getNick() + "> " + Util::validateMessage(aMessage, false) + "|"); }

	void search(int aSizeType, int64_t aSize, int aFileType, const string& aString);
	void searchResults(const string& aResults) { send(aResults); };
	
	void connectToMe(const User::Ptr& aUser) {
		checkstate(); 
		dcdebug("Client::connectToMe %s\n", aUser->getNick().c_str());
		send("$ConnectToMe " + aUser->getNick() + " " + getLocalIp() + ":" + Util::toString(SETTING(IN_PORT)) + "|");
	}
	void connectToMe(User* aUser) {
		checkstate(); 
		dcdebug("Client::connectToMe %s\n", aUser->getNick().c_str());
		send("$ConnectToMe " + aUser->getNick() + " " + getLocalIp() + ":" + Util::toString(SETTING(IN_PORT)) + "|");
	}
	void privateMessage(const User::Ptr& aUser, const string& aMessage) {
		privateMessage(aUser->getNick(), aMessage);
	}
	void privateMessage(User* aUser, const string& aMessage) {
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
		dcdebug("Client::revConnectToMe %s\n", aUser->getNick().c_str());
		send("$RevConnectToMe " + getNick() + " " + aUser->getNick()  + "|");
	}
	void revConnectToMe(User* aUser) {
		checkstate(); 
		dcdebug("Client::revConnectToMe %s\n", aUser->getNick().c_str());
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
	string getServerWithPort() const { return port == 411 ? server : server + ':' + Util::toString(port); };

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

	static string getCounts() {
		char buf[128];
		return string(buf, sprintf(buf, "%ld/%ld/%ld", counts.normal, counts.registered, counts.op));
	}

	const string& getIp() {	return socket->getIp().empty() ? server : socket->getIp(); };
	const short getPort() { return port; };
	string getIpWithPort() { return port == 411 ? getIp() : getIp() + ':' + Util::toString(port); };
	
	string getLocalIp() { 
		if(!SETTING(SERVER).empty()) {
			return Socket::resolve(SETTING(SERVER));
		}
		if(socket == NULL)
			return Util::getLocalIp();
		string tmp = socket->getLocalIp();
		if(tmp.empty())
			return Util::getLocalIp();
		return tmp;
	}

	const string& getDescription() const { return description.empty() ? SETTING(DESCRIPTION) : description; };
	void setDescription(const string& aDesc) { description = aDesc; };

	GETSETREF(string, nick, Nick);
	GETSETREF(string, defpassword, Password);
	GETSET(int, supportFlags, SupportFlags);
	GETSET(bool, userInfo, UserInfo);
	GETSET(bool, op, Op);
	GETSET(bool, registered, Registered);
	GETSET(bool, firstHello, FirstHello);
private:
	enum States {
		STATE_CONNECT,
		STATE_LOCK,
		STATE_HELLO,
		STATE_CONNECTED
	} state;

	enum {
		COUNT_UNCOUNTED,
		COUNT_NORMAL,
		COUNT_REGISTERED,
		COUNT_OP
	};

	string server;
	short port;

	string name;
	string description;

	BufferedSocket* socket;
	u_int32_t lastActivity;

	CriticalSection cs;

	User::NickMap users;

	struct Counts {
		Counts(long n = 0, long r = 0, long o = 0) : normal(n), registered(r), op(o) { };
		long normal;
		long registered;
		long op;
		bool operator !=(const Counts& rhs) { return normal != rhs.normal || registered != rhs.registered || op != rhs.op; };
	};

	static Counts counts;

	Counts lastCounts;

	int countType;
	bool reconnect;
	u_int32_t lastUpdate;
	
	typedef list<pair<string, u_int32_t> > FloodMap;
	typedef FloodMap::iterator FloodIter;
	FloodMap seekers;
	FloodMap flooders;

	void updateCounts(bool aRemove);

	Client() : nick(SETTING(NICK)), userInfo(true), supportFlags(0), op(false), 
		registered(false), firstHello(true), state(STATE_CONNECT), 
		socket(BufferedSocket::getSocket('|')), lastActivity(GET_TICK()), 
		countType(COUNT_UNCOUNTED), reconnect(true), lastUpdate(0) {
		TimerManager::getInstance()->addListener(this);
		socket->addListener(this);
	
	};
	
	virtual ~Client() throw();
	void connect();

	void clearUsers();
	void onLine(const string& aLine) throw();
	
	// TimerManagerListener
	virtual void onAction(TimerManagerListener::Types type, u_int32_t aTick) throw();

	// BufferedSocketListener
	virtual void onAction(BufferedSocketListener::Types type, const string& aLine) throw();
	virtual void onAction(BufferedSocketListener::Types type) throw();

};

#endif // !defined(AFX_Client_H__089CBD05_4833_4E30_9A57_BB636231D78E__INCLUDED_)

/**
 * @file
 * $Id: Client.h,v 1.70 2003/10/28 15:27:53 arnetheduck Exp $
 */


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
	template<int I>	struct X { enum { TYPE = I };  };
	
	typedef X<0> Connecting;
	typedef X<1> Connected;
	typedef X<2> BadPassword;
	typedef X<3> MyInfo;
	typedef X<4> NickList;
	typedef X<5> OpList;
	typedef X<6> Redirect;
	typedef X<7> Failed;
	typedef X<8> GetPassword;
	typedef X<9> HubName;
	typedef X<11> Message;
	typedef X<12> PrivateMessage;
	typedef X<13> UserCommand;
	typedef X<14> HubFull;
	typedef X<15> NickTaken;
	typedef X<16> SearchFlood;
	typedef X<17> ConnectToMe;
	typedef X<18> Hello;
	typedef X<19> Supports;
	typedef X<20> CLock;
	typedef X<21> LoggedIn;
	typedef X<22> UserIp;
	typedef X<23> RevConnectToMe;
	typedef X<24> Search;
	typedef X<25> Unknown;
	typedef X<26> ValidateDenied;
	typedef X<26> Quit;

	virtual void on(Connecting, NmdcHub*) throw() { }
	virtual void on(Connected, NmdcHub*) throw() { }
	virtual void on(BadPassword, NmdcHub*) throw() { }
	virtual void on(MyInfo, NmdcHub*, const User::Ptr&) throw() { }
	virtual void on(NickList, NmdcHub*, const User::List&) throw() { }
	virtual void on(OpList, NmdcHub*, const User::List&) throw() { }
	virtual void on(Quit, NmdcHub*, const User::Ptr&) throw() { }
	virtual void on(Redirect, NmdcHub*, const string&) throw() { }
	virtual void on(Failed, NmdcHub*, const string&) throw() { }
	virtual void on(GetPassword, NmdcHub*) throw() { }
	virtual void on(HubName, NmdcHub*) throw() { }
	virtual void on(Message, NmdcHub*, const string&) throw() { }
	virtual void on(PrivateMessage, NmdcHub*, const User::Ptr&, const string&) throw() { }
	virtual void on(UserCommand, NmdcHub*, int, int, const string&, const string&) throw() { }
	virtual void on(HubFull, NmdcHub*) throw() { }
	virtual void on(NickTaken, NmdcHub*) throw() { }
	virtual void on(SearchFlood, NmdcHub*, const string&) throw() { }
	virtual void on(ValidateDenied, NmdcHub*) throw() { }
	virtual void on(Search, NmdcHub*, const string&, int, int64_t, int, const string&) throw() { }
	virtual void on(ConnectToMe, NmdcHub*, const string&, short) throw() { }
	virtual void on(RevConnectToMe, NmdcHub*, const User::Ptr&) throw() { }
	virtual void on(Supports, NmdcHub*, const StringList&) throw() { }
	virtual void on(CLock, NmdcHub*, const string&, const string&) throw() { }
	virtual void on(UserIp, NmdcHub*, const User::List&) throw() { }
	virtual void on(LoggedIn, NmdcHub*) throw() { }
	virtual void on(Hello, NmdcHub*, const User::Ptr&) throw() { }
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
	void version() { send("$Version 1,0091|"); };
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
		privateMessage(aUser->getNick(), string("<") + getNick() + "> " + aMessage);
	}
	void privateMessage(const User* aUser, const string& aMessage) {
		privateMessage(aUser->getNick(), string("<") + getNick() + "> " + aMessage);
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

	virtual void ban(const User*, const string&, time_t) {
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

	virtual string escape(string const& str) const { return Util::validateMessage(str, false); };

	GETSET(int, supportFlags, SupportFlags);
private:

	struct ClientAdapter : public NmdcHubListener {
		ClientAdapter(NmdcHub* aClient) : c(aClient) { aClient->Speaker<NmdcHubListener>::addListener(this); }
		Client* c;
		virtual void on(Connecting, NmdcHub*) throw() { c->fire(ClientListener::Connecting(), c); }
		virtual void on(Connected, NmdcHub*) throw() { c->fire(ClientListener::Connected(), c); }
		virtual void on(BadPassword, NmdcHub*) throw() { c->fire(ClientListener::BadPassword(), c); }
		virtual void on(MyInfo, NmdcHub*, const User::Ptr& u) throw() { c->fire(ClientListener::UserUpdated(), c, u); }
		virtual void on(NickList, NmdcHub*, const User::List& l) throw() { c->fire(ClientListener::UsersUpdated(), c, l); }
		virtual void on(OpList, NmdcHub*, const User::List& l) throw() { c->fire(ClientListener::UsersUpdated(), c, l); }
		virtual void on(Quit, NmdcHub*, const User::Ptr& u) throw() { c->fire(ClientListener::UserRemoved(), c, u); }
		virtual void on(Redirect, NmdcHub*, const string& aLine) throw() { c->fire(ClientListener::Redirect(), c, aLine); }
		virtual void on(Failed, NmdcHub*, const string& aLine) throw() { c->fire(ClientListener::Failed(), c, aLine); }
		virtual void on(GetPassword, NmdcHub*) throw() { c->fire(ClientListener::GetPassword(), c); }
		virtual void on(HubName, NmdcHub*) throw() { c->fire(ClientListener::HubUpdated(), c); }
		virtual void on(Message, NmdcHub*, const string& aLine) throw() { c->fire(ClientListener::Message(), c, aLine); }
		virtual void on(PrivateMessage, NmdcHub*, const User::Ptr& u, const string& aLine) throw() { c->fire(ClientListener::PrivateMessage(), c, u, aLine); }
		virtual void on(UserCommand, NmdcHub*, int a, int b, const string& l1, const string& l2) throw() { c->fire(ClientListener::UserCommand(), c, a, b, l1, l2); }
		virtual void on(HubFull, NmdcHub*) throw() { c->fire(ClientListener::HubFull(), c); }
		virtual void on(NickTaken, NmdcHub*) throw() { c->fire(ClientListener::NickTaken(), c); }
		virtual void on(SearchFlood, NmdcHub*, const string& aLine) throw() { c->fire(ClientListener::SearchFlood(), c, aLine); }
		virtual void on(ValidateDenied, NmdcHub*) throw() { c->fire(ClientListener::NickTaken(), c); }
		virtual void on(Hello, NmdcHub*, const User::Ptr& u) throw() { c->fire(ClientListener::UserUpdated(), c, u); }
		virtual void on(Search, NmdcHub*, const string& a, int b, int64_t d, int e, const string& f) throw() { c->fire(ClientListener::NmdcSearch(), c, a, b, d, e, f); }
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
	virtual void on(TimerManagerListener::Second, u_int32_t aTick) throw();

	virtual void on(Connecting) throw() { Speaker<NmdcHubListener>::fire(NmdcHubListener::Connecting(), this); }
	virtual void on(Connected) throw() { lastActivity = GET_TICK(); Speaker<NmdcHubListener>::fire(NmdcHubListener::Connected(), this); }
	virtual void on(Line, const string& l) throw() { onLine(l); }
	virtual void on(Failed, const string&) throw();

};

#endif // !defined(AFX_NmdcHub_H__089CBD05_4833_4E30_9A57_BB636231D78E__INCLUDED_)

/**
 * @file
 * $Id: NmdcHub.h,v 1.7 2004/09/07 01:36:52 arnetheduck Exp $
 */


/* 
 * Copyright (C) 2001-2005 Jacek Sieka, arnetheduck on gmail point com
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
#include "Text.h"

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
	typedef X<27> Quit;

	virtual void on(Connecting, NmdcHub*) throw() { }
	virtual void on(Connected, NmdcHub*) throw() { }
	virtual void on(BadPassword, NmdcHub*) throw() { }
	virtual void on(MyInfo, NmdcHub*, const OnlineUser&) throw() { }
	virtual void on(NickList, NmdcHub*, const OnlineUser::List&) throw() { }
	virtual void on(OpList, NmdcHub*, const OnlineUser::List&) throw() { }
	virtual void on(Quit, NmdcHub*, const OnlineUser&) throw() { }
	virtual void on(Redirect, NmdcHub*, const string&) throw() { }
	virtual void on(Failed, NmdcHub*, const string&) throw() { }
	virtual void on(GetPassword, NmdcHub*) throw() { }
	virtual void on(HubName, NmdcHub*) throw() { }
	virtual void on(Message, NmdcHub*, const string&) throw() { }
	virtual void on(PrivateMessage, NmdcHub*, const OnlineUser&, const string&) throw() { }
	virtual void on(UserCommand, NmdcHub*, int, int, const string&, const string&) throw() { }
	virtual void on(HubFull, NmdcHub*) throw() { }
	virtual void on(NickTaken, NmdcHub*) throw() { }
	virtual void on(SearchFlood, NmdcHub*, const string&) throw() { }
	virtual void on(ValidateDenied, NmdcHub*) throw() { }
	virtual void on(Search, NmdcHub*, const string&, int, int64_t, int, const string&) throw() { }
	virtual void on(ConnectToMe, NmdcHub*, const string&, short) throw() { }
	virtual void on(RevConnectToMe, NmdcHub*, const OnlineUser&) throw() { }
	virtual void on(Supports, NmdcHub*, const StringList&) throw() { }
	virtual void on(CLock, NmdcHub*, const string&, const string&) throw() { }
	virtual void on(UserIp, NmdcHub*, const OnlineUser::List&) throw() { }
	virtual void on(LoggedIn, NmdcHub*) throw() { }
	virtual void on(Hello, NmdcHub*, const OnlineUser&) throw() { }
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
		SUPPORTS_USERIP2 = 0x04
	};

#define checkstate() if(state != STATE_CONNECTED) return

	virtual void connect(const OnlineUser& aUser);
	virtual void hubMessage(const string& aMessage) { checkstate(); send(toNmdc( "<" + getNick() + "> " + Util::validateMessage(aMessage, false) + "|" ) ); }
	virtual void privateMessage(const OnlineUser& aUser, const string& aMessage) { privateMessage(aUser.getIdentity().getNick(), string("<") + getNick() + "> " + aMessage); }
	virtual void send(const string& a) throw() {
		lastActivity = GET_TICK();
		//dcdebug("Sending %d to %s: %.40s\n", a.size(), getName().c_str(), a.c_str());
		socket->write(a);
	}
	virtual void sendUserCmd(const string& aUserCmd) throw() {
		send(toNmdc(aUserCmd));
	}
	virtual void search(int aSizeType, int64_t aSize, int aFileType, const string& aString, const string& aToken);
	virtual void password(const string& aPass) { send("$MyPass " + toNmdc(aPass) + "|"); }
	virtual void info(bool alwaysSend) { myInfo(alwaysSend); }
	
	virtual size_t getUserCount() const {  Lock l(cs); return users.size(); }
	virtual int64_t getAvailable() const;
	virtual const string& getName() const { return name; };
	virtual bool getOp() const { return getMe() ? getMe()->isSet(User::OP) : false; };

	virtual string escape(string const& str) const { return Util::validateMessage(str, false); };

	void disconnect() throw();
	void myInfo(bool alwaysSend);
	
	void refreshUserList(bool unknownOnly = false);

	void validateNick(const string& aNick) { send("$ValidateNick " + toNmdc(aNick) + "|"); }
	void key(const string& aKey) { send("$Key " + aKey + "|"); };	
	void version() { send("$Version 1,0091|"); };
	void getNickList() { checkstate(); send("$GetNickList|"); };
	void getInfo(const OnlineUser& aUser) { checkstate(); send("$GetINFO " + toNmdc(aUser.getIdentity().getNick()) + " " + toNmdc(getNick()) + "|"); };

	void connectToMe(const OnlineUser& aUser) {
		checkstate(); 
		dcdebug("NmdcHub::connectToMe %s\n", aUser.getIdentity().getNick().c_str());
		send("$ConnectToMe " + toNmdc(aUser.getIdentity().getNick()) + " " + getLocalIp() + ":" + Util::toString(SETTING(IN_PORT)) + "|");
	}

	void privateMessage(const string& aNick, const string& aMessage) {
		checkstate(); 
		send("$To: " + toNmdc(aNick) + " From: " + toNmdc(getNick()) + " $" + toNmdc(Util::validateMessage(aMessage, false)) + "|");
	}
	void supports(const StringList& feat) { 
		string x;
		for(StringList::const_iterator i = feat.begin(); i != feat.end(); ++i) {
			x+= *i + ' ';
		}
		send("$Supports " + x + '|');
	}
	void revConnectToMe(const OnlineUser& aUser) {
		checkstate(); 
		dcdebug("NmdcHub::revConnectToMe %s\n", aUser.getIdentity().getNick().c_str());
		send("$RevConnectToMe " + toNmdc(getNick()) + " " + toNmdc(aUser.getIdentity().getNick()) + "|");
	}

	void send(const char* aBuf, int aLen) throw() {
		lastActivity = GET_TICK();
		socket->write(aBuf, aLen);
	}

	GETSET(int, supportFlags, SupportFlags);
private:

	struct ClientAdapter : public NmdcHubListener {
		ClientAdapter(NmdcHub* aClient) : c(aClient) { aClient->Speaker<NmdcHubListener>::addListener(this); }
		Client* c;
		virtual void on(Connecting, NmdcHub*) throw() { c->fire(ClientListener::Connecting(), c); }
		virtual void on(Connected, NmdcHub*) throw() { c->fire(ClientListener::Connected(), c); }
		virtual void on(BadPassword, NmdcHub*) throw() { c->fire(ClientListener::BadPassword(), c); }
		virtual void on(MyInfo, NmdcHub*, const OnlineUser& u) throw() { c->fire(ClientListener::UserUpdated(), c, u); }
		virtual void on(NickList, NmdcHub*, const OnlineUser::List& l) throw() { c->fire(ClientListener::UsersUpdated(), c, l); }
		virtual void on(OpList, NmdcHub*, const OnlineUser::List& l) throw() { c->fire(ClientListener::UsersUpdated(), c, l); }
		virtual void on(Quit, NmdcHub*, const OnlineUser& u) throw() { c->fire(ClientListener::UserRemoved(), c, u); }
		virtual void on(Redirect, NmdcHub*, const string& aLine) throw() { c->fire(ClientListener::Redirect(), c, aLine); }
		virtual void on(Failed, NmdcHub*, const string& aLine) throw() { c->fire(ClientListener::Failed(), c, aLine); }
		virtual void on(GetPassword, NmdcHub*) throw() { c->fire(ClientListener::GetPassword(), c); }
		virtual void on(HubName, NmdcHub*) throw() { c->fire(ClientListener::HubUpdated(), c); }
		virtual void on(Message, NmdcHub*, const string& aLine) throw() { c->fire(ClientListener::Message(), c, aLine); }
		virtual void on(PrivateMessage, NmdcHub*, const OnlineUser& u, const string& aLine) throw() { c->fire(ClientListener::PrivateMessage(), c, u, aLine); }
		virtual void on(UserCommand, NmdcHub*, int a, int b, const string& l1, const string& l2) throw() { c->fire(ClientListener::UserCommand(), c, a, b, l1, l2); }
		virtual void on(HubFull, NmdcHub*) throw() { c->fire(ClientListener::HubFull(), c); }
		virtual void on(NickTaken, NmdcHub*) throw() { c->fire(ClientListener::NickTaken(), c); }
		virtual void on(SearchFlood, NmdcHub*, const string& aLine) throw() { c->fire(ClientListener::SearchFlood(), c, aLine); }
		virtual void on(ValidateDenied, NmdcHub*) throw() { c->fire(ClientListener::NickTaken(), c); }
		virtual void on(Hello, NmdcHub*, const OnlineUser& u) throw() { c->fire(ClientListener::UserUpdated(), c, u); }
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

	typedef HASH_MAP_X(string, OnlineUser*, noCaseStringHash, noCaseStringEq, noCaseStringLess) NickMap;
	typedef NickMap::iterator NickIter;

	NickMap users;

	bool reconnect;
	u_int32_t lastUpdate;
	string lastMyInfoA, lastMyInfoB;

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

	OnlineUser& getUser(const string& aNick);
	OnlineUser* findUser(const string& aNick);
	void putUser(const string& aNick);

	string fromNmdc(const string& str) const { return Text::acpToUtf8(str); }
	string toNmdc(const string& str) const { return Text::utf8ToAcp(str); }

	virtual string checkNick(const string& aNick);
	virtual string getHubURL();

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
 * $Id: NmdcHub.h,v 1.23 2005/04/12 23:24:13 arnetheduck Exp $
 */


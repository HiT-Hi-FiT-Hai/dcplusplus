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

#ifndef _CLIENT_H
#define _CLIENT_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "User.h"
#include "BufferedSocket.h"
#include "SettingsManager.h"

class Client;

class ClientListener  
{
public:
	template<int I>	struct X { enum { TYPE = I };  };

	typedef X<0> Connecting;
	typedef X<1> Connected;
	typedef X<2> BadPassword;
	typedef X<3> UserUpdated;
	typedef X<4> UsersUpdated;
	typedef X<5> UserRemoved;
	typedef X<6> Redirect;
	typedef X<7> Failed;
	typedef X<8> GetPassword;
	typedef X<9> HubUpdated;
	typedef X<11> Message;
	typedef X<12> PrivateMessage;
	typedef X<13> UserCommand;
	typedef X<14> HubFull;
	typedef X<15> NickTaken;
	typedef X<16> SearchFlood;
	typedef X<17> NmdcSearch;

	virtual void on(Connecting, Client*) throw() { }
	virtual void on(Connected, Client*) throw() { }
	virtual void on(BadPassword, Client*) throw() { }
	virtual void on(UserUpdated, Client*, const User::Ptr&) throw() { }
	virtual void on(UsersUpdated, Client*, const User::List&) throw() { }
	virtual void on(UserRemoved, Client*, const User::Ptr&) throw() { }
	virtual void on(Redirect, Client*, const string&) throw() { }
	virtual void on(Failed, Client*, const string&) throw() { }
	virtual void on(GetPassword, Client*) throw() { }
	virtual void on(HubUpdated, Client*) throw() { }
	virtual void on(Message, Client*, const string&) throw() { }
	virtual void on(PrivateMessage, Client*, const User::Ptr&, const string&) throw() { }
	virtual void on(UserCommand, Client*, int, int, const string&, const string&) throw() { }
	virtual void on(HubFull, Client*) throw() { }
	virtual void on(NickTaken, Client*) throw() { }
	virtual void on(SearchFlood, Client*, const string&) throw() { }
	virtual void on(NmdcSearch, Client*, const string&, int, int64_t, int, const string&) throw() { }
};

/** Yes, this should probably be called a Hub */
class Client : public Speaker<ClientListener>, public BufferedSocketListener {
public:
	typedef Client* Ptr;
	typedef list<Ptr> List;
	typedef List::iterator Iter;

	Client(const string& hubURL, char separator, bool usesEscapes = false);
	virtual ~Client();

	virtual void connect(const User* user) = 0;
	virtual void hubMessage(const string& aMessage) = 0;
	virtual void privateMessage(const User* user, const string& aMessage) = 0;
	virtual void send(const string& aMessage) = 0;
	virtual void sendUserCmd(const string& aUserCmd) = 0;
	virtual void search(int aSizeMode, int64_t aSize, int aFileType, const string& aString) = 0;
	virtual void password(const string& pwd) = 0;
	virtual void info(bool alwaysSend) = 0;
    
	virtual size_t getUserCount() const = 0;
	virtual int64_t getAvailable() const = 0;
	virtual const string& getName() const = 0;
	virtual bool getOp() const = 0;

	virtual User::NickMap& lockUserList() = 0;
	virtual void unlockUserList() = 0;

	const string& getAddress() const { return address; }
	const string& getAddressPort() const { return addressPort; }
	short getPort() const { return port; }

	const string& getIp() const {	return socket->getIp().empty() ? getAddress() : socket->getIp(); };
	string getIpPort() const { return port == 411 ? getIp() : getIp() + ':' + Util::toString(port); };
	string getLocalIp() const;

	virtual void connect();
	bool isConnected() const { return socket->isConnected(); }
	void disconnect() { socket->disconnect(); }

	void updated(User::Ptr& aUser) { 
		fire(ClientListener::UserUpdated(), this, aUser);
	}

	static string getCounts() {
		char buf[128];
		return string(buf, sprintf(buf, "%ld/%ld/%ld", counts.normal, counts.registered, counts.op));
	}

	const User::Ptr& getMe() const { return me; };
	User::Ptr& getMe() { return me; }
	void setMe(const User::Ptr& aMe) { me = aMe; }

	const string& getDescription() const { return description.empty() ? SETTING(DESCRIPTION) : description; };
	void setDescription(const string& aDesc) { description = aDesc; };

	void scheduleDestruction() const { socket->shutdown(); }

	virtual string escape(string const& str) const { return str; };
	StringMap& escapeParams(StringMap& sm) {
		for(StringMapIter i = sm.begin(); i != sm.end(); ++i) {
			i->second = escape(i->second);
		}
		return sm;
	}

protected:
	struct Counts {
		Counts(long n = 0, long r = 0, long o = 0) : normal(n), registered(r), op(o) { };
		volatile long normal;
		volatile long registered;
		volatile long op;
		bool operator !=(const Counts& rhs) { return normal != rhs.normal || registered != rhs.registered || op != rhs.op; };
	};

	BufferedSocket* socket;

	User::Ptr me;
	static Counts counts;
	Counts lastCounts;

	void updateCounts(bool aRemove);

	void setPort(short aPort) { port = aPort; }

	// reload nick from settings, other details from hubmanager
	void reloadSettings();

	virtual string checkNick(const string& nick) = 0;
	virtual string getHubURL() = 0;

	GETSET(string, nick, Nick);
	GETSET(string, defpassword, Password);
	GETSET(bool, registered, Registered);
	GETSET(u_int32_t, reconnDelay, ReconnDelay);
private:

	enum CountType {
		COUNT_UNCOUNTED,
		COUNT_NORMAL,
		COUNT_REGISTERED,
		COUNT_OP
	};

	Client(const Client&);
	Client& operator=(const Client&);

	string description;

	string address;
	string addressPort;
	u_int16_t port;

	CountType countType;

	// BufferedSocketListener
	virtual void on(BufferedSocketListener::Shutdown) throw() {
		removeListeners();
		delete this;
	}
};

#endif // _CLIENT_H
/**
 * @file
 * $Id: Client.h,v 1.96 2005/01/05 19:30:28 arnetheduck Exp $
 */

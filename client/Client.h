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
	typedef ClientListener* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;

	enum Types {
		CONNECTING,
		CONNECTED,
		BAD_PASSWORD,
		USER_UPDATED,
		USERS_UPDATED,
		USER_REMOVED,
		FAILED,
		FORCE_MOVE,
		GET_PASSWORD,
		HUB_NAME,
		HUB_FULL,
		MESSAGE,
		PRIVATE_MESSAGE,
		USER_COMMAND,
		SEARCH,
		NICK_TAKEN,
		SEARCH_FLOOD,

	};

	virtual void onAction(Types, Client*) throw() { };						// BAD_PASSWORD,CONNECTING,CONNECTED,GET_PASSWORD,HUB_FULL,NICK_TAKEN
	virtual void onAction(Types, Client*, const User::Ptr&) throw() { };	// USER_UPDATED
	virtual void onAction(Types, Client*, const string&) throw() { };		// FAILED, FORCE_MOVE, HUB_NAME
	virtual void onAction(Types, Client*, const User::Ptr&, const string&) throw() { };	// PRIVATE_MESSAGE
	virtual void onAction(Types, Client*, const string&, int, const string&, int, const string&) throw() { };	// SEARCH
	virtual void onAction(Types, Client*, const User::List&) throw() { };		// USER_IP,OP_LIST
	virtual void onAction(Types, Client*, int, int, const string&, const string&) throw() { }; // USER_COMMAND
};

/** Yes, this should probably be called a Hub */
class Client : public Speaker<ClientListener>, public BufferedSocketListener {
public:
	typedef Client* Ptr;
	typedef list<Ptr> List;
	typedef List::iterator Iter;

	Client(const string& hubURL, char separator);
	virtual ~Client();

	virtual void connect(const User* user) = 0;
	virtual void hubMessage(const string& aMessage) = 0;
	virtual void privateMessage(const User* user, const string& aMessage) = 0;
	virtual void kick(const User* user, const string& aMessage) = 0;
	virtual void ban(const User* user, const string& aMessage, time_t seconds) = 0;
	virtual void send(const string& aMessage) = 0;
	virtual void redirect(const User* user, const string& aHub, const string& aMessage) = 0;
	virtual void search(int aSizeMode, int64_t aSize, int aFileType, const string& aString) = 0;
	virtual void password(const string& pwd) = 0;
	virtual void info() = 0;
    
	virtual int getUserCount() const = 0;
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

	virtual void connect() { socket->connect(address, port); }
	bool isConnected() const { return socket->isConnected(); }
	void disconnect() { socket->disconnect(); }

	void updated(User::Ptr& aUser) { 
		fire(ClientListener::USER_UPDATED, this, aUser);
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

	GETSETREF(string, nick, Nick);
	GETSETREF(string, defpassword, Password);
	GETSET(bool, registered, Registered);
protected:
	struct Counts {
		Counts(long n = 0, long r = 0, long o = 0) : normal(n), registered(r), op(o) { };
		long normal;
		long registered;
		long op;
		bool operator !=(const Counts& rhs) { return normal != rhs.normal || registered != rhs.registered || op != rhs.op; };
	};

	BufferedSocket* socket;

	User::Ptr me;
	static Counts counts;
	Counts lastCounts;

	void updateCounts(bool aRemove);

	void setPort(short aPort) { port = aPort; }
private:

	enum {
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
	short port;

	int countType;

};

#endif // _CLIENT_H
/**
 * @file
 * $Id: Client.h,v 1.78 2004/04/08 18:17:59 arnetheduck Exp $
 */

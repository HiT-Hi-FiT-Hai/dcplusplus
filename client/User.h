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

#ifndef USER_H
#define USER_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Util.h"
#include "Pointer.h"
#include "CID.h"
#include "FastAlloc.h"

/** A user connected to one or more hubs. */
class User : public FastAlloc<User>, public PointerBase, public Flags
{
public:
	enum Bits {
		OP_BIT,
		ONLINE_BIT,
		DCPLUSPLUS_BIT,
		PASSIVE_BIT,
		NMDC_BIT,
		BOT_BIT,
		HUB_BIT
	};

	/** Each flag is set if it's true in at least one hub */
	enum UserFlags {
		OP = 1<<OP_BIT,
		ONLINE = 1<<ONLINE_BIT,
		DCPLUSPLUS = 1<<DCPLUSPLUS_BIT,
		PASSIVE = 1<<PASSIVE_BIT,
		NMDC = 1<<NMDC_BIT,
		BOT = 1<<BOT_BIT,
		HUB = 1<<HUB_BIT,

	};

	typedef Pointer<User> Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;

	struct HashFunction {
		static const size_t bucket_size = 4;
		static const size_t min_buckets = 8;
		size_t operator()(const Ptr& x) const { return ((size_t)(&(*x)))/sizeof(User); }
		bool operator()(const Ptr& a, const Ptr& b) const { return (&(*a)) < (&(*b)); }
	};

	User(const string& nick) : firstNick(nick), Flags(NMDC) { }
	User(const CID& aCID) : cid(aCID) { }

	virtual ~User() throw() { };

	operator CID() { return cid; }

	bool isOnline() const { return isSet(ONLINE); }
	bool isNMDC() const { return isSet(NMDC); }

	GETSET(CID, cid, CID);
	GETSET(string, firstNick, FirstNick);
private:
	User(const User&);
	User& operator=(const User&);
};

/** One of possibly many identities of a user, mainly for UI purposes */
class Identity : public Flags {
public:
	enum {
		GOT_INF_BIT
	};
	enum Flags {
		GOT_INF
	};

	Identity() { }
	Identity(const User::Ptr& ptr) : user(ptr) { }
	Identity(const Identity& rhs) : user(rhs.user), hubURL(rhs.hubURL), info(rhs.info) { }
	Identity& operator=(const Identity& rhs) { user = rhs.user; hubURL = rhs.hubURL; info = rhs.info; return *this; }

#define GS(n, x) const string& get##n() const { return get(x); } void set##n(const string& v) { set(x, v); }
	GS(Nick, "NI")
	GS(Description, "DE")
	GS(Ip, "I4")
	GS(UdpPort, "U4")
	GS(Email, "EM")

	void setBytesShared(const string& bs) { set("SS", bs); }
	int64_t getBytesShared() const { return Util::toInt64(get("SS")); }
	
	void setOp(bool op) { set("OP", op ? "1" : Util::emptyString); }

	string getTag() const { 
		if(get("VE").empty() || get("HN").empty() || get("HR").empty() ||get("HO").empty() || get("SL").empty())
			return Util::emptyString;
		return "<" + get("VE") + ",M:" + string(isTcpActive() ? "A" : "P") + ",H:" + get("HN") + "/" + 
			get("HR") + "/" + get("HO") + ",S:" + get("SL") + ">";
	}

	const bool isHub() const { return !get("HU").empty(); }
	const bool isOp() const { return !get("OP").empty(); }
	const bool isHidden() const { return !get("HI").empty(); }
	const bool isTcpActive() const { return !getIp().empty(); }
	const bool isUdpActive() const { return !getIp().empty() && !getUdpPort().empty(); }

	const string& get(const char* name) const {
		InfMap::const_iterator i = info.find(*(short*)name);
		return i == info.end() ? Util::emptyString : i->second;
	}

	void set(const char* name, const string& val) {
		if(val.empty())
			info.erase(*(short*)name);
		else
			info[*(short*)name] = val;
	}
	
	GETSET(User::Ptr, user, User);
	GETSET(string, hubURL, HubURL);
private:
	typedef map<short, string> InfMap;
	typedef InfMap::iterator InfIter;

	InfMap info;
};

class Client;
class NmdcHub;

class OnlineUser : public FastAlloc<OnlineUser> {
public:
	typedef vector<OnlineUser*> List;
	typedef List::iterator Iter;

	OnlineUser() : client(NULL) { }
	OnlineUser(const User::Ptr& ptr, Client& client_) : user(ptr), identity(ptr), client(&client_) { }

	operator User::Ptr&() { return user; }
	operator const User::Ptr&() const { return user; }

	User::Ptr& getUser() { return user; }
	Identity& getIdentity() { return identity; }
	Client& getClient() { return *client; }
	const Client& getClient() const { return *client; }

	GETSET(User::Ptr, user, User);
	GETSET(Identity, identity, Identity);

private:
	friend class NmdcHub;

	OnlineUser(const OnlineUser&);
	OnlineUser& operator=(const OnlineUser&);

	Client* client;
};

#endif // USER_H

/**
 * @file
 * $Id: User.h,v 1.56 2005/04/23 15:45:32 arnetheduck Exp $
 */

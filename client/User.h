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

#if !defined(AFX_USER_H__26AA222C_500B_4AD2_A5AA_A594E1A6D639__INCLUDED_)
#define AFX_USER_H__26AA222C_500B_4AD2_A5AA_A594E1A6D639__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Util.h"
#include "Pointer.h"
#include "CriticalSection.h"
#include "CID.h"

class Client;
class FavoriteUser;

/**
 * A user connected to a hubs.
 */
class User : public PointerBase, public Flags
{
public:
	enum {
		OP_BIT,
		ONLINE_BIT,
		DCPLUSPLUS_BIT,
		PASSIVE_BIT,
		QUIT_HUB_BIT,
		HIDDEN_BIT,
		HUB_BIT,
		BOT_BIT
	};

	enum {
		OP = 1<<OP_BIT,
		ONLINE = 1<<ONLINE_BIT,
		DCPLUSPLUS = 1<<DCPLUSPLUS_BIT,
		PASSIVE = 1<<PASSIVE_BIT,
		QUIT_HUB = 1<<QUIT_HUB_BIT,
		HIDDEN = 1<<HIDDEN_BIT,
		HUB = 1<<HUB_BIT,
		BOT = 1<<BOT_BIT,
	};

	typedef Pointer<User> Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;
	typedef HASH_MAP_X(string,Ptr,noCaseStringHash,noCaseStringEq,noCaseStringLess) NickMap;
	typedef NickMap::iterator NickIter;
	typedef HASH_MAP_X(CID, Ptr, CID::Hash, equal_to<CID>, less<CID>) CIDMap;
	typedef CIDMap::iterator CIDIter;

	struct HashFunction {
		static const size_t bucket_size = 4;
		static const size_t min_buckets = 8;
		size_t operator()(const Ptr& x) const { return ((size_t)(&(*x)))/sizeof(User); };
		bool operator()(const Ptr& a, const Ptr& b) const { return (&(*a)) < (&(*b)); };
	};

	User(const CID& aCID) : cid(aCID), bytesShared(0), client(NULL), favoriteUser(NULL) { }
	User(const string& aNick) throw() : nick(aNick), bytesShared(0), client(NULL), favoriteUser(NULL) { }

	virtual ~User() throw();

	void setClient(Client* aClient);
	void connect();
	const string& getClientNick() const;
	const CID getClientCID() const;
	const string& getClientName() const;
	string getClientAddressPort() const;
	void privateMessage(const string& aMsg);
	void clientMessage(const string& aMsg);
	void kick(const string& aMsg);
	void redirect(const string& aTarget, const string& aReason);
	bool isClientOp() const;
	void send(const string& msg);

	string getFullNick() const { 
		string tmp(getNick());
		tmp += " (";
		tmp += getClientName();
		tmp += ")";
		return tmp;
	}
	
	void setBytesShared(const string& aSharing) { setBytesShared(Util::toInt64(aSharing)); };

	bool isOnline() const { return isSet(ONLINE); };
	bool isClient(Client* aClient) const { return client == aClient; };
	
	void getParams(StringMap& ucParams);

	// favorite user stuff
	void setFavoriteUser(FavoriteUser* aUser);
	bool isFavoriteUser() const;
	bool getFavoriteGrantSlot() const;
	void setFavoriteGrantSlot(bool grant);
	void setFavoriteLastSeen(u_int32_t anOfflineTime = 0);
	u_int32_t getFavoriteLastSeen() const;
	const string& getUserDescription() const;
	void setUserDescription(const string& aDescription);

	static void updated(User::Ptr& aUser);

	StringMap& clientEscapeParams(StringMap& sm) const;
	
	GETSET(string, connection, Connection);
	GETSET(string, nick, Nick);
	GETSET(string, email, Email);
	GETSET(string, description, Description);
	GETSET(string, tag, Tag);
	GETSET(string, lastHubAddress, LastHubAddress);
	GETSET(string, lastHubName, LastHubName);
	GETSET(string, ip, Ip);
	GETSET(CID, cid, CID);
	GETSET(int64_t, bytesShared, BytesShared);
private:
	mutable RWLock cs;

	User(const User&);
	User& operator=(const User&);

	Client* client;
	FavoriteUser* favoriteUser;
};

#endif // !defined(AFX_USER_H__26AA222C_500B_4AD2_A5AA_A594E1A6D639__INCLUDED_)

/**
 * @file
 * $Id: User.h,v 1.45 2004/09/07 01:36:52 arnetheduck Exp $
 */

/* 
 * Copyright (C) 2001 Jacek Sieka, jacek@creatio.se
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

class Client;

/**
 * A user connected to one or more hubs.
 * @todo Something clever for handling different users with the same nick (on different hubs)
 */
class User
{
public:
	enum {
		FLAG_OP = 0x01
	};
	typedef User* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;
	typedef map<string,Ptr> NickMap;
	typedef NickMap::iterator NickIter;

	Client* getClient() const { return client; };
	void setClient(Client* aClient) { client = aClient; };

	const string& getNick() const { return nick; };
	void setNick(const string& aNick) { nick = aNick; };
	
	const string& getEmail() const { return email; };
	void setEmail(const string& aEmail) { email = aEmail; };

	const string& getDescription() const { return description; };
	void setDescription(const string& aDescription) { description = aDescription; };

	const string& getConnection() const { return connection; };
	void setConnection(const string& aConnection) { connection = aConnection; };
	
	LONGLONG getBytesShared() const { return _atoi64(sharing.c_str()); };
	const string& getBytesSharedString() const { return sharing; };
	void setBytesShared(LONGLONG aSharing) { char buf[24]; sharing = _i64toa(aSharing, buf, 10); };
	void setBytesShared(const string& aSharing) { sharing = aSharing; };

	void setFlag(DWORD aFlag) { flags &= aFlag; };
	void unsetFlag(DWORD aFlag) { flags &= ~aFlag; };
	bool isSet(DWORD aFlag) { return (flags&aFlag) > 0; };

	User() : client(NULL), flags(0) { };
	User(const string& aNick, DWORD aFlags = 0) : client(NULL), nick(aNick), flags(aFlags) { };
	~User() { };

private:
	DWORD flags;
	Client* client;
	string connection;
	string nick;
	string sharing;
	string email;
	string description;
};

#endif // !defined(AFX_USER_H__26AA222C_500B_4AD2_A5AA_A594E1A6D639__INCLUDED_)

/**
 * @file User.cpp
 * $Id: User.h,v 1.2 2001/12/13 19:21:57 arnetheduck Exp $
 * @if LOG
 * $Log: User.h,v $
 * Revision 1.2  2001/12/13 19:21:57  arnetheduck
 * A lot of work done almost everywhere, mainly towards a friendlier UI
 * and less bugs...time to release 0.06...
 *
 * Revision 1.1  2001/12/01 17:17:22  arnetheduck
 * New additions to the reworked connection manager and huffman encoder
 *
 * @endif
 */


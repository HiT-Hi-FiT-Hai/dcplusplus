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

#if !defined(AFX_USER_H__26AA222C_500B_4AD2_A5AA_A594E1A6D639__INCLUDED_)
#define AFX_USER_H__26AA222C_500B_4AD2_A5AA_A594E1A6D639__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Util.h"
#include "Pointer.h"

class Client;
class SocketException;

/**
 * A user connected to a hubs.
 */
class User : public PointerBase, public Flags
{
public:
	enum {
		OP = 0x01,
		ONLINE = 0x02,
		DCPLUSPLUS = 0x04,
		PASSIVE = 0x08
	};
	typedef Pointer<User> Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;
	typedef HASH_MAP<string,Ptr> NickMap;
	typedef NickMap::iterator NickIter;

	static Ptr nuser;

	void setClient(Client* aClient);
	void connect();
	string getClientNick();
	void update();
	string getClientName();
	void privateMessage(const string& aMsg);
	void clientMessage(const string& aMsg);
	void kick(const string& aMsg);
	void redirect(const string& aTarget, const string& aReason);
	bool isClientOp();
	
	LONGLONG getBytesShared() const { return sharingLong; };
	const string& getBytesSharedString() const { return sharing; };
	void setBytesShared(LONGLONG aSharing) { sharing = Util::toString(aSharing); sharingLong = aSharing; };
	void setBytesShared(const string& aSharing) { sharing = aSharing; sharingLong = Util::toInt64(aSharing); };

	bool isOnline() const { return isSet(ONLINE); };
	bool isClient(Client* aClient) const { return client == aClient; };
	
	static void updated(User::Ptr& aUser);
	
	User(const string& aNick) : sharingLong(0), client(NULL), nick(aNick) { };
	~User() { };

	GETSETREF(string, connection, Connection);
	GETSETREF(string, nick, Nick);
	GETSETREF(string, email, Email);
	GETSETREF(string, description, Description);
	GETSETREF(string, lastHubIp, LastHubIp);
	GETSETREF(string, lastHubName, LastHubName)
private:
	CriticalSection cs;
	
	Client* client;
	string sharing;
	LONGLONG sharingLong;		// Cache this...requested very frequently...
	
};

#endif // !defined(AFX_USER_H__26AA222C_500B_4AD2_A5AA_A594E1A6D639__INCLUDED_)

/**
 * @file User.cpp
 * $Id: User.h,v 1.14 2002/03/07 19:07:52 arnetheduck Exp $
 * @if LOG
 * $Log: User.h,v $
 * Revision 1.14  2002/03/07 19:07:52  arnetheduck
 * Minor fixes + started code review
 *
 * Revision 1.13  2002/03/04 23:52:31  arnetheduck
 * Updates and bugfixes, new user handling almost finished...
 *
 * Revision 1.12  2002/02/27 12:02:09  arnetheduck
 * Completely new user handling, wonder how it turns out...
 *
 * Revision 1.11  2002/02/18 23:48:32  arnetheduck
 * New prerelease, bugs fixed and features added...
 *
 * Revision 1.10  2002/02/07 17:25:28  arnetheduck
 * many bugs fixed, time for 0.152 I think
 *
 * Revision 1.9  2002/01/20 22:54:46  arnetheduck
 * Bugfixes to 0.131 mainly...
 *
 * Revision 1.8  2002/01/18 17:41:43  arnetheduck
 * Reworked many right button menus, adding op commands and making more easy to use
 *
 * Revision 1.7  2002/01/17 23:35:59  arnetheduck
 * Reworked threading once more, now it actually seems stable. Also made
 * sure that noone tries to access client objects that have been deleted
 * as well as some other minor updates
 *
 * Revision 1.6  2002/01/10 12:33:14  arnetheduck
 * Various fixes
 *
 * Revision 1.5  2002/01/07 20:17:59  arnetheduck
 * Finally fixed the reconnect bug that's been annoying me for a whole day...
 * Hopefully the app works better in w95 now too...
 *
 * Revision 1.4  2001/12/18 12:32:18  arnetheduck
 * Stability fixes
 *
 * Revision 1.3  2001/12/16 19:47:48  arnetheduck
 * Reworked downloading and user handling some, and changed some small UI things
 *
 * Revision 1.2  2001/12/13 19:21:57  arnetheduck
 * A lot of work done almost everywhere, mainly towards a friendlier UI
 * and less bugs...time to release 0.06...
 *
 * Revision 1.1  2001/12/01 17:17:22  arnetheduck
 * New additions to the reworked connection manager and huffman encoder
 *
 * @endif
 */


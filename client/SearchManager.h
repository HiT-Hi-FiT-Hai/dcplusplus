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

#if !defined(AFX_SEARCHMANAGER_H__E8F009DF_D216_4F8F_8C81_07D2FA0BFB7F__INCLUDED_)
#define AFX_SEARCHMANAGER_H__E8F009DF_D216_4F8F_8C81_07D2FA0BFB7F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BufferedSocket.h"
#include "User.h"

class SearchResult {
public:	
	typedef SearchResult* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;
	
	SearchResult() : size(0), slots(0), freeSlots(0) { };
	SearchResult(const SearchResult& rhs) : user(rhs.user), nick(rhs.nick), file(rhs.file), 
		hubName(rhs.hubName), hubAddress(rhs.hubAddress), size(rhs.size), slots(rhs.slots), freeSlots(rhs.freeSlots) { };

	string getFileName() { 
		string::size_type i;
		if( (i=file.rfind('\\')) != string::npos ) {
			if((i + 1) < file.size()) {
				return file.substr(i + 1);
			}
		}
		return Util::emptyString;
	}

	User::Ptr& getUser() { return user; };
	void setUser(const User::Ptr& aUser) { user = aUser; };
	
	LONGLONG getSize() { return size; };
	void setSize(LONGLONG aSize) { size = aSize; };
	void setSize(const string& aSize) { size = _atoi64(aSize.c_str()); };
	
	int getSlots() { return slots; };
	int getFreeSlots() { return freeSlots; };
	string getSlotString() { char buf[16]; sprintf(buf, "%d/%d", freeSlots, slots); return buf; };
	void setSlots(int aSlots) { slots = aSlots; };
	void setSlots(const string& aSlots) { setSlots(atoi(aSlots.c_str())); };
	
	void setFreeSlots(int aFreeSlots) { freeSlots = aFreeSlots; };
	void setFreeSlots(const string& aSlots) { setFreeSlots(atoi(aSlots.c_str())); };
	
	GETSETREF(string, nick, Nick);
	GETSETREF(string, file, File);
	GETSETREF(string, hubName, HubName);
	GETSETREF(string, hubAddress, HubAddress);

private:
	User::Ptr user;	
	LONGLONG size;
	int slots;
	int freeSlots;
};

class SearchManagerListener {
public:
	typedef SearchManagerListener* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;
	enum Types {
		SEARCH_RESULT
	};
	virtual void onAction(Types, SearchResult*) { }
};

class SearchManager : public Speaker<SearchManagerListener>, private BufferedSocketListener, public Singleton<SearchManager>
{
public:
	enum {
		SIZE_DONTCARE = 0x00,
		SIZE_ATLEAST = 0x01,
		SIZE_ATMOST = 0x02,
	};

	enum {
		TYPE_ANY = 0,
		TYPE_AUDIO,
		TYPE_COMPRESSED,
		TYPE_DOCUMENT,
		TYPE_EXECUTABLE,
		TYPE_PICTURE,
		TYPE_VIDEO,
		TYPE_FOLDER
	};
	
	void search(const string& aName, LONGLONG aSize = 0, DWORD aFlags = 0, int aType = 0);
	void search(const string& aName, const string& aSize, DWORD aFlags = 0, int aType = 0) {
		search(aName, Util::toInt64(aSize), aFlags, aType);
	}
	
	void setPort(short aPort) throw(SocketException) {
		socket.disconnect();
		socket.create(Socket::TYPE_UDP);
		socket.bind(aPort);
	}

	void onSearchResult(const string& aLine) {
		onData((const BYTE*)aLine.data(), aLine.length());
	}
	
private:
	
	BufferedSocket socket;
	short port;

	friend class Singleton<SearchManager>;

	SearchManager() : socket('|') { 
		try {
			socket.addListener(this);
		} catch(Exception e) {
			// Not good...
			dcdebug("SearchManager::SearchManager caught %s\n", e.getError().c_str());
		}
	};
	// We won't be copying it anyway...
	SearchManager(const SearchManager&) { dcassert(0); };

	virtual ~SearchManager() { 
		socket.removeListener(this);
	};

	// BufferedSocketListener
	virtual void onAction(BufferedSocketListener::Types type, const BYTE* aBuf, int aLen) {
		if(type == BufferedSocketListener::DATA) {
			onData(aBuf, aLen);
		}
	}
	void onData(const BYTE* buf, int aLen);

};

#endif // !defined(AFX_SEARCHMANAGER_H__E8F009DF_D216_4F8F_8C81_07D2FA0BFB7F__INCLUDED_)

/**
 * @file SearchManager.h
 * $Id: SearchManager.h,v 1.13 2002/02/18 23:48:32 arnetheduck Exp $
 * @if LOG
 * $Log: SearchManager.h,v $
 * Revision 1.13  2002/02/18 23:48:32  arnetheduck
 * New prerelease, bugs fixed and features added...
 *
 * Revision 1.12  2002/02/09 18:13:51  arnetheduck
 * Fixed level 4 warnings and started using new stl
 *
 * Revision 1.11  2002/01/25 00:11:26  arnetheduck
 * New settings dialog and various fixes
 *
 * Revision 1.10  2002/01/20 22:54:46  arnetheduck
 * Bugfixes to 0.131 mainly...
 *
 * Revision 1.9  2002/01/18 17:41:43  arnetheduck
 * Reworked many right button menus, adding op commands and making more easy to use
 *
 * Revision 1.8  2002/01/13 22:50:48  arnetheduck
 * Time for 0.12, added favorites, a bunch of new icons and lot's of other stuff
 *
 * Revision 1.7  2002/01/11 14:52:57  arnetheduck
 * Huge changes in the listener code, replaced most of it with templates,
 * also moved the getinstance stuff for the managers to a template
 *
 * Revision 1.6  2002/01/06 00:14:54  arnetheduck
 * Incoming searches almost done, just need some testing...
 *
 * Revision 1.5  2001/12/15 17:01:06  arnetheduck
 * Passive mode searching as well as some searching code added
 *
 * Revision 1.4  2001/12/13 19:21:57  arnetheduck
 * A lot of work done almost everywhere, mainly towards a friendlier UI
 * and less bugs...time to release 0.06...
 *
 * Revision 1.3  2001/12/08 20:59:26  arnetheduck
 * Fixing bugs...
 *
 * Revision 1.2  2001/12/08 14:25:49  arnetheduck
 * More bugs removed...did my first search as well...
 *
 * Revision 1.1  2001/12/07 20:04:32  arnetheduck
 * Time to start working on searching...
 *
 * @endif
 */


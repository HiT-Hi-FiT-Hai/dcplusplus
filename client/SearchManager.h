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

#include "Socket.h"
#include "User.h"
#include "Thread.h"

#include "SearchManagerListener.h"

class SearchResult {
public:	

	enum Types {
		TYPE_FILE,
		TYPE_DIRECTORY
	};

	typedef SearchResult* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;
	
	SearchResult() : type(TYPE_FILE), slots(0), freeSlots(0), size(0) { };
	SearchResult(const SearchResult& rhs) : type(rhs.type), slots(rhs.slots), freeSlots(rhs.freeSlots), size(rhs.size), 
		file(rhs.file), hubName(rhs.hubName), hubAddress(rhs.hubAddress), user(rhs.user) { };

	string getFileName();

	User::Ptr& getUser() { return user; };
	void setUser(const User::Ptr& aUser) { user = aUser; };
	
	void setSize(const string& aSize) { setSize(Util::toInt64(aSize)); };
	void setSlots(const string& aSlots) { setSlots(Util::toInt(aSlots)); };
	void setFreeSlots(const string& aSlots) { setFreeSlots(Util::toInt(aSlots)); };
	
	string getSlotString() { return Util::toString(getFreeSlots()) + '/' + Util::toString(getSlots()); };
	
	GETSET(Types, type, Type);
	GETSET(int, slots, Slots);
	GETSET(int, freeSlots, FreeSlots);
	GETSET(int64_t, size, Size);
	GETSETREF(string, file, File);
	GETSETREF(string, hubName, HubName);
	GETSETREF(string, hubAddress, HubAddress);

private:
	User::Ptr user;
};

class SearchManager : public Speaker<SearchManagerListener>, public Singleton<SearchManager>, public Thread
{
public:
	enum SizeModes {
		SIZE_DONTCARE = 0x00,
		SIZE_ATLEAST = 0x01,
		SIZE_ATMOST = 0x02,
	};

	enum TypeModes {
		TYPE_ANY = 0,
		TYPE_AUDIO,
		TYPE_COMPRESSED,
		TYPE_DOCUMENT,
		TYPE_EXECUTABLE,
		TYPE_PICTURE,
		TYPE_VIDEO,
		TYPE_DIRECTORY
	};
	
	void search(const string& aName, int64_t aSize = 0, TypeModes aTypeMode = TYPE_ANY, SizeModes aSizeMode = SIZE_ATLEAST);
	void search(const string& aName, const string& aSize, TypeModes aTypeMode = TYPE_ANY, SizeModes aSizeMode = SIZE_ATLEAST) {
		search(aName, Util::toInt64(aSize), aTypeMode, aSizeMode);
	}
	
	static string clean(const string& aSearchString) {
		static const char* badChars = "$|.[]()-_+";
		string::size_type i = aSearchString.find_first_of(badChars);
		if(i == string::npos)
			return aSearchString;

		string tmp = aSearchString;
		// Remove all strange characters from the search string
		do {
			tmp[i] = ' ';
		} while ( (i = tmp.find_first_of(badChars, i)) != string::npos);
		
		return tmp;
	}

	void setPort(short aPort) throw(SocketException);
	void onSearchResult(const string& aLine) {
		onData((const u_int8_t*)aLine.data(), aLine.length());
	}
	
private:
	
	Socket* socket;
	short port;

	friend class Singleton<SearchManager>;

	SearchManager() : socket(NULL) {  };
	// We won't be copying it anyway...
	SearchManager(const SearchManager&) { dcassert(0); };

	virtual int run();

	virtual ~SearchManager() { 
		if(socket) {
			socket->disconnect();
			join();
			delete socket;
		}
	};

	void onData(const u_int8_t* buf, int aLen);
};

#endif // !defined(AFX_SEARCHMANAGER_H__E8F009DF_D216_4F8F_8C81_07D2FA0BFB7F__INCLUDED_)

/**
 * @file SearchManager.h
 * $Id: SearchManager.h,v 1.22 2002/12/28 01:31:49 arnetheduck Exp $
 */

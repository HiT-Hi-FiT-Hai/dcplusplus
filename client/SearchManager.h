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
	
	SearchResult() : slots(0), freeSlots(0), size(0) { };
	SearchResult(const SearchResult& rhs) : slots(rhs.slots), freeSlots(rhs.freeSlots), size(rhs.size), 
		file(rhs.file), hubName(rhs.hubName), hubAddress(rhs.hubAddress), user(rhs.user) { };

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
	
	void setSize(const string& aSize) { setSize(Util::toInt64(aSize)); };
	void setSlots(const string& aSlots) { setSlots(Util::toInt(aSlots)); };
	void setFreeSlots(const string& aSlots) { setFreeSlots(Util::toInt(aSlots)); };
	
	string getSlotString() { return Util::toString(getFreeSlots()) + '/' + Util::toString(getSlots()); };
	
	GETSET(int, slots, Slots);
	GETSET(int, freeSlots, FreeSlots);
	GETSET(int64_t, size, Size);
	GETSETREF(string, file, File);
	GETSETREF(string, hubName, HubName);
	GETSETREF(string, hubAddress, HubAddress);

private:
	User::Ptr user;	
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
		TYPE_FOLDER
	};
	
	void search(const string& aName, int64_t aSize = 0, TypeModes aTypeMode = TYPE_ANY, SizeModes aSizeMode = SIZE_ATLEAST);
	void search(const string& aName, const string& aSize, TypeModes aTypeMode = TYPE_ANY, SizeModes aSizeMode = SIZE_ATLEAST) {
		search(aName, Util::toInt64(aSize), aTypeMode, aSizeMode);
	}
	
	static string clean(const string& aSearchString) {
		string tmp = aSearchString;
		// Remove all strange characters from the search string
		string::size_type i = 0;

		while( (i = tmp.find_first_of("$|.[]()-_+")) != string::npos) {
			tmp[i] = ' ';
		}
		return tmp;
	}
	void setPort(short aPort) throw(SocketException) {
		if(!socket) {
			socket = BufferedSocket::getSocket('|');
			socket->addListener(this);
		}
		socket->disconnect();
		socket->create(Socket::TYPE_UDP);
		socket->bind(aPort);
	}

	void onSearchResult(const string& aLine) {
		onData((const u_int8_t*)aLine.data(), aLine.length());
	}
	
private:
	
	BufferedSocket* socket;
	short port;

	friend class Singleton<SearchManager>;

	SearchManager() : socket(NULL) { 
	};
	// We won't be copying it anyway...
	SearchManager(const SearchManager&) { dcassert(0); };

	virtual ~SearchManager() { 
		if(socket) {
			socket->removeListener(this);
			BufferedSocket::putSocket(socket);
		}

	};

	// BufferedSocketListener
	virtual void onAction(BufferedSocketListener::Types type, const u_int8_t* aBuf, int aLen) {
		if(type == BufferedSocketListener::DATA) {
			onData(aBuf, aLen);
		}
	}
	void onData(const u_int8_t* buf, int aLen);

};

#endif // !defined(AFX_SEARCHMANAGER_H__E8F009DF_D216_4F8F_8C81_07D2FA0BFB7F__INCLUDED_)

/**
 * @file SearchManager.h
 * $Id: SearchManager.h,v 1.19 2002/04/13 12:57:23 arnetheduck Exp $
 */


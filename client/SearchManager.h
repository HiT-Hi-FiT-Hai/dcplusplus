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

#if !defined(AFX_SEARCHMANAGER_H__E8F009DF_D216_4F8F_8C81_07D2FA0BFB7F__INCLUDED_)
#define AFX_SEARCHMANAGER_H__E8F009DF_D216_4F8F_8C81_07D2FA0BFB7F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SettingsManager.h"

#include "Socket.h"
#include "User.h"
#include "Thread.h"
#include "Client.h"
#include "Singleton.h"

#include "SearchManagerListener.h"

class SearchManager;

class SearchResult {
public:	

	enum Types {
		TYPE_FILE,
		TYPE_DIRECTORY
	};

	typedef SearchResult* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;
	
	SearchResult(const User::Ptr& aUser, Types aType, int aSlots, int aFreeSlots, int64_t aSize, 
		const string& aFile, const string& aHubName, const string& aHubIpPort) : 
	file(aFile), hubName(aHubName), hubIpPort(aHubIpPort), user(aUser), size(aSize), slots(aSlots),
		freeSlots(aFreeSlots), ref(1) { };

	string getFileName();
	string toSR();

	User::Ptr& getUser() { return user; };
	string getSlotString() { return Util::toString(getFreeSlots()) + '/' + Util::toString(getSlots()); };

	const string& getFile() { return file; };
	const string& getHubIpPort() { return hubIpPort; };
	const string& getHubName() { return hubName; };
	int64_t getSize() { return size; };
	Types getType() { return type; };
	int getSlots() { return slots; };
	int getFreeSlots() { return freeSlots; };

	void incRef() { Thread::safeInc(&ref); };
	void decRef() { 
		if(Thread::safeDec(&ref) == 0) 
			delete this; 
	};

private:
	friend class SearchManager;

	SearchResult() : size(0), slots(0), freeSlots(0), ref(1) { };
	~SearchResult() { };

	SearchResult(const SearchResult& rhs) : 
	   file(rhs.file), hubName(rhs.hubName), hubIpPort(rhs.hubIpPort), 
		   user(rhs.user), size(rhs.size), slots(rhs.slots), freeSlots(rhs.freeSlots), ref(1) { };

	string file;
	string hubName;
	string hubIpPort;
	User::Ptr user;
	int64_t size;
	Types type;
	int slots;
	int freeSlots;

	long ref;
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
	
	void search(Client::List& who, const string& aName, int64_t aSize = 0, TypeModes aTypeMode = TYPE_ANY, SizeModes aSizeMode = SIZE_ATLEAST);
	void search(Client::List& who, const string& aName, const string& aSize, TypeModes aTypeMode = TYPE_ANY, SizeModes aSizeMode = SIZE_ATLEAST) {
		search(who, aName, Util::toInt64(aSize), aTypeMode, aSizeMode);
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
	void disconnect() throw();
	void onSearchResult(const string& aLine) {
		onData((const u_int8_t*)aLine.data(), aLine.length());
	}
	
private:
	
	Socket* socket;
	short port;
	bool stop;
	friend class Singleton<SearchManager>;

	SearchManager() : socket(NULL), port(0), stop(false) {  };

	virtual int run();

	virtual ~SearchManager() { 
		if(socket) {
			stop = true;
			socket->disconnect();
			join();
			delete socket;
		}
	};

	void onData(const u_int8_t* buf, int aLen);
};

#endif // !defined(AFX_SEARCHMANAGER_H__E8F009DF_D216_4F8F_8C81_07D2FA0BFB7F__INCLUDED_)

/**
 * @file
 * $Id: SearchManager.h,v 1.28 2003/11/12 21:45:00 arnetheduck Exp $
 */

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
 * $Id: SearchManager.h,v 1.18 2002/04/09 18:43:28 arnetheduck Exp $
 * @if LOG
 * $Log: SearchManager.h,v $
 * Revision 1.18  2002/04/09 18:43:28  arnetheduck
 * Major code reorganization, to ease maintenance and future port...
 *
 * Revision 1.17  2002/03/13 20:35:26  arnetheduck
 * Release canditate...internationalization done as far as 0.155 is concerned...
 * Also started using mirrors of the public hub lists
 *
 * Revision 1.16  2002/03/10 22:41:08  arnetheduck
 * Working on internationalization...
 *
 * Revision 1.15  2002/02/27 12:02:09  arnetheduck
 * Completely new user handling, wonder how it turns out...
 *
 * Revision 1.14  2002/02/25 15:39:29  arnetheduck
 * Release 0.154, lot of things fixed...
 *
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


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

#if !defined(AFX_SEARCHMANAGER_H__E8F009DF_D216_4F8F_8C81_07D2FA0BFB7F__INCLUDED_)
#define AFX_SEARCHMANAGER_H__E8F009DF_D216_4F8F_8C81_07D2FA0BFB7F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BufferedSocket.h"

class SearchResult {
public:	
	const string& getNick() { return nick; };
	void setNick(const string& aNick) { nick = aNick; };
	
	const string& getFile() { return file; };
	void setFile(const string& aFile) { file = aFile; };
	
	const string& getHubName() { return hubName; };
	void setHubName(const string aHub) { hubName = aHub; };
	
	const string& getHubAddress() { return hubAddress; };
	void setHubAddress(const string& aAddress) { hubAddress = aAddress; };
	
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
	
private:
	string nick;
	string file;
	string hubName;
	string hubAddress;
	
	LONGLONG size;
	int slots;
	int freeSlots;
};

class SearchManagerListener {
public:
	typedef SearchManagerListener* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;
	
	virtual void onSearchResult(SearchResult* aResult) { };
};

class SearchManager : public Speaker<SearchManagerListener>, private BufferedSocketListener
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
		search(aName, _atoi64(aSize.c_str()), aFlags, aType);
	}
	
	static SearchManager* getInstance() {
		dcassert(instance);
		return instance;
	}
	static void newInstance() {
		if(instance)
			delete instance;
		
		instance = new SearchManager();
	}
	static void deleteInstance() {
		delete instance;
		instance = NULL;
	}
	
	void setPort(short aPort) throw(SocketException) {
		socket.disconnect();
		socket.create(Socket::TYPE_UDP);
		socket.bind(aPort);
	}
private:
	static SearchManager* instance;
	
	BufferedSocket socket;
	short port;

	SearchManager() : socket('|') { 
		try {
			socket.addListener(this);
		} catch(Exception e) {
			// Not good...
			dcdebug("SearchManager::SearchManager caught %s\n", e.getError().c_str());
		}
	};
	virtual ~SearchManager() { 
		socket.removeListener(this);
	};

	virtual void onData(BYTE* buf, int aLen);

	void fireResult(SearchResult* aResult) {
		listenerCS.enter();
		SearchManagerListener::List tmp = listeners;
		listenerCS.leave();
		for(SearchManagerListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onSearchResult(aResult);
		}
	}
	
	
};

#endif // !defined(AFX_SEARCHMANAGER_H__E8F009DF_D216_4F8F_8C81_07D2FA0BFB7F__INCLUDED_)

/**
 * @file SearchManager.h
 * $Id: SearchManager.h,v 1.4 2001/12/13 19:21:57 arnetheduck Exp $
 * @if LOG
 * $Log: SearchManager.h,v $
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


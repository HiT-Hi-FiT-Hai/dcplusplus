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

#if !defined(AFX_HUBMANAGER_H__75858D5D_F12F_40D0_B127_5DDED226C098__INCLUDED_)
#define AFX_HUBMANAGER_H__75858D5D_F12F_40D0_B127_5DDED226C098__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Exception.h"
STANDARD_EXCEPTION(HubException);

class HubManagerListener {
public:
	typedef HubManagerListener* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;

	virtual void onMessage(const string& aMessage) { };
	virtual void onHub(const string& aName, const string& aServer, const string& aDescription, const string& aUsers) { };
	virtual void onFinished() { };
};

class HubManager  
{
public:

	/**
	 * @todo A reliable way of handling more than one listener...
	 */
	void addListener(HubManagerListener::Ptr aListener) {
		if(listeners.size() < 1)
			listeners.push_back(aListener);
	}
	
	void removeListener(HubManagerListener::Ptr aListener) {
		for(HubManagerListener::Iter i = listeners.begin(); i != listeners.end(); ++i) {
			if(*i == aListener) {
				listeners.erase(i);
				break;
			}
		}
		if(listeners.size() == 0)
			stopLister();
	}
	
	void removeListeners() {
		listeners.clear();
		stopLister();
	}
	
	static HubManager* getInstance() {
		dcassert(instance);
		return instance;
	}
	static void newInstance() {
		if(instance)
			delete instance;
		
		instance = new HubManager();
	}
	static void deleteInstance() {
		delete instance;
		instance = NULL;
	}
	
	void getPublicHubs(bool aRefresh=false) {
		if(aRefresh) {
			startReader();
		}

		startLister();
	}

	void stopListing() {
		stopLister();
	}

private:
	
	class HubEntry {
	public:
		typedef vector<HubEntry> List;
		typedef List::iterator Iter;
		string name;
		string server;
		string description;
		string users;
		HubEntry(const string& aName, const string& aServer, const string& aDescription, const string& aUsers) : 
		name(aName), server(aServer), description(aDescription), users(aUsers) { };
	};
	
	HubManagerListener::List listeners;
	
	static HubManager* instance;
	HubEntry::List publicHubs;
	CRITICAL_SECTION publicCS;
	
	HubManager() : readerThread(NULL), readerEvent(NULL), listerThread(NULL), listerEvent(NULL) {
		InitializeCriticalSection(&publicCS);
		startReader();
	}

	~HubManager() {
		DeleteCriticalSection(&publicCS);
		stopReader();
	}
	
	HANDLE listerThread;
	HANDLE listerEvent;
	static DWORD WINAPI lister(void* p);
	
	void startLister() {
		stopLister();
		DWORD threadId;
		listerEvent=CreateEvent(NULL, FALSE, FALSE, NULL);
		listerThread=CreateThread(NULL, 0, &lister, this, 0, &threadId);
	}
	
	void stopLister() {
		if(listerThread != NULL) {
			SetEvent(listerEvent);
			
			if(WaitForSingleObject(listerThread, 2000) == WAIT_TIMEOUT) {
				MessageBox(NULL, _T("Unable to stop lister thread!!!"), _T("Internal error"), MB_OK | MB_ICONERROR);
			}
			
			listerThread = NULL;
			CloseHandle(listerEvent);
			listerEvent = NULL;
		}
	}
	
	HANDLE readerThread;
	HANDLE readerEvent;
	static DWORD WINAPI reader(void* p);

	void startReader() {
		stopReader();
		DWORD threadId;
		readerEvent=CreateEvent(NULL, FALSE, FALSE, NULL);
		readerThread=CreateThread(NULL, 0, &reader, this, 0, &threadId);
	}
	
	void stopReader() {
		if(readerThread != NULL) {
			SetEvent(readerEvent);
			
			if(WaitForSingleObject(readerThread, 1000) == WAIT_TIMEOUT) {
				MessageBox(NULL, _T("Unable to stop reader thread!!!"), _T("Internal error"), MB_OK | MB_ICONERROR);
			}
			
			readerThread = NULL;
			CloseHandle(readerEvent);
			readerEvent = NULL;
		}
	}
	
	void fireMessage(const string& aMessage) {
		HubManagerListener::List tmp = listeners;
//		dcdebug("fireMessage\n");
		for(HubManagerListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onMessage(aMessage);
		}
	}

	void fireHub(const string& aName, const string& aServer, const string& aDescription, const string& aUsers) {
		HubManagerListener::List tmp = listeners;
//		dcdebug("fireHub\n");
		for(HubManagerListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onHub(aName, aServer, aDescription, aUsers);
		}
	}

	void fireFinished() {
		HubManagerListener::List tmp = listeners;
		//		dcdebug("fireMessage\n");
		for(HubManagerListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onFinished();
		}
	}
};

#endif // !defined(AFX_HUBMANAGER_H__75858D5D_F12F_40D0_B127_5DDED226C098__INCLUDED_)

/**
 * @file HubManager.h
 * $Id: HubManager.h,v 1.3 2001/11/26 23:40:36 arnetheduck Exp $
 * @if LOG
 * $Log: HubManager.h,v $
 * Revision 1.3  2001/11/26 23:40:36  arnetheduck
 * Downloads!! Now downloads are possible, although the implementation is
 * likely to change in the future...more UI work (splitters...) and some bug
 * fixes. Only user file listings are downloadable, but at least it's something...
 *
 * Revision 1.2  2001/11/25 22:06:25  arnetheduck
 * Finally downloading is working! There are now a few quirks and bugs to be fixed
 * but what the heck....!
 *
 * Revision 1.1.1.1  2001/11/21 17:33:20  arnetheduck
 * Inital release
 *
 * @endif
 */


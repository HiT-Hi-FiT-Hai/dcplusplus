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
#include "Util.h"
#include "CriticalSection.h"
#include "HttpConnection.h"

STANDARD_EXCEPTION(HubException);

class HubManagerListener {
public:
	typedef HubManagerListener* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;

	virtual void onHubMessage(const string& aMessage) { };
	virtual void onHubStarting() { };
	virtual void onHub(const string& aName, const string& aServer, const string& aDescription, const string& aUsers) { };
	virtual void onHubFinished() { };
};

class HubManager : public Speaker<HubManagerListener>, private HttpConnectionListener
{
public:

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
			refresh();
		}

		startLister();
	}

	void stopListing() {
		SetEvent(listerEvent);
	}

	void refresh() {
		publicCS.enter();
		publicHubs.clear();
		publicCS.leave();
		
		if(conn) {
			conn->removeListener(this);
			delete conn;
		}
		ResetEvent(downloadEvent);
		
		conn = new HttpConnection();
		conn->addListener(this);
		conn->DownloadFile("http://www.neo-modus.com/PublicHubList.config");
		
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
		HubEntry() { };
	};
	
	static HubManager* instance;
	HubEntry::List publicHubs;
	CriticalSection publicCS;
	HttpConnection* conn;
		
	HubManager() : listerThread(NULL), listerEvent(NULL), conn(NULL) {
		downloadEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	}

	~HubManager() {
		CloseHandle(downloadEvent);

		stopLister();

		if(conn) {
			conn->removeListener(this);
			delete conn;
		}
	}
	
	HANDLE listerThread;
	HANDLE listerEvent;
	static DWORD WINAPI lister(void* p);
	
	HANDLE downloadEvent;
	string downloadBuf;

	// HttpConnectionListener
	virtual void onHttpData(HttpConnection* aConn, const BYTE* aBuf, int aLen);
	virtual void onHttpError(HttpConnection* aConn, const string& aError);
	virtual void onHttpComplete(HttpConnection* aConn);	

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
			CloseHandle(listerThread);
			listerThread = NULL;
		}
		if(listerEvent) {
			CloseHandle(listerEvent);
			listerEvent = NULL;
		}
	}
	
	void fireMessage(const string& aMessage) {
		listenerCS.enter();
		HubManagerListener::List tmp = listeners;
		listenerCS.leave();
		//		dcdebug("fireMessage\n");
		for(HubManagerListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onHubMessage(aMessage);
		}
	}

	void fireHub(const string& aName, const string& aServer, const string& aDescription, const string& aUsers) {
		listenerCS.enter();
		HubManagerListener::List tmp = listeners;
		listenerCS.leave();
		//		dcdebug("fireHub\n");
		for(HubManagerListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onHub(aName, aServer, aDescription, aUsers);
		}
	}

	void fireFinished() {
		listenerCS.enter();
		HubManagerListener::List tmp = listeners;
		listenerCS.leave();
		//		dcdebug("fireMessage\n");
		for(HubManagerListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onHubFinished();
		}
	}
	void fireStarting() {
		listenerCS.enter();
		HubManagerListener::List tmp = listeners;
		listenerCS.leave();
		//		dcdebug("fireMessage\n");
		for(HubManagerListener::Iter i=tmp.begin(); i != tmp.end(); ++i) {
			(*i)->onHubStarting();
		}
	}
};

#endif // !defined(AFX_HUBMANAGER_H__75858D5D_F12F_40D0_B127_5DDED226C098__INCLUDED_)

/**
 * @file HubManager.h
 * $Id: HubManager.h,v 1.10 2001/12/29 13:47:14 arnetheduck Exp $
 * @if LOG
 * $Log: HubManager.h,v $
 * Revision 1.10  2001/12/29 13:47:14  arnetheduck
 * Fixing bugs and UI work
 *
 * Revision 1.9  2001/12/15 17:01:06  arnetheduck
 * Passive mode searching as well as some searching code added
 *
 * Revision 1.8  2001/12/13 19:21:57  arnetheduck
 * A lot of work done almost everywhere, mainly towards a friendlier UI
 * and less bugs...time to release 0.06...
 *
 * Revision 1.7  2001/12/11 01:10:29  arnetheduck
 * More bugfixes...I really have to change the bufferedsocket so that it only
 * uses one thread...or maybe even multiple sockets/thread...
 *
 * Revision 1.6  2001/12/07 20:03:10  arnetheduck
 * More work done towards application stability
 *
 * Revision 1.5  2001/12/02 23:47:35  arnetheduck
 * Added the framework for uploading and file sharing...although there's something strange about
 * the file lists...my client takes them, but not the original...
 *
 * Revision 1.4  2001/12/02 11:16:46  arnetheduck
 * Optimised hub listing, removed a few bugs and leaks, and added a few small
 * things...downloads are now working, time to start writing the sharing
 * code...
 *
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


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

#include "stdafx.h"
#include "DCPlusPlus.h"

#include "HubManager.h"

#include "HttpConnection.h"
#include "StringTokenizer.h"

HubManager* HubManager::instance = NULL;

DWORD WINAPI HubManager::lister(void* p) {
	ATLASSERT(p);
	HubManager* hm = (HubManager*)p;
	HANDLE hEvent[2];
	hEvent[0] = hm->listerEvent;

	hm->fireMessage("Downloading public server list...");
	
	if(hm->readerThread != NULL) {
		hEvent[1] = hm->readerThread;
		if(WaitForMultipleObjects(2, hEvent, FALSE, INFINITE)==WAIT_OBJECT_0) {
			ATLTRACE("Hub Lister Thread ended");
			hm->listerThread = NULL;
			return 0;
		}
	}
	
	EnterCriticalSection(&hm->publicCS);

	if(hm->publicHubs.size() == 0) {
		hm->fireMessage("Unable to download public server list. Check your internet connection!");
	} else {
		hm->fireMessage("Download complete");
	}
	
	for(HubManager::HubEntry::Iter i = hm->publicHubs.begin(); i != hm->publicHubs.end(); ++i) {
		if(WaitForSingleObject(hEvent[0], 0)!=WAIT_TIMEOUT) {
			ATLTRACE("Hub Lister Thread ended");
			hm->listerThread = NULL;
			LeaveCriticalSection(&hm->publicCS);
			return 0;
		}
		hm->fireHub(i->name, i->server, i->description, i->users);
	}

	LeaveCriticalSection(&hm->publicCS);
	hm->listerThread = NULL;
	return 0;
}

/**
 * Return a list of hub entries for the currently known public hubs.
 * Locks execution until list has been downloaded.
 * @param aRefresh Refresh list from neomodus server.
 */
DWORD WINAPI HubManager::reader(void *p) {

	ATLASSERT(p);
	HubManager* hm = (HubManager*)p;
	HANDLE hEvent[2];
	hEvent[0] = hm->readerEvent;
	
	EnterCriticalSection(&hm->publicCS);
	
	hm->publicHubs.clear();

	try {
		StringTokenizer tokens(HttpConnection::DownloadTextFile("http://www.neo-modus.com/PublicHubList.config"));
		StringList t = tokens.getTokens();

		for(StringIter i = t.begin(); i!=t.end();i++) {
			StringTokenizer hub(*i, '|');
			StringIter j = hub.getTokens().begin();
			string& name = *j++;
			string& server = *j++;
			string& desc = *j++;
			string& users = *j++;
			hm->publicHubs.push_back(HubEntry(name, server, desc, users));
		}
	} catch(Exception e) {
		dcdebug("Can't get Hub List...");
	}

	LeaveCriticalSection(&hm->publicCS);
	hm->readerThread = NULL;

	return 0;	
}

/**
 * @file HubManager.cpp
 * $Id: HubManager.cpp,v 1.2 2001/11/25 22:06:25 arnetheduck Exp $
 * @if LOG
 * $Log: HubManager.cpp,v $
 * Revision 1.2  2001/11/25 22:06:25  arnetheduck
 * Finally downloading is working! There are now a few quirks and bugs to be fixed
 * but what the heck....!
 *
 * Revision 1.1.1.1  2001/11/21 17:33:20  arnetheduck
 * Inital release
 *
 * @endif
 */


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
	hm->fireStarting();
	
	// Wait for downloaders to finish
	hEvent[1] = hm->downloadEvent;
	if(WaitForMultipleObjects(2, hEvent, FALSE, INFINITE)==WAIT_OBJECT_0) {
		ATLTRACE("Hub Lister Thread ended");
		return 0;
	}
	
	hm->publicCS.enter();
	if(hm->publicHubs.size() == 0) {
		hm->fireMessage("Unable to download public server list. Check your internet connection!");
	} else {
		hm->fireMessage("Download complete");
	}
	
	for(HubManager::HubEntry::Iter i = hm->publicHubs.begin(); i != hm->publicHubs.end(); ++i) {
		if(WaitForSingleObject(hEvent[0], 0)!=WAIT_TIMEOUT) {
			ATLTRACE("Hub Lister Thread ended");
			hm->publicCS.leave();
			return 0;
		}
		hm->fireHub(i->name, i->server, i->description, i->users);
	}
	hm->fireFinished();
	
	hm->publicCS.leave();
	return 0;
}

void HubManager::onHttpData(HttpConnection* aConn, const BYTE* aBuf, int aLen) {
	downloadBuf.append((char*)aBuf, aLen);
	string::size_type i;
	
	publicCS.enter();

	while( (i=downloadBuf.find("\r\n")) != string::npos) {
		StringTokenizer tok(downloadBuf.substr(0, i), '|');
		downloadBuf = downloadBuf.substr(i+2);

		StringIter j = tok.getTokens().begin();
		string& name = *j++;
		string& server = *j++;
		string& desc = *j++;
		string& users = *j++;
		publicHubs.push_back(HubEntry(name, server, desc, users));
	}

	publicCS.leave();
}

void HubManager::onHttpComplete(HttpConnection* aConn) {
	SetEvent(downloadEvent);
	aConn->removeListener(this);
}

void HubManager::onHttpError(HttpConnection* aConn, const string& aError) {
	SetEvent(downloadEvent);
	aConn->removeListener(this);
}


/**
 * @file HubManager.cpp
 * $Id: HubManager.cpp,v 1.8 2002/01/05 10:13:39 arnetheduck Exp $
 * @if LOG
 * $Log: HubManager.cpp,v $
 * Revision 1.8  2002/01/05 10:13:39  arnetheduck
 * Automatic version detection and some other updates
 *
 * Revision 1.7  2001/12/15 17:01:06  arnetheduck
 * Passive mode searching as well as some searching code added
 *
 * Revision 1.6  2001/12/13 19:21:57  arnetheduck
 * A lot of work done almost everywhere, mainly towards a friendlier UI
 * and less bugs...time to release 0.06...
 *
 * Revision 1.5  2001/12/11 01:10:29  arnetheduck
 * More bugfixes...I really have to change the bufferedsocket so that it only
 * uses one thread...or maybe even multiple sockets/thread...
 *
 * Revision 1.4  2001/12/07 20:03:07  arnetheduck
 * More work done towards application stability
 *
 * Revision 1.3  2001/12/02 11:16:46  arnetheduck
 * Optimised hub listing, removed a few bugs and leaks, and added a few small
 * things...downloads are now working, time to start writing the sharing
 * code...
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


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

#include "UploadManager.h"

UploadManager* UploadManager::instance = NULL;

void UploadManager::onGet(UserConnection* aSource, const string& aFile, LONGLONG aResume) {
	Upload* u;
	HANDLE h;
	cs.enter();
	
	try {
		if((getFreeSlots()<0)) {
			aSource->maxedOut();
			removeConnection(aSource);
			cs.leave();
			return;
		}
		// We only give out one connection / user...
		for(UserConnection::Iter k = connections.begin(); k != connections.end(); ++k) {
			if(aSource != *k && aSource->getUser() == (*k)->getUser()) {
				aSource->maxedOut();
				removeConnection(aSource);
				cs.leave();
				return;					
			}
		}
		string file = ShareManager::getInstance()->translateFileName(aFile);
		Upload::MapIter i = uploads.find(aSource);
		if(i != uploads.end()) {
			// This is bad!
			
			dcdebug("UploadManager::onGet Unexpected command\n");				
			fire(UploadManagerListener::FAILED, i->second, "Unexpected command");
			delete i->second;
			dcdebug("onGet: Removing upload\n");
			uploads.erase(i);
			cs.leave();
			removeConnection(aSource);
			return;
		} 
		h = CreateFile(file.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
		if(h == INVALID_HANDLE_VALUE) {
			aSource->error("File Not Available");
			cs.leave();
			return;
		}
		
		u = new Upload();
		u->setFile(h, true);
		u->setPos(aResume, true);
		u->setFileName(aFile);
		u->setUser(aSource->getUser());
		
		char buf[24];
		aSource->fileLength(_i64toa(u->getSize(), buf, 10));
		uploads[aSource] = u;
		
	} catch (ShareException e) {
		aSource->error("File Not Available");
	} catch(SocketException e) {
		dcdebug("UploadManager::onGet caught: %s\n", e.getError().c_str());
	}
	
	cs.leave();
}

void UploadManager::onSend(UserConnection* aSource) {
	Upload* u;
	cs.enter();
	Upload::MapIter i = uploads.find(aSource);
	if(i==uploads.end()) {
		// Huh? Where did this come from?
		removeConnection(aSource);
		cs.leave();
		return;
	}
	
	u = i->second;
	try {
		u->setStart(TimerManager::getTick());
		aSource->transmitFile(u->getFile());
		fire(UploadManagerListener::STARTING, u);
	} catch(Exception e) {
		dcdebug("UploadManager::onGet caught: %s\n", e.getError().c_str());
		dcdebug("onSend: Removing upload\n");
		uploads.erase(i);
		delete u;
		removeConnection(aSource);
	}
	cs.leave();
}

void UploadManager::onBytesSent(UserConnection* aSource, DWORD aBytes) {
	Upload* u;
	cs.enter();
	Upload::MapIter i = uploads.find(aSource);
	if(i == uploads.end()) {
		// Something strange happened?
		dcdebug("onBytesSent: Upload not found???\n");
		cs.leave();
		removeConnection(aSource);
		return;
	}
	u = i->second;
	cs.leave();
	u->addPos(aBytes);
	//fire(UploadManagerListener::TICK, u);
}

void UploadManager::onFailed(UserConnection* aSource, const string& aError) {
	Upload* u;
	aSource->disconnect();
	cs.enter();
	Upload::MapIter i = uploads.find(aSource);
	if(i != uploads.end()) {
		u = i->second;
		fire(UploadManagerListener::FAILED, u, aError);
		dcdebug("onError: Removing upload\n");
		uploads.erase(i);
		delete u;
	}
	cs.leave();
	removeConnection(aSource);
}

void UploadManager::onTransmitDone(UserConnection* aSource) {
	Upload * u;
	cs.enter();
	Upload::MapIter i = uploads.find(aSource);
	if(i == uploads.end()) {
		// Something strange happened?
		dcdebug("onTransmitDone: Upload not found???\n");
		
		cs.leave();
		removeConnection(aSource);
		return;
	}
	u = i->second;
	fire(UploadManagerListener::COMPLETE, u);
	dcdebug("onTransmitDone: Removing upload\n");
	uploads.erase(i);
	cs.leave();
	delete u;
	
}

/**
 * @file UploadManger.cpp
 * $Id: UploadManager.cpp,v 1.5 2002/01/15 00:41:54 arnetheduck Exp $
 * @if LOG
 * $Log: UploadManager.cpp,v $
 * Revision 1.5  2002/01/15 00:41:54  arnetheduck
 * late night fixes...
 *
 * Revision 1.4  2002/01/13 22:50:48  arnetheduck
 * Time for 0.12, added favorites, a bunch of new icons and lot's of other stuff
 *
 * Revision 1.3  2002/01/11 14:52:57  arnetheduck
 * Huge changes in the listener code, replaced most of it with templates,
 * also moved the getinstance stuff for the managers to a template
 *
 * Revision 1.2  2002/01/05 10:13:40  arnetheduck
 * Automatic version detection and some other updates
 *
 * Revision 1.1  2001/11/25 22:06:25  arnetheduck
 * Finally downloading is working! There are now a few quirks and bugs to be fixed
 * but what the heck....!
 *
 * @endif
 */

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

#include "stdafx.h"
#include "DCPlusPlus.h"

#include "UploadManager.h"

UploadManager* UploadManager::instance = NULL;

void UploadManager::onGet(UserConnection* aSource, const string& aFile, LONGLONG aResume) {
	Upload* u;
	dcassert(aFile.size() > 0);
	
	try {
		bool userlist = false;
		bool smallfile = false;

		string file;
		try {
			file = ShareManager::getInstance()->translateFileName(aFile);
		} catch(ShareException e) {
			aSource->error("File Not Available");
			return;
		}
		
		if( stricmp(aFile.c_str(), "MyList.DcLst") == 0 ) {
			userlist = true;
		}

		if( Util::getFileSize(file) < (LONGLONG)(16 * 1024) ) {
			smallfile = true;
		}

		cs.enter();
		if( (getFreeSlots()<=0) && !( (smallfile || userlist) && (getFreeExtraSlots() > 0) && (aSource->getUser()->isSet(User::DCPLUSPLUS)) ) ) {
			cs.leave();
			aSource->maxedOut();
			removeConnection(aSource);
			return;
		}

		// We only give out one connection / user...
		for(UserConnection::Iter k = connections.begin(); k != connections.end(); ++k) {
			if(aSource != *k && aSource->getUser() == (*k)->getUser()) {
				cs.leave();
				aSource->maxedOut();
				removeConnection(aSource);
				return;					
			}
		}

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

		File* f;
		try {
			f = new File(file, File::READ, File::OPEN);
		} catch(FileException e) {
			cs.leave();
			aSource->error("File Not Available");
			return;
		}
		
		u = new Upload();
		u->setFile(f, true);
		u->setPos(aResume, true);
		u->setFileName(aFile);
		u->setUser(aSource->getUser());
		if(smallfile)
			u->setFlag(Upload::SMALL_FILE);
		if(userlist)
			u->setFlag(Upload::USER_LIST);
		
		try {
			aSource->fileLength(Util::toString(u->getSize()));
		} catch (...) {
			// Make sure we leave the critical section...
			cs.leave();
			throw;
		}
		
		uploads[aSource] = u;
		if(isExtra(u)) {
			extra++;
		} else {
			running++;
		}
		
		cs.leave();
			
	} catch(SocketException e) {
		dcdebug("UploadManager::onGet caught: %s\n", e.getError().c_str());
		removeConnection(aSource);
	}
}

void UploadManager::onSend(UserConnection* aSource) {
	Upload* u;
	cs.enter();
	Upload::MapIter i = uploads.find(aSource);
	if(i==uploads.end()) {
		// Huh? Where did this come from?
		cs.leave();
		removeConnection(aSource);
		return;
	}
	
	u = i->second;
	cs.leave();

	try {
		u->setStart(TimerManager::getTick());
		aSource->transmitFile(u->getFile());
		fire(UploadManagerListener::STARTING, u);
	} catch(Exception e) {
		dcdebug("UploadManager::onGet caught: %s\n", e.getError().c_str());

		{
			Lock l(cs);
			uploads.erase(aSource);

			if(isExtra(u)) {
				extra--;
			} else {
				running--;
			}
		}
		
		delete u;
		removeConnection(aSource);
	}
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
	cs.enter();
	Upload::MapIter i = uploads.find(aSource);
	if(i != uploads.end()) {
		u = i->second;
		uploads.erase(i);

		if(isExtra(u)) {
			extra--;
		} else {
			running--;
		}
		
		cs.leave();

		fire(UploadManagerListener::FAILED, u, aError);
		dcdebug("onError: Removing upload\n");
		delete u;
	} else {
		cs.leave();
	}

	removeConnection(aSource);
}

void UploadManager::onTransmitDone(UserConnection* aSource) {
	Upload* u;

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
	dcdebug("onTransmitDone: Removing upload\n");
	uploads.erase(i);

	if(isExtra(u)) {
		extra--;
	} else {
		running--;
	}
	
	cs.leave();

	fire(UploadManagerListener::COMPLETE, u);
	delete u;
	
}

/**
 * @file UploadManger.cpp
 * $Id: UploadManager.cpp,v 1.11 2002/01/22 00:10:37 arnetheduck Exp $
 * @if LOG
 * $Log: UploadManager.cpp,v $
 * Revision 1.11  2002/01/22 00:10:37  arnetheduck
 * Version 0.132, removed extra slots feature for nm dc users...and some bug
 * fixes...
 *
 * Revision 1.10  2002/01/20 22:54:46  arnetheduck
 * Bugfixes to 0.131 mainly...
 *
 * Revision 1.9  2002/01/19 19:07:39  arnetheduck
 * Last fixes before 0.13
 *
 * Revision 1.8  2002/01/19 13:09:10  arnetheduck
 * Added a file class to hide ugly file code...and fixed a small resume bug (I think...)
 *
 * Revision 1.7  2002/01/17 23:35:59  arnetheduck
 * Reworked threading once more, now it actually seems stable. Also made
 * sure that noone tries to access client objects that have been deleted
 * as well as some other minor updates
 *
 * Revision 1.6  2002/01/16 20:56:27  arnetheduck
 * Bug fixes, file listing sort and some other small changes
 *
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

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
#include "ConnectionManager.h"

UploadManager* UploadManager::instance = NULL;

void UploadManager::onGet(UserConnection* aSource, const string& aFile, LONGLONG aResume) {
	if(aSource->getState() != UserConnection::STATE_GET) {
		dcdebug("UM:onGet Wrong state, ignoring\n");
		return;
	}
	
	Upload* u;
	dcassert(aFile.size() > 0);
	
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

	if( File::getSize(file) < (LONGLONG)(16 * 1024) ) {
		smallfile = true;
	}

	cs.enter();
	map<User::Ptr, DWORD>::iterator ui = reservedSlots.find(aSource->getUser());

	if( (!aSource->isSet(UserConnection::FLAG_HASSLOT)) && 
		(getFreeSlots()<=0) && 
		(ui == reservedSlots.end()) ) {

		if( !(	(smallfile || userlist) && 
				( (aSource->isSet(UserConnection::FLAG_HASEXTRASLOT)) || (getFreeExtraSlots() > 0) ) && 
				(aSource->getUser()->isSet(User::DCPLUSPLUS)) 
			) ) {
			
			cs.leave();
			aSource->maxedOut();
			removeConnection(aSource);
			return;

		}
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

	File* f;
	try {
		f = new File(file, File::READ, File::OPEN);
	} catch(FileException e) {
		cs.leave();
		aSource->error("File Not Available");
		return;
	}
	
	u = new Upload(ConnectionManager::getInstance()->getQueueItem(aSource));
	u->setFile(f);
	u->setSize(f->getSize());
	u->setPos(aResume, true);
	u->setUser(aSource->getUser());
	u->setFileName(aFile);
	if(smallfile)
		u->setFlag(Upload::SMALL_FILE);
	if(userlist)
		u->setFlag(Upload::USER_LIST);

	aSource->setStatus(UserConnection::BUSY);
	dcassert(aSource->getUpload() == NULL);
	aSource->setUpload(u);
	uploads.push_back(u);
	if(!aSource->isSet(UserConnection::FLAG_HASSLOT)) {
		if(ui != reservedSlots.end()) {
			aSource->setFlag(UserConnection::FLAG_HASSLOT);
			running++;
			reservedSlots.erase(ui);
		} else {
			if( (getFreeSlots() == 0) && (smallfile || userlist)) {
				if(!aSource->isSet(UserConnection::FLAG_HASEXTRASLOT)) {
					extra++;
					aSource->setFlag(UserConnection::FLAG_HASEXTRASLOT);
				}
			} else {
				running++;
				aSource->setFlag(UserConnection::FLAG_HASSLOT);
			}
		}
	}
	
	if(aSource->isSet(UserConnection::FLAG_HASEXTRASLOT) && aSource->isSet(UserConnection::FLAG_HASSLOT)) {
		extra--;
		aSource->unsetFlag(UserConnection::FLAG_HASEXTRASLOT);
	}
	
	aSource->setState(UserConnection::STATE_SEND);
	cs.leave();

	aSource->fileLength(Util::toString(u->getSize()));

}

void UploadManager::onSend(UserConnection* aSource) {
	if(aSource->getState() != UserConnection::STATE_SEND) {
		dcdebug("UM::onSend Bad state, ignoring\n");
		return;
	}

	Upload* u = aSource->getUpload();
	dcassert(u != NULL);

	u->setStart(TimerManager::getTick());
	aSource->setState(UserConnection::STATE_DONE);
	aSource->transmitFile(u->getFile());
	fire(UploadManagerListener::STARTING, u);
}

void UploadManager::onBytesSent(UserConnection* aSource, DWORD aBytes) {
	dcassert(aSource->getState() == UserConnection::STATE_DONE);
	Upload* u = aSource->getUpload();
	dcassert(u != NULL);
	u->addPos(aBytes);
}

void UploadManager::onFailed(UserConnection* aSource, const string& aError) {
	Upload* u = aSource->getUpload();

	if(u) {
		aSource->setUpload(NULL);
		fire(UploadManagerListener::FAILED, u, aError);
		{
			Lock l(cs);
			dcassert(find(uploads.begin(), uploads.end(), u) != uploads.end());
			uploads.erase(find(uploads.begin(), uploads.end(), u));
		}

		dcdebug("UM::onFailed: Removing upload\n");
		delete u;
	}

	removeConnection(aSource);
}

void UploadManager::onTransmitDone(UserConnection* aSource) {
	dcassert(aSource->getState() == UserConnection::STATE_DONE);
	Upload* u = aSource->getUpload();
	dcassert(u != NULL);

	{
		Lock l(cs);
		dcassert(find(uploads.begin(), uploads.end(), u) != uploads.end());
		uploads.erase(find(uploads.begin(), uploads.end(), u));
	}
	
	aSource->setUpload(NULL);
	aSource->setStatus(UserConnection::IDLE);
	aSource->setState(UserConnection::STATE_GET);

	fire(UploadManagerListener::COMPLETE, u);
	delete u;
	
}

void UploadManager::removeConnection(UserConnection::Ptr aConn) {
	dcassert(aConn->getUpload() == NULL);
	aConn->removeListener(this);

	{
		Lock l(cs);
		
	
		UserConnection::Iter i = find(connections.begin(), connections.end(), aConn);
		dcassert(i != connections.end());
		connections.erase(i);
		
		if(aConn->isSet(UserConnection::FLAG_HASSLOT)) {
			running--;
			aConn->unsetFlag(UserConnection::FLAG_HASSLOT);
		} 
		if(aConn->isSet(UserConnection::FLAG_HASEXTRASLOT)) {
			extra--;
			aConn->unsetFlag(UserConnection::FLAG_HASEXTRASLOT);
		}
	}
	ConnectionManager::getInstance()->putUploadConnection(aConn);
	
}

void UploadManager::removeConnections() {
	
	UserConnection::List tmp;
	{
		Lock l(cs);
		tmp = connections;
		connections.clear();
	}
	
	for(UserConnection::Iter i = tmp.begin(); i != tmp.end(); ++i) {
		(*i)->removeListener(this);
		ConnectionManager::getInstance()->putUploadConnection(*i);
	}
}

void UploadManager::onTimerMinute(DWORD aTick) {
	Lock l(cs);
	for(map<User::Ptr, DWORD>::iterator j = reservedSlots.begin(); j != reservedSlots.end();) {
		if(j->second + 600 * 1000 < aTick) {
			reservedSlots.erase(j++);
		} else {
			++j;
		}
	}
}	

/**
 * @file UploadManger.cpp
 * $Id: UploadManager.cpp,v 1.21 2002/03/04 23:52:31 arnetheduck Exp $
 * @if LOG
 * $Log: UploadManager.cpp,v $
 * Revision 1.21  2002/03/04 23:52:31  arnetheduck
 * Updates and bugfixes, new user handling almost finished...
 *
 * Revision 1.20  2002/02/28 00:10:47  arnetheduck
 * Some fixes to the new user model
 *
 * Revision 1.19  2002/02/27 12:02:09  arnetheduck
 * Completely new user handling, wonder how it turns out...
 *
 * Revision 1.18  2002/02/25 15:39:29  arnetheduck
 * Release 0.154, lot of things fixed...
 *
 * Revision 1.17  2002/02/18 23:48:32  arnetheduck
 * New prerelease, bugs fixed and features added...
 *
 * Revision 1.16  2002/02/09 18:13:51  arnetheduck
 * Fixed level 4 warnings and started using new stl
 *
 * Revision 1.15  2002/02/03 01:06:56  arnetheduck
 * More bugfixes and some minor changes
 *
 * Revision 1.14  2002/02/02 17:21:27  arnetheduck
 * Fixed search bugs and some other things...
 *
 * Revision 1.13  2002/02/01 02:00:45  arnetheduck
 * A lot of work done on the new queue manager, hopefully this should reduce
 * the number of crashes...
 *
 * Revision 1.12  2002/01/25 00:11:26  arnetheduck
 * New settings dialog and various fixes
 *
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

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

#include "DownloadManager.h"
#include "ConnectionManager.h"
#include "Client.h"
#include "User.h"
#include "ClientManager.h"
#include "SimpleXML.h"

DownloadManager* DownloadManager::instance = NULL;

static const string USER_LIST_NAME = "MyList.DcLst";

/**
 * Each minute we check whether any of the users in the download queue have gone offline or connected
 */
void DownloadManager::onTimerMinute(DWORD aTick) {
	cs.enter();

	for(Download::Iter i = queue.begin(); i != queue.end(); ++i) {
		Download* d = *i;

		if(d->isSet(Download::RUNNING)) {
			continue;
		}
		bool online = false;
		for(Download::Source::Iter j = d->getSources().begin(); j != d->getSources().end(); ++j) {
			Download::Source* s = *j;

			// Check if we've got a user pointer at all
			if(!s->getUser()) {
				s->setUser(ClientManager::getInstance()->findUser(s->getNick()));
				
				if(!s->getUser()) {
					// Still no user, go on to the next one...
					continue;
				}
				fire(DownloadManagerListener::SOURCE_UPDATED, d, s);
			}
			
			// Check if the user is still online
			if(!s->getUser()->isOnline()) {
				bool found = false;
				for(UserConnection::Iter k = connections.begin(); k != connections.end(); ++k) {
					if((*k)->getUser() == s->getUser()) {
						found = true;
						online = true;
					}
				}
				if(!found) {
					// Damn, we've lost him...check if he's/she's reconnected...
					s->setUser(ClientManager::getInstance()->findUser(s->getNick()));
					fire(DownloadManagerListener::SOURCE_UPDATED, d, s);
					if(!s->getUser()) {
						continue;
					}
				}
			}

			online = true;

			// Check if we already have a connection to this fellow...
			bool found = false;
			for(UserConnection::Iter k = connections.begin(); k != connections.end(); ++k) {
				if((*k)->getUser() == s->getUser()) {
					found = true;
					break;
				}
			}

			if(found) {
				continue;
			}

			// Alright, we've made it this far, add the user to the waiting queue (unless he's there already...)
			map<User::Ptr, DWORD>::iterator i = waiting.find(s->getUser());
			if(i == waiting.end()) {
				waiting[s->getUser()] = 0;
			}
		}

		if(!online) {
			if(d->getSources().size() > 1) {
				fire(DownloadManagerListener::FAILED, d, "All users offline");
			} else {
				fire(DownloadManagerListener::FAILED, d, "User is offline");
			}
		}
	}
	cs.leave();
}

void DownloadManager::onTimerSecond(DWORD aTick) {
	cs.enter();

	map<User::Ptr, DWORD>::iterator i = waiting.begin();

	// Check on the users we're waiting for...
	while(i != waiting.end()) {
		// Check if something's happened the last 60 seconds...
		if(i->second + 60*1000 < aTick) {
			// Update the timer
			i->second = aTick;

			Download* d = getNextDownload(i->first);
			
			if(d && i->first->isOnline()) {
				int status = ConnectionManager::getInstance()->getDownloadConnection(i->first);
				if(status==UserConnection::CONNECTING) {
					fire(DownloadManagerListener::CONNECTING, d);
				} else if(status == UserConnection::FREE) {
					// Alright, the connection was reused, so the waiting pool might have changed...try again...
					i = waiting.begin();
					continue;
				}
				
			} else {
				// Duuh...user has gone offline or there's nothing more for him/her, remove from the waiting list...
				i = waiting.erase(i);
				continue;
			}
		}
		++i;
	}

	// Tick each ongoing download
	for(Download::MapIter m = running.begin(); m != running.end(); ++m) {
		if(m->second->getPos() > 0) {
			fire(DownloadManagerListener::TICK, m->second);
		}
	}
	cs.leave();
}

void DownloadManager::connectFailed(const User::Ptr& aUser) {
	Lock l(cs);

	map<User::Ptr, DWORD>::iterator i = waiting.find(aUser);
	if(i != waiting.end()) {
		Download* d = getNextDownload(aUser);
		if(d) {
			fire(DownloadManagerListener::FAILED, d, "Connection timeout");
			return;
		}
	}
}

/**
 * Add a file to the download queue. When added, a connection attempt will automatically be
 * made, unless there is an existing connection to the user specified that is busy.
 * Note; make sure that there is a point in asking for a download, i.e. if the target file
 * has the same size as a download, the other client will complain that there's nothing to
 * send...
 * @param aFile Filename and path at server.
 * @param aSize Size of file, set to -1 if unknown.
 * @param aUser Pointer to a _connected_ user.
 * @param aTarget Target location of a file.
 * @param aResume Try to resume download if possible (not recommended for MyList.DcLst).
 */
void DownloadManager::download(const string& aFile, LONGLONG aSize, const User::Ptr& aUser, const string& aTarget, bool aResume /* = true /*/) throw(DownloadException) {

	cs.enter();
	
	// First, search the queue for the same download...
	for(Download::Iter i = queue.begin(); i != queue.end(); ++i) {
		Download* dd = *i;
		if(strcmpi(dd->getTarget().c_str(), aTarget.c_str()) == 0) {
			if(dd->getSize() != aSize) {
				// Same target, but different sizes...not good...
				cs.leave();
				throw DownloadException("A download with the same target but different size already exists in the queue");
			}

			if(!dd->getSources().empty() && dd->getSources()[0]->getFileName() == USER_LIST_NAME) {
				// Only one source for user listings...
				cs.leave();
				throw DownloadException("Already downloading that user's file list");
			}

			// Same download it seems, add this user / path as a source
			if(!dd->isSource(aUser)) {
				string fileName, path;
				
				if(aFile.find('\\')) {
					fileName = aFile.substr(aFile.rfind('\\')+1);
					path = aFile.substr(0, aFile.rfind('\\')+1);
				} else {
					fileName = aFile;
				}
				SettingsManager::getInstance()->save();
				fire(DownloadManagerListener::SOURCE_ADDED, dd, dd->addSource(aUser, fileName, path));
			}
			
			if(dd->isSet(Download::RUNNING)) {
				// Ignore it for now
				cs.leave();
				return;
			}

			if(waiting.find(aUser) == waiting.end()) {
				bool found = false;
				for(UserConnection::Iter i = connections.begin(); i != connections.end(); ++i) {
					if((*i)->getUser() == aUser) {
						found = true;
					}
				}
				if(!found) {
					waiting[aUser] = 0;
				}
			}
			cs.leave();
			return;
		}
	}

	// No such download, check if the target file is smaller than the one being downloaded...
	if(aResume && (aSize != -1) ) {
		if(Util::getFileSize(aTarget) >= aSize) {
			cs.leave();
			throw DownloadException("Target file is larger than the source");
		}
	}

	Download* d = new Download();
	string fileName, path;

	if(aFile.find('\\')) {
		fileName = aFile.substr(aFile.rfind('\\')+1);
		path = aFile.substr(0, aFile.rfind('\\')+1);
	} else {
		fileName = aFile;
	}
	Download::Source::Ptr s = d->addSource(aUser, fileName, path);

	if(aFile == USER_LIST_NAME)
		d->setFlag(Download::USER_LIST); 

	d->setTarget(aTarget);
	d->setSize(aSize);
	if(aResume)
		d->setFlag(Download::RESUME);

	queue.push_back(d);
	
	if(waiting.find(aUser) == waiting.end()) {
		bool found = false;
		for(UserConnection::Iter i = connections.begin(); i != connections.end(); ++i) {
			if((*i)->getUser() == aUser) {
				found = true;
			}
		}
		if(!found) {
			waiting[aUser] = 0;
		}
	}
	
	cs.leave();
	SettingsManager::getInstance()->save();
	fire(DownloadManagerListener::ADDED, d);
	fire(DownloadManagerListener::SOURCE_ADDED, d, s);
	
}

/**
 * Add a file to the download queue. Useful if it is not certain that the user is connected.
 * If he/she is, it will be added as usual, otherwise it will be put on the queue and the 
 * DownloadManager will hopefully continue downloading it later on...
 * @param aFile Filename and path at server.
 * @param aSize Size of file, set to -1 if unknown.
 * @param aUser Pointer to a _connected_ user.
 * @param aTarget Target location of a file.
 * @param aResume Try to resume download if possible (not recommended for MyList.DcLst).
 */
void DownloadManager::download(const string& aFile, LONGLONG aSize, const string& aUser, const string& aTarget, bool aResume /* = true /*/) throw(DownloadException) {

	User::Ptr& user = ClientManager::getInstance()->findUser(aUser);
	if(user) {
		download(aFile, aSize, user, aTarget, aResume);
		return;
	}

	// We don't know who this user is, so we just add it to the list...
	cs.enter();
	
	for(Download::Iter i = queue.begin(); i != queue.end(); ++i) {
		Download* dd = *i;
		if(strcmpi(dd->getTarget().c_str(), aTarget.c_str()) == 0) {
			if(dd->getSize() != aSize) {
				// Same target, but different sizes...not good...
				cs.leave();
				throw DownloadException("A file of different size but same target already exists in the queue");
			}
			
			if(!dd->getSources().empty() && dd->getSources()[0]->getFileName() == USER_LIST_NAME) {
				// Only one source for user listings...
				cs.leave();
				throw DownloadException("Already downloading that user's file list");
			}
			
			// Same download it seems, add this user / path as a source
			if(!dd->isSource(aUser)) {
				string fileName, path;
				
				if(aFile.find('\\')) {
					fileName = aFile.substr(aFile.rfind('\\')+1);
					path = aFile.substr(0, aFile.rfind('\\')+1);
				} else {
					fileName = aFile;
				}
				SettingsManager::getInstance()->save();
				fire(DownloadManagerListener::SOURCE_ADDED, dd, dd->addSource(aUser, fileName, path));
			}
			
			cs.leave();
			return;
		}
	}
	
	// No such download, check if the target file is smaller than the one being downloaded...
	if(aResume && (aSize != -1) ) {
		if(Util::getFileSize(aTarget) >= aSize) {
			cs.leave();
			throw DownloadException(aTarget + ": A file of larger or equal size already exists");
		}
	}
	
	Download* d = new Download();

	string fileName, path;
	
	if(aFile.find('\\')) {
		fileName = aFile.substr(aFile.rfind('\\')+1);
		path = aFile.substr(0, aFile.rfind('\\')+1);
	} else {
		fileName = aFile;
	}
	Download::Source::Ptr s = d->addSource(aUser, fileName, path);

	if(aFile == USER_LIST_NAME)
		d->setFlag(Download::USER_LIST); 

	d->setTarget(aTarget);
	d->setSize(aSize);
	if(aResume)
		d->setFlag(Download::RESUME);

	queue.push_back(d);
	cs.leave();

	SettingsManager::getInstance()->save();
	fire(DownloadManagerListener::ADDED, d);
	fire(DownloadManagerListener::SOURCE_ADDED, d, s);

}

void DownloadManager::downloadList(const User::Ptr& aUser) throw(DownloadException) {
	string file = Util::getAppPath() + aUser->getNick() + ".DcLst";
	download(USER_LIST_NAME, -1, aUser, file, false);
	userLists.push_back(file);
}

void DownloadManager::downloadList(const string& aUser) throw(DownloadException) {
	string file = Util::getAppPath() + aUser + ".DcLst";
	download(USER_LIST_NAME, -1, aUser, file, false);
	userLists.push_back(file);
}

void DownloadManager::removeDownload(Download* aDownload) {
	{
		Lock l(cs);
		
		// Check the running downloads...
		for(Download::MapIter j = running.begin(); j != running.end(); ++j) {
			if(j->second == aDownload) {
				// This is worse, we have to abort the download...
				UserConnection* conn = j->first;
				running.erase(j);
				removeConnection(conn);
				break;
			}
		}
		
		// Search the queue
		Download::Iter i = find(queue.begin(), queue.end(), aDownload);
		dcassert(i != queue.end());
		
		queue.erase(i);
	}

	SettingsManager::getInstance()->save();
	
	fire(DownloadManagerListener::REMOVED, aDownload);
	delete aDownload;

}

void DownloadManager::removeConnection(UserConnection::Ptr aConn, bool reuse /* = false */) {
	{
		Lock l(cs);

		dcassert(find(connections.begin(), connections.end(), aConn) != connections.end());
		connections.erase(find(connections.begin(), connections.end(), aConn));
	}

	aConn->removeListener(this);
	ConnectionManager::getInstance()->putDownloadConnection(aConn, reuse);
	
}

void DownloadManager::removeConnections() {

	UserConnection::List tmp;

	{
		Lock l(cs);
		tmp = connections;
		connections.clear();
	}

	for(UserConnection::Iter i = tmp.begin(); i != tmp.end(); ++i) {
		UserConnection* c = *i;
		c->removeListener(this);
		ConnectionManager::getInstance()->putDownloadConnection(c);
	}
}

void DownloadManager::checkDownloads(UserConnection* aConn) {
	dcdebug("Checking downloads...");
	
	// If the user is offline, check if he's maybe back online, and change the user pointer if that is the case...
	if( !aConn->getUser()->isOnline() ) {
		ConnectionManager::getInstance()->updateUser(aConn);
	}

	cs.enter();
	Download* d = getNextDownload(aConn->getUser());
	
	if(d) {
		running[aConn] = d;
		d->setFlag(Download::RUNNING);

		cs.leave();


		Download::Source* s = d->getSource(aConn->getUser());
		dcassert(s);
		
		d->setCurrentSource(s);
		
		if(d->isSet(Download::RESUME)) {
			LONGLONG x = Util::getFileSize(d->getTarget());
			if(x < (LONGLONG)SETTING(ROLLBACK)) {
				d->setPos(0);
			} else {
				d->setPos(x - (LONGLONG)SETTING(ROLLBACK));
				d->setFlag(Download::ROLLBACK);
			}
			
		} else {
			d->setPos(0);
		}
		
		aConn->get(s->getPath()+s->getFileName(), d->getPos());
		dcdebug("Found!\n");
		return;
	}
	// Connection not needed any more, return it to the ConnectionManager...
	dcdebug("Not found!\n");
	
	// No more downloads for this user, make sure we're not waiting for a connection...
	waiting.erase(aConn->getUser());
	cs.leave();

	removeConnection(aConn, true);
}

void DownloadManager::removeSource(Download* aDownload, Download::Source::Ptr aSource) {
	cs.enter();
	if(aDownload->isSet(Download::RUNNING) && aSource == aDownload->getCurrentSource()) {
		// We have to abort the download...
		for(Download::MapIter i = running.begin(); i != running.end(); ++i) {
			if(i->second == aDownload) {
				// Good, found it...
				UserConnection* uc = i->first;
				running.erase(i);
				removeConnection(uc);
				aDownload->unsetFlag(Download::RUNNING);
				aDownload->setCurrentSource(NULL);
				fire(DownloadManagerListener::FAILED, aDownload, "User removed");
				break;
			}
		}
	}

	if(aDownload->isSet(Download::USER_LIST)) {
		Download::Iter i = find(queue.begin(), queue.end(), aDownload);
		if(i != queue.end()) {
			queue.erase(i);
			cs.leave();
			SettingsManager::getInstance()->save();
			fire(DownloadManagerListener::REMOVED, aDownload);
			delete aDownload;
		}
	} else {
		aDownload->removeSource(aSource);
		cs.leave();
		SettingsManager::getInstance()->save();
		fire(DownloadManagerListener::SOURCE_REMOVED, aDownload, aSource);
	}
}

void DownloadManager::onData(UserConnection* aSource, const BYTE* aData, int aLen) {
	cs.enter();
	Download::MapIter i = running.find(aSource);
	if(i != running.end()) {
		Download* d = i->second;

		if(d->isSet(Download::ROLLBACK)) {
			dcassert(d->getRollbackBuffer());
			if(d->getTotal() + aLen >= d->getRollbackSize()) {
				BYTE* buf = new BYTE[d->getRollbackSize()];
				
				memcpy(d->getRollbackBuffer() + d->getTotal(), aData, d->getRollbackSize() - d->getTotal());

				d->getFile()->read(buf, d->getRollbackSize());

				if(memcmp(d->getRollbackBuffer(), buf, d->getRollbackSize()) != 0) {
					// We have a problem...
					cs.leave();

					d->unsetFlag(Download::ROLLBACK);
					d->setRollbackBuffer(0);
					
					failDownload(aSource, d, "Rollback discovered resume inconsistency, removing download source from the queue");
					
					delete buf;
					return;
				}

				// Alright, write the rest...the file pointer should have been moved to the correct position by now...
				try {
					d->getFile()->write(aData+d->getRollbackSize(), aLen - d->getRollbackSize());
				} catch(FileException e) {
					cs.leave();

					d->unsetFlag(Download::ROLLBACK);
					d->setRollbackBuffer(0);
					
					failDownload(aSource, d, "File write failed: " + e.getError());

					delete buf;
					return;
				}

				d->unsetFlag(Download::ROLLBACK);
				d->setRollbackBuffer(0);
				delete buf;

			} else {
				memcpy(d->getRollbackBuffer() + d->getTotal(), aData, aLen - d->getTotal());
			}
			cs.leave();
		} else {
			cs.leave();

			try {
				d->getFile()->write(aData, aLen);
			} catch(FileException e) {
				failDownload(aSource, d, "File write failed: " + e.getError());
				return;
			}
		}
		
		d->addPos(aLen);
	}
}

void DownloadManager::onFileLength(UserConnection* aSource, const string& aFileLength) {

	LONGLONG fileLength = Util::toInt64(aFileLength);

	cs.enter();
	Download::MapIter i = running.find(aSource);
	if(i != running.end()) {
		Download* d = i->second;
		
		Util::ensureDirectory(d->getTarget());

		File* file;
		try {
			file = new File(d->getTarget(), File::WRITE | File::READ, File::OPEN | File::CREATE | (d->isSet(Download::RESUME) ? 0 : File::TRUNCATE));
		} catch(FileException e) {
			running.erase(i);
			d->unsetFlag(Download::RUNNING);
			cs.leave();
			removeConnection(aSource);
			fire(DownloadManagerListener::FAILED, d, "Could not open target file:" + e.getError());
			return;
		}
			
		d->setFile(file, true);

		if(d->isSet(Download::RESUME) && d->isSet(Download::ROLLBACK)) {
			if(fileLength > SETTING(ROLLBACK)) {
				d->setPos(file->getSize() - SETTING(ROLLBACK), true);
				d->setRollbackBuffer(SETTING(ROLLBACK));
			} else {
				d->setPos(0, true);
				d->setRollbackBuffer(fileLength);
			}
		}

		d->setSize(fileLength);
		
		if(d->getSize() == d->getPos()) {
			Download::Iter j = find(queue.begin(), queue.end(), d);
			dcassert(j != queue.end());
			queue.erase(j);
			
			cs.leave();
			removeConnection(aSource);
			
			// We're done...and this connection is broken...
			SettingsManager::getInstance()->save();
			fire(DownloadManagerListener::COMPLETE, d);
			fire(DownloadManagerListener::REMOVED, d);
			delete d;
			
		} else {
			cs.leave();
			d->setStart(TimerManager::getTick());
			fire(DownloadManagerListener::STARTING, d);
			
			aSource->setDataMode(d->getSize() - d->getPos());
			aSource->startSend();
		}
	}
}

/** Download finished! */
void DownloadManager::onModeChange(UserConnection* aSource, int aNewMode) {
	cs.enter();

	Download::MapIter i = running.find(aSource);
	if(i != running.end()) {
		Download::Ptr p = i->second;
		running.erase(i);
		Download::Iter j = find(queue.begin(), queue.end(), p);
		dcassert(j != queue.end());
		queue.erase(j);
		
		cs.leave();
		
		if(p->getPos() != p->getSize())
			dcdebug("Download incomplete??? : ");
		
		p->setFile(NULL);
		
		SettingsManager::getInstance()->save();
		dcdebug("Download finished: %s, size %I64d\n", p->getTarget().c_str(), p->getSize());
		fire(DownloadManagerListener::COMPLETE, p);
		fire(DownloadManagerListener::REMOVED, p);
		delete p;
		
		checkDownloads(aSource);
	}
	
}

void DownloadManager::onMaxedOut(UserConnection* aSource) { 
	Download* d;

	{
		Lock l(cs);

		Download::MapIter i = running.find(aSource);
		dcassert(i != running.end());
		
		d = i->second;
		
		running.erase(i);
		d->unsetFlag(Download::RUNNING);
		waiting[aSource->getUser()] = TimerManager::getTick();
	}

	fire(DownloadManagerListener::FAILED, d, "No slots available");
	removeConnection(aSource);
}

void DownloadManager::onFailed(UserConnection* aSource, const string& aError) {
	cs.enter();
	Download::MapIter i = running.find(aSource);
	
	if(i == running.end()) {
		cs.leave();
		removeConnection(aSource);
		return;
	}

	Download* d = i->second;
	running.erase(i);
	d->unsetFlag(Download::RUNNING);
	d->unsetFlag(Download::ROLLBACK); // Just in case
	d->resetTotal();
	cs.leave();
	
	d->setFile(NULL);
	fire(DownloadManagerListener::FAILED, d, aError);
	d->setCurrentSource(NULL);
	removeConnection(aSource);
}

void DownloadManager::save(SimpleXML* aXml) {
	Lock l(cs);

	aXml->addTag("Downloads");
	aXml->stepIn();
	for(Download::Iter i = queue.begin(); i != queue.end(); ++i) {
		Download::Ptr d = *i;
		if(!d->isSet(Download::USER_LIST)) {
			aXml->addTag("Download");
			aXml->addChildAttrib("Target", d->getTarget());
			aXml->addChildAttrib("Resume", d->isSet(Download::RESUME));
			aXml->addChildAttrib("Size", d->getSize());

			aXml->stepIn();
			for(Download::Source::List::const_iterator j = d->getSources().begin(); j != d->getSources().end(); ++j) {
				Download::Source* s = *j;
				aXml->addTag("Source");
				aXml->addChildAttrib("Nick", s->getNick());
				aXml->addChildAttrib("Path", s->getPath());
				aXml->addChildAttrib("FileName", s->getFileName());
			}
			aXml->stepOut();

		}
	}
	aXml->stepOut();
}

void DownloadManager::load(SimpleXML* aXml) {
	Lock l(cs);
	aXml->resetCurrentChild();
	if(aXml->findChild("Downloads")) {
		aXml->stepIn();

		while(aXml->findChild("Download")) {
			const string& target = aXml->getChildAttrib("Target");
			bool resume = aXml->getBoolChildAttrib("Resume");
			LONGLONG size = aXml->getLongLongChildAttrib("Size");
			aXml->stepIn();
			while(aXml->findChild("Source")) {
				const string& nick = aXml->getChildAttrib("Nick");
				const string& path = aXml->getChildAttrib("Path");
				const string& file = aXml->getChildAttrib("FileName");
				try {
					download(path + file, size, nick, target, resume);
				} catch(...) {
					// ...
				}
			}
			aXml->stepOut();
		}
		aXml->stepOut();
	}
}

/**
 * @file DownloadManger.cpp
 * $Id: DownloadManager.cpp,v 1.37 2002/01/22 00:10:37 arnetheduck Exp $
 * @if LOG
 * $Log: DownloadManager.cpp,v $
 * Revision 1.37  2002/01/22 00:10:37  arnetheduck
 * Version 0.132, removed extra slots feature for nm dc users...and some bug
 * fixes...
 *
 * Revision 1.36  2002/01/20 22:54:46  arnetheduck
 * Bugfixes to 0.131 mainly...
 *
 * Revision 1.35  2002/01/19 19:07:39  arnetheduck
 * Last fixes before 0.13
 *
 * Revision 1.34  2002/01/19 13:09:10  arnetheduck
 * Added a file class to hide ugly file code...and fixed a small resume bug (I think...)
 *
 * Revision 1.33  2002/01/18 17:41:43  arnetheduck
 * Reworked many right button menus, adding op commands and making more easy to use
 *
 * Revision 1.32  2002/01/17 23:35:59  arnetheduck
 * Reworked threading once more, now it actually seems stable. Also made
 * sure that noone tries to access client objects that have been deleted
 * as well as some other minor updates
 *
 * Revision 1.31  2002/01/15 00:41:54  arnetheduck
 * late night fixes...
 *
 * Revision 1.30  2002/01/14 22:19:43  arnetheduck
 * Commiting minor bugfixes
 *
 * Revision 1.29  2002/01/13 22:50:48  arnetheduck
 * Time for 0.12, added favorites, a bunch of new icons and lot's of other stuff
 *
 * Revision 1.28  2002/01/11 14:52:57  arnetheduck
 * Huge changes in the listener code, replaced most of it with templates,
 * also moved the getinstance stuff for the managers to a template
 *
 * Revision 1.27  2002/01/08 00:24:10  arnetheduck
 * Last bugs fixed before 0.11
 *
 * Revision 1.26  2002/01/07 23:05:48  arnetheduck
 * Resume rollback implemented
 *
 * Revision 1.25  2002/01/06 00:14:54  arnetheduck
 * Incoming searches almost done, just need some testing...
 *
 * Revision 1.24  2002/01/05 19:06:09  arnetheduck
 * Added user list images, fixed bugs and made things more effective
 *
 * Revision 1.22  2002/01/05 10:13:39  arnetheduck
 * Automatic version detection and some other updates
 *
 * Revision 1.21  2002/01/02 16:55:56  arnetheduck
 * Time for 0.09
 *
 * Revision 1.20  2002/01/02 16:12:32  arnetheduck
 * Added code for multiple download sources
 *
 * Revision 1.19  2001/12/30 17:41:16  arnetheduck
 * Fixed some XML parsing bugs
 *
 * Revision 1.18  2001/12/29 13:47:14  arnetheduck
 * Fixing bugs and UI work
 *
 * Revision 1.17  2001/12/21 23:52:30  arnetheduck
 * Last commit for five days
 *
 * Revision 1.16  2001/12/21 20:21:17  arnetheduck
 * Private messaging added, and a lot of other updates as well...
 *
 * Revision 1.15  2001/12/19 23:07:59  arnetheduck
 * Added directory downloading from the directory tree (although it hasn't been
 * tested at all) and password support.
 *
 * Revision 1.14  2001/12/18 12:32:18  arnetheduck
 * Stability fixes
 *
 * Revision 1.13  2001/12/16 19:47:48  arnetheduck
 * Reworked downloading and user handling some, and changed some small UI things
 *
 * Revision 1.12  2001/12/15 17:01:06  arnetheduck
 * Passive mode searching as well as some searching code added
 *
 * Revision 1.11  2001/12/13 19:21:57  arnetheduck
 * A lot of work done almost everywhere, mainly towards a friendlier UI
 * and less bugs...time to release 0.06...
 *
 * Revision 1.10  2001/12/11 01:10:29  arnetheduck
 * More bugfixes...I really have to change the bufferedsocket so that it only
 * uses one thread...or maybe even multiple sockets/thread...
 *
 * Revision 1.9  2001/12/10 10:48:40  arnetheduck
 * Ahh, finally found one bug that's been annoying me for days...=) the connections
 * in the pool were not reset correctly before being put back for later use...
 *
 * Revision 1.8  2001/12/07 20:03:06  arnetheduck
 * More work done towards application stability
 *
 * Revision 1.7  2001/12/05 14:27:35  arnetheduck
 * Premature disconnection bugs removed.
 *
 * Revision 1.6  2001/12/04 21:50:34  arnetheduck
 * Work done towards application stability...still a lot to do though...
 * a bit more and it's time for a new release.
 *
 * Revision 1.5  2001/12/02 23:47:35  arnetheduck
 * Added the framework for uploading and file sharing...although there's something strange about
 * the file lists...my client takes them, but not the original...
 *
 * Revision 1.4  2001/12/01 17:15:03  arnetheduck
 * Added a crappy version of huffman encoding, and some other minor changes...
 *
 * Revision 1.3  2001/11/29 19:10:54  arnetheduck
 * Refactored down/uploading and some other things completely.
 * Also added download indicators and download resuming, along
 * with some other stuff.
 *
 * Revision 1.2  2001/11/26 23:40:36  arnetheduck
 * Downloads!! Now downloads are possible, although the implementation is
 * likely to change in the future...more UI work (splitters...) and some bug
 * fixes. Only user file listings are downloadable, but at least it's something...
 *
 * Revision 1.1  2001/11/25 22:06:25  arnetheduck
 * Finally downloading is working! There are now a few quirks and bugs to be fixed
 * but what the heck....!
 *
 * @endif
 */

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

#include "DownloadManager.h"
#include "ConnectionManager.h"
#include "Client.h"
#include "User.h"

DownloadManager* DownloadManager::instance = NULL;

void DownloadManager::onTimerMinute(DWORD aTick) {
	cs.enter();
	
	for(Download::Iter i = queue.begin(); i != queue.end(); ++i) {
		Download* d = *i;
		User::Ptr& u = d->getUser();
		
		if(!u) {
			// Let's see if the user maybe connected...
			u = Client::findUser(d->getLastNick());
			if(!u) {
				continue;
			}	
		}


		if(waiting.find(u) != waiting.end()) {
			continue;
		}

		if(u->isOnline()) {
			if(ConnectionManager::getInstance()->getDownloadConnection(u)==UserConnection::CONNECTING) {
				waiting[u] = d;
				fireConnecting(d);
			}
		} else {
			// We need to check if we already have a connection to this user
			bool found = false;
			for(UserConnection::Iter i = connections.begin(); i != connections.end(); ++i) {
				if((*i)->getUser() == u) {
					found = true;
				}
			}

			if(!found) {
				d->setUser(User::nuser);
				fireFailed(d, d->getLastNick() + " has gone offline");
			}
		}
	}
	cs.leave();
	
}

void DownloadManager::connectFailed(const User::Ptr& aUser) {
	Download::UserIter i = waiting.find(aUser);
	if(i != waiting.end()) {
		if(!aUser->isOnline()) {
			i->second->setUser(User::nuser);
			fireFailed(i->second, "Connection timeout, user disconnected");
		} else {
			fireFailed(i->second, "Connection timeout, user not responding");
		}
		waiting.erase(i);
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
void DownloadManager::download(const string& aFile, LONGLONG aSize, User::Ptr& aUser, const string& aTarget, bool aResume /* = true /*/) {
	Download* d = NULL;

	cs.enter();
	
	for(Download::Iter i = queue.begin(); i != queue.end(); ++i) {
		// First, search the queue for the same download...
		Download* dd = *i;
		if(dd->getTarget() == aTarget) {
			// Same download it seems
			d = dd;
		}
	}

	if(d == NULL) {
		// No such download, check if the target file is smaller than the one being downloaded...
		if(aResume && (aSize != -1) ) {
			if(Util::getFileSize(aTarget) >= aSize) {
				cs.leave();
				return;
			}
		}

		d = new Download();
	
		if(aFile.find('\\')) {
			d->setFileName(aFile.substr(aFile.rfind('\\')+1));
			d->setLast(aUser->getNick(), aFile.substr(0, aFile.rfind('\\')+1));
		} else {
			d->setFileName(aFile);
			d->setLast(aUser->getNick(), "");
		}
		d->setUser(aUser);
		d->setTarget(aTarget);
		d->setSize(aSize);
		d->setResume(aResume);

		queue.push_back(d);
		fireAdded(d);
	}

	if(waiting.find(aUser) == waiting.end()) {
		int status = ConnectionManager::getInstance()->getDownloadConnection(aUser);
		if(status == UserConnection::CONNECTING) {
			waiting[d->getUser()] = d;
			fireConnecting(d);
		}
	}
	cs.leave();
	
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
void DownloadManager::download(const string& aFile, LONGLONG aSize, const string& aUser, const string& aTarget, bool aResume /* = true /*/) {
	Download* d = NULL;

	User::Ptr& user = Client::findUser(aUser);
	if(user) {
		download(aFile, aSize, user, aTarget, aResume);
		return;
	}

	// We don't know who this user is, so we just add it to the list...
	cs.enter();
	
	for(Download::Iter i = queue.begin(); i != queue.end(); ++i) {
		// First, search the queue for the same download...
		Download* dd = *i;
		if(dd->getTarget() == aTarget) {
			// Same download it seems
			d = dd;
		}
	}

	if(d == NULL) {
		d = new Download();
	
		if(aFile.find('\\')) {
			d->setFileName(aFile.substr(aFile.rfind('\\')+1));
			d->setLast(aUser, aFile.substr(0, aFile.rfind('\\')+1));
		} else {
			d->setFileName(aFile);
			d->setLast(aUser, "");
		}

		d->setTarget(aTarget);
		d->setSize(aSize);
		d->setResume(aResume);

		queue.push_back(d);
		fireAdded(d);
		fireFailed(d, d->getLastNick() + " has gone offline");
	}
	cs.leave();
}

void DownloadManager::downloadList(User::Ptr& aUser) {
	string file = Settings::getAppPath() + aUser->getNick() + ".DcLst";
	download("MyList.DcLst", -1, aUser, file, false);
	userLists.push_back(file);
}

void DownloadManager::downloadList(const string& aUser) {
	string file = Settings::getAppPath() + aUser + ".DcLst";
	download("MyList.DcLst", -1, aUser, file, false);
	userLists.push_back(file);
}

void DownloadManager::removeDownload(Download* aDownload) {
	cs.enter();
	// First, we search the queue
	for(Download::Iter i = queue.begin(); i != queue.end(); ++i) {
		if(*i == aDownload) {
			// Good! It's in the queue, we can simply remove it...and check if we're waiting for a connection to the user...
			if(aDownload->getUser()) {
				Download::UserIter i = waiting.find(aDownload->getUser());
				if(i != waiting.end()) {
					waiting.erase(i);
				}
			}
			queue.erase(i);
			delete aDownload;
			cs.leave();
			return;
		}
	}

	// Check the running downloads...
	for(Download::MapIter j = running.begin(); j != running.end(); ++j) {
		if(j->second == aDownload) {
			// This is worse, we have to abort the download...
			UserConnection* conn = j->first;
			running.erase(j);
			removeConnection(conn);
			delete aDownload;
			cs.leave();
			return;
		}
	}

	// Not found...
	cs.leave();

}
void DownloadManager::removeConnection(UserConnection::Ptr aConn) {
	cs.enter();
	for(UserConnection::Iter i = connections.begin(); i != connections.end(); ++i) {
		if(*i == aConn) {
			aConn->removeListener(this);
			connections.erase(i);
			ConnectionManager::getInstance()->putDownloadConnection(aConn);
			break;
		}
	}
	cs.leave();
}

void DownloadManager::removeConnections() {
	cs.enter();
	UserConnection::Iter i = connections.begin();

	while(i != connections.end()) {
		UserConnection* c = *i;
		i = connections.erase(i);
		c->removeListener(this);
		ConnectionManager::getInstance()->putDownloadConnection(c);
	}
	cs.leave();
}

void DownloadManager::checkDownloads(UserConnection* aConn) {
	cs.enter();
	dcdebug("Checking downloads...");
	for(Download::Iter i = queue.begin(); i != queue.end(); ++i) {
		if(aConn->getUser()->getNick() == (*i)->getLastNick()) {
			Download* d = *i;
			queue.erase(i);

			d->setUser(aConn->getUser());

			running[aConn] = d;
			
			if(d->getResume()) {
				LONGLONG x = Util::getFileSize(d->getTarget());
				d->setPos( (x == -1) ? 0 : x);
			} else {
				d->setPos(0);
			}
			aConn->get(d->getLastPath()+d->getFileName(), d->getPos());
			dcdebug("Found!\n");
			cs.leave();
			return;
		}
	}
	// Connection not needed any more, return it to the ConnectionManager...
	dcdebug("Not found!\n");
	removeConnection(aConn);
	cs.leave();
}

void DownloadManager::onData(UserConnection* aSource, const BYTE* aData, int aLen) {
	cs.enter();
	dcassert(running.find(aSource) != running.end());
	DWORD len;
	Download* d = running[aSource];
	cs.leave();
	
	WriteFile(d->getFile(), aData, aLen, &len, NULL);
	d->addPos(len);
	fireTick(d);
}

void DownloadManager::onFileLength(UserConnection* aSource, const string& aFileLength) {
	cs.enter();
	Download::MapIter i = running.find(aSource);
	dcassert(i != running.end());
	Download* d = i->second;

	HANDLE file;
	Util::ensureDirectory(d->getTarget());
	if(d->getResume())
		file = CreateFile(d->getTarget().c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	else
		file = CreateFile(d->getTarget().c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	
	if(file == INVALID_HANDLE_VALUE) {
		running.erase(aSource);
		queue.push_back(d);

		fireFailed(d, "Could not open target file");
		removeConnection(aSource);
		cs.leave();
		return;
	}
	
	d->setFile(file, true);
	d->setPos(d->getSize(), true);
	d->setSize(aFileLength);

	if(d->getSize() == d->getPos()) {
		// We're done...and this connection is broken...
		running.erase(i);
		fireComplete(d);
		delete d;
		
		removeConnection(aSource);
	} else {
		fireStarting(d);
		aSource->setDataMode(d->getSize() - d->getPos());
		aSource->startSend();
	}
	cs.leave();
}

/** Download finished! */
void DownloadManager::onModeChange(UserConnection* aSource, int aNewMode) {
	cs.enter();
	Download::MapIter i = running.find(aSource);
	dcassert(i != running.end());
	
	Download::Ptr p = i->second;
	running.erase(i);
	cs.leave();

	if(p->getPos() != p->getSize())
		dcdebug("Download incomplete??? : ");
	
	CloseHandle(p->getFile());
	p->setFile(NULL);

	dcdebug("Download finished: %s to %s, size %I64d\n", p->getFileName().c_str(), p->getTarget().c_str(), p->getSize());
	fireComplete(p);
	delete p;

	checkDownloads(aSource);
}

void DownloadManager::onMaxedOut(UserConnection* aSource) { 
	cs.enter();
	Download::MapIter i = running.find(aSource);
	dcassert(i != running.end());

	Download* d = i->second;
	
	fireFailed(d, "No slots available");
	running.erase(i);

	removeConnection(aSource);
	queue.insert(queue.begin(), d);
	cs.leave();
}

void DownloadManager::onError(UserConnection* aSource, const string& aError) {
	cs.enter();
	Download::MapIter i = running.find(aSource);
	
	dcassert(i != running.end());

	Download* d = i->second;

	if(d->getFile()) {
		CloseHandle(d->getFile());
		d->setFile(NULL);
	}
	fireFailed(d, aError);
	
	running.erase(i);
	
	removeConnection(aSource);
	queue.insert(queue.begin(), d);
	cs.leave();
}


/**
 * @file DownloadManger.cpp
 * $Id: DownloadManager.cpp,v 1.14 2001/12/18 12:32:18 arnetheduck Exp $
 * @if LOG
 * $Log: DownloadManager.cpp,v $
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

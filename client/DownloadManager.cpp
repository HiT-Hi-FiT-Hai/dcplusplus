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

DownloadManager* DownloadManager::instance = NULL;

void DownloadManager::onTimerSecond(DWORD aTick) {
	Download::List tmp = queue;

	for(Download::Iter i = tmp.begin(); i != tmp.end(); ++i) {
		Download* d = *i;
		if(aTick > d->getLastTry() + 60 * 1000) {
			// Time to go hot...
			User* u = Client::findUser(d->getLastNick());
			if(u != NULL) {
				d->setUser(u);
				d->setLastTry(aTick);
				if(ConnectionManager::getInstance()->getDownloadConnection(u)!=UserConnection::BUSY) {
					fireConnecting(d);
				}
			}
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
void DownloadManager::download(const string& aFile, LONGLONG aSize, User* aUser, const string& aTarget, bool aResume /* = true /*/) {
	Download* d = NULL;
	for(Download::Iter i = queue.begin(); i != queue.end(); ++i) {
		// First, search the queue for the same download...
		Download* dd = *i;
		if(dd->getUser() == aUser && (dd->getLastPath() + dd->getFileName()) == aFile && dd->getTarget() == aTarget) {
			// Same download it seems
			d = dd;
		}
	}

	if(d == NULL) {
		d = new Download();
	
		if(aFile.find('\\')) {
			d->setFileName(aFile.substr(aFile.rfind('\\')+1));
			d->setLast(aUser->getNick(), aFile.substr(0, aFile.rfind('\\')+1));
		} else {
			d->setFileName(aFile);
			d->setLast(aUser->getNick(), "");
		}

		d->setTarget(aTarget);
		d->setUser(aUser);
		d->setSize(aSize);
		d->setLastTry(TimerManager::getInstance()->getTick());
		d->setResume(aResume);

		queue.push_back(d);
		fireAdded(d);
	}

	int status = ConnectionManager::getInstance()->getDownloadConnection(aUser);
	if(status == UserConnection::CONNECTING) {
		fireConnecting(d);
	}
}

void DownloadManager::removeConnection(UserConnection::Ptr aConn) {
	connectionCS.enter();
	for(UserConnection::Iter i = connections.begin(); i != connections.end(); ++i) {
		if(*i == aConn) {
			aConn->removeListener(this);
			connections.erase(i);
			ConnectionManager::getInstance()->putDownloadConnection(aConn);
			break;
		}
	}
	connectionCS.leave();
}

void DownloadManager::removeConnections() {
	connectionCS.enter();
	UserConnection::Iter i = connections.begin();

	while(i != connections.end()) {
		UserConnection* c = *i;
		i = connections.erase(i);
		c->removeListener(this);
		ConnectionManager::getInstance()->putDownloadConnection(c);
	}
	connectionCS.leave();
}

void DownloadManager::checkDownloads(UserConnection* aConn) {
	for(Download::Iter i = queue.begin(); i != queue.end(); ++i) {
		if(aConn->getUser() == (*i)->getUser()) {
			Download* d = *i;
			queue.erase(i);

			runningCS.enter();
			running[aConn] = d;
			runningCS.leave();
			
			if(d->getResume()) {
				WIN32_FIND_DATA fd;
				HANDLE hFind;
				
				hFind = FindFirstFile(d->getTarget().c_str(), &fd);
				
				if (hFind == INVALID_HANDLE_VALUE) {
					d->setPos(0);
				} else {
					d->setPos(fd.nFileSizeHigh << 32 | fd.nFileSizeLow);
					FindClose(hFind);
				}
			} else {
				d->setPos(0);
			}
			aConn->get(d->getLastPath()+d->getFileName(), d->getPos());
			return;
		}
	}
	// Connection not needed any more, return it to the ConnectionManager...
	removeConnection(aConn);
}

void DownloadManager::onData(UserConnection* aSource, BYTE* aData, int aLen) {
	runningCS.enter();
	dcassert(running.find(aSource) != running.end());
	DWORD len;
	Download* d = running[aSource];
	runningCS.leave();
	
	WriteFile(d->getFile(), aData, aLen, &len, NULL);
	d->addPos(len);
	fireTick(d);
}

void DownloadManager::onFileLength(UserConnection* aSource, const string& aFileLength) {
	runningCS.enter();
	Download::MapIter i = running.find(aSource);
	dcassert(i != running.end());
	Download* d = i->second;

	HANDLE file;
	if(d->getResume())
		file = CreateFile(d->getTarget().c_str(), GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	else
		file = CreateFile(d->getTarget().c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	
	if(file == NULL) {
		aSource->disconnect();
		fireFailed(d, "Could not open target file");
		ConnectionManager::getInstance()->putDownloadConnection(aSource);
		runningCS.leave();
		return;
	}
	
	d->setFile(file, true);
	d->setPos(d->getSize(), true);
	d->setSize(aFileLength);

	if(d->getSize() == d->getPos()) {
		// We're done...
		running.erase(i);
		fireComplete(d);
		delete d;

		aSource->startSend();
		checkDownloads(aSource);
	} else {
		fireStarting(d);
		aSource->setDataMode(d->getSize() - d->getPos());
		aSource->startSend();
	}
	runningCS.leave();
}

/** Download finished! */
void DownloadManager::onModeChange(UserConnection* aSource, int aNewMode) {
	runningCS.enter();
	Download::MapIter i = running.find(aSource);
	dcassert(i != running.end());
	
	Download::Ptr p = i->second;
	running.erase(i);
	runningCS.leave();

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
	runningCS.enter();
	Download::MapIter i = running.find(aSource);
	dcassert(i != running.end());

	Download* d = i->second;
	
	fireFailed(d, "No slots available");
	running.erase(i);
	runningCS.leave();

	removeConnection(aSource);
	queue.insert(queue.begin(), d);
}

void DownloadManager::onError(UserConnection* aSource, const string& aError) {
	runningCS.enter();
	Download::MapIter i = running.find(aSource);
	
	dcassert(i != running.end());

	Download* d = i->second;

	fireFailed(d, aError);
	
	running.erase(i);
	runningCS.leave();
	
	removeConnection(aSource);
	queue.insert(queue.begin(), d);
}


/**
 * @file DownloadManger.cpp
 * $Id: DownloadManager.cpp,v 1.8 2001/12/07 20:03:06 arnetheduck Exp $
 * @if LOG
 * $Log: DownloadManager.cpp,v $
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

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

void DownloadManager::download(const string& aFile, const string& aSize, User* aUser, const string& aDestination, bool aResume /* = true /*/) {
	
	Download* d = new Download();
	
	if(aFile.find('\\')) {
		d->fileName = aFile.substr(aFile.rfind('\\')+1);
		d->lastPath = aFile.substr(0, aFile.rfind('\\')+1);
	} else {
		d->fileName = aFile;
		d->lastPath = "";
	}
	d->targetFileName = aDestination;
	d->lastNick = aUser->getNick();
	d->user = aUser;
	d->size = _atoi64(aSize.c_str());
	queue.push_back(d);
	fireAdded(d);

	if(ConnectionManager::getInstance()->getDownloadConnection(aUser)!=UserConnection::BUSY) {
		fireConnecting(d);
	}
}

void DownloadManager::checkDownloads(UserConnection* aConn) {
	for(Download::Iter i = queue.begin(); i != queue.end(); ++i) {
		if(aConn->getUser() == (*i)->user) {
			Download* d = *i;
			queue.erase(i);
			running[aConn] = d;
			
			WIN32_FIND_DATA fd;
			HANDLE hFind;
			
			hFind = FindFirstFile(d->targetFileName.c_str(), &fd);
			
			if (hFind == INVALID_HANDLE_VALUE) {
				d->pos = 0;
			} else {
				d->pos = fd.nFileSizeHigh << 32 | fd.nFileSizeLow;
				FindClose(hFind);
			}
			
			aConn->get(d->lastPath+d->fileName, d->pos);
			return;
		}
	}

	// Connection not needed any more, return it to the ConnectionManager...
	aConn->removeListener(this);
	ConnectionManager::getInstance()->putDownloadConnection(aConn);
}

void DownloadManager::onData(UserConnection* aSource, BYTE* aData, int aLen) {
	dcassert(running.find(aSource) != running.end());
	DWORD len;
	Download* d = running[aSource];
	
	WriteFile(d->hFile, aData, aLen, &len, NULL);
	d->pos += len;
	fireTick(d);
}

void DownloadManager::onFileLength(UserConnection* aSource, const string& aFileLength) {
	Download::MapIter i = running.find(aSource);
	dcassert(i != running.end());
	Download* d = i->second;

	d->hFile = CreateFile(d->targetFileName.c_str(), GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if(d->hFile == NULL) {
		fireFailed(d, "Could not open target file");
	}
	SetFilePointer(d->hFile, 0, NULL, FILE_END);
	DWORD high;
	d->pos = GetFileSize(d->hFile, &high);
	d->pos|=high<<32;
	d->size = _atoi64(aFileLength.c_str());
	if(d->size == d->pos) {
		// We're done...
		CloseHandle(d->hFile);
		running.erase(i);
		fireComplete(d);
		delete d;

		aSource->startSend();
		checkDownloads(aSource);
	} else {
		aSource->setDataMode(d->size - d->pos);
		aSource->startSend();
	}
}

/** Download finished! */
void DownloadManager::onModeChange(UserConnection* aSource, int aNewMode) {
	Download::MapIter i = running.find(aSource);
	dcassert(i != running.end());
	
	Download::Ptr p = i->second;
	running.erase(i);

	CloseHandle(p->hFile);
	if(p->pos != p->size)
		dcdebug("Download incomplete??? : ");

	dcdebug("Download finished: %s to %s, size %I64d\n", p->fileName.c_str(), p->targetFileName.c_str(), p->size);
	fireComplete(p);
	delete p;

	checkDownloads(aSource);
}

void DownloadManager::onMaxedOut(UserConnection* aSource) { 
	Download* d = running[aSource];
	
	fireFailed(d, "No slots available"); 
	ConnectionManager::getInstance()->putDownloadConnection(aSource);
	queue.insert(queue.begin(), d);
};

/**
 * @file DownloadManger.cpp
 * $Id: DownloadManager.cpp,v 1.3 2001/11/29 19:10:54 arnetheduck Exp $
 * @if LOG
 * $Log: DownloadManager.cpp,v $
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

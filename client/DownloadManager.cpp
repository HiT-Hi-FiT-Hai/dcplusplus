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
#include "DCClient.h"

DownloadManager* DownloadManager::instance = NULL;

void DownloadManager::download(const string& aFile, const string& aUser, const string& aDestination) {
	expectedNicks.push_back(aUser);

	Download* d = new Download();
	
	if(aFile.find('\\')) {
		d->fileName = aFile.substr(aFile.rfind('\\')+1);
		d->lastPath = aFile.substr(0, aFile.rfind('\\')+1);
	} else {
		d->fileName = aFile;
		d->lastPath = "";
	}
	d->destination = aDestination;
	d->hFile = CreateFile(aDestination.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, NULL);

	d->lastUser = aUser;

	queue.push_back(d);

	DCClient::List& clients = DCClient::getList();

	for(DCClient::Iter i = clients.begin(); i != clients.end(); ++i) {
		if((*i)->userConnected(d->lastUser)) {
			if(Settings::getConnectionType() == Settings::CONNECTION_ACTIVE)
				(*i)->connectToMe(d->lastUser);
			else
				(*i)->revConnectToMe(d->lastUser);
		}
	}
}

void DownloadManager::checkDownloads(UserConnection* aConn) {
	for(Download::Iter i = queue.begin(); i != queue.end(); ++i) {
		if(aConn->getNick() == (*i)->lastUser) {
			Download* d = *i;
			queue.erase(i);
			running[aConn] = d;
			return;
		}
	}
}

void DownloadManager::onLock(UserConnection* aSource, const string& aLock, const string& aPk) {
	if(!aSource->hasSentNick())
		aSource->myNick(Settings::getNick());
	if(!aSource->hasSentLock())
		aSource->lock(CryptoManager::getInstance()->getLock(), CryptoManager::getInstance()->getPk());
	
	aSource->direction("Download", "666");
	aSource->key(CryptoManager::getInstance()->makeKey(aLock));
}

void DownloadManager::onKey(UserConnection* aSource, const string& aKey) {
	dcassert(running.find(aSource) != running.end());
	aSource->get(running[aSource]->lastPath + running[aSource]->fileName, 0);
}

void DownloadManager::onData(UserConnection* aSource, BYTE* aData, int aLen) {
	dcassert(running.find(aSource) != running.end());
	DWORD len;
	Download* d = running[aSource];
	
	WriteFile(d->hFile, aData, aLen, &len, NULL);
	d->pos += len;
}

void DownloadManager::onFileLength(UserConnection* aSource, const string& aFileLength) {
	dcassert(running.find(aSource) != running.end());
	Download* d = running[aSource];
	d->size = _atoi64(aFileLength.c_str());
	d->pos = 0;
	aSource->setDataMode(d->size);
	aSource->startSend();
}

/** Download finished! */
void DownloadManager::onModeChange(UserConnection* aSource, int aNewMode) {
	Download::MapIter i = running.find(aSource);
	
	Download::Ptr p = i->second;
	running.erase(i);

	CloseHandle(p->hFile);
	if(p->pos != p->size)
		dcdebug("Download incomplete??? : ");

	dcdebug("Download finished: %s to %s, size %I64d\n", p->fileName.c_str(), p->destination.c_str(), p->size);
	fireDownloadComplete(p);
	delete p;

	checkDownloads(aSource);
}

/**
 * @file DownloadManger.cpp
 * $Id: DownloadManager.cpp,v 1.2 2001/11/26 23:40:36 arnetheduck Exp $
 * @if LOG
 * $Log: DownloadManager.cpp,v $
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

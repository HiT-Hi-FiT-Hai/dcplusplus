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
	
	d->hFile = CreateFile(aDestination.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, NULL);

	d->lastUser = aUser;

	queue.push_back(d);

	DCClient::List& clients = DCClient::getList();

	for(DCClient::Iter i = clients.begin(); i != clients.end(); ++i) {
		if((*i)->userConnected(d->lastUser)) {
			(*i)->connectToMe(d->lastUser);
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

/**
 * @file DownloadManger.cpp
 * $Id: DownloadManager.cpp,v 1.1 2001/11/25 22:06:25 arnetheduck Exp $
 * @if LOG
 * $Log: DownloadManager.cpp,v $
 * Revision 1.1  2001/11/25 22:06:25  arnetheduck
 * Finally downloading is working! There are now a few quirks and bugs to be fixed
 * but what the heck....!
 *
 * @endif
 */

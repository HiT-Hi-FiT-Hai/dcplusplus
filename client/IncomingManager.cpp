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

#include "IncomingManager.h"
#include "DownloadManager.h"
#include "UploadManager.h"

#include "UserConnection.h"
#include "CryptoManager.h"

IncomingManager* IncomingManager::instance = NULL;

void IncomingManager::onIncomingConnection() {
	UserConnection::Ptr uc = new UserConnection();
	try { uc->accept(socket);
		uc->addListener(this);
		uc->myNick(Settings::getNick());
		uc->lock(CryptoManager::getInstance()->getLock(), CryptoManager::getInstance()->getPk());
	} catch(Exception e) {
		dcdebug("Error creating connection\n");
		delete uc;
	}
}


void IncomingManager::onMyNick(UserConnection* aSource, const string& aNick) {
	removeConnection(aSource);
	
	if(DownloadManager::getInstance()->isExpected(aNick)) {
		DownloadManager::getInstance()->addConnection(aSource);
	} else {
		UploadManager::getInstance()->addConnection(aSource);
	}
}

void IncomingManager::onConnected(UserConnection* aSource) {
//	aSource->myNick(Settings::getNick());
//	aSource->lock(CryptoManager::getLock(), CryptoManager::getPk());
}

void IncomingManager::connect(const string& aServer, short aPort) {
	UserConnection* c = new UserConnection();
	addConnection(c);
	c->connect(aServer, aPort);
}

/**
 * @file IncomingManger.cpp
 * $Id: IncomingManager.cpp,v 1.2 2001/11/26 23:40:36 arnetheduck Exp $
 * @if LOG
 * $Log: IncomingManager.cpp,v $
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

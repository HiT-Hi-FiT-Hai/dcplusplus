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

#include "stdinc.h"
#include "DCPlusPlus.h"

#include "ConnectionManager.h"
#include "DownloadManager.h"
#include "UploadManager.h"
#include "CryptoManager.h"
#include "ShareManager.h"
#include "SearchManager.h"
#include "QueueManager.h"
#include "ClientManager.h"
#include "LogManager.h"
#include "HubManager.h"
#include "SettingsManager.h"
#include "StringTokenizer.h"

void startup() {
	ResourceManager::newInstance();
	SettingsManager::newInstance();

	LogManager::newInstance();
	TimerManager::newInstance();
	ShareManager::newInstance();
	CryptoManager::newInstance();
	SearchManager::newInstance();
	ClientManager::newInstance();
	ConnectionManager::newInstance();
	DownloadManager::newInstance();
	UploadManager::newInstance();
	HubManager::newInstance();
	QueueManager::newInstance();

	SettingsManager::getInstance()->load();	

	int i;
	for(i = 0; i < SettingsManager::SPEED_LAST; i++) {
		if(SETTING(CONNECTION) == SettingsManager::connectionSpeeds[i])
			break;
	}
	if(i == SettingsManager::SPEED_LAST) {
		SettingsManager::getInstance()->set(SettingsManager::CONNECTION, SettingsManager::connectionSpeeds[0]);
	}

	// Update server list to use bz2 lists instead...
	StringTokenizer st(SETTING(HUBLIST_SERVERS), ';');
	string tmp;
	for(StringIter j = st.getTokens().begin(); j != st.getTokens().end(); ++j) {
		if(*j == "http://dcpp.lichlord.org/PublicHubList.config") {
			tmp += string("http://dcpp.lichlord.org/PublicHubList.config.bz2") + ";";
		} else if(*j == "http://dcpp.lichlord.org/FullList.config") {
			tmp += string("http://dcpp.lichlord.org/FullList.config.bz2") + ";";
		} else {
			tmp += *j + ';';
		}
	}
	
	if(!tmp.empty()) {
		tmp.erase(tmp.size()-1);
	}

	SettingsManager::getInstance()->set(SettingsManager::HUBLIST_SERVERS, tmp);

	ShareManager::getInstance()->refresh(false, false);
	HubManager::getInstance()->refresh();
}

void shutdown() {
	TimerManager::getInstance()->removeListeners();
	
	SettingsManager::getInstance()->save();
	
	ShareManager::deleteInstance();
	CryptoManager::deleteInstance();
	DownloadManager::deleteInstance();
	UploadManager::deleteInstance();
	QueueManager::deleteInstance();
	ConnectionManager::deleteInstance();
	SearchManager::deleteInstance();
	ClientManager::deleteInstance();
	HubManager::deleteInstance();
	LogManager::deleteInstance();
	SettingsManager::deleteInstance();
	TimerManager::deleteInstance();
	ResourceManager::deleteInstance();
}

/**
 * @file DCPlusPlus.cpp
 * $Id: DCPlusPlus.cpp,v 1.12 2002/04/28 08:25:50 arnetheduck Exp $
 */


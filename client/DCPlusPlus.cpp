/* 
 * Copyright (C) 2001-2003 Jacek Sieka, j_s@telia.com
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
#include "FinishedManager.h"
#include "ADLSearch.h"

void startup(void (*f)(void*, const string&), void* p) {
	Util::initialize();

	ResourceManager::newInstance();
	SettingsManager::newInstance();

	if(f != NULL)
		(*f)(p, STRING(SHARED_FILES));
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
	FinishedManager::newInstance();
	ADLSearchManager::newInstance();

	SettingsManager::getInstance()->load();	

	int i;
	for(i = 0; i < SettingsManager::SPEED_LAST; i++) {
		if(SETTING(CONNECTION) == SettingsManager::connectionSpeeds[i])
			break;
	}
	if(i == SettingsManager::SPEED_LAST) {
		SettingsManager::getInstance()->set(SettingsManager::CONNECTION, SettingsManager::connectionSpeeds[0]);
	}

	double v = Util::toDouble(SETTING(CONFIG_VERSION));
	if(v <= 0.22) {
		// Disable automatic public hublist opening
		SettingsManager::getInstance()->set(SettingsManager::OPEN_PUBLIC, false);
	}
	if(v <= 0.251) {
		StringTokenizer st(SETTING(HUBLIST_SERVERS), ';');
		StringList& sl = st.getTokens();
		StringList sl2;
		bool defFound = false;
		StringIter si;
		for(si = sl.begin(); si != sl.end(); ++si) {
			if((si->find("http://dcplusplus.sourceforge.net") != string::npos) ||
				(si->find("http://dcpp.lichlord.org") != string::npos))
			{
				if(!defFound) {
					sl2.push_back("http://www.hublist.org/PublicHubList.config.bz2");
					defFound = true;
				}
			} else {
				sl2.push_back(*si);
			}
		}
		string tmp;
		for(si = sl2.begin(); si != sl2.end(); ++si) {
			tmp += *si + ';';
		}

		if(!tmp.empty()) {
			tmp.erase(tmp.length()-1);
			SettingsManager::getInstance()->set(SettingsManager::HUBLIST_SERVERS, tmp);
		}
	}

	if(f != NULL)
		(*f)(p, STRING(DOWNLOAD_QUEUE));
	QueueManager::getInstance()->loadQueue();

	ShareManager::getInstance()->refresh(false, false, true);
}

void shutdown() {
	ConnectionManager::getInstance()->shutdown();

	TimerManager::getInstance()->removeListeners();
	SettingsManager::getInstance()->save();
	
	ADLSearchManager::deleteInstance();
	FinishedManager::deleteInstance();
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
 * @file
 * $Id: DCPlusPlus.cpp,v 1.24 2003/07/15 14:53:10 arnetheduck Exp $
 */


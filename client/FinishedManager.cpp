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

#include <time.h>
#include "FinishedManager.h"

FinishedManager* Singleton<FinishedManager>::instance = NULL;

FinishedManager::~FinishedManager()
{
	Lock l(cs);
	for_each(list.begin(), list.end(), DeleteFunction<FinishedItem*>());
}

void FinishedManager::onAction(DownloadManagerListener::Types type, Download* d) throw() {
	switch(type) {
	case DownloadManagerListener::COMPLETE:
		{
			char buf[32];
			time_t _tt;
			time(&_tt);
			tm* _tm = localtime(&_tt);
			strftime(buf, 31, "%Y-%m-%d %H:%M:%S", _tm);
			
			FinishedItem *item = new FinishedItem(
				d->getTarget(), d->getUserConnection()->getUser()->getNick(),
				d->getUserConnection()->getUser()->getLastHubName(),
				d->getSize(), d->getTotal(), (GET_TICK() - d->getStart()), buf, d->isSet(Download::FLAG_CRC32_OK));
			{
				dcdebug("Adding finished: \"%s\" - \"%s\" (user: \"%s\")", item->getTime().c_str(), 
					item->getTarget().c_str(), item->getUser().c_str());
				Lock l(cs);
				list.push_back(item);
			}
			
			fire(FinishedManagerListener::ADDED, item);
		}
		break;
		
	default:
		break;
	}
}

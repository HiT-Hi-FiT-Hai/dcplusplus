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

#include "FinishedManager.h"

FinishedManager* Singleton<FinishedManager>::instance = NULL;

FinishedManager::~FinishedManager()
{
	Lock l(cs);
	for_each(downloads.begin(), downloads.end(), DeleteFunction<FinishedItem*>());
	for_each(uploads.begin(), uploads.end(), DeleteFunction<FinishedItem*>());
}

void FinishedManager::onAction(DownloadManagerListener::Types type, Download* d) throw()
{
	switch(type) {
	case DownloadManagerListener::COMPLETE:
		{
			if(!d->isSet(Download::FLAG_USER_LIST) || BOOLSETTING(LOG_FILELIST_TRANSFERS)) {
				FinishedItem *item = new FinishedItem(
					d->getTarget(), d->getUserConnection()->getUser()->getNick(),
					d->getUserConnection()->getUser()->getLastHubName(),
					d->getSize(), d->getTotal(), (GET_TICK() - d->getStart()), GET_TIME(), d->isSet(Download::FLAG_CRC32_OK));
				{
					Lock l(cs);
					downloads.push_back(item);
				}

				fire(FinishedManagerListener::ADDED_DL, item);
			}
		}
		break;
		
	default:
		break;
	}
}

void FinishedManager::onAction(UploadManagerListener::Types type, Upload* u) throw()
{
	switch(type) {
	case UploadManagerListener::COMPLETE:
		{
			if(!u->isSet(Upload::FLAG_USER_LIST) || BOOLSETTING(LOG_FILELIST_TRANSFERS)) {
				FinishedItem *item = new FinishedItem(
					u->getLocalFileName(), u->getUserConnection()->getUser()->getNick(),
					u->getUserConnection()->getUser()->getLastHubName(),
					u->getSize(), u->getTotal(), (GET_TICK() - u->getStart()), GET_TIME());
				{
					Lock l(cs);
					uploads.push_back(item);
				}
				
				fire(FinishedManagerListener::ADDED_UL, item);
			}
		}
		break;
		
	default:
		break;
	}
}

/**
 * @file
 * $Id: FinishedManager.cpp,v 1.14 2004/01/07 14:14:52 arnetheduck Exp $
 */

/* 
 * Copyright (C) 2001-2005 Jacek Sieka, arnetheduck on gmail point com
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

FinishedManager::~FinishedManager()
{
	Lock l(cs);
	for_each(downloads.begin(), downloads.end(), DeleteFunction<FinishedItem*>());
	for_each(uploads.begin(), uploads.end(), DeleteFunction<FinishedItem*>());
	DownloadManager::getInstance()->removeListener(this);
	UploadManager::getInstance()->removeListener(this);

}

void FinishedManager::remove(FinishedItem *item, bool upload /* = false */) {
	{
		Lock l(cs);
		FinishedItem::List *listptr = upload ? &uploads : &downloads;
		FinishedItem::Iter it = find(listptr->begin(), listptr->end(), item);

		if(it != listptr->end())
			listptr->erase(it);
		else
			return;
	}
	if (!upload)
		fire(FinishedManagerListener::RemovedDl(), item);
	else
		fire(FinishedManagerListener::RemovedUl(), item);
	delete item;		
}

void FinishedManager::removeAll(bool upload /* = false */) {
	{
		Lock l(cs);
		FinishedItem::List *listptr = upload ? &uploads : &downloads;
		for_each(listptr->begin(), listptr->end(), DeleteFunction<FinishedItem*>());
		listptr->clear();
	}
	if (!upload)
		fire(FinishedManagerListener::RemovedAllDl());
	else
		fire(FinishedManagerListener::RemovedAllUl());
}

void FinishedManager::on(DownloadManagerListener::Complete, Download* d) throw()
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

		fire(FinishedManagerListener::AddedDl(), item);
	}
}

void FinishedManager::on(UploadManagerListener::Complete, Upload* u) throw()
{
	if(!u->isSet(Upload::FLAG_TTH_LEAVES) && (BOOLSETTING(LOG_FILELIST_TRANSFERS) || !u->isSet(Upload::FLAG_USER_LIST))) {
		FinishedItem *item = new FinishedItem(
			u->getLocalFileName(), u->getUserConnection()->getUser()->getNick(),
			u->getUserConnection()->getUser()->getLastHubName(),
			u->getSize(), u->getTotal(), (GET_TICK() - u->getStart()), GET_TIME());
		{
			Lock l(cs);
			uploads.push_back(item);
		}
		
		fire(FinishedManagerListener::AddedUl(), item);
	}
}

/**
 * @file
 * $Id: FinishedManager.cpp,v 1.21 2005/01/05 19:30:27 arnetheduck Exp $
 */

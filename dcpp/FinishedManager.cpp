/*
 * Copyright (C) 2001-2008 Jacek Sieka, arnetheduck on gmail point com
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

#include "ClientManager.h"
#include "FinishedManagerListener.h"
#include "Download.h"
#include "Upload.h"
#include "DownloadManager.h"
#include "UploadManager.h"

namespace dcpp {

FinishedManager::FinishedManager() {
	DownloadManager::getInstance()->addListener(this);
	UploadManager::getInstance()->addListener(this);
}

FinishedManager::~FinishedManager() throw() {
	DownloadManager::getInstance()->removeListener(this);
	UploadManager::getInstance()->removeListener(this);

	Lock l(cs);
	for_each(downloads.begin(), downloads.end(), DeleteFunction());
	for_each(uploads.begin(), uploads.end(), DeleteFunction());
}

void FinishedManager::remove(FinishedItemPtr item, bool upload /* = false */) {
	{
		Lock l(cs);
		FinishedItemList *listptr = upload ? &uploads : &downloads;
		FinishedItemList::iterator it = find(listptr->begin(), listptr->end(), item);

		if(it != listptr->end())
			listptr->erase(it);
		else
			return;
	}
	fire(FinishedManagerListener::Removed(), upload, item);
	delete item;
}

void FinishedManager::removeAll(bool upload /* = false */) {
	{
		Lock l(cs);
		FinishedItemList *listptr = upload ? &uploads : &downloads;
		for_each(listptr->begin(), listptr->end(), DeleteFunction());
		listptr->clear();
	}
	fire(FinishedManagerListener::RemovedAll(), upload);
}

void FinishedManager::on(DownloadManagerListener::Complete, Download* d) throw()
{
	if(d->getType() == Transfer::TYPE_FILE || (d->getType() == Transfer::TYPE_FULL_LIST && BOOLSETTING(LOG_FILELIST_TRANSFERS))) {
		FinishedItemPtr item = new FinishedItem(
			d->getPath(), Util::toString(ClientManager::getInstance()->getNicks(d->getUser()->getCID())),
			Util::toString(ClientManager::getInstance()->getHubNames(d->getUser()->getCID())),
			d->getSize(), d->getPos(), (GET_TICK() - d->getStart()), GET_TIME(), d->isSet(Download::FLAG_CRC32_OK));
		{
			Lock l(cs);
			downloads.push_back(item);
		}

		fire(FinishedManagerListener::Added(), false, item);
	}
}

void FinishedManager::on(UploadManagerListener::Complete, Upload* u) throw()
{
	if(u->getType() == Transfer::TYPE_FILE || (u->getType() == Transfer::TYPE_FULL_LIST && BOOLSETTING(LOG_FILELIST_TRANSFERS))) {
		FinishedItemPtr item = new FinishedItem(
			u->getPath(), Util::toString(ClientManager::getInstance()->getNicks(u->getUser()->getCID())),
			Util::toString(ClientManager::getInstance()->getHubNames(u->getUser()->getCID())),
			u->getSize(), u->getPos(), (GET_TICK() - u->getStart()), GET_TIME());
		{
			Lock l(cs);
			uploads.push_back(item);
		}

		fire(FinishedManagerListener::Added(), true, item);
	}
}

} // namespace dcpp

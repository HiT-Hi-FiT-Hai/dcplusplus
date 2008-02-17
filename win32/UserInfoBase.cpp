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

#include "stdafx.h"

#include "UserInfoBase.h"

#include <dcpp/QueueManager.h>
#include <dcpp/LogManager.h>
#include <dcpp/FavoriteManager.h>
#include <dcpp/UploadManager.h>
#include <dcpp/User.h>

#include "PrivateFrame.h"
#include "HubFrame.h"

void UserInfoBase::matchQueue() {
	try {
		QueueManager::getInstance()->addList(user, QueueItem::FLAG_MATCH_QUEUE);
	} catch(const Exception& e) {
		LogManager::getInstance()->message(e.getError());
	}
}
void UserInfoBase::getList() {
	try {
		QueueManager::getInstance()->addList(user, QueueItem::FLAG_CLIENT_VIEW);
	} catch(const Exception& e) {
		LogManager::getInstance()->message(e.getError());
	}
}
void UserInfoBase::browseList() {
	if(user->getCID().isZero())
		return;
	try {
		QueueManager::getInstance()->addPfs(user, "");
	} catch(const Exception& e) {
		LogManager::getInstance()->message(e.getError());
	}
}

void UserInfoBase::addFav() {
	FavoriteManager::getInstance()->addFavoriteUser(user);
}

void UserInfoBase::pm(SmartWin::WidgetTabView* mdiParent) {
	PrivateFrame::openWindow(mdiParent, user);
}

void UserInfoBase::grant() {
	UploadManager::getInstance()->reserveSlot(user);
}

void UserInfoBase::removeAll() {
	QueueManager::getInstance()->removeSource(user, QueueItem::Source::FLAG_REMOVED);
}

void UserInfoBase::UserTraits::operator()(UserInfoBase* ui) { 
	if(ui->getUser()->isSet(User::NMDC)) 
		adcOnly = false;
	bool fav = FavoriteManager::getInstance()->isFavoriteUser(ui->getUser());
	if(fav)
		nonFavOnly = false;
	if(!fav)
		favOnly = false;
}

void UserInfoBase::connectFav(SmartWin::WidgetTabView* mdiParent) {
	std::string url = FavoriteManager::getInstance()->getUserURL(user);
	if(!url.empty()) {
		HubFrame::openWindow(mdiParent, url);
	}
}

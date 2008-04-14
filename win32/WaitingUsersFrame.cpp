/*
 * Copyright (C) 2001-2006 Jacek Sieka, arnetheduck on gmail point com
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

#include "resource.h"

#include "WaitingUsersFrame.h"
#include "PrivateFrame.h"

#include <dcpp/Client.h>
#include <dcpp/QueueManager.h>
#include <dcpp/FavoriteManager.h>
#include <dcpp/UploadManager.h>

// Constructor
WaitingUsersFrame::WaitingUsersFrame(dwt::TabView* mdiParent) :
	BaseType(mdiParent, T_("Waiting Users"), IDH_WAITING_USERS, IDR_WAITING_USERS)
{
	UploadManager::getInstance()->addListener(this);

	// Create tree control
	{
		queued = addChild(WinUtil::Seeds::treeView);
		addWidget(queued);
		queued->onContextMenu(std::tr1::bind(&WaitingUsersFrame::handleContextMenu, this, _1));
		queued->onChar(std::tr1::bind(&WaitingUsersFrame::handleChar, this, _1));
	}

	initStatus();

	layout();
	
	onSpeaker(std::tr1::bind(&WaitingUsersFrame::handleSpeaker, this, _1, _2));
	// Load all waiting users & files.
	loadAll();
}

// Recalculate frame control layout
void WaitingUsersFrame::layout() {
	dwt::Rectangle r(this->getClientAreaSize());

	layoutStatus(r);

	queued->setBounds(r);
}

bool WaitingUsersFrame::preClosing() {
	UploadManager::getInstance()->removeListener(this);
	return true;
}

void WaitingUsersFrame::postClosing() {
	for(HTREEITEM userNode = queued->getRoot(); userNode; userNode = queued->getNextSibling(userNode)) {
		delete reinterpret_cast<UserItem *>(queued->getData(userNode));
	}
}

bool WaitingUsersFrame::handleContextMenu(dwt::ScreenCoordinate pt) {
	if(pt.x() == -1 || pt.y() == -1) {
		pt = queued->getContextMenuPos();
	}
	MenuPtr menu = createMenu(WinUtil::Seeds::menu);
	menu->appendItem(IDC_GETLIST, T_("&Get file list"), std::tr1::bind(&WaitingUsersFrame::onGetList, this));
	menu->appendItem(IDC_COPY_FILENAME, T_("Copy Filename"), std::tr1::bind(&WaitingUsersFrame::onCopyFilename, this));
	menu->appendItem(IDC_REMOVE, T_("&Remove"), std::tr1::bind(&WaitingUsersFrame::onRemove, this));
	menu->appendItem(IDC_GRANTSLOT, T_("Grant &extra slot"), std::tr1::bind(&WaitingUsersFrame::onGrantSlot, this));
	menu->appendItem(IDC_ADD_TO_FAVORITES, T_("Add To &Favorites"), std::tr1::bind(&WaitingUsersFrame::onAddToFavorites, this));
	menu->appendItem(IDC_PRIVATEMESSAGE, T_("&Send private message"), std::tr1::bind(&WaitingUsersFrame::onPrivateMessage, this));
	menu->trackPopupMenu(pt, TPM_LEFTALIGN | TPM_RIGHTBUTTON);
	return true;
}

HRESULT WaitingUsersFrame::handleSpeaker(WPARAM wParam, LPARAM lParam) {
	if(wParam == SPEAK_ADD_FILE) {
		boost::scoped_ptr<pair<UserPtr, string> > p((pair<UserPtr, string> *)lParam);
		onAddFile(p->first, p->second);
		setDirty(SettingsManager::BOLD_WAITING_USERS);
	} else if(wParam == SPEAK_REMOVE_USER) {
		boost::scoped_ptr<UserItem> p(reinterpret_cast<UserItem *>(lParam));
		onRemoveUser(p->u);
		setDirty(SettingsManager::BOLD_WAITING_USERS);
	}
	return 0;
}

// Load all searches from manager
void WaitingUsersFrame::loadAll()
{
	// Load queue
	UserList users = UploadManager::getInstance()->getWaitingUsers();
	for (UserList::iterator uit = users.begin(); uit != users.end(); ++uit) {
		HTREEITEM lastInserted = queued->insert(
			(WinUtil::getNicks(*uit) + _T(" - ") + WinUtil::getHubNames(*uit).first),
			NULL, (LPARAM)(new UserPtr(*uit)));
		UploadManager::FileSet files = UploadManager::getInstance()->getWaitingUserFiles(*uit);
		for (UploadManager::FileSet::const_iterator fit = files.begin(); fit != files.end(); ++fit) {
			queued->insert(Text::toT(*fit), lastInserted);
		}
	}
}

void WaitingUsersFrame::onPrivateMessage() {
	UserPtr user = getSelectedUser();
	if (user) {
		PrivateFrame::openWindow(getParent(), user);
	}
}

void WaitingUsersFrame::onGrantSlot() {
	UserPtr user = getSelectedUser();
	if (user) {
		UploadManager::getInstance()->reserveSlot(user);
	}
}

void WaitingUsersFrame::onAddToFavorites() {
	UserPtr user = getSelectedUser();
	if (user) {
		FavoriteManager::getInstance()->addFavoriteUser(user);
	}
}

HTREEITEM WaitingUsersFrame::getParentItem() {
	HTREEITEM sel = queued->getSelected();
	if(!sel) {
		return NULL;
	}
	HTREEITEM parent = queued->getParent(sel);
	return parent ? parent : sel;
}

void WaitingUsersFrame::onGetList()
{
	UserPtr user = getSelectedUser();
	if (user) {
		QueueManager::getInstance()->addList(user, QueueItem::FLAG_CLIENT_VIEW);
	}
}

void WaitingUsersFrame::onCopyFilename() {
	HTREEITEM selectedItem = queued->getSelected(), parentItem = getParentItem();

	if (!selectedItem || !parentItem || selectedItem == parentItem)
		return;
	
	tstring txt = queued->getText(selectedItem);
	tstring::size_type i = txt.find(_T('('));
	if(i != tstring::npos && i > 0) {
		txt.erase(i - 1);
	}
	if(!txt.empty()) {
		WinUtil::setClipboard(txt);
	}
}

// Remove queued item
void WaitingUsersFrame::onRemove()
{
	UserPtr user = getSelectedUser();
	if (user) {
		UploadManager::getInstance()->clearUserFiles(user);
	}
}

// UploadManagerListener
void WaitingUsersFrame::on(UploadManagerListener::WaitingRemoveUser, const UserPtr& aUser) throw() {
	speak(SPEAK_REMOVE_USER, (LPARAM)new UserItem(aUser));
}

void WaitingUsersFrame::on(UploadManagerListener::WaitingAddFile, const UserPtr& aUser, const string& aFilename) throw() {
	speak(SPEAK_ADD_FILE, (LPARAM)new pair<UserPtr, string>(aUser, aFilename));
}

// Keyboard shortcuts
bool WaitingUsersFrame::handleChar(int c) {
	if(c == VK_DELETE) {
		onRemove();
		return true;
	}
	return false;
}

void WaitingUsersFrame::onRemoveUser(const UserPtr& aUser) {
	HTREEITEM userNode = queued->getRoot();

	while (userNode) {
		UserPtr *u = reinterpret_cast<UserPtr *>(queued->getData(userNode));
		if (aUser == *u) {
			delete u;
			queued->erase(userNode);
			return;
		}
		userNode = queued->getNextSibling(userNode);
	}
}

void WaitingUsersFrame::onAddFile(const UserPtr& aUser, const string& aFile) {
	HTREEITEM userNode = queued->getRoot();

	string fname = aFile.substr(0, aFile.find(_T('(')));
	
	while (userNode) {
		if (aUser == *reinterpret_cast<UserPtr *>(queued->getData(userNode))) {
			HTREEITEM childNode = queued->getChild(userNode);
			while (childNode) {
				tstring buf = queued->getText(childNode);
				
				if (fname == Text::fromT(buf.substr(0, buf.find(_T('('))))) {
					delete reinterpret_cast<UserPtr *>(queued->getData(childNode));
					queued->erase(childNode);
					break;
				}
				childNode = queued->getNextSibling(childNode);
			}

			//file isn't already listed, add it
			queued->insert(Text::toT(aFile), userNode, (LPARAM)new UserPtr(aUser));

			return;
		}

		userNode = queued->getNextSibling(userNode);
	}

	userNode = queued->insert(WinUtil::getNicks(aUser) + _T(" - ") + WinUtil::getHubNames(aUser).first,
		NULL, (LPARAM)new UserPtr(aUser));
	queued->insert(Text::toT(aFile), userNode);
	queued->expand(userNode);
}


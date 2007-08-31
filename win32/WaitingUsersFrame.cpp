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
WaitingUsersFrame::WaitingUsersFrame(SmartWin::WidgetMDIParent* mdiParent) :
	BaseType(mdiParent)
{
	UploadManager::getInstance()->addListener(this);

	// Create tree control
	{
		WidgetTreeView::Seed cs;
		cs.style = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_HSCROLL | WS_VSCROLL | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_SHOWSELALWAYS;
		cs.exStyle = WS_EX_CLIENTEDGE;
		queued = createTreeView(cs);
		addWidget(queued);
#ifdef PORT_ME
		queued->setColor(WinUtil::textColor, WinUtil::bgColor);
#endif
	}

	initStatus();

	layout();
	
	onSpeaker(std::tr1::bind(&WaitingUsersFrame::handleSpeaker, this, _1, _2));
	// Load all waiting users & files.
	loadAll();
}

// Recalculate frame control layout
void WaitingUsersFrame::layout()
{
	const int border = 2;

	SmartWin::Rectangle r(this->getClientAreaSize());

	layoutStatus(r);

	queued->setBounds(r);
}

bool WaitingUsersFrame::preClosing() {
	UploadManager::getInstance()->removeListener(this);
	return true;
}
void WaitingUsersFrame::postClosing() {
	
	// This looks like the correct way of doing this according to
	// the getNode implementation, but the abstraction leakage is
	// horrid.
	SmartWin::TreeViewNode userNode;
	queued->getNode(SmartWin::TreeViewNode(), TVGN_ROOT, userNode);

	// SmartWin doesn't appear to have any way to access the item data,
	// rather than text. TVM_GETITEM, at least, appears nowhere in it.
	while (userNode.handle) {
		delete reinterpret_cast<UserItem *>(queued->getData(userNode.handle));
		queued->getNode(userNode, TVGN_NEXT, userNode);
	}
}

HRESULT WaitingUsersFrame::handleContextMenu(WPARAM wParam, LPARAM lParam) {
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

	if(reinterpret_cast<HWND>(wParam) == queued->handle()) {
		if(pt.x == -1 || pt.y == -1) {
			//pt = queued->getContextMenuPos();
		}
		WidgetMenuPtr menu = createMenu(true);
		menu->appendItem(IDC_GETLIST, CTSTRING(GET_FILE_LIST), std::tr1::bind(&WaitingUsersFrame::onGetList, this));
		menu->appendItem(IDC_COPY_FILENAME, CTSTRING(COPY_FILENAME), std::tr1::bind(&WaitingUsersFrame::onCopyFilename, this));
		menu->appendItem(IDC_REMOVE, CTSTRING(REMOVE), std::tr1::bind(&WaitingUsersFrame::onRemove, this));
		menu->appendItem(IDC_GRANTSLOT, CTSTRING(GRANT_EXTRA_SLOT), std::tr1::bind(&WaitingUsersFrame::onGrantSlot, this));
		menu->appendItem(IDC_ADD_TO_FAVORITES, CTSTRING(ADD_TO_FAVORITES), std::tr1::bind(&WaitingUsersFrame::onAddToFavorites, this));
		menu->appendItem(IDC_PRIVATEMESSAGE, CTSTRING(SEND_PRIVATE_MESSAGE), std::tr1::bind(&WaitingUsersFrame::onPrivateMessage, this));
		menu->trackPopupMenu(this, pt.x, pt.y, TPM_LEFTALIGN | TPM_RIGHTBUTTON);
		return TRUE;
	}
	return FALSE;
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
		SmartWin::TreeViewNode lastInserted = queued->insertNode(
			(WinUtil::getNicks(*uit) + _T(" - ") + WinUtil::getHubNames(*uit).first),
			SmartWin::TreeViewNode(TVI_ROOT), (LPARAM)(new UserPtr(*uit)));
		UploadManager::FileSet files = UploadManager::getInstance()->getWaitingUserFiles(*uit);
		for (UploadManager::FileSet::const_iterator fit = files.begin(); fit != files.end(); ++fit) {
			queued->insertNode(Text::toT(*fit), lastInserted);
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

SmartWin::TreeViewNode WaitingUsersFrame::GetParentItem() {
	SmartWin::TreeViewNode item, parent;
	queued->getNode(SmartWin::TreeViewNode(), TVM_GETNEXTITEM, item);
#ifdef PORT_ME
	// @todo presumably another getNode WRT YA TVGN constant. Hurray.
	parent = queued->GetParentItem(item);
#endif
	return parent.handle?parent:item;
}

void WaitingUsersFrame::onGetList()
{
	UserPtr user = getSelectedUser();
	if (user) {
		QueueManager::getInstance()->addList(user, QueueItem::FLAG_CLIENT_VIEW);
	}
}

void WaitingUsersFrame::onCopyFilename() {
#ifdef PORT_ME
	// @todo see previous comment. 
	SmartWin::TreeViewNode selectedItem = getSelectedItem(), parentItem = queued->GetParentItem(selectedItem);

	if (!selectedItem || !parentItem || selectedItem == parentItem)
		return;
	TCHAR filenameBuf[256];
	queued->GetItemText(selectedItem, filenameBuf, 255);
	*_tcschr(filenameBuf, _T('(')) = NULL;
	tstring tmpstr(filenameBuf);
	if(!tmpstr.empty()) {
		// remove last space
		tmpstr.erase(tmpstr.length() - 1);
		WinUtil::setClipboard(tmpstr);
	}
#endif
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
void WaitingUsersFrame::on(UploadManagerListener::WaitingRemoveUser, const UserPtr aUser) throw() {
	speak(SPEAK_REMOVE_USER, (LPARAM)new UserItem(aUser));
}

void WaitingUsersFrame::on(UploadManagerListener::WaitingAddFile, const UserPtr aUser, const string& aFilename) throw() {
	speak(SPEAK_ADD_FILE, (LPARAM)new pair<UserPtr, string>(aUser, aFilename));
}

#ifdef PORT_ME

// Keyboard shortcuts
LRESULT WaitingUsersFrame::onChar(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
	switch(wParam)
	{
	case VK_DELETE:
		onRemove(0, 0, 0, bHandled);
		break;
	default:
		bHandled = FALSE;
	}
	return 0;
}

LRESULT WaitingUsersFrame::onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	// Get the bounding rectangle of the client area.
	RECT rc;
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
	queued->GetClientRect(&rc);
	queued->ScreenToClient(&pt);

	// Change selected item
	HTREEITEM item = queued->HitTest(pt, NULL);
	if (item == NULL) return FALSE;
	queued->SelectItem(item);

	// Hit-test
	if(PtInRect(&rc, pt))
	{
		queued->ClientToScreen(&pt);
		contextMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
		return TRUE;
	}

	return FALSE;
}
#endif

void WaitingUsersFrame::onRemoveUser(const UserPtr& aUser) {
#ifdef PORT_ME
	HTREEITEM userNode = queued->GetRootItem();

	while (userNode) {
		UserPtr *u = reinterpret_cast<UserPtr *>(queued->GetItemData(userNode));
		if (aUser == u->u) {
			delete u;
			queued->DeleteItem(userNode);
			return;
		}
		userNode = queued->GetNextSiblingItem(userNode);
	}
#endif
}

void WaitingUsersFrame::onAddFile(const UserPtr& aUser, const string& aFile) {
#ifdef PORT_ME
	HTREEITEM userNode = queued->GetRootItem();

	while (userNode) {
		if (aUser == reinterpret_cast<UserPtr *>(queued->GetItemData(userNode))->u) {
			HTREEITEM childNode = queued->GetChildItem(userNode);
			while (childNode) {
				TCHAR nickBuf[256];
				queued->GetItemText(childNode, nickBuf, 255);
				if (aFile.substr(0, aFile.find(_T('('))) ==
					Text::fromT(tstring(nickBuf).substr(0, tstring(nickBuf).find(_T('('))))) {
						delete reinterpret_cast<UserPtr *>(queued->GetItemData(childNode));
						queued->DeleteItem(childNode);
						break;
					}
					childNode = queued->GetNextSiblingItem(childNode);
			}

			//file isn't already listed, add it
			queued->InsertItem(TVIF_PARAM | TVIF_TEXT, Text::toT(aFile).c_str(), 0,
				0, 0, 0, (LPARAM)new UserPtr(aUser), userNode, TVI_LAST);

			return;
		}

		userNode = queued->GetNextSiblingItem(userNode);
	}

	userNode = queued->InsertItem(TVIF_PARAM | TVIF_TEXT, (WinUtil::getNicks(aUser) + _T(" - ") + WinUtil::getHubNames(aUser).first).c_str(),
		0, 0, 0, 0, (LPARAM)new UserPtr(aUser),	TVI_ROOT, TVI_LAST);
	queued->InsertItem(Text::toT(aFile).c_str(), userNode, TVI_LAST);
	queued->Expand(userNode);
#endif
}


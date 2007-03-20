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
#include "client/DCPlusPlus.h"
#include "Resource.h"

#include "client/Client.h"
#include "client/ClientManager.h"
#include "client/QueueManager.h"

#include "WaitingUsersFrame.h"

// Constructor
WaitingUsersFrame::WaitingUsersFrame(SmartWin::Widget* mdiParent) {
	onClosing(&WaitingUsersFrame::onClose);

	UploadManager::getInstance()->addListener(this);

	// Create tree control
	{
		WidgetTreeView::Seed cs;
		cs.style = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_HSCROLL | WS_VSCROLL | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_SHOWSELALWAYS;
		cs.exStyle = WS_EX_CLIENTEDGE;
		queued = createTreeView(cs);
		// @todo subclass WidgetTreeView to have onChar so MDIChildFrame can tab around it
		//add_widget(queued);
	}

	// Create context menu
	contextMenu = createMenu();
	contextMenu->appendItem(IDC_GETLIST, CTSTRING(GET_FILE_LIST), &WaitingUsersFrame::onGetList);
	contextMenu->appendItem(IDC_COPY_FILENAME, CTSTRING(COPY_FILENAME), &WaitingUsersFrame::onCopyFilename);
	contextMenu->appendItem(IDC_REMOVE, CTSTRING(REMOVE), &WaitingUsersFrame::onRemove);
	contextMenu->appendItem(IDC_GRANTSLOT, CTSTRING(GRANT_EXTRA_SLOT), &WaitingUsersFrame::onGrantSlot);
	contextMenu->appendItem(IDC_ADD_TO_FAVORITES, CTSTRING(ADD_TO_FAVORITES), &WaitingUsersFrame::onAddToFavorites);
	contextMenu->appendItem(IDC_PRIVATEMESSAGE, CTSTRING(SEND_PRIVATE_MESSAGE), &WaitingUsersFrame::onPrivateMessage);

#ifdef PORT_ME
	ctrlQueued->SetBkColor(WinUtil::bgColor);
	ctrlQueued->SetTextColor(WinUtil::textColor);
#endif

	closed = false;

	// Load all waiting users & files.
	LoadAll();
}

// Recalculate frame control layout
void WaitingUsersFrame::layout()
{
	queued->setBounds(SmartWin::Rectangle(SmartWin::Point(0, 0), getClientAreaSize()));
}

bool WaitingUsersFrame::onClose()
{
	if (!closed) {
		UploadManager::getInstance()->removeListener(this);

		closed = true;
		StupidWin::postMessage(this, WM_CLOSE);
		return true;
	} else {
		// This looks like the correct way of doing this according to
		// the getNode implementation, but the abstraction leakage is
		// horrid.
		SmartWin::TreeViewNode userNode;
		queued->getNode(SmartWin::TreeViewNode(), TVGN_ROOT, userNode);

		// SmartWin doesn't appear to have any way to access the item data,
		// rather than text. TVM_GETITEM, at least, appears nowhere in it.
		while (userNode.handle) {
			delete reinterpret_cast<UserPtr *>(StupidWin::getTreeItemData(queued, userNode));
			queued->getNode(userNode, TVGN_NEXT, userNode);
		}

		return true;
	}
}

// Load all searches from manager
void WaitingUsersFrame::LoadAll()
{
#ifdef PORT_ME
	// @todo Relies on apparently unimplemented WinUtil
	// Load queue
	User::List users = UploadManager::getInstance()->getWaitingUsers();
	for (User::Iter uit = users.begin(); uit != users.end(); ++uit) {
		HTREEITEM lastInserted = queued->insertNode(
			(WinUtil::getNicks(*uit) + _T(" - ") + WinUtil::getHubNames(*uit).first).c_str()
			,
			0, 0, 0, 0, (LPARAM)(new UserPtr(*uit)), TVI_ROOT, TVI_LAST);
		UploadManager::FileSet files = UploadManager::getInstance()->getWaitingUserFiles(*uit);
		for (UploadManager::FileSet::const_iterator fit = files.begin(); fit != files.end(); ++fit) {
			queued->InsertItem(Text::toT(*fit).c_str(), lastInserted, TVI_LAST);
		}
	}
#endif
}

void WaitingUsersFrame::onPrivateMessage(WidgetMenuPtr, unsigned int) {
	User::Ptr user = getSelectedUser();
	if (user) {
#ifdef PORT_ME
		PrivateFrame::openWindow(user);
#endif
	}
}

void WaitingUsersFrame::onGrantSlot(WidgetMenuPtr, unsigned int) {
	User::Ptr user = getSelectedUser();
	if (user) {
		UploadManager::getInstance()->reserveSlot(user);
	}
}

void WaitingUsersFrame::onAddToFavorites(WidgetMenuPtr, unsigned int) {
	User::Ptr user = getSelectedUser();
	if (user) {
#ifdef PORT_ME
		FavoriteManager::getInstance()->addFavoriteUser(user);
#endif PORT_ME
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

void WaitingUsersFrame::onGetList(WidgetMenuPtr, unsigned int)
{
	User::Ptr user = getSelectedUser();
	if (user) {
		QueueManager::getInstance()->addList(user, QueueItem::FLAG_CLIENT_VIEW);
	}
}

void WaitingUsersFrame::onCopyFilename(WidgetMenuPtr, unsigned int) {
#ifdef PORT_ME
	// @todo see previous comment. More StupidWin fodder.
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
void WaitingUsersFrame::onRemove(WidgetMenuPtr, unsigned int)
{
	User::Ptr user = getSelectedUser();
	if (user) {
		UploadManager::getInstance()->clearUserFiles(user);
	}
}

// UploadManagerListener
void WaitingUsersFrame::on(UploadManagerListener::WaitingRemoveUser, const User::Ptr aUser) throw() {
	StupidWin::postMessage(this, WM_SPEAKER, SPEAK_REMOVE_USER, (LPARAM)new UserPtr(aUser));
}

void WaitingUsersFrame::on(UploadManagerListener::WaitingAddFile, const User::Ptr aUser, const string& aFilename) throw() {
	StupidWin::postMessage(this, WM_SPEAKER, SPEAK_ADD_FILE, (LPARAM)new pair<User::Ptr, string>(aUser, aFilename));
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

void WaitingUsersFrame::onRemoveUser(const User::Ptr aUser) {
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
}

void WaitingUsersFrame::onAddFile(const User::Ptr aUser, const string& aFile) {
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
}

LRESULT WaitingUsersFrame::onSpeaker(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	if(wParam == SPEAK_ADD_FILE) {
		const pair<User::Ptr, string> *p = (pair<User::Ptr, string> *)lParam;
		onAddFile(p->first, p->second);
		delete p;
		if(BOOLSETTING(BOLD_WAITING_USERS))
			setDirty();
	} else if(wParam == SPEAK_REMOVE_USER) {
		onRemoveUser(reinterpret_cast<UserPtr *>(lParam)->u);
		delete reinterpret_cast<UserPtr *>(lParam);
		if(BOOLSETTING(BOLD_WAITING_USERS))
			setDirty();
	}
	return 0;
}
#endif

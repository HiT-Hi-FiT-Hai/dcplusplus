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

#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "Resource.h"

#include "UsersFrame.h"
#include "PrivateFrame.h"

#include "../client/StringTokenizer.h"
#include "../client/QueueManager.h"

int UsersFrame::columnIndexes[] = { COLUMN_NICK, COLUMN_STATUS, COLUMN_HUB, COLUMN_GRANT_SLOT };
int UsersFrame::columnSizes[] = { 200, 150, 300, 100 };
static ResourceManager::Strings columnNames[] = { ResourceManager::NICK, ResourceManager::STATUS, ResourceManager::LAST_HUB, ResourceManager::GRANT_EXTRA_SLOT };

LRESULT UsersFrame::onCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	CreateSimpleStatusBar(ATL_IDS_IDLEMESSAGE, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SBARS_SIZEGRIP);
	ctrlStatus.Attach(m_hWndStatusBar);

	ctrlUsers.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS , WS_EX_CLIENTEDGE, IDC_USERS);

	if(BOOLSETTING(FULL_ROW_SELECT)) {
		ctrlUsers.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP);
	} else {
		ctrlUsers.SetExtendedListViewStyle(LVS_EX_HEADERDRAGDROP);
	}
	
	ctrlUsers.SetBkColor(WinUtil::bgColor);
	ctrlUsers.SetTextBkColor(WinUtil::bgColor);
	ctrlUsers.SetTextColor(WinUtil::textColor);
	
	// Create listview columns
	WinUtil::splitTokens(columnIndexes, SETTING(USERSFRAME_ORDER), COLUMN_LAST);
	WinUtil::splitTokens(columnSizes, SETTING(USERSFRAME_WIDTHS), COLUMN_LAST);
	
	for(int j=0; j<COLUMN_LAST; j++) {
		ctrlUsers.InsertColumn(j, CSTRING_I(columnNames[j]), LVCFMT_LEFT, columnSizes[j], j);
	}
	
	ctrlUsers.SetColumnOrderArray(COLUMN_LAST, columnIndexes);
	usersMenu.CreatePopupMenu();
	usersMenu.AppendMenu(MF_STRING, IDC_PRIVATEMESSAGE, CSTRING(SEND_PRIVATE_MESSAGE));
	usersMenu.AppendMenu(MF_STRING, IDC_GETLIST, CSTRING(GET_FILE_LIST));
	usersMenu.AppendMenu(MF_STRING, IDC_REMOVE, CSTRING(REMOVE));
	usersMenu.AppendMenu(MF_STRING, IDC_GRANTSLOT, CSTRING(GRANT_EXTRA_SLOT));

	HubManager::getInstance()->addListener(this);
	ClientManager::getInstance()->addListener(this);

	User::List ul = HubManager::getInstance()->getFavoriteUsers();
	ctrlUsers.SetRedraw(FALSE);
	for(User::Iter i = ul.begin(); i != ul.end(); ++i) {
		addUser(*i);
	}
	ctrlUsers.SetRedraw(TRUE);

	m_hMenu = WinUtil::mainMenu;

	bHandled = FALSE;
	return TRUE;

}

LRESULT UsersFrame::onRemove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int i = -1;
	while( (i = ctrlUsers.GetNextItem(-1, LVNI_SELECTED)) != -1) {
		HubManager::getInstance()->removeFavoriteUser(((UserInfo*)ctrlUsers.GetItemData(i))->user);
	}
	return 0;
}

void UsersFrame::addUser(const User::Ptr& aUser) {
	dcassert(ctrlUsers.find(aUser->getNick()) == -1);

	StringList l;
	l.push_back(aUser->getNick());
	l.push_back(aUser->isOnline() ? STRING(ONLINE) : STRING(OFFLINE));
	if(aUser->getLastHubAddress().empty()) {
		l.push_back(aUser->getClientName());
	} else {
		l.push_back(aUser->getClientName() + " (" + aUser->getLastHubAddress() + ")");
	}
	l.push_back(aUser->getFavoriteGrantSlot() ? STRING(YES) : STRING(NO));
	ctrlUsers.insert(l, 0, (LPARAM)new UserInfo(aUser));
}

void UsersFrame::updateUser(const User::Ptr& aUser) {
	int i = ctrlUsers.find(aUser->getNick());
	dcassert(i != -1);
	dcassert( ((UserInfo*)ctrlUsers.GetItemData(i))->user == aUser );
	ctrlUsers.SetItemText(i, 1, aUser->isOnline() ? CSTRING(ONLINE) : CSTRING(OFFLINE) );
	ctrlUsers.SetItemText(i, 2, (aUser->getLastHubName() + " (" + aUser->getLastHubAddress() + ")").c_str());
	ctrlUsers.SetItemText(i, 3, aUser->getFavoriteGrantSlot() ? CSTRING(YES) : CSTRING(NO));
}

void UsersFrame::removeUser(const User::Ptr& aUser) {
	int i = ctrlUsers.find(aUser->getNick());
	dcassert(i != -1);
	dcassert( ((UserInfo*)ctrlUsers.GetItemData(i))->user == aUser );
	delete (UserInfo*)ctrlUsers.GetItemData(i);
	ctrlUsers.DeleteItem(i);
}

LRESULT UsersFrame::onGrantSlot(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int i=-1;
	//char buf[256];
	
	bool needToSave = false;

	while( (i = ctrlUsers.GetNextItem(i, LVNI_SELECTED)) != -1) {
		UserInfo* ui = (UserInfo*)ctrlUsers.GetItemData(i);
		ui->user->setFavoriteGrantSlot(!ui->user->getFavoriteGrantSlot());
		updateUser(ui->user);
		needToSave = true;
	}

	if (needToSave)
		HubManager::getInstance()->save();

	return 0;
}

LRESULT UsersFrame::onGetList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int i=-1;
	char buf[256];
	
	while( (i = ctrlUsers.GetNextItem(i, LVNI_SELECTED)) != -1) {
		ctrlUsers.GetItemText(i, COLUMN_NICK, buf, 256);
		UserInfo* ui = (UserInfo*)ctrlUsers.GetItemData(i);
		try {
			QueueManager::getInstance()->addList(ui->user, QueueItem::FLAG_CLIENT_VIEW);
		} catch(...) {
			// ...
		}
	}
	return 0;
}

LRESULT UsersFrame::onPrivateMessage(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int i=-1;
	char buf[256];
	while( (i = ctrlUsers.GetNextItem(i, LVNI_SELECTED)) != -1) {
		ctrlUsers.GetItemText(i, COLUMN_NICK, buf, 256);
		UserInfo* ui = (UserInfo*)ctrlUsers.GetItemData(i);
		if(ui->user->isOnline())
			PrivateFrame::openWindow(ui->user);
	}
	return 0;
}

LRESULT UsersFrame::onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	if(!closed) {
		HubManager::getInstance()->removeListener(this);
		ClientManager::getInstance()->removeListener(this);

		closed = true;
		PostMessage(WM_CLOSE);
		return 0;
	} else {
		WinUtil::saveHeaderOrder(ctrlUsers, SettingsManager::USERSFRAME_ORDER, 
			SettingsManager::USERSFRAME_WIDTHS, COLUMN_LAST, columnIndexes, columnSizes);

		MDIDestroy(m_hWnd);
		return 0;
	}
}

/**
 * @file
 * $Id: UsersFrame.cpp,v 1.15 2003/11/07 00:42:41 arnetheduck Exp $
 */


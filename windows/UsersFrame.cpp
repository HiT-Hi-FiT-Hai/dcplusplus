/* 
 * Copyright (C) 2001 Jacek Sieka, j_s@telia.com
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

UsersFrame* UsersFrame::frame = NULL;

int UsersFrame::columnIndexes[] = { COLUMN_NICK, COLUMN_STATUS, COLUMN_HUB };
int UsersFrame::columnSizes[] = { 200, 150, 300 };
static ResourceManager::Strings columnNames[] = { ResourceManager::NICK, ResourceManager::STATUS, ResourceManager::LAST_HUB };

LRESULT UsersFrame::onCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	// Only one of this window please...
	dcassert(frame == NULL);
	frame = this;
	
	SetWindowText(CSTRING(FAVORITE_USERS));
	
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

	HubManager::getInstance()->addListener(this);
	ClientManager::getInstance()->addListener(this);

	User::List ul = HubManager::getInstance()->getFavoriteUsers();
	ctrlUsers.SetRedraw(FALSE);
	for(User::Iter i = ul.begin(); i != ul.end(); ++i) {
		addUser(*i);
	}
	ctrlUsers.SetRedraw(TRUE);

	bHandled = FALSE;
	return TRUE;

}

LRESULT UsersFrame::onRemove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int i = -1;
	while( (i = ctrlUsers.GetNextItem(-1, LVNI_SELECTED)) != -1) {
		HubManager::getInstance()->removeFavoriteUser(((UserInfo*)ctrlUsers.GetItemData(i))->user);
		break;
	}
	return 0;
}

void UsersFrame::addUser(const User::Ptr& aUser) {
	dcassert(ctrlUsers.find(aUser->getNick()) == -1);

	StringList l;
	l.push_back(aUser->getNick());
	l.push_back(aUser->isOnline() ? STRING(ONLINE) : STRING(OFFLINE));
	l.push_back(aUser->getLastHubName() + " (" + aUser->getLastHubIp() + ")");
	ctrlUsers.insert(l, 0, (LPARAM)new UserInfo(aUser));
}

void UsersFrame::updateUser(const User::Ptr& aUser) {
	int i = ctrlUsers.find(aUser->getNick());
	dcassert(i != -1);
	dcassert( ((UserInfo*)ctrlUsers.GetItemData(i))->user == aUser );
	ctrlUsers.SetItemText(i, 1, aUser->isOnline() ? CSTRING(ONLINE) : CSTRING(OFFLINE) );
	ctrlUsers.SetItemText(i, 2, (aUser->getLastHubName() + " (" + aUser->getLastHubIp() + ")").c_str());
}

void UsersFrame::removeUser(const User::Ptr& aUser) {
	int i = ctrlUsers.find(aUser->getNick());
	dcassert(i != -1);
	dcassert( ((UserInfo*)ctrlUsers.GetItemData(i))->user == aUser );
	delete (UserInfo*)ctrlUsers.GetItemData(i);
	ctrlUsers.DeleteItem(i);
}

LRESULT UsersFrame::onGetList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int i=-1;
	char buf[256];
	
	while( (i = ctrlUsers.GetNextItem(i, LVNI_SELECTED)) != -1) {
		ctrlUsers.GetItemText(i, COLUMN_NICK, buf, 256);
		UserInfo* ui = (UserInfo*)ctrlUsers.GetItemData(i);
		try {
			QueueManager::getInstance()->addList(ui->user);
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
			PrivateFrame::openWindow(ui->user, m_hWndMDIClient, getTab());
	}
	return 0;
}

LRESULT UsersFrame::onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	HubManager::getInstance()->removeListener(this);
	ClientManager::getInstance()->removeListener(this);
	
	string tmp1;
	string tmp2;
	
	ctrlUsers.GetColumnOrderArray(COLUMN_LAST, columnIndexes);
	for(int j = COLUMN_FIRST; j != COLUMN_LAST; j++) {
		columnSizes[j] = ctrlUsers.GetColumnWidth(j);
		tmp1 += Util::toString(columnIndexes[j]) + ",";
		tmp2 += Util::toString(columnSizes[j]) + ",";
	}
	tmp1.erase(tmp1.size()-1, 1);
	tmp2.erase(tmp2.size()-1, 1);
	
	SettingsManager::getInstance()->set(SettingsManager::USERSFRAME_ORDER, tmp1);
	SettingsManager::getInstance()->set(SettingsManager::USERSFRAME_WIDTHS, tmp2);
	
	bHandled = FALSE;
	return 0;
}

/**
 * @file UsersFrame.cpp
 * $Id: UsersFrame.cpp,v 1.6 2002/12/28 01:31:50 arnetheduck Exp $
 */


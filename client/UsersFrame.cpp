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
#include "DCPlusPlus.h"

#include "UsersFrame.h"
#include "ResourceManager.h"
#include "StringTokenizer.h"
#include "QueueManager.h"
#include "PrivateFrame.h"

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
	
	ctrlUsers.SetBkColor(Util::bgColor);
	ctrlUsers.SetTextBkColor(Util::bgColor);
	ctrlUsers.SetTextColor(Util::textColor);
	
	// Create listview columns
	StringList l = StringTokenizer(SETTING(USERSFRAME_ORDER), ',').getTokens();
	{
		int k = 0;
		for(StringIter i = l.begin(); i != l.end(); ++i) {
			if(k >= COLUMN_LAST)
				break;
			columnIndexes[k++] = Util::toInt(*i);
		}
	}
	
	l = StringTokenizer(SETTING(USERSFRAME_WIDTHS), ',').getTokens();
	{
		int k = 0;
		for(StringIter i = l.begin(); i != l.end(); ++i) {
			if(k >= COLUMN_LAST)
				break;
			columnSizes[k++] = Util::toInt(*i);
		}
	}
	
	LV_COLUMN lvc;
	ZeroMemory(&lvc, sizeof(lvc));
	lvc.mask = LVCF_FMT | LVCF_ORDER | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	
	for(int j=0; j<COLUMN_LAST; j++)
	{
		lvc.pszText = const_cast<char*>(ResourceManager::getInstance()->getString(columnNames[j]).c_str());
		lvc.fmt = LVCFMT_LEFT;
		lvc.cx = columnSizes[j];
		lvc.iOrder = columnIndexes[j];
		lvc.iSubItem = j;
		ctrlUsers.InsertColumn(j, &lvc);
	}
	
	int n = 0;
	usersMenu.CreatePopupMenu();
	CMenuItemInfo mi;
	mi.fMask = MIIM_ID | MIIM_TYPE;
	mi.fType = MFT_STRING;

	mi.dwTypeData = const_cast<char*>(CSTRING(SEND_PRIVATE_MESSAGE));
	mi.wID = IDC_PRIVATEMESSAGE;
	usersMenu.InsertMenuItem(n++, TRUE, &mi);
	
	mi.dwTypeData = const_cast<char*>(CSTRING(GET_FILE_LIST));
	mi.wID = IDC_GETLIST;
	usersMenu.InsertMenuItem(n++, TRUE, &mi);
	
	mi.dwTypeData = const_cast<char*>(CSTRING(REMOVE));
	mi.wID = IDC_REMOVE;
	usersMenu.InsertMenuItem(n++, TRUE, &mi);
	
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
 * @file FavoriteHubsFrm.cpp
 * $Id: UsersFrame.cpp,v 1.1 2002/03/26 09:17:59 arnetheduck Exp $
 * @if LOG
 * $Log: UsersFrame.cpp,v $
 * Revision 1.1  2002/03/26 09:17:59  arnetheduck
 * New UsersFrame
 *
 * Revision 1.10  2002/03/23 01:58:42  arnetheduck
 * Work done on favorites...
 *
 * Revision 1.9  2002/03/13 23:06:07  arnetheduck
 * New info sent in the description part of myinfo...
 *
 * Revision 1.8  2002/03/13 20:35:25  arnetheduck
 * Release canditate...internationalization done as far as 0.155 is concerned...
 * Also started using mirrors of the public hub lists
 *
 * Revision 1.7  2002/02/12 00:35:37  arnetheduck
 * 0.153
 *
 * Revision 1.6  2002/02/10 12:25:24  arnetheduck
 * New properties for favorites, and some minor performance tuning...
 *
 * Revision 1.5  2002/02/09 18:13:51  arnetheduck
 * Fixed level 4 warnings and started using new stl
 *
 * Revision 1.4  2002/01/26 21:09:51  arnetheduck
 * Release 0.14
 *
 * Revision 1.3  2002/01/26 12:38:50  arnetheduck
 * Added some user options
 *
 * Revision 1.2  2002/01/20 22:54:46  arnetheduck
 * Bugfixes to 0.131 mainly...
 *
 * Revision 1.1  2002/01/13 22:53:26  arnetheduck
 * Favorites...
 *
 * @endif
 */


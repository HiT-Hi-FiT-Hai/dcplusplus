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

#include "FavoritesFrm.h"
#include "HubFrame.h"
#include "FavHubProperties.h"

#include "../client/ClientManager.h"
#include "../client/StringTokenizer.h"

FavoriteHubsFrame* FavoriteHubsFrame::frame = NULL;

int FavoriteHubsFrame::columnIndexes[] = { COLUMN_NAME, COLUMN_DESCRIPTION, COLUMN_NICK, COLUMN_PASSWORD, COLUMN_SERVER, COLUMN_USERDESCRIPTION };
int FavoriteHubsFrame::columnSizes[] = { 200, 290, 125, 100, 100, 125 };
static ResourceManager::Strings columnNames[] = { ResourceManager::AUTO_CONNECT, ResourceManager::DESCRIPTION, 
ResourceManager::NICK, ResourceManager::PASSWORD, ResourceManager::SERVER, ResourceManager::USER_DESCRIPTION
};

LRESULT FavoriteHubsFrame::onCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	// Only one of this window please...
	dcassert(frame == NULL);
	frame = this;
	
	SetWindowText(CSTRING(FAVORITE_HUBS));
	
	CreateSimpleStatusBar(ATL_IDS_IDLEMESSAGE, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SBARS_SIZEGRIP);
	ctrlStatus.Attach(m_hWndStatusBar);

	ctrlHubs.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS , WS_EX_CLIENTEDGE, IDC_HUBLIST);

	if(BOOLSETTING(FULL_ROW_SELECT)) {
		ctrlHubs.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES | LVS_EX_HEADERDRAGDROP);
	} else {
		ctrlHubs.SetExtendedListViewStyle(LVS_EX_CHECKBOXES | LVS_EX_HEADERDRAGDROP);
	}
	
	ctrlHubs.SetBkColor(WinUtil::bgColor);
	ctrlHubs.SetTextBkColor(WinUtil::bgColor);
	ctrlHubs.SetTextColor(WinUtil::textColor);
	
	// Create listview columns
	WinUtil::splitTokens(columnIndexes, SETTING(FAVORITESFRAME_ORDER), COLUMN_LAST);
	WinUtil::splitTokens(columnSizes, SETTING(FAVORITESFRAME_WIDTHS), COLUMN_LAST);
	
	for(int j=0; j<COLUMN_LAST; j++) {
		int fmt = LVCFMT_LEFT;
		ctrlHubs.InsertColumn(j, CSTRING_I(columnNames[j]), fmt, columnSizes[j], j);
	}
	
	ctrlHubs.SetColumnOrderArray(COLUMN_LAST, columnIndexes);
	
	ctrlConnect.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
		BS_PUSHBUTTON , 0, IDC_CONNECT);
	ctrlConnect.SetWindowText(CSTRING(CONNECT));
	ctrlConnect.SetFont(WinUtil::font);

	ctrlNew.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
		BS_PUSHBUTTON , 0, IDC_NEWFAV);
	ctrlNew.SetWindowText(CSTRING(NEW));
	ctrlNew.SetFont(WinUtil::font);

	ctrlProps.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
		BS_PUSHBUTTON , 0, IDC_EDIT);
	ctrlProps.SetWindowText(CSTRING(PROPERTIES));
	ctrlProps.SetFont(WinUtil::font);

	HubManager::getInstance()->addListener(this);
	updateList(HubManager::getInstance()->getFavoriteHubs());
	
	hubsMenu.CreatePopupMenu();
	hubsMenu.AppendMenu(MF_STRING, IDC_CONNECT, CSTRING(CONNECT));
	hubsMenu.AppendMenu(MF_STRING, IDC_NEWFAV, CSTRING(NEW));
	hubsMenu.AppendMenu(MF_STRING, IDC_EDIT, CSTRING(PROPERTIES));
	hubsMenu.AppendMenu(MF_STRING, IDC_REMOVE, CSTRING(REMOVE));

	bHandled = FALSE;
	return TRUE;
}

LRESULT FavoriteHubsFrame::onDoubleClickHublist(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	if(!checkNick())
		return 0;
	
	NMITEMACTIVATE* item = (NMITEMACTIVATE*) pnmh;

	if(item->iItem != -1) {
		FavoriteHubEntry* entry = (FavoriteHubEntry*)ctrlHubs.GetItemData(item->iItem);
		HubFrame::openWindow(m_hWndMDIClient, getTab(), entry->getServer(), entry->getNick(), entry->getPassword(), entry->getUserDescription());
	}

	return 0;
}

LRESULT FavoriteHubsFrame::onEnter(int /*idCtrl*/, LPNMHDR /* pnmh */, BOOL& /*bHandled*/) {
	if(!checkNick())
		return 0;
	
	int item = ctrlHubs.GetNextItem(-1, LVNI_FOCUSED);

	if(item != -1) {
		FavoriteHubEntry* entry = (FavoriteHubEntry*)ctrlHubs.GetItemData(item);
		HubFrame::openWindow(m_hWndMDIClient, getTab(), entry->getServer(), entry->getNick(), entry->getPassword(), entry->getUserDescription());
	}

	return 0;
}

LRESULT FavoriteHubsFrame::onClickedConnect(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(!checkNick())
		return 0;
	
	int i = -1;
	while( (i = ctrlHubs.GetNextItem(i, LVNI_SELECTED)) != -1) {
		FavoriteHubEntry* entry = (FavoriteHubEntry*)ctrlHubs.GetItemData(i);
		HubFrame::openWindow(m_hWndMDIClient, getTab(), entry->getServer(), entry->getNick(), entry->getPassword(), entry->getUserDescription());
	}
	return 0;
}

LRESULT FavoriteHubsFrame::onRemove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int i = -1;
	while( (i = ctrlHubs.GetNextItem(-1, LVNI_SELECTED)) != -1) {
		HubManager::getInstance()->removeFavorite((FavoriteHubEntry*)ctrlHubs.GetItemData(i));
	}
	return 0;
}

LRESULT FavoriteHubsFrame::onEdit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int i = -1;
	if((i = ctrlHubs.GetNextItem(i, LVNI_SELECTED)) != -1)
	{
		FavoriteHubEntry* e = (FavoriteHubEntry*)ctrlHubs.GetItemData(i);
		dcassert(e != NULL);
		FavHubProperties dlg(e);
		if(dlg.DoModal(m_hWnd) == IDOK)
		{
			ctrlHubs.SetItemText(i, COLUMN_NAME, e->getName().c_str());
			ctrlHubs.SetItemText(i, COLUMN_DESCRIPTION, e->getDescription().c_str());
			ctrlHubs.SetItemText(i, COLUMN_SERVER, e->getServer().c_str());
			ctrlHubs.SetItemText(i, COLUMN_NICK, e->getNick(false).c_str());
			ctrlHubs.SetItemText(i, COLUMN_PASSWORD, string(e->getPassword().size(), '*').c_str());
			ctrlHubs.SetItemText(i, COLUMN_USERDESCRIPTION, e->getUserDescription().c_str());
		}
	}
	return 0;
}

LRESULT FavoriteHubsFrame::onNew(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	FavoriteHubEntry e;
	FavHubProperties dlg(&e);

	if(dlg.DoModal((HWND)*this) == IDOK)
		HubManager::getInstance()->addFavorite(e);
	return 0;
}

bool FavoriteHubsFrame::checkNick() {
	if(SETTING(NICK).empty()) {
		MessageBox(CSTRING(ENTER_NICK));
		return false;
	}
	return true;
}

LRESULT FavoriteHubsFrame::onItemChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NMITEMACTIVATE* l = (NMITEMACTIVATE*)pnmh;
	if(l->iItem != -1) {
		FavoriteHubEntry* f = (FavoriteHubEntry*)ctrlHubs.GetItemData(l->iItem);
		f->setConnect(ctrlHubs.GetCheckState(l->iItem) != FALSE);
		HubManager::getInstance()->save();
	}
	return 0;
}

LRESULT FavoriteHubsFrame::onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	HubManager::getInstance()->removeListener(this);
	
	string tmp1;
	string tmp2;

	ctrlHubs.GetColumnOrderArray(COLUMN_LAST, columnIndexes);
	for(int j = COLUMN_FIRST; j != COLUMN_LAST; j++) {
		columnSizes[j] = ctrlHubs.GetColumnWidth(j);
		tmp1 += Util::toString(columnIndexes[j]) + ",";
		tmp2 += Util::toString(columnSizes[j]) + ",";
	}
	tmp1.erase(tmp1.size()-1, 1);
	tmp2.erase(tmp2.size()-1, 1);
	
	SettingsManager::getInstance()->set(SettingsManager::FAVORITESFRAME_ORDER, tmp1);
	SettingsManager::getInstance()->set(SettingsManager::FAVORITESFRAME_WIDTHS, tmp2);
	
	bHandled = FALSE;
	return 0;
}

void FavoriteHubsFrame::UpdateLayout(BOOL bResizeBars /* = TRUE */)
{
	RECT rect;
	GetClientRect(&rect);
	// position bars and offset their dimensions
	UpdateBarsPosition(rect, bResizeBars);

	if(ctrlStatus.IsWindow()) {
		CRect sr;
		int w[3];
		ctrlStatus.GetClientRect(sr);
		int tmp = (sr.Width()) > 316 ? 216 : ((sr.Width() > 116) ? sr.Width()-100 : 16);

		w[0] = sr.right - tmp;
		w[1] = w[0] + (tmp-16)/2;
		w[2] = w[0] + (tmp-16);

		ctrlStatus.SetParts(3, w);
	}

	CRect rc = rect;
	rc.bottom -=28;
	ctrlHubs.MoveWindow(rc);

	rc = rect;
	rc.bottom -= 2;
	rc.top = rc.bottom - 22;

	rc.left = rc.right - 96;
	rc.right -= 2;
	ctrlConnect.MoveWindow(rc);

	rc.left = rc.left - 96;
	rc.right -= 98;
	ctrlProps.MoveWindow(rc);

	rc.left = rc.left - 96;
	rc.right -= 98;
	ctrlNew.MoveWindow(rc);
}

void FavoriteHubsFrame::onAction(HubManagerListener::Types type, FavoriteHubEntry* entry) throw() {
	switch(type) {
		case HubManagerListener::FAVORITE_ADDED: addEntry(entry); break;
		case HubManagerListener::FAVORITE_REMOVED: ctrlHubs.DeleteItem(ctrlHubs.find((LPARAM)entry)); break;
	}
};

/**
 * @file
 * $Id: FavoritesFrm.cpp,v 1.10 2003/04/15 10:14:01 arnetheduck Exp $
 */


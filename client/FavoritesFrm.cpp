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

#include "FavoritesFrm.h"
#include "HubFrame.h"
#include "Client.h"
#include "LineDlg.h"
#include "FavHubProperties.h"

FavoriteHubsFrame* FavoriteHubsFrame::frame = NULL;

LRESULT FavoriteHubsFrame::onCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{

	// Only one of this window please...
	dcassert(frame == NULL);
	frame = this;
	
	SetWindowText("Favorite Hubs");
	
	CreateSimpleStatusBar(ATL_IDS_IDLEMESSAGE, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SBARS_SIZEGRIP);
	ctrlStatus.Attach(m_hWndStatusBar);

	ctrlHubs.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS , WS_EX_CLIENTEDGE, IDC_HUBLIST);

	if(BOOLSETTING(FULL_ROW_SELECT)) {
		ctrlHubs.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);
	} else {
		ctrlHubs.SetExtendedListViewStyle(LVS_EX_CHECKBOXES);
	}
	
	ctrlHubs.SetBkColor(Util::bgColor);
	ctrlHubs.SetTextBkColor(Util::bgColor);
	ctrlHubs.SetTextColor(Util::textColor);
	
	ctrlHubs.InsertColumn(COLUMN_NAME, _T("Auto Connect / Name"), LVCFMT_LEFT, 200, COLUMN_NAME);
	ctrlHubs.InsertColumn(COLUMN_DESCRIPTION, _T("Description"), LVCFMT_LEFT, 290, COLUMN_DESCRIPTION);
	ctrlHubs.InsertColumn(COLUMN_NICK, _T("Nick"), LVCFMT_LEFT, 125, COLUMN_NICK);
	ctrlHubs.InsertColumn(COLUMN_PASSWORD, _T("Password"), LVCFMT_LEFT, 100, COLUMN_PASSWORD);
	ctrlHubs.InsertColumn(COLUMN_SERVER, _T("Server"), LVCFMT_LEFT, 100, COLUMN_SERVER);
	
	ctrlConnect.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
		BS_PUSHBUTTON , 0, IDC_CONNECT);
	ctrlConnect.SetWindowText("Connect");
	ctrlConnect.SetFont(ctrlHubs.GetFont());

	ctrlNew.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
		BS_PUSHBUTTON , 0, IDC_NEWFAV);
	ctrlNew.SetWindowText("New...");
	ctrlNew.SetFont(ctrlHubs.GetFont());

	HubManager::getInstance()->addListener(this);
	HubManager::getInstance()->getFavoriteHubs();
	
	hubsMenu.CreatePopupMenu();
	CMenuItemInfo mi;
	mi.fMask = MIIM_ID | MIIM_TYPE;
	mi.fType = MFT_STRING;
	mi.cch = 7;
	mi.dwTypeData = "Connect";
	mi.wID = IDC_CONNECT;
	hubsMenu.InsertMenuItem(0, TRUE, &mi);
	
	mi.fMask = MIIM_ID | MIIM_TYPE;
	mi.fType = MFT_STRING;
	mi.cch = 21;
	mi.dwTypeData = "Remove from favorites";
	mi.wID = IDC_REMOVE;
	hubsMenu.InsertMenuItem(1, TRUE, &mi);
	
	mi.fMask = MIIM_ID | MIIM_TYPE;
	mi.fType = MFT_STRING;
	mi.cch = 6;
	mi.dwTypeData = "New...";
	mi.wID = IDC_NEWFAV;
	hubsMenu.InsertMenuItem(1, TRUE, &mi);
	
	mi.fMask = MIIM_ID | MIIM_TYPE;
	mi.fType = MFT_STRING;
	mi.cch = 13;
	mi.dwTypeData = "Properties...";
	mi.wID = IDC_EDIT;
	hubsMenu.InsertMenuItem(1, TRUE, &mi);
	
	bHandled = FALSE;
	return TRUE;
}

LRESULT FavoriteHubsFrame::onDoubleClickHublist(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	if(!checkNick())
		return 0;
	
	NMITEMACTIVATE* item = (NMITEMACTIVATE*) pnmh;

	if(item->iItem != -1) {
		FavoriteHubEntry* entry = (FavoriteHubEntry*)ctrlHubs.GetItemData(item->iItem);
		if(!ClientManager::getInstance()->isConnected(entry->getServer())) {
			HubFrame* frm = new HubFrame(entry->getServer(), entry->getNick(), entry->getPassword());
			frm->setTab(getTab());
			frm->CreateEx(m_hWndMDIClient);
		}
	}

	return 0;
}

LRESULT FavoriteHubsFrame::onClickedConnect(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(!checkNick())
		return 0;
	
	int i = -1;
	while( (i = ctrlHubs.GetNextItem(i, LVNI_SELECTED)) != -1) {
		FavoriteHubEntry* entry = (FavoriteHubEntry*)ctrlHubs.GetItemData(i);
		if(!ClientManager::getInstance()->isConnected(entry->getServer())) {
			HubFrame* frm = new HubFrame(entry->getServer(), entry->getNick(), entry->getPassword());
			frm->setTab(getTab());
			frm->CreateEx(m_hWndMDIClient);
		}
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
		if(dlg.DoModal((HWND)*this))
		{
			ctrlHubs.SetItemText(i, COLUMN_NAME, e->getName().c_str());
			ctrlHubs.SetItemText(i, COLUMN_DESCRIPTION, e->getDescription().c_str());
			ctrlHubs.SetItemText(i, COLUMN_SERVER, e->getServer().c_str());
			ctrlHubs.SetItemText(i, COLUMN_NICK, e->getNick(false).c_str());
			ctrlHubs.SetItemText(i, COLUMN_PASSWORD, string(e->getPassword().size(), '*').c_str());
		}
	}
	return 0;
}

LRESULT FavoriteHubsFrame::onNew(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	FavoriteHubEntry e;
	FavHubProperties dlg(&e);

	if(dlg.DoModal((HWND)*this))
		HubManager::getInstance()->addFavorite(e);
	return 0;
}

/**
 * @file FavoriteHubsFrm.cpp
 * $Id: FavoritesFrm.cpp,v 1.6 2002/02/10 12:25:24 arnetheduck Exp $
 * @if LOG
 * $Log: FavoritesFrm.cpp,v $
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


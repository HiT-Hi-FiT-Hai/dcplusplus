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
#include "ClientManager.h"
#include "LineDlg.h"
#include "FavHubProperties.h"
#include "ResourceManager.h"
#include "StringTokenizer.h"

FavoriteHubsFrame* FavoriteHubsFrame::frame = NULL;

int FavoriteHubsFrame::columnIndexes[] = { COLUMN_NAME, COLUMN_DESCRIPTION, COLUMN_NICK, COLUMN_PASSWORD, COLUMN_SERVER };
int FavoriteHubsFrame::columnSizes[] = { 200, 290, 125, 100, 100 };
static ResourceManager::Strings columnNames[] = { ResourceManager::AUTO_CONNECT, ResourceManager::DESCRIPTION, 
	ResourceManager::NICK, ResourceManager::PASSWORD, ResourceManager::SERVER
};

LRESULT FavoriteHubsFrame::onCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{

	// Only one of this window please...
	dcassert(frame == NULL);
	frame = this;
	
	SetWindowText(CSTRING(FAVORITE_HUBS));
	
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
	
	// Create listview columns
	StringList l = StringTokenizer(SETTING(FAVORITESFRAME_ORDER), ',').getTokens();
	{
		int k = 0;
		for(StringIter i = l.begin(); i != l.end(); ++i) {
			if(k >= COLUMN_LAST)
				break;
			columnIndexes[k++] = Util::toInt(*i);
		}
	}
	
	l = StringTokenizer(SETTING(FAVORITESFRAME_WIDTHS), ',').getTokens();
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
		lvc.fmt = j = LVCFMT_LEFT;
		lvc.cx = columnSizes[j];
		lvc.iOrder = columnIndexes[j];
		lvc.iSubItem = j;
		ctrlHubs.InsertColumn(j, &lvc);
	}
	
	ctrlConnect.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
		BS_PUSHBUTTON , 0, IDC_CONNECT);
	ctrlConnect.SetWindowText(CSTRING(CONNECT));
	ctrlConnect.SetFont(ctrlHubs.GetFont());

	ctrlNew.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
		BS_PUSHBUTTON , 0, IDC_NEWFAV);
	ctrlNew.SetWindowText(CSTRING(NEW));
	ctrlNew.SetFont(ctrlHubs.GetFont());

	HubManager::getInstance()->addListener(this);
	HubManager::getInstance()->getFavoriteHubs();
	
	int n = 0;
	hubsMenu.CreatePopupMenu();
	CMenuItemInfo mi;
	mi.fMask = MIIM_ID | MIIM_TYPE;
	mi.fType = MFT_STRING;
	mi.cch = 7;
	mi.dwTypeData = const_cast<char*>(CSTRING(CONNECT));
	mi.wID = IDC_CONNECT;
	hubsMenu.InsertMenuItem(n++, TRUE, &mi);
	
	mi.fMask = MIIM_ID | MIIM_TYPE;
	mi.fType = MFT_STRING;
	mi.cch = 6;
	mi.dwTypeData = const_cast<char*>(CSTRING(NEW));
	mi.wID = IDC_NEWFAV;
	hubsMenu.InsertMenuItem(n++, TRUE, &mi);

	mi.fMask = MIIM_ID | MIIM_TYPE;
	mi.fType = MFT_STRING;
	mi.cch = 13;
	mi.dwTypeData = const_cast<char*>(CSTRING(PROPERTIES));
	mi.wID = IDC_EDIT;
	hubsMenu.InsertMenuItem(n++, TRUE, &mi);
	
	mi.fMask = MIIM_ID | MIIM_TYPE;
	mi.fType = MFT_STRING;
	mi.cch = 21;
	mi.dwTypeData = const_cast<char*>(CSTRING(REMOVE));
	mi.wID = IDC_REMOVE;
	hubsMenu.InsertMenuItem(n++, TRUE, &mi);
	
	
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
		if(dlg.DoModal(m_hWnd) == IDOK)
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

	if(dlg.DoModal((HWND)*this) == IDOK)
		HubManager::getInstance()->addFavorite(e);
	return 0;
}

/**
 * @file FavoriteHubsFrm.cpp
 * $Id: FavoritesFrm.cpp,v 1.8 2002/03/13 20:35:25 arnetheduck Exp $
 * @if LOG
 * $Log: FavoritesFrm.cpp,v $
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


/* 
 * Copyright (C) 2001 Jacek Sieka, jacek@creatio.se
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

#include "PublicHubsFrm.h"

void PublicHubsFrame::onHub(const string& aName, const string& aServer, const string& aDescription, const string& aUsers) {
	StringList l;
	l.push_back(aName);
	l.push_back(aDescription);
	l.push_back(aUsers);
	l.push_back(aServer);
	ctrlHubs.insert(l);
}

LRESULT PublicHubsFrame::onCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CreateSimpleStatusBar(ATL_IDS_IDLEMESSAGE, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SBARS_SIZEGRIP);
	ctrlStatus.Attach(m_hWndStatusBar);

	ctrlHubs.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS, WS_EX_CLIENTEDGE, IDC_HUBLIST);
	
	ctrlHubs.InsertColumn(0, _T("Name"), LVCFMT_LEFT, 200, 0);
	ctrlHubs.InsertColumn(1, _T("Description"), LVCFMT_LEFT, 290, 1);
	ctrlHubs.InsertColumn(2, _T("Users"), LVCFMT_RIGHT, 50, 2);
	ctrlHubs.InsertColumn(3, _T("Server"), LVCFMT_LEFT, 100, 3);
	
	ctrlHubs.setSort(2, ExListViewCtrl::SORT_INT, false);

	ctrlHub.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		ES_AUTOHSCROLL, WS_EX_CLIENTEDGE);
	
	ctrlConnect.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
		BS_DEFPUSHBUTTON , 0, IDC_CONNECT);
	ctrlConnect.SetWindowText("Connect");

	listing = true;
	HubManager::getInstance()->addListener(this);
	HubManager::getInstance()->getPublicHubs();
	
	return TRUE;
}

LRESULT PublicHubsFrame::onDoubleClickHublist(int idCtrl, LPNMHDR pnmh, BOOL& bHandled) {
	// Do nothing for now...
	return 0;
}

LRESULT PublicHubsFrame::onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	if(listing) {
		close = true;
		HubManager::getInstance()->stopListing();

	}
	else
		DestroyWindow();
	
	return 0;
}

LRESULT PublicHubsFrame::OnItemchangedHublist(int idCtrl, LPNMHDR pnmh, BOOL& bHandled) {
	NM_LISTVIEW* lv = (NM_LISTVIEW*) pnmh;
	
	if(lv->uNewState & LVIS_FOCUSED) {
		char buf[1024];
		ctrlHubs.GetItemText(lv->iItem, 3, buf, 1024);
		SetDlgItemText(IDC_SERVER, buf);
	}
	return 0;
}

LRESULT PublicHubsFrame::onClickedRefresh(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	listing = true;
	HubManager::getInstance()->getPublicHubs(true);

	return 0;
}

LRESULT PublicHubsFrame::onClickedConnect(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {

}

/**
 * @file PublicHubsFrame.cpp
 * $Id: PublicHubsFrm.cpp,v 1.1 2001/12/11 21:17:29 arnetheduck Exp $
 * @if LOG
 * $Log: PublicHubsFrm.cpp,v $
 * Revision 1.1  2001/12/11 21:17:29  arnetheduck
 * Changed the dialog to a frame
 *
 * @endif
 */


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
#include "HubFrame.h"
#include "Client.h"

PublicHubsFrame* PublicHubsFrame::frame = NULL;

void PublicHubsFrame::onHub(const string& aName, const string& aServer, const string& aDescription, const string& aUsers) {
	StringList l;
	l.push_back(aName);
	l.push_back(aDescription);
	l.push_back(aUsers);
	l.push_back(aServer);
	ctrlHubs.insert(l);
	hubs++;
	users += atoi(aUsers.c_str());
	updateStatus();
}

LRESULT PublicHubsFrame::onCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{

	// Only one of this window please...
	dcassert(frame == NULL);
	frame = this;
	
	SetWindowText("Public Hubs");
	
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
	ctrlHub.SetFont(ctrlHubs.GetFont());
	
	ctrlConnect.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
		BS_PUSHBUTTON , 0, IDC_CONNECT);
	ctrlConnect.SetWindowText("Connect");
	ctrlConnect.SetFont(ctrlHubs.GetFont());

	ctrlRefresh.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
		BS_PUSHBUTTON , 0, IDC_REFRESH);
	ctrlRefresh.SetWindowText("Refresh");
	ctrlRefresh.SetFont(ctrlHubs.GetFont());

	ctrlAddress.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	ctrlAddress.SetWindowText("Address");
	ctrlAddress.SetFont(ctrlHubs.GetFont());
	
	listing = true;
	HubManager::getInstance()->addListener(this);
	HubManager::getInstance()->getPublicHubs();
	
	return TRUE;
}

LRESULT PublicHubsFrame::onDoubleClickHublist(int idCtrl, LPNMHDR pnmh, BOOL& bHandled) {
	// Do nothing for now...
	return 0;
}

LRESULT PublicHubsFrame::onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	if(listing) {
		close = true;
		HubManager::getInstance()->stopListing();
	}
	else {
		bHandled = FALSE;
	}
	
	return 0;
}

LRESULT PublicHubsFrame::onClickedRefresh(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	listing = true;
	HubManager::getInstance()->getPublicHubs(true);

	return 0;
}

LRESULT PublicHubsFrame::onClickedConnect(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	char buf[1024];
	
	int i = -1;
	while( (i = ctrlHubs.GetNextItem(i, LVNI_SELECTED)) != -1) {
		ctrlHubs.GetItemText(i, 3, buf, 1024);
		string tmp = buf;
		if(!Client::isConnected(tmp)) {
			HubFrame* frm = new HubFrame(buf);
			frm->CreateEx(GetParent());
		}
	}

	if(ctrlHub.GetWindowTextLength() > 0) {
		ctrlHub.GetWindowText(buf, 1024);
		ctrlHub.SetWindowText("");
		string tmp = buf;
		if(!Client::isConnected(tmp)) {
			HubFrame* frm = new HubFrame(buf);
			frm->CreateEx(GetParent());
		}
		
	}
	return 0;
}

/**
 * @file PublicHubsFrm.cpp
 * $Id: PublicHubsFrm.cpp,v 1.2 2001/12/12 00:06:04 arnetheduck Exp $
 * @if LOG
 * $Log: PublicHubsFrm.cpp,v $
 * Revision 1.2  2001/12/12 00:06:04  arnetheduck
 * Updated the public hub listings, fixed some minor transfer bugs, reworked the
 * sockets to use only one thread (instead of an extra thread for sending files),
 * and fixed a major bug in the client command decoding (still have to fix this
 * one for the userconnections...)
 *
 * Revision 1.1  2001/12/11 21:17:29  arnetheduck
 * Changed the dialog to a frame
 *
 * @endif
 */


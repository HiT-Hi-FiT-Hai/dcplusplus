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

#include "PublicHubsFrm.h"
#include "HubFrame.h"
#include "Client.h"

PublicHubsFrame* PublicHubsFrame::frame = NULL;

LRESULT PublicHubsFrame::onCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{

	// Only one of this window please...
	dcassert(frame == NULL);
	frame = this;
	
	SetWindowText("Public Hubs");
	
	CreateSimpleStatusBar(ATL_IDS_IDLEMESSAGE, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SBARS_SIZEGRIP);
	ctrlStatus.Attach(m_hWndStatusBar);

	ctrlHubs.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS, WS_EX_CLIENTEDGE, IDC_HUBLIST);
	ctrlHubs.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);

	ctrlHubs.InsertColumn(0, _T("Name"), LVCFMT_LEFT, 200, 0);
	ctrlHubs.InsertColumn(1, _T("Description"), LVCFMT_LEFT, 290, 1);
	ctrlHubs.InsertColumn(2, _T("Users"), LVCFMT_RIGHT, 50, 2);
	ctrlHubs.InsertColumn(3, _T("Server"), LVCFMT_LEFT, 100, 3);
	
	ctrlHubs.setSort(2, ExListViewCtrl::SORT_INT, false);

	ctrlHub.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		ES_AUTOHSCROLL, WS_EX_CLIENTEDGE);
	ctrlHub.SetFont(ctrlHubs.GetFont());
	
	ctrlHubContainer.SubclassWindow(ctrlHub.m_hWnd);
	
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
	
	ctrlStatus.SetText(0, "Downloading hub list...");
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
	mi.cch = 16;
	mi.dwTypeData = "Add To Favorites";
	mi.wID = IDC_ADD;
	hubsMenu.InsertMenuItem(1, TRUE, &mi);
	
	HubManager::getInstance()->addListener(this);
	if(HubManager::getInstance()->hasDownloaded()) {
		PostMessage(WM_SPEAKER);
	} else if(!HubManager::getInstance()->isRunning()) {
		HubManager::getInstance()->refresh();
	}
	
	bHandled = FALSE;
	return TRUE;
}

LRESULT PublicHubsFrame::onDoubleClickHublist(int idCtrl, LPNMHDR pnmh, BOOL& bHandled) {
	if(!checkNick())
		return 0;
	
	NMITEMACTIVATE* item = (NMITEMACTIVATE*) pnmh;

	if(item->iItem != -1) {
		char buf[256];
		
		ctrlHubs.GetItemText(item->iItem, COLUMN_SERVER, buf, 256);
		string tmp = buf;
		if(!ClientManager::getInstance()->isConnected(tmp)) {
			HubFrame* frm = new HubFrame(buf);
			frm->setTab(getTab());
			frm->CreateEx(m_hWndMDIClient);
		}
	}

	return 0;
}

LRESULT PublicHubsFrame::onClickedRefresh(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	ctrlHubs.DeleteAllItems();
	users = 0;
	hubs = 0;
	ctrlStatus.SetText(0, "Downloading hub list...");
	HubManager::getInstance()->addListener(this);
	HubManager::getInstance()->refresh();

	return 0;
}

LRESULT PublicHubsFrame::onClickedConnect(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	if(!checkNick())
		return 0;

	char buf[256];
	
	int i = -1;
	while( (i = ctrlHubs.GetNextItem(i, LVNI_SELECTED)) != -1) {
		ctrlHubs.GetItemText(i, COLUMN_SERVER, buf, 256);
		string tmp = buf;
		if(!ClientManager::getInstance()->isConnected(tmp)) {
			HubFrame* frm = new HubFrame(buf);
			frm->setTab(getTab());
			frm->CreateEx(m_hWndMDIClient);
		}
	}

	if(ctrlHub.GetWindowTextLength() > 0) {
		ctrlHub.GetWindowText(buf, 256);
		ctrlHub.SetWindowText("");
		string tmp = buf;
		if(!ClientManager::getInstance()->isConnected(tmp)) {
			HubFrame* frm = new HubFrame(buf);
			frm->setTab(getTab());
			frm->CreateEx(m_hWndMDIClient);
		}
		
	}
	return 0;
}

LRESULT PublicHubsFrame::onAdd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	if(!checkNick())
		return 0;
	
	char buf[256];
	
	int i = -1;
	while( (i = ctrlHubs.GetNextItem(i, LVNI_SELECTED)) != -1) {
		FavoriteHubEntry e;
		ctrlHubs.GetItemText(i, COLUMN_NAME, buf, 256);
		e.setName(buf);
		ctrlHubs.GetItemText(i, COLUMN_DESCRIPTION, buf, 256);
		e.setDescription(buf);
		ctrlHubs.GetItemText(i, COLUMN_SERVER, buf, 256);
		e.setServer(buf);
		HubManager::getInstance()->addFavorite(e);
	}
	
	return 0;
}

LRESULT PublicHubsFrame::OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	char* hub;
	
	if(wParam == VK_RETURN && ctrlHub.GetWindowTextLength() > 0) {
		if(!checkNick()) {
			return 0;
		}

		hub = new char[ctrlHub.GetWindowTextLength()+1];
		ctrlHub.GetWindowText(hub, ctrlHub.GetWindowTextLength()+1);
		string s(hub, ctrlHub.GetWindowTextLength());
		delete hub;
		if(!ClientManager::getInstance()->isConnected(s)) {
			HubFrame* frm = new HubFrame(s);
			frm->setTab(getTab());
			frm->CreateEx(m_hWndMDIClient);
		}
		ctrlHub.SetWindowText("");
	} else {
		bHandled = FALSE;
	}
	return 0;
}

/**
 * @file PublicHubsFrm.cpp
 * $Id: PublicHubsFrm.cpp,v 1.14 2002/01/20 22:54:46 arnetheduck Exp $
 * @if LOG
 * $Log: PublicHubsFrm.cpp,v $
 * Revision 1.14  2002/01/20 22:54:46  arnetheduck
 * Bugfixes to 0.131 mainly...
 *
 * Revision 1.13  2002/01/15 21:57:53  arnetheduck
 * Hopefully fixed the two annoying bugs...
 *
 * Revision 1.12  2002/01/13 22:50:48  arnetheduck
 * Time for 0.12, added favorites, a bunch of new icons and lot's of other stuff
 *
 * Revision 1.11  2002/01/07 20:17:59  arnetheduck
 * Finally fixed the reconnect bug that's been annoying me for a whole day...
 * Hopefully the app works better in w95 now too...
 *
 * Revision 1.10  2002/01/06 21:55:20  arnetheduck
 * Some minor bugs fixed, but there remains one strange thing, the reconnect
 * button doesn't work...
 *
 * Revision 1.9  2002/01/05 19:06:09  arnetheduck
 * Added user list images, fixed bugs and made things more effective
 *
 * Revision 1.7  2002/01/02 16:12:32  arnetheduck
 * Added code for multiple download sources
 *
 * Revision 1.6  2001/12/29 13:47:14  arnetheduck
 * Fixing bugs and UI work
 *
 * Revision 1.5  2001/12/27 12:05:00  arnetheduck
 * Added flat tabs, fixed sorting and a StringTokenizer bug
 *
 * Revision 1.4  2001/12/21 20:21:17  arnetheduck
 * Private messaging added, and a lot of other updates as well...
 *
 * Revision 1.3  2001/12/13 19:21:57  arnetheduck
 * A lot of work done almost everywhere, mainly towards a friendlier UI
 * and less bugs...time to release 0.06...
 *
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


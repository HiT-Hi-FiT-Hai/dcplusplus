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
#include "ResourceManager.h"
#include "StringTokenizer.h"

PublicHubsFrame* PublicHubsFrame::frame = NULL;

int PublicHubsFrame::columnIndexes[] = { COLUMN_NAME, COLUMN_DESCRIPTION, COLUMN_USERS, COLUMN_SERVER };

int PublicHubsFrame::columnSizes[] = { 200, 290, 50, 100 };

static ResourceManager::Strings columnNames[] = { ResourceManager::HUB_NAME, ResourceManager::DESCRIPTION, 
ResourceManager::USERS, ResourceManager::HUB_ADDRESS };

LRESULT PublicHubsFrame::onCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{

	// Only one of this window please...
	dcassert(frame == NULL);
	frame = this;
	
	SetWindowText(CSTRING(PUBLIC_HUBS));
	
	CreateSimpleStatusBar(ATL_IDS_IDLEMESSAGE, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SBARS_SIZEGRIP);
	ctrlStatus.Attach(m_hWndStatusBar);

	ctrlHubs.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS, WS_EX_CLIENTEDGE, IDC_HUBLIST);
	if(BOOLSETTING(FULL_ROW_SELECT)) {
		ctrlHubs.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);
	}
	
	// Create listview columns
	StringList l = StringTokenizer(SETTING(PUBLICHUBSFRAME_ORDER), ',').getTokens();
	{
		int k = 0;
		for(StringIter i = l.begin(); i != l.end(); ++i) {
			if(k >= COLUMN_LAST)
				break;
			columnIndexes[k++] = Util::toInt(*i);
		}
	}
	
	l = StringTokenizer(SETTING(PUBLICHUBSFRAME_WIDTHS), ',').getTokens();
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
		lvc.fmt = ((j == COLUMN_USERS) ? LVCFMT_RIGHT : LVCFMT_LEFT);
		lvc.cx = columnSizes[j];
		lvc.iOrder = columnIndexes[j];
		lvc.iSubItem = j;
		ctrlHubs.InsertColumn(j, &lvc);
	}
	
	ctrlHubs.SetBkColor(Util::bgColor);
	ctrlHubs.SetTextBkColor(Util::bgColor);
	ctrlHubs.SetTextColor(Util::textColor);
	
	ctrlHubs.setSort(2, ExListViewCtrl::SORT_INT, false);

	ctrlHub.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		ES_AUTOHSCROLL, WS_EX_CLIENTEDGE);
	ctrlHub.SetFont(ctrlHubs.GetFont());
	
	ctrlHubContainer.SubclassWindow(ctrlHub.m_hWnd);
	
	ctrlConnect.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
		BS_PUSHBUTTON , 0, IDC_CONNECT);
	ctrlConnect.SetWindowText(CSTRING(CONNECT));
	ctrlConnect.SetFont(ctrlHubs.GetFont());

	ctrlRefresh.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
		BS_PUSHBUTTON , 0, IDC_REFRESH);
	ctrlRefresh.SetWindowText(CSTRING(REFRESH));
	ctrlRefresh.SetFont(ctrlHubs.GetFont());

	ctrlAddress.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	ctrlAddress.SetWindowText(CSTRING(ADDRESS));
	ctrlAddress.SetFont(ctrlHubs.GetFont());
	
	ctrlFilter.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		ES_AUTOHSCROLL, WS_EX_CLIENTEDGE);
	filterContainer.SubclassWindow(ctrlFilter.m_hWnd);
	ctrlFilter.SetFont(ctrlHubs.GetFont());
	
	ctrlFilterDesc.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
		BS_GROUPBOX, WS_EX_TRANSPARENT);
	ctrlFilterDesc.SetWindowText(CSTRING(FILTER));
	ctrlFilterDesc.SetFont(ctrlHubs.GetFont());

	ctrlStatus.SetText(0, CSTRING(DOWNLOADING_HUB_LIST));
	hubsMenu.CreatePopupMenu();

	CMenuItemInfo mi;
	mi.fMask = MIIM_ID | MIIM_TYPE;
	mi.fType = MFT_STRING;
	mi.dwTypeData = const_cast<char*>(CSTRING(CONNECT));
	mi.wID = IDC_CONNECT;
	hubsMenu.InsertMenuItem(0, TRUE, &mi);
	
	mi.fMask = MIIM_ID | MIIM_TYPE;
	mi.fType = MFT_STRING;
	mi.dwTypeData = const_cast<char*>(CSTRING(ADD_TO_FAVORITES));
	mi.wID = IDC_ADD;
	hubsMenu.InsertMenuItem(1, TRUE, &mi);
	
	HubManager::getInstance()->addListener(this);

	hubs = HubManager::getInstance()->getPublicHubs();
	updateList();
	
	bHandled = FALSE;
	return TRUE;
}

LRESULT PublicHubsFrame::onDoubleClickHublist(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
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

LRESULT PublicHubsFrame::onClickedRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	ctrlHubs.DeleteAllItems();
	users = 0;
	visibleHubs = 0;
	ctrlStatus.SetText(0, CSTRING(DOWNLOADING_HUB_LIST));
	HubManager::getInstance()->addListener(this);
	HubManager::getInstance()->refresh();

	return 0;
}

LRESULT PublicHubsFrame::onClickedConnect(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(!checkNick())
		return 0;

	char buf[256];

	if(ctrlHub.GetWindowTextLength() > 0) {
		ctrlHub.GetWindowText(buf, 256);
		ctrlHub.SetWindowText("");
		string tmp = buf;
		if(!ClientManager::getInstance()->isConnected(tmp)) {
			HubFrame* frm = new HubFrame(buf);
			frm->setTab(getTab());
			frm->CreateEx(m_hWndMDIClient);
		}
		
	} else {
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
	}

	return 0;
}

LRESULT PublicHubsFrame::onAdd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
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

LRESULT PublicHubsFrame::onChar(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled) {
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

void PublicHubsFrame::UpdateLayout(BOOL bResizeBars /* = TRUE */) {
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
	rc.top += 56;
	rc.bottom -=28;
	ctrlHubs.MoveWindow(rc);
	
	rc = rect;
	rc.top+=2;
	rc.bottom = rc.top + 46;
	ctrlFilterDesc.MoveWindow(rc);

	rc.top += 16;
	rc.bottom -= 8;
	rc.right -= 8;
	rc.left += 8;
	ctrlFilter.MoveWindow(rc);

	rc = rect;
	rc.bottom -= 2;
	rc.top = rc.bottom - 22;
	rc.left += 140;
	rc.right -= 100;
	ctrlHub.MoveWindow(rc);
	
	rc.left -= 43;
	rc.right = rc.left + 43;
	rc.top += 3;
	ctrlAddress.MoveWindow(rc);
	
	rc = rect;
	rc.bottom -= 2;
	rc.top = rc.bottom - 22;
	
	rc.left += 2;
	rc.right = rc.left + 80;
	ctrlRefresh.MoveWindow(rc);
	
	rc = rect;
	rc.bottom -= 2;
	rc.top = rc.bottom - 22;
	
	rc.left = rc.right - 96;
	rc.right -= 2;
	ctrlConnect.MoveWindow(rc);
}

bool PublicHubsFrame::checkNick() {
	if(SETTING(NICK).empty()) {
		MessageBox(CSTRING(ENTER_NICK));
		return false;
	}
	return true;
}

void PublicHubsFrame::updateList() {
	ctrlHubs.DeleteAllItems();
	users = 0;
	visibleHubs = 0;
	
	ctrlHubs.SetRedraw(FALSE);
	
	for(HubEntry::List::const_iterator i = hubs.begin(); i != hubs.end(); ++i) {
		if( (filter.empty()) ||
			(Util::findSubString(i->getName(), filter) != string::npos) ||
			(Util::findSubString(i->getDescription(), filter) != string::npos) ||
			(Util::findSubString(i->getServer(), filter) != string::npos) ) {

			StringList l;
			l.push_back(i->getName());
			l.push_back(i->getDescription());
			l.push_back(i->getUsers());
			l.push_back(i->getServer());
			ctrlHubs.insert(l);
			visibleHubs++;
			users += Util::toInt(i->getUsers());
		}

	}
	
	ctrlHubs.SetRedraw(TRUE);
	ctrlHubs.Invalidate();

	updateStatus();
	
}

void PublicHubsFrame::updateStatus() {
	ctrlStatus.SetText(1, (STRING(HUBS) + ": " + Util::toString(visibleHubs)).c_str());
	ctrlStatus.SetText(2, (STRING(USERS) + ": " + Util::toString(users)).c_str());
}

LRESULT PublicHubsFrame::onSpeaker(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	if(wParam == FINISHED) {
		hubs = HubManager::getInstance()->getPublicHubs();
		updateList();
	} else if(wParam == STARTING) {
		string* x = (string*)lParam;
		ctrlStatus.SetText(0, (STRING(DOWNLOADING_HUB_LIST) + "(" + (*x) + ")").c_str());
		delete x;
	} else if(wParam == FAILED) {
		string* x = (string*)lParam;
		ctrlStatus.SetText(0, (STRING(DOWNLOAD_FAILED) + ": " + (*x) ).c_str());
		delete x;
	}
	return 0;
}

LRESULT PublicHubsFrame::onFilterChar(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled) {
	char* str;
	
	if(wParam == VK_RETURN) {
		str = new char[ctrlFilter.GetWindowTextLength()+1];
		ctrlFilter.GetWindowText(str, ctrlFilter.GetWindowTextLength()+1);
		filter = string(str, ctrlFilter.GetWindowTextLength());
		delete str;
		updateList();
	} else {
		bHandled = FALSE;
	}
	return 0;
}

/**
 * @file PublicHubsFrm.cpp
 * $Id: PublicHubsFrm.cpp,v 1.18 2002/03/23 01:58:42 arnetheduck Exp $
 * @if LOG
 * $Log: PublicHubsFrm.cpp,v $
 * Revision 1.18  2002/03/23 01:58:42  arnetheduck
 * Work done on favorites...
 *
 * Revision 1.17  2002/02/09 18:13:51  arnetheduck
 * Fixed level 4 warnings and started using new stl
 *
 * Revision 1.16  2002/01/26 21:09:51  arnetheduck
 * Release 0.14
 *
 * Revision 1.15  2002/01/26 12:38:50  arnetheduck
 * Added some user options
 *
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


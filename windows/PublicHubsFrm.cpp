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

#include "PublicHubsFrm.h"
#include "HubFrame.h"

#include "../client/Client.h"
#include "../client/StringTokenizer.h"

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

	int w[3] = { 0, 0, 0};
	ctrlStatus.SetParts(3, w);
	
	ctrlHubs.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL, WS_EX_CLIENTEDGE, IDC_HUBLIST);
	if(BOOLSETTING(FULL_ROW_SELECT)) {
		ctrlHubs.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP);
	} else {
		ctrlHubs.SetExtendedListViewStyle(LVS_EX_HEADERDRAGDROP);
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
	
	ctrlHubs.SetBkColor(WinUtil::bgColor);
	ctrlHubs.SetTextBkColor(WinUtil::bgColor);
	ctrlHubs.SetTextColor(WinUtil::textColor);
	
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

	ctrlAddress.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
		BS_GROUPBOX, WS_EX_TRANSPARENT);
	ctrlAddress.SetWindowText(CSTRING(MANUAL_ADDRESS));
	ctrlAddress.SetFont(ctrlHubs.GetFont());
	
	ctrlFilter.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		ES_AUTOHSCROLL, WS_EX_CLIENTEDGE);
	filterContainer.SubclassWindow(ctrlFilter.m_hWnd);
	ctrlFilter.SetFont(ctrlHubs.GetFont());
	
	ctrlFilterDesc.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
		BS_GROUPBOX, WS_EX_TRANSPARENT);
	ctrlFilterDesc.SetWindowText(CSTRING(FILTER));
	ctrlFilterDesc.SetFont(ctrlHubs.GetFont());

	HubManager::getInstance()->addListener(this);

	if(HubManager::getInstance()->isDownloading()) 
		ctrlStatus.SetText(0, CSTRING(DOWNLOADING_HUB_LIST));

	hubs = HubManager::getInstance()->getPublicHubs();
	updateList();
	
	hubsMenu.CreatePopupMenu();
	hubsMenu.AppendMenu(MF_STRING, IDC_CONNECT, CSTRING(CONNECT));
	hubsMenu.AppendMenu(MF_STRING, IDC_ADD, CSTRING(ADD_TO_FAVORITES));
	
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
			HubFrame* frm = new HubFrame(tmp);
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
			HubFrame* frm = new HubFrame(tmp);
			frm->setTab(getTab());
			frm->CreateEx(m_hWndMDIClient);
		}
		
	} else {
		if(ctrlHubs.GetSelectedCount() == 1) {
			int i = ctrlHubs.GetNextItem(-1, LVNI_SELECTED);
			ctrlHubs.GetItemText(i, COLUMN_SERVER, buf, 256);
			string tmp = buf;
			if(!ClientManager::getInstance()->isConnected(tmp)) {
				HubFrame* frm = new HubFrame(tmp);
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
	
	if(ctrlHubs.GetSelectedCount() == 1) {
		int i = ctrlHubs.GetNextItem(-1, LVNI_SELECTED);
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
	rc.top += 2;
	rc.bottom -=(56);
	ctrlHubs.MoveWindow(rc);

	rc = rect;
	rc.top = rc.bottom - 52;
	rc.bottom = rc.top + 46;
	rc.right -= 100;
	rc.right -= ((rc.right - rc.left) / 2) + 1;
	ctrlFilterDesc.MoveWindow(rc);

	rc.top += 16;
	rc.bottom -= 8;
	rc.right -= 8;
	rc.left += 8;
	ctrlFilter.MoveWindow(rc);

	rc = rect;
	rc.top = rc.bottom - 52;
	rc.bottom = rc.top + 46;
	rc.right -= 100;
	rc.left += ((rc.right - rc.left) / 2) + 1;
	ctrlAddress.MoveWindow(rc);
	
	rc.top += 16;
	rc.bottom -= 8;
	rc.right -= 8;
	rc.left += 8;
	ctrlHub.MoveWindow(rc);
	
	rc = rect;
	rc.bottom -= 2;
	rc.top = rc.bottom - 22;
	rc.left = rc.right - 96;
	rc.right -= 2;
	ctrlConnect.MoveWindow(rc);

	rc.top -= 24;
	rc.bottom -= 24;
	ctrlRefresh.MoveWindow(rc);
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
			ctrlHubs.insert(ctrlHubs.GetItemCount(), l);
			visibleHubs++;
			users += Util::toInt(i->getUsers());
		}
	}
	
	ctrlHubs.SetRedraw(TRUE);
	ctrlHubs.resort();

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
		ctrlStatus.SetText(0, CSTRING(HUB_LIST_DOWNLOADED));
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
		delete[] str;
		updateList();
	} else {
		bHandled = FALSE;
	}
	return 0;
}

LRESULT PublicHubsFrame::onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	RECT rc;                    // client area of window 
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };        // location of mouse click 
	
	// Get the bounding rectangle of the client area. 
	ctrlHubs.GetClientRect(&rc);
	ctrlHubs.ScreenToClient(&pt); 
	
	if (PtInRect(&rc, pt)) 
	{ 
		ctrlHubs.ClientToScreen(&pt);
		hubsMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
		
		return TRUE; 
	}
	
	return FALSE; 
}

/**
 * @file PublicHubsFrm.cpp
 * $Id: PublicHubsFrm.cpp,v 1.5 2002/05/18 11:20:37 arnetheduck Exp $
 */


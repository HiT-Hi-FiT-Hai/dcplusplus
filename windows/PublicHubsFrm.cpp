/*
 * Copyright (C) 2001-2005 Jacek Sieka, arnetheduck on gmail point com
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
#include "WinUtil.h"
#include "PublicHubsListDlg.h"

#include "../client/Client.h"
#include "../client/StringTokenizer.h"
#include "../client/version.h"

int PublicHubsFrame::columnIndexes[] = { 
	COLUMN_NAME,
	COLUMN_DESCRIPTION,
	COLUMN_USERS,
	COLUMN_SERVER,
	COLUMN_COUNTRY,
	COLUMN_SHARED,
	COLUMN_MINSHARE,
	COLUMN_MINSLOTS,
	COLUMN_MAXHUBS,
	COLUMN_MAXUSERS,
	COLUMN_RELIABILITY,
	COLUMN_RATING
 };

int PublicHubsFrame::columnSizes[] = { 200, 290, 50, 100, 100, 100, 100, 100, 100, 100, 100, 100 };

static ResourceManager::Strings columnNames[] = { 
	ResourceManager::HUB_NAME, 
	ResourceManager::DESCRIPTION, 
	ResourceManager::USERS, 
	ResourceManager::HUB_ADDRESS,
	ResourceManager::COUNTRY,
	ResourceManager::SHARED,
	ResourceManager::MIN_SHARE,
	ResourceManager::MIN_SLOTS,
	ResourceManager::MAX_HUBS,
	ResourceManager::MAX_USERS,
	ResourceManager::RELIABILITY,
	ResourceManager::RATING,
	
};

LRESULT PublicHubsFrame::onCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	CreateSimpleStatusBar(ATL_IDS_IDLEMESSAGE, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SBARS_SIZEGRIP);
	ctrlStatus.Attach(m_hWndStatusBar);

	int w[3] = { 0, 0, 0};
	ctrlStatus.SetParts(3, w);
	
	ctrlHubs.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL, WS_EX_CLIENTEDGE, IDC_HUBLIST);
	ctrlHubs.SetExtendedListViewStyle(LVS_EX_LABELTIP | LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT);
	
	// Create listview columns
	WinUtil::splitTokens(columnIndexes, SETTING(PUBLICHUBSFRAME_ORDER), COLUMN_LAST);
	WinUtil::splitTokens(columnSizes, SETTING(PUBLICHUBSFRAME_WIDTHS), COLUMN_LAST);
	
	for(int j=0; j<COLUMN_LAST; j++) {
		int fmt = (j == COLUMN_USERS) ? LVCFMT_RIGHT : LVCFMT_LEFT;
		ctrlHubs.InsertColumn(j, CTSTRING_I(columnNames[j]), fmt, columnSizes[j], j);
	}
	
	ctrlHubs.SetColumnOrderArray(COLUMN_LAST, columnIndexes);
	
	ctrlHubs.SetBkColor(WinUtil::bgColor);
	ctrlHubs.SetTextBkColor(WinUtil::bgColor);
	ctrlHubs.SetTextColor(WinUtil::textColor);
	
	ctrlHubs.setSort(COLUMN_USERS, ExListViewCtrl::SORT_INT, false);
	ctrlHubs.SetFocus();

	ctrlConfigure.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
		BS_PUSHBUTTON , 0, IDC_PUB_LIST_CONFIG);
	ctrlConfigure.SetWindowText(CTSTRING(CONFIGURE));
	ctrlConfigure.SetFont(WinUtil::systemFont);

	ctrlRefresh.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
		BS_PUSHBUTTON , 0, IDC_REFRESH);
	ctrlRefresh.SetWindowText(CTSTRING(REFRESH));
	ctrlRefresh.SetFont(WinUtil::systemFont);

	ctrlLists.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
		BS_GROUPBOX, WS_EX_TRANSPARENT);
	ctrlLists.SetFont(WinUtil::systemFont);
	ctrlLists.SetWindowText(CTSTRING(CONFIGURED_HUB_LISTS));

	ctrlPubLists.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		WS_HSCROLL | WS_VSCROLL | CBS_DROPDOWNLIST, WS_EX_CLIENTEDGE, IDC_PUB_LIST_DROPDOWN);
	ctrlPubLists.SetFont(WinUtil::systemFont, FALSE);
	// populate with values from the settings
	updateDropDown();

	ctrlFilter.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		ES_AUTOHSCROLL, WS_EX_CLIENTEDGE);
	filterContainer.SubclassWindow(ctrlFilter.m_hWnd);
	ctrlFilter.SetFont(WinUtil::systemFont);
	
	ctrlFilterDesc.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
		BS_GROUPBOX, WS_EX_TRANSPARENT);
	ctrlFilterDesc.SetWindowText(CTSTRING(FILTER));
	ctrlFilterDesc.SetFont(WinUtil::systemFont);

	FavoriteManager::getInstance()->addListener(this);

	hubs = FavoriteManager::getInstance()->getPublicHubs();
	if(FavoriteManager::getInstance()->isDownloading()) 
		ctrlStatus.SetText(0, CTSTRING(DOWNLOADING_HUB_LIST));
	else {
		if(hubs.empty())
			FavoriteManager::getInstance()->refresh();
	}

	updateList();
	
	hubsMenu.CreatePopupMenu();
	hubsMenu.AppendMenu(MF_STRING, IDC_CONNECT, CTSTRING(CONNECT));
	hubsMenu.AppendMenu(MF_STRING, IDC_ADD, CTSTRING(ADD_TO_FAVORITES));
	hubsMenu.AppendMenu(MF_STRING, IDC_COPY_HUB, CTSTRING(COPY_HUB));
	hubsMenu.SetMenuDefaultItem(IDC_CONNECT);
	
	bHandled = FALSE;
	return TRUE;
}

LRESULT PublicHubsFrame::onColumnClickHublist(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NMLISTVIEW* l = (NMLISTVIEW*)pnmh;
	if(l->iSubItem == ctrlHubs.getSortColumn()) {
		if (!ctrlHubs.isAscending())
			ctrlHubs.setSort(-1, ctrlHubs.getSortType());
		else
			ctrlHubs.setSortDirection(false);
	} else {
		// BAH, sorting on bytes will break of course...oh well...later...
		if(l->iSubItem == COLUMN_USERS || l->iSubItem == COLUMN_MINSLOTS ||l->iSubItem == COLUMN_MAXHUBS || l->iSubItem == COLUMN_MAXUSERS) {
			ctrlHubs.setSort(l->iSubItem, ExListViewCtrl::SORT_INT);
		} else if(l->iSubItem == COLUMN_SHARED || l->iSubItem == COLUMN_MINSHARE || l->iSubItem == COLUMN_RELIABILITY) {
			ctrlHubs.setSort(l->iSubItem, ExListViewCtrl::SORT_FLOAT);
		} else {
			ctrlHubs.setSort(l->iSubItem, ExListViewCtrl::SORT_STRING_NOCASE);
		}
	}
	return 0;
}

LRESULT PublicHubsFrame::onDoubleClickHublist(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	if(!checkNick())
		return 0;
	
	NMITEMACTIVATE* item = (NMITEMACTIVATE*) pnmh;

	if(item->iItem != -1) {
		TCHAR buf[256];
		
		ctrlHubs.GetItemText(item->iItem, COLUMN_SERVER, buf, 256);
		HubFrame::openWindow(buf);
	}

	return 0;
}

LRESULT PublicHubsFrame::onEnter(int /*idCtrl*/, LPNMHDR /* pnmh */, BOOL& /*bHandled*/) {
	if(!checkNick())
		return 0;

	int item = ctrlHubs.GetNextItem(-1, LVNI_FOCUSED);
	if(item != -1) {
		TCHAR buf[256];

		ctrlHubs.GetItemText(item, COLUMN_SERVER, buf, 256);
		HubFrame::openWindow(buf);
	}

	return 0;
}

LRESULT PublicHubsFrame::onClickedRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	ctrlHubs.DeleteAllItems();
	users = 0;
	visibleHubs = 0;
	ctrlStatus.SetText(0, CTSTRING(DOWNLOADING_HUB_LIST));
	FavoriteManager::getInstance()->refresh();

	return 0;
}

LRESULT PublicHubsFrame::onClickedConfigure(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	PublicHubListDlg dlg;
	if(dlg.DoModal(m_hWnd) == IDOK) {
		updateDropDown();
	}
	return 0;
}

LRESULT PublicHubsFrame::onClickedConnect(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(!checkNick())
		return 0;

	if(ctrlHubs.GetSelectedCount() == 1) {
		TCHAR buf[256];
		int i = ctrlHubs.GetNextItem(-1, LVNI_SELECTED);
		ctrlHubs.GetItemText(i, COLUMN_SERVER, buf, 256);
		HubFrame::openWindow(buf);
	}

	return 0;
}

LRESULT PublicHubsFrame::onFilterFocus(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled) {
	bHandled = true;
	ctrlFilter.SetFocus();
	return 0;
}

LRESULT PublicHubsFrame::onAdd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(!checkNick())
		return 0;
	
	TCHAR buf[256];
	
	if(ctrlHubs.GetSelectedCount() == 1) {
		int i = ctrlHubs.GetNextItem(-1, LVNI_SELECTED);
		FavoriteHubEntry e;
		ctrlHubs.GetItemText(i, COLUMN_NAME, buf, 256);
		e.setName(Text::fromT(buf));
		ctrlHubs.GetItemText(i, COLUMN_DESCRIPTION, buf, 256);
		e.setDescription(Text::fromT(buf));
		ctrlHubs.GetItemText(i, COLUMN_SERVER, buf, 256);
		e.setServer(Text::fromT(buf));
		FavoriteManager::getInstance()->addFavorite(e);
	}
	return 0;
}

LRESULT PublicHubsFrame::onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	if(!closed) {
		FavoriteManager::getInstance()->removeListener(this);
		closed = true;
		PostMessage(WM_CLOSE);
		return 0;
	} else {
		WinUtil::saveHeaderOrder(ctrlHubs, SettingsManager::PUBLICHUBSFRAME_ORDER,
			SettingsManager::PUBLICHUBSFRAME_WIDTHS, COLUMN_LAST, columnIndexes, columnSizes);
		bHandled = FALSE;
		return 0;
	}
}

LRESULT PublicHubsFrame::onListSelChanged(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled) {
	FavoriteManager::getInstance()->setHubList(ctrlPubLists.GetCurSel());
	hubs = FavoriteManager::getInstance()->getPublicHubs();
	updateList();
	bHandled = FALSE;
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
	
	// listview
	CRect rc = rect;
	rc.top += 2;
	rc.bottom -=(56);
	ctrlHubs.MoveWindow(rc);

	// filter box
	rc = rect;
	rc.top = rc.bottom - 52;
	rc.bottom = rc.top + 46;
	rc.right -= 100;
	rc.right -= ((rc.right - rc.left) / 2) + 1;
	ctrlFilterDesc.MoveWindow(rc);

	// filter edit
	rc.top += 16;
	rc.bottom -= 8;
	rc.right -= 8;
	rc.left += 8;
	ctrlFilter.MoveWindow(rc);

	// lists box
	rc = rect;
	rc.top = rc.bottom - 52;
	rc.bottom = rc.top + 46;
	rc.right -= 100;
	rc.left += ((rc.right - rc.left) / 2) + 1;
	ctrlLists.MoveWindow(rc);

	// lists dropdown
	rc.top += 16;
	rc.bottom -= 8 - 140;
	rc.right -= 8 + 100;
	rc.left += 8;
	ctrlPubLists.MoveWindow(rc);

	// configure button
	rc.left = rc.right + 4;
	rc.bottom -= 140;
	rc.right += 100;
	ctrlConfigure.MoveWindow(rc);

	// refresh button
	rc = rect;
	rc.bottom -= 2 + 8 + 4;
	rc.top = rc.bottom - 22;
	rc.left = rc.right - 96;
	rc.right -= 2;
	ctrlRefresh.MoveWindow(rc);
}

bool PublicHubsFrame::checkNick() {
	if(SETTING(NICK).empty()) {
		MessageBox(CTSTRING(ENTER_NICK), _T(APPNAME) _T(" ") _T(VERSIONSTRING), MB_ICONSTOP | MB_OK);
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
		if( filter.getPattern().empty() ||
			filter.match(i->getName()) ||
			filter.match(i->getDescription()) ||
			filter.match(i->getServer()) ) {

			TStringList l;
			l.resize(COLUMN_LAST);
			l[COLUMN_NAME] = Text::toT(i->getName());
			l[COLUMN_DESCRIPTION] = Text::toT(i->getDescription());
			l[COLUMN_USERS] = Text::toT(Util::toString(i->getUsers()));
			l[COLUMN_SERVER] = Text::toT(i->getServer());
			l[COLUMN_COUNTRY] = Text::toT(i->getCountry());
			l[COLUMN_SHARED] = Text::toT(Util::formatBytes(i->getShared()));
			l[COLUMN_MINSHARE] = Text::toT(Util::formatBytes(i->getMinShare()));
			l[COLUMN_MINSLOTS] = Text::toT(Util::toString(i->getMinSlots()));
			l[COLUMN_MAXHUBS] = Text::toT(Util::toString(i->getMaxHubs()));
			l[COLUMN_MAXUSERS] = Text::toT(Util::toString(i->getMaxUsers()));
			l[COLUMN_RELIABILITY] = Text::toT(Util::toString(i->getReliability()));
			l[COLUMN_RATING] = Text::toT(i->getRating());
			ctrlHubs.insert(ctrlHubs.GetItemCount(), l);
			visibleHubs++;
			users += i->getUsers();
		}
	}
	
	ctrlHubs.SetRedraw(TRUE);
	ctrlHubs.resort();

	updateStatus();
}

void PublicHubsFrame::updateStatus() {
	ctrlStatus.SetText(1, Text::toT(STRING(HUBS) + ": " + Util::toString(visibleHubs)).c_str());
	ctrlStatus.SetText(2, Text::toT(STRING(USERS) + ": " + Util::toString(users)).c_str());
}

LRESULT PublicHubsFrame::onSpeaker(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	if(wParam == FINISHED) {
		hubs = FavoriteManager::getInstance()->getPublicHubs();
		updateList();
		tstring* x = (tstring*)lParam;
		ctrlStatus.SetText(0, (TSTRING(HUB_LIST_DOWNLOADED) + _T(" (") + (*x) + _T(")")).c_str());
		delete x;
	} else if(wParam == STARTING) {
		tstring* x = (tstring*)lParam;
		ctrlStatus.SetText(0, (TSTRING(DOWNLOADING_HUB_LIST) + _T(" (") + (*x) + _T(")")).c_str());
		delete x;
	} else if(wParam == FAILED) {
		tstring* x = (tstring*)lParam;
		ctrlStatus.SetText(0, (TSTRING(DOWNLOAD_FAILED) + (*x) ).c_str());
		delete x;
	}
	return 0;
}

LRESULT PublicHubsFrame::onFilterChar(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled) {
	if(wParam == VK_RETURN) {
		AutoArray<TCHAR> str(ctrlFilter.GetWindowTextLength()+1);
		ctrlFilter.GetWindowText(str, ctrlFilter.GetWindowTextLength()+1);
		filter = Text::fromT(tstring(str, ctrlFilter.GetWindowTextLength()));
		updateList();
	} else {
		bHandled = FALSE;
	}
	return 0;
}

LRESULT PublicHubsFrame::onContextMenu(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	if(reinterpret_cast<HWND>(wParam) == ctrlHubs && ctrlHubs.GetSelectedCount() == 1) {
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		
		if(pt.x == -1 && pt.y == -1) {
			WinUtil::getContextMenuPos(ctrlHubs, pt);
		}

		hubsMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
		return TRUE; 
	}
	
	bHandled = FALSE;
	return FALSE; 
}

LRESULT PublicHubsFrame::onCopyHub(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(ctrlHubs.GetSelectedCount() == 1) {
		TCHAR buf[256];
		int i = ctrlHubs.GetNextItem(-1, LVNI_SELECTED);
		ctrlHubs.GetItemText(i, COLUMN_SERVER, buf, 256);
		WinUtil::setClipboard(buf);
	}
	return 0;
}

void PublicHubsFrame::updateDropDown() {
	ctrlPubLists.ResetContent();
	StringList lists(FavoriteManager::getInstance()->getHubLists());
	for(StringList::iterator idx = lists.begin(); idx != lists.end(); ++idx) {
		ctrlPubLists.AddString(Text::toT(*idx).c_str());
	}
	ctrlPubLists.SetCurSel(FavoriteManager::getInstance()->getSelectedHubList());
}

/**
 * @file
 * $Id: PublicHubsFrm.cpp,v 1.41 2006/01/01 17:49:59 arnetheduck Exp $
 */

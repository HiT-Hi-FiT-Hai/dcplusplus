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

#include "TreePropertySheet.h"

static const char SEPARATOR = '\\';

int TreePropertySheet::PropSheetProc(HWND hwndDlg, UINT uMsg, LPARAM lParam) {
	if(uMsg == PSCB_INITIALIZED) {
		::PostMessage(hwndDlg, WM_USER_INITDIALOG, 0, 0);
	}

	return CPropertySheet::PropSheetCallback(hwndDlg, uMsg, lParam);
}

LRESULT TreePropertySheet::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /* bHandled */) {
	hideTab();
	addTree();
	fillTree();
	return 0;
}

void TreePropertySheet::hideTab() {
	CRect rcClient, rcTab, rcPage, rcWindow;
	CWindow tab = GetTabControl();
	CWindow page = IndexToHwnd(0);

	GetClientRect(&rcClient);

	tab.GetWindowRect(&rcTab);
	page.GetClientRect(&rcPage);
	page.MapWindowPoints(m_hWnd,&rcPage);
	GetWindowRect(&rcWindow);
	ScreenToClient(&rcTab);

	ScrollWindow(SPACE_LEFT + TREE_WIDTH + SPACE_MID-rcPage.left, SPACE_TOP-rcPage.top);
	rcWindow.right += SPACE_LEFT + TREE_WIDTH + SPACE_MID - rcPage.left - (rcClient.Width()-rcTab.right) + SPACE_RIGHT;
	rcWindow.bottom += SPACE_TOP - rcPage.top;

	tab.ShowWindow(SW_HIDE);

	MoveWindow(&rcWindow, TRUE);

	tabContainer.SubclassWindow(tab.m_hWnd);
}

void TreePropertySheet::addTree()
{
	// Insert the space to the left
	CRect rcWindow,rcPage;

	HWND page = IndexToHwnd(0);
	::GetWindowRect(page, &rcPage);
	ScreenToClient(&rcPage);

	CRect rc(SPACE_LEFT, rcPage.top, TREE_WIDTH, rcPage.bottom);
	ctrlTree.Create(m_hWnd, rc, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_SHOWSELALWAYS | TVS_DISABLEDRAGDROP, WS_EX_CLIENTEDGE, IDC_PAGE);
}

void TreePropertySheet::fillTree() {
	CTabCtrl tab = GetTabControl();
	int pages = tab.GetItemCount();

	char buf[MAX_NAME_LENGTH];
	TCITEM item;
	item.mask = TCIF_TEXT;
	item.pszText = buf;
	item.cchTextMax = MAX_NAME_LENGTH - 1;

	HTREEITEM first = NULL;
	for(int i = 0; i < pages; ++i) {
		tab.GetItem(i, &item);
		if(i == 0)
			first = createTree(buf, TVI_ROOT, i);
		else
			createTree(buf, TVI_ROOT, i);
	}
	ctrlTree.SelectItem(first);
}

HTREEITEM TreePropertySheet::createTree(const string& str, HTREEITEM parent, int page) {
	TVINSERTSTRUCT tvi;
	tvi.hInsertAfter = TVI_LAST;
	tvi.hParent = parent;

	HTREEITEM first = (parent == TVI_ROOT) ? ctrlTree.GetRootItem() : ctrlTree.GetChildItem(parent);

	string::size_type i = str.find(SEPARATOR);
	if(i == string::npos) {
		// Last dir, the actual page
		HTREEITEM item = findItem(str, first);
		if(item == NULL) {
			// Doesn't exist, add
			tvi.item.mask = TVIF_PARAM | TVIF_TEXT;
			tvi.item.pszText = const_cast<LPSTR>(str.c_str());
			tvi.item.lParam = page;
			item = ctrlTree.InsertItem(&tvi);
			ctrlTree.Expand(parent);
			return item;
		} else {
			// Update page
			if(ctrlTree.GetItemData(item) == -1)
				ctrlTree.SetItemData(item, page);
			return item;
		}
	} else {
		string name = str.substr(0, i);
		HTREEITEM item = findItem(name, first);
		if(item == NULL) {
			// Doesn't exist, add...
			tvi.item.mask = TVIF_PARAM | TVIF_TEXT;
			tvi.item.lParam = -1;
			tvi.item.pszText = const_cast<LPSTR>(name.c_str());
			item = ctrlTree.InsertItem(&tvi);
		} 
		ctrlTree.Expand(parent);
		// Recurse...
		return createTree(str.substr(i+1), item, page);
	}	
}

HTREEITEM TreePropertySheet::findItem(const string& str, HTREEITEM start) {
	char buf[MAX_NAME_LENGTH];

	while(start != NULL) {
		ctrlTree.GetItemText(start, buf, MAX_NAME_LENGTH-1);
		if(str == buf) {
			return start;
		}
		start = ctrlTree.GetNextSiblingItem(start);
	}
	return start;
}

HTREEITEM TreePropertySheet::findItem(int page, HTREEITEM start) {
	while(start != NULL) {
		if(((int)ctrlTree.GetItemData(start)) == page)
			return start;
		HTREEITEM ret = findItem(page, ctrlTree.GetChildItem(start));
		if(ret != NULL)
			return ret;
		start = ctrlTree.GetNextSiblingItem(start);
	}
	return NULL;
}

LRESULT TreePropertySheet::onSelChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /* bHandled */) {
	NMTREEVIEW* nmtv = (NMTREEVIEW*)pnmh;
	int page = nmtv->itemNew.lParam;
	if(page == -1) {
		HTREEITEM next = ctrlTree.GetChildItem(nmtv->itemNew.hItem);
		if(next == NULL) {
			next = ctrlTree.GetNextSiblingItem(nmtv->itemNew.hItem);
			if(next == NULL) {
				next = ctrlTree.GetParentItem(nmtv->itemNew.hItem);
				if(next != NULL)
					next = ctrlTree.GetNextSiblingItem(next);
			}
		}
		if(next != NULL) {
			ctrlTree.SelectItem(next);
		}
	} else {
		if(HwndToIndex(GetActivePage()) != page)
			SetActivePage(page);
	}
	return 0;
}	

LRESULT TreePropertySheet::onSetCurSel(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled) {
	ctrlTree.SelectItem(findItem((int)wParam, ctrlTree.GetRootItem()));
	bHandled = FALSE;
	return 0;
}

/**
* @file
* $Id: TreePropertySheet.cpp,v 1.6 2003/12/03 22:09:22 arnetheduck Exp $
*/

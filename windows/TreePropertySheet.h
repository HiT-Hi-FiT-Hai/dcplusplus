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

#if !defined(AFX_TREEPROPERTYSHEET_H__9B8B3ABC_D165_47D8_AA4B_AF695F7A7D54__INCLUDED_)
#define AFX_TREEPROPERTYSHEET_H__9B8B3ABC_D165_47D8_AA4B_AF695F7A7D54__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"

class TreePropertySheet : public CPropertySheetImpl<TreePropertySheet> {
public:
	enum { WM_USER_INITDIALOG = WM_APP + 501 };
	enum { TAB_MESSAGE_MAP = 13 };
	TreePropertySheet(WTL::_U_STRINGorID title = (LPCTSTR)NULL, UINT uStartPage = 0, HWND hWndParent = NULL) :
		CPropertySheetImpl<TreePropertySheet>(title, uStartPage, hWndParent), tabContainer(WC_TABCONTROL, this, TAB_MESSAGE_MAP) {

		m_psh.pfnCallback = &PropSheetProc;

	}
	
	typedef CPropertySheetImpl<TreePropertySheet> baseClass;
	BEGIN_MSG_MAP(TreePropertySheet)
		MESSAGE_HANDLER(WM_COMMAND, baseClass::OnCommand)
		MESSAGE_HANDLER(WM_USER_INITDIALOG, onInitDialog)
		NOTIFY_HANDLER(IDC_PAGE, TVN_SELCHANGED, onSelChanged)
		NOTIFY_HANDLER(IDC_PAGE, TVN_SELCHANGEDW, onSelChanged)
		CHAIN_MSG_MAP(baseClass)
	ALT_MSG_MAP(TAB_MESSAGE_MAP)
		MESSAGE_HANDLER(TCM_SETCURSEL, onSetCurSel)
	END_MSG_MAP()

	LRESULT onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onSetCurSel(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);

	LRESULT onSelChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled); 
	
	static int CALLBACK PropSheetProc(HWND hwndDlg, UINT uMsg, LPARAM lParam);
private:

	enum {
		SPACE_MID = 5,
		SPACE_TOP = 10,
		SPACE_BOTTOM = 10,
		SPACE_LEFT = 10,
		SPACE_RIGHT = 10,
		TREE_WIDTH = 125,
	};

	enum {
		MAX_NAME_LENGTH = 256
	};

	void hideTab();
	void addTree();
	void fillTree();

	HTREEITEM createTree(const string& str, HTREEITEM parent, int page);
	HTREEITEM findItem(const string& str, HTREEITEM start);
	HTREEITEM findItem(int page, HTREEITEM start);

	CTreeViewCtrl ctrlTree;
	CContainedWindow tabContainer;

};

#endif // !defined(AFX_TREEPROPERTYSHEET_H__9B8B3ABC_D165_47D8_AA4B_AF695F7A7D54__INCLUDED_)

/**
* @file
* $Id: TreePropertySheet.h,v 1.1 2003/10/07 00:38:39 arnetheduck Exp $
*/
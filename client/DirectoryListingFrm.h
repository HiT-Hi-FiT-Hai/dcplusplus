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

#if !defined(AFX_DIRECTORYFRM_H__A7078724_FD85_4F39_8463_5A08A5F45E33__INCLUDED_)
#define AFX_DIRECTORYFRM_H__A7078724_FD85_4F39_8463_5A08A5F45E33__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "AtlCmdBar2.h"

#include "DirectoryListing.h"
#include "ExListViewCtrl.h"

class DirectoryListingFrame : public CMDIChildWindowImpl2<DirectoryListingFrame>, CSplitterImpl<DirectoryListingFrame>
{
public:

	DirectoryListingFrame(DirectoryListing* aList, const string& aName) : dl(aList), name(aName) { };

	DECLARE_FRAME_WND_CLASS(NULL, IDR_MDIDIRECTORY)

	virtual void OnFinalMessage(HWND /*hWnd*/)
	{
		delete dl;
		delete this;
	}

	BEGIN_MSG_MAP(DirectoryListingFrame)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_FORWARDMSG, OnForwardMsg)
		NOTIFY_HANDLER(IDC_DIRECTORIES, TVN_SELCHANGED, onSelChangedDirectories)
		NOTIFY_HANDLER(IDC_DIRECTORIES, TVN_GETDISPINFO, onGetDispInfoDirectories)
		CHAIN_MSG_MAP(CMDIChildWindowImpl2<DirectoryListingFrame>)
		CHAIN_MSG_MAP(CSplitterImpl<DirectoryListingFrame>)
	END_MSG_MAP()

	LRESULT OnForwardMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
		return 0;
	}
	
	void setWindowTitle() {
		SetWindowText(name.c_str());
	}

	LRESULT onSelChangedDirectories(int idCtrl, LPNMHDR pnmh, BOOL& bHandled); 
	LRESULT onGetDispInfoDirectories(int idCtrl, LPNMHDR pnmh, BOOL& bHandled); 
	
	void updateTree(DirectoryListing::Directory* tree, HTREEITEM treeItem);

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);

	LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		// handled, no background painting needed
		return 1;
	}
	
	LRESULT OnSize(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if(wParam != SIZE_MINIMIZED)
			SetSplitterRect();
		
		bHandled = FALSE;
		return 1;
	}
	
private:
	CImageList ctrlImages;
	string name;
	CTreeViewCtrl ctrlTree;
	ExListViewCtrl ctrlList;

	DirectoryListing* dl;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHILDFRM_H__A7078724_FD85_4F39_8463_5A08A5F45E33__INCLUDED_)

/**
 * @file DirectoryListingFrm.h
 * $Id: DirectoryListingFrm.h,v 1.1 2001/11/26 23:40:36 arnetheduck Exp $
 * @if LOG
 * $Log: DirectoryListingFrm.h,v $
 * Revision 1.1  2001/11/26 23:40:36  arnetheduck
 * Downloads!! Now downloads are possible, although the implementation is
 * likely to change in the future...more UI work (splitters...) and some bug
 * fixes. Only user file listings are downloadable, but at least it's something...
 *
 * @endif
 */

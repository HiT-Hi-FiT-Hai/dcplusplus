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

/*
 * Automatic Directory Listing Search
 * Henrik Engstr�m, henrikengstrom@home.se
 */

#if !defined(__ADL_SEARCH_FRAME_H__)
#define __ADL_SEARCH_FRAME_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FlatTabCtrl.h"
#include "ExListViewCtrl.h"
#include "WinUtil.h"
#include "../client/ADLSearch.h"

#define ADLLIST_MESSAGE_MAP 6

///////////////////////////////////////////////////////////////////////////////
//
//	Class that represent an ADL search manager interface
//
///////////////////////////////////////////////////////////////////////////////
class ADLSearchFrame : public MDITabChildWindowImpl<ADLSearchFrame>
{
public:

	// Base class typedef
	typedef MDITabChildWindowImpl<ADLSearchFrame> baseClass;

	// Static instance
	static ADLSearchFrame* frame;

	// Constructor/destructor
	ADLSearchFrame() : listContainer("list", this, ADLLIST_MESSAGE_MAP) {}
	virtual ~ADLSearchFrame() { }

	// Frame window declaration
	DECLARE_FRAME_WND_CLASS_EX("ADLSearchFrame", IDR_ADLSEARCH, 0, COLOR_3DFACE);

	// Inline message map
	BEGIN_MSG_MAP(ADLSearchFrame)
		MESSAGE_HANDLER(WM_CREATE, onCreate)
		MESSAGE_HANDLER(WM_CLOSE, onClose)
		MESSAGE_HANDLER(WM_FORWARDMSG, onForwardMsg)
		MESSAGE_HANDLER(WM_CTLCOLOREDIT, onCtlColor)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, onCtlColor)
		MESSAGE_HANDLER(WM_CONTEXTMENU, onContextMenu)
		COMMAND_HANDLER(IDC_ADD, BN_CLICKED, onAdd)
		COMMAND_HANDLER(IDC_EDIT, BN_CLICKED, onEdit)
		COMMAND_HANDLER(IDC_REMOVE, BN_CLICKED, onRemove)
		COMMAND_HANDLER(IDC_HELP_FAQ, BN_CLICKED, onHelp)
		COMMAND_HANDLER(IDC_MOVE_UP, BN_CLICKED, onMoveUp)
		COMMAND_HANDLER(IDC_MOVE_DOWN, BN_CLICKED, onMoveDown)
		NOTIFY_HANDLER(IDC_ADLLIST, NM_DBLCLK, onDoubleClickList)
		NOTIFY_HANDLER(IDC_ADLLIST, LVN_ITEMCHANGED, onItemChanged)
		CHAIN_MSG_MAP(baseClass)
	ALT_MSG_MAP(ADLLIST_MESSAGE_MAP)
		MESSAGE_HANDLER(WM_KEYDOWN, onChar)
	END_MSG_MAP()

	// Message handlers
	LRESULT onCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onAdd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT onEdit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT onRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT onHelp(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT onMoveUp(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT onMoveDown(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT onDoubleClickList(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT onItemChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);
	LRESULT onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled);
	LRESULT onChar(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled);
	
	// Update colors
	LRESULT onCtlColor(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled) 
	{
		HWND hWnd = (HWND)lParam;
		HDC hDC   = (HDC)wParam;
		if(hWnd == ctrlList.m_hWnd) 
		{
			::SetBkColor(hDC, WinUtil::bgColor);
			::SetTextColor(hDC, WinUtil::textColor);
			return (LRESULT)WinUtil::bgBrush;
		}
		bHandled = FALSE;
		return FALSE;
	};

	// Forward message
	LRESULT onForwardMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) 
	{
		return baseClass::PreTranslateMessage((LPMSG)lParam);
	}

	// Final message
	virtual void onFinalMessage(HWND /*hWnd*/) 
	{
		frame = NULL;
		delete this;
	}

	// Update control layouts
	void UpdateLayout(BOOL bResizeBars = TRUE);
	
private:

	// Communication with manager
	void LoadAll();
	void UpdateSearch(int index, BOOL doDelete = TRUE);

	// Contained controls
	CStatusBarCtrl ctrlStatus;
	ExListViewCtrl ctrlList;
	CButton ctrlAdd;
	CButton ctrlEdit;
	CButton ctrlRemove;
	CButton ctrlMoveUp;
	CButton ctrlMoveDown;
	CButton ctrlHelp;
	CMenu contextMenu;
	CContainedWindow listContainer;

	// Column order
	enum 
	{
		COLUMN_FIRST = 0,
		COLUMN_ACTIVE_SEARCH_STRING = COLUMN_FIRST,
		COLUMN_SOURCE_TYPE,
		COLUMN_DEST_DIR,
		COLUMN_MIN_FILE_SIZE,
		COLUMN_MAX_FILE_SIZE,
		COLUMN_LAST
	};

	// Column parameters
	static int columnIndexes[];
	static int columnSizes[];
};

#endif

/**
 * @file
 * $Id: ADLSearchFrame.h,v 1.3 2003/05/07 09:52:09 arnetheduck Exp $
 */

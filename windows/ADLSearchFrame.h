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
 * Henrik Engström, henrikengstrom@home.se
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

///////////////////////////////////////////////////////////////////////////////
//
//	Class a simple edit control used for inline editing in list control
//
///////////////////////////////////////////////////////////////////////////////
class CInlineEdit : public CWindowImpl<CInlineEdit, CEdit>
{
public:

	// Base class typedef
	typedef CWindowImpl<CInlineEdit, CEdit> baseClass;

	// Constructor/destructor
	CInlineEdit() { }
	virtual ~CInlineEdit() { }

	// Inline message map
	BEGIN_MSG_MAP(CInlineEdit)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)
	END_MSG_MAP()

	// Key pressed
	LRESULT OnKeyDown(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		// Key-dependant processing
		if(wParam == VK_RETURN)
		{
			// Get edited text
			char text[256];
			GetWindowText(text, 255);

			// Prepare to send LVN_ENDLABELEDIT command to frame
			LV_DISPINFO dispInfo;
			dispInfo.hdr.hwndFrom    = GetParent();
			dispInfo.hdr.idFrom      = IDC_ADLLIST; 
			dispInfo.hdr.code        = LVN_ENDLABELEDIT;
			dispInfo.item.mask       = LVIF_TEXT;
			dispInfo.item.iItem      = iItem;
			dispInfo.item.iSubItem   = iSubItem;
			dispInfo.item.pszText    = text;
			dispInfo.item.cchTextMax = strlen(text);

			// Send to frame (two levels up)
			::SendMessage(::GetParent(GetParent()), WM_NOTIFY, (WPARAM)0, (LPARAM)&dispInfo);

			// Take down window
			DestroyWindow();
		}
		else
		if(wParam == VK_ESCAPE)
		{
			// Hit escape, take down window
			DestroyWindow();
		}

		bHandled = FALSE;
		return 0;
	}

	// Focus lost
	LRESULT OnKillFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		// Focus lost, simulate enter key
		OnKeyDown(0, VK_RETURN, 0, bHandled);
		return 0;
	}

	// Owner variables
	int iItem;
	int iSubItem;
};

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
	ADLSearchFrame() { }
	virtual ~ADLSearchFrame() { }

	// Frame window declaration
	DECLARE_FRAME_WND_CLASS_EX("ADLSearchFrame", IDR_ADLSEARCH, 0, COLOR_3DFACE);

	// Inline message map
	BEGIN_MSG_MAP(ADLSearchFrame)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_FORWARDMSG, OnForwardMsg)
		MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
		MESSAGE_HANDLER(WM_CTLCOLOREDIT, OnCtlColor)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColor)
		COMMAND_HANDLER(IDC_ADD, BN_CLICKED, OnAdd)
		COMMAND_HANDLER(IDC_REMOVE, BN_CLICKED, OnRemove)
		COMMAND_HANDLER(IDC_MOVE_UP, BN_CLICKED, OnMoveUp)
		COMMAND_HANDLER(IDC_MOVE_DOWN, BN_CLICKED, OnMoveDown)
		NOTIFY_HANDLER(IDC_ADLLIST, NM_DBLCLK, OnDoubleClickList)
		NOTIFY_HANDLER(IDC_ADLLIST, LVN_ENDLABELEDIT, OnEndLabelEditList)
		CHAIN_MSG_MAP(baseClass)
	END_MSG_MAP()

	// Message handlers
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT OnContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);	
	LRESULT OnAdd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnMoveUp(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnMoveDown(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnDoubleClickList(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnEndLabelEditList(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	// Update colors
	LRESULT OnCtlColor(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled) 
	{
		HWND hWnd = (HWND)lParam;
		HDC hDC   = (HDC)wParam;
		if(hWnd == ctrlList.m_hWnd || hWnd == inlineEdit.m_hWnd) 
		{
			::SetBkColor(hDC, WinUtil::bgColor);
			::SetTextColor(hDC, WinUtil::textColor);
			return (LRESULT)WinUtil::bgBrush;
		}
		bHandled = FALSE;
		return FALSE;
	};

	// Forward message
	LRESULT OnForwardMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) 
	{
		return baseClass::PreTranslateMessage((LPMSG)lParam);
	}

	// Final message
	virtual void OnFinalMessage(HWND /*hWnd*/) 
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
	CMenu contextMenu;

	// Sub-item editing
	CInlineEdit inlineEdit;
	void EditSubItem(int iItem, int iSubItem);

	// Column order
	enum 
	{
		COLUMN_FIRST = 0,
		COLUMN_SEARCH_STRING = COLUMN_FIRST,
		COLUMN_SOURCE_TYPE,
		COLUMN_DEST_DIR,
		COLUMN_IS_ACTIVE,
		COLUMN_MIN_FILE_SIZE,
		COLUMN_MAX_FILE_SIZE,
		COLUMN_LAST
	};

	// Column parameters
	static int columnIndexes[];
	static int columnSizes[];
};

#endif


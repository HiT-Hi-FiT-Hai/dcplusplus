/*
 * Copyright (C) 2001-2007 Jacek Sieka, arnetheduck on gmail point com
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

#ifndef DCPLUSPLUS_WIN32_ADL_SEARCH_FRAME_H
#define DCPLUSPLUS_WIN32_ADL_SEARCH_FRAME_H

#include "StaticFrame.h"
#include <dcpp/ADLSearch.h>

class ADLSearchFrame : public StaticFrame<ADLSearchFrame> {
public:
	enum Status {
		STATUS_STATUS,
		STATUS_LAST
	};

	static const ResourceManager::Strings TITLE_RESOURCE = ResourceManager::ADL_SEARCH;

protected:
	//typedef MDIChildFrame<ADLSearchFrame> Base;
	friend class StaticFrame<ADLSearchFrame>;
	friend class MDIChildFrame<ADLSearchFrame>;
	
	ADLSearchFrame(SmartWin::Widget* mdiParent);
	virtual ~ADLSearchFrame();

	void layout();
	void UpdateLayout(BOOL bResizeBars = TRUE);

private:
	enum {
		COLUMN_FIRST,
		COLUMN_ACTIVE_SEARCH_STRING = COLUMN_FIRST,
		COLUMN_SOURCE_TYPE,
		COLUMN_DEST_DIR,
		COLUMN_MIN_FILE_SIZE,
		COLUMN_MAX_FILE_SIZE,
		COLUMN_LAST
	};

	static int columnSizes[COLUMN_LAST];
	static int columnIndexes[COLUMN_LAST];

	WidgetDataGridPtr items;
	WidgetButtonPtr add;
	WidgetButtonPtr remove;
	WidgetButtonPtr properties;
	WidgetButtonPtr up;
	WidgetButtonPtr down;
	WidgetButtonPtr help;
	WidgetMenuPtr contextMenu;
	
	void handleAdd(WidgetButtonPtr);
	void handleRemove(WidgetButtonPtr);
	void handleProperties(WidgetButtonPtr);
	void handleUp(WidgetButtonPtr);
	void handleDown(WidgetButtonPtr);
	void handleHelp(WidgetButtonPtr);
//	void handleCheckBox(WidgetButtonPtr);

	void popupNew(WidgetMenuPtr, unsigned);

	void LoadAll();
	void UpdateSearch(int index, BOOL doDelete);
	
	bool preClosing();
//	virtual void on(FavoriteAdded, const FavoriteHubEntryPtr e) throw();
//	virtual void on(FavoriteRemoved, const FavoriteHubEntryPtr e) throw();
};

#ifdef PORT_ME

#include "../client/ADLSearch.h"

///////////////////////////////////////////////////////////////////////////////
//
//	Class that represent an ADL search manager interface
//
///////////////////////////////////////////////////////////////////////////////
class ADLSearchFrame : public MDITabChildWindowImpl<ADLSearchFrame>, public StaticFrame<ADLSearchFrame, ResourceManager::ADL_SEARCH>
{
public:

	// Base class typedef
	typedef MDITabChildWindowImpl<ADLSearchFrame> baseClass;

	// Constructor/destructor
	ADLSearchFrame() {}
	virtual ~ADLSearchFrame() { }

	// Frame window declaration
	DECLARE_FRAME_WND_CLASS_EX(_T("ADLSearchFrame"), IDR_ADLSEARCH, 0, COLOR_3DFACE);

	// Inline message map
	BEGIN_MSG_MAP(ADLSearchFrame)
		MESSAGE_HANDLER(WM_CREATE, onCreate)
		MESSAGE_HANDLER(WM_CLOSE, onClose)
		MESSAGE_HANDLER(WM_CTLCOLOREDIT, onCtlColor)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, onCtlColor)
		MESSAGE_HANDLER(WM_CONTEXTMENU, onContextMenu)
		MESSAGE_HANDLER(WM_HELP, onHelpKey)
		COMMAND_ID_HANDLER(IDC_ADD, onAdd)
		COMMAND_ID_HANDLER(IDC_EDIT, onEdit)
		COMMAND_ID_HANDLER(IDC_REMOVE, onRemove)
		COMMAND_ID_HANDLER(IDC_HELP_FAQ, onHelpButton)
		COMMAND_ID_HANDLER(IDC_MOVE_UP, onMoveUp)
		COMMAND_ID_HANDLER(IDC_MOVE_DOWN, onMoveDown)
		NOTIFY_HANDLER(IDC_ADLLIST, NM_DBLCLK, onDoubleClickList)
		NOTIFY_HANDLER(IDC_ADLLIST, LVN_ITEMCHANGED, onItemChanged)
		NOTIFY_HANDLER(IDC_ADLLIST, LVN_KEYDOWN, onKeyDown)
		CHAIN_MSG_MAP(baseClass)
	END_MSG_MAP()

	// Message handlers
	LRESULT onCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onAdd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT onEdit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT onRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT onHelpButton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onHelpKey(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onMoveUp(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT onMoveDown(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT onDoubleClickList(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT onItemChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);
	LRESULT onKeyDown(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);
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
#endif // !defined(ADL_SEARCH_FRAME_H)

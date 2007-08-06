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
#include "resource.h"

class ADLSearchFrame : public StaticFrame<ADLSearchFrame> {
public:
	enum Status {
		STATUS_STATUS,
		STATUS_LAST
	};

	static const ResourceManager::Strings TITLE_RESOURCE = ResourceManager::ADL_SEARCH;
	static const unsigned ICON_RESOURCE = IDR_ADLSEARCH;

protected:
	typedef StaticFrame<ADLSearchFrame> BaseType;
	friend class StaticFrame<ADLSearchFrame>;
	friend class MDIChildFrame<ADLSearchFrame>;
	
	ADLSearchFrame(SmartWin::WidgetMDIParent* mdiParent);
	virtual ~ADLSearchFrame();

	void layout();

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
	
	void handleAdd();
	void handleRemove();
	void handleProperties();
	void handleUp();
	void handleDown();
	void handleHelp();
//	void handleCheckBox(WidgetButtonPtr);

	void popupNew();

	void LoadAll();
	void UpdateSearch(int index, BOOL doDelete);
	
	bool preClosing();
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

};

#endif 
#endif // !defined(ADL_SEARCH_FRAME_H)

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

#ifndef DCPLUSPLUS_WIN32_FAV_HUBS_FRAME_H
#define DCPLUSPLUS_WIN32_FAV_HUBS_FRAME_H

#include "StaticFrame.h"

#include <dcpp/FavoriteManagerListener.h>

class FavHubsFrame : 
	public StaticFrame<FavHubsFrame>, 
	private FavoriteManagerListener 
{
public:
	enum Status {
		STATUS_STATUS,
		STATUS_DUMMY,
		STATUS_LAST
	};
	
	static const ResourceManager::Strings TITLE_RESOURCE = ResourceManager::FAVORITE_HUBS;

protected:
	friend class StaticFrame<FavHubsFrame>;
	friend class MDIChildFrame<FavHubsFrame>;
	
	FavHubsFrame(SmartWin::Widget* mdiParent);
	virtual ~FavHubsFrame();

	void layout();

private:
	enum {
		COLUMN_FIRST,
		COLUMN_NAME = COLUMN_FIRST,
		COLUMN_DESCRIPTION,
		COLUMN_NICK,
		COLUMN_PASSWORD,
		COLUMN_SERVER,
		COLUMN_USERDESCRIPTION,
		COLUMN_LAST
	};

	static int columnSizes[COLUMN_LAST];
	static int columnIndexes[COLUMN_LAST];

	WidgetDataGridPtr hubs;
	WidgetButtonPtr connect;
	WidgetButtonPtr add;
	WidgetButtonPtr remove;
	WidgetButtonPtr properties;
	WidgetButtonPtr up;
	WidgetButtonPtr down;
	
	void handleConnect(WidgetButtonPtr);
	void handleAdd(WidgetButtonPtr);
	void handleRemove(WidgetButtonPtr);
	void handleProperties(WidgetButtonPtr);
	void handleUp(WidgetButtonPtr);
	void handleDown(WidgetButtonPtr);

	void openSelected();
	void addEntry(const FavoriteHubEntryPtr entry, int pos);
	
	virtual void on(FavoriteAdded, const FavoriteHubEntryPtr e) throw();
	virtual void on(FavoriteRemoved, const FavoriteHubEntryPtr e) throw();
};

#ifdef PORT_ME

class FavoriteHubsFrame : public MDITabChildWindowImpl<FavoriteHubsFrame>, public StaticFrame<FavoriteHubsFrame, ResourceManager::FAVORITE_HUBS>,
	private FavoriteManagerListener
{
public:
	DECLARE_FRAME_WND_CLASS_EX(_T("FavoriteHubsFrame"), IDR_FAVORITES, 0, COLOR_3DFACE);

	BEGIN_MSG_MAP(FavoriteHubsFrame)
		MESSAGE_HANDLER(WM_CREATE, onCreate)
		MESSAGE_HANDLER(WM_CLOSE, onClose)
		MESSAGE_HANDLER(WM_CONTEXTMENU, onContextMenu)
		MESSAGE_HANDLER(WM_SETFOCUS, onSetFocus)
		COMMAND_ID_HANDLER(IDC_CONNECT, onClickedConnect)
		COMMAND_ID_HANDLER(IDC_REMOVE, onRemove)
		COMMAND_ID_HANDLER(IDC_EDIT, onEdit)
		COMMAND_ID_HANDLER(IDC_NEWFAV, onNew)
		COMMAND_ID_HANDLER(IDC_MOVE_UP, onMoveUp);
		COMMAND_ID_HANDLER(IDC_MOVE_DOWN, onMoveDown);
//		NOTIFY_HANDLER(IDC_HUBLIST, LVN_COLUMNCLICK, onColumnClickHublist)
		NOTIFY_HANDLER(IDC_HUBLIST, NM_DBLCLK, onDoubleClickHublist)
		NOTIFY_HANDLER(IDC_HUBLIST, LVN_KEYDOWN, onKeyDown)
		NOTIFY_HANDLER(IDC_HUBLIST, LVN_ITEMCHANGED, onItemChanged)
		CHAIN_MSG_MAP(MDITabChildWindowImpl<FavoriteHubsFrame>)
	END_MSG_MAP()

	LRESULT onCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onDoubleClickHublist(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);
	LRESULT onKeyDown(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT onEdit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT onRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT onNew(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT onItemChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);
	LRESULT onMoveUp(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT onMoveDown(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);

	bool checkNick();

	LRESULT onEnter(int /*idCtrl*/, LPNMHDR /* pnmh */, BOOL& /*bHandled*/) {
		openSelected();
		return 0;
	}

	LRESULT onClickedConnect(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		openSelected();
		return 0;
	}

private:

	CMenu hubsMenu;

	bool nosave;
};

#endif // !defined(FAVORITE_HUBS_FRM_H)
#endif

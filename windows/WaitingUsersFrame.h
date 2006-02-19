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


#if !defined(WAITING_QUEUE_FRAME_H)
#define WAITING_QUEUE_FRAME_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FlatTabCtrl.h"
#include "ExListViewCtrl.h"
#include "WinUtil.h"
#include "../client/UploadManager.h"

#define UPLOADQUEUE_MESSAGE_MAP 666

class WaitingUsersFrame : public MDITabChildWindowImpl<WaitingUsersFrame>, public UploadManagerListener, public StaticFrame<WaitingUsersFrame, ResourceManager::WAITING_USERS>
{
public:

	// Base class typedef
	typedef MDITabChildWindowImpl<WaitingUsersFrame> baseClass;

	// Constructor
	WaitingUsersFrame() {
		UploadManager::getInstance()->addListener(this);
	}

	// Frame window declaration
	DECLARE_FRAME_WND_CLASS_EX(_T("WaitingUsersFrame"), IDR_WAITING_USERS, 0, COLOR_3DFACE);

	// Inline message map
	BEGIN_MSG_MAP(WaitingUsersFrame)
		MESSAGE_HANDLER(WM_CREATE, onCreate)
		MESSAGE_HANDLER(WM_CLOSE, onClose)
		MESSAGE_HANDLER(WM_CTLCOLOREDIT, onCtlColor)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, onCtlColor)
		MESSAGE_HANDLER(WM_CONTEXTMENU, onContextMenu)
		MESSAGE_HANDLER(WM_SPEAKER, onSpeaker)
		COMMAND_HANDLER(IDC_GETLIST, BN_CLICKED, onGetList)
		COMMAND_HANDLER(IDC_COPY_FILENAME, BN_CLICKED, onCopyFilename)
		COMMAND_HANDLER(IDC_REMOVE, BN_CLICKED, onRemove)
		COMMAND_HANDLER(IDC_GRANTSLOT, BN_CLICKED, onGrantSlot)
		COMMAND_HANDLER(IDC_ADD_TO_FAVORITES, BN_CLICKED, onAddToFavorites)
		COMMAND_HANDLER(IDC_PRIVATEMESSAGE, BN_CLICKED, onPrivateMessage)
		CHAIN_MSG_MAP(baseClass)
		ALT_MSG_MAP(UPLOADQUEUE_MESSAGE_MAP)
		MESSAGE_HANDLER(WM_KEYDOWN, onChar)
	END_MSG_MAP()

	// Message handlers
	LRESULT onCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onGetList(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT onCopyFilename(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled);
	LRESULT onSpeaker(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onChar(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onPrivateMessage(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onGrantSlot(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onAddToFavorites(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	void onRemoveUser(const User::Ptr);
	void onAddFile(const User::Ptr, const string&);

	// Update colors
	LRESULT onCtlColor(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled) 
	{
		HWND hWnd = (HWND)lParam;
		HDC hDC   = (HDC)wParam;
		if(hWnd == ctrlQueued.m_hWnd) 
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
	enum {
		SPEAK_ADD_FILE,
		SPEAK_REMOVE_USER
	};

	bool closed;

	struct UserPtr {
		User::Ptr u;
		UserPtr(User::Ptr u) : u(u) { }
	};

	User::Ptr getSelectedUser() {
		HTREEITEM selectedItem = GetParentItem();
		return selectedItem?reinterpret_cast<UserPtr *>(ctrlQueued.GetItemData(selectedItem))->u:User::Ptr(0);
	}

	// Communication with manager
	void LoadAll();
	void UpdateSearch(int index, BOOL doDelete = TRUE);

	// UploadManagerListener
	virtual void on(UploadManagerListener::WaitingRemoveUser, const User::Ptr) throw();
	virtual void on(UploadManagerListener::WaitingAddFile, const User::Ptr, const string&) throw();

	HTREEITEM GetParentItem();

	// Contained controls
	CTreeViewCtrl ctrlQueued;
	CMenu contextMenu;
};

#endif	/* WAITING_QUEUE_FRAME_H */

/**
 * @file
 * $Id$
 */

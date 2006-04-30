/*
 * Copyright (C) 2001-2006 Jacek Sieka, arnetheduck on gmail point com
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

#if !defined(USERS_FRAME_H)
#define USERS_FRAME_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FlatTabCtrl.h"
#include "TypedListViewCtrl.h"
#include "WinUtil.h"

#include "../client/FavoriteManager.h"

class UsersFrame : public MDITabChildWindowImpl<UsersFrame>, public StaticFrame<UsersFrame, ResourceManager::FAVORITE_USERS>,
	private FavoriteManagerListener, public UserInfoBaseHandler<UsersFrame> {
public:
	
	UsersFrame() : closed(false), startup(true) { }
	virtual ~UsersFrame() { }

	DECLARE_FRAME_WND_CLASS_EX(_T("UsersFrame"), IDR_USERS, 0, COLOR_3DFACE);
		
	typedef MDITabChildWindowImpl<UsersFrame> baseClass;
	typedef UserInfoBaseHandler<UsersFrame> uibBase;

	BEGIN_MSG_MAP(UsersFrame)
		NOTIFY_HANDLER(IDC_USERS, LVN_GETDISPINFO, ctrlUsers.onGetDispInfo)
		NOTIFY_HANDLER(IDC_USERS, LVN_COLUMNCLICK, ctrlUsers.onColumnClick)
		NOTIFY_HANDLER(IDC_USERS, LVN_ITEMCHANGED, onItemChanged)
		MESSAGE_HANDLER(WM_CREATE, onCreate)
		MESSAGE_HANDLER(WM_CLOSE, onClose)
		MESSAGE_HANDLER(WM_CONTEXTMENU, onContextMenu)
		MESSAGE_HANDLER(WM_SPEAKER, onSpeaker)
		MESSAGE_HANDLER(WM_SETFOCUS, onSetFocus)
		COMMAND_ID_HANDLER(IDC_REMOVE, onRemove)
		COMMAND_ID_HANDLER(IDC_EDIT, onEdit)
		COMMAND_ID_HANDLER(IDC_CONNECT, onConnect)
		CHAIN_MSG_MAP(uibBase)
		CHAIN_MSG_MAP(baseClass)
	END_MSG_MAP()
		
	LRESULT onCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT onEdit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT onItemChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);
	LRESULT onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT onConnect(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	void UpdateLayout(BOOL bResizeBars = TRUE);
	
	LRESULT onSpeaker(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	
	LRESULT onSetFocus(UINT /* uMsg */, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		ctrlUsers.SetFocus();
		return 0;
	}

private:
	class UserInfo;
public:
	TypedListViewCtrl<UserInfo, IDC_USERS>& getUserList() { return ctrlUsers; }
private:
	enum {
		COLUMN_FIRST,
		COLUMN_NICK = COLUMN_FIRST,
		COLUMN_HUB,
		COLUMN_SEEN,
		COLUMN_DESCRIPTION,
		COLUMN_CID,
		COLUMN_LAST
	};

	enum {
		USER_UPDATED
	};

	class UserInfo : public UserInfoBase {
	public:
		UserInfo(const FavoriteUser& u) : UserInfoBase(u.getUser()) { 
			update(u);
		}

		const tstring& getText(int col) const {
			return columns[col];
		}

		static int compareItems(UserInfo* a, UserInfo* b, int col) {
			return lstrcmpi(a->columns[col].c_str(), b->columns[col].c_str());
		}

		void remove() { FavoriteManager::getInstance()->removeFavoriteUser(user); }

		void update(const FavoriteUser& u);

		tstring columns[COLUMN_LAST];
	};

	CStatusBarCtrl ctrlStatus;
	CMenu usersMenu;
	
	TypedListViewCtrl<UserInfo, IDC_USERS> ctrlUsers;

	bool closed;
	
	bool startup;
	static int columnSizes[COLUMN_LAST];
	static int columnIndexes[COLUMN_LAST];

	// FavoriteManagerListener
	virtual void on(UserAdded, const FavoriteUser& aUser) throw() { addUser(aUser); }
	virtual void on(UserRemoved, const FavoriteUser& aUser) throw() { removeUser(aUser); }
	virtual void on(StatusChanged, const User::Ptr& aUser) throw() { PostMessage(WM_SPEAKER, (WPARAM)USER_UPDATED, (LPARAM)new UserInfoBase(aUser)); }

	void addUser(const FavoriteUser& aUser);
	void updateUser(const User::Ptr& aUser);
	void removeUser(const FavoriteUser& aUser);
};

#endif // !defined(USERS_FRAME_H)

/**
 * @file
 * $Id: UsersFrame.h,v 1.29 2006/02/19 16:19:06 arnetheduck Exp $
 */

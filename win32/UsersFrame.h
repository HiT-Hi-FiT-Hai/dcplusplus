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

#ifndef DCPLUSPLUS_WIN32_USERS_FRAME_H
#define DCPLUSPLUS_WIN32_USERS_FRAME_H

#include <dcpp/FavoriteManagerListener.h>

#include "StaticFrame.h"
#include "WinUtil.h"
#include "TypedListViewCtrl.h"

#include "UserInfoBase.h"

class UsersFrame : 
	public StaticFrame<UsersFrame>, 
	private FavoriteManagerListener,
	public AspectUserInfo<UsersFrame>
{
public:
	enum Status {
		STATUS_STATUS,
		STATUS_LAST
	};
	static const ResourceManager::Strings TITLE_RESOURCE = ResourceManager::FAVORITE_USERS;

protected:
	typedef StaticFrame<UsersFrame> BaseType;
	friend class StaticFrame<UsersFrame>;
	friend class MDIChildFrame<UsersFrame>;
	friend class AspectUserInfo<UsersFrame>;
	
	void layout();
	HRESULT spoken(LPARAM lp, WPARAM wp);
	bool preClosing();
	void postClosing();

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
		UserInfo(const FavoriteUser& u);
		
		const tstring& getText(int col) const {
			return columns[col];
		}

		int getImage() const {
			return 0;
		}

		static int compareItems(UserInfo* a, UserInfo* b, int col) {
			return lstrcmpi(a->columns[col].c_str(), b->columns[col].c_str());
		}

		void remove();

		void update(const FavoriteUser& u);

		tstring columns[COLUMN_LAST];
	};
	
	typedef TypedListViewCtrl<UsersFrame, UserInfo> WidgetUsers;
	typedef WidgetUsers* WidgetUsersPtr;
	WidgetUsersPtr users;

	bool startup;

	static int columnSizes[COLUMN_LAST];
	static int columnIndexes[COLUMN_LAST];

	UsersFrame(SmartWin::Widget* mdiParent);
	virtual ~UsersFrame() { }

	// FavoriteManagerListener
	virtual void on(UserAdded, const FavoriteUser& aUser) throw() { addUser(aUser); }
	virtual void on(UserRemoved, const FavoriteUser& aUser) throw() { removeUser(aUser); }
	virtual void on(StatusChanged, const UserPtr& aUser) throw() { speak(USER_UPDATED, reinterpret_cast<LPARAM>(new UserInfoBase(aUser))); }

	void addUser(const FavoriteUser& aUser);
	void updateUser(const UserPtr& aUser);
	void removeUser(const FavoriteUser& aUser);

	HRESULT handleContextMenu(LPARAM lParam, WPARAM wParam);
	void handleRemove(WidgetMenuPtr menu, unsigned id);
	void handleProperties(WidgetMenuPtr menu, unsigned id); 
	
	WidgetUsersPtr getUserList() { return users; }
	
#ifdef PORT_ME
	BEGIN_MSG_MAP(UsersFrame)
		NOTIFY_HANDLER(IDC_USERS, LVN_GETDISPINFO, ctrlUsers.onGetDispInfo)
		NOTIFY_HANDLER(IDC_USERS, LVN_COLUMNCLICK, ctrlUsers.onColumnClick)
		NOTIFY_HANDLER(IDC_USERS, LVN_ITEMCHANGED, onItemChanged)
		NOTIFY_HANDLER(IDC_USERS, LVN_KEYDOWN, onKeyDown)
		NOTIFY_HANDLER(IDC_USERS, NM_DBLCLK, onDoubleClick)
		COMMAND_ID_HANDLER(IDC_CONNECT, onConnect)
		CHAIN_MSG_MAP(uibBase)
		CHAIN_MSG_MAP(baseClass)
	END_MSG_MAP()

	LRESULT onItemChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);
	LRESULT onConnect(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onDoubleClick(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);
	LRESULT onKeyDown(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled);

#endif
};

#endif // !defined(USERS_FRAME_H)

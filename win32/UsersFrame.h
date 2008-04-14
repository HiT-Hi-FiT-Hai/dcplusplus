/*
 * Copyright (C) 2001-2008 Jacek Sieka, arnetheduck on gmail point com
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
#include "TypedTable.h"

#include "UserInfoBase.h"
#include "resource.h"

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

protected:
	typedef StaticFrame<UsersFrame> BaseType;
	friend class StaticFrame<UsersFrame>;
	friend class MDIChildFrame<UsersFrame>;
	friend class AspectUserInfo<UsersFrame>;

	UsersFrame(dwt::TabView* mdiParent);
	virtual ~UsersFrame();

	void layout();

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
	
	typedef TypedTable<UserInfo> WidgetUsers;
	typedef WidgetUsers* WidgetUsersPtr;
	WidgetUsersPtr users;

	static int columnSizes[COLUMN_LAST];
	static int columnIndexes[COLUMN_LAST];

	bool startup;

	void addUser(const FavoriteUser& aUser);
	void updateUser(const UserPtr& aUser);
	void removeUser(const FavoriteUser& aUser);

	void handleDescription();
	void handleRemove();
	bool handleKeyDown(int c);
	LRESULT handleItemChanged(LPARAM lParam);
	bool handleContextMenu(dwt::ScreenCoordinate pt);
	LRESULT handleSpeaker(WPARAM wParam, LPARAM lParam);

	WidgetUsersPtr getUserList() { return users; }

	// FavoriteManagerListener
	virtual void on(UserAdded, const FavoriteUser& aUser) throw();
	virtual void on(UserRemoved, const FavoriteUser& aUser) throw();
	virtual void on(StatusChanged, const UserPtr& aUser) throw();
};

#endif // !defined(USERS_FRAME_H)

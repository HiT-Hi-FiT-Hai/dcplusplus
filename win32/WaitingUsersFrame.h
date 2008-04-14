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

#ifndef DCPLUSPLUS_WIN32_WAITING_USERS_FRAME_H
#define DCPLUSPLUS_WIN32_WAITING_USERS_FRAME_H

#include "StaticFrame.h"
#include "WinUtil.h"

#include <dcpp/forward.h>
#include <dcpp/UploadManagerListener.h>
#include "resource.h"

class WaitingUsersFrame : 
	public StaticFrame<WaitingUsersFrame>, 
	public UploadManagerListener 
{
public:
	enum Status {
		STATUS_STATUS,
		STATUS_LAST
	};
protected:
	typedef StaticFrame<WaitingUsersFrame> BaseType;
	friend class StaticFrame<WaitingUsersFrame>;
	friend class MDIChildFrame<WaitingUsersFrame>;

	// Constructor
	WaitingUsersFrame(dwt::TabView* mdiParent);
	virtual ~WaitingUsersFrame() { }

	bool preClosing();
	void postClosing();
	// Update control layouts
	void layout();

	// Message handlers
	void onGetList();
	void onCopyFilename();
	void onRemove();
	bool handleContextMenu(dwt::ScreenCoordinate pt);
	HRESULT handleSpeaker(WPARAM wParam, LPARAM lParam);
	bool handleChar(int c);
	void onPrivateMessage();
	void onGrantSlot();
	void onAddToFavorites();

	void onRemoveUser(const UserPtr&);
	void onAddFile(const UserPtr&, const string&);

private:
	enum {
		SPEAK_ADD_FILE,
		SPEAK_REMOVE_USER
	};

	struct UserItem {
		UserPtr u;
		UserItem(UserPtr u) : u(u) { }
	};

	// Contained controls
	TreePtr queued;

	HTREEITEM getParentItem();

	UserPtr getSelectedUser() {
		HTREEITEM selectedItem = getParentItem();
		return selectedItem ? reinterpret_cast<UserItem *>(queued->getData(selectedItem))->u : UserPtr(0);
	}

	// Communication with manager
	void loadAll();
	void updateSearch(int index, BOOL doDelete = TRUE);

	// UploadManagerListener
	virtual void on(UploadManagerListener::WaitingRemoveUser, const UserPtr&) throw();
	virtual void on(UploadManagerListener::WaitingAddFile, const UserPtr&, const string&) throw();
};

#endif	/* WAITING_QUEUE_FRAME_H */

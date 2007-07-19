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

#ifndef DCPLUSPLUS_WIN32_WAITING_USERS_FRAME_H
#define DCPLUSPLUS_WIN32_WAITING_USERS_FRAME_H

#include "StaticFrame.h"
#include "WinUtil.h"

#include <dcpp/forward.h>
#include <dcpp/UploadManagerListener.h>

class WaitingUsersFrame : 
	public StaticFrame<WaitingUsersFrame>, 
	public UploadManagerListener 
{
public:
	enum Status {
		STATUS_STATUS,
		STATUS_LAST
	};
	static const ResourceManager::Strings TITLE_RESOURCE = ResourceManager::WAITING_USERS;
protected:
	typedef StaticFrame<WaitingUsersFrame> BaseType;
	friend class StaticFrame<WaitingUsersFrame>;
	friend class MDIChildFrame<WaitingUsersFrame>;

	// Constructor
	WaitingUsersFrame(SmartWin::Widget* mdiParent);
	virtual ~WaitingUsersFrame() { }

	bool preClosing();
	void postClosing();
	// Update control layouts
	void layout();

	// Message handlers
	void onGetList();
	void onCopyFilename();
	void onRemove();
	HRESULT handleContextMenu(WPARAM wParam, LPARAM lParam);
	HRESULT handleSpeaker(WPARAM wParam, LPARAM lParam);
	//LRESULT onChar(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled);
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
	WidgetTreeViewPtr queued;

	SmartWin::TreeViewNode GetParentItem();

	UserPtr getSelectedUser() {
		SmartWin::TreeViewNode selectedItem = GetParentItem();
		return selectedItem.handle?reinterpret_cast<UserItem *>(StupidWin::getTreeItemData(queued, selectedItem))->u:UserPtr(0);
	}

	// Communication with manager
	void loadAll();
	void updateSearch(int index, BOOL doDelete = TRUE);

	// UploadManagerListener
	virtual void on(UploadManagerListener::WaitingRemoveUser, const UserPtr) throw();
	virtual void on(UploadManagerListener::WaitingAddFile, const UserPtr, const string&) throw();
};

#ifdef PORT_ME
		MESSAGE_HANDLER(WM_CTLCOLOREDIT, onCtlColor)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, onCtlColor)
		MESSAGE_HANDLER(WM_CONTEXTMENU, onContextMenu)
		MESSAGE_HANDLER(WM_SPEAKER, onSpeaker)
		MESSAGE_HANDLER(WM_KEYDOWN, onChar)

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


#endif	/* PORT_ME */
#endif	/* WAITING_QUEUE_FRAME_H */

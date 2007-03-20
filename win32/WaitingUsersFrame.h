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


#if !defined(WAITING_QUEUE_FRAME_H)
#define WAITING_QUEUE_FRAME_H

#include "StaticFrame.h"
#include "WinUtil.h"
#include "client/UploadManager.h"

class WaitingUsersFrame : public StaticFrame<WaitingUsersFrame>, public UploadManagerListener {
public:
	static const ResourceManager::Strings TITLE_RESOURCE = ResourceManager::WAITING_USERS;

protected:
	friend class StaticFrame<WaitingUsersFrame>;
	friend class MDIChildFrame<WaitingUsersFrame>;

	// Constructor
	WaitingUsersFrame(SmartWin::Widget* mdiParent);
	virtual ~WaitingUsersFrame() { }

	// Update control layouts
	void layout();

	// Message handlers
	bool onClose();
	void onGetList(WidgetMenuPtr, unsigned int);
	void onCopyFilename(WidgetMenuPtr, unsigned int);
	void onRemove(WidgetMenuPtr, unsigned int);
	LRESULT onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled);
	LRESULT onSpeaker(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	//LRESULT onChar(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled);
	void onPrivateMessage(WidgetMenuPtr, unsigned int);
	void onGrantSlot(WidgetMenuPtr, unsigned int);
	void onAddToFavorites(WidgetMenuPtr, unsigned int);

	void onRemoveUser(const User::Ptr);
	void onAddFile(const User::Ptr, const string&);

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

	// Contained controls
	WidgetTreeViewPtr queued;
	WidgetMenuPtr contextMenu;

	SmartWin::TreeViewNode GetParentItem();

	User::Ptr getSelectedUser() {
		SmartWin::TreeViewNode selectedItem = GetParentItem();
		return selectedItem.handle?reinterpret_cast<UserPtr *>(StupidWin::getTreeItemData(queued, selectedItem))->u:User::Ptr(0);
	}

	// Communication with manager
	void LoadAll();
	void UpdateSearch(int index, BOOL doDelete = TRUE);

	// UploadManagerListener
	virtual void on(UploadManagerListener::WaitingRemoveUser, const User::Ptr) throw();
	virtual void on(UploadManagerListener::WaitingAddFile, const User::Ptr, const string&) throw();
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

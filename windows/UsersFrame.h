/* 
 * Copyright (C) 2001-2003 Jacek Sieka, j_s@telia.com
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

#if !defined(AFX_USERSFRAME_H__F6D75CA8_F229_4E7D_8ADC_0B1F3B0083C4__INCLUDED_)
#define AFX_USERSFRAME_H__F6D75CA8_F229_4E7D_8ADC_0B1F3B0083C4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FlatTabCtrl.h"
#include "ExListViewCtrl.h"

#include "../client/ClientManager.h"
#include "../client/HubManager.h"

class UsersFrame : public MDITabChildWindowImpl<UsersFrame>, public StaticFrame<UsersFrame, ResourceManager::FAVORITE_USERS>,
	private HubManagerListener, private ClientManagerListener {
public:
	
	UsersFrame() : closed(false) { };
	virtual ~UsersFrame() { };

	DECLARE_FRAME_WND_CLASS_EX("UsersFrame", IDR_USERS, 0, COLOR_3DFACE);
		
	virtual void OnFinalMessage(HWND /*hWnd*/) {
		frame = NULL;
		delete this;
	}

	typedef MDITabChildWindowImpl<UsersFrame> baseClass;
	BEGIN_MSG_MAP(UsersFrame)
		MESSAGE_HANDLER(WM_CREATE, onCreate)
		MESSAGE_HANDLER(WM_CLOSE, onClose)
		MESSAGE_HANDLER(WM_CONTEXTMENU, onContextMenu)
		MESSAGE_HANDLER(WM_SPEAKER, onSpeaker)
		COMMAND_ID_HANDLER(IDC_REMOVE, onRemove)
		COMMAND_ID_HANDLER(IDC_PRIVATEMESSAGE, onPrivateMessage)
		COMMAND_ID_HANDLER(IDC_GETLIST, onGetList)
		COMMAND_ID_HANDLER(IDC_GRANTSLOT, onGrantSlot)
		NOTIFY_HANDLER(IDC_USERS, LVN_COLUMNCLICK, onColumnClickHublist)
		CHAIN_MSG_MAP(baseClass)
	END_MSG_MAP()
		
	LRESULT onCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT onPrivateMessage(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT onGetList(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT onGrantSlot(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	
	LRESULT onSpeaker(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
		if(wParam == USER_UPDATED) {
			updateUser(((UserInfo*)lParam)->user);
			delete (UserInfo*)lParam;
		}
		return 0;
	}
	
	LRESULT onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
		RECT rc;                    // client area of window 
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };        // location of mouse click 
		
		// Get the bounding rectangle of the client area. 
		ctrlUsers.GetClientRect(&rc);
		ctrlUsers.ScreenToClient(&pt); 
		
		if (ctrlUsers.GetSelectedCount() > 0 && PtInRect(&rc, pt)) 
		{ 
			ctrlUsers.ClientToScreen(&pt);
			usersMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
			
			return TRUE; 
		}
		
		return FALSE; 
	}
	
	LRESULT onColumnClickHublist(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
		NMLISTVIEW* l = (NMLISTVIEW*)pnmh;
		if(l->iSubItem == ctrlUsers.getSortColumn()) {
			if (!ctrlUsers.getSortDirection())
				ctrlUsers.setSort(-1, ctrlUsers.getSortType());
			else
				ctrlUsers.setSortDirection(false);
		} else {
			ctrlUsers.setSort(l->iSubItem, ExListViewCtrl::SORT_STRING_NOCASE);
		}
		return 0;
	}
	
	void UpdateLayout(BOOL bResizeBars = TRUE) {
		RECT rect;
		GetClientRect(&rect);
		// position bars and offset their dimensions
		UpdateBarsPosition(rect, bResizeBars);
		
		if(ctrlStatus.IsWindow()) {
			CRect sr;
			int w[3];
			ctrlStatus.GetClientRect(sr);
			int tmp = (sr.Width()) > 316 ? 216 : ((sr.Width() > 116) ? sr.Width()-100 : 16);
			
			w[0] = sr.right - tmp;
			w[1] = w[0] + (tmp-16)/2;
			w[2] = w[0] + (tmp-16);
			
			ctrlStatus.SetParts(3, w);
		}
		
		CRect rc = rect;
		ctrlUsers.MoveWindow(rc);
	}
	
private:
	enum {
		COLUMN_FIRST,
		COLUMN_NICK = COLUMN_FIRST,
		COLUMN_STATUS,
		COLUMN_HUB,
		COLUMN_GRANT_SLOT,
		COLUMN_LAST
	};

	enum {
		USER_UPDATED
	};
	class UserInfo {
	public:
		UserInfo(const User::Ptr& aUser) : user(aUser) { };
		User::Ptr user;
	};

	CStatusBarCtrl ctrlStatus;
	CMenu usersMenu;
	
	ExListViewCtrl ctrlUsers;

	bool closed;
	
	static int columnSizes[COLUMN_LAST];
	static int columnIndexes[COLUMN_LAST];

	// HubManagerListener
	virtual void onAction(HubManagerListener::Types type, const User::Ptr& aUser) throw() {
		switch(type) {
		case HubManagerListener::USER_ADDED: addUser(aUser); break;
		case HubManagerListener::USER_REMOVED: removeUser(aUser); break;
		}
	}

	// ClientManagerListener
	virtual void onAction(ClientManagerListener::Types type, const User::Ptr& aUser) throw() {
		switch(type) {
		case ClientManagerListener::USER_UPDATED:
			if(aUser->isFavoriteUser()) {
				PostMessage(WM_SPEAKER, USER_UPDATED, (LPARAM) new UserInfo(aUser));
			}
		}
	}

	void addUser(const User::Ptr& aUser);
	void updateUser(const User::Ptr& aUser);
	void removeUser(const User::Ptr& aUser);
};

#endif // !defined(AFX_USERSFRAME_H__F6D75CA8_F229_4E7D_8ADC_0B1F3B0083C4__INCLUDED_)

/**
 * @file
 * $Id: UsersFrame.h,v 1.9 2003/11/07 00:42:41 arnetheduck Exp $
 */


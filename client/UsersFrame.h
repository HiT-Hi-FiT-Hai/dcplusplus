/* 
 * Copyright (C) 2001 Jacek Sieka, j_s@telia.com
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
#include "HubManager.h"
#include "ExListViewCtrl.h"
#include "ClientManager.h"

class UsersFrame : public MDITabChildWindowImpl<UsersFrame>, private HubManagerListener, private ClientManagerListener {
public:
	
	UsersFrame() { };
	virtual ~UsersFrame() { };

	DECLARE_FRAME_WND_CLASS_EX("UsersFrame", IDR_USERS, 0, COLOR_3DFACE);
		
	virtual void OnFinalMessage(HWND /*hWnd*/) {
		frame = NULL;
		delete this;
	}

	BEGIN_MSG_MAP(UsersFrame)
		MESSAGE_HANDLER(WM_CREATE, onCreate)
		MESSAGE_HANDLER(WM_FORWARDMSG, OnForwardMsg)
		MESSAGE_HANDLER(WM_CLOSE, onClose)
		MESSAGE_HANDLER(WM_CONTEXTMENU, onContextMenu)
		MESSAGE_HANDLER(WM_SPEAKER, onSpeaker)
		COMMAND_ID_HANDLER(IDC_REMOVE, onRemove)
		COMMAND_ID_HANDLER(IDC_PRIVATEMESSAGE, onPrivateMessage)
		COMMAND_ID_HANDLER(IDC_GETLIST, onGetList)
		NOTIFY_HANDLER(IDC_USERS, LVN_COLUMNCLICK, onColumnClickHublist)
		CHAIN_MSG_MAP(MDITabChildWindowImpl<UsersFrame>)
	END_MSG_MAP()
		
	LRESULT onCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT onPrivateMessage(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT onGetList(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	
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
		
		if (PtInRect(&rc, pt)) 
		{ 
			ctrlUsers.ClientToScreen(&pt);
			usersMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
			
			return TRUE; 
		}
		
		return FALSE; 
	}
	
	LRESULT OnForwardMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
		LPMSG pMsg = (LPMSG)lParam;
		return CMDIChildWindowImpl2<UsersFrame>::PreTranslateMessage(pMsg);
	}
	
	LRESULT onColumnClickHublist(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
		NMLISTVIEW* l = (NMLISTVIEW*)pnmh;
		if(l->iSubItem == ctrlUsers.getSortColumn()) {
			ctrlUsers.setSortDirection(!ctrlUsers.getSortDirection());
		} else {
			ctrlUsers.setSort(l->iSubItem, ExListViewCtrl::SORT_STRING_NOCASE);
		}
		return 0;
	}
	
	void UpdateLayout(BOOL bResizeBars = TRUE)
	{
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
	
	static UsersFrame* frame;
	
private:
	enum {
		COLUMN_FIRST,
		COLUMN_NICK = COLUMN_FIRST,
		COLUMN_STATUS,
		COLUMN_HUB,
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
	
	static int columnSizes[COLUMN_LAST];
	static int columnIndexes[COLUMN_LAST];

	// HubManagerListener
	virtual void onAction(HubManagerListener::Types type, const User::Ptr& aUser) {
		switch(type) {
		case HubManagerListener::USER_ADDED: addUser(aUser); break;
		case HubManagerListener::USER_REMOVED: removeUser(aUser); break;
		}
	}

	// ClientManagerListener
	virtual void onAction(ClientManagerListener::Types type, const User::Ptr& aUser) {
		switch(type) {
		case ClientManagerListener::USER_UPDATED:
			if(HubManager::getInstance()->isFavoriteUser(aUser)) {
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
 * @file FavoriteHubsFrm.h
 * $Id: UsersFrame.h,v 1.1 2002/03/26 09:17:59 arnetheduck Exp $
 * @if LOG
 * $Log: UsersFrame.h,v $
 * Revision 1.1  2002/03/26 09:17:59  arnetheduck
 * New UsersFrame
 *
 * Revision 1.6  2002/03/23 01:58:42  arnetheduck
 * Work done on favorites...
 *
 * Revision 1.5  2002/03/13 20:35:25  arnetheduck
 * Release canditate...internationalization done as far as 0.155 is concerned...
 * Also started using mirrors of the public hub lists
 *
 * Revision 1.4  2002/02/10 12:25:24  arnetheduck
 * New properties for favorites, and some minor performance tuning...
 *
 * Revision 1.3  2002/02/09 18:13:51  arnetheduck
 * Fixed level 4 warnings and started using new stl
 *
 * Revision 1.2  2002/01/20 22:54:46  arnetheduck
 * Bugfixes to 0.131 mainly...
 *
 * Revision 1.1  2002/01/13 22:53:26  arnetheduck
 * Favorites...
 *
 * @endif
 */


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
#include "TypedListViewCtrl.h"
#include "WinUtil.h"

#include "../client/ClientManager.h"
#include "../client/HubManager.h"

class UsersFrame : public MDITabChildWindowImpl<UsersFrame>, public StaticFrame<UsersFrame, ResourceManager::FAVORITE_USERS>,
	private HubManagerListener, private ClientManagerListener, public UserInfoBaseHandler<UsersFrame> {
public:
	
	UsersFrame() : closed(false), startup(true) { };
	virtual ~UsersFrame() { };

	DECLARE_FRAME_WND_CLASS_EX("UsersFrame", IDR_USERS, 0, COLOR_3DFACE);
		
	virtual void OnFinalMessage(HWND /*hWnd*/) {
		frame = NULL;
		delete this;
	}

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
		CHAIN_MSG_MAP(uibBase)
		CHAIN_MSG_MAP(baseClass)
	END_MSG_MAP()
		
	LRESULT onCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT onEdit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT onItemChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);
	
	LRESULT onSpeaker(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
		if(wParam == USER_UPDATED) {
			updateUser(((UserInfoBase*)lParam)->user);
			delete (UserInfoBase*)lParam;
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
		COLUMN_STATUS,
		COLUMN_HUB,
		COLUMN_SEEN,
		COLUMN_DESCRIPTION,
		COLUMN_LAST
	};

	enum {
		USER_UPDATED
	};

	class UserInfo : public UserInfoBase {
	public:
		UserInfo(const User::Ptr& u) : UserInfoBase(u) { 
			columns[COLUMN_NICK] = u->getNick();
			update();
		};

		const string& getText(int col) const {
			return columns[col];
		}

		static int compareItems(UserInfo* a, UserInfo* b, int col) {
			return Util::stricmp(a->columns[col], b->columns[col]);
		}

		void remove() { HubManager::getInstance()->removeFavoriteUser(user); }

		void update() {
			columns[COLUMN_STATUS] = user->isOnline() ? STRING(ONLINE) : STRING(OFFLINE);
			columns[COLUMN_HUB] = user->getClientName();
			if(!user->getLastHubAddress().empty()) {
				columns[COLUMN_HUB] += " (" + user->getLastHubAddress() + ")";
			}
			columns[COLUMN_SEEN] = user->isOnline() ? Util::emptyString : Util::formatTime("%Y-%m-%d %H:%M", user->getFavoriteLastSeen());
			columns[COLUMN_DESCRIPTION] = user->getUserDescription();
		}

		string columns[COLUMN_LAST];
	};

	CStatusBarCtrl ctrlStatus;
	CMenu usersMenu;
	
	TypedListViewCtrl<UserInfo, IDC_USERS> ctrlUsers;

	bool closed;
	
	bool startup;
	static int columnSizes[COLUMN_LAST];
	static int columnIndexes[COLUMN_LAST];

	// HubManagerListener
	virtual void on(UserAdded, const User::Ptr& aUser) throw() { addUser(aUser); }
	virtual void on(UserRemoved, const User::Ptr& aUser) throw() { removeUser(aUser); }

	// ClientManagerListener
	virtual void on(ClientManagerListener::UserUpdated, const User::Ptr& aUser) throw() {
		if(aUser->isFavoriteUser()) {
			PostMessage(WM_SPEAKER, USER_UPDATED, (LPARAM) new UserInfoBase(aUser));
		}
	}

	void addUser(const User::Ptr& aUser);
	void updateUser(const User::Ptr& aUser);
	void removeUser(const User::Ptr& aUser);
};

#endif // !defined(AFX_USERSFRAME_H__F6D75CA8_F229_4E7D_8ADC_0B1F3B0083C4__INCLUDED_)

/**
 * @file
 * $Id: UsersFrame.h,v 1.15 2004/04/18 12:51:15 arnetheduck Exp $
 */


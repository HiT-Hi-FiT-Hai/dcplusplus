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

#if !defined(AFX_CHILDFRM_H__A7078724_FD85_4F39_8463_5A08A5F45E33__INCLUDED_)
#define AFX_CHILDFRM_H__A7078724_FD85_4F39_8463_5A08A5F45E33__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "FlatTabCtrl.h"
#include "ExListViewCtrl.h"
#include "PrivateFrame.h"

#include "../client/Client.h"
#include "../client/User.h"
#include "../client/ClientManager.h"
#include "../client/TimerManager.h"

#include "WinUtil.h"

#define EDIT_MESSAGE_MAP 10		// This could be any number, really...

class HubFrame : public MDITabChildWindowImpl<HubFrame>, private ClientListener, public CSplitterImpl<HubFrame>, private TimerManagerListener
{
public:
	HubFrame(const string& aServer, const string& aNick = Util::emptyString, const string& aPassword = Util::emptyString) : 
	  waitingForPW(false), op(false), ctrlMessageContainer("edit", this, EDIT_MESSAGE_MAP), server(aServer) {
		client = ClientManager::getInstance()->getClient();
		client->setNick(aNick);
		client->setPassword(aPassword);
		client->addListener(this);
		TimerManager::getInstance()->addListener(this);
		timeStamps = BOOLSETTING(TIME_STAMPS);
	}

	~HubFrame() {
		dcassert(client == NULL);
	}

	DECLARE_FRAME_WND_CLASS("HubFrame", IDR_HUB);

	virtual void OnFinalMessage(HWND /*hWnd*/) {
		delete this;
	}

	typedef CSplitterImpl<HubFrame> splitBase;

	BEGIN_MSG_MAP(HubFrame)
		MESSAGE_HANDLER(WM_CLOSE, onClose)
		MESSAGE_HANDLER(WM_ACTIVATE, onActivate)
		MESSAGE_HANDLER(WM_MDIACTIVATE, onActivate)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_FORWARDMSG, OnForwardMsg)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_SPEAKER, onSpeaker)
		MESSAGE_HANDLER(WM_CONTEXTMENU, onContextMenu)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, onCtlColor)
		MESSAGE_HANDLER(WM_CTLCOLOREDIT, onCtlColor)
		COMMAND_ID_HANDLER(ID_FILE_RECONNECT, OnFileReconnect)
		COMMAND_ID_HANDLER(IDC_GETLIST, onGetList)
		COMMAND_ID_HANDLER(IDC_PRIVATEMESSAGE, onPrivateMessage)
		COMMAND_ID_HANDLER(IDC_REFRESH, onRefresh)
		COMMAND_ID_HANDLER(IDC_KICK, onKick)
		COMMAND_ID_HANDLER(IDC_GRANTSLOT, onGrantSlot)
		COMMAND_ID_HANDLER(IDC_REDIRECT, onRedirect)
		COMMAND_ID_HANDLER(IDC_FOLLOW, onFollow)
		COMMAND_ID_HANDLER(IDC_ADD_TO_FAVORITES, onAddToFavorites)
		NOTIFY_HANDLER(IDC_USERS, NM_DBLCLK, onDoubleClickUsers)	
		NOTIFY_HANDLER(IDC_USERS, LVN_COLUMNCLICK, onColumnClickUsers)
		CHAIN_MSG_MAP(MDITabChildWindowImpl<HubFrame>)
		CHAIN_MSG_MAP(splitBase)
	ALT_MSG_MAP(EDIT_MESSAGE_MAP)
		MESSAGE_HANDLER(WM_CHAR, onChar)
		MESSAGE_HANDLER(WM_KEYDOWN, onChar)
		MESSAGE_HANDLER(WM_KEYUP, onChar)
	END_MSG_MAP()

	LRESULT onSpeaker(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT onGetList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onAddToFavorites(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onPrivateMessage(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onGrantSlot(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onKick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onRedirect(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onDoubleClickUsers(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
		
	void UpdateLayout(BOOL bResizeBars = TRUE);
	void addLine(const string& aLine);
	
	LRESULT onActivate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
		ctrlMessage.SetFocus();
		bHandled = FALSE;
		return 0;
	}
		
	LRESULT onCtlColor(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
		HWND hWnd = (HWND)lParam;
		HDC hDC = (HDC)wParam;
		if(hWnd == ctrlClient.m_hWnd || hWnd == ctrlMessage.m_hWnd) {
			::SetBkColor(hDC, WinUtil::bgColor);
			::SetTextColor(hDC, WinUtil::textColor);
			return (LRESULT)WinUtil::bgBrush;
			
		} else {
			return 0;
		}
		
	}

	LRESULT onFollow(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		if(!redirect.empty()) {
			server = redirect;
			if(client)
				client->connect(redirect);
		}
		return 0;
	}

	LRESULT onRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		
		if(client && client->isConnected()) {
			clearUserList();
			client->getNickList();
		}
		return 0;
	}
	
	LRESULT OnFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
		bHandled = FALSE;
		ctrlMessage.SetFocus();
		return 0;
	}
	
	LRESULT OnFileReconnect(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		if(client) {
			clearUserList();
			client->connect(server);
		}
		return 0;
	}

	LRESULT OnForwardMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
		LPMSG pMsg = (LPMSG)lParam;
		return MDITabChildWindowImpl<HubFrame>::PreTranslateMessage(pMsg);
	}
	
	LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		return 0;
	}
		
	static int sortSize(LPARAM a, LPARAM b) {
		UserInfo* c = (UserInfo*)a;
		UserInfo* d = (UserInfo*)b;

		if(c->size < d->size) {
			return -1;
		} else if(c->size == d->size) {
			return 0;
		} else {
			return 1;
		}
	}

	LRESULT onColumnClickUsers(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
		NMLISTVIEW* l = (NMLISTVIEW*)pnmh;
		if(l->iSubItem == ctrlUsers.getSortColumn()) {
			ctrlUsers.setSortDirection(!ctrlUsers.getSortDirection());
		} else {
			if(l->iSubItem == COLUMN_SHARED) {
				ctrlUsers.setSort(l->iSubItem, ExListViewCtrl::SORT_FUNC, true, sortSize);
			} else {
				ctrlUsers.setSort(l->iSubItem, ExListViewCtrl::SORT_STRING_NOCASE);
			}
		}
		return 0;
	}

	void addClientLine(const string& aLine, bool inChat = true) {
		ctrlStatus.SetText(0, ("[" + Util::getShortTimeString() + "] " + aLine).c_str());
		setDirty();

		if(BOOLSETTING(STATUS_IN_CHAT) && inChat) {
			addLine("*** " + aLine);
		}
	}

	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(&ps);
		FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_ACTIVEBORDER+1));
		EndPaint(&ps);
		return 0;
	}

	LRESULT onChar(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled) {
		switch(wParam) {
		case VK_RETURN:
			if( (GetKeyState(VK_SHIFT) & 0x8000) || 
				(GetKeyState(VK_CONTROL) & 0x8000) || 
				(GetKeyState(VK_MENU) & 0x8000) ) {
				bHandled = FALSE;
			} else {
				if(uMsg == WM_KEYDOWN) {
					onEnter();
				}
			}
			break;
		default:
			bHandled = FALSE;
		}
		return 0;
	}

	void onEnter();
private:
	enum {
		CLIENT_CONNECTING,
		CLIENT_CONNECTED,
		CLIENT_FAILED,
		CLIENT_GETPASSWORD,
		CLIENT_HUBNAME,
		CLIENT_MESSAGE,
		CLIENT_MYINFO,
		CLIENT_HELLO,
		CLIENT_PRIVATEMESSAGE,
		CLIENT_QUIT,
		CLIENT_UNKNOWN,
		CLIENT_VALIDATEDENIED,
		CLIENT_SEARCH_FLOOD,
		CLIENT_STATUS,
		STATS,
		REDIRECT
	};

	enum {
		IMAGE_USER = 0,
		IMAGE_OP
	};
	
	enum {
		COLUMN_FIRST,
		COLUMN_NICK = COLUMN_FIRST,
		COLUMN_SHARED,
		COLUMN_DESCRIPTION,
		COLUMN_CONNECTION,
		COLUMN_EMAIL,
		COLUMN_LAST
	};
	
	class UserInfo {
	public:
		LONGLONG size;
	};

	class PMInfo {
	public:
		User::Ptr user;
		string msg;
	};

	string redirect;
	bool timeStamps;
	bool showJoins;
	
	string lastKick;
	string lastRedir;
	string lastServer;

	bool waitingForPW;
	
	Client::Ptr client;
	string server;
	CContainedWindow ctrlMessageContainer;
	CMenu userMenu;
	CMenu opMenu;
	bool op;
	
	CEdit ctrlClient;
	CEdit ctrlMessage;
	ExListViewCtrl ctrlUsers;
	CStatusBarCtrl ctrlStatus;
	
	static CImageList* images;
	static int columnIndexes[COLUMN_LAST];
	static int columnSizes[COLUMN_LAST];
	
	void clearUserList() {
		int j = ctrlUsers.GetItemCount();
		for(int i = 0; i < j; i++) {
			delete (UserInfo*) ctrlUsers.GetItemData(i);
		}
		ctrlUsers.DeleteAllItems();
	}

	int getImage(const User::Ptr& u) {
		int image = u->isSet(User::OP) ? IMAGE_OP : IMAGE_USER;
		
		if(u->isSet(User::DCPLUSPLUS))
			image+=2;
		if(u->isSet(User::PASSIVE)) {
			image+=4;
		}
		return image;	
	}
	
	// TimerManagerListener
	virtual void onAction(TimerManagerListener::Types type, DWORD /*aTick*/) {
		switch(type) {
		case TimerManagerListener::SECOND:
			updateStatusBar(); break;
		}
	}

	// ClientListener
	virtual void onAction(ClientListener::Types type, Client* client) {
		switch(type) {
		case ClientListener::CONNECTING:
			PostMessage(WM_SPEAKER, CLIENT_CONNECTING); break;
		case ClientListener::CONNECTED:
			PostMessage(WM_SPEAKER, CLIENT_CONNECTED); break;
		case ClientListener::BAD_PASSWORD:
			client->setPassword(Util::emptyString); break;
		case ClientListener::GET_PASSWORD:
			PostMessage(WM_SPEAKER, CLIENT_GETPASSWORD); break;
		case ClientListener::HUB_NAME:
			PostMessage(WM_SPEAKER, CLIENT_HUBNAME); break;
		case ClientListener::VALIDATE_DENIED:
			PostMessage(WM_SPEAKER, CLIENT_VALIDATEDENIED); break;
		}
	}
	
	virtual void onAction(ClientListener::Types type, Client* /*client*/, const string& line) {
		string* x;
		switch(type) {
		case ClientListener::SEARCH_FLOOD:
			x = new string(line);
			PostMessage(WM_SPEAKER, CLIENT_SEARCH_FLOOD, (LPARAM)x); break;
		case ClientListener::FAILED:
			x = new string(line);
			PostMessage(WM_SPEAKER, CLIENT_FAILED, (LPARAM)x); break;
		case ClientListener::MESSAGE: 
			x = new string(line);
			if(SETTING(FILTER_KICKMSGS)) {
				if((line.find("Hub-Security") != string::npos) && (line.find("was kicked by") != string::npos)) {
					delete x;
					// Do nothing...
				} else if((line.find("is kicking") != string::npos) && (line.find("because:") != string::npos)) {
					PostMessage(WM_SPEAKER, CLIENT_STATUS, (LPARAM)x); 
				} else {
					PostMessage(WM_SPEAKER, CLIENT_MESSAGE, (LPARAM) x);
				}
			} else {
				PostMessage(WM_SPEAKER, CLIENT_MESSAGE, (LPARAM) x);
			}
			break;

		case ClientListener::FORCE_MOVE:
			redirect = line;
			if(BOOLSETTING(AUTO_FOLLOW)) {
				PostMessage(WM_COMMAND, IDC_FOLLOW);
			} else {
				PostMessage(WM_SPEAKER, REDIRECT);
			}
			
			break;
		}
	}

	virtual void onAction(ClientListener::Types type, Client* /*client*/, const User::Ptr& user) {
		User::Ptr* x;
		switch(type) {
		case ClientListener::MY_INFO:
			x = new User::Ptr(user);
			PostMessage(WM_SPEAKER, CLIENT_MYINFO, (LPARAM)x); break;
		case ClientListener::QUIT:
			x = new User::Ptr(user);
			PostMessage(WM_SPEAKER, CLIENT_QUIT, (LPARAM)x); break;
		case ClientListener::HELLO:
			x = new User::Ptr(user);
			PostMessage(WM_SPEAKER, CLIENT_HELLO, (LPARAM)x); break;
		}
	}
	
	virtual void onAction(ClientListener::Types type, Client* /*client*/, const User::List& aList) {
		switch(type) {
		case ClientListener::OP_LIST:
			for(User::List::const_iterator i = aList.begin(); i != aList.end(); ++i) {
				if((*i)->getNick() == client->getNick()) {
					op = true;
					return;
				}
			}
			break;
		}
	}

	virtual void onAction(ClientListener::Types type, Client* /*client*/, const User::Ptr& user, const string&  line) {
		switch(type) {
		case ClientListener::PRIVATE_MESSAGE:
			PMInfo* i = new PMInfo();
			
			i->user = user;
			i->msg = line;
			PostMessage(WM_SPEAKER, CLIENT_PRIVATEMESSAGE, (LPARAM)i);
			break;
		}
	}

	void updateStatusBar() {
		if(m_hWnd)
			PostMessage(WM_SPEAKER, STATS);
	}

};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHILDFRM_H__A7078724_FD85_4F39_8463_5A08A5F45E33__INCLUDED_)

/**
 * @file HubFrame.h
 * $Id: HubFrame.h,v 1.2 2002/04/13 12:57:23 arnetheduck Exp $
 */


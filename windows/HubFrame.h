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
	  waitingForPW(false), ctrlMessageContainer("edit", this, EDIT_MESSAGE_MAP), 
	  clientContainer("edit", this, EDIT_MESSAGE_MAP), server(aServer), needSort(false) {
		client = ClientManager::getInstance()->getClient();
		client->setNick(aNick);
		client->setPassword(aPassword);
		client->addListener(this);
		TimerManager::getInstance()->addListener(this);
		timeStamps = BOOLSETTING(TIME_STAMPS);
	}

	~HubFrame() {
		ClientManager::getInstance()->putClient(client);
	}

	DECLARE_FRAME_WND_CLASS_EX("HubFrame", IDR_HUB, 0, COLOR_3DFACE);

	virtual void OnFinalMessage(HWND /*hWnd*/) {
		delete this;
	}

	typedef CSplitterImpl<HubFrame> splitBase;
	typedef MDITabChildWindowImpl<HubFrame> baseClass;
	
	BEGIN_MSG_MAP(HubFrame)
		MESSAGE_HANDLER(WM_CLOSE, onClose)
		MESSAGE_HANDLER(WM_ACTIVATE, onActivate)
		MESSAGE_HANDLER(WM_MDIACTIVATE, onActivate)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_FORWARDMSG, OnForwardMsg)
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
		CHAIN_MSG_MAP(baseClass)
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
	LRESULT onChar(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled);
		
	void UpdateLayout(BOOL bResizeBars = TRUE);
	void addLine(const string& aLine);
	void onEnter();
	void onTab();
	
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
			client->addListener(this);
			client->connect(redirect);
		}
		return 0;
	}

	LRESULT onRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		if(client->isConnected()) {
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
		clearUserList();
		client->addListener(this);
		client->connect(server);
		return 0;
	}

	LRESULT OnForwardMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
		LPMSG pMsg = (LPMSG)lParam;
		return MDITabChildWindowImpl<HubFrame>::PreTranslateMessage(pMsg);
	}
	
	static int sortSize(LPARAM a, LPARAM b) {
		UserInfo* c = (UserInfo*)a;
		UserInfo* d = (UserInfo*)b;
		u_int64_t e = c->user->getBytesShared();
		u_int64_t f = d->user->getBytesShared();
		
		return (e < f) ? -1 : ((e == f) ? 0 : 1);
	}

	static int sortNick(LPARAM a, LPARAM b) {
		UserInfo* c = (UserInfo*)a;
		UserInfo* d = (UserInfo*)b;
		if(c->user->isSet(User::OP) && !d->user->isSet(User::OP)) {
			return -1;
		} else if(!c->user->isSet(User::OP) && d->user->isSet(User::OP)) {
			return 1;
		}
		return stricmp(c->user->getNick().c_str(), d->user->getNick().c_str());		
	}
	
	LRESULT onColumnClickUsers(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
		NMLISTVIEW* l = (NMLISTVIEW*)pnmh;
		if(l->iSubItem == ctrlUsers.getSortColumn()) {
			ctrlUsers.setSortDirection(!ctrlUsers.getSortDirection());
		} else {
			if(l->iSubItem == COLUMN_SHARED) {
				ctrlUsers.setSort(l->iSubItem, ExListViewCtrl::SORT_FUNC, true, sortSize);
			} else if(l->iSubItem == COLUMN_NICK) {
				ctrlUsers.setSort(l->iSubItem, ExListViewCtrl::SORT_FUNC, true, sortNick);
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

private:
	enum Speakers {
		UPDATE_USER,
		UPDATE_USERS,
		REMOVE_USER,
		REMOVE_USERS,
		ADD_CHAT_LINE,
		ADD_STATUS_LINE,
		SET_WINDOW_TITLE,
		GET_PASSWORD,
		PRIVATE_MESSAGE,
		STATS
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
		UserInfo(const User::Ptr& u) : user(u) { };
		User::Ptr user;
	};

	class PMInfo {
	public:
		PMInfo(const User::Ptr& u, const string& m) : user(u), msg(m) { };
		User::Ptr user;
		string msg;
	};

	string redirect;
	bool timeStamps;
	bool showJoins;
	
	string lastKick;
	string lastRedir;
	string lastServer;
	
	bool needSort;
	bool waitingForPW;
	
	Client* client;
	string server;
	CContainedWindow ctrlMessageContainer;
	CContainedWindow clientContainer;

	CMenu userMenu;
	CMenu opMenu;
	
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

	void updateStatusBar() {
		if(m_hWnd)
			PostMessage(WM_SPEAKER, STATS);
	}
	
	// TimerManagerListener
	virtual void onAction(TimerManagerListener::Types type, DWORD /*aTick*/) {
		switch(type) {
		case TimerManagerListener::SECOND:
			updateStatusBar(); break;
		}
	}

	void speak(Speakers s) { PostMessage(WM_SPEAKER, (WPARAM)s); };
	void speak(Speakers s, const string& msg) { PostMessage(WM_SPEAKER, (WPARAM)s, (LPARAM)new string(msg)); };
	void speak(Speakers s, const User::Ptr& u) { PostMessage(WM_SPEAKER, (WPARAM)s, (LPARAM)new UserInfo(u)); };
	void speak(Speakers s, const User::List& l) { PostMessage(WM_SPEAKER, (WPARAM)s, (LPARAM)new User::List(l)); };
	void speak(Speakers s, const User::Ptr& u, const string& line) { PostMessage(WM_SPEAKER, (WPARAM)s, (LPARAM)new PMInfo(u, line)); };
	
	// ClientListener
	virtual void onAction(ClientListener::Types type, Client* client) {
		switch(type) {
		case ClientListener::CONNECTING:
			speak(ADD_STATUS_LINE, STRING(CONNECTING_TO) + client->getServer() + "...");
			speak(SET_WINDOW_TITLE, client->getServer());
			break;
		case ClientListener::CONNECTED: speak(ADD_STATUS_LINE, STRING(CONNECTED)); break;
		case ClientListener::BAD_PASSWORD: client->setPassword(Util::emptyString); break;
		case ClientListener::GET_PASSWORD: speak(GET_PASSWORD); break;
		case ClientListener::HUB_NAME: speak(SET_WINDOW_TITLE, client->getName()); break;
		case ClientListener::VALIDATE_DENIED:
			client->removeListener(this);
			client->disconnect();
			speak(ADD_STATUS_LINE, STRING(NICK_TAKEN));
			break;
		}
	}
	
	virtual void onAction(ClientListener::Types type, Client* /*client*/, const string& line) {
		switch(type) {
		case ClientListener::SEARCH_FLOOD: speak(ADD_STATUS_LINE, STRING(SEARCH_SPAM_FROM) + line); break;
		case ClientListener::FAILED: speak(ADD_STATUS_LINE, line); speak(REMOVE_USERS); break;
		case ClientListener::MESSAGE: 
			if(SETTING(FILTER_KICKMSGS)) {
				if((line.find("Hub-Security") != string::npos) && (line.find("was kicked by") != string::npos)) {
					// Do nothing...
				} else if((line.find("is kicking") != string::npos) && (line.find("because:") != string::npos)) {
					speak(ADD_STATUS_LINE, line);
				} else {
					speak(ADD_CHAT_LINE, line);
				}
			} else {
				speak(ADD_CHAT_LINE, line);
			}
			break;

		case ClientListener::FORCE_MOVE:
			redirect = line;
			if(BOOLSETTING(AUTO_FOLLOW)) {
				server = line;
				client->connect(line);
			} else {
				client->disconnect();
				speak(ADD_STATUS_LINE, STRING(PRESS_FOLLOW) + line);
			}
			break;
		}
	}

	virtual void onAction(ClientListener::Types type, Client* /*client*/, const User::Ptr& user) {
		switch(type) {
		case ClientListener::MY_INFO: speak(UPDATE_USER, user); break;
		case ClientListener::QUIT: speak(REMOVE_USER, user); break;
		case ClientListener::HELLO: speak(UPDATE_USER, user); break;
		}
	}
	
	virtual void onAction(ClientListener::Types type, Client* /*client*/, const User::List& aList) {
		switch(type) {
		case ClientListener::OP_LIST: // Fall through
		case ClientListener::NICK_LIST: 
			speak(UPDATE_USERS, aList); break;
		}
	}

	virtual void onAction(ClientListener::Types type, Client* /*client*/, const User::Ptr& user, const string&  line) {
		switch(type) {
		case ClientListener::PRIVATE_MESSAGE: speak(PRIVATE_MESSAGE, user, line); break;
		}
	}
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHILDFRM_H__A7078724_FD85_4F39_8463_5A08A5F45E33__INCLUDED_)

/**
 * @file HubFrame.h
 * $Id: HubFrame.h,v 1.7 2002/05/09 15:26:46 arnetheduck Exp $
 */


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

#if !defined(AFX_CHILDFRM_H__A7078724_FD85_4F39_8463_5A08A5F45E33__INCLUDED_)
#define AFX_CHILDFRM_H__A7078724_FD85_4F39_8463_5A08A5F45E33__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "FlatTabCtrl.h"
#include "TypedListViewCtrl.h"

#include "../client/Client.h"
#include "../client/User.h"
#include "../client/ClientManager.h"
#include "../client/TimerManager.h"

#include "WinUtil.h"
#include "UCHandler.h"

#define EDIT_MESSAGE_MAP 10		// This could be any number, really...

class HubFrame : public MDITabChildWindowImpl<HubFrame>, private ClientListener, 
	public CSplitterImpl<HubFrame>, private TimerManagerListener, public UCHandler<HubFrame>
{
public:
	DECLARE_FRAME_WND_CLASS_EX("HubFrame", IDR_HUB, 0, COLOR_3DFACE);

	typedef CSplitterImpl<HubFrame> splitBase;
	typedef MDITabChildWindowImpl<HubFrame> baseClass;
	typedef UCHandler<HubFrame> ucBase;

	BEGIN_MSG_MAP(HubFrame)
		MESSAGE_HANDLER(WM_CLOSE, onClose)
		MESSAGE_HANDLER(WM_SETFOCUS, onSetFocus)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SPEAKER, onSpeaker)
		MESSAGE_HANDLER(WM_CONTEXTMENU, onContextMenu)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, onCtlColor)
		MESSAGE_HANDLER(WM_CTLCOLOREDIT, onCtlColor)
		MESSAGE_HANDLER(FTM_CONTEXTMENU, onTabContextMenu)
		COMMAND_ID_HANDLER(ID_FILE_RECONNECT, OnFileReconnect)
		COMMAND_ID_HANDLER(IDC_GETLIST, onGetList)
		COMMAND_ID_HANDLER(IDC_PRIVATEMESSAGE, onPrivateMessage)
		COMMAND_ID_HANDLER(IDC_REFRESH, onRefresh)
		COMMAND_ID_HANDLER(IDC_GRANTSLOT, onGrantSlot)
		COMMAND_ID_HANDLER(IDC_FOLLOW, onFollow)
		COMMAND_ID_HANDLER(IDC_SEND_MESSAGE, onSendMessage)
		COMMAND_ID_HANDLER(IDC_ADD_TO_FAVORITES, onAddToFavorites)
		COMMAND_ID_HANDLER(IDC_COPY_NICK, onCopyNick)
		COMMAND_ID_HANDLER(IDC_ADD_AS_FAVORITE, onAddAsFavorite)
		COMMAND_ID_HANDLER(IDC_CLOSE_WINDOW, onCloseWindow)
		COMMAND_ID_HANDLER(IDC_MATCH_QUEUE, onMatchQueue)
		NOTIFY_HANDLER(IDC_USERS, NM_DBLCLK, onDoubleClickUsers)	
		NOTIFY_HANDLER(IDC_USERS, LVN_KEYDOWN, onKeyDownUsers)
		NOTIFY_HANDLER(IDC_USERS, NM_RETURN, onEnterUsers)
		NOTIFY_CODE_HANDLER(TTN_GETDISPINFO, onGetToolTip)
		CHAIN_MSG_MAP(baseClass)
		CHAIN_MSG_MAP(splitBase)
		CHAIN_MSG_MAP(ucBase)
		REFLECT_NOTIFICATIONS()
	ALT_MSG_MAP(EDIT_MESSAGE_MAP)
		MESSAGE_HANDLER(WM_CHAR, onChar)
		MESSAGE_HANDLER(WM_KEYDOWN, onChar)
		MESSAGE_HANDLER(WM_KEYUP, onChar)
		MESSAGE_HANDLER(BM_SETCHECK, onShowUsers)
		MESSAGE_HANDLER(WM_LBUTTONDBLCLK, onLButton)
		MESSAGE_HANDLER(WM_RBUTTONDOWN, onContextMenu)
	END_MSG_MAP()

	virtual void OnFinalMessage(HWND /*hWnd*/) {
		dcassert(frames.find(server) != frames.end());
		dcassert(frames[server] == this);
		frames.erase(server);
		delete this;
	}

	LRESULT onSpeaker(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT onGetList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onAddToFavorites(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onCopyNick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onPrivateMessage(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onMatchQueue(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onGrantSlot(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onDoubleClickUsers(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT onTabContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT onChar(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onShowUsers(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onFollow(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onLButton(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onEnterUsers(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);
	LRESULT onGetToolTip(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);
	
	void UpdateLayout(BOOL bResizeBars = TRUE);
	void addLine(const string& aLine);
	void addClientLine(const string& aLine, bool inChat = true);
	void onEnter();
	void onTab();
	void runUserCommand(UserCommand& uc);

	static void openWindow(const string& server, const string& nick = Util::emptyString, const string& password = Util::emptyString, const string& description = Util::emptyString);
	static void closeDisconnected();
	
	LRESULT onSetFocus(UINT /* uMsg */, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		ctrlMessage.SetFocus();
		return 0;
	}

	LRESULT onSendMessage(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		onEnter();
		return 0;
	}
	
	LRESULT onAddAsFavorite(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		addAsFavorite();
		return 0;
	}

	LRESULT onCloseWindow(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		PostMessage(WM_CLOSE);
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

	LRESULT onRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		if(client->isConnected()) {
			clearUserList();
			client->refreshUserList();
		}
		return 0;
	}
	
	LRESULT OnFileReconnect(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		clearUserList();
		client->addListener(this);
		client->connect(server);
		return 0;
	}

	LRESULT onKeyDownUsers(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
		NMLVKEYDOWN* l = (NMLVKEYDOWN*)pnmh;
		if(l->wVKey == VK_TAB) {
			onTab();
		}
		return 0;
	}

private:

	enum { IDC_USER_COMMAND = 3500 };

	enum Speakers { UPDATE_USER, UPDATE_USERS, REMOVE_USER, ADD_CHAT_LINE,
		ADD_STATUS_LINE, ADD_SILENT_STATUS_LINE, SET_WINDOW_TITLE, GET_PASSWORD, 
		PRIVATE_MESSAGE, STATS, CONNECTED, DISCONNECTED
	};

	enum {
		IMAGE_USER = 0, IMAGE_OP
	};
	
	enum {
		COLUMN_FIRST, 
		COLUMN_NICK = COLUMN_FIRST, 
		COLUMN_SHARED, 
		COLUMN_DESCRIPTION, 
		COLUMN_TAG,
		COLUMN_CONNECTION, 
		COLUMN_EMAIL, 
		COLUMN_LAST
	};
	
	class UserInfo {
	public:
		UserInfo(const User::Ptr& u) : user(u) { update(); };
		User::Ptr user;

		const string& getText(int col) const {
			switch(col) {
				case COLUMN_NICK: return user->getNick();
				case COLUMN_SHARED: return shared;
				case COLUMN_DESCRIPTION: return user->getDescription();
				case COLUMN_TAG: return user->getTag();
				case COLUMN_CONNECTION: return user->getConnection();
				case COLUMN_EMAIL: return user->getEmail();
				default: return Util::emptyString;
			}
		}

		static int compareItems(UserInfo* a, UserInfo* b, int col) {
			switch(col) {
				case COLUMN_NICK:
					if(a->user->isSet(User::OP) && !b->user->isSet(User::OP)) {
						return -1;
					} else if(!a->user->isSet(User::OP) && b->user->isSet(User::OP)) {
						return 1;
					}
					return Util::stricmp(a->user->getNick(), b->user->getNick());	
				case COLUMN_SHARED:	return compare(a->user->getBytesShared(), b->user->getBytesShared());
				case COLUMN_DESCRIPTION: return Util::stricmp(a->user->getDescription(), b->user->getDescription());
				case COLUMN_TAG: return Util::stricmp(a->user->getTag(), b->user->getTag());
				case COLUMN_CONNECTION: return Util::stricmp(a->user->getConnection(), b->user->getConnection());
				case COLUMN_EMAIL: return Util::stricmp(a->user->getEmail(), b->user->getEmail());
				default: return 0;
			}
		}

		void update() { shared = Util::formatBytes(user->getBytesShared()); }

		GETSETREF(string, shared, Shared)
	};

	class PMInfo {
	public:
		PMInfo(const User::Ptr& u, const string& m) : user(u), msg(m) { };
		User::Ptr user;
		string msg;
	};

	HubFrame(const string& aServer, const string& aNick, const string& aPassword, const string& aDescription) : 
	waitingForPW(false), server(aServer), closed(false), updateUsers(false),
		ctrlMessageContainer("edit", this, EDIT_MESSAGE_MAP), 
		showUsersContainer("BUTTON", this, EDIT_MESSAGE_MAP),
		clientContainer("edit", this, EDIT_MESSAGE_MAP)
	{
		client = ClientManager::getInstance()->getClient();
		client->setUserInfo(BOOLSETTING(GET_USER_INFO));
		if(!aNick.empty())
			client->setNick(aNick);
		if (!aDescription.empty())
			client->setDescription(aDescription);
		client->setPassword(aPassword);
		client->addListener(this);
		TimerManager::getInstance()->addListener(this);
		timeStamps = BOOLSETTING(TIME_STAMPS);
	}

	~HubFrame() {
		ClientManager::getInstance()->putClient(client);
	}

	typedef HASH_MAP<string, HubFrame*> FrameMap;
	typedef FrameMap::iterator FrameIter;
	static FrameMap frames;

	string redirect;
	bool timeStamps;
	bool showJoins;
	string complete;

	string lastKick;
	string lastRedir;
	string lastServer;
	
	bool waitingForPW;

	Client* client;
	string server;
	CContainedWindow ctrlMessageContainer;
	CContainedWindow clientContainer;
	CContainedWindow showUsersContainer;

	CMenu userMenu;
	CMenu tabMenu;

	CButton ctrlShowUsers;
	CEdit ctrlClient;
	CEdit ctrlMessage;
	TypedListViewCtrl<UserInfo, IDC_USERS> ctrlUsers;
	CStatusBarCtrl ctrlStatus;

	bool closed;

	StringMap ucParams;
	StringMap tabParams;
	bool tabMenuShown;

	typedef vector<pair<User::Ptr, Speakers> > UpdateList;
	typedef UpdateList::iterator UpdateIter;
	UpdateList updateList;
	CriticalSection updateCS;
	bool updateUsers;

	enum { MAX_CLIENT_LINES = 5 };
	StringList lastLinesList;
	string lastLines;
	CToolTipCtrl ctrlLastLines;
	
	static int columnIndexes[COLUMN_LAST];
	static int columnSizes[COLUMN_LAST];
	
	bool updateUser(const User::Ptr& u, bool sorted = false);
	void addAsFavorite();

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
	virtual void onAction(TimerManagerListener::Types type, DWORD /*aTick*/) throw();

	// ClientListener
	virtual void onAction(ClientListener::Types type, Client* client) throw();
	virtual void onAction(ClientListener::Types type, Client* /*client*/, const string& line) throw();
	virtual void onAction(ClientListener::Types type, Client* /*client*/, const User::Ptr& user) throw();
	virtual void onAction(ClientListener::Types type, Client* /*client*/, const User::List& aList) throw();
	virtual void onAction(ClientListener::Types type, Client* /*client*/, const User::Ptr& user, const string&  line) throw();

	void speak(Speakers s) { PostMessage(WM_SPEAKER, (WPARAM)s); };
	void speak(Speakers s, const string& msg) { PostMessage(WM_SPEAKER, (WPARAM)s, (LPARAM)new string(msg)); };
	void speak(Speakers s, const User::Ptr& u) { 
		Lock l(updateCS);
		updateList.push_back(make_pair(u, s));
		updateUsers = true;
	};
	void speak(Speakers s, const User::Ptr& u, const string& line) { PostMessage(WM_SPEAKER, (WPARAM)s, (LPARAM)new PMInfo(u, line)); };

};
/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHILDFRM_H__A7078724_FD85_4F39_8463_5A08A5F45E33__INCLUDED_)

/**
 * @file
 * $Id: HubFrame.h,v 1.30 2003/11/04 20:18:14 arnetheduck Exp $
 */


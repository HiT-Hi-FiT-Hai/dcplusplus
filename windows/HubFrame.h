/* 
 * Copyright (C) 2001-2004 Jacek Sieka, j_s at telia com
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
#include "../client/FastAlloc.h"

#include "WinUtil.h"
#include "UCHandler.h"

#define EDIT_MESSAGE_MAP 10		// This could be any number, really...
struct CompareItems;

class HubFrame : public MDITabChildWindowImpl<HubFrame>, private ClientListener, 
	public CSplitterImpl<HubFrame>, private TimerManagerListener, public UCHandler<HubFrame>,
	public UserInfoBaseHandler<HubFrame>
{
public:
	DECLARE_FRAME_WND_CLASS_EX(_T("HubFrame"), IDR_HUB, 0, COLOR_3DFACE);

	typedef CSplitterImpl<HubFrame> splitBase;
	typedef MDITabChildWindowImpl<HubFrame> baseClass;
	typedef UCHandler<HubFrame> ucBase;
	typedef UserInfoBaseHandler<HubFrame> uibBase;

	BEGIN_MSG_MAP(HubFrame)
		NOTIFY_HANDLER(IDC_USERS, LVN_GETDISPINFO, ctrlUsers.onGetDispInfo)
		NOTIFY_HANDLER(IDC_USERS, LVN_COLUMNCLICK, ctrlUsers.onColumnClick)
		NOTIFY_HANDLER(IDC_USERS, NM_DBLCLK, onDoubleClickUsers)
		NOTIFY_HANDLER(IDC_USERS, LVN_KEYDOWN, onKeyDownUsers)
		NOTIFY_HANDLER(IDC_USERS, NM_RETURN, onEnterUsers)
		NOTIFY_CODE_HANDLER(TTN_GETDISPINFO, onGetToolTip)
		MESSAGE_HANDLER(WM_CLOSE, onClose)
		MESSAGE_HANDLER(WM_SETFOCUS, onSetFocus)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SPEAKER, onSpeaker)
		MESSAGE_HANDLER(WM_CONTEXTMENU, onContextMenu)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, onCtlColor)
		MESSAGE_HANDLER(WM_CTLCOLOREDIT, onCtlColor)
		MESSAGE_HANDLER(FTM_CONTEXTMENU, onTabContextMenu)
		COMMAND_ID_HANDLER(ID_FILE_RECONNECT, OnFileReconnect)
		COMMAND_ID_HANDLER(IDC_FOLLOW, onFollow)
		COMMAND_ID_HANDLER(IDC_SEND_MESSAGE, onSendMessage)
		COMMAND_ID_HANDLER(IDC_ADD_AS_FAVORITE, onAddAsFavorite)
		COMMAND_ID_HANDLER(IDC_COPY_NICK, onCopyNick)
		COMMAND_ID_HANDLER(IDC_CLOSE_WINDOW, onCloseWindow)
		CHAIN_COMMANDS(ucBase)
		CHAIN_COMMANDS(uibBase)
		CHAIN_MSG_MAP(baseClass)
		CHAIN_MSG_MAP(splitBase)
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
	LRESULT onCopyNick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
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
	void addLine(const tstring& aLine);
	void addClientLine(const tstring& aLine, bool inChat = true);
	void onEnter();
	void onTab();
	void runUserCommand(::UserCommand& uc);

	static void openWindow(const tstring& server);
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

	LRESULT OnFileReconnect(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		client->disconnect();
		clearUserList();
		client->connect();
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
	class UserInfo;
public:
	TypedListViewCtrl<UserInfo, IDC_USERS>& getUserList() { return ctrlUsers; };
private:

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
	friend struct CompareItems;
	class UserInfo : public UserInfoBase, public FastAlloc<UserInfo> {
	public:
		UserInfo(const User::Ptr& u) : UserInfoBase(u), op(false) { 
			update(); 
		};

		const tstring& getText(int col) const {
			return columns[col];
		}

		static int compareItems(const UserInfo* a, const UserInfo* b, int col) {
			if(col == COLUMN_NICK) {
				if(a->getOp() && !b->getOp()) {
					return -1;
				} else if(!a->getOp() && b->getOp()) {
					return 1;
				}
			}
			if(col == COLUMN_SHARED) {
				return compare(a->user->getBytesShared(), b->user->getBytesShared());
			}
			return Util::stricmp(a->columns[col], b->columns[col]);	
		}

		void update() { 
			columns[COLUMN_NICK] = Text::toT(user->getNick());
			columns[COLUMN_SHARED] = Text::toT(Util::formatBytes(user->getBytesShared()));
			columns[COLUMN_DESCRIPTION] = Text::toT(user->getDescription());
			columns[COLUMN_TAG] = Text::toT(user->getTag());
			columns[COLUMN_CONNECTION] = Text::toT(user->getConnection());
			columns[COLUMN_EMAIL] = Text::toT(user->getEmail());
			op = user->isSet(User::OP); 
		
		}

		tstring columns[COLUMN_LAST];
		GETSET(bool, op, Op);
	};

	class PMInfo {
	public:
		PMInfo(const User::Ptr& u, const string& m) : user(u), msg(Text::toT(m)) { };
		User::Ptr user;
		tstring msg;
	};

	HubFrame(const tstring& aServer) : 
	waitingForPW(false), extraSort(false), server(aServer), closed(false), 
		updateUsers(false), curCommandPosition(0),
		ctrlMessageContainer(WC_EDIT, this, EDIT_MESSAGE_MAP), 
		showUsersContainer(WC_BUTTON, this, EDIT_MESSAGE_MAP),
		clientContainer(WC_EDIT, this, EDIT_MESSAGE_MAP)
	{
		client = ClientManager::getInstance()->getClient(Text::fromT(aServer));
		client->addListener(this);
		timeStamps = BOOLSETTING(TIME_STAMPS);
	}

	~HubFrame() {
		ClientManager::getInstance()->putClient(client);
	}

	typedef HASH_MAP<tstring, HubFrame*> FrameMap;
	typedef FrameMap::iterator FrameIter;
	static FrameMap frames;

	tstring redirect;
	bool timeStamps;
	bool showJoins;
	bool favShowJoins;
	tstring complete;

	tstring lastKick;
	tstring lastRedir;
	tstring lastServer;
	
	bool waitingForPW;
	bool extraSort;

	TStringList prevCommands;
	tstring currentCommand;
	TStringList::size_type curCommandPosition;		//can't use an iterator because StringList is a vector, and vector iterators become invalid after resizing

	Client* client;
	tstring server;
	CContainedWindow ctrlMessageContainer;
	CContainedWindow clientContainer;
	CContainedWindow showUsersContainer;

	CMenu userMenu;
	CMenu tabMenu;

	CButton ctrlShowUsers;
	CEdit ctrlClient;
	CEdit ctrlMessage;
	typedef TypedListViewCtrl<UserInfo, IDC_USERS> CtrlUsers;
	CtrlUsers ctrlUsers;
	CStatusBarCtrl ctrlStatus;

	bool closed;

	StringMap ucParams;
	TStringMap tabParams;
	bool tabMenuShown;

	typedef vector<pair<User::Ptr, Speakers> > UpdateList;
	typedef UpdateList::iterator UpdateIter;
	typedef HASH_MAP<User::Ptr, UserInfo*, User::HashFunction> UserMap;
	typedef UserMap::iterator UserMapIter;

	UserMap userMap;
	UpdateList updateList;
	CriticalSection updateCS;
	bool updateUsers;

	enum { MAX_CLIENT_LINES = 5 };
	TStringList lastLinesList;
	tstring lastLines;
	CToolTipCtrl ctrlLastLines;
	
	static int columnIndexes[COLUMN_LAST];
	static int columnSizes[COLUMN_LAST];
	
	int findUser(const User::Ptr& aUser);

	bool updateUser(const User::Ptr& u);
	void addAsFavorite();

	bool getUserInfo() { return ctrlShowUsers.GetCheck() == BST_CHECKED; }

	void clearUserList() {
		{
			Lock l(updateCS);
			updateList.clear();
		}

		userMap.clear();
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
	virtual void on(TimerManagerListener::Second, DWORD /*aTick*/) throw();

	// ClientListener
	virtual void on(Connecting, Client*) throw();
	virtual void on(Connected, Client*) throw();
	virtual void on(BadPassword, Client*) throw();
	virtual void on(UserUpdated, Client*, const User::Ptr&) throw();
	virtual void on(UsersUpdated, Client*, const User::List&) throw();
	virtual void on(UserRemoved, Client*, const User::Ptr&) throw();
	virtual void on(Redirect, Client*, const string&) throw();
	virtual void on(Failed, Client*, const string&) throw();
	virtual void on(GetPassword, Client*) throw();
	virtual void on(HubUpdated, Client*) throw();
	virtual void on(Message, Client*, const string&) throw();
	virtual void on(PrivateMessage, Client*, const User::Ptr&, const string&) throw();
	virtual void on(NickTaken, Client*) throw();
	virtual void on(SearchFlood, Client*, const string&) throw();

	void speak(Speakers s) { PostMessage(WM_SPEAKER, (WPARAM)s); };
	void speak(Speakers s, const string& msg) { PostMessage(WM_SPEAKER, (WPARAM)s, (LPARAM)new tstring(Text::toT(msg))); };
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
 * $Id: HubFrame.h,v 1.55 2005/01/05 19:21:41 arnetheduck Exp $
 */


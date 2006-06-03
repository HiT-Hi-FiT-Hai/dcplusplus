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

#if !defined(HUB_FRAME_H)
#define HUB_FRAME_H

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
#define FILTER_MESSAGE_MAP 8
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
		NOTIFY_HANDLER(IDC_USERS, LVN_ITEMCHANGED, onItemChanged)
		NOTIFY_CODE_HANDLER(TTN_GETDISPINFO, onGetToolTip)
		MESSAGE_HANDLER(WM_CLOSE, onClose)
		MESSAGE_HANDLER(WM_SETFOCUS, onSetFocus)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SPEAKER, onSpeaker)
		MESSAGE_HANDLER(WM_CONTEXTMENU, onContextMenu)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, onCtlColor)
		MESSAGE_HANDLER(WM_CTLCOLOREDIT, onCtlColor)
		MESSAGE_HANDLER(FTM_CONTEXTMENU, onTabContextMenu)
		COMMAND_ID_HANDLER(ID_FILE_RECONNECT, onFileReconnect)
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
		MESSAGE_HANDLER(WM_CONTEXTMENU, onContextMenu)
	ALT_MSG_MAP(FILTER_MESSAGE_MAP)
		MESSAGE_HANDLER(WM_CHAR, onFilterChar)
		MESSAGE_HANDLER(WM_KEYUP, onFilterChar)
		COMMAND_CODE_HANDLER(CBN_SELCHANGE, onSelChange)
	END_MSG_MAP()

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
	LRESULT onFilterChar(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onSelChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onCtlColor(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT onFileReconnect(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	
	void UpdateLayout(BOOL bResizeBars = TRUE);
	void addLine(const tstring& aLine);
	void addClientLine(const tstring& aLine, bool inChat = true);
	void onEnter();
	void onTab();
	void handleTab(bool reverse);
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

	LRESULT onItemChanged(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/) {
		updateStatusBar();
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
	TypedListViewCtrl<UserInfo, IDC_USERS>& getUserList() { return ctrlUsers; }
private:
	enum Speakers { UPDATE_USER_JOIN, UPDATE_USER, REMOVE_USER, ADD_CHAT_LINE,
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
		COLUMN_CID,
		COLUMN_LAST
	};

	struct Task {
		Task(Speakers speaker_) : speaker(speaker_) { }
		virtual ~Task() { }
		Speakers speaker;
	};

	struct UserTask : public Task {
		UserTask(Speakers speaker_, const OnlineUser& ou) : Task(speaker_), user(ou.getUser()), identity(ou.getIdentity()) { }

		User::Ptr user;
		Identity identity;
	};

	struct StringTask : public Task {
		StringTask(Speakers speaker_, const tstring& msg_) : Task(speaker_), msg(msg_) { }
		tstring msg;
	};

	struct PMTask : public StringTask {
		PMTask(const User::Ptr& from_, const User::Ptr& to_, const User::Ptr& replyTo_, const tstring& m) : StringTask(PRIVATE_MESSAGE, m), from(from_), to(to_), replyTo(replyTo_) { }
		User::Ptr from;
		User::Ptr to;
		User::Ptr replyTo;
	};

	friend struct CompareItems;
	class UserInfo : public UserInfoBase, public FastAlloc<UserInfo> {
	public:
		UserInfo(const UserTask& u) : UserInfoBase(u.user) { 
			update(u.identity, -1); 
		}

		const tstring& getText(int col) const {
			return columns[col];
		}

		static int compareItems(const UserInfo* a, const UserInfo* b, int col) {
			if(col == COLUMN_NICK) {
				if(a->getIdentity().isOp() && !b->getIdentity().isOp()) {
					return -1;
				} else if(!a->getIdentity().isOp() && b->getIdentity().isOp()) {
					return 1;
				}
			}
			if(col == COLUMN_SHARED) {
				return compare(a->identity.getBytesShared(), b->identity.getBytesShared());;
			}
			return lstrcmpi(a->columns[col].c_str(), b->columns[col].c_str());	
		}

		bool update(const Identity& identity, int sortCol);

		tstring columns[COLUMN_LAST];
		GETSET(Identity, identity, Identity);
	};


	HubFrame(const tstring& aServer) : 
	waitingForPW(false), extraSort(false), server(aServer), closed(false), 
		showUsers(BOOLSETTING(GET_USER_INFO)), updateUsers(false), resort(false),
		curCommandPosition(0), timeStamps(BOOLSETTING(TIME_STAMPS)),
		ctrlMessageContainer(WC_EDIT, this, EDIT_MESSAGE_MAP), 
		showUsersContainer(WC_BUTTON, this, EDIT_MESSAGE_MAP),
		clientContainer(WC_EDIT, this, EDIT_MESSAGE_MAP),
		ctrlFilterContainer(WC_EDIT, this, FILTER_MESSAGE_MAP),
		ctrlFilterSelContainer(WC_COMBOBOX, this, FILTER_MESSAGE_MAP)
	{
		client = ClientManager::getInstance()->getClient(Text::fromT(aServer));
		client->addListener(this);
	}

	virtual ~HubFrame() {
		ClientManager::getInstance()->putClient(client);

		dcassert(frames.find(server) != frames.end());
		dcassert(frames[server] == this);
		frames.erase(server);

		clearTaskList();
	}

	typedef HASH_MAP<tstring, HubFrame*> FrameMap;
	typedef FrameMap::iterator FrameIter;
	static FrameMap frames;

	typedef vector<Task*> TaskList;
	typedef TaskList::iterator TaskIter;
	typedef HASH_MAP<User::Ptr, UserInfo*, User::HashFunction> UserMap;
	typedef UserMap::iterator UserMapIter;

	tstring redirect;
	bool timeStamps;
	bool showJoins;
	bool favShowJoins;
	tstring complete;

	bool waitingForPW;
	bool extraSort;

	TStringList prevCommands;
	tstring currentCommand;
	TStringList::size_type curCommandPosition;		//can't use an iterator because StringList is a vector, and vector iterators become invalid after resizing

	struct CountAvailable {
		CountAvailable() : available(0) { }
		int64_t available;
		void operator()(UserInfo *ui) {
			available += ui->getIdentity().getBytesShared();
		}
		void operator()(UserMap::const_reference ui) {
			available += ui.second->getIdentity().getBytesShared();
		}
	};

	size_t getUserCount() const {
		size_t sel = ctrlUsers.GetSelectedCount();
		return sel > 1 ? sel : userMap.size();
	}

	int64_t getAvailable() {
		if (ctrlUsers.GetSelectedCount() > 1) {
			return ctrlUsers.forEachSelectedT(CountAvailable()).available;
		} else
			return for_each(userMap.begin(), userMap.end(), CountAvailable()).available;
	}

	const tstring& getNick(const User::Ptr& u);

	Client* client;
	tstring server;
	CContainedWindow ctrlMessageContainer;
	CContainedWindow clientContainer;
	CContainedWindow showUsersContainer;
	CContainedWindow ctrlFilterContainer;
	CContainedWindow ctrlFilterSelContainer;
	
	CMenu userMenu;
	CMenu tabMenu;

	CButton ctrlShowUsers;
	CEdit ctrlClient;
	CEdit ctrlMessage;
	CEdit ctrlFilter;
	CComboBox ctrlFilterSel;
	typedef TypedListViewCtrl<UserInfo, IDC_USERS> CtrlUsers;
	CtrlUsers ctrlUsers;
	CStatusBarCtrl ctrlStatus;

	tstring filter;

	bool closed;
	bool showUsers;

	TStringMap tabParams;
	bool tabMenuShown;

	UserMap userMap;
	TaskList taskList;
	CriticalSection taskCS;
	bool updateUsers;
	bool resort;

	StringMap ucLineParams;

	enum { MAX_CLIENT_LINES = 5 };
	TStringList lastLinesList;
	tstring lastLines;
	CToolTipCtrl ctrlLastLines;
	
	static int columnIndexes[COLUMN_LAST];
	static int columnSizes[COLUMN_LAST];
	
	bool updateUser(const UserTask& u);
	void removeUser(const User::Ptr& aUser);

	void updateUserList(UserInfo* ui = NULL);
	bool parseFilter(int& mode, int64_t& size);
	bool matchFilter(const UserInfo& ui, int sel, bool doSizeCompare = false, int mode = 0, int64_t size = 0);
	UserInfo* findUser(const tstring& nick);

	void addAsFavorite();
	void removeFavoriteHub();

	void clearUserList();
	void clearTaskList();

	int getImage(const Identity& u);

	void updateStatusBar() { if(m_hWnd) speak(STATS); }

	// TimerManagerListener
	virtual void on(TimerManagerListener::Second, DWORD /*aTick*/) throw();

	// ClientListener
	virtual void on(Connecting, Client*) throw();
	virtual void on(Connected, Client*) throw();
	virtual void on(BadPassword, Client*) throw();
	virtual void on(UserUpdated, Client*, const OnlineUser&) throw();
	virtual void on(UsersUpdated, Client*, const OnlineUser::List&) throw();
	virtual void on(UserRemoved, Client*, const OnlineUser&) throw();
	virtual void on(Redirect, Client*, const string&) throw();
	virtual void on(Failed, Client*, const string&) throw();
	virtual void on(GetPassword, Client*) throw();
	virtual void on(HubUpdated, Client*) throw();
	virtual void on(Message, Client*, const OnlineUser&, const string&) throw();
	virtual void on(StatusMessage, Client*, const string&) throw();
	virtual void on(PrivateMessage, Client*, const OnlineUser&, const OnlineUser&, const OnlineUser&, const string&) throw();
	virtual void on(NickTaken, Client*) throw();
	virtual void on(SearchFlood, Client*, const string&) throw();

	void speak(Speakers s) { Lock l(taskCS); taskList.push_back(new Task(s)); PostMessage(WM_SPEAKER); }
	void speak(Speakers s, const string& msg) { Lock l(taskCS); taskList.push_back(new StringTask(s, Text::toT(msg))); PostMessage(WM_SPEAKER); }
	void speak(Speakers s, const OnlineUser& u) { Lock l(taskCS); taskList.push_back(new UserTask(s, u)); updateUsers = true; }
	void speak(const OnlineUser& from, const OnlineUser& to, const OnlineUser& replyTo, const string& line) { Lock l(taskCS); taskList.push_back(new PMTask(from, to, replyTo, Text::toT(line))); }
};

#endif // !defined(HUB_FRAME_H)

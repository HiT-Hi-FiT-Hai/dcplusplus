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

#include "Client.h"
#include "ExListViewCtrl.h"
#include "User.h"
#include "CriticalSection.h"
#include "ClientManager.h"
#include "PrivateFrame.h"
#include "TimerManager.h"

#include "FlatTabCtrl.h"

#define EDIT_MESSAGE_MAP 10		// This could be any number, really...

class HubFrame : public MDITabChildWindowImpl<HubFrame>, private ClientListener, public CSplitterImpl<HubFrame>, private TimerManagerListener
{
public:
	HubFrame(const string& aServer, const string& aNick = "", const string& aPassword = "") : op(false), ctrlMessageContainer("edit", this, EDIT_MESSAGE_MAP), server(aServer) {
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
			::SetBkColor(hDC, Util::bgColor);
			::SetTextColor(hDC, Util::textColor);
			return (LRESULT)Util::bgBrush;
			
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

	void addClientLine(const string& aLine) {
		ctrlStatus.SetText(0, ("[" + Util::getShortTimeString() + "] " + aLine).c_str());
		setDirty();
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
	
	string lastKick;
	string lastRedir;
	string lastServer;

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
			client->setPassword(""); break;
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
			PostMessage(WM_SPEAKER, CLIENT_MYINFO, (LPARAM)x); break;
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
 * $Id: HubFrame.h,v 1.60 2002/03/25 22:23:25 arnetheduck Exp $
 * @if LOG
 * $Log: HubFrame.h,v $
 * Revision 1.60  2002/03/25 22:23:25  arnetheduck
 * Lots of minor updates
 *
 * Revision 1.59  2002/03/15 11:59:35  arnetheduck
 * Final changes (I hope...) for 0.155
 *
 * Revision 1.58  2002/03/11 22:58:54  arnetheduck
 * A step towards internationalization
 *
 * Revision 1.57  2002/03/05 11:19:35  arnetheduck
 * Fixed a window closing bug
 *
 * Revision 1.56  2002/03/04 23:52:31  arnetheduck
 * Updates and bugfixes, new user handling almost finished...
 *
 * Revision 1.55  2002/02/28 00:10:47  arnetheduck
 * Some fixes to the new user model
 *
 * Revision 1.54  2002/02/27 12:02:09  arnetheduck
 * Completely new user handling, wonder how it turns out...
 *
 * Revision 1.53  2002/02/26 23:25:22  arnetheduck
 * Minor updates and fixes
 *
 * Revision 1.52  2002/02/25 15:39:28  arnetheduck
 * Release 0.154, lot of things fixed...
 *
 * Revision 1.51  2002/02/18 23:48:32  arnetheduck
 * New prerelease, bugs fixed and features added...
 *
 * Revision 1.50  2002/02/12 00:35:37  arnetheduck
 * 0.153
 *
 * Revision 1.49  2002/02/09 18:13:51  arnetheduck
 * Fixed level 4 warnings and started using new stl
 *
 * Revision 1.48  2002/02/07 17:25:28  arnetheduck
 * many bugs fixed, time for 0.152 I think
 *
 * Revision 1.47  2002/02/04 01:10:30  arnetheduck
 * Release 0.151...a lot of things fixed
 *
 * Revision 1.46  2002/02/03 01:06:56  arnetheduck
 * More bugfixes and some minor changes
 *
 * Revision 1.45  2002/02/01 02:00:30  arnetheduck
 * A lot of work done on the new queue manager, hopefully this should reduce
 * the number of crashes...
 *
 * Revision 1.44  2002/01/26 21:09:51  arnetheduck
 * Release 0.14
 *
 * Revision 1.43  2002/01/26 16:34:00  arnetheduck
 * Colors dialog added, as well as some other options
 *
 * Revision 1.42  2002/01/26 14:59:22  arnetheduck
 * Fixed disconnect crash
 *
 * Revision 1.41  2002/01/26 12:06:39  arnetheduck
 * Småsaker
 *
 * Revision 1.40  2002/01/20 22:54:46  arnetheduck
 * Bugfixes to 0.131 mainly...
 *
 * Revision 1.39  2002/01/19 19:07:39  arnetheduck
 * Last fixes before 0.13
 *
 * Revision 1.38  2002/01/17 23:35:59  arnetheduck
 * Reworked threading once more, now it actually seems stable. Also made
 * sure that noone tries to access client objects that have been deleted
 * as well as some other minor updates
 *
 * Revision 1.37  2002/01/16 20:56:26  arnetheduck
 * Bug fixes, file listing sort and some other small changes
 *
 * Revision 1.36  2002/01/15 21:57:53  arnetheduck
 * Hopefully fixed the two annoying bugs...
 *
 * Revision 1.35  2002/01/14 01:56:33  arnetheduck
 * All done for 0.12
 *
 * Revision 1.34  2002/01/13 22:50:48  arnetheduck
 * Time for 0.12, added favorites, a bunch of new icons and lot's of other stuff
 *
 * Revision 1.33  2002/01/11 16:13:33  arnetheduck
 * Fixed some locks and bugs, added type field to the search frame
 *
 * Revision 1.32  2002/01/11 14:52:57  arnetheduck
 * Huge changes in the listener code, replaced most of it with templates,
 * also moved the getinstance stuff for the managers to a template
 *
 * Revision 1.31  2002/01/10 12:33:14  arnetheduck
 * Various fixes
 *
 * Revision 1.30  2002/01/08 00:24:10  arnetheduck
 * Last bugs fixed before 0.11
 *
 * Revision 1.29  2002/01/07 20:17:59  arnetheduck
 * Finally fixed the reconnect bug that's been annoying me for a whole day...
 * Hopefully the app works better in w95 now too...
 *
 * Revision 1.28  2002/01/06 21:55:20  arnetheduck
 * Some minor bugs fixed, but there remains one strange thing, the reconnect
 * button doesn't work...
 *
 * Revision 1.27  2002/01/05 19:06:09  arnetheduck
 * Added user list images, fixed bugs and made things more effective
 *
 * Revision 1.25  2002/01/05 10:13:39  arnetheduck
 * Automatic version detection and some other updates
 *
 * Revision 1.24  2002/01/02 16:12:32  arnetheduck
 * Added code for multiple download sources
 *
 * Revision 1.23  2001/12/29 13:47:14  arnetheduck
 * Fixing bugs and UI work
 *
 * Revision 1.22  2001/12/27 18:14:36  arnetheduck
 * Version 0.08, here we go...
 *
 * Revision 1.21  2001/12/27 12:05:00  arnetheduck
 * Added flat tabs, fixed sorting and a StringTokenizer bug
 *
 * Revision 1.20  2001/12/21 23:52:30  arnetheduck
 * Last commit for five days
 *
 * Revision 1.19  2001/12/21 20:21:17  arnetheduck
 * Private messaging added, and a lot of other updates as well...
 *
 * Revision 1.18  2001/12/19 23:07:59  arnetheduck
 * Added directory downloading from the directory tree (although it hasn't been
 * tested at all) and password support.
 *
 * Revision 1.17  2001/12/18 12:32:18  arnetheduck
 * Stability fixes
 *
 * Revision 1.16  2001/12/16 19:47:48  arnetheduck
 * Reworked downloading and user handling some, and changed some small UI things
 *
 * Revision 1.15  2001/12/15 17:01:06  arnetheduck
 * Passive mode searching as well as some searching code added
 *
 * Revision 1.14  2001/12/13 19:21:57  arnetheduck
 * A lot of work done almost everywhere, mainly towards a friendlier UI
 * and less bugs...time to release 0.06...
 *
 * Revision 1.13  2001/12/12 00:06:04  arnetheduck
 * Updated the public hub listings, fixed some minor transfer bugs, reworked the
 * sockets to use only one thread (instead of an extra thread for sending files),
 * and fixed a major bug in the client command decoding (still have to fix this
 * one for the userconnections...)
 *
 * Revision 1.12  2001/12/08 20:59:26  arnetheduck
 * Fixing bugs...
 *
 * Revision 1.11  2001/12/08 14:25:49  arnetheduck
 * More bugs removed...did my first search as well...
 *
 * Revision 1.10  2001/12/07 20:03:07  arnetheduck
 * More work done towards application stability
 *
 * Revision 1.9  2001/12/04 21:50:34  arnetheduck
 * Work done towards application stability...still a lot to do though...
 * a bit more and it's time for a new release.
 *
 * Revision 1.8  2001/12/02 23:47:35  arnetheduck
 * Added the framework for uploading and file sharing...although there's something strange about
 * the file lists...my client takes them, but not the original...
 *
 * Revision 1.7  2001/12/02 11:16:46  arnetheduck
 * Optimised hub listing, removed a few bugs and leaks, and added a few small
 * things...downloads are now working, time to start writing the sharing
 * code...
 *
 * Revision 1.6  2001/11/29 19:10:55  arnetheduck
 * Refactored down/uploading and some other things completely.
 * Also added download indicators and download resuming, along
 * with some other stuff.
 *
 * Revision 1.5  2001/11/26 23:40:36  arnetheduck
 * Downloads!! Now downloads are possible, although the implementation is
 * likely to change in the future...more UI work (splitters...) and some bug
 * fixes. Only user file listings are downloadable, but at least it's something...
 *
 * Revision 1.4  2001/11/25 22:06:25  arnetheduck
 * Finally downloading is working! There are now a few quirks and bugs to be fixed
 * but what the heck....!
 *
 * Revision 1.3  2001/11/24 10:37:09  arnetheduck
 * onQuit is now handled
 * User list sorting
 * File sizes correcly cut down to B, kB, MB, GB and TB
 *
 * Revision 1.2  2001/11/22 19:47:42  arnetheduck
 * A simple XML parser. Doesn't have all the features, but works good enough for
 * the configuration file.
 *
 * Revision 1.1.1.1  2001/11/21 17:33:20  arnetheduck
 * Inital release
 *
 * @endif
 */


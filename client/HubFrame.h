/* 
 * Copyright (C) 2001 Jacek Sieka, jacek@creatio.se
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
	enum {
		COLUMN_NICK,
		COLUMN_SHARED,
		COLUMN_DESCRIPTION,
		COLUMN_CONNECTION,
		COLUMN_EMAIL
	};
	HubFrame(const string& aServer, const string& aNick = "", const string& aPassword = "") : op(false), ctrlMessageContainer("edit", this, EDIT_MESSAGE_MAP), server(aServer), stopperThread(NULL) {
		client = ClientManager::getInstance()->getClient();
		client->setNick(aNick);
		client->setPassword(aPassword);
		client->addListener(this);
		TimerManager::getInstance()->addListener(this);
	}

	~HubFrame() {
		dcassert(client == NULL);
	}

	DECLARE_FRAME_WND_CLASS("HubFrame", IDR_HUB);

	CEdit ctrlClient;
	CEdit ctrlMessage;
	ExListViewCtrl ctrlUsers;
	CStatusBarCtrl ctrlStatus;
	CriticalSection cs;

	HANDLE stopperThread;
	
	virtual void OnFinalMessage(HWND /*hWnd*/) {
		delete this;
	}

	typedef CSplitterImpl<HubFrame> splitBase;

	BEGIN_MSG_MAP(HubFrame)
		MESSAGE_HANDLER(WM_CLOSE, onClose)
		MESSAGE_HANDLER(WM_SETFOCUS, OnFocus)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_FORWARDMSG, OnForwardMsg)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_SPEAKER, onSpeaker)
		MESSAGE_HANDLER(WM_CONTEXTMENU, onContextMenu)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, onCtlColor)
		COMMAND_ID_HANDLER(ID_FILE_RECONNECT, OnFileReconnect)
		COMMAND_ID_HANDLER(IDC_GETLIST, onGetList)
		COMMAND_ID_HANDLER(IDC_PRIVATEMESSAGE, onPrivateMessage)
		COMMAND_ID_HANDLER(IDC_REFRESH, onRefresh)
		COMMAND_ID_HANDLER(IDC_KICK, onKick)
		COMMAND_ID_HANDLER(IDC_REDIRECT, onRedirect)
		COMMAND_ID_HANDLER(IDC_FOLLOW, onFollow)
		NOTIFY_HANDLER(IDC_USERS, NM_DBLCLK, onDoubleClickUsers)	
		NOTIFY_HANDLER(IDC_USERS, LVN_COLUMNCLICK, onColumnClickUsers)
		CHAIN_MSG_MAP(MDITabChildWindowImpl<HubFrame>)
		CHAIN_MSG_MAP(splitBase)
	ALT_MSG_MAP(EDIT_MESSAGE_MAP)
		MESSAGE_HANDLER(WM_CHAR, OnChar)
	END_MSG_MAP()


	LRESULT onCtlColor(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		HWND hWnd = (HWND)lParam;
		if(hWnd == ctrlClient.m_hWnd) {
			return (LRESULT)GetSysColorBrush(COLOR_WINDOW);
		}
		bHandled = FALSE;
		return FALSE;
	};
	
	LRESULT onSpeaker(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT onGetList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onPrivateMessage(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	LRESULT onFollow(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		if(!redirect.empty()) {
			client->disconnect();
			client->connect(redirect);
		}
		return 0;
	}

	LRESULT onRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		if(client->isConnected()) {
			ctrlUsers.DeleteAllItems();
			client->getNickList();
		}
		return 0;
	}

	LRESULT onKick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onRedirect(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	
	LRESULT onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled) {
		RECT rc;                    // client area of window 
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };        // location of mouse click 
		
		// Get the bounding rectangle of the client area. 
		ctrlUsers.GetClientRect(&rc);
		ctrlUsers.ScreenToClient(&pt); 
		
		if (PtInRect(&rc, pt)) 
		{ 
			ctrlUsers.ClientToScreen(&pt);
			if(op) {
				opMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
			} else {
				userMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
			}
			
			return TRUE; 
		}

		return FALSE; 
	}

	LRESULT onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled) {
		DWORD id;
		if(stopperThread) {
			if(WaitForSingleObject(stopperThread, 0) == WAIT_TIMEOUT) {
				// Hm, the thread's not finished stopping the client yet...post a close message and continue processing...
				PostMessage(WM_CLOSE);
				return 0;
			}
			int i = 0;
			while(i < ctrlUsers.GetItemCount()) {
				delete (UserInfo*)ctrlUsers.GetItemData(i);
				i++;
			}
			CloseHandle(stopperThread);
			stopperThread = FALSE;
			bHandled = FALSE;
		} else {
			stopperThread = CreateThread(NULL, 0, stopper, this, 0, &id);
		
		}
		return 0;
	}

	static DWORD WINAPI stopper(void* p) {
		HubFrame* f = (HubFrame*)p;

		TimerManager::getInstance()->removeListener(f);
		f->cs.enter();
		ClientManager::getInstance()->putClient(f->client);
		f->client = NULL;
		f->cs.leave();
		f->PostMessage(WM_CLOSE);
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
		rc.bottom -=28;
		SetSplitterRect(rc);
		
		rc = rect;
		rc.bottom -= 2;
		rc.top = rc.bottom - 22;
		rc.left +=2;
		rc.right -=2;
		ctrlMessage.MoveWindow(rc);

	}
	
	LRESULT OnFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled) {
		bHandled = FALSE;
		ctrlMessage.SetFocus();
		return 0;
	}
	
	LRESULT OnFileReconnect(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled) {
		cs.enter();
		client->disconnect();
		ctrlUsers.DeleteAllItems();
		client->connect(server);
		cs.leave();
		return 0;
	}

	LRESULT OnForwardMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
	{
		LPMSG pMsg = (LPMSG)lParam;
		
		return MDITabChildWindowImpl<HubFrame>::PreTranslateMessage(pMsg);
	}
	
	LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
		return 0;
	}
		
	LRESULT onDoubleClickUsers(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	
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

	LRESULT onColumnClickUsers(int idCtrl, LPNMHDR pnmh, BOOL& bHandled) {
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

	void addLine(const string& aLine) {
		if(ctrlClient.GetWindowTextLength() > 20000) {
			// We want to limit the buffer to the last 20000 characters...after that, w95 becomes sad...
			ctrlClient.SetRedraw(FALSE);
			ctrlClient.SetSel(0, ctrlClient.LineIndex(ctrlClient.LineFromChar(2000)), TRUE);
			ctrlClient.ReplaceSel("");
			ctrlClient.SetRedraw(TRUE);
		}
		ctrlClient.AppendText(aLine.c_str());
		ctrlClient.AppendText("\r\n");
	}

	void addClientLine(const char* aLine) {
		ctrlStatus.SetText(0, aLine);
	}
	void addClientLine(const string& aLine) {
		addClientLine(aLine.c_str());
	}

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(&ps);
		FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_ACTIVEBORDER+1));
		EndPaint(&ps);
		return 0;
	}
	LRESULT OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		char* message;

		if(wParam == VK_RETURN && ctrlMessage.GetWindowTextLength() > 0) {
			message = new char[ctrlMessage.GetWindowTextLength()+1];
			ctrlMessage.GetWindowText(message, ctrlMessage.GetWindowTextLength()+1);
			string s(message, ctrlMessage.GetWindowTextLength());
			delete message;
			client->sendMessage(s);
			ctrlMessage.SetWindowText("");
		} else {
			bHandled = FALSE;
		}
		return 0;
	}
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
		STATS
	};

	enum {
		IMAGE_USER = 0,
		IMAGE_OP
	};
	
	class UserInfo {
	public:
		LONGLONG size;
	};

	class PMInfo {
	public:
		PrivateFrame* frm;
		string msg;
	};

	string redirect;

	// TimerManagerListener
	virtual void onAction(TimerManagerListener::Types type, DWORD aTick) {
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
	
	virtual void onAction(ClientListener::Types type, Client* client, const string& line) {
		string* x = new string(line);
		switch(type) {
		case ClientListener::FAILED:
			PostMessage(WM_SPEAKER, CLIENT_FAILED, (LPARAM)x); break;
		case ClientListener::MESSAGE:
			PostMessage(WM_SPEAKER, CLIENT_MESSAGE, (LPARAM) x); break;
		case ClientListener::FORCE_MOVE:
			redirect = line; break;
		}
	}

	virtual void onAction(ClientListener::Types type, Client* client, const User::Ptr& user) {
		User::Ptr* x = new User::Ptr();
		*x = user;
		switch(type) {
		case ClientListener::MY_INFO:
			PostMessage(WM_SPEAKER, CLIENT_MYINFO, (LPARAM)x); break;
		case ClientListener::QUIT:
			PostMessage(WM_SPEAKER, CLIENT_QUIT, (LPARAM)x); break;
			
		}
	}
	
	virtual void onAction(ClientListener::Types type, Client* client, const StringList& aList) {
		switch(type) {
		case ClientListener::OP_LIST:
			for(StringIterC i = aList.begin(); i != aList.end(); ++i) {
				if(*i == client->getNick()) {
					op = true;
					return;
				}
			}
			break;
		}
	}

	virtual void onAction(ClientListener::Types type, Client* client, const User::Ptr& user, const string&  line) {
		User::Ptr* x = new User::Ptr();
		*x = user;
		switch(type) {
		case ClientListener::PRIVATE_MESSAGE:
			PMInfo* i = new PMInfo();
			
			i->frm = PrivateFrame::getFrame(user, m_hWndMDIClient);
			i->msg = line;
			PostMessage(WM_SPEAKER, CLIENT_PRIVATEMESSAGE, (LPARAM)i);
			break;
		}
	}

	virtual void onAction(ClientListener::Types type, Client* client, const string& line1, const string& line2) {
		switch(type) {
		case ClientListener::PRIVATE_MESSAGE:
			string* msg = new string("Private message from " + line1 + "\r\n" + line2);
			PostMessage(WM_SPEAKER, CLIENT_MESSAGE, (LPARAM) msg);
			break;
		}
	}
	
	void updateStatusBar() {
		PostMessage(WM_SPEAKER, STATS);
	}

	Client::Ptr client;
	string server;
	CContainedWindow ctrlMessageContainer;
	CMenu userMenu;
	CMenu opMenu;
	bool op;

	static CImageList* images;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHILDFRM_H__A7078724_FD85_4F39_8463_5A08A5F45E33__INCLUDED_)

/**
 * @file HubFrame.h
 * $Id: HubFrame.h,v 1.39 2002/01/19 19:07:39 arnetheduck Exp $
 * @if LOG
 * $Log: HubFrame.h,v $
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


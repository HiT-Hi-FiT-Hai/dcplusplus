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

#include "ClientListener.h"
#include "Client.h"
#include "ExListViewCtrl.h"
#include "DownloadManager.h"
#include "User.h"
#include "LineDlg.h"
#include "CriticalSection.h"
#include "ClientManager.h"
#include "PrivateFrame.h"

#include "AtlCmdBar2.h"

#define EDIT_MESSAGE_MAP 10		// This could be any number, really...

class HubFrame : public MDITabChildWindowImpl<HubFrame>, private ClientListener, public CSplitterImpl<HubFrame>
{
protected:
	enum {
		CLIENT_CONNECTING,
		CLIENT_ERROR,
		CLIENT_GETPASSWORD,
		CLIENT_HUBNAME,
		CLIENT_MESSAGE,
		CLIENT_MYINFO,
		CLIENT_PRIVATEMESSAGE,
		CLIENT_QUIT,
		CLIENT_UNKNOWN,
		CLIENT_VALIDATEDENIED
	};

	StringList clientMessages;
	string clientError;
	User::List clientMyInfo;
	User::List clientQuit;
	map<PrivateFrame*, string> clientPrivateMessage;

	class UserInfo {
	public:
		LONGLONG size;
	};

	LRESULT onSpeaker(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
		cs.enter();
		// First some specials to handle those messages that have to initialize variables...
		if(wParam == CLIENT_MESSAGE) {
			StringIter i = clientMessages.begin();
			while(i != clientMessages.end()) {
				addLine(*i);
				i = clientMessages.erase(i);
			}
		} else if(wParam == CLIENT_MYINFO) {
			User::Iter i = clientMyInfo.begin();
			while(i != clientMyInfo.end()) {
				User::Ptr& u = *i;
				LV_FINDINFO fi;
				fi.flags = LVFI_STRING;
				fi.psz = u->getNick().c_str();
				int j = ctrlUsers.FindItem(&fi, -1);
				if(j == -1) {
					UserInfo* ui = new UserInfo;
					ui->size = u->getBytesShared();
					StringList l;
					l.push_back(u->getNick());
					l.push_back(Util::formatBytes(u->getBytesSharedString()));
					l.push_back(u->getDescription());
					l.push_back(u->getConnection());
					l.push_back(u->getEmail());
					ctrlUsers.insert(l, 0, (LPARAM)ui);
				} else {
					ctrlUsers.SetItemText(j, 1, Util::formatBytes(u->getBytesShared()).c_str());
					ctrlUsers.SetItemText(j, 2, u->getDescription().c_str());
					ctrlUsers.SetItemText(j, 3, u->getConnection().c_str());
					ctrlUsers.SetItemText(j, 4, u->getEmail().c_str());
				}
				
				updateStatusBar();
				i = clientMyInfo.erase(i);
			}
		} else if(wParam == CLIENT_QUIT) {
			User::Iter i = clientQuit.begin();
			while(i != clientQuit.end()) {
				int item = ctrlUsers.find((*i)->getNick());
				if(item != -1) {
					delete (UserInfo*)ctrlUsers.GetItemData(item);
					ctrlUsers.DeleteItem(item);
				}
				updateStatusBar();		
				i = clientQuit.erase(i);
			}
		} else if(wParam == CLIENT_GETPASSWORD) {
			LineDlg dlg;
			dlg.title = "Hub Password";
			dlg.description = "Please enter your password";
			dlg.password = true;
			
			if(dlg.DoModal() == IDOK) {
				client->password(dlg.line);
			} else {
				client->disconnect();
			}
		} else if(wParam == CLIENT_CONNECTING) {
			addClientLine("Connecting to " + client->getServer() + "...");
			SetWindowText(client->getServer().c_str());
		} else if(wParam == CLIENT_ERROR) {
			addClientLine(clientError);
		} else if(wParam == CLIENT_HUBNAME) {
			SetWindowText(client->getName().c_str());
			addClientLine("Connected");
		} else if(wParam == CLIENT_VALIDATEDENIED) {
			addClientLine("Your nick was already taken, please change to something else!");
			client->disconnect();
		} else if(wParam == CLIENT_PRIVATEMESSAGE) {
			dcassert(clientPrivateMessage.find((PrivateFrame*)lParam) != clientPrivateMessage.end());
			((PrivateFrame*)lParam)->addLine(clientPrivateMessage[(PrivateFrame*)lParam]);
			clientPrivateMessage.erase((PrivateFrame*)lParam);
		}
		cs.leave();
		return 0;
	};

	virtual void onClientConnecting(Client* aClient) { 
		PostMessage(WM_SPEAKER, CLIENT_CONNECTING); 
	};
	virtual void onClientError(Client* aClient, const string& aReason) {
		cs.enter();
		clientError = aReason;
		cs.leave();
		PostMessage(WM_SPEAKER, CLIENT_ERROR);
	}
	virtual void onClientGetPassword(Client* aClient) { PostMessage(WM_SPEAKER, CLIENT_GETPASSWORD); };
	virtual void onClientHubName(Client* aClient) { PostMessage(WM_SPEAKER, CLIENT_HUBNAME); };
	virtual void onClientMessage(Client* aClient, const string& aMessage) {
		cs.enter();
		clientMessages.push_back(aMessage);
		cs.leave();
		PostMessage(WM_SPEAKER, CLIENT_MESSAGE);
	}
	virtual void onClientMyInfo(Client* aClient, User::Ptr& aUser) {
		cs.enter();
		clientMyInfo.push_back(aUser);
		cs.leave();
		PostMessage(WM_SPEAKER, CLIENT_MYINFO);
	}
	virtual void onClientOpList(Client* aClient, StringList& aOps) {
		for(StringIter i = aOps.begin(); i != aOps.end(); ++i) {
			if(*i == Settings::getNick()) {
				op = true;
				return;
			}
		}
	}
	virtual void onClientPrivateMessage(Client* aClient, User::Ptr& aUser, const string& aMessage) {
		cs.enter();
		PrivateFrame* frm = PrivateFrame::getFrame(aUser, m_hWndMDIClient);
		frm->setTab(getTab());
		clientPrivateMessage[frm] = aMessage;
		cs.leave();
		PostMessage(WM_SPEAKER, CLIENT_PRIVATEMESSAGE, (LPARAM)frm);
	}
	
	virtual void onClientUnknown(Client* aClient, const string& aCommand) {
		cs.enter();
		clientError = "Unknown: " + aCommand;
		cs.leave();
		PostMessage(WM_SPEAKER, CLIENT_ERROR);
	}
	virtual void onClientQuit(Client* aClient, User::Ptr& aUser) {
		cs.enter();
		clientQuit.push_back(aUser);
		cs.leave();
		PostMessage(WM_SPEAKER, CLIENT_QUIT);
	}
	virtual void onClientValidateDenied(Client* aClient) { PostMessage(WM_SPEAKER, CLIENT_VALIDATEDENIED); };

	void updateStatusBar() {
		char buf[256];
		sprintf(buf, "%d users", client->getUserCount());
		ctrlStatus.SetText(1, buf);
		ctrlStatus.SetText(2, Util::formatBytes(client->getAvailable()).c_str());
	}

	Client::Ptr client;
	string server;
	CContainedWindow ctrlMessageContainer;
	CMenu userMenu;
	CMenu opMenu;
	bool op;

public:

	HubFrame(const string& aServer) : op(false), ctrlMessageContainer("edit", this, EDIT_MESSAGE_MAP), server(aServer), stopperThread(NULL) {
		client = ClientManager::getInstance()->getClient();
		client->addListener(this);
	}

	~HubFrame() {
		dcassert(client == NULL);
	}

	DECLARE_FRAME_WND_CLASS("HubFrame", IDR_MDICHILD);

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
		COMMAND_ID_HANDLER(ID_FILE_RECONNECT, OnFileReconnect)
		COMMAND_ID_HANDLER(IDC_GETLIST, onGetList)
		COMMAND_ID_HANDLER(IDC_PRIVATEMESSAGE, onPrivateMessage)
		COMMAND_ID_HANDLER(IDC_REFRESH, onRefresh)
		COMMAND_ID_HANDLER(IDC_KICK, onKick)
		COMMAND_ID_HANDLER(IDC_REDIRECT, onRedirect)
		NOTIFY_HANDLER(IDC_USERS, NM_DBLCLK, onDoubleClickUsers)	
		NOTIFY_HANDLER(IDC_USERS, LVN_COLUMNCLICK, onColumnClickUsers)
		CHAIN_MSG_MAP(MDITabChildWindowImpl<HubFrame>)
		CHAIN_MSG_MAP(splitBase)
	ALT_MSG_MAP(EDIT_MESSAGE_MAP)
		MESSAGE_HANDLER(WM_CHAR, OnChar)
	END_MSG_MAP()

	LRESULT onGetList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		int i=-1;
		char buf[256];
		if(client->isConnected()) {
			while( (i = ctrlUsers.GetNextItem(i, LVNI_SELECTED)) != -1) {
				ctrlUsers.GetItemText(i, 0, buf, 256);
				string user = buf;
				User::Ptr& u = client->getUser(user);
				if(u)
					DownloadManager::getInstance()->downloadList(u);
				else 
					DownloadManager::getInstance()->downloadList(user);
			}
		}
		return 0;
	}

	LRESULT onPrivateMessage(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		int i=-1;
		char buf[256];
		if(client->isConnected()) {
			while( (i = ctrlUsers.GetNextItem(i, LVNI_SELECTED)) != -1) {
				ctrlUsers.GetItemText(i, 0, buf, 256);
				string user = buf;
				User::Ptr& u = client->getUser(user);
				if(u) {
					PrivateFrame* frm = PrivateFrame::getFrame(u, m_hWndMDIClient);
					frm->setTab(getTab());
					frm->CreateEx(m_hWndMDIClient);
				}
			}
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

	LRESULT onKick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) { 
		LineDlg dlg;
		dlg.title = "Kick user(s)";
		dlg.description = "Please enter a reason";
		if(dlg.DoModal() == IDOK) {
			int i = -1;
			while( (i = ctrlUsers.GetNextItem(i, LVNI_SELECTED)) != -1) {
				char buf[256];
				ctrlUsers.GetItemText(i, 0, buf, 256);
				string user = buf;
				User::Ptr& u = client->getUser(user);
				if(u) {
					client->sendMessage(Settings::getNick() + " is kicking " + u->getNick() + " because: " + dlg.line);
					client->privateMessage(u, "You are being kicked because: " + dlg.line);
					client->kick(u);
				}
			}
		}
			
		return 0; 
	};
	LRESULT onRedirect(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) { 
		LineDlg dlg1, dlg2;
		dlg1.title = "Redirect user(s)";
		dlg1.description = "Please enter a reason";
		if(dlg1.DoModal() == IDOK) {
			dlg2.title = "Redirect user(s)";
			dlg2.description = "Please enter destination server";
			if(dlg2.DoModal() == IDOK) {
				int i = -1;
				while( (i = ctrlUsers.GetNextItem(i, LVNI_SELECTED)) != -1) {
					char buf[256];
					ctrlUsers.GetItemText(i, 0, buf, 256);
					string user = buf;
					User::Ptr& u = client->getUser(user);
					if(u) {
						client->opForceMove(u, dlg2.line, "You are being redirected to " + dlg2.line + ": " + dlg1.line);
					}
				}
			}
		}
		
		return 0; 
	};
	
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
				opMenu.TrackPopupMenuEx(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
			} else {
				userMenu.TrackPopupMenuEx(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
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

		ClientManager::getInstance()->putClient(f->client);

		f->client = NULL;
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
	
	LRESULT OnFileReconnect(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	LRESULT OnForwardMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
	{
		LPMSG pMsg = (LPMSG)lParam;
		
		return MDITabChildWindowImpl<HubFrame>::PreTranslateMessage(pMsg);
	}
	
	LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
		return 0;
	}
		
	LRESULT onDoubleClickUsers(int idCtrl, LPNMHDR pnmh, BOOL& bHandled) {
		NMITEMACTIVATE* item = (NMITEMACTIVATE*)pnmh;
		string user;
		char buf[256];

		if(client->isConnected() && item->iItem != -1) {
			ctrlUsers.GetItemText(item->iItem, 0, buf, 256);
			user = buf;
			User::Ptr& u = client->getUser(user);
			if(u)
				DownloadManager::getInstance()->downloadList(u);
			else 
				DownloadManager::getInstance()->downloadList(user);
		}
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

	LRESULT onColumnClickUsers(int idCtrl, LPNMHDR pnmh, BOOL& bHandled) {
		NMLISTVIEW* l = (NMLISTVIEW*)pnmh;
		if(l->iSubItem == ctrlUsers.getSortColumn()) {
			ctrlUsers.setSortDirection(!ctrlUsers.getSortDirection());
		} else {
			if(l->iSubItem == 1) {
				ctrlUsers.setSort(l->iSubItem, ExListViewCtrl::SORT_FUNC, true, sortSize);
			} else {
				ctrlUsers.setSort(l->iSubItem, ExListViewCtrl::SORT_STRING_NOCASE);
			}
		}
		return 0;
	}

	void addLine(const string& aLine) {
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
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHILDFRM_H__A7078724_FD85_4F39_8463_5A08A5F45E33__INCLUDED_)

/**
 * @file HubFrame.h
 * $Id: HubFrame.h,v 1.25 2002/01/05 10:13:39 arnetheduck Exp $
 * @if LOG
 * $Log: HubFrame.h,v $
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


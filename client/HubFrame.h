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
#include "ProtocolHandler.h"
#include "ExListViewCtrl.h"
#include "DownloadManager.h"

#include "AtlCmdBar2.h"

#define EDIT_MESSAGE_MAP 5		// This could be any number, really...

class HubFrame : public CMDIChildWindowImpl2<HubFrame>, public ClientListener, public CSplitterImpl<HubFrame>
{
protected:
	virtual void onClientConnecting(Client* aClient) {
		addClientLine("Connecting to " + aClient->getServer() + "...");
		SetWindowText(aClient->getServer().c_str());
	}
	virtual void onClientMessage(Client* aClient, const string& aMessage) {
		addLine(aMessage);
	}
	
	virtual void onClientUnknown(Client* aClient, const string& aCommand) {
		addClientLine("Unknown: " + aCommand);
	}
	virtual void onClientQuit(Client* aClient, const string& aNick) {
		ctrlUsers.deleteItem(aNick);
	}
	virtual void onClientHubName(Client* aClient, const string& aHubName) {
		SetWindowText(aHubName.c_str());
	}
	virtual void onClientError(Client* aClient, const string& aReason) {
		addClientLine("Connection failed: " + aReason);
	}
	virtual void onClientValidateDenied(Client* aClient) {
		addClientLine("Your nick was already taken, please change to something else!");
		client->disconnect();
	}

	string convertBytes(const string& aString) {
		char buf[64];
		__int64 x = _atoi64(aString.c_str());
		if(x < 1024) {
			sprintf(buf, "%d B", x );
		} else if(x < 1024*1024) {
			sprintf(buf, "%.02f kB", (double)x/(1024.0) );
		} else if(x < 1024*1024*1024) {
			sprintf(buf, "%.02f MB", (double)x/(1024.0*1024.0) );
		} else if(x < 1024I64*1024I64*1024I64*1024I64) {
			sprintf(buf, "%.02f GB", (double)x/(1024.0*1024.0*1024.0) );
		} else {
			sprintf(buf, "%.02f TB", (double)x/(1024.0*1024.0*1024.0*1024.0));
		}

		return buf;
	}
	virtual void onClientMyInfo(Client* aClient, User* aUser) {
		
		LV_FINDINFO fi;
		fi.flags = LVFI_STRING;
		fi.psz = aUser->getNick().c_str();
		int i = ctrlUsers.FindItem(&fi, -1);
		if(i == -1) {
			i = ctrlUsers.InsertItem(ctrlUsers.GetItemCount(), aUser->getNick().c_str());
		}
		ctrlUsers.SetItemText(i, 1, convertBytes(aUser->getBytesSharedString()).c_str());
		ctrlUsers.SetItemText(i, 2, aUser->getDescription().c_str());
		ctrlUsers.SetItemText(i, 3, aUser->getConnection().c_str());
		ctrlUsers.SetItemText(i, 4, aUser->getEmail().c_str());
	}
	virtual void onClientPrivateMessage(Client* aClient, const string& aFrom, const string& aMessage) {
		addLine("Private message from " + aFrom + ":\r\n" + aMessage);
	}

	Client::Ptr client;
	ProtocolHandler::Ptr ph;
	string server;
	CContainedWindow ctrlMessageContainer;

public:

	HubFrame(const string& aServer) : ctrlMessageContainer("edit", this, EDIT_MESSAGE_MAP), server(aServer) {
		client = new Client();
		client->addListener(this);
	}

	~HubFrame() {
		delete ph;
		client->removeListeners();
		delete client;
	}

	DECLARE_FRAME_WND_CLASS(NULL, IDR_MDICHILD)

	CEdit ctrlClient;
	CEdit ctrlMessage;
	ExListViewCtrl ctrlUsers;
	
	virtual void OnFinalMessage(HWND /*hWnd*/) {
		delete this;
	}

	typedef CSplitterImpl<HubFrame> splitBase;

	BEGIN_MSG_MAP(HubFrame)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_FORWARDMSG, OnForwardMsg)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		NOTIFY_HANDLER(IDC_USERS, NM_DBLCLK, onDoubleClickUsers)	
		NOTIFY_HANDLER(IDC_USERS, LVN_COLUMNCLICK, onColumnClickUsers)
		CHAIN_MSG_MAP(CMDIChildWindowImpl2<HubFrame>)
		CHAIN_MSG_MAP(splitBase)
	ALT_MSG_MAP(EDIT_MESSAGE_MAP)
		MESSAGE_HANDLER(WM_CHAR, OnChar)
	END_MSG_MAP()

	LRESULT OnForwardMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
		return 0;
	};
	
	LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
		return 0;
	}
		
	LRESULT onDoubleClickUsers(int idCtrl, LPNMHDR pnmh, BOOL& bHandled) {
		NMITEMACTIVATE* item = (NMITEMACTIVATE*)pnmh;
		string user;
		char buf[1024];
		if(item->iItem != -1) {
			ctrlUsers.GetItemText(item->iItem, 0, buf, 1024);
			DownloadManager::getInstance()->download("MyList.DcLst", "", client->getUser(buf), Settings::getAppPath() + user + ".DcLst");
		}
		return 0;
	}
		
	LRESULT onColumnClickUsers(int idCtrl, LPNMHDR pnmh, BOOL& bHandled) {
		NMLISTVIEW* l = (NMLISTVIEW*)pnmh;
		if(l->iSubItem == ctrlUsers.getSortColumn()) {
			ctrlUsers.setSortDirection(!ctrlUsers.getSortDirection());
		} else {
			if(l->iSubItem == 2) {
				ctrlUsers.setSort(l->iSubItem, ExListViewCtrl::SORT_INT);
			} else {
				ctrlUsers.setSort(l->iSubItem, ExListViewCtrl::SORT_STRING);
			}
		}
		return 0;
	}

	void addLine(const string& aLine) {
		ctrlClient.AppendText(aLine.c_str());
		ctrlClient.AppendText("\r\n");
	}

	void addClientLine(const string& aLine) {
		ctrlClient.AppendText("<Client> ");
		addLine(aLine);
	}

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
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
 * $Id: HubFrame.h,v 1.6 2001/11/29 19:10:55 arnetheduck Exp $
 * @if LOG
 * $Log: HubFrame.h,v $
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


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
#include "DCClient.h"
#include "ProtocolHandler.h"
#include "ExListViewCtrl.h"

#define EDIT_MESSAGE_MAP 5		// This could be any number, really...

class HubFrame : public CMDIChildWindowImpl<HubFrame>, public ClientListener
{
protected:
	virtual void onConnecting(const string& aServer) {
		addClientLine("Connecting to " + aServer + "...");
		SetWindowText(aServer.c_str());
	}
	virtual void onMessage(const string& aMessage) {
		addLine(aMessage);
	}
	
	virtual void onUnknown(const string& aCommand) {
		addClientLine("Unknown: " + aCommand);
	}
	virtual void onQuit(const string& aNick) {
		ctrlUsers.deleteItem(aNick);
	}
	virtual void onHubName(const string& aHubName) {
		SetWindowText(aHubName.c_str());
	}
	virtual void onConnectionFailed(const string& aReason) {
		addClientLine("Connection failed: " + aReason);
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
	virtual void onMyInfo(const string& aNick, const string& aDescription, const string& aSpeed, const string& aEmail,
		const string& aBytesShared) {
		
		LV_FINDINFO fi;
		fi.flags = LVFI_STRING;
		fi.psz = aNick.c_str();
		int i = ctrlUsers.FindItem(&fi, -1);
		if(i == -1) {
			i = ctrlUsers.InsertItem(ctrlUsers.GetItemCount(), aNick.c_str());
		}
		ctrlUsers.SetItemText(i, 1, convertBytes(aBytesShared).c_str());
		ctrlUsers.SetItemText(i, 2, aDescription.c_str());
		ctrlUsers.SetItemText(i, 3, aSpeed.c_str());
		ctrlUsers.SetItemText(i, 4, aEmail.c_str());
	}
	virtual void onNickList(StringList& aList) {
		for(StringIter i = aList.begin(); i != aList.end(); ++i) {
			ctrlUsers.InsertItem(ctrlUsers.GetItemCount(), i->c_str());
		}
	}
	virtual void onOpList(StringList& aList) {
		for(StringIter i = aList.begin(); i != aList.end(); ++i) {
			ctrlUsers.InsertItem(ctrlUsers.GetItemCount(), i->c_str());
		}
	}
	virtual void onPrivateMessage(const string& aFrom, const string& aMessage) {
		addLine("Private message from " + aFrom + ":\r\n" + aMessage);
	}
	

	DCClient::Ptr client;
	ProtocolHandler::Ptr ph;
	string server;
	CContainedWindow ctrlMessageContainer;

public:

	HubFrame(const string& aServer) : ctrlMessageContainer("edit", this, EDIT_MESSAGE_MAP), server(aServer) {
		client = new DCClient();
		client->addListener(this);
		ph = new ProtocolHandler(client);
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

	BEGIN_MSG_MAP(HubFrame)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		NOTIFY_HANDLER(IDC_USERS, LVN_COLUMNCLICK, onColumnClickUsers)
	ALT_MSG_MAP(EDIT_MESSAGE_MAP)
		MESSAGE_HANDLER(WM_CHAR, OnChar)
	END_MSG_MAP()

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
 * $Id: HubFrame.h,v 1.3 2001/11/24 10:37:09 arnetheduck Exp $
 * @if LOG
 * $Log: HubFrame.h,v $
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


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

#if !defined(AFX_PUBLICHUBSFRM_H__F6D75CA8_F229_4E7D_8ADC_0B1F3B0083C4__INCLUDED_)
#define AFX_PUBLICHUBSFRM_H__F6D75CA8_F229_4E7D_8ADC_0B1F3B0083C4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FlatTabCtrl.h"
#include "HubManager.h"
#include "ExListViewCtrl.h"

#define SERVER_MESSAGE_MAP 7

class PublicHubsFrame : public MDITabChildWindowImpl<PublicHubsFrame>, private HubManagerListener
{
public:
	PublicHubsFrame() : close(false), listing(false), ctrlHubContainer("edit", this, SERVER_MESSAGE_MAP) {
		
	};

	virtual ~PublicHubsFrame() {
	};

	DECLARE_FRAME_WND_CLASS("PublicHubsFrame", IDR_MDICHILD);
		
	virtual void OnFinalMessage(HWND /*hWnd*/) {
		frame = NULL;
		delete this;
	}

	BEGIN_MSG_MAP(PublicHubsFrame)
		MESSAGE_HANDLER(WM_CREATE, onCreate)
		MESSAGE_HANDLER(WM_FORWARDMSG, OnForwardMsg)
		MESSAGE_HANDLER(WM_CLOSE, onClose)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		COMMAND_HANDLER(IDC_REFRESH, BN_CLICKED, onClickedRefresh)
		COMMAND_HANDLER(IDC_CONNECT, BN_CLICKED, onClickedConnect)
		NOTIFY_HANDLER(IDC_HUBLIST, LVN_COLUMNCLICK, onColumnClickHublist)
		NOTIFY_HANDLER(IDC_HUBLIST, NM_DBLCLK, onDoubleClickHublist)
		CHAIN_MSG_MAP(MDITabChildWindowImpl<PublicHubsFrame>)
	ALT_MSG_MAP(SERVER_MESSAGE_MAP)
		MESSAGE_HANDLER(WM_CHAR, OnChar)
	END_MSG_MAP()
		
	LRESULT OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(&ps);
		FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_ACTIVEBORDER+1));
		EndPaint(&ps);
		return 0;
	}
	
	LRESULT OnForwardMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
	{
		LPMSG pMsg = (LPMSG)lParam;
		
		return CMDIChildWindowImpl2<PublicHubsFrame>::PreTranslateMessage(pMsg);
	}
	
	LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
		return 0;
	}
	LRESULT onCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onDoubleClickHublist(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	
	LRESULT onColumnClickHublist(int idCtrl, LPNMHDR pnmh, BOOL& bHandled) {
		NMLISTVIEW* l = (NMLISTVIEW*)pnmh;
		if(l->iSubItem == ctrlHubs.getSortColumn()) {
			ctrlHubs.setSortDirection(!ctrlHubs.getSortDirection());
		} else {
			if(l->iSubItem == 2) {
				ctrlHubs.setSort(l->iSubItem, ExListViewCtrl::SORT_INT);
			} else {
				ctrlHubs.setSort(l->iSubItem, ExListViewCtrl::SORT_STRING_NOCASE);
			}
		}
		return 0;
	}

	LRESULT onClickedRefresh(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT onClickedConnect(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

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
		ctrlHubs.MoveWindow(rc);
		
		rc = rect;
		rc.bottom -= 2;
		rc.top = rc.bottom - 22;

		rc.left += 140;
		rc.right -= 100;
		ctrlHub.MoveWindow(rc);

		rc.left -= 43;
		rc.right = rc.left + 43;
		rc.top += 3;
		ctrlAddress.MoveWindow(rc);

		rc = rect;
		rc.bottom -= 2;
		rc.top = rc.bottom - 22;

		rc.left += 2;
		rc.right = rc.left + 80;
		ctrlRefresh.MoveWindow(rc);
		
		rc = rect;
		rc.bottom -= 2;
		rc.top = rc.bottom - 22;

		rc.left = rc.right - 96;
		rc.right -= 2;
		ctrlConnect.MoveWindow(rc);
	}
	
	static PublicHubsFrame* frame;
	
private:
	int hubs;
	int users;
	CStatusBarCtrl ctrlStatus;
	CButton ctrlConnect;
	CButton ctrlRefresh;
	CStatic ctrlAddress;

	CContainedWindow ctrlHubContainer;
	
	CEdit ctrlHub;
	ExListViewCtrl ctrlHubs;
	bool listing;
	bool close;
	
	virtual void onHubMessage(const string& aMessage) {
		ctrlStatus.SetText(0, aMessage.c_str());
	}
	
	virtual void onHub(const string& aName, const string& aServer, const string& aDescription, const string& aUsers);
	virtual void onHubFinished() {
		HubManager::getInstance()->removeListener(this);
		listing = false;
		if(close)
			PostMessage(WM_CLOSE);
	}

	virtual void onHubStarting() {
		ctrlHubs.DeleteAllItems();
		hubs = users = 0;
		updateStatus();
	}
	
	void updateStatus() {
		char buf[1024];
		sprintf(buf, "Users: %d", users);
		ctrlStatus.SetText(2, buf);
		sprintf(buf, "Hubs: %d", hubs);
		ctrlStatus.SetText(1, buf);
	}
};

#endif // !defined(AFX_PUBLICHUBSFRM_H__F6D75CA8_F229_4E7D_8ADC_0B1F3B0083C4__INCLUDED_)

/**
 * @file PublicHubsFrm.h
 * $Id: PublicHubsFrm.h,v 1.4 2001/12/27 12:05:00 arnetheduck Exp $
 * @if LOG
 * $Log: PublicHubsFrm.h,v $
 * Revision 1.4  2001/12/27 12:05:00  arnetheduck
 * Added flat tabs, fixed sorting and a StringTokenizer bug
 *
 * Revision 1.3  2001/12/13 19:21:57  arnetheduck
 * A lot of work done almost everywhere, mainly towards a friendlier UI
 * and less bugs...time to release 0.06...
 *
 * Revision 1.2  2001/12/12 00:06:04  arnetheduck
 * Updated the public hub listings, fixed some minor transfer bugs, reworked the
 * sockets to use only one thread (instead of an extra thread for sending files),
 * and fixed a major bug in the client command decoding (still have to fix this
 * one for the userconnections...)
 *
 * Revision 1.1  2001/12/11 21:17:29  arnetheduck
 * Changed the dialog to a frame
 *
 * @endif
 */


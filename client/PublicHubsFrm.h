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

	enum {
		COLUMN_NAME,
		COLUMN_DESCRIPTION,
		COLUMN_USERS,
		COLUMN_SERVER
	};
	PublicHubsFrame() : stopperThread(NULL), users(0), hubs(0), ctrlHubContainer("edit", this, SERVER_MESSAGE_MAP) {
		
	};

	virtual ~PublicHubsFrame() {
	};

	DECLARE_FRAME_WND_CLASS("PublicHubsFrame", IDR_PUBLICHUBS);
		
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
		MESSAGE_HANDLER(WM_CONTEXTMENU, onContextMenu)
		MESSAGE_HANDLER(WM_SPEAKER, onSpeaker)
		MESSAGE_HANDLER(WM_CTLCOLOREDIT, onCtlColor)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, onCtlColor)
		COMMAND_HANDLER(IDC_ADD, BN_CLICKED, onAdd)
		COMMAND_HANDLER(IDC_REFRESH, BN_CLICKED, onClickedRefresh)
		COMMAND_HANDLER(IDC_CONNECT, BN_CLICKED, onClickedConnect)
		NOTIFY_HANDLER(IDC_HUBLIST, LVN_COLUMNCLICK, onColumnClickHublist)
		NOTIFY_HANDLER(IDC_HUBLIST, NM_DBLCLK, onDoubleClickHublist)
		CHAIN_MSG_MAP(MDITabChildWindowImpl<PublicHubsFrame>)
	ALT_MSG_MAP(SERVER_MESSAGE_MAP)
		MESSAGE_HANDLER(WM_CHAR, OnChar)
	END_MSG_MAP()
		
	LRESULT onCtlColor(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		HWND hWnd = (HWND)lParam;
		HDC hDC = (HDC)wParam;
		if(hWnd == ctrlHub.m_hWnd) {
			::SetBkColor(hDC, Util::bgColor);
			::SetTextColor(hDC, Util::textColor);
			return (LRESULT)Util::bgBrush;
		}
		bHandled = FALSE;
		return FALSE;
	};
	
	bool checkNick() {
		if(SETTING(NICK).empty()) {
			MessageBox("Please enter a nickname in the settings dialog!");
			return false;
		}
		return true;
	}
	LRESULT OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT onSpeaker(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		HubManager::getInstance()->getPublicHubs();
		return 0;
	}
		
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(&ps);
		FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_ACTIVEBORDER+1));
		EndPaint(&ps);
		return 0;
	}

	LRESULT onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled) {
		RECT rc;                    // client area of window 
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };        // location of mouse click 
		
		// Get the bounding rectangle of the client area. 
		ctrlHubs.GetClientRect(&rc);
		ctrlHubs.ScreenToClient(&pt); 
		
		if (PtInRect(&rc, pt)) 
		{ 
			ctrlHubs.ClientToScreen(&pt);
			hubsMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
			
			return TRUE; 
		}
		
		return FALSE; 
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

	LRESULT onClose(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled) {
		DWORD id;
		if(stopperThread) {
			if(WaitForSingleObject(stopperThread, 0) == WAIT_TIMEOUT) {
				// Hm, the thread's not finished stopping the client yet...post a close message and continue processing...
				PostMessage(WM_CLOSE);
				return 0;
			}
			CloseHandle(stopperThread);
			stopperThread = NULL;
			bHandled = FALSE;
		} else {
			stopperThread = CreateThread(NULL, 0, stopper, this, 0, &id);
		}
		return 0;
	}

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
	LRESULT onAdd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
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
	CMenu hubsMenu;
	
	CContainedWindow ctrlHubContainer;
	
	CEdit ctrlHub;
	ExListViewCtrl ctrlHubs;
	HANDLE stopperThread;
	
	static DWORD WINAPI stopper(void* p) {
		PublicHubsFrame* frm = (PublicHubsFrame*)p;
		HubManager::getInstance()->removeListener(frm);
		HubManager::getInstance()->reset();
		frm->PostMessage(WM_CLOSE);	
		return 0;
	}
	
	// HubManagerListener
	virtual void onAction(HubManagerListener::Types type) {
		switch(type) {
		case HubManagerListener::FINISHED:
			PostMessage(WM_SPEAKER);
		}
	}
	virtual void onAction(HubManagerListener::Types type, const HubEntry::List& aList) {
		switch(type) {
		case HubManagerListener::GET_PUBLIC_HUBS:
			onHubFinished(aList); break;
		}
	}

	virtual void onAction(HubManagerListener::Types type, const string& line) {
		switch(type) {
		case HubManagerListener::MESSAGE:
			ctrlStatus.SetText(0, line.c_str());
		}
	}
	
	void onHubFinished(const HubEntry::List& aList) {
		HubManager::getInstance()->removeListener(this);
		ctrlStatus.SetText(0, "Done!");
		ctrlHubs.DeleteAllItems();
		users = 0;
		hubs = 0;
		
		ctrlHubs.SetRedraw(FALSE);

		for(HubEntry::List::const_iterator i = aList.begin(); i != aList.end(); ++i) {
			StringList l;
			l.push_back(i->getName());
			l.push_back(i->getDescription());
			l.push_back(i->getUsers());
			l.push_back(i->getServer());
			ctrlHubs.insert(l);
			hubs++;
			users += Util::toInt(i->getUsers());
		}

		ctrlHubs.SetRedraw(TRUE);
		updateStatus();
		ctrlHubs.Invalidate();
	}

	void updateStatus() {
		char buf[128];
		sprintf(buf, "Users: %d", users);
		ctrlStatus.SetText(2, buf);
		sprintf(buf, "Hubs: %d", hubs);
		ctrlStatus.SetText(1, buf);
	}
};

#endif // !defined(AFX_PUBLICHUBSFRM_H__F6D75CA8_F229_4E7D_8ADC_0B1F3B0083C4__INCLUDED_)

/**
 * @file PublicHubsFrm.h
 * $Id: PublicHubsFrm.h,v 1.15 2002/01/26 21:09:51 arnetheduck Exp $
 * @if LOG
 * $Log: PublicHubsFrm.h,v $
 * Revision 1.15  2002/01/26 21:09:51  arnetheduck
 * Release 0.14
 *
 * Revision 1.14  2002/01/20 22:54:46  arnetheduck
 * Bugfixes to 0.131 mainly...
 *
 * Revision 1.13  2002/01/13 22:50:48  arnetheduck
 * Time for 0.12, added favorites, a bunch of new icons and lot's of other stuff
 *
 * Revision 1.12  2002/01/11 14:52:57  arnetheduck
 * Huge changes in the listener code, replaced most of it with templates,
 * also moved the getinstance stuff for the managers to a template
 *
 * Revision 1.11  2002/01/10 12:33:14  arnetheduck
 * Various fixes
 *
 * Revision 1.10  2002/01/07 20:17:59  arnetheduck
 * Finally fixed the reconnect bug that's been annoying me for a whole day...
 * Hopefully the app works better in w95 now too...
 *
 * Revision 1.9  2002/01/06 21:55:20  arnetheduck
 * Some minor bugs fixed, but there remains one strange thing, the reconnect
 * button doesn't work...
 *
 * Revision 1.8  2002/01/05 19:06:09  arnetheduck
 * Added user list images, fixed bugs and made things more effective
 *
 * Revision 1.6  2002/01/02 16:12:33  arnetheduck
 * Added code for multiple download sources
 *
 * Revision 1.5  2001/12/29 13:47:14  arnetheduck
 * Fixing bugs and UI work
 *
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


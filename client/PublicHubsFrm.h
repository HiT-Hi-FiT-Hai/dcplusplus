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
#define FILTER_MESSAGE_MAP 8
class PublicHubsFrame : public MDITabChildWindowImpl<PublicHubsFrame>, private HubManagerListener
{
public:

	PublicHubsFrame() : users(0), hubs(0), ctrlHubContainer("edit", this, SERVER_MESSAGE_MAP), 
		filterContainer("edit", this, FILTER_MESSAGE_MAP) {
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
		MESSAGE_HANDLER(WM_CHAR, onChar)
	ALT_MSG_MAP(FILTER_MESSAGE_MAP)
		MESSAGE_HANDLER(WM_CHAR, onFilterChar)
	END_MSG_MAP()
		
	LRESULT onChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT onFilterChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT onCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onDoubleClickHublist(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT onAdd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT onClickedRefresh(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT onClickedConnect(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT onSpeaker(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	
	void UpdateLayout(BOOL bResizeBars = TRUE);
	bool checkNick();
	
	LRESULT onCtlColor(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		HWND hWnd = (HWND)lParam;
		HDC hDC = (HDC)wParam;
		if(hWnd == ctrlHub.m_hWnd || hWnd == ctrlFilter.m_hWnd) {
			::SetBkColor(hDC, Util::bgColor);
			::SetTextColor(hDC, Util::textColor);
			return (LRESULT)Util::bgBrush;
		}
		bHandled = FALSE;
		return FALSE;
	};
	
	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(&ps);
		FillRect(hdc, &ps.rcPaint, ::GetSysColorBrush(COLOR_BTNFACE));
		EndPaint(&ps);
		return 0;
	}

	LRESULT onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
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
	
	LRESULT OnForwardMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
		LPMSG pMsg = (LPMSG)lParam;
		return CMDIChildWindowImpl2<PublicHubsFrame>::PreTranslateMessage(pMsg);
	}
	
	LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		return 0;
	}

	LRESULT onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
		HubManager::getInstance()->removeListener(this);
		bHandled = FALSE;
		return 0;
	}
	
	LRESULT onColumnClickHublist(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
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
	
	static PublicHubsFrame* frame;
	
private:
	enum {
		COLUMN_FIRST,
		COLUMN_NAME = COLUMN_FIRST,
		COLUMN_DESCRIPTION,
		COLUMN_USERS,
		COLUMN_SERVER,
		COLUMN_LAST
	};

	enum {
		FINISHED,
		STARTING,
		FAILED
	};

	int visibleHubs;
	int users;
	CStatusBarCtrl ctrlStatus;
	CButton ctrlConnect;
	CButton ctrlRefresh;
	CButton ctrlAddress;
	CButton ctrlFilterDesc;
	CEdit ctrlFilter;
	CMenu hubsMenu;
	
	CContainedWindow ctrlHubContainer;
	CContainedWindow filterContainer;	
	CEdit ctrlHub;
	ExListViewCtrl ctrlHubs;

	HubEntry::List hubs;
	string filter;
	
	static int columnIndexes[];
	static int columnSizes[];
	
	// HubManagerListener
	virtual void onAction(HubManagerListener::Types type) {
		switch(type) {
		case HubManagerListener::DOWNLOAD_FINISHED:
			PostMessage(WM_SPEAKER, FINISHED); break;
		}
	}

	virtual void onAction(HubManagerListener::Types type, const string& line) {
		string* x = new string(line);
		switch(type) {
		case HubManagerListener::DOWNLOAD_STARTING:
			PostMessage(WM_SPEAKER, STARTING, (LPARAM)x); break;
		case HubManagerListener::DOWNLOAD_FAILED:
			PostMessage(WM_SPEAKER, FINISHED, (LPARAM)x); break;
		}
	}
	
	void updateStatus();
	void updateList();
};

#endif // !defined(AFX_PUBLICHUBSFRM_H__F6D75CA8_F229_4E7D_8ADC_0B1F3B0083C4__INCLUDED_)

/**
 * @file PublicHubsFrm.h
 * $Id: PublicHubsFrm.h,v 1.18 2002/03/25 22:23:25 arnetheduck Exp $
 * @if LOG
 * $Log: PublicHubsFrm.h,v $
 * Revision 1.18  2002/03/25 22:23:25  arnetheduck
 * Lots of minor updates
 *
 * Revision 1.17  2002/03/23 01:58:42  arnetheduck
 * Work done on favorites...
 *
 * Revision 1.16  2002/02/09 18:13:51  arnetheduck
 * Fixed level 4 warnings and started using new stl
 *
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


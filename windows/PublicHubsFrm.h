/* 
 * Copyright (C) 2001-2003 Jacek Sieka, j_s@telia.com
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
#include "ExListViewCtrl.h"

#include "../client/HubManager.h"

#include "WinUtil.h"

#define SERVER_MESSAGE_MAP 7
#define FILTER_MESSAGE_MAP 8
class PublicHubsFrame : public MDITabChildWindowImpl<PublicHubsFrame>, private HubManagerListener
{
public:

	static PublicHubsFrame* frame;

	PublicHubsFrame() : users(0), hubs(0), ctrlHubContainer("edit", this, SERVER_MESSAGE_MAP), 
		filterContainer("edit", this, FILTER_MESSAGE_MAP) {
	};

	virtual ~PublicHubsFrame() {
	};

	DECLARE_FRAME_WND_CLASS_EX("PublicHubsFrame", IDR_PUBLICHUBS, 0, COLOR_3DFACE);
		
	virtual void OnFinalMessage(HWND /*hWnd*/) {
		frame = NULL;
		delete this;
	}
	
	typedef MDITabChildWindowImpl<PublicHubsFrame> baseClass;
	BEGIN_MSG_MAP(PublicHubsFrame)
		MESSAGE_HANDLER(WM_CREATE, onCreate)
		MESSAGE_HANDLER(WM_FORWARDMSG, OnForwardMsg)
		MESSAGE_HANDLER(WM_CLOSE, onClose)
		MESSAGE_HANDLER(WM_CONTEXTMENU, onContextMenu)
		MESSAGE_HANDLER(WM_SPEAKER, onSpeaker)
		MESSAGE_HANDLER(WM_CTLCOLOREDIT, onCtlColor)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, onCtlColor)
		MESSAGE_HANDLER(WM_SETFOCUS, onSetFocus)
		COMMAND_HANDLER(IDC_ADD, BN_CLICKED, onAdd)
		COMMAND_HANDLER(IDC_REFRESH, BN_CLICKED, onClickedRefresh)
		COMMAND_HANDLER(IDC_CONNECT, BN_CLICKED, onClickedConnect)
		NOTIFY_HANDLER(IDC_HUBLIST, LVN_COLUMNCLICK, onColumnClickHublist)
		NOTIFY_HANDLER(IDC_HUBLIST, NM_RETURN, onEnter)
		CHAIN_MSG_MAP(baseClass)
	ALT_MSG_MAP(SERVER_MESSAGE_MAP)
		MESSAGE_HANDLER(WM_CHAR, onChar)
	ALT_MSG_MAP(FILTER_MESSAGE_MAP)
		MESSAGE_HANDLER(WM_CHAR, onFilterChar)
	END_MSG_MAP()
		
	LRESULT onChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT onFilterChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT onCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onDoubleClickHublist(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT onEnter(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT onAdd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT onClickedRefresh(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT onClickedConnect(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT onSpeaker(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);	
	
	void UpdateLayout(BOOL bResizeBars = TRUE);
	bool checkNick();
	
	LRESULT onCtlColor(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		HWND hWnd = (HWND)lParam;
		HDC hDC = (HDC)wParam;
		if(hWnd == ctrlHub.m_hWnd || hWnd == ctrlFilter.m_hWnd) {
			::SetBkColor(hDC, WinUtil::bgColor);
			::SetTextColor(hDC, WinUtil::textColor);
			return (LRESULT)WinUtil::bgBrush;
		}
		bHandled = FALSE;
		return FALSE;
	};
	
	LRESULT OnForwardMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
		LPMSG pMsg = (LPMSG)lParam;
		return baseClass::PreTranslateMessage(pMsg);
	}

	LRESULT onSetFocus(UINT /* uMsg */, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		ctrlHubs.SetFocus();
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
	
	virtual void onAction(HubManagerListener::Types type, const string& line) throw() {
		string* x = new string(line);
		switch(type) {
		case HubManagerListener::DOWNLOAD_STARTING:
			PostMessage(WM_SPEAKER, STARTING, (LPARAM)x); break;
		case HubManagerListener::DOWNLOAD_FINISHED:
			PostMessage(WM_SPEAKER, FINISHED, (LPARAM)x); break;
		case HubManagerListener::DOWNLOAD_FAILED:
			PostMessage(WM_SPEAKER, FAILED, (LPARAM)x); break;
		}
	}
	
	void updateStatus();
	void updateList();
};

#endif // !defined(AFX_PUBLICHUBSFRM_H__F6D75CA8_F229_4E7D_8ADC_0B1F3B0083C4__INCLUDED_)

/**
 * @file
 * $Id: PublicHubsFrm.h,v 1.8 2003/04/15 10:14:03 arnetheduck Exp $
 */

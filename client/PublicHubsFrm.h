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

#include "AtlCmdBar2.h"
#include "HubManager.h"
#include "ExListViewCtrl.h"

class PublicHubsFrame : public CMDIChildWindowImpl2<PublicHubsFrame>, private HubManagerListener
{
public:
	PublicHubsFrame() : close(false), listing(false) {
		
	};

	virtual ~PublicHubsFrame() {
		HubManager::getInstance()->removeListener(this);
	};

	DECLARE_FRAME_WND_CLASS("PublicHubsFrame", IDR_MDICHILD);
		
	virtual void OnFinalMessage(HWND /*hWnd*/) {
		delete this;
	}

	BEGIN_MSG_MAP(PublicHubsDlg)
		MESSAGE_HANDLER(WM_CREATE, onCreate)
		MESSAGE_HANDLER(WM_CLOSE, onClose)
		COMMAND_HANDLER(IDC_REFRESH, BN_CLICKED, onClickedRefresh)
		COMMAND_HANDLER(IDC_CONNECT, BN_CLICKED, onClickedConnect)
		NOTIFY_HANDLER(IDC_HUBLIST, LVN_COLUMNCLICK, onColumnClickHublist)
		NOTIFY_HANDLER(IDC_HUBLIST, NM_DBLCLK, onDoubleClickHublist)
		CHAIN_MSG_MAP(CMDIChildWindowImpl2<PublicHubsFrame>)
	END_MSG_MAP()
		
	
	LRESULT onCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnItemchangedHublist(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT onDoubleClickHublist(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	
	LRESULT onColumnClickHublist(int idCtrl, LPNMHDR pnmh, BOOL& bHandled) {
		NMLISTVIEW* l = (NMLISTVIEW*)pnmh;
		if(l->iSubItem == ctrlHubs.getSortColumn()) {
			ctrlHubs.setSortDirection(!ctrlHubs.getSortDirection());
		} else {
			if(l->iSubItem == 2) {
				ctrlHubs.setSort(l->iSubItem, ExListViewCtrl::SORT_INT);
			} else {
				ctrlHubs.setSort(l->iSubItem, ExListViewCtrl::SORT_STRING);
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
		rc.left +=2;
		rc.right -= 105;
		ctrlHub.MoveWindow(rc);
		
		rc.left = rc.right + 5;
		rc.right += 103;
		ctrlConnect.MoveWindow(rc);
	}
	
	
private:
	CStatusBarCtrl ctrlStatus;
	CButton ctrlConnect;
	CEdit ctrlHub;
	ExListViewCtrl ctrlHubs;
	bool listing;
	bool close;

	virtual void onHubMessage(const string& aMessage) {
		SetWindowText(("Public Hub List - " + aMessage).c_str());
	}
	
	virtual void onHub(const string& aName, const string& aServer, const string& aDescription, const string& aUsers);
	virtual void onHubFinished() {
		listing = false;
		if(close)
			DestroyWindow();
	}

	virtual void onHubStarting() {
		ctrlHubs.DeleteAllItems();
	}
	
};

#endif // !defined(AFX_PUBLICHUBSFRM_H__F6D75CA8_F229_4E7D_8ADC_0B1F3B0083C4__INCLUDED_)

/**
 * @file PublicHubsFrm.h
 * $Id: PublicHubsFrm.h,v 1.1 2001/12/11 21:17:29 arnetheduck Exp $
 * @if LOG
 * $Log: PublicHubsFrm.h,v $
 * Revision 1.1  2001/12/11 21:17:29  arnetheduck
 * Changed the dialog to a frame
 *
 * @endif
 */


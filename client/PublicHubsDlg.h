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

#if !defined(AFX_PUBLICHUBSDLG_H__B70BEF34_84E7_4DD9_A2CE_785F29A00C27__INCLUDED_)
#define AFX_PUBLICHUBSDLG_H__B70BEF34_84E7_4DD9_A2CE_785F29A00C27__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "HubManager.h"
#include "ExListViewCtrl.h"

class PublicHubsDlg : public CDialogImpl<PublicHubsDlg>, HubManagerListener
{
private:

	ExListViewCtrl ctrlHubs;
	bool listing;
	int wId;
	
public:
	PublicHubsDlg() : listing(false), wId(-1) { };
	~PublicHubsDlg() {
		HubManager::getInstance()->removeListener(this);
	}
	string server;

	enum { IDD = IDD_HUBS };
	
	BEGIN_MSG_MAP(PublicHubsDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		NOTIFY_HANDLER(IDC_HUBLIST, LVN_ITEMCHANGED, OnItemchangedHublist)
		COMMAND_HANDLER(IDC_REFRESH, BN_CLICKED, OnClickedRefresh)
		NOTIFY_HANDLER(IDC_HUBLIST, LVN_COLUMNCLICK, onColumnClickHublist)
	END_MSG_MAP()
	
	virtual void onMessage(const string& aMessage) {
		SetWindowText(("Public Hub List - " + aMessage).c_str());
	}

	virtual void onHub(const string& aName, const string& aServer, const string& aDescription, const string& aUsers);
	virtual void onFinished() {
		HubManager::getInstance()->removeListener(this);
		listing = false;
	}
	
	static DWORD WINAPI stopper(void* p);
	
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnItemchangedHublist(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
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
	LRESULT OnClickedRefresh(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
};

#endif // !defined(AFX_PUBLICHUBSDLG_H__B70BEF34_84E7_4DD9_A2CE_785F29A00C27__INCLUDED_)

/**
 * @file PublicHubsDlg.h
 * $Id: PublicHubsDlg.h,v 1.4 2001/11/26 23:40:36 arnetheduck Exp $
 * @if LOG
 * $Log: PublicHubsDlg.h,v $
 * Revision 1.4  2001/11/26 23:40:36  arnetheduck
 * Downloads!! Now downloads are possible, although the implementation is
 * likely to change in the future...more UI work (splitters...) and some bug
 * fixes. Only user file listings are downloadable, but at least it's something...
 *
 * Revision 1.3  2001/11/25 22:06:25  arnetheduck
 * Finally downloading is working! There are now a few quirks and bugs to be fixed
 * but what the heck....!
 *
 * Revision 1.2  2001/11/24 10:31:45  arnetheduck
 * Added hub list sorting and fixed deadlock bugs (hopefully...).
 *
 * Revision 1.1.1.1  2001/11/21 17:33:20  arnetheduck
 * Inital release
 *
 * @endif
 */




















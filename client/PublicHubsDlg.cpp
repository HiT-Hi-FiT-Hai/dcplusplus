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

#include "stdafx.h"
#include "DCPlusPlus.h"

#include "PublicHubsDlg.h"

DWORD WINAPI PublicHubsDlg::FillHubList(void* p) {
	ATLASSERT(p);
	PublicHubsDlg* dlg = (PublicHubsDlg*)p;
	HANDLE hEvent = dlg->stopEvent;
	dlg->SetWindowText("Public Hub List - Downloading public server list...");

	HubManager::HubEntry::List hubs = HubManager::getPublicHubs(dlg->refresh);
	
	if(WaitForSingleObject(hEvent, 0)!=WAIT_TIMEOUT) {
		ATLTRACE("Hublist Thread ended");
		return 0;
	}
	
	EnterCriticalSection(&dlg->stopCS);
	if(hubs.size() == 0) {
		dlg->SetWindowText("Public Hub List - Unable to download public server list. Check your internet connection!");
	} else {
		dlg->SetWindowText("Public Hub List - Download complete");
	}

	for(HubManager::HubEntry::Iter i = hubs.begin(); i != hubs.end(); ++i) {
		int j = dlg->ctrlHubs.AddItem(dlg->ctrlHubs.GetItemCount(), 0, i->name.c_str());
		dlg->ctrlHubs.SetItemText(j, 1, i->description.c_str());
		dlg->ctrlHubs.SetItemText(j, 2, i->users.c_str());
		dlg->ctrlHubs.SetItemText(j, 3, i->server.c_str());
	}
	LeaveCriticalSection(&dlg->stopCS);
	return 0;
}

LRESULT PublicHubsDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	ctrlHubs.Attach(GetDlgItem(IDC_HUBLIST));
	ctrlHubs.InsertColumn(0, _T("Name"), LVCFMT_LEFT, 200, 0);
	ctrlHubs.InsertColumn(1, _T("Description"), LVCFMT_LEFT, 290, 1);
	ctrlHubs.InsertColumn(2, _T("Users"), LVCFMT_RIGHT, 50, 2);
	ctrlHubs.InsertColumn(3, _T("Server"), LVCFMT_LEFT, 100, 3);
	
	SetDlgItemText(IDC_SERVER, server.c_str());

	startReader();
	
	CenterWindow(GetParent());
	return TRUE;
}

LRESULT PublicHubsDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(wID == IDOK) {
		char buf[1024];
		GetDlgItemText(IDC_SERVER, buf, 1024);
		server = buf;
	}
	stopReader();
	EndDialog(wID);
	return 0;
}

LRESULT PublicHubsDlg::OnItemchangedHublist(int idCtrl, LPNMHDR pnmh, BOOL& bHandled) {
	NM_LISTVIEW* lv = (NM_LISTVIEW*) pnmh;
	
	if(lv->uNewState & LVIS_FOCUSED) {
		char buf[1024];
		ctrlHubs.GetItemText(lv->iItem, 3, buf, 1024);
		SetDlgItemText(IDC_SERVER, buf);
	}
	return 0;
}

LRESULT PublicHubsDlg::OnClickedRefresh(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	SetWindowText("Waiting for previous refresh to finish...");
	stopReader();
	refresh = true;
	startReader();
	return 0;
}

/**
 * @file PublicHubsDlg.cpp
 * $Id: PublicHubsDlg.cpp,v 1.1 2001/11/21 17:33:20 arnetheduck Exp $
 * @if LOG
 * $Log: PublicHubsDlg.cpp,v $
 * Revision 1.1  2001/11/21 17:33:20  arnetheduck
 * Initial revision
 *
 * @endif
 */


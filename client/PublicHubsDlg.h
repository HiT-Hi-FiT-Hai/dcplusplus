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

class PublicHubsDlg : public CDialogImpl<PublicHubsDlg>  
{
private:

	static DWORD WINAPI FillHubList(void* p);
	HANDLE readerThread;
	HANDLE stopEvent;
	CRITICAL_SECTION stopCS;
	CListViewCtrl ctrlHubs;
	boolean refresh;

	void startReader() {
		DWORD threadId;
		InitializeCriticalSection(&stopCS);
		stopEvent=CreateEvent(NULL, FALSE, FALSE, NULL);
		readerThread=CreateThread(NULL, 0, &FillHubList, this, 0, &threadId);
	}
	
	void stopReader() {
		if(readerThread != NULL) {
			EnterCriticalSection(&stopCS);
			SetEvent(stopEvent);
			LeaveCriticalSection(&stopCS);
			
			/* Not necessary here, as the thread will enter critical as soon as it actually matters
			if(WaitForSingleObject(readerThread, 1000) == WAIT_TIMEOUT) {
				::MessageBox(NULL, _T("Unable to stop reader thread!!!"), _T("Internal error"), MB_OK | MB_ICONERROR);
			}
			*/
			readerThread = NULL;
			CloseHandle(stopEvent);
			stopEvent = NULL;
			DeleteCriticalSection(&stopCS);
		}
	}
	
public:
	PublicHubsDlg() : readerThread(NULL), stopEvent(NULL), refresh(false) { };
	~PublicHubsDlg() {
		stopReader();
	}
	string server;

	enum { IDD = IDD_HUBS };
	
	BEGIN_MSG_MAP(PublicHubsDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		NOTIFY_HANDLER(IDC_HUBLIST, LVN_ITEMCHANGED, OnItemchangedHublist)
		COMMAND_HANDLER(IDC_REFRESH, BN_CLICKED, OnClickedRefresh)
	END_MSG_MAP()
		
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnItemchangedHublist(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnClickedRefresh(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
};

#endif // !defined(AFX_PUBLICHUBSDLG_H__B70BEF34_84E7_4DD9_A2CE_785F29A00C27__INCLUDED_)

/**
 * @file PublicHubsDlg.h
 * $Id: PublicHubsDlg.h,v 1.1 2001/11/21 17:33:20 arnetheduck Exp $
 * @if LOG
 * $Log: PublicHubsDlg.h,v $
 * Revision 1.1  2001/11/21 17:33:20  arnetheduck
 * Initial revision
 *
 * @endif
 */




















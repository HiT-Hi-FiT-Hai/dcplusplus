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

#if !defined(AFX_LineDLG_H__A7EB85C3_1EEA_4FEC_8450_C090219B8619__INCLUDED_)
#define AFX_LineDLG_H__A7EB85C3_1EEA_4FEC_8450_C090219B8619__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class LineDlg : public CDialogImpl<LineDlg>
{
	CEdit ctrlLine;
	CStatic ctrlDescription;
public:
	string line;
	string description;
	string title;
	bool password;

	enum { IDD = IDD_LINE };
	
	BEGIN_MSG_MAP(LineDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_SETFOCUS, onFocus)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()
	
	LineDlg() : password(false) { };
	
	LRESULT onFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		ctrlLine.SetFocus();
		return FALSE;
	}

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		ctrlLine.Attach(GetDlgItem(IDC_LINE));
		ctrlLine.SetFocus();
		ctrlLine.SetWindowText(line.c_str());
		if(password) {
			ctrlLine.SetWindowLong(GWL_STYLE, ctrlLine.GetWindowLong(GWL_STYLE) | ES_PASSWORD);
		}

		ctrlDescription.Attach(GetDlgItem(IDC_DESCRIPTION));
		ctrlDescription.SetWindowText(description.c_str());
		
		SetWindowText(title.c_str());
		
		CenterWindow(GetParent());
		return FALSE;
	}
	
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		if(wID == IDOK) {
			char buf[256];
			GetDlgItemText(IDC_LINE, buf, 256);
			line = buf;
		}
		EndDialog(wID);
		return 0;
	}
	
};

#endif // !defined(AFX_LineDLG_H__A7EB85C3_1EEA_4FEC_8450_C090219B8619__INCLUDED_)

/**
 * @file LineDlg.h
 * $Id: LineDlg.h,v 1.3 2002/01/20 22:54:46 arnetheduck Exp $
 * @if LOG
 * $Log: LineDlg.h,v $
 * Revision 1.3  2002/01/20 22:54:46  arnetheduck
 * Bugfixes to 0.131 mainly...
 *
 * Revision 1.2  2001/12/27 18:14:36  arnetheduck
 * Version 0.08, here we go...
 *
 * Revision 1.1  2001/12/27 16:07:40  arnetheduck
 * Replaced PasswordDlg with LineDlg
 *
 * Revision 1.1  2001/12/19 23:08:39  arnetheduck
 * Line dialog window
 *
 * @endif
 */
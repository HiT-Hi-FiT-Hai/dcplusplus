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

#if !defined(AFX_SETTINGSDLG_H__25031C63_A95B_43D9_8A1E_892FF932890B__INCLUDED_)
#define AFX_SETTINGSDLG_H__25031C63_A95B_43D9_8A1E_892FF932890B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define SETTINGS_BUF_LEN 1024

class SettingsDlg : public CDialogImpl<SettingsDlg>  
{
public:
	string nick;
	string email;
	string description;
	string connection;
	
	enum { IDD = IDD_SETTINGS };
	
	BEGIN_MSG_MAP(CAboutDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()
		
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		SetDlgItemText(IDC_NICK, nick.c_str());
		SetDlgItemText(IDC_EMAIL, email.c_str());
		SetDlgItemText(IDC_DESCRIPTION, description.c_str());
		SetDlgItemText(IDC_CONNECTION, connection.c_str());

		CenterWindow(GetParent());
		return TRUE;
	}
	
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		if(wID == IDOK) {
			char buf[SETTINGS_BUF_LEN];
			GetDlgItemText(IDC_NICK, buf, SETTINGS_BUF_LEN);
			nick = buf;
			GetDlgItemText(IDC_EMAIL, buf, SETTINGS_BUF_LEN);
			email = buf;
			GetDlgItemText(IDC_DESCRIPTION, buf, SETTINGS_BUF_LEN);
			description = buf;
			GetDlgItemText(IDC_CONNECTION, buf, SETTINGS_BUF_LEN);
			connection = buf;
			
		}
		EndDialog(wID);
		return 0;
	}
	
};

#endif // !defined(AFX_SETTINGSDLG_H__25031C63_A95B_43D9_8A1E_892FF932890B__INCLUDED_)

/**
 * @file SettingsDlg.h
 * $Id: SettingsDlg.h,v 1.1 2001/11/22 19:47:42 arnetheduck Exp $
 * @if LOG
 * $Log: SettingsDlg.h,v $
 * Revision 1.1  2001/11/22 19:47:42  arnetheduck
 * A simple XML parser. Doesn't have all the features, but works good enough for
 * the configuration file.
 *
 * @endif
 */

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

#if !defined(AFX_CommandDlg_H__A7EB85C3_1EEA_4FEC_8450_C090219B8619__INCLUDED_)
#define AFX_CommandDlg_H__A7EB85C3_1EEA_4FEC_8450_C090219B8619__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CommandDlg : public CDialogImpl<CommandDlg>
{
	CEdit ctrlName;
	CEdit ctrlCommand;
	CEdit ctrlHub;
	CEdit ctrlNick;
public:
	string name;
	string command;
	string hub;
	string nick;

	enum { IDD = IDD_USER_COMMAND };

	BEGIN_MSG_MAP(CommandDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_SETFOCUS, onFocus)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	CommandDlg() { };

	LRESULT onFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		ctrlName.SetFocus();
		return FALSE;
	}

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{

#define ATTACH(id, var, txt) \
		var.Attach(GetDlgItem(id)); \
		var.SetWindowText(txt.c_str());

		ATTACH(IDC_NAME, ctrlName, name);
		ATTACH(IDC_COMMAND, ctrlCommand, command);
		ATTACH(IDC_HUB, ctrlHub, hub);
		ATTACH(IDC_NICK, ctrlNick, nick);
#undef ATTACH

		SetDlgItemText(IDC_COMMAND_DESCRIPTION, "\
You can add parameters to your commands using the same format as for the logs, %[parameter], choosing from these:\n\
nick\t\tNick of the user\n\
mynick\t\tYour nick on that hub\n\
file\t\tFilename (search only)\n\
line:desc\tFor each of these, a one line dialog will ask what to put there (try and you'll understand).\n\
Date and time specifiers work as well (%Y, %m, ...)\n\
\n\
Example: +ban %[nick] %[line:Time] %[line:Reason]");
		ctrlName.SetFocus();

		CenterWindow(GetParent());
		return FALSE;
	}

	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		if(wID == IDOK) {
			char buf[256];

#define GET_TEXT(id, var) \
			GetDlgItemText(id, buf, 256); \
			var = buf;

			GET_TEXT(IDC_NAME, name);
			GET_TEXT(IDC_COMMAND, command);
			GET_TEXT(IDC_HUB, hub);
			GET_TEXT(IDC_NICK, nick);

			if(name.empty() || command.empty()) {
				MessageBox("Name and command must not be empty");
				return 0;
			}
			if(command.find_first_of("|$") != string::npos) {
				MessageBox("Command may not contain $ or |");
				return 0;
			}
		}
		EndDialog(wID);
		return 0;
	}

};

#endif // !defined(AFX_CommandDlg_H__A7EB85C3_1EEA_4FEC_8450_C090219B8619__INCLUDED_)

/**
 * @file
 * $Id: CommandDlg.h,v 1.3 2003/05/28 11:53:05 arnetheduck Exp $
 */

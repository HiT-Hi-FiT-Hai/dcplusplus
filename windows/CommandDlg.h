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

#include "../client/Util.h"

class CommandDlg : public CDialogImpl<CommandDlg>
{
	CEdit ctrlName;
	CEdit ctrlCommand;
	CEdit ctrlHub;
	CEdit ctrlNick;
	CButton ctrlSeparator;
	CButton ctrlRaw;
	CButton ctrlChat;
	CButton ctrlPM;
	CButton ctrlHubMenu;
	CButton ctrlUserMenu;
	CButton ctrlSearchMenu;
	CButton ctrlOnce;
	CEdit ctrlResult;

public:
	int type;
	int ctx;
	string name;
	string command;
	string hub;

	enum { IDD = IDD_USER_COMMAND };

	BEGIN_MSG_MAP(CommandDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_SETFOCUS, onFocus)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		COMMAND_ID_HANDLER(IDC_SETTINGS_SEPARATOR, onType)
		COMMAND_ID_HANDLER(IDC_SETTINGS_RAW, onType)
		COMMAND_ID_HANDLER(IDC_SETTINGS_CHAT, onType)
		COMMAND_ID_HANDLER(IDC_SETTINGS_PM, onType)
		COMMAND_HANDLER(IDC_COMMAND, EN_CHANGE, onChange)
		COMMAND_HANDLER(IDC_NICK, EN_CHANGE, onChange)
	END_MSG_MAP()

	CommandDlg() : type(0), ctx(0) { };

	LRESULT onFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		ctrlName.SetFocus();
		return FALSE;
	}

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onType(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onChange(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		updateContext();
		if(wID == IDOK) {
			char buf[256];

			if((type != 0) && 
				((ctrlName.GetWindowTextLength() == 0) || (ctrlCommand.GetWindowTextLength()== 0)))
			{
				MessageBox("Name and command must not be empty");
				return 0;
			}

#define GET_TEXT(id, var) \
			GetDlgItemText(id, buf, 256); \
			var = buf;

			GET_TEXT(IDC_NAME, name);
			GET_TEXT(IDC_HUB, hub);

			if(type != 0) {
				type = (ctrlOnce.GetCheck() == BST_CHECKED) ? 2 : 1;
			}
		}
		EndDialog(wID);
		return 0;
	}
private:
	void updateType() {
		if(ctrlSeparator.GetCheck() == BST_CHECKED) {
			type = 0;
		} else if(ctrlRaw.GetCheck() == BST_CHECKED) {
			type = 1;
		} else if(ctrlChat.GetCheck() == BST_CHECKED) {
			type = 2;
		} else if(ctrlPM.GetCheck() == BST_CHECKED) {
			type = 3;
		}
	}
	enum { BUF_LEN = 1024 };
	void updateCommand() {
		char buf[BUF_LEN];
		if(type == 0) {
			command.clear();
		} else if(type == 1) {
			ctrlCommand.GetWindowText(buf, BUF_LEN-1);
			command = buf;
		} else if(type == 2) {
			ctrlCommand.GetWindowText(buf, BUF_LEN - 1);
			command = "<%[mynick]> " + Util::validateMessage(buf, false) + "|";
		} else if(type == 3) {
			ctrlNick.GetWindowText(buf, BUF_LEN - 1);
			string to(buf);
			ctrlCommand.GetWindowText(buf, BUF_LEN - 1);
			command = "$To: " + to + " From: %[mynick] $<%[mynick]> " + Util::validateMessage(buf, false) + "|";
		}
	}
	void updateControls() {
		switch(type) {
		case 0:
			ctrlName.EnableWindow(FALSE);
			ctrlCommand.EnableWindow(FALSE);
			ctrlNick.EnableWindow(FALSE);
			break;
		case 1:
		case 2:
			ctrlName.EnableWindow(TRUE);
			ctrlCommand.EnableWindow(TRUE);
			ctrlNick.EnableWindow(FALSE);
			break;
		case 3:
			ctrlName.EnableWindow(TRUE);
			ctrlCommand.EnableWindow(TRUE);
			ctrlNick.EnableWindow(TRUE);
			break;
		}
	}
	void updateContext();
};

#endif // !defined(AFX_CommandDlg_H__A7EB85C3_1EEA_4FEC_8450_C090219B8619__INCLUDED_)

/**
 * @file
 * $Id: CommandDlg.h,v 1.6 2003/11/13 15:32:16 arnetheduck Exp $
 */

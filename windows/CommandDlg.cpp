/*
 * Copyright (C) 2001-2005 Jacek Sieka, arnetheduck on gmail point com
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
#include "../client/DCPlusPlus.h"
#include "Resource.h"
#include "../client/ResourceManager.h"

#include "../client/UserCommand.h"

#include "CommandDlg.h"

LRESULT CommandDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// Translate
	SetWindowText(CTSTRING(USER_CMD_WINDOW));
	SetDlgItemText(IDC_SETTINGS_TYPE, CTSTRING(USER_CMD_TYPE));
	SetDlgItemText(IDC_SETTINGS_SEPARATOR, CTSTRING(SEPARATOR));
	SetDlgItemText(IDC_SETTINGS_RAW, CTSTRING(USER_CMD_RAW));
	SetDlgItemText(IDC_SETTINGS_CHAT, CTSTRING(USER_CMD_CHAT));
	SetDlgItemText(IDC_SETTINGS_PM, CTSTRING(USER_CMD_PM));
	SetDlgItemText(IDC_SETTINGS_CONTEXT, CTSTRING(USER_CMD_CONTEXT));
	SetDlgItemText(IDC_SETTINGS_HUB_MENU, CTSTRING(USER_CMD_HUB_MENU));
	SetDlgItemText(IDC_SETTINGS_USER_MENU, CTSTRING(USER_CMD_USER_MENU));
	SetDlgItemText(IDC_SETTINGS_SEARCH_MENU, CTSTRING(USER_CMD_SEARCH_MENU));
	SetDlgItemText(IDC_SETTINGS_FILELIST_MENU, CTSTRING(USER_CMD_FILELIST_MENU));
	SetDlgItemText(IDC_SETTINGS_PARAMETERS, CTSTRING(USER_CMD_PARAMETERS));
	SetDlgItemText(IDC_SETTINGS_NAME, CTSTRING(HUB_NAME));
	SetDlgItemText(IDC_SETTINGS_COMMAND, CTSTRING(USER_CMD_COMMAND));
	SetDlgItemText(IDC_SETTINGS_HUB, CTSTRING(USER_CMD_HUB));
	SetDlgItemText(IDC_SETTINGS_TO, CTSTRING(USER_CMD_TO));
	SetDlgItemText(IDC_SETTINGS_ONCE, CTSTRING(USER_CMD_ONCE));
	SetDlgItemText(IDC_USER_CMD_PREVIEW, CTSTRING(USER_CMD_PREVIEW));
	SetDlgItemText(IDC_USER_CMD_OPEN_HELP, CTSTRING(SETTINGS_OPEN_USER_CMD_HELP));

#define ATTACH(id, var) var.Attach(GetDlgItem(id))
	ATTACH(IDC_RESULT, ctrlResult);
	ATTACH(IDC_NAME, ctrlName);
	ATTACH(IDC_HUB, ctrlHub);
	ATTACH(IDC_SETTINGS_SEPARATOR, ctrlSeparator);
	ATTACH(IDC_SETTINGS_RAW, ctrlRaw);
	ATTACH(IDC_SETTINGS_CHAT, ctrlChat);
	ATTACH(IDC_SETTINGS_PM, ctrlPM);
	ATTACH(IDC_SETTINGS_ONCE, ctrlOnce);
	ATTACH(IDC_SETTINGS_HUB_MENU, ctrlHubMenu);
	ATTACH(IDC_SETTINGS_USER_MENU, ctrlUserMenu);
	ATTACH(IDC_SETTINGS_SEARCH_MENU, ctrlSearchMenu);
	ATTACH(IDC_SETTINGS_FILELIST_MENU, ctrlFilelistMenu);
	ATTACH(IDC_NICK, ctrlNick);
	ATTACH(IDC_COMMAND, ctrlCommand);

	// launch the help file, instead of having the help in the dialog
	bool openHelp(BOOLSETTING(OPEN_USER_CMD_HELP));
	::CheckDlgButton(m_hWnd, IDC_USER_CMD_OPEN_HELP, openHelp);
	if(openHelp) {
		HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_UCPAGE);
	}

	if(type == UserCommand::TYPE_SEPARATOR) {
		ctrlSeparator.SetCheck(BST_CHECKED);
	} else {
		// More difficult, determine type by what it seems to be...
		if((_tcsncmp(command.c_str(), _T("$To: "), 5) == 0) &&
			(command.find(_T(" From: %[myNI] $<%[myNI]> ")) != string::npos ||
			command.find(_T(" From: %[mynick] $<%[mynick]> ")) != string::npos) &&
			command.find(_T('|')) == command.length() - 1) // if it has | anywhere but the end, it is raw
		{
			string::size_type i = command.find(_T(' '), 5);
			dcassert(i != string::npos);
			tstring to = command.substr(5, i-5);
			string::size_type cmd_pos = command.find(_T('>'), 5) + 2;
			tstring cmd = Text::toT(Util::validateMessage(Text::fromT(command.substr(cmd_pos, command.length()-cmd_pos-1)), true, false));
			ctrlPM.SetCheck(BST_CHECKED);
			ctrlNick.SetWindowText(to.c_str());
			ctrlCommand.SetWindowText(cmd.c_str());
		} else if(((_tcsncmp(command.c_str(), _T("<%[mynick]> "), 12) == 0) ||
			(_tcsncmp(command.c_str(), _T("<%[myNI]> "), 10) == 0)) &&
			command[command.length()-1] == '|')
		{
			// Looks like a chat thing...
			string::size_type cmd_pos = command.find(_T('>')) + 2;
			tstring cmd = Text::toT(Util::validateMessage(Text::fromT(command.substr(cmd_pos, command.length()-cmd_pos-1)), true, false));
			ctrlChat.SetCheck(BST_CHECKED);
			ctrlCommand.SetWindowText(cmd.c_str());
		} else {
			tstring cmd = command;
			ctrlRaw.SetCheck(BST_CHECKED);
			ctrlCommand.SetWindowText(cmd.c_str());
		}
		if(type == UserCommand::TYPE_RAW_ONCE) {
			ctrlOnce.SetCheck(BST_CHECKED);
			type = 1;
		}
	}

	ctrlHub.SetWindowText(hub.c_str());
	ctrlName.SetWindowText(name.c_str());

	if(ctx & UserCommand::CONTEXT_HUB)
		ctrlHubMenu.SetCheck(BST_CHECKED);
	if(ctx & UserCommand::CONTEXT_CHAT)
		ctrlUserMenu.SetCheck(BST_CHECKED);
	if(ctx & UserCommand::CONTEXT_SEARCH)
		ctrlSearchMenu.SetCheck(BST_CHECKED);
	if(ctx & UserCommand::CONTEXT_FILELIST)
		ctrlFilelistMenu.SetCheck(BST_CHECKED);
	
	updateControls();
	updateCommand();
	ctrlResult.SetWindowText(command.c_str());

	ctrlSeparator.SetFocus();
	
	CenterWindow(GetParent());
	return FALSE;
}

LRESULT CommandDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	updateContext();
	if(wID == IDOK) {
		TCHAR buf[256];

		if((type != 0) &&
			((ctrlName.GetWindowTextLength() == 0) || (ctrlCommand.GetWindowTextLength()== 0)))
		{
			MessageBox(_T("Name and command must not be empty"));
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
		SettingsManager::getInstance()->set(SettingsManager::OPEN_USER_CMD_HELP, IsDlgButtonChecked(IDC_USER_CMD_OPEN_HELP) == BST_CHECKED);
	}
	EndDialog(wID);
	return 0;
}

LRESULT CommandDlg::onChange(WORD , WORD , HWND , BOOL& ) {
	updateCommand();
	ctrlResult.SetWindowText(command.c_str());
	return 0;
}

LRESULT CommandDlg::onType(WORD , WORD, HWND , BOOL& ) {
	updateType();
	updateCommand();
	ctrlResult.SetWindowText(command.c_str());
	updateControls();
	return 0;
}

void CommandDlg::updateContext() {
	ctx = 0;
	if(ctrlHubMenu.GetCheck() & BST_CHECKED)
		ctx |= UserCommand::CONTEXT_HUB;
	if(ctrlUserMenu.GetCheck() & BST_CHECKED)
		ctx |= UserCommand::CONTEXT_CHAT;
	if(ctrlSearchMenu.GetCheck() & BST_CHECKED)
		ctx |= UserCommand::CONTEXT_SEARCH;
	if(ctrlFilelistMenu.GetCheck() & BST_CHECKED)
		ctx |= UserCommand::CONTEXT_FILELIST;
}

LRESULT CommandDlg::onHelp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_UCPAGE);
	return 0;
}

LRESULT CommandDlg::onHelpCmd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_UCPAGE);
	return 0;
}

/**
 * @file
 * $Id: CommandDlg.cpp,v 1.18 2006/01/11 21:31:01 arnetheduck Exp $
 */

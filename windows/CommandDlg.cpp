/* 
* Copyright (C) 2001-2004 Jacek Sieka, j_s at telia com
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

#include "../client/UserCommand.h"

#include "CommandDlg.h"

LRESULT CommandDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{

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
	ATTACH(IDC_NICK, ctrlNick);
	ATTACH(IDC_COMMAND, ctrlCommand);

	SetDlgItemText(IDC_COMMAND_DESCRIPTION, _T("\
Command Types:\r\n\
Separator: Adds a separator to the menu\r\n\
Raw: Sends raw command to the hub (experts only, end it with '|'!)\r\n\
Chat: Sends command as if you were typing it in the chat\r\n\
PM: Sends command as if you sent it by pm\r\n\
Contexts determine where the command is shown:\r\n\
Hub Menu: Hub tab (at the bottom of the screen) right-click menu\r\n\
Chat Menu: User right-click menu in chat and PM tab menu\r\n\
Search Menu: Search right-click menu\r\n\
Parameters:\r\n\
Name: Name (use '\\' to create submenus)\r\n\
Command: Command text (may contain parameters)\r\n")
_T("Hub: Hub ip as typed when connecting (empty = all hubs, \"op\" = hubs where you're op)\r\n\
To: PM recipient\r\n\
Only once: Send only once per user from search frame\r\n\
In the parameters, you can use %[xxx] variables and date/time specifiers (%Y, %m, ...). The following are available:\r\n\
%[mynick]: your own nick\r\n\
%[nick]: the users nick (user && search context only)\r\n\
%[tag]: user tag (user && search context only)\r\n\
%[description]: user description (user && search context only)\r\n\
%[email]: user email (user && search context only)\r\n\
%[share]: user shared bytes (exact) (user && search context only)\r\n\
%[shareshort]: user shared bytes (formatted) (user && search context only)\r\n\
%[ip]: user ip (if supported by hub)\r\n\
%[file]: filename (search context only)\r\n\
%[line:reason]: opens up a window asking for \"reason\"\
"));

	if(type == UserCommand::TYPE_SEPARATOR) {
		ctrlSeparator.SetCheck(BST_CHECKED);
	} else {
		// More difficult, determine type by what it seems to be...
		if((_tcsncmp(command.c_str(), _T("$To: "), 5) == 0) &&
			command.find(_T(" From: %[mynick] $<%[mynick]> ")) != string::npos &&
			command.find(_T('|')) == string::npos) 
		{
			string::size_type i = command.find(_T(' '), 5);
			dcassert(i != string::npos);
			tstring to = command.substr(5, i-5);
			tstring cmd = Text::toT(Util::validateMessage(Text::fromT(command.substr(i + 30, command.length()-i-31)), true, false));
			ctrlPM.SetCheck(BST_CHECKED);
			ctrlNick.SetWindowText(to.c_str());
			ctrlCommand.SetWindowText(cmd.c_str());
		} else if((_tcsncmp(command.c_str(), _T("<%[mynick]> "), 12) == 0) &&
			command[command.length()-1] == '|') 
		{
			// Looks like a chat thing...
			tstring cmd = Text::toT(Util::validateMessage(Text::fromT(command.substr(12, command.length()-13)), true, false));
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
}

/**
* @file
* $Id: CommandDlg.cpp,v 1.11 2004/09/10 14:44:17 arnetheduck Exp $
*/

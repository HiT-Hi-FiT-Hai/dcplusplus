/*
 * Copyright (C) 2001-2008 Jacek Sieka, arnetheduck on gmail point com
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

#include "resource.h"

#include "CommandDlg.h"

#include <dcpp/UserCommand.h>
#include <dcpp/NmdcHub.h>
#include <dcpp/version.h>
#include "WinUtil.h"

static const WinUtil::HelpItem helpItems[] = {
	{ IDC_SETTINGS_SEPARATOR, IDH_USER_COMMAND_SEPARATOR },
	{ IDC_SETTINGS_RAW, IDH_USER_COMMAND_RAW },
	{ IDC_SETTINGS_CHAT, IDH_USER_COMMAND_CHAT },
	{ IDC_SETTINGS_PM, IDH_USER_COMMAND_PM },
	{ IDC_SETTINGS_CONTEXT, IDH_USER_COMMAND_CONTEXT },
	{ IDC_SETTINGS_HUB_MENU, IDH_USER_COMMAND_HUB_MENU },
	{ IDC_SETTINGS_USER_MENU, IDH_USER_COMMAND_USER_MENU },
	{ IDC_SETTINGS_SEARCH_MENU, IDH_USER_COMMAND_SEARCH_MENU },
	{ IDC_SETTINGS_FILELIST_MENU, IDH_USER_COMMAND_FILELIST_MENU },
	{ IDC_SETTINGS_NAME, IDH_USER_COMMAND_NAME },
	{ IDC_NAME, IDH_USER_COMMAND_NAME },
	{ IDC_SETTINGS_COMMAND, IDH_USER_COMMAND_COMMAND },
	{ IDC_COMMAND, IDH_USER_COMMAND_COMMAND },
	{ IDC_SETTINGS_HUB, IDH_USER_COMMAND_HUB },
	{ IDC_HUB, IDH_USER_COMMAND_HUB },
	{ IDC_SETTINGS_TO, IDH_USER_COMMAND_NICK },
	{ IDC_NICK, IDH_USER_COMMAND_NICK },
	{ IDC_SETTINGS_ONCE, IDH_USER_COMMAND_ONCE },
	{ IDOK, IDH_DCPP_OK },
	{ IDCANCEL, IDH_DCPP_CANCEL },
	{ IDHELP, IDH_DCPP_HELP },
	{ 0, 0 }
};

CommandDlg::CommandDlg(SmartWin::Widget* parent, int type_, int ctx_, const tstring& name_, const tstring& command_, const tstring& hub_) :
	WidgetFactory<SmartWin::ModalDialog>(parent),
	separator(0),
	raw(0),
	chat(0),
	PM(0),
	hubMenu(0),
	userMenu(0),
	searchMenu(0),
	fileListMenu(0),
	nameBox(0),
	commandBox(0),
	hubBox(0),
	nick(0),
	once(0),
	result(0),
	openHelp(0),
	type(type_),
	ctx(ctx_),
	name(name_),
	command(command_),
	hub(hub_)
{
	onInitDialog(std::tr1::bind(&CommandDlg::handleInitDialog, this));
	onFocus(std::tr1::bind(&CommandDlg::handleFocus, this));
	onHelp(std::tr1::bind(&WinUtil::help, _1, _2));
}

CommandDlg::~CommandDlg() {
}

bool CommandDlg::handleInitDialog() {
	setHelpId(IDH_USER_COMMAND);

	WinUtil::setHelpIds(this, helpItems);

	// Translate
	setText(T_("Create / Modify User Command"));
	setItemText(IDC_SETTINGS_TYPE, T_("Command Type"));
	setItemText(IDC_SETTINGS_CONTEXT, T_("Context"));
	setItemText(IDC_SETTINGS_PARAMETERS, T_("Parameters"));
	setItemText(IDC_SETTINGS_NAME, T_("Name"));
	setItemText(IDC_SETTINGS_COMMAND, T_("Command"));
	setItemText(IDC_SETTINGS_HUB, T_("Hub IP / DNS (empty = all, 'op' = where operator)"));
	setItemText(IDC_SETTINGS_TO, T_("To"));
	setItemText(IDC_USER_CMD_PREVIEW, T_("Text sent to hub"));

	attachChild(separator, IDC_SETTINGS_SEPARATOR);
	separator->setText(T_("Separator"));
	separator->onClicked(std::tr1::bind(&CommandDlg::handleTypeChanged, this));

	attachChild(raw, IDC_SETTINGS_RAW);
	raw->setText(T_("Raw"));
	raw->onClicked(std::tr1::bind(&CommandDlg::handleTypeChanged, this));

	attachChild(chat, IDC_SETTINGS_CHAT);
	chat->setText(T_("Chat"));
	chat->onClicked(std::tr1::bind(&CommandDlg::handleTypeChanged, this));

	attachChild(PM, IDC_SETTINGS_PM);
	PM->setText(T_("PM"));
	PM->onClicked(std::tr1::bind(&CommandDlg::handleTypeChanged, this));

	hubMenu = attachCheckBox(IDC_SETTINGS_HUB_MENU);
	hubMenu->setText(T_("Hub Menu"));

	userMenu = attachCheckBox(IDC_SETTINGS_USER_MENU);
	userMenu->setText(T_("User Menu"));

	searchMenu = attachCheckBox(IDC_SETTINGS_SEARCH_MENU);
	searchMenu->setText(T_("Search Menu"));

	fileListMenu = attachCheckBox(IDC_SETTINGS_FILELIST_MENU);
	fileListMenu->setText(T_( "Filelist Menu"));

	nameBox = attachTextBox(IDC_NAME);

	commandBox = attachTextBox(IDC_COMMAND);
	commandBox->onTextChanged(std::tr1::bind(&CommandDlg::updateCommand, this));

	hubBox = attachTextBox(IDC_HUB);

	nick = attachTextBox(IDC_NICK);
	nick->onTextChanged(std::tr1::bind(&CommandDlg::updateCommand, this));

	once = attachCheckBox(IDC_SETTINGS_ONCE);
	once->setText(T_("Send once per nick"));

	result = attachTextBox(IDC_RESULT);

	openHelp = attachCheckBox(IDC_USER_CMD_OPEN_HELP);
	openHelp->setText(T_("Always open help file with this dialog"));
	bool bOpenHelp = BOOLSETTING(OPEN_USER_CMD_HELP);
	openHelp->setChecked(bOpenHelp);

	{
		ButtonPtr button = attachButton(IDOK);
		button->setText(T_("OK"));
		button->onClicked(std::tr1::bind(&CommandDlg::handleOKClicked, this));

		button = attachButton(IDCANCEL);
		button->setText(T_("Cancel"));
		button->onClicked(std::tr1::bind(&CommandDlg::endDialog, this, IDCANCEL));

		button = attachButton(IDHELP);
		button->setText(T_("Help"));
		button->onClicked(std::tr1::bind(&WinUtil::help, handle(), IDH_USER_COMMAND));
	}

	if(bOpenHelp) {
		// launch the help file, instead of having the help in the dialog
		WinUtil::help(handle(), IDH_USER_COMMAND);
	}

	if(type == UserCommand::TYPE_SEPARATOR) {
		separator->setChecked(true);
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
			tstring cmd = Text::toT(NmdcHub::validateMessage(Text::fromT(command.substr(cmd_pos, command.length()-cmd_pos-1)), true));
			PM->setChecked(true);
			nick->setText(to);
			commandBox->setText(cmd.c_str());
		} else if(((_tcsncmp(command.c_str(), _T("<%[mynick]> "), 12) == 0) ||
			(_tcsncmp(command.c_str(), _T("<%[myNI]> "), 10) == 0)) &&
			command[command.length()-1] == '|')
		{
			// Looks like a chat thing...
			string::size_type cmd_pos = command.find(_T('>')) + 2;
			tstring cmd = Text::toT(NmdcHub::validateMessage(Text::fromT(command.substr(cmd_pos, command.length()-cmd_pos-1)), true));
			chat->setChecked(true);
			commandBox->setText(cmd);
		} else {
			tstring cmd = command;
			raw->setChecked(true);
			commandBox->setText(cmd);
		}
		if(type == UserCommand::TYPE_RAW_ONCE) {
			once->setChecked(true);
			type = 1;
		}
	}

	hubBox->setText(hub);
	nameBox->setText(name);

	if(ctx & UserCommand::CONTEXT_HUB)
		hubMenu->setChecked(true);
	if(ctx & UserCommand::CONTEXT_CHAT)
		userMenu->setChecked(true);
	if(ctx & UserCommand::CONTEXT_SEARCH)
		searchMenu->setChecked(true);
	if(ctx & UserCommand::CONTEXT_FILELIST)
		fileListMenu->setChecked(true);

	updateControls();
	updateCommand();

	separator->setFocus();

	centerWindow();
	return false;
}

void CommandDlg::handleFocus() {
	nameBox->setFocus();
}

void CommandDlg::handleTypeChanged() {
	updateType();
	updateCommand();
	updateControls();
}

void CommandDlg::handleOKClicked() {
	name = nameBox->getText();
	if((type != 0) && (name.empty() || commandBox->getText().empty())) {
		createMessageBox().show(T_("Name and command must not be empty"), _T(APPNAME) _T(" ") _T(VERSIONSTRING), MessageBox::BOX_OK, MessageBox::BOX_ICONEXCLAMATION);
		return;
	}

	ctx = 0;
	if(hubMenu->getChecked())
		ctx |= UserCommand::CONTEXT_HUB;
	if(userMenu->getChecked())
		ctx |= UserCommand::CONTEXT_CHAT;
	if(searchMenu->getChecked())
		ctx |= UserCommand::CONTEXT_SEARCH;
	if(fileListMenu->getChecked())
		ctx |= UserCommand::CONTEXT_FILELIST;

	hub = hubBox->getText();

	if(type != 0)
		type = once->getChecked() ? 2 : 1;

	SettingsManager::getInstance()->set(SettingsManager::OPEN_USER_CMD_HELP, openHelp->getChecked());

	endDialog(IDOK);
}

void CommandDlg::updateType() {
	if(separator->getChecked()) {
		type = 0;
	} else if(raw->getChecked()) {
		type = 1;
	} else if(chat->getChecked()) {
		type = 2;
	} else if(PM->getChecked()) {
		type = 3;
	}
}

void CommandDlg::updateCommand() {
	if(type == 0) {
		command.clear();
	} else if(type == 1) {
		command = commandBox->getText();
	} else if(type == 2) {
		command = Text::toT("<%[myNI]> " + NmdcHub::validateMessage(Text::fromT(commandBox->getText()), false) + "|");
	} else if(type == 3) {
		command = _T("$To: ") + nick->getText() + _T(" From: %[myNI] $<%[myNI]> ") + Text::toT(NmdcHub::validateMessage(Text::fromT(commandBox->getText()), false)) + _T("|");
	}
	result->setText(command);
}

void CommandDlg::updateControls() {
	switch(type) {
		case 0:
			nameBox->setEnabled(false);
			commandBox->setEnabled(false);
			nick->setEnabled(false);
			break;
		case 1:
		case 2:
			nameBox->setEnabled(true);
			commandBox->setEnabled(true);
			nick->setEnabled(false);
			break;
		case 3:
			nameBox->setEnabled(true);
			commandBox->setEnabled(true);
			nick->setEnabled(true);
			break;
	}
}

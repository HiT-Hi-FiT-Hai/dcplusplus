/*
 * Copyright (C) 2001-2007 Jacek Sieka, arnetheduck on gmail point com
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
#include <dcpp/DCPlusPlus.h>

#include "resource.h"

#include "CommandDlg.h"

#include <dcpp/ResourceManager.h>
#include <dcpp/UserCommand.h>
#include <dcpp/NmdcHub.h>
#include <dcpp/version.h>

CommandDlg::CommandDlg(SmartWin::Widget* parent, int type_, int ctx_, const tstring& name_, const tstring& command_, const tstring& hub_) :
	SmartWin::Widget(parent),
	type(type_),
	ctx(ctx_),
	name(name_),
	command(command_),
	hub(hub_)
{
	onInitDialog(std::tr1::bind(&CommandDlg::handleInitDialog, this));
	onFocus(std::tr1::bind(&CommandDlg::handleFocus, this));
}

CommandDlg::~CommandDlg() {
}

bool CommandDlg::handleInitDialog() {
	// Translate
	setText(TSTRING(USER_CMD_WINDOW));
	::SetDlgItemText(handle(), IDC_SETTINGS_TYPE, CTSTRING(USER_CMD_TYPE));
	::SetDlgItemText(handle(), IDC_SETTINGS_CONTEXT, CTSTRING(USER_CMD_CONTEXT));
	::SetDlgItemText(handle(), IDC_SETTINGS_PARAMETERS, CTSTRING(USER_CMD_PARAMETERS));
	::SetDlgItemText(handle(), IDC_SETTINGS_NAME, CTSTRING(HUB_NAME));
	::SetDlgItemText(handle(), IDC_SETTINGS_COMMAND, CTSTRING(USER_CMD_COMMAND));
	::SetDlgItemText(handle(), IDC_SETTINGS_HUB, CTSTRING(USER_CMD_HUB));
	::SetDlgItemText(handle(), IDC_SETTINGS_TO, CTSTRING(USER_CMD_TO));
	::SetDlgItemText(handle(), IDC_USER_CMD_PREVIEW, CTSTRING(USER_CMD_PREVIEW));

	separator = subclassRadioButton(IDC_SETTINGS_SEPARATOR);
	separator->setText(TSTRING(SEPARATOR));
	separator->onClicked(std::tr1::bind(&CommandDlg::handleTypeChanged, this));

	raw = subclassRadioButton(IDC_SETTINGS_RAW);
	raw->setText(TSTRING(USER_CMD_RAW));
	raw->onClicked(std::tr1::bind(&CommandDlg::handleTypeChanged, this));

	chat = subclassRadioButton(IDC_SETTINGS_CHAT);
	chat->setText(TSTRING(USER_CMD_CHAT));
	chat->onClicked(std::tr1::bind(&CommandDlg::handleTypeChanged, this));

	PM = subclassRadioButton(IDC_SETTINGS_PM);
	PM->setText(TSTRING(USER_CMD_PM));
	PM->onClicked(std::tr1::bind(&CommandDlg::handleTypeChanged, this));

	hubMenu = subclassCheckBox(IDC_SETTINGS_HUB_MENU);
	hubMenu->setText(TSTRING(USER_CMD_HUB_MENU));

	userMenu = subclassCheckBox(IDC_SETTINGS_USER_MENU);
	userMenu->setText(TSTRING(USER_CMD_USER_MENU));

	searchMenu = subclassCheckBox(IDC_SETTINGS_SEARCH_MENU);
	searchMenu->setText(TSTRING(USER_CMD_SEARCH_MENU));

	fileListMenu = subclassCheckBox(IDC_SETTINGS_FILELIST_MENU);
	fileListMenu->setText(TSTRING(USER_CMD_FILELIST_MENU));

	nameBox = static_cast<WidgetTextBoxPtr>(subclassTextBox(IDC_NAME));

	commandBox = static_cast<WidgetTextBoxPtr>(subclassTextBox(IDC_COMMAND));
	commandBox->onRaw(std::tr1::bind(&CommandDlg::handleTextChanged, this, _1, _2), SmartWin::Message(WM_COMMAND, EN_CHANGE));

	hubBox = static_cast<WidgetTextBoxPtr>(subclassTextBox(IDC_HUB));

	nick = static_cast<WidgetTextBoxPtr>(subclassTextBox(IDC_NICK));
	nick->onRaw(std::tr1::bind(&CommandDlg::handleTextChanged, this, _1, _2), SmartWin::Message(WM_COMMAND, EN_CHANGE));

	once = subclassCheckBox(IDC_SETTINGS_ONCE);
	once->setText(TSTRING(USER_CMD_ONCE));

	result = static_cast<WidgetTextBoxPtr>(subclassTextBox(IDC_RESULT));

	openHelp = subclassCheckBox(IDC_USER_CMD_OPEN_HELP);
	openHelp->setText(TSTRING(SETTINGS_OPEN_USER_CMD_HELP));
	bool bOpenHelp = BOOLSETTING(OPEN_USER_CMD_HELP);
	openHelp->setChecked(bOpenHelp);

	subclassButton(IDOK)->onClicked(std::tr1::bind(&CommandDlg::handleOKClicked, this));

	subclassButton(IDCANCEL)->onClicked(std::tr1::bind(&CommandDlg::endDialog, this, IDCANCEL));

	subclassButton(IDHELP)->onClicked(std::tr1::bind(&CommandDlg::handleHelpClicked, this));

	if(bOpenHelp) {
#ifdef PORT_ME
		// launch the help file, instead of having the help in the dialog
		HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_UCPAGE);
#endif
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

#ifdef PORT_ME
	CenterWindow(GetParent());
#endif
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

HRESULT CommandDlg::handleTextChanged(WPARAM wParam, LPARAM lParam) {
	updateCommand();
	return 0;
}

void CommandDlg::handleOKClicked() {
	name = nameBox->getText();
	if((type != 0) && (name.empty() || commandBox->getText().empty())) {
		createMessageBox().show(_T("Name and command must not be empty"), _T(APPNAME) _T(" ") _T(VERSIONSTRING), WidgetMessageBox::BOX_OK, WidgetMessageBox::BOX_ICONEXCLAMATION);
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

void CommandDlg::handleHelpClicked() {
#ifdef PORT_ME
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_UCPAGE);
#endif
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

#ifdef PORT_ME
LRESULT CommandDlg::onHelp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_UCPAGE);
	return 0;
}
#endif

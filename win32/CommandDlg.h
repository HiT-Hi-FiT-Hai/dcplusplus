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

#ifndef DCPLUSPLUS_WIN32_COMMAND_DLG_H
#define DCPLUSPLUS_WIN32_COMMAND_DLG_H

#include <dcpp/Util.h>

class CommandDlg : public SmartWin::WidgetFactory<SmartWin::WidgetModalDialog, CommandDlg>
{
public:
	CommandDlg(SmartWin::Widget* parent, int type_ = 0, int ctx_ = 0, const tstring& name_ = Util::emptyStringT, const tstring& command_ = Util::emptyStringT, const tstring& hub_ = Util::emptyStringT);
	virtual ~CommandDlg();

	int run() { return createDialog(IDD_USER_COMMAND); }

#ifdef PORT_ME
	BEGIN_MSG_MAP(CommandDlg)
		MESSAGE_HANDLER(WM_HELP, onHelp)
	END_MSG_MAP()

	LRESULT onHelp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
#endif

	int getType() { return type; }
	int getCtx() { return ctx; }
	tstring getName() { return name; }
	tstring getCommand() { return command; }
	tstring getHub() { return hub; }

private:
	WidgetRadioButtonPtr separator, raw, chat, PM;
	WidgetCheckBoxPtr hubMenu, userMenu, searchMenu, fileListMenu;
	WidgetTextBoxPtr nameBox, commandBox, hubBox, nick;
	WidgetCheckBoxPtr once;
	WidgetTextBoxPtr result;
	WidgetCheckBoxPtr openHelp;

	int type;
	int ctx;
	tstring name;
	tstring command;
	tstring hub;

	bool handleInitDialog();
	void handleFocus();
	void handleTypeChanged(WidgetRadioButtonPtr);

	typedef SmartWin::WidgetTextBox<CommandDlg>* TextBoxMessageType;
	HRESULT handleTextChanged(TextBoxMessageType, LPARAM /*lParam*/, WPARAM /*wParam*/);

	void handleOKClicked(WidgetButtonPtr);
	void handleCancelClicked(WidgetButtonPtr);
	void handleHelpClicked(WidgetButtonPtr);

	void updateType();
	void updateCommand();
	void updateControls();
};

#endif // !defined(DCPLUSPLUS_WIN32_COMMAND_DLG_H)

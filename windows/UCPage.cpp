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

#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "Resource.h"

#include "UCPage.h"
#include "CommandDlg.h"

#include "../client/SettingsManager.h"
#include "../client/HubManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

PropPage::TextItem UCPage::texts[] = {
	{ IDC_MOVE_UP, ResourceManager::MOVE_UP },
	{ IDC_MOVE_DOWN, ResourceManager::MOVE_DOWN },
	{ IDC_ADD_MENU, ResourceManager::ADD_ACCEL },
	{ IDC_CHANGE_MENU, ResourceManager::SETTINGS_CHANGE },
	{ IDC_REMOVE_MENU, ResourceManager::REMOVE_ACCEL },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

PropPage::Item UCPage::items[] = {
	{ IDC_ROLLBACK, SettingsManager::ROLLBACK, PropPage::T_INT }, 
	{ IDC_CLVERSION, SettingsManager::CLIENTVERSION, PropPage::T_STR }, 
	{ IDC_BUFFERSIZE, SettingsManager::BUFFER_SIZE, PropPage::T_INT },
	{ IDC_MAX_TAB_ROWS, SettingsManager::MAX_TAB_ROWS, PropPage::T_INT },
	{ 0, 0, PropPage::T_END }
};

LRESULT UCPage::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	PropPage::tanslate((HWND)(*this), texts);
	PropPage::read((HWND)*this, items);

	CRect rc;

	ctrlCommands.Attach(GetDlgItem(IDC_MENU_ITEMS));
	ctrlCommands.GetClientRect(rc);

	ctrlCommands.InsertColumn(0, CSTRING(SETTINGS_NAME), LVCFMT_LEFT, rc.Width()/4, 0);
	ctrlCommands.InsertColumn(1, CSTRING(SETTINGS_COMMAND), LVCFMT_LEFT, rc.Width()*2 / 4, 1);
	ctrlCommands.InsertColumn(2, CSTRING(HUB), LVCFMT_LEFT, rc.Width() / 4, 2);

	if(BOOLSETTING(FULL_ROW_SELECT))
		ctrlCommands.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);

	// Do specialized reading here
	

	return TRUE;
}

LRESULT UCPage::onAddMenu(WORD , WORD , HWND , BOOL& ) {
	CommandDlg dlg;

	if(dlg.DoModal() == IDOK) {
		StringList lst;
		lst.push_back(dlg.name);
		lst.push_back(dlg.command);
		lst.push_back(dlg.hub);
		ctrlCommands.insert(lst);
	}
	return 0;
}

#define BUFLEN 256
LRESULT UCPage::onChangeMenu(WORD , WORD , HWND , BOOL& ) {
	if(ctrlCommands.GetSelectedCount() == 1) {
		int sel = ctrlCommands.GetSelectedIndex();
		char buf[BUFLEN];
		CommandDlg dlg;
		ctrlCommands.GetItemText(sel, 0, buf, BUFLEN);
		dlg.name = buf;
		ctrlCommands.GetItemText(sel, 1, buf, BUFLEN);
		dlg.command = buf;
		ctrlCommands.GetItemText(sel, 2, buf, BUFLEN);
		dlg.hub = buf;
		ctrlCommands.GetItemText(sel, 3, buf, BUFLEN);
		dlg.nick = buf;

		if(dlg.DoModal() == IDOK) {
			ctrlCommands.SetItemText(sel, 0, dlg.name.c_str());
			ctrlCommands.SetItemText(sel, 1, dlg.command.c_str());
			ctrlCommands.SetItemText(sel, 2, dlg.hub.c_str());
			ctrlCommands.SetItemText(sel, 3, dlg.nick.c_str());
		}
	}
	return 0;
}

void UCPage::write() {
	PropPage::write((HWND)*this, items);
}

/**
 * @file
 * $Id: UCPage.cpp,v 1.1 2003/10/21 17:12:22 arnetheduck Exp $
 */


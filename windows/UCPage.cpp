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
	UserCommand::List lst = HubManager::getInstance()->getUserCommands();
	for(UserCommand::Iter i = lst.begin(); i != lst.end(); ++i) {
		UserCommand& uc = *i;	
		if(!uc.isSet(UserCommand::FLAG_NOSAVE)) {
			addEntry(uc, ctrlCommands.GetItemCount());
		}
	}
	
	return TRUE;
}

LRESULT UCPage::onAddMenu(WORD , WORD , HWND , BOOL& ) {
	CommandDlg dlg;

	if(dlg.DoModal() == IDOK) {
		addEntry(HubManager::getInstance()->addUserCommand(dlg.type, dlg.ctx,
			0, dlg.name, dlg.command, dlg.hub), ctrlCommands.GetItemCount());
	}
	return 0;
}

LRESULT UCPage::onChangeMenu(WORD , WORD , HWND , BOOL& ) {
	if(ctrlCommands.GetSelectedCount() == 1) {
		int sel = ctrlCommands.GetSelectedIndex();
		UserCommand uc;
		HubManager::getInstance()->getUserCommand(ctrlCommands.GetItemData(sel), uc);
		
		CommandDlg dlg;
		dlg.type = uc.getType();
		dlg.ctx = uc.getCtx();
		dlg.name = uc.getName();
		dlg.command = uc.getCommand();
		dlg.hub = uc.getHub();

		if(dlg.DoModal() == IDOK) {
			if(dlg.type == UserCommand::TYPE_SEPARATOR)
				ctrlCommands.SetItemText(sel, 0, CSTRING(SEPARATOR));
			else
				ctrlCommands.SetItemText(sel, 0, dlg.name.c_str());
			ctrlCommands.SetItemText(sel, 1, dlg.command.c_str());
			ctrlCommands.SetItemText(sel, 2, dlg.hub.c_str());
			uc.setName(dlg.name);
			uc.setCommand(dlg.command);
			uc.setHub(dlg.hub);
			uc.setType(dlg.type);
			uc.setCtx(dlg.ctx);
			HubManager::getInstance()->updateUserCommand(uc);
		}
	}
	return 0;
}

LRESULT UCPage::onRemoveMenu(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(ctrlCommands.GetSelectedCount() == 1) {
		int i = ctrlCommands.GetNextItem(-1, LVNI_SELECTED);
		HubManager::getInstance()->removeUserCommnad(ctrlCommands.GetItemData(i));
		ctrlCommands.DeleteItem(i);
	}
	return 0;
}

LRESULT UCPage::onMoveUp(WORD , WORD , HWND , BOOL& ) {
	int i = ctrlCommands.GetSelectedIndex();
	if(i != -1 && i != 0) {
		int n = ctrlCommands.GetItemData(i);
		HubManager::getInstance()->moveUserCommand(n, -1);
		ctrlCommands.SetRedraw(FALSE);
		ctrlCommands.DeleteItem(i);
		UserCommand uc;
		HubManager::getInstance()->getUserCommand(n, uc);
		addEntry(uc, i-1);
		ctrlCommands.SelectItem(i-1);
		ctrlCommands.EnsureVisible(i-1, FALSE);
		ctrlCommands.SetRedraw(TRUE);
	}
	return 0;
}

LRESULT UCPage::onMoveDown(WORD , WORD , HWND , BOOL& ) {
	int i = ctrlCommands.GetSelectedIndex();
	if(i != -1 && i != (ctrlCommands.GetItemCount()-1) ) {
		int n = ctrlCommands.GetItemData(i);
		HubManager::getInstance()->moveUserCommand(n, 1);
		ctrlCommands.SetRedraw(FALSE);
		ctrlCommands.DeleteItem(i);
		UserCommand uc;
		HubManager::getInstance()->getUserCommand(n, uc);
		addEntry(uc, i+1);
		ctrlCommands.SelectItem(i+1);
		ctrlCommands.EnsureVisible(i+1, FALSE);
		ctrlCommands.SetRedraw(TRUE);
	}
	return 0;
}

void UCPage::addEntry(const UserCommand& uc, int pos) {
	StringList lst;
	if(uc.getType() == UserCommand::TYPE_SEPARATOR)
		lst.push_back(STRING(SEPARATOR));
	else
		lst.push_back(uc.getName());
	lst.push_back(uc.getCommand());
	lst.push_back(uc.getHub());
	ctrlCommands.insert(pos, lst, 0, (LPARAM)uc.getId());
}

void UCPage::write() {
	PropPage::write((HWND)*this, items);
}


/**
 * @file
 * $Id: UCPage.cpp,v 1.3 2003/10/24 23:35:42 arnetheduck Exp $
 */


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
#include <client/DCPlusPlus.h>

#include "resource.h"

#include "UCPage.h"

#include <client/SettingsManager.h>
#include <client/FavoriteManager.h>

PropPage::TextItem UCPage::texts[] = {
	{ IDC_MOVE_UP, ResourceManager::MOVE_UP },
	{ IDC_MOVE_DOWN, ResourceManager::MOVE_DOWN },
	{ IDC_ADD_MENU, ResourceManager::ADD },
	{ IDC_CHANGE_MENU, ResourceManager::SETTINGS_CHANGE },
	{ IDC_REMOVE_MENU, ResourceManager::REMOVE },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

PropPage::Item UCPage::items[] = {
	{ 0, 0, PropPage::T_END }
};

UCPage::UCPage(SmartWin::Widget* parent) : SmartWin::Widget(parent), PropPage() {
	createDialog(IDD_UCPAGE);

	PropPage::translate(handle(), texts);
	PropPage::read(handle(), items);

	HWND commands = ::GetDlgItem(handle(), IDC_MENU_ITEMS);
	ListView_SetExtendedListViewStyle(commands, LVS_EX_LABELTIP | LVS_EX_FULLROWSELECT);

	LVCOLUMN lv = { LVCF_FMT | LVCF_WIDTH| LVCF_TEXT | LVCF_SUBITEM  };

	lv.fmt = LVCFMT_LEFT;
	lv.cx = 100;
	lv.pszText = const_cast<LPTSTR>(CTSTRING(SETTINGS_NAME));
	lv.iSubItem = 0;
	ListView_InsertColumn(commands, 0, &lv);

	lv.fmt = LVCFMT_CENTER;
	RECT rc;
	::GetClientRect(commands, &rc);
	lv.cx = rc.right - rc.left - 220;
	lv.pszText = const_cast<LPTSTR>(CTSTRING(SETTINGS_COMMAND));
	lv.iSubItem = 1;
	ListView_InsertColumn(commands, 1, &lv);

	lv.fmt = LVCFMT_RIGHT;
	lv.cx = 100;
	lv.pszText = const_cast<LPTSTR>(CTSTRING(HUB));
	lv.iSubItem = 2;
	ListView_InsertColumn(commands, 2, &lv);

	UserCommand::List lst = FavoriteManager::getInstance()->getUserCommands();
	for(UserCommand::List::const_iterator i = lst.begin(); i != lst.end(); ++i) {
		const UserCommand& uc = *i;
		if(!uc.isSet(UserCommand::FLAG_NOSAVE)) {
			addEntry(uc, ListView_GetItemCount(commands));
		}
	}
}

UCPage::~UCPage() {
}

void UCPage::write() {
	PropPage::write(handle(), items);
}

void UCPage::addEntry(const UserCommand& uc, int pos) {
	HWND commands = ::GetDlgItem(handle(), IDC_MENU_ITEMS);
	LVITEM lvi = { LVIF_TEXT | LVIF_PARAM };
	lvi.iItem = pos;
	lvi.pszText = const_cast<LPTSTR>(((uc.getType() == UserCommand::TYPE_SEPARATOR) ? TSTRING(SEPARATOR) : uc.getName()).c_str());
	lvi.lParam = (LPARAM)uc.getId();
	int i = ListView_InsertItem(commands, &lvi);
	ListView_SetItemText(commands, i, 1, const_cast<LPTSTR>(uc.getCommand().c_str()));
	ListView_SetItemText(commands, i, 2, const_cast<LPTSTR>(uc.getHub().c_str()));
}

#ifdef PORT_ME

LRESULT UCPage::onAddMenu(WORD , WORD , HWND , BOOL& ) {
	CommandDlg dlg;

	if(dlg.DoModal() == IDOK) {
		addEntry(FavoriteManager::getInstance()->addUserCommand(dlg.type, dlg.ctx,
			0, Text::fromT(dlg.name), Text::fromT(dlg.command), Text::fromT(dlg.hub)), ctrlCommands.GetItemCount());
	}
	return 0;
}

LRESULT UCPage::onChangeMenu(WORD , WORD , HWND , BOOL& ) {
	if(ctrlCommands.GetSelectedCount() == 1) {
		int sel = ctrlCommands.GetSelectedIndex();
		UserCommand uc;
		FavoriteManager::getInstance()->getUserCommand(ctrlCommands.GetItemData(sel), uc);

		CommandDlg dlg;
		dlg.type = uc.getType();
		dlg.ctx = uc.getCtx();
		dlg.name = Text::toT(uc.getName());
		dlg.command = Text::toT(uc.getCommand());
		dlg.hub = Text::toT(uc.getHub());

		if(dlg.DoModal() == IDOK) {
			if(dlg.type == UserCommand::TYPE_SEPARATOR)
				ctrlCommands.SetItemText(sel, 0, CTSTRING(SEPARATOR));
			else
				ctrlCommands.SetItemText(sel, 0, dlg.name.c_str());
			ctrlCommands.SetItemText(sel, 1, dlg.command.c_str());
			ctrlCommands.SetItemText(sel, 2, dlg.hub.c_str());
			uc.setName(Text::fromT(dlg.name));
			uc.setCommand(Text::fromT(dlg.command));
			uc.setHub(Text::fromT(dlg.hub));
			uc.setType(dlg.type);
			uc.setCtx(dlg.ctx);
			FavoriteManager::getInstance()->updateUserCommand(uc);
		}
	}
	return 0;
}

LRESULT UCPage::onRemoveMenu(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(ctrlCommands.GetSelectedCount() == 1) {
		int i = ctrlCommands.GetNextItem(-1, LVNI_SELECTED);
		FavoriteManager::getInstance()->removeUserCommand(ctrlCommands.GetItemData(i));
		ctrlCommands.DeleteItem(i);
	}
	return 0;
}

LRESULT UCPage::onMoveUp(WORD , WORD , HWND , BOOL& ) {
	int i = ctrlCommands.GetSelectedIndex();
	if(i != -1 && i != 0) {
		int n = ctrlCommands.GetItemData(i);
		FavoriteManager::getInstance()->moveUserCommand(n, -1);
		ctrlCommands.SetRedraw(FALSE);
		ctrlCommands.DeleteItem(i);
		UserCommand uc;
		FavoriteManager::getInstance()->getUserCommand(n, uc);
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
		FavoriteManager::getInstance()->moveUserCommand(n, 1);
		ctrlCommands.SetRedraw(FALSE);
		ctrlCommands.DeleteItem(i);
		UserCommand uc;
		FavoriteManager::getInstance()->getUserCommand(n, uc);
		addEntry(uc, i+1);
		ctrlCommands.SelectItem(i+1);
		ctrlCommands.EnsureVisible(i+1, FALSE);
		ctrlCommands.SetRedraw(TRUE);
	}
	return 0;
}

LRESULT UCPage::onKeyDown(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled) {
	NMLVKEYDOWN* kd = (NMLVKEYDOWN*) pnmh;
	switch(kd->wVKey) {
	case VK_INSERT:
		PostMessage(WM_COMMAND, IDC_ADD_MENU, 0);
		break;
	case VK_DELETE:
		PostMessage(WM_COMMAND, IDC_REMOVE_MENU, 0);
		break;
	default:
		bHandled = FALSE;
	}
	return 0;
}

LRESULT UCPage::onDoubleClick(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NMITEMACTIVATE* item = (NMITEMACTIVATE*)pnmh;

	if(item->iItem >= 0) {
		PostMessage(WM_COMMAND, IDC_CHANGE_MENU, 0);
	} else if(item->iItem == -1) {
		PostMessage(WM_COMMAND, IDC_ADD_MENU, 0);
	}

	return 0;
}

LRESULT UCPage::onHelpInfo(LPNMHDR /*pnmh*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_UCPAGE);
	return 0;
}

LRESULT UCPage::onHelp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_UCPAGE);
	return 0;
}
#endif

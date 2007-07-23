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

#include "UCPage.h"

#include <dcpp/SettingsManager.h>
#include <dcpp/FavoriteManager.h>
#include "StupidWin.h"
#include "CommandDlg.h"

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

UCPage::UCPage(SmartWin::Widget* parent) : PropPage(parent) {
	createDialog(IDD_UCPAGE);

	PropPage::translate(handle(), texts);
	PropPage::read(handle(), items);

	commands = static_cast<WidgetDataGridPtr>(subclassList(IDC_MENU_ITEMS));
	commands->setListViewStyle(LVS_EX_LABELTIP | LVS_EX_FULLROWSELECT);

	TStringList columns;
	columns.push_back(TSTRING(SETTINGS_NAME));
	columns.push_back(TSTRING(SETTINGS_COMMAND));
	columns.push_back(TSTRING(HUB));
	commands->createColumns(columns);
	commands->setColumnWidth(0, 100);
	commands->setColumnWidth(1, commands->getSize().x - 220);
	commands->setColumnWidth(2, 100);

	UserCommand::List lst = FavoriteManager::getInstance()->getUserCommands();
	for(UserCommand::List::const_iterator i = lst.begin(); i != lst.end(); ++i) {
		const UserCommand& uc = *i;
		if(!uc.isSet(UserCommand::FLAG_NOSAVE))
			addEntry(uc);
	}

	commands->onRaw(std::tr1::bind(&UCPage::handleDoubleClick, this, _1, _2), SmartWin::Message(WM_NOTIFY, NM_DBLCLK));
	commands->onRaw(std::tr1::bind(&UCPage::handleKeyDown, this, _1, _2), SmartWin::Message(WM_NOTIFY, LVN_KEYDOWN));

	subclassButton(IDC_ADD_MENU)->onClicked(std::tr1::bind(&UCPage::handleAddClicked, this));

	subclassButton(IDC_CHANGE_MENU)->onClicked(std::tr1::bind(&UCPage::handleChangeClicked, this));

	subclassButton(IDC_MOVE_UP)->onClicked(std::tr1::bind(&UCPage::handleMoveUpClicked, this));

	subclassButton(IDC_MOVE_DOWN)->onClicked(std::tr1::bind(&UCPage::handleMoveDownClicked, this));

	subclassButton(IDC_REMOVE_MENU)->onClicked(std::tr1::bind(&UCPage::handleRemoveClicked, this));
}

UCPage::~UCPage() {
}

void UCPage::write() {
	PropPage::write(handle(), items);
}

HRESULT UCPage::handleDoubleClick(WPARAM wParam, LPARAM lParam) {
	LPNMITEMACTIVATE item = (LPNMITEMACTIVATE)lParam;
	if(item->iItem >= 0) {
		handleChangeClicked();
	} else if(item->iItem == -1) {
		handleAddClicked();
	}
	return 0;
}

HRESULT UCPage::handleKeyDown(WPARAM wParam, LPARAM lParam) {
	switch(((LPNMLVKEYDOWN)lParam)->wVKey) {
	case VK_INSERT:
		handleAddClicked();
		break;
	case VK_DELETE:
		handleRemoveClicked();
		break;
	}
	return 0;
}

void UCPage::handleAddClicked() {
	CommandDlg dlg(this);
	if(dlg.run() == IDOK)
		addEntry(FavoriteManager::getInstance()->addUserCommand(dlg.getType(), dlg.getCtx(), 0, Text::fromT(dlg.getName()), Text::fromT(dlg.getCommand()), Text::fromT(dlg.getHub())));
}

void UCPage::handleChangeClicked() {
	if(commands->getSelectedCount() == 1) {
		int i = commands->getSelectedIndex();
		UserCommand uc;
		FavoriteManager::getInstance()->getUserCommand(commands->getItemData(i), uc);

		CommandDlg dlg(this, uc.getType(), uc.getCtx(), Text::toT(uc.getName()), Text::toT(uc.getCommand()), Text::toT(uc.getHub()));
		if(dlg.run() == IDOK) {
			commands->setCellText(0, i, (dlg.getType() == UserCommand::TYPE_SEPARATOR) ? TSTRING(SEPARATOR) : dlg.getName());
			commands->setCellText(1, i, dlg.getCommand());
			commands->setCellText(2, i, dlg.getHub());

			uc.setName(Text::fromT(dlg.getName()));
			uc.setCommand(Text::fromT(dlg.getCommand()));
			uc.setHub(Text::fromT(dlg.getHub()));
			uc.setType(dlg.getType());
			uc.setCtx(dlg.getCtx());
			FavoriteManager::getInstance()->updateUserCommand(uc);
		}
	}
}

void UCPage::handleMoveUpClicked() {
	if(commands->getSelectedCount() == 1) {
		int i = commands->getSelectedIndex();
		if(i == 0)
			return;
		int n = commands->getItemData(i);
		FavoriteManager::getInstance()->moveUserCommand(n, -1);
#ifdef PORT_ME
		commands->SetRedraw(FALSE);
#endif
		commands->removeRow(i);
		UserCommand uc;
		FavoriteManager::getInstance()->getUserCommand(n, uc);
		addEntry(uc, --i);
		commands->setSelectedIndex(i);
#ifdef PORT_ME
		commands->EnsureVisible(i, FALSE);
		commands->SetRedraw(TRUE);
#endif
	}
}

void UCPage::handleMoveDownClicked() {
	if(commands->getSelectedCount() == 1) {
		int i = commands->getSelectedIndex();
		if(i == commands->getRowCount() - 1)
			return;
		int n = commands->getItemData(i);
		FavoriteManager::getInstance()->moveUserCommand(n, 1);
#ifdef PORT_ME
		commands->SetRedraw(FALSE);
#endif
		commands->removeRow(i);
		UserCommand uc;
		FavoriteManager::getInstance()->getUserCommand(n, uc);
		addEntry(uc, ++i);
		commands->setSelectedIndex(i);
#ifdef PORT_ME
		commands->EnsureVisible(i, FALSE);
		commands->SetRedraw(TRUE);
#endif
	}
}

void UCPage::handleRemoveClicked() {
	if(commands->getSelectedCount() == 1) {
		int i = commands->getSelectedIndex();
		FavoriteManager::getInstance()->removeUserCommand(commands->getItemData(i));
		commands->removeRow(i);
	}
}

void UCPage::addEntry(const UserCommand& uc, int index) {
	TStringList row;
	row.push_back((uc.getType() == UserCommand::TYPE_SEPARATOR) ? TSTRING(SEPARATOR) : Text::toT(uc.getName()));
	row.push_back(Text::toT(uc.getCommand()));
	row.push_back(Text::toT(uc.getHub()));
	commands->insertRow(row, (LPARAM)uc.getId(), index);
}

#ifdef PORT_ME

LRESULT UCPage::onHelpInfo(LPNMHDR /*pnmh*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_UCPAGE);
	return 0;
}

LRESULT UCPage::onHelp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_UCPAGE);
	return 0;
}
#endif

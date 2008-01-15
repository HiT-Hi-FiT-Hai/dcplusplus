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

#include "resource.h"

#include "UCPage.h"

#include <dcpp/SettingsManager.h>
#include <dcpp/FavoriteManager.h>
#include "CommandDlg.h"
#include "HoldRedraw.h"

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

	commands = attachList(IDC_MENU_ITEMS);
	commands->setListViewStyle(LVS_EX_LABELTIP | LVS_EX_FULLROWSELECT);

	TStringList columns;
	columns.push_back(T_("Name"));
	columns.push_back(T_("Command"));
	columns.push_back(T_("Hub"));
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

	commands->onDblClicked(std::tr1::bind(&UCPage::handleDoubleClick, this));
	commands->onKeyDown(std::tr1::bind(&UCPage::handleKeyDown, this, _1));

	attachButton(IDC_ADD_MENU)->onClicked(std::tr1::bind(&UCPage::handleAddClicked, this));

	attachButton(IDC_CHANGE_MENU)->onClicked(std::tr1::bind(&UCPage::handleChangeClicked, this));

	attachButton(IDC_MOVE_UP)->onClicked(std::tr1::bind(&UCPage::handleMoveUpClicked, this));

	attachButton(IDC_MOVE_DOWN)->onClicked(std::tr1::bind(&UCPage::handleMoveDownClicked, this));

	attachButton(IDC_REMOVE_MENU)->onClicked(std::tr1::bind(&UCPage::handleRemoveClicked, this));
}

UCPage::~UCPage() {
}

void UCPage::write() {
	PropPage::write(handle(), items);
}

void UCPage::handleDoubleClick() {
	if(commands->hasSelection()) {
		handleChangeClicked();
	} else {
		handleAddClicked();
	}
}

bool UCPage::handleKeyDown(int c) {
	switch(c) {
	case VK_INSERT:
		handleAddClicked();
		return true;
	case VK_DELETE:
		handleRemoveClicked();
		return true;
	}
	return false;
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
		FavoriteManager::getInstance()->getUserCommand(commands->getData(i), uc);

		CommandDlg dlg(this, uc.getType(), uc.getCtx(), Text::toT(uc.getName()), Text::toT(uc.getCommand()), Text::toT(uc.getHub()));
		if(dlg.run() == IDOK) {
			commands->setText(0, i, (dlg.getType() == UserCommand::TYPE_SEPARATOR) ? T_("Separator") : dlg.getName());
			commands->setText(1, i, dlg.getCommand());
			commands->setText(2, i, dlg.getHub());

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
		int n = commands->getData(i);
		FavoriteManager::getInstance()->moveUserCommand(n, -1);
		HoldRedraw hold(commands);
		commands->erase(i);
		UserCommand uc;
		FavoriteManager::getInstance()->getUserCommand(n, uc);
		addEntry(uc, --i);
		commands->setSelectedIndex(i);
		commands->ensureVisible(i);
	}
}

void UCPage::handleMoveDownClicked() {
	if(commands->getSelectedCount() == 1) {
		int i = commands->getSelectedIndex();
		if(i == commands->size() - 1)
			return;
		int n = commands->getData(i);
		FavoriteManager::getInstance()->moveUserCommand(n, 1);
		HoldRedraw hold(commands);
		commands->erase(i);
		UserCommand uc;
		FavoriteManager::getInstance()->getUserCommand(n, uc);
		addEntry(uc, ++i);
		commands->setSelectedIndex(i);
		commands->ensureVisible(i);
	}
}

void UCPage::handleRemoveClicked() {
	if(commands->getSelectedCount() == 1) {
		int i = commands->getSelectedIndex();
		FavoriteManager::getInstance()->removeUserCommand(commands->getData(i));
		commands->erase(i);
	}
}

void UCPage::addEntry(const UserCommand& uc, int index) {
	TStringList row;
	row.push_back((uc.getType() == UserCommand::TYPE_SEPARATOR) ? T_("Separator") : Text::toT(uc.getName()));
	row.push_back(Text::toT(uc.getCommand()));
	row.push_back(Text::toT(uc.getHub()));
	commands->insert(row, (LPARAM)uc.getId(), index);
}

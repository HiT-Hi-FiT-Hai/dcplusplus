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

#include "HubListsDlg.h"

#include <dcpp/FavoriteManager.h>
#include <dcpp/StringTokenizer.h>
#include "HoldRedraw.h"
#include "LineDlg.h"
#include "WinUtil.h"

static const WinUtil::HelpItem helpItems[] = {
	{ IDC_LIST_EDIT_BOX, IDH_PUBLIC_HUB_LISTS_EDIT_BOX },
	{ IDC_LIST_ADD, IDH_PUBLIC_HUB_LISTS_ADD },
	{ IDC_LIST_LIST, IDH_PUBLIC_HUB_LISTS_LIST },
	{ IDC_LIST_UP, IDH_PUBLIC_HUB_LISTS_MOVE_UP },
	{ IDC_LIST_DOWN, IDH_PUBLIC_HUB_LISTS_MOVE_DOWN },
	{ IDC_LIST_EDIT, IDH_PUBLIC_HUB_LISTS_EDIT },
	{ IDC_LIST_REMOVE, IDH_PUBLIC_HUB_LISTS_REMOVE },
	{ IDOK, IDH_DCPP_OK },
	{ IDCANCEL, IDH_DCPP_CANCEL },
	{ 0, 0 }
};

HubListsDlg::HubListsDlg(SmartWin::Widget* parent) :
	WidgetFactory<SmartWin::ModalDialog>(parent),
	editBox(0),
	hubLists(0)
{
	onInitDialog(std::tr1::bind(&HubListsDlg::handleInitDialog, this));
	onFocus(std::tr1::bind(&HubListsDlg::handleFocus, this));
}

HubListsDlg::~HubListsDlg() {
}

bool HubListsDlg::handleInitDialog() {
	setHelpId(IDH_PUBLIC_HUB_LISTS);

	WinUtil::setHelpIds(this, helpItems);

	setText(T_("Configured Public Hub Lists"));

	editBox = attachTextBox(IDC_LIST_EDIT_BOX);

	attachChild(hubLists, IDC_LIST_LIST);
	hubLists->setTableStyle(LVS_EX_LABELTIP | LVS_EX_FULLROWSELECT);

	TStringList columns;
	columns.push_back(Util::emptyStringT);
	hubLists->createColumns(columns);
	hubLists->setColumnWidth(0, hubLists->getSize().x - 20);

	StringList lists(FavoriteManager::getInstance()->getHubLists());
	for(StringIterC idx = lists.begin(); idx != lists.end(); ++idx)
		addHubList(Text::toT(*idx));

	hubLists->onDblClicked(std::tr1::bind(&HubListsDlg::handleDoubleClick, this));
	hubLists->onKeyDown(std::tr1::bind(&HubListsDlg::handleKeyDown, this, _1));

	ButtonPtr button = attachButton(IDC_LIST_ADD);
	button->setText(T_("&Add"));
	button->onClicked(std::tr1::bind(&HubListsDlg::handleAddClicked, this));

	button = attachButton(IDC_LIST_UP);
	button->setText(T_("Move &Up"));
	button->onClicked(std::tr1::bind(&HubListsDlg::handleMoveUpClicked, this));

	button = attachButton(IDC_LIST_DOWN);
	button->setText(T_("Move &Down"));
	button->onClicked(std::tr1::bind(&HubListsDlg::handleMoveDownClicked, this));

	button = attachButton(IDC_LIST_EDIT);
	button->setText(T_("&Edit"));
	button->onClicked(std::tr1::bind(&HubListsDlg::handleEditClicked, this));

	button = attachButton(IDC_LIST_REMOVE);
	button->setText(T_("&Remove"));
	button->onClicked(std::tr1::bind(&HubListsDlg::handleRemoveClicked, this));

	attachButton(IDOK)->onClicked(std::tr1::bind(&HubListsDlg::handleOKClicked, this));

	attachButton(IDCANCEL)->onClicked(std::tr1::bind(&HubListsDlg::endDialog, this, IDCANCEL));

	centerWindow();
	
	return false;
}

void HubListsDlg::handleFocus() {
	editBox->setFocus();
}

void HubListsDlg::handleDoubleClick() {
	if(hubLists->hasSelected()) {
		handleEditClicked();
	}
}

bool HubListsDlg::handleKeyDown(int c) {
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

void HubListsDlg::handleAddClicked() {
	StringTokenizer<tstring> t(editBox->getText(), ';');
	for(TStringList::reverse_iterator i = t.getTokens().rbegin(); i != t.getTokens().rend(); ++i)
		if(!i->empty())
			addHubList(*i);
}

void HubListsDlg::handleMoveUpClicked() {
	HoldRedraw hold(hubLists);
	std::vector<unsigned> selected = hubLists->getSelection();
	for(std::vector<unsigned>::const_iterator i = selected.begin(); i != selected.end(); ++i) {
		if(*i > 0) {
			tstring selText = hubLists->getText(*i, 0);
			hubLists->erase(*i);
			addHubList(selText, *i - 1);
			hubLists->select(*i - 1);
		}
	}
}

void HubListsDlg::handleMoveDownClicked() {
	HoldRedraw hold(hubLists);
	std::vector<unsigned> selected = hubLists->getSelection();
	for(std::vector<unsigned>::reverse_iterator i = selected.rbegin(); i != selected.rend(); ++i) {
		if(*i < hubLists->size() - 1) {
			tstring selText = hubLists->getText(*i, 0);
			hubLists->erase(*i);
			addHubList(selText, *i + 1);
			hubLists->select(*i + 1);
		}
	}
}

void HubListsDlg::handleEditClicked() {
	int i = -1;
	while((i = hubLists->getNext(i, LVNI_SELECTED)) != -1) {
		LineDlg dlg(this, T_("Hublist"), T_("Edit the hublist"), hubLists->getText(i, 0));
		if(dlg.run() == IDOK)
			hubLists->setText(i, 0, dlg.getLine());
	}
}

void HubListsDlg::handleRemoveClicked() {
	int i = -1;
	while((i = hubLists->getNext(-1, LVNI_SELECTED)) != -1)
		hubLists->erase(i);
}

void HubListsDlg::handleOKClicked() {
	tstring tmp;
	int j = hubLists->size();
	for(int i = 0; i < j; ++i) {
		if(i != 0)
			tmp += ';';
		tmp += hubLists->getText(i, 0);
	}
	SettingsManager::getInstance()->set(SettingsManager::HUBLIST_SERVERS, Text::fromT(tmp));
	endDialog(IDOK);
}

void HubListsDlg::addHubList(const tstring& address, int index) {
	TStringList row;
	row.push_back(address);
	int itemCount = hubLists->insert(row, 0, index);
	if(index == -1)
		index = itemCount;
	hubLists->ensureVisible(index);
}

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

#include "PropPage.h"

#include <dcpp/SettingsManager.h>
#include "WinUtil.h"

PropPage::PropPage(dwt::Widget* parent) : WidgetFactory<dwt::ModelessDialog>(parent) {
}

PropPage::~PropPage() {
}

void PropPage::read(HWND page, const Item* items) {
	dcassert(page && items);
	SettingsManager* settings = SettingsManager::getInstance();
	for(Item const* i = items; i->type != T_END; i++)
	{
		switch(i->type)
		{
		case T_STR:
			if(!settings->isDefault(i->setting)) {
				::SetDlgItemText(page, i->itemID,
					Text::toT(settings->get((SettingsManager::StrSetting)i->setting, true)).c_str());
			}
			break;
		case T_INT:
			if(!settings->isDefault(i->setting)) {
				::SetDlgItemInt(page, i->itemID,
					settings->get((SettingsManager::IntSetting)i->setting, true), FALSE);
			}
			break;
		case T_BOOL:
			if(settings->getBool((SettingsManager::IntSetting)i->setting, true))
				::CheckDlgButton(page, i->itemID, BST_CHECKED);
			else
				::CheckDlgButton(page, i->itemID, BST_UNCHECKED);
		}
	}
}

void PropPage::read(const ListItem* listItems, TablePtr list) {
	dcassert(listItems && list);
	initList(list);
	SettingsManager* settings = SettingsManager::getInstance();
	for(size_t i = 0; listItems[i].setting != 0; ++i) {
		TStringList row;
		row.push_back(T_(listItems[i].desc));
		list->setChecked(list->insert(row), settings->getBool(SettingsManager::IntSetting(listItems[i].setting), true));
	}
	list->setColumnWidth(0, LVSCW_AUTOSIZE);
}

void PropPage::initList(TablePtr list) {
	list->setTableStyle(LVS_EX_LABELTIP | LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);

	TStringList dummy;
	dummy.push_back(Util::emptyStringT);
	list->createColumns(dummy);
}

void PropPage::write(HWND page, const Item* items) {
	dcassert(page && items);
	SettingsManager* settings = SettingsManager::getInstance();
	tstring buf;
	for(Item const* i = items; i->type != T_END; i++)
	{
		switch(i->type)
		{
		case T_STR:
			{
				buf.resize(SETTINGS_BUF_LEN);
				buf.resize(::GetDlgItemText(page, i->itemID, &buf[0], buf.size()));
				settings->set((SettingsManager::StrSetting)i->setting, Text::fromT(buf));

				break;
			}
		case T_INT:
			{
				buf.resize(SETTINGS_BUF_LEN);
				buf.resize(::GetDlgItemText(page, i->itemID, &buf[0], buf.size()));
				settings->set((SettingsManager::IntSetting)i->setting, Text::fromT(buf));
				break;
			}
		case T_BOOL:
			{
				if(::IsDlgButtonChecked(page, i->itemID) == BST_CHECKED)
					settings->set((SettingsManager::IntSetting)i->setting, true);
				else
					settings->set((SettingsManager::IntSetting)i->setting, false);
			}
		}
	}
}

void PropPage::write(const ListItem* listItems, TablePtr list) {
	dcassert(listItems && list);
	SettingsManager* settings = SettingsManager::getInstance();
	for(size_t i = 0; listItems[i].setting != 0; ++i)
		settings->set(SettingsManager::IntSetting(listItems[i].setting), list->isChecked(i));
}

void PropPage::translate(HWND page, TextItem* items) {
	if(items)
		for(size_t i = 0; items[i].itemID != 0; ++i)
			::SetDlgItemText(page, items[i].itemID, CT_(items[i].stringToTranslate));
}

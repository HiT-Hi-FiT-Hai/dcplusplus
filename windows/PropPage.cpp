/* 
 * Copyright (C) 2001-2005 Jacek Sieka, arnetheduck on gmail point com
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

#include "PropPage.h"

#include "../client/SettingsManager.h"
#include "WinUtil.h"

#define SETTING_STR_MAXLEN 1024

void PropPage::read(HWND page, Item const* items, ListItem* listItems /* = NULL */, HWND list /* = 0 */)
{
	dcassert(page != NULL);

	bool const useDef = true;
	for(Item const* i = items; i->type != T_END; i++)
	{
		switch(i->type)
		{
		case T_STR:
			if(!SettingsManager::getInstance()->isDefault(i->setting)) {
				::SetDlgItemText(page, i->itemID,
					Text::toT(settings->get((SettingsManager::StrSetting)i->setting, useDef)).c_str());
			}
			break;
		case T_INT:
			if(!SettingsManager::getInstance()->isDefault(i->setting)) {
				::SetDlgItemInt(page, i->itemID,
					settings->get((SettingsManager::IntSetting)i->setting, useDef), FALSE);
			}
			break;
		case T_BOOL:
			if(settings->getBool((SettingsManager::IntSetting)i->setting, useDef))
				::CheckDlgButton(page, i->itemID, BST_CHECKED);
			else
				::CheckDlgButton(page, i->itemID, BST_UNCHECKED);
		}
	}

	if(listItems != NULL) {
		CListViewCtrl ctrl;

		ctrl.Attach(list);
		CRect rc;
		ctrl.GetClientRect(rc);
		ctrl.SetExtendedListViewStyle(LVS_EX_LABELTIP | LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);
		ctrl.InsertColumn(0, _T("Dummy"), LVCFMT_LEFT, rc.Width(), 0);

		LVITEM lvi;
		lvi.mask = LVIF_TEXT;
		lvi.iSubItem = 0;

		for(int i = 0; listItems[i].setting != 0; i++) {
			lvi.iItem = i;
			lvi.pszText = const_cast<TCHAR*>(CTSTRING_I(listItems[i].desc));
			ctrl.InsertItem(&lvi);
			ctrl.SetCheckState(i, SettingsManager::getInstance()->getBool(SettingsManager::IntSetting(listItems[i].setting), true));
		}
		ctrl.SetColumnWidth(0, LVSCW_AUTOSIZE);
		ctrl.Detach();
	}
}

void PropPage::write(HWND page, Item const* items, ListItem* listItems /* = NULL */, HWND list /* = NULL */)
{
	dcassert(page != NULL);

	AutoArray<TCHAR> buf(SETTING_STR_MAXLEN);
	for(Item const* i = items; i->type != T_END; i++)
	{
		switch(i->type)
		{
		case T_STR:
			{
				::GetDlgItemText(page, i->itemID, buf, SETTING_STR_MAXLEN);
				settings->set((SettingsManager::StrSetting)i->setting, Text::fromT(tstring(buf)));

				break;
			}
		case T_INT:
			{
				::GetDlgItemText(page, i->itemID, buf, SETTING_STR_MAXLEN);
				settings->set((SettingsManager::IntSetting)i->setting, Text::fromT(tstring(buf)));
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

	if(listItems != NULL) {
		CListViewCtrl ctrl;
		ctrl.Attach(list);

		int i;
		for(i = 0; listItems[i].setting != 0; i++) {
			SettingsManager::getInstance()->set(SettingsManager::IntSetting(listItems[i].setting), ctrl.GetCheckState(i));
		}

		ctrl.Detach();
	}
}

void PropPage::translate(HWND page, TextItem* textItems) 
{
	if (textItems != NULL) {
		for(int i = 0; textItems[i].itemID != 0; i++) {
			::SetDlgItemText(page, textItems[i].itemID,
				CTSTRING_I(textItems[i].translatedString));
		}
	}
}

/**
 * @file
 * $Id: PropPage.cpp,v 1.13 2005/04/10 21:23:28 arnetheduck Exp $
 */


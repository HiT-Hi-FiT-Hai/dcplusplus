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

#include "AppearancePage.h"

#include <dcpp/SettingsManager.h>
#include "WinUtil.h"

PropPage::TextItem AppearancePage::texts[] = {
	{ IDC_SETTINGS_APPEARANCE_OPTIONS, ResourceManager::SETTINGS_OPTIONS },
	{ IDC_SETTINGS_DEFAULT_AWAY_MSG, ResourceManager::SETTINGS_DEFAULT_AWAY_MSG },
	{ IDC_SETTINGS_TIME_STAMPS_FORMAT, ResourceManager::SETTINGS_TIME_STAMPS_FORMAT },
	{ IDC_SETTINGS_LANGUAGE_FILE, ResourceManager::SETTINGS_LANGUAGE_FILE },
	{ IDC_BROWSE, ResourceManager::BROWSE_ACCEL },
	{ IDC_SETTINGS_REQUIRES_RESTART, ResourceManager::SETTINGS_REQUIRES_RESTART },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

PropPage::Item AppearancePage::items[] = {
	{ IDC_DEFAULT_AWAY_MESSAGE, SettingsManager::DEFAULT_AWAY_MESSAGE, PropPage::T_STR },
	{ IDC_TIME_STAMPS_FORMAT, SettingsManager::TIME_STAMPS_FORMAT, PropPage::T_STR },
	{ IDC_LANGUAGE, SettingsManager::LANGUAGE_FILE, PropPage::T_STR },
	{ 0, 0, PropPage::T_END }
};

PropPage::ListItem AppearancePage::listItems[] = {
	{ SettingsManager::ALT_SORT_ORDER, ResourceManager::SETTINGS_ALT_SORT_ORDER },
	{ SettingsManager::FILTER_MESSAGES, ResourceManager::SETTINGS_FILTER_MESSAGES },
	{ SettingsManager::MINIMIZE_TRAY, ResourceManager::SETTINGS_MINIMIZE_TRAY },
	{ SettingsManager::TIME_STAMPS, ResourceManager::SETTINGS_TIME_STAMPS },
	{ SettingsManager::STATUS_IN_CHAT, ResourceManager::SETTINGS_STATUS_IN_CHAT },
	{ SettingsManager::SHOW_JOINS, ResourceManager::SETTINGS_SHOW_JOINS },
	{ SettingsManager::FAV_SHOW_JOINS, ResourceManager::SETTINGS_FAV_SHOW_JOINS },
	{ SettingsManager::SORT_FAVUSERS_FIRST, ResourceManager::SETTINGS_SORT_FAVUSERS_FIRST },
	{ SettingsManager::USE_SYSTEM_ICONS, ResourceManager::SETTINGS_USE_SYSTEM_ICONS },
	{ SettingsManager::USE_OEM_MONOFONT, ResourceManager::SETTINGS_USE_OEM_MONOFONT },
	{ SettingsManager::GET_USER_COUNTRY, ResourceManager::SETTINGS_GET_USER_COUNTRY },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

AppearancePage::AppearancePage(SmartWin::Widget* parent) : PropPage(parent) {
	createDialog(IDD_APPEARANCEPAGE);

	PropPage::translate(handle(), texts);
	PropPage::read(handle(), items, listItems, ::GetDlgItem(handle(), IDC_APPEARANCE_BOOLEANS));

	attachButton(IDC_BROWSE)->onClicked(std::tr1::bind(&AppearancePage::handleBrowse, this));
}

AppearancePage::~AppearancePage() {
}

void AppearancePage::write()
{
	PropPage::write(handle(), items, listItems, ::GetDlgItem(handle(), IDC_APPEARANCE_BOOLEANS));
}

void AppearancePage::handleBrowse() {
	TCHAR buf[MAX_PATH];
	static const TCHAR types[] = _T("Language Files\0*.xml\0All Files\0*.*\0");

	::GetDlgItemText(handle(), IDC_LANGUAGE, buf, MAX_PATH);
	tstring x = buf;

	if(WinUtil::browseFile(x, handle(), false, Text::toT(Util::getDataPath()), types) == IDOK) {
		::SetDlgItemText(handle(), IDC_LANGUAGE, x.c_str());
	}
}

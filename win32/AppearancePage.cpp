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
#include <dcpp/File.h>

#include "WinUtil.h"

PropPage::TextItem AppearancePage::texts[] = {
#ifdef PORT_ME
	{ IDC_SETTINGS_APPEARANCE_OPTIONS, ResourceManager::SETTINGS_OPTIONS },
	{ IDC_SETTINGS_DEFAULT_AWAY_MSG, ResourceManager::SETTINGS_DEFAULT_AWAY_MSG },
	{ IDC_SETTINGS_TIME_STAMPS_FORMAT, ResourceManager::SETTINGS_TIME_STAMPS_FORMAT },
	{ IDC_SETTINGS_LANGUAGE_FILE, ResourceManager::SETTINGS_LANGUAGE_FILE },
	{ IDC_SETTINGS_REQUIRES_RESTART, ResourceManager::SETTINGS_REQUIRES_RESTART },
#endif
	{ 0, 0 }
};

PropPage::Item AppearancePage::items[] = {
	{ IDC_DEFAULT_AWAY_MESSAGE, SettingsManager::DEFAULT_AWAY_MESSAGE, PropPage::T_STR },
	{ IDC_TIME_STAMPS_FORMAT, SettingsManager::TIME_STAMPS_FORMAT, PropPage::T_STR },
	{ 0, 0, PropPage::T_END }
};

PropPage::ListItem AppearancePage::listItems[] = {
#ifdef PORT_ME
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
#endif
	{ 0, 0 }
};

AppearancePage::AppearancePage(SmartWin::Widget* parent) : PropPage(parent), languages(0) {
	createDialog(IDD_APPEARANCEPAGE);

	PropPage::translate(handle(), texts);
	PropPage::read(handle(), items, listItems, ::GetDlgItem(handle(), IDC_APPEARANCE_BOOLEANS));

	languages = attachComboBox(IDC_LANGUAGE);

	StringList dirs = File::findFiles(Util::getLocalePath(), "*");

	TStringList langs;
	
	langs.push_back(_T("en"));
	
	for(StringList::const_iterator i = dirs.begin(); i != dirs.end(); ++i) {
		string dir = *i + "LC_MESSAGES" PATH_SEPARATOR_STR;
		StringList files = File::findFiles(dir, "*.mo");
		if(find(files.begin(), files.end(), dir + "dcpp.mo") == files.end() && find(files.begin(), files.end(), dir + "dcpp-win32.mo") == files.end()) {
			continue;
		}
		// TODO Convert to real language name?
		langs.push_back(Text::toT(Util::getLastDir(*i)));
	}
	
	std::sort(langs.begin(), langs.end(), noCaseStringLess());
	
	languages->addValue(T_("Default"));
	
	int selected = 0, j = 1;
	const tstring cur = Text::toT(SETTING(LANGUAGE));
	for(TStringList::const_iterator i = langs.begin(); i != langs.end(); ++i, ++j) {
		languages->addValue(*i);
		if(selected != 0 && *i == cur || (*i == _T("en") && cur == _T("C"))) {
			selected = j;
		}
	}
	
	languages->setSelectedIndex(selected);
}

AppearancePage::~AppearancePage() {
}

void AppearancePage::write()
{
	PropPage::write(handle(), items, listItems, ::GetDlgItem(handle(), IDC_APPEARANCE_BOOLEANS));

	tstring lang = languages->getText();
	
	if(lang == T_("Default")) {
		SettingsManager::getInstance()->set(SettingsManager::LANGUAGE, "");
	} else if(lang == _T("en")) {
		SettingsManager::getInstance()->set(SettingsManager::LANGUAGE, "C");
	} else {
		SettingsManager::getInstance()->set(SettingsManager::LANGUAGE, Text::fromT(lang));
	}
}

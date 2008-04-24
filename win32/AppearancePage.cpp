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

#include "AppearancePage.h"

#include <dcpp/SettingsManager.h>
#include <dcpp/File.h>

#include "WinUtil.h"

static const WinUtil::HelpItem helpItems[] = {
	{ IDC_SETTINGS_DEFAULT_AWAY_MSG, IDH_SETTINGS_APPEARANCE_DEFAULT_AWAY_MESSAGE },
	{ IDC_DEFAULT_AWAY_MESSAGE, IDH_SETTINGS_APPEARANCE_DEFAULT_AWAY_MESSAGE },
	{ IDC_SETTINGS_TIME_STAMPS_FORMAT, IDH_SETTINGS_APPEARANCE_TIME_STAMPS_FORMAT },
	{ IDC_TIME_STAMPS_FORMAT, IDH_SETTINGS_APPEARANCE_TIME_STAMPS_FORMAT },
	{ IDC_SETTINGS_LANGUAGE, IDH_SETTINGS_APPEARANCE_LANGUAGE },
	{ IDC_LANGUAGE, IDH_SETTINGS_APPEARANCE_LANGUAGE },
	{ IDC_SETTINGS_REQUIRES_RESTART, IDH_SETTINGS_APPEARANCE_REQUIRES_RESTART },
	{ 0, 0 }
};

PropPage::TextItem AppearancePage::texts[] = {
	{ IDC_SETTINGS_APPEARANCE_OPTIONS, N_("Options") },
	{ IDC_SETTINGS_DEFAULT_AWAY_MSG, N_("Default away message") },
	{ IDC_SETTINGS_TIME_STAMPS_FORMAT, N_("Set timestamps") },
	{ IDC_SETTINGS_LANGUAGE, N_("Language") },
	{ IDC_SETTINGS_REQUIRES_RESTART, N_("Note; most of these options require that you restart DC++") },
	{ 0, 0 }
};

PropPage::Item AppearancePage::items[] = {
	{ IDC_DEFAULT_AWAY_MESSAGE, SettingsManager::DEFAULT_AWAY_MESSAGE, PropPage::T_STR },
	{ IDC_TIME_STAMPS_FORMAT, SettingsManager::TIME_STAMPS_FORMAT, PropPage::T_STR },
	{ 0, 0, PropPage::T_END }
};

PropPage::ListItem AppearancePage::listItems[] = {
	{ SettingsManager::ALT_SORT_ORDER, N_("Sort all downloads first"), IDH_SETTINGS_APPEARANCE_ALT_SORT_ORDER },
	{ SettingsManager::FILTER_MESSAGES, N_("Filter kick messages"), IDH_SETTINGS_APPEARANCE_FILTER_MESSAGES },
	{ SettingsManager::MINIMIZE_TRAY, N_("Minimize to tray"), IDH_SETTINGS_APPEARANCE_MINIMIZE_TRAY },
	{ SettingsManager::TIME_STAMPS, N_("Show timestamps in chat by default"), IDH_SETTINGS_APPEARANCE_TIME_STAMPS },
	{ SettingsManager::STATUS_IN_CHAT, N_("View status messages in main chat"), IDH_SETTINGS_APPEARANCE_STATUS_IN_CHAT },
	{ SettingsManager::SHOW_JOINS, N_("Show joins / parts in chat by default"), IDH_SETTINGS_APPEARANCE_SHOW_JOINS },
	{ SettingsManager::FAV_SHOW_JOINS, N_("Only show joins / parts for favorite users"), IDH_SETTINGS_APPEARANCE_FAV_SHOW_JOINS },
	{ SettingsManager::SORT_FAVUSERS_FIRST, N_("Sort favorite users first"), IDH_SETTINGS_APPEARANCE_SORT_FAVUSERS_FIRST },
	{ SettingsManager::USE_SYSTEM_ICONS, N_("Use system icons when browsing files (slows browsing down a bit)"), IDH_SETTINGS_APPEARANCE_USE_SYSTEM_ICONS },
	{ SettingsManager::USE_OEM_MONOFONT, N_("Use OEM monospaced font for viewing text files"), IDH_SETTINGS_APPEARANCE_USE_OEM_MONOFONT },
	{ SettingsManager::GET_USER_COUNTRY, N_("Guess user country from IP"), IDH_SETTINGS_APPEARANCE_GET_USER_COUNTRY },
	{ 0, 0 }
};

AppearancePage::AppearancePage(dwt::Widget* parent) : PropPage(parent), languages(0) {
	createDialog(IDD_APPEARANCEPAGE);
	setHelpId(IDH_APPEARANCEPAGE);

	WinUtil::setHelpIds(this, helpItems);
	PropPage::translate(handle(), texts);
	PropPage::read(handle(), items);

	attachChild(options, IDC_APPEARANCE_BOOLEANS);
	PropPage::read(listItems, options);

	attachChild(languages, IDC_LANGUAGE);

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
		
		if(selected == 0 && (*i == cur || (*i == _T("en") && cur == _T("C")))) {
			selected = j;
		}
	}
	
	languages->setSelected(selected);

	attachChild<TextBox>(IDC_DEFAULT_AWAY_MESSAGE);
	attachChild<TextBox>(IDC_TIME_STAMPS_FORMAT);
}

AppearancePage::~AppearancePage() {
}

void AppearancePage::write()
{
	PropPage::write(handle(), items);
	PropPage::write(listItems, options);

	tstring lang = languages->getText();
	
	if(lang == T_("Default")) {
		SettingsManager::getInstance()->set(SettingsManager::LANGUAGE, "");
	} else if(lang == _T("en")) {
		SettingsManager::getInstance()->set(SettingsManager::LANGUAGE, "C");
	} else {
		SettingsManager::getInstance()->set(SettingsManager::LANGUAGE, Text::fromT(lang));
	}
}

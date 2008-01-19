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

#include "AdvancedPage.h"

#include <dcpp/SettingsManager.h>

PropPage::Item AdvancedPage::items[] = { { 0, 0, PropPage::T_END } };

AdvancedPage::ListItem AdvancedPage::listItems[] = {
#ifdef PORT_ME
	{ SettingsManager::AUTO_AWAY, ResourceManager::SETTINGS_AUTO_AWAY },
	{ SettingsManager::AUTO_FOLLOW, ResourceManager::SETTINGS_AUTO_FOLLOW },
	{ SettingsManager::CLEAR_SEARCH, ResourceManager::SETTINGS_CLEAR_SEARCH },
	{ SettingsManager::LIST_DUPES, ResourceManager::SETTINGS_LIST_DUPES },
	{ SettingsManager::URL_HANDLER, ResourceManager::SETTINGS_URL_HANDLER },
	{ SettingsManager::MAGNET_REGISTER, ResourceManager::SETTINGS_URL_MAGNET },
	{ SettingsManager::KEEP_LISTS, ResourceManager::SETTINGS_KEEP_LISTS },
	{ SettingsManager::AUTO_KICK, ResourceManager::SETTINGS_AUTO_KICK },
	{ SettingsManager::SHOW_PROGRESS_BARS, ResourceManager::SETTINGS_SHOW_PROGRESS_BARS },
	{ SettingsManager::SFV_CHECK, ResourceManager::SETTINGS_SFV_CHECK },
	{ SettingsManager::NO_AWAYMSG_TO_BOTS, ResourceManager::SETTINGS_NO_AWAYMSG_TO_BOTS },
	{ SettingsManager::ADLS_BREAK_ON_FIRST, ResourceManager::SETTINGS_ADLS_BREAK_ON_FIRST },
	{ SettingsManager::COMPRESS_TRANSFERS, ResourceManager::SETTINGS_COMPRESS_TRANSFERS },
	{ SettingsManager::HUB_USER_COMMANDS, ResourceManager::SETTINGS_HUB_USER_COMMANDS },
	{ SettingsManager::SEND_UNKNOWN_COMMANDS, ResourceManager::SETTINGS_SEND_UNKNOWN_COMMANDS },
	{ SettingsManager::ADD_FINISHED_INSTANTLY, ResourceManager::SETTINGS_ADD_FINISHED_INSTANTLY },
	{ SettingsManager::USE_CTRL_FOR_LINE_HISTORY, ResourceManager::SETTINGS_USE_CTRL_FOR_LINE_HISTORY },
	{ SettingsManager::AUTO_KICK_NO_FAVS, ResourceManager::SETTINGS_AUTO_KICK_NO_FAVS },
	{ SettingsManager::SHOW_SHELL_MENU, ResourceManager::SETTINGS_SHOW_SHELL_MENU },
#endif
	{ 0, 0 }
};

AdvancedPage::AdvancedPage(SmartWin::Widget* parent) : PropPage(parent) {
	createDialog(IDD_ADVANCEDPAGE);

	PropPage::read(handle(), items, listItems, ::GetDlgItem(handle(), IDC_ADVANCED_BOOLEANS));
}

AdvancedPage::~AdvancedPage() {
}

void AdvancedPage::write() {
	PropPage::write(handle(), items, listItems, ::GetDlgItem(handle(), IDC_ADVANCED_BOOLEANS));
}

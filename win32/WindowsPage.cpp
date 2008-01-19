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

#include "WindowsPage.h"

#include <dcpp/SettingsManager.h>

PropPage::Item WindowsPage::items[] = { { 0, 0, PropPage::T_END } };

PropPage::TextItem WindowsPage::textItem[] = {
#ifdef PORT_ME
	{ IDC_SETTINGS_AUTO_OPEN, ResourceManager::SETTINGS_AUTO_OPEN },
	{ IDC_SETTINGS_WINDOWS_OPTIONS, ResourceManager::SETTINGS_WINDOWS_OPTIONS },
	{ IDC_SETTINGS_CONFIRM_OPTIONS, ResourceManager::SETTINGS_CONFIRM_DIALOG_OPTIONS },
#endif
	{ 0, 0 }
};

WindowsPage::ListItem WindowsPage::listItems[] = {
#ifdef PORT_ME
	{ SettingsManager::OPEN_SYSTEM_LOG, ResourceManager::SYSTEM_LOG },
	{ SettingsManager::OPEN_FAVORITE_USERS, ResourceManager::FAVORITE_USERS },
	{ SettingsManager::OPEN_QUEUE, ResourceManager::DOWNLOAD_QUEUE },
	{ SettingsManager::OPEN_FINISHED_DOWNLOADS, ResourceManager::FINISHED_DOWNLOADS },
	{ SettingsManager::OPEN_WAITING_USERS, ResourceManager::WAITING_USERS },
	{ SettingsManager::OPEN_FINISHED_UPLOADS, ResourceManager::FINISHED_UPLOADS },
	{ SettingsManager::OPEN_SEARCH_SPY, ResourceManager::SEARCH_SPY },
	{ SettingsManager::OPEN_NETWORK_STATISTICS, ResourceManager::NETWORK_STATISTICS },
	{ SettingsManager::OPEN_NOTEPAD, ResourceManager::NOTEPAD },
	{ SettingsManager::OPEN_PUBLIC, ResourceManager::PUBLIC_HUBS },
	{ SettingsManager::OPEN_FAVORITE_HUBS, ResourceManager::FAVORITE_HUBS },
	{ SettingsManager::OPEN_DOWNLOADS, ResourceManager::DOWNLOADS },
#endif
	{ 0, 0 }
};

WindowsPage::ListItem WindowsPage::optionItems[] = {
#ifdef PORT_ME
	{ SettingsManager::POPUP_PMS, ResourceManager::SETTINGS_POPUP_PMS },
	{ SettingsManager::POPUP_HUB_PMS, ResourceManager::SETTINGS_POPUP_HUB_PMS },
	{ SettingsManager::POPUP_BOT_PMS, ResourceManager::SETTINGS_POPUP_BOT_PMS },
	{ SettingsManager::POPUNDER_FILELIST, ResourceManager::SETTINGS_POPUNDER_FILELIST },
	{ SettingsManager::POPUNDER_PM, ResourceManager::SETTINGS_POPUNDER_PM },
	{ SettingsManager::JOIN_OPEN_NEW_WINDOW, ResourceManager::SETTINGS_OPEN_NEW_WINDOW },
	{ SettingsManager::IGNORE_HUB_PMS, ResourceManager::SETTINGS_IGNORE_HUB_PMS },
	{ SettingsManager::IGNORE_BOT_PMS, ResourceManager::SETTINGS_IGNORE_BOT_PMS },
	{ SettingsManager::TOGGLE_ACTIVE_WINDOW, ResourceManager::SETTINGS_TOGGLE_ACTIVE_WINDOW },
	{ SettingsManager::PROMPT_PASSWORD, ResourceManager::SETTINGS_PROMPT_PASSWORD },
#endif
	{ 0, 0 }
};

WindowsPage::ListItem WindowsPage::confirmItems[] = {
#ifdef PORT_ME
	{ SettingsManager::CONFIRM_EXIT, ResourceManager::SETTINGS_CONFIRM_EXIT },
	{ SettingsManager::CONFIRM_HUB_REMOVAL, ResourceManager::SETTINGS_CONFIRM_HUB_REMOVAL },
	{ SettingsManager::CONFIRM_ITEM_REMOVAL, ResourceManager::SETTINGS_CONFIRM_ITEM_REMOVAL },
#endif
	{ 0, 0 }
};

WindowsPage::WindowsPage(SmartWin::Widget* parent) : PropPage(parent) {
	createDialog(IDD_WINDOWSPAGE);

	PropPage::translate(handle(), textItem);
	PropPage::read(handle(), items, listItems, ::GetDlgItem(handle(), IDC_WINDOWS_STARTUP));
	PropPage::read(handle(), items, optionItems, ::GetDlgItem(handle(), IDC_WINDOWS_OPTIONS));
	PropPage::read(handle(), items, confirmItems, ::GetDlgItem(handle(), IDC_CONFIRM_OPTIONS));
}

WindowsPage::~WindowsPage() {
}

void WindowsPage::write() {
	PropPage::write(handle(), items, listItems, ::GetDlgItem(handle(), IDC_WINDOWS_STARTUP));
	PropPage::write(handle(), items, optionItems, ::GetDlgItem(handle(), IDC_WINDOWS_OPTIONS));
	PropPage::write(handle(), items, confirmItems, ::GetDlgItem(handle(), IDC_CONFIRM_OPTIONS));
}

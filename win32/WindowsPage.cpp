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

#include "WindowsPage.h"

#include <dcpp/SettingsManager.h>

PropPage::TextItem WindowsPage::textItem[] = {
	{ IDC_SETTINGS_AUTO_OPEN, N_("Auto-open at startup") },
	{ IDC_SETTINGS_WINDOWS_OPTIONS, N_("Window options") },
	{ IDC_SETTINGS_CONFIRM_OPTIONS, N_("Confirm dialog options") },
	{ 0, 0 }
};

WindowsPage::ListItem WindowsPage::autoOpenItems[] = {
	{ SettingsManager::OPEN_SYSTEM_LOG, N_("System Log"), IDH_SETTINGS_WINDOWS_OPEN_SYSTEM_LOG },
	{ SettingsManager::OPEN_FAVORITE_USERS, N_("Favorite Users"), IDH_SETTINGS_WINDOWS_OPEN_FAVORITE_USERS },
	{ SettingsManager::OPEN_QUEUE, N_("Download Queue"), IDH_SETTINGS_WINDOWS_OPEN_QUEUE },
	{ SettingsManager::OPEN_FINISHED_DOWNLOADS, N_("Finished Downloads"), IDH_SETTINGS_WINDOWS_OPEN_FINISHED_DOWNLOADS },
	{ SettingsManager::OPEN_WAITING_USERS, N_("Waiting Users"), IDH_SETTINGS_WINDOWS_OPEN_WAITING_USERS },
	{ SettingsManager::OPEN_FINISHED_UPLOADS, N_("Finished Uploads"), IDH_SETTINGS_WINDOWS_OPEN_FINISHED_UPLOADS },
	{ SettingsManager::OPEN_SEARCH_SPY, N_("Search Spy"), IDH_SETTINGS_WINDOWS_OPEN_SEARCH_SPY },
	{ SettingsManager::OPEN_NETWORK_STATISTICS, N_("Network Statistics"), IDH_SETTINGS_WINDOWS_OPEN_NETWORK_STATISTICS },
	{ SettingsManager::OPEN_NOTEPAD, N_("Notepad"), IDH_SETTINGS_WINDOWS_OPEN_NOTEPAD },
	{ SettingsManager::OPEN_PUBLIC, N_("Public Hubs"), IDH_SETTINGS_WINDOWS_OPEN_PUBLIC },
	{ SettingsManager::OPEN_FAVORITE_HUBS, N_("Favorite Hubs"), IDH_SETTINGS_WINDOWS_OPEN_FAVORITE_HUBS },
	{ 0, 0 }
};

WindowsPage::ListItem WindowsPage::optionItems[] = {
	{ SettingsManager::POPUP_PMS, N_("Open private messages in their own window"), IDH_SETTINGS_WINDOWS_POPUP_PMS },
	{ SettingsManager::POPUP_HUB_PMS, N_("Open private messages from bots in their own window"), IDH_SETTINGS_WINDOWS_POPUP_HUB_PMS },
	{ SettingsManager::POPUP_BOT_PMS, N_("Open private messages from the hub in their own window"), IDH_SETTINGS_WINDOWS_POPUP_BOT_PMS },
	{ SettingsManager::POPUNDER_FILELIST, N_("Open new file list windows in the background"), IDH_SETTINGS_WINDOWS_POPUNDER_FILELIST },
	{ SettingsManager::POPUNDER_PM, N_("Open new private message windows in the background"), IDH_SETTINGS_WINDOWS_POPUNDER_PM },
	{ SettingsManager::JOIN_OPEN_NEW_WINDOW, N_("Open new window when using /join"), IDH_SETTINGS_WINDOWS_JOIN_OPEN_NEW_WINDOW },
	{ SettingsManager::IGNORE_HUB_PMS, N_("Ignore private messages from the hub"), IDH_SETTINGS_WINDOWS_IGNORE_HUB_PMS },
	{ SettingsManager::IGNORE_BOT_PMS, N_("Ignore private messages from bots"), IDH_SETTINGS_WINDOWS_IGNORE_BOT_PMS },
	{ SettingsManager::TOGGLE_ACTIVE_WINDOW, N_("Toggle window when selecting an active tab"), IDH_SETTINGS_WINDOWS_TOGGLE_ACTIVE_WINDOW },
	{ SettingsManager::PROMPT_PASSWORD, N_("Popup box to input password for hubs"), IDH_SETTINGS_WINDOWS_PROMPT_PASSWORD },
	{ 0, 0 }
};

WindowsPage::ListItem WindowsPage::confirmItems[] = {
	{ SettingsManager::CONFIRM_EXIT, N_("Confirm application exit"), IDH_SETTINGS_WINDOWS_CONFIRM_EXIT },
	{ SettingsManager::CONFIRM_HUB_REMOVAL, N_("Confirm favorite hub removal"), IDH_SETTINGS_WINDOWS_CONFIRM_HUB_REMOVAL },
	{ SettingsManager::CONFIRM_ITEM_REMOVAL, N_("Confirm item removal in download queue"), IDH_SETTINGS_WINDOWS_CONFIRM_ITEM_REMOVAL },
	{ 0, 0 }
};

WindowsPage::WindowsPage(dwt::Widget* parent) : PropPage(parent) {
	createDialog(IDD_WINDOWSPAGE);
	setHelpId(IDH_WINDOWSPAGE);

	PropPage::translate(handle(), textItem);

	attachChild(autoOpen, IDC_WINDOWS_STARTUP);
	PropPage::read(autoOpenItems, autoOpen);

	attachChild(options, IDC_WINDOWS_OPTIONS);
	PropPage::read(optionItems, options);

	attachChild(confirm, IDC_CONFIRM_OPTIONS);
	PropPage::read(confirmItems, confirm);
}

WindowsPage::~WindowsPage() {
}

void WindowsPage::write() {
	PropPage::write(autoOpenItems, autoOpen);
	PropPage::write(optionItems, options);
	PropPage::write(confirmItems, confirm);
}

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

PropPage::Item WindowsPage::items[] = { { 0, 0, PropPage::T_END } };

PropPage::TextItem WindowsPage::textItem[] = {
	{ IDC_SETTINGS_AUTO_OPEN, N_("Auto-open at startup") },
	{ IDC_SETTINGS_WINDOWS_OPTIONS, N_("Window options") },
	{ IDC_SETTINGS_CONFIRM_OPTIONS, N_("Confirm dialog options") },
	{ 0, 0 }
};

WindowsPage::ListItem WindowsPage::listItems[] = {
	{ SettingsManager::OPEN_SYSTEM_LOG, N_("System Log") },
	{ SettingsManager::OPEN_FAVORITE_USERS, N_("Favorite Users") },
	{ SettingsManager::OPEN_QUEUE, N_("Download Queue") },
	{ SettingsManager::OPEN_FINISHED_DOWNLOADS, N_("Finished Downloads") },
	{ SettingsManager::OPEN_WAITING_USERS, N_("Waiting Users") },
	{ SettingsManager::OPEN_FINISHED_UPLOADS, N_("Finished Uploads") },
	{ SettingsManager::OPEN_SEARCH_SPY, N_("Search Spy") },
	{ SettingsManager::OPEN_NETWORK_STATISTICS, N_("Network Statistics") },
	{ SettingsManager::OPEN_NOTEPAD, N_("Notepad") },
	{ SettingsManager::OPEN_PUBLIC, N_("Public Hubs") },
	{ SettingsManager::OPEN_FAVORITE_HUBS, N_("Favorite Hubs") },
	{ 0, 0 }
};

WindowsPage::ListItem WindowsPage::optionItems[] = {
	{ SettingsManager::POPUP_PMS, N_("Open private messages in their own window") },
	{ SettingsManager::POPUP_HUB_PMS, N_("Open private messages from bots in their own window") },
	{ SettingsManager::POPUP_BOT_PMS, N_("Open private messages from the hub in their own window") },
	{ SettingsManager::POPUNDER_FILELIST, N_("Open new file list windows in the background") },
	{ SettingsManager::POPUNDER_PM, N_("Open new private message windows in the background") },
	{ SettingsManager::JOIN_OPEN_NEW_WINDOW, N_("Open new window when using /join") },
	{ SettingsManager::IGNORE_HUB_PMS, N_("Ignore private messages from the hub") },
	{ SettingsManager::IGNORE_BOT_PMS, N_("Ignore private messages from bots") },
	{ SettingsManager::TOGGLE_ACTIVE_WINDOW, N_("Toggle window when selecting an active tab") },
	{ SettingsManager::PROMPT_PASSWORD, N_("Popup box to input password for hubs") },
	{ 0, 0 }
};

WindowsPage::ListItem WindowsPage::confirmItems[] = {
	{ SettingsManager::CONFIRM_EXIT, N_("Confirm application exit") },
	{ SettingsManager::CONFIRM_HUB_REMOVAL, N_("Confirm favorite hub removal") },
	{ SettingsManager::CONFIRM_ITEM_REMOVAL, N_("Confirm item removal in download queue") },
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

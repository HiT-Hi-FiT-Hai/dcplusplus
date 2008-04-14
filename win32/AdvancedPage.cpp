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

#include "AdvancedPage.h"

#include <dcpp/SettingsManager.h>

PropPage::Item AdvancedPage::items[] = { { 0, 0, PropPage::T_END } };

AdvancedPage::ListItem AdvancedPage::listItems[] = {
	{ SettingsManager::AUTO_AWAY, N_("Auto-away on minimize (and back on restore)") },
	{ SettingsManager::AUTO_FOLLOW, N_("Automatically follow redirects") },
	{ SettingsManager::CLEAR_SEARCH, N_("Clear search box after each search") },
	{ SettingsManager::LIST_DUPES, N_("Keep duplicate files in your file list") },
	{ SettingsManager::URL_HANDLER, N_("Register with Windows to handle dchub:// and adc:// URL links") },
	{ SettingsManager::MAGNET_REGISTER, N_("Register with Windows to handle magnet: URI links") },
	{ SettingsManager::KEEP_LISTS, N_("Don't delete file lists when exiting") },
	{ SettingsManager::AUTO_KICK, N_("Automatically disconnect users who leave the hub") },
	{ SettingsManager::SHOW_PROGRESS_BARS, N_("Show progress bars for transfers") },
	{ SettingsManager::SFV_CHECK, N_("Enable automatic SFV checking") },
	{ SettingsManager::NO_AWAYMSG_TO_BOTS, N_("Don't send the away message to bots") },
	{ SettingsManager::ADLS_BREAK_ON_FIRST, N_("Break on first ADLSearch match") },
	{ SettingsManager::COMPRESS_TRANSFERS, N_("Enable safe and compressed transfers") },
	{ SettingsManager::HUB_USER_COMMANDS, N_("Accept custom user commands from hub") },
	{ SettingsManager::SEND_UNKNOWN_COMMANDS, N_("Send unknown /commands to the hub") },
	{ SettingsManager::ADD_FINISHED_INSTANTLY, N_("Add finished files to share instantly (if shared)") },
	{ SettingsManager::USE_CTRL_FOR_LINE_HISTORY, N_("Use CTRL for line history") },
	{ SettingsManager::AUTO_KICK_NO_FAVS, N_("Don't automatically disconnect favorite users who leave the hub") },
	{ SettingsManager::SHOW_SHELL_MENU, N_("Show shell menu where possible") },
	{ SettingsManager::OWNER_DRAWN_MENUS, N_("Use extended menus with icons and titles") },
	{ SettingsManager::CORAL, N_("Use Coral network when downloading hub lists (improves reliability)") },
	{ 0, 0 }
};

AdvancedPage::AdvancedPage(dwt::Widget* parent) : PropPage(parent) {
	createDialog(IDD_ADVANCEDPAGE);
	setHelpId(IDH_ADVANCEDPAGE);

	PropPage::read(handle(), items, listItems, ::GetDlgItem(handle(), IDC_ADVANCED_BOOLEANS));
}

AdvancedPage::~AdvancedPage() {
}

void AdvancedPage::write() {
	PropPage::write(handle(), items, listItems, ::GetDlgItem(handle(), IDC_ADVANCED_BOOLEANS));
}

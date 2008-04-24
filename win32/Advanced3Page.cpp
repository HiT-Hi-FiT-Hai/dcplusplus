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

#include "Advanced3Page.h"

#include <dcpp/SettingsManager.h>
#include "WinUtil.h"

#include <dwt/widgets/Spinner.h>

static const WinUtil::HelpItem helpItems[] = {
	{ IDC_SETTINGS_B, IDH_SETTINGS_ADVANCED3_ROLLBACK },
	{ IDC_SETTINGS_MAX_HASH_SPEED, IDH_SETTINGS_ADVANCED3_MAX_HASH_SPEED },
	{ IDC_MAX_HASH_SPEED, IDH_SETTINGS_ADVANCED3_MAX_HASH_SPEED },
	{ IDC_SETTINGS_MBS, IDH_SETTINGS_ADVANCED3_MAX_HASH_SPEED },
	{ IDC_SETTINGS_PM_HISTORY, IDH_SETTINGS_ADVANCED3_PM_HISTORY },
	{ IDC_SHOW_LAST_LINES_LOG, IDH_SETTINGS_ADVANCED3_PM_HISTORY },
	{ IDC_SETTINGS_TEXT_MINISLOT, IDH_SETTINGS_ADVANCED3_MINISLOT_SIZE },
	{ IDC_SET_MINISLOT_SIZE, IDH_SETTINGS_ADVANCED3_MINISLOT_SIZE },
	{ IDC_SETTINGS_KB2, IDH_SETTINGS_ADVANCED3_MINISLOT_SIZE },
	{ IDC_SETTINGS_MAX_FILELIST_SIZE, IDH_SETTINGS_ADVANCED3_MAX_FILELIST_SIZE },
	{ IDC_MAX_FILELIST_SIZE, IDH_SETTINGS_ADVANCED3_MAX_FILELIST_SIZE },
	{ IDC_SETTINGS_MB, IDH_SETTINGS_ADVANCED3_MAX_FILELIST_SIZE },
	{ IDC_SETTINGS_PID, IDH_SETTINGS_ADVANCED3_PRIVATE_ID },
	{ IDC_PRIVATE_ID, IDH_SETTINGS_ADVANCED3_PRIVATE_ID },
	{ IDC_SETTINGS_AUTO_REFRESH_TIME, IDH_SETTINGS_ADVANCED3_AUTO_REFRESH_TIME },
	{ IDC_AUTO_REFRESH_TIME, IDH_SETTINGS_ADVANCED3_AUTO_REFRESH_TIME },
	{ IDC_SETTINGS_WRITE_BUFFER, IDH_SETTINGS_ADVANCED3_BUFFERSIZE },
	{ IDC_BUFFERSIZE, IDH_SETTINGS_ADVANCED3_BUFFERSIZE },
	{ IDC_SETTINGS_KB, IDH_SETTINGS_ADVANCED3_BUFFERSIZE },
	{ IDC_SETTINGS_AUTO_SEARCH_LIMIT, IDH_SETTINGS_ADVANCED3_AUTO_SEARCH_LIMIT },
	{ IDC_AUTO_SEARCH_LIMIT, IDH_SETTINGS_ADVANCED3_AUTO_SEARCH_LIMIT },
	{ IDC_SETTINGS_SEARCH_HISTORY, IDH_SETTINGS_ADVANCED3_SEARCH_HISTORY },
	{ IDC_SEARCH_HISTORY, IDH_SETTINGS_ADVANCED3_SEARCH_HISTORY },
	{ IDC_SEARCH_HISTORY_SPIN, IDH_SETTINGS_ADVANCED3_SEARCH_HISTORY },
	{ IDC_SETTINGS_BIND_ADDRESS, IDH_SETTINGS_ADVANCED3_BIND_ADDRESS },
	{ IDC_BIND_ADDRESS, IDH_SETTINGS_ADVANCED3_BIND_ADDRESS },
	{ IDC_SETTINGS_SOCKET_IN_BUFFER, IDH_SETTINGS_ADVANCED3_SOCKET_IN_BUFFER },
	{ IDC_SOCKET_IN_BUFFER, IDH_SETTINGS_ADVANCED3_SOCKET_IN_BUFFER },
	{ IDC_SETTINGS_B2, IDH_SETTINGS_ADVANCED3_SOCKET_IN_BUFFER },
	{ IDC_SETTINGS_SOCKET_OUT_BUFFER, IDH_SETTINGS_ADVANCED3_SOCKET_OUT_BUFFER },
	{ IDC_SOCKET_OUT_BUFFER, IDH_SETTINGS_ADVANCED3_SOCKET_OUT_BUFFER },
	{ IDC_SETTINGS_B3, IDH_SETTINGS_ADVANCED3_SOCKET_OUT_BUFFER },
	{ 0, 0 }
};

PropPage::TextItem Advanced3Page::texts[] = {
	{ IDC_SETTINGS_B, N_("B") },
	{ IDC_SETTINGS_MAX_HASH_SPEED, N_("Max hash speed") },
	{ IDC_SETTINGS_MBS, N_("MiB/s") },
	{ IDC_SETTINGS_PM_HISTORY, N_("PM history") },
	{ IDC_SETTINGS_TEXT_MINISLOT, N_("Mini slot size") },
	{ IDC_SETTINGS_KB2, N_("KiB") },
	{ IDC_SETTINGS_MAX_FILELIST_SIZE, N_("Max filelist size") },
	{ IDC_SETTINGS_MB, N_("MiB") },
	{ IDC_SETTINGS_PID, N_("PID") },
	{ IDC_SETTINGS_AUTO_REFRESH_TIME, N_("Auto refresh time") },
	{ IDC_SETTINGS_WRITE_BUFFER, N_("Write buffer size") },
	{ IDC_SETTINGS_KB, N_("KiB") },
	{ IDC_SETTINGS_AUTO_SEARCH_LIMIT, N_("Auto-search limit") },
	{ IDC_SETTINGS_SEARCH_HISTORY, N_("Search history") },
	{ IDC_SETTINGS_BIND_ADDRESS, N_("Bind address") },
	{ IDC_SETTINGS_SOCKET_IN_BUFFER, N_("Socket read buffer") },
	{ IDC_SETTINGS_B2, N_("B") },
	{ IDC_SETTINGS_SOCKET_OUT_BUFFER, N_("Socket write buffer") },
	{ IDC_SETTINGS_B3, N_("B") },
	{ 0, 0 }
};

PropPage::Item Advanced3Page::items[] = {
	{ IDC_BUFFERSIZE, SettingsManager::BUFFER_SIZE, PropPage::T_INT },
	{ IDC_MAX_HASH_SPEED, SettingsManager::MAX_HASH_SPEED, PropPage::T_INT },
	{ IDC_SHOW_LAST_LINES_LOG, SettingsManager::SHOW_LAST_LINES_LOG, PropPage::T_INT },
	{ IDC_SEARCH_HISTORY, SettingsManager::SEARCH_HISTORY, PropPage::T_INT },
	{ IDC_SET_MINISLOT_SIZE, SettingsManager::SET_MINISLOT_SIZE, PropPage::T_INT },
	{ IDC_BIND_ADDRESS, SettingsManager::BIND_ADDRESS, PropPage::T_STR },
	{ IDC_MAX_FILELIST_SIZE, SettingsManager::MAX_FILELIST_SIZE, PropPage::T_INT },
	{ IDC_SOCKET_IN_BUFFER, SettingsManager::SOCKET_IN_BUFFER, PropPage::T_INT },
	{ IDC_SOCKET_OUT_BUFFER, SettingsManager::SOCKET_OUT_BUFFER, PropPage::T_INT },
	{ IDC_PRIVATE_ID, SettingsManager::PRIVATE_ID, PropPage::T_STR },
	{ IDC_AUTO_REFRESH_TIME, SettingsManager::AUTO_REFRESH_TIME, PropPage::T_INT },
	{ IDC_AUTO_SEARCH_LIMIT, SettingsManager::AUTO_SEARCH_LIMIT, PropPage::T_INT },
	{ 0, 0, PropPage::T_END }
};

Advanced3Page::Advanced3Page(dwt::Widget* parent) : PropPage(parent) {
	createDialog(IDD_ADVANCED3PAGE);
	setHelpId(IDH_ADVANCED3PAGE);

	WinUtil::setHelpIds(this, helpItems);
	PropPage::translate(handle(), texts);
	PropPage::read(handle(), items, 0, 0);

	SpinnerPtr spinner = attachChild<Spinner>(IDC_SEARCH_HISTORY_SPIN);
	spinner->setRange(0, 100);

	attachChild<TextBox>(IDC_MAX_HASH_SPEED);
	attachChild<TextBox>(IDC_SHOW_LAST_LINES_LOG);
	attachChild<TextBox>(IDC_SET_MINISLOT_SIZE);
	attachChild<TextBox>(IDC_MAX_FILELIST_SIZE);
	attachChild<TextBox>(IDC_PRIVATE_ID);
	attachChild<TextBox>(IDC_AUTO_REFRESH_TIME);
	attachChild<TextBox>(IDC_BUFFERSIZE);
	attachChild<TextBox>(IDC_AUTO_SEARCH_LIMIT);
	attachChild<TextBox>(IDC_SEARCH_HISTORY);
	attachChild<TextBox>(IDC_BIND_ADDRESS);
	attachChild<TextBox>(IDC_SOCKET_IN_BUFFER);
	attachChild<TextBox>(IDC_SOCKET_OUT_BUFFER);
}

Advanced3Page::~Advanced3Page() {
}

void Advanced3Page::write() {
	PropPage::write(handle(), items, 0, 0);

	SettingsManager* settings = SettingsManager::getInstance();
	if(SETTING(SET_MINISLOT_SIZE) < 64)
		settings->set(SettingsManager::SET_MINISLOT_SIZE, 64);
	if(SETTING(AUTO_SEARCH_LIMIT) > 5)
		settings->set(SettingsManager::AUTO_SEARCH_LIMIT, 5);
	else if(SETTING(AUTO_SEARCH_LIMIT) < 1)
		settings->set(SettingsManager::AUTO_SEARCH_LIMIT, 1);
}

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

#include "Advanced3Page.h"

#include <dcpp/SettingsManager.h>

PropPage::TextItem Advanced3Page::texts[] = {
	{ IDC_SETTINGS_B, N_("B") },
	{ IDC_SETTINGS_WRITE_BUFFER, N_("Write buffer size") },
	{ IDC_SETTINGS_KB, N_("KiB") },
	{ IDC_SETTINGS_MAX_HASH_SPEED, N_("Max hash speed") },
	{ IDC_SETTINGS_MBS, N_("MiB/s") },
	{ IDC_SETTINGS_PM_HISTORY, N_("PM history") },
	{ IDC_SETTINGS_SEARCH_HISTORY, N_("Search history") },
	{ IDC_SETTINGS_TEXT_MINISLOT, N_("Mini slot size") },
	{ IDC_SETTINGS_KB2, N_("KiB") },
	{ IDC_SETTINGS_BIND_ADDRESS, N_("Bind address") },
	{ IDC_SETTINGS_MAX_FILELIST_SIZE, N_("Max filelist size") },
	{ IDC_SETTINGS_MB, N_("MiB") },
	{ IDC_SETTINGS_AUTO_REFRESH_TIME, N_("Auto refresh time") },
	{ IDC_SETTINGS_AUTO_SEARCH_LIMIT, N_("Auto-search limit") },
	{ IDC_SETTINGS_MIN_SEGMENT_SIZE, N_("Min segment size") },
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
	{ IDC_MIN_SEGMENT_SIZE, SettingsManager::MIN_SEGMENT_SIZE, PropPage::T_INT },
	{ 0, 0, PropPage::T_END }
};

Advanced3Page::Advanced3Page(SmartWin::Widget* parent) : PropPage(parent) {
	createDialog(IDD_ADVANCED3PAGE);

	PropPage::translate(handle(), texts);
	PropPage::read(handle(), items, 0, 0);

	WidgetSpinnerPtr spinner = attachSpinner(IDC_SEARCH_HISTORY_SPIN);
	spinner->setRange(0, 100);
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

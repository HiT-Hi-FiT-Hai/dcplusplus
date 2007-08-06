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
	{ IDC_SETTINGS_B, ResourceManager::B },
	{ IDC_SETTINGS_WRITE_BUFFER, ResourceManager::SETTINGS_WRITE_BUFFER },
	{ IDC_SETTINGS_KB, ResourceManager::KiB },
	{ IDC_SETTINGS_MAX_HASH_SPEED, ResourceManager::SETTINGS_MAX_HASH_SPEED },
	{ IDC_SETTINGS_MBS, ResourceManager::MiBPS },
	{ IDC_SETTINGS_PM_HISTORY, ResourceManager::SETTINGS_PM_HISTORY },
	{ IDC_SETTINGS_SEARCH_HISTORY, ResourceManager::SETTINGS_SEARCH_HISTORY },
	{ IDC_SETTINGS_TEXT_MINISLOT, ResourceManager::SETTINGS_TEXT_MINISLOT },
	{ IDC_SETTINGS_KB2, ResourceManager::KiB },
	{ IDC_SETTINGS_BIND_ADDRESS, ResourceManager::SETTINGS_BIND_ADDRESS },
	{ IDC_SETTINGS_MAX_FILELIST_SIZE, ResourceManager::SETTINGS_MAX_FILELIST_SIZE },
	{ IDC_SETTINGS_MB, ResourceManager::MiB },
	{ IDC_SETTINGS_AUTO_REFRESH_TIME, ResourceManager::SETTINGS_AUTO_REFRESH_TIME },
	{ IDC_SETTINGS_AUTO_SEARCH_LIMIT, ResourceManager::SETTINGS_AUTO_SEARCH_LIMIT },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
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

Advanced3Page::Advanced3Page(SmartWin::Widget* parent) : PropPage(parent) {
	createDialog(IDD_ADVANCED3PAGE);

	PropPage::translate(handle(), texts);
	PropPage::read(handle(), items, 0, 0);

	WidgetSpinnerPtr spinner = subclassSpinner(IDC_SEARCH_HISTORY_SPIN);
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

#ifdef PORT_ME

LRESULT Advanced3Page::onHelpInfo(LPNMHDR /*pnmh*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_ADVANCED3PAGE);
	return 0;
}

LRESULT Advanced3Page::onHelp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_ADVANCED3PAGE);
	return 0;
}
#endif

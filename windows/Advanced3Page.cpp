/*
 * Copyright (C) 2001-2005 Jacek Sieka, arnetheduck on gmail point com
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
#include "../client/DCPlusPlus.h"
#include "Resource.h"

#include "Advanced3Page.h"
#include "CommandDlg.h"

#include "../client/SettingsManager.h"
#include "../client/FavoriteManager.h"
#include "WinUtil.h"

PropPage::TextItem Advanced3Page::texts[] = {
	{ IDC_SETTINGS_ADVANCED, ResourceManager::SETTINGS_ADVANCED_SETTINGS },
	{ IDC_SETTINGS_ROLLBACK, ResourceManager::SETTINGS_ROLLBACK },
	{ IDC_SETTINGS_B, ResourceManager::B },
	{ IDC_SETTINGS_WRITE_BUFFER, ResourceManager::SETTINGS_WRITE_BUFFER },
	{ IDC_SETTINGS_KB, ResourceManager::KiB },
	{ IDC_SETTINGS_MAX_TAB_ROWS, ResourceManager::SETTINGS_MAX_TAB_ROWS },
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
	{ IDC_ROLLBACK, SettingsManager::ROLLBACK, PropPage::T_INT }, 
	{ IDC_BUFFERSIZE, SettingsManager::BUFFER_SIZE, PropPage::T_INT },
	{ IDC_MAX_TAB_ROWS, SettingsManager::MAX_TAB_ROWS, PropPage::T_INT },
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

LRESULT Advanced3Page::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	PropPage::translate((HWND)(*this), texts);
	PropPage::read((HWND)*this, items, 0, 0);

	CUpDownCtrl spin;
	spin.Attach(GetDlgItem(IDC_SEARCH_HISTORY_SPIN));
	spin.SetRange32(0, 100);
	SetDlgItemText(IDC_SEARCH_HISTORY,Text::toT(Util::toString( SETTING(SEARCH_HISTORY))).c_str());

	// Do specialized reading here
	return TRUE;
}

void Advanced3Page::write() {
	PropPage::write((HWND)*this, items, 0, 0);

	if(SETTING(SET_MINISLOT_SIZE) < 64)
		settings->set(SettingsManager::SET_MINISLOT_SIZE, 64);
	if(SETTING(AUTO_SEARCH_LIMIT) > 5)
		settings->set(SettingsManager::AUTO_SEARCH_LIMIT, 5);
	else if(SETTING(AUTO_SEARCH_LIMIT) < 1)
		settings->set(SettingsManager::AUTO_SEARCH_LIMIT, 1);
}

LRESULT Advanced3Page::onHelpInfo(LPNMHDR /*pnmh*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_ADVANCED3PAGE);
	return 0;
}

LRESULT Advanced3Page::onHelp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_ADVANCED3PAGE);
	return 0;
}

/**
 * @file
 * $Id: Advanced3Page.cpp,v 1.14 2006/02/10 07:56:47 arnetheduck Exp $
 */

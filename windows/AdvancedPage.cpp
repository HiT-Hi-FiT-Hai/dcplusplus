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

#include "AdvancedPage.h"
#include "CommandDlg.h"

#include "../client/SettingsManager.h"
#include "../client/FavoriteManager.h"
#include "WinUtil.h"

PropPage::Item AdvancedPage::items[] = { { 0, 0, PropPage::T_END } };

AdvancedPage::ListItem AdvancedPage::listItems[] = {
	{ SettingsManager::AUTO_AWAY, ResourceManager::SETTINGS_AUTO_AWAY },
	{ SettingsManager::AUTO_FOLLOW, ResourceManager::SETTINGS_AUTO_FOLLOW },
	{ SettingsManager::CLEAR_SEARCH, ResourceManager::SETTINGS_CLEAR_SEARCH },
	{ SettingsManager::AUTO_SEARCH, ResourceManager::SETTINGS_AUTO_SEARCH },
	{ SettingsManager::LIST_DUPES, ResourceManager::SETTINGS_LIST_DUPES },
	{ SettingsManager::URL_HANDLER, ResourceManager::SETTINGS_URL_HANDLER },
	{ SettingsManager::IGNORE_OFFLINE, ResourceManager::SETTINGS_IGNORE_OFFLINE },
	{ SettingsManager::MAGNET_REGISTER, ResourceManager::SETTINGS_URL_MAGNET },
	{ SettingsManager::SMALL_SEND_BUFFER, ResourceManager::SETTINGS_SMALL_SEND_BUFFER },
	{ SettingsManager::KEEP_LISTS, ResourceManager::SETTINGS_KEEP_LISTS },
	{ SettingsManager::AUTO_KICK, ResourceManager::SETTINGS_AUTO_KICK },
	{ SettingsManager::SHOW_PROGRESS_BARS, ResourceManager::SETTINGS_SHOW_PROGRESS_BARS },
	{ SettingsManager::SFV_CHECK, ResourceManager::SETTINGS_SFV_CHECK },
	{ SettingsManager::AUTO_UPDATE_LIST, ResourceManager::SETTINGS_AUTO_UPDATE_LIST },
	{ SettingsManager::ANTI_FRAG, ResourceManager::SETTINGS_ANTI_FRAG },
	{ SettingsManager::NO_AWAYMSG_TO_BOTS, ResourceManager::SETTINGS_NO_AWAYMSG_TO_BOTS },
	{ SettingsManager::SKIP_ZERO_BYTE, ResourceManager::SETTINGS_SKIP_ZERO_BYTE },
	{ SettingsManager::ADLS_BREAK_ON_FIRST, ResourceManager::SETTINGS_ADLS_BREAK_ON_FIRST },
	{ SettingsManager::TAB_COMPLETION, ResourceManager::SETTINGS_TAB_COMPLETION },
	{ SettingsManager::COMPRESS_TRANSFERS, ResourceManager::SETTINGS_COMPRESS_TRANSFERS },
	{ SettingsManager::HUB_USER_COMMANDS, ResourceManager::SETTINGS_HUB_USER_COMMANDS },
	{ SettingsManager::AUTO_SEARCH_AUTO_MATCH, ResourceManager::SETTINGS_AUTO_SEARCH_AUTO_MATCH },
	{ SettingsManager::LOG_FILELIST_TRANSFERS, ResourceManager::SETTINGS_LOG_FILELIST_TRANSFERS },
	{ SettingsManager::SEND_UNKNOWN_COMMANDS, ResourceManager::SETTINGS_SEND_UNKNOWN_COMMANDS },
	{ SettingsManager::ADD_FINISHED_INSTANTLY, ResourceManager::SETTINGS_ADD_FINISHED_INSTANTLY },
	{ SettingsManager::DONT_DL_ALREADY_SHARED, ResourceManager::SETTINGS_DONT_DL_ALREADY_SHARED },
	{ SettingsManager::SETTINGS_USE_CTRL_FOR_LINE_HISTORY, ResourceManager::SETTINGS_USE_CTRL_FOR_LINE_HISTORY },
	{ SettingsManager::SEARCH_ONLY_TTH, ResourceManager::SETTINGS_ONLY_TTH },
	{ SettingsManager::ADVANCED_RESUME, ResourceManager::SETTINGS_ADVANCED_RESUME },
	{ SettingsManager::TOGGLE_ACTIVE_WINDOW, ResourceManager::SETTINGS_TOGGLE_ACTIVE_WINDOW },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

LRESULT AdvancedPage::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	PropPage::read((HWND)*this, items, listItems, GetDlgItem(IDC_ADVANCED_BOOLEANS));

	// Do specialized reading here
	return TRUE;
}

void AdvancedPage::write() {
	PropPage::write((HWND)*this, items, listItems, GetDlgItem(IDC_ADVANCED_BOOLEANS));
}

LRESULT AdvancedPage::onHelpInfo(LPNMHDR /*pnmh*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_ADVANCEDPAGE);
	return 0;
}

LRESULT AdvancedPage::onHelp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_ADVANCEDPAGE);
	return 0;
}

/**
 * @file
 * $Id: AdvancedPage.cpp,v 1.52 2005/04/24 08:13:03 arnetheduck Exp $
 */

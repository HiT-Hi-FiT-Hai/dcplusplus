/* 
 * Copyright (C) 2001-2003 Jacek Sieka, j_s@telia.com
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
#include "../client/HubManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

PropPage::TextItem AdvancedPage::texts[] = {
	{ IDC_SETTINGS_ADVANCED, ResourceManager::SETTINGS_ADVANCED_SETTINGS },
	{ IDC_SETTINGS_ROLLBACK, ResourceManager::SETTINGS_ROLLBACK },
	{ IDC_SETTINGS_B, ResourceManager::B },
	{ IDC_SETTINGS_CLIENT_VER, ResourceManager::SETTINGS_CLIENT_VER },
	{ IDC_SETTINGS_WRITE_BUFFER, ResourceManager::SETTINGS_WRITE_BUFFER },
	{ IDC_SETTINGS_KB, ResourceManager::KB },
	{ IDC_SETTINGS_MAX_TAB_ROWS, ResourceManager::SETTINGS_MAX_TAB_ROWS },
	{ IDC_SETTINGS_MAX_HASH_SPEED, ResourceManager::SETTINGS_MAX_HASH_SPEED },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

PropPage::Item AdvancedPage::items[] = {
	{ IDC_ROLLBACK, SettingsManager::ROLLBACK, PropPage::T_INT }, 
	{ IDC_CLVERSION, SettingsManager::CLIENTVERSION, PropPage::T_STR }, 
	{ IDC_BUFFERSIZE, SettingsManager::BUFFER_SIZE, PropPage::T_INT },
	{ IDC_MAX_TAB_ROWS, SettingsManager::MAX_TAB_ROWS, PropPage::T_INT },
	{ IDC_MAX_HASH_SPEED, SettingsManager::MAX_HASH_SPEED, PropPage::T_INT },
	{ 0, 0, PropPage::T_END }
};

AdvancedPage::ListItem AdvancedPage::listItems[] = {
	{ SettingsManager::AUTO_AWAY, ResourceManager::SETTINGS_AUTO_AWAY },
	{ SettingsManager::AUTO_FOLLOW, ResourceManager::SETTINGS_AUTO_FOLLOW },
	{ SettingsManager::CLEAR_SEARCH, ResourceManager::SETTINGS_CLEAR_SEARCH },
	{ SettingsManager::OPEN_PUBLIC, ResourceManager::SETTINGS_OPEN_PUBLIC },
	{ SettingsManager::OPEN_QUEUE, ResourceManager::SETTINGS_OPEN_QUEUE },
	{ SettingsManager::OPEN_FAVORITE_HUBS, ResourceManager::SETTINGS_OPEN_FAVORITE_HUBS },
	{ SettingsManager::OPEN_FINISHED_DOWNLOADS, ResourceManager::SETTINGS_OPEN_FINISHED_DOWNLOADS },
	{ SettingsManager::AUTO_SEARCH, ResourceManager::SETTINGS_AUTO_SEARCH },
	{ SettingsManager::AUTO_SEARCH_AUTO_STRING, ResourceManager::SETTINGS_AUTO_SEARCH_AUTO_STRING }, 
	{ SettingsManager::POPUP_PMS, ResourceManager::SETTINGS_POPUP_PMS },
	{ SettingsManager::IGNORE_OFFLINE, ResourceManager::SETTINGS_IGNORE_OFFLINE },
	{ SettingsManager::POPUP_OFFLINE, ResourceManager::SETTINGS_POPUP_OFFLINE },
	{ SettingsManager::LIST_DUPES, ResourceManager::SETTINGS_LIST_DUPES },
	{ SettingsManager::URL_HANDLER, ResourceManager::SETTINGS_URL_HANDLER },
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
	{ SettingsManager::AUTO_SEARCH_EXACT, ResourceManager::SETTINGS_AUTO_SEARCH_EXACT },
	{ SettingsManager::SEND_UNKNOWN_COMMANDS, ResourceManager::SETTINGS_SEND_UNKNOWN_COMMANDS },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

LRESULT AdvancedPage::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	PropPage::translate((HWND)(*this), texts);
	PropPage::read((HWND)*this, items, listItems, GetDlgItem(IDC_ADVANCED_BOOLEANS));

	// Do specialized reading here
	return TRUE;
}

void AdvancedPage::write() {
	PropPage::write((HWND)*this, items, listItems, GetDlgItem(IDC_ADVANCED_BOOLEANS));
}

/**
 * @file
 * $Id: AdvancedPage.cpp,v 1.28 2004/04/18 12:51:15 arnetheduck Exp $
 */


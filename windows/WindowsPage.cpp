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

#include "WindowsPage.h"

#include "../client/SettingsManager.h"
#include "../client/FavoriteManager.h"
#include "WinUtil.h"

PropPage::Item WindowsPage::items[] = { { 0, 0, PropPage::T_END } };

WindowsPage::ListItem WindowsPage::listItems[] = {
	{ SettingsManager::OPEN_PUBLIC, ResourceManager::PUBLIC_HUBS },
	{ SettingsManager::OPEN_FAVORITE_HUBS, ResourceManager::FAVORITE_HUBS },
	{ SettingsManager::OPEN_FAVORITE_USERS, ResourceManager::FAVORITE_USERS },
	{ SettingsManager::OPEN_QUEUE, ResourceManager::DOWNLOAD_QUEUE },
	{ SettingsManager::OPEN_FINISHED_DOWNLOADS, ResourceManager::FINISHED_DOWNLOADS },
	{ SettingsManager::OPEN_FINISHED_UPLOADS, ResourceManager::FINISHED_UPLOADS },
	{ SettingsManager::OPEN_SEARCH_SPY, ResourceManager::SEARCH_SPY },
	{ SettingsManager::OPEN_NETWORK_STATISTICS, ResourceManager::NETWORK_STATISTICS },
	{ SettingsManager::OPEN_NOTEPAD, ResourceManager::NOTEPAD },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

WindowsPage::ListItem WindowsPage::optionItems[] = {
	{ SettingsManager::POPUP_PMS, ResourceManager::SETTINGS_POPUP_PMS },
	{ SettingsManager::POPUP_OFFLINE, ResourceManager::SETTINGS_POPUP_OFFLINE },
	{ SettingsManager::POPUNDER_FILELIST, ResourceManager::SETTINGS_POPUNDER_FILELIST },
	{ SettingsManager::POPUNDER_PM, ResourceManager::SETTINGS_POPUNDER_PM },
	{ SettingsManager::SETTINGS_OPEN_NEW_WINDOW, ResourceManager::SETTINGS_OPEN_NEW_WINDOW },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

LRESULT WindowsPage::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	PropPage::read((HWND)*this, items, listItems, GetDlgItem(IDC_WINDOWS_STARTUP));
	PropPage::read((HWND)*this, items, optionItems, GetDlgItem(IDC_WINDOWS_OPTIONS));

	// Do specialized reading here
	return TRUE;
}

void WindowsPage::write() {
	PropPage::write((HWND)*this, items, listItems, GetDlgItem(IDC_WINDOWS_STARTUP));
	PropPage::write((HWND)*this, items, optionItems, GetDlgItem(IDC_WINDOWS_OPTIONS));
}

LRESULT WindowsPage::onHelpInfo(LPNMHDR /*pnmh*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_WINDOWSPAGE);
	return 0;
}

LRESULT WindowsPage::onHelp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_WINDOWSPAGE);
	return 0;
}

/**
 * @file
 * $Id: WindowsPage.cpp,v 1.4 2005/04/12 23:24:03 arnetheduck Exp $
 */


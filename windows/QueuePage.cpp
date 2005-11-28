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

#include "QueuePage.h"
#include "CommandDlg.h"

#include "../client/SettingsManager.h"
#include "WinUtil.h"

PropPage::TextItem QueuePage::texts[] = {
	{ IDC_SETTINGS_PRIO_HIGHEST, ResourceManager::SETTINGS_PRIO_HIGHEST },
	{ IDC_SETTINGS_KB3, ResourceManager::KiB },
	{ IDC_SETTINGS_PRIO_HIGH, ResourceManager::SETTINGS_PRIO_HIGH },
	{ IDC_SETTINGS_KB4, ResourceManager::KiB },
	{ IDC_SETTINGS_PRIO_NORMAL, ResourceManager::SETTINGS_PRIO_NORMAL },
	{ IDC_SETTINGS_KB5, ResourceManager::KiB },
	{ IDC_SETTINGS_PRIO_LOW, ResourceManager::SETTINGS_PRIO_LOW },
	{ IDC_SETTINGS_KB6, ResourceManager::KiB },
	{ IDC_SETTINGS_AUTODROP_SPEED, ResourceManager::SETTINGS_AUTODROP_SPEED },
	{ IDC_SETTINGS_BPS, ResourceManager::BPS },
	{ IDC_SETTINGS_AUTODROP_INTERVAL, ResourceManager::SETTINGS_AUTODROP_INTERVAL },
	{ IDC_SETTINGS_S1, ResourceManager::S },
	{ IDC_SETTINGS_AUTODROP_ELAPSED, ResourceManager::SETTINGS_AUTODROP_ELAPSED },
	{ IDC_SETTINGS_S2, ResourceManager::S },
	{ IDC_SETTINGS_AUTODROP_INACTIVITY, ResourceManager::SETTINGS_AUTODROP_INACTIVITY },
	{ IDC_SETTINGS_S3, ResourceManager::S },
	{ IDC_SETTINGS_AUTODROP_MINSOURCES, ResourceManager::SETTINGS_AUTODROP_MINSOURCES },
	{ IDC_SETTINGS_AUTODROP_FILESIZE, ResourceManager::SETTINGS_AUTODROP_FILESIZE },
	{ IDC_SETTINGS_KB7, ResourceManager::KiB },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

PropPage::Item QueuePage::items[] = {
	{ IDC_PRIO_HIGHEST_SIZE, SettingsManager::PRIO_HIGHEST_SIZE, PropPage::T_INT },
	{ IDC_PRIO_HIGH_SIZE, SettingsManager::PRIO_HIGH_SIZE, PropPage::T_INT },
	{ IDC_PRIO_NORMAL_SIZE, SettingsManager::PRIO_NORMAL_SIZE, PropPage::T_INT },
	{ IDC_PRIO_LOW_SIZE, SettingsManager::PRIO_LOW_SIZE, PropPage::T_INT },
	{ IDC_AUTODROP_SPEED, SettingsManager::AUTODROP_SPEED, PropPage::T_INT },
	{ IDC_AUTODROP_INTERVAL, SettingsManager::AUTODROP_INTERVAL, PropPage::T_INT },
	{ IDC_AUTODROP_ELAPSED, SettingsManager::AUTODROP_ELAPSED, PropPage::T_INT },
	{ IDC_AUTODROP_INACTIVITY, SettingsManager::AUTODROP_INACTIVITY, PropPage::T_INT },
	{ IDC_AUTODROP_MINSOURCES, SettingsManager::AUTODROP_MINSOURCES, PropPage::T_INT },
	{ IDC_AUTODROP_FILESIZE, SettingsManager::AUTODROP_FILESIZE, PropPage::T_INT },
	{ 0, 0, PropPage::T_END }
};

PropPage::ListItem QueuePage::optionItems[] = {
	{ SettingsManager::PRIO_LOWEST, ResourceManager::SETTINGS_PRIO_LOWEST },
	{ SettingsManager::AUTODROP_ALL, ResourceManager::SETTINGS_AUTODROP_ALL },
	{ SettingsManager::AUTODROP_FILELISTS, ResourceManager::SETTINGS_AUTODROP_FILELISTS },
	{ SettingsManager::AUTODROP_DISCONNECT, ResourceManager::SETTINGS_AUTODROP_DISCONNECT },
	{ SettingsManager::AUTO_SEARCH, ResourceManager::SETTINGS_AUTO_SEARCH },
	{ SettingsManager::AUTO_SEARCH_AUTO_MATCH, ResourceManager::SETTINGS_AUTO_SEARCH_AUTO_MATCH },
	{ SettingsManager::SKIP_ZERO_BYTE, ResourceManager::SETTINGS_SKIP_ZERO_BYTE },
	{ SettingsManager::DONT_DL_ALREADY_SHARED, ResourceManager::SETTINGS_DONT_DL_ALREADY_SHARED },
	{ SettingsManager::ANTI_FRAG, ResourceManager::SETTINGS_ANTI_FRAG },
	{ SettingsManager::ADVANCED_RESUME, ResourceManager::SETTINGS_ADVANCED_RESUME },
	{ SettingsManager::ONLY_DL_TTH_FILES, ResourceManager::SETTINGS_ONLY_DL_TTH_FILES },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

LRESULT QueuePage::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	PropPage::translate((HWND)(*this), texts);
	PropPage::read((HWND)*this, items, 0, 0);
	PropPage::read((HWND)*this, items, optionItems, GetDlgItem(IDC_OTHER_QUEUE_OPTIONS));

	// Do specialized reading here
	return TRUE;
}

void QueuePage::write() {
	PropPage::write((HWND)*this, items, 0, 0);
	PropPage::write((HWND)*this, items, optionItems, GetDlgItem(IDC_OTHER_QUEUE_OPTIONS));

	if(SETTING(AUTODROP_INTERVAL) < 1)
		settings->set(SettingsManager::AUTODROP_INTERVAL, 1);
	if(SETTING(AUTODROP_ELAPSED) < 1)
		settings->set(SettingsManager::AUTODROP_ELAPSED, 1);
}

LRESULT QueuePage::onHelpInfo(LPNMHDR /*pnmh*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_QUEUEPAGE);
	return 0;
}

LRESULT QueuePage::onHelp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_QUEUEPAGE);
	return 0;
}

/**
 * @file
 * $Id: QueuePage.cpp,v 1.7 2005/11/28 01:21:07 arnetheduck Exp $
 */

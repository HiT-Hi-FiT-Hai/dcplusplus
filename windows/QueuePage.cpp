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
	{ IDC_SETTINGS_PRIO_LOWEST, ResourceManager::SETTINGS_PRIO_LOWEST },
	{ IDC_SETTINGS_BOOL, ResourceManager::BOOL },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

PropPage::Item QueuePage::items[] = {
	{ IDC_PRIO_HIGHEST_SIZE, SettingsManager::PRIO_HIGHEST_SIZE, PropPage::T_INT },
	{ IDC_PRIO_HIGH_SIZE, SettingsManager::PRIO_HIGH_SIZE, PropPage::T_INT },
	{ IDC_PRIO_NORMAL_SIZE, SettingsManager::PRIO_NORMAL_SIZE, PropPage::T_INT },
	{ IDC_PRIO_LOW_SIZE, SettingsManager::PRIO_LOW_SIZE, PropPage::T_INT },
	{ IDC_PRIO_LOWEST, SettingsManager::PRIO_LOWEST, PropPage::T_INT },
	{ 0, 0, PropPage::T_END }
};

LRESULT QueuePage::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	PropPage::translate((HWND)(*this), texts);
	PropPage::read((HWND)*this, items, 0, 0);

	// Do specialized reading here
	return TRUE;
}

void QueuePage::write() {
	PropPage::write((HWND)*this, items, 0, 0);

	if(SETTING(PRIO_HIGHEST_SIZE) < 16)
		settings->set(SettingsManager::PRIO_HIGHEST_SIZE, 16);
	if(SETTING(PRIO_HIGH_SIZE) < 0)
		settings->set(SettingsManager::PRIO_HIGH_SIZE, 0);
	if(SETTING(PRIO_NORMAL_SIZE) < 0)
		settings->set(SettingsManager::PRIO_NORMAL_SIZE, 0);
	if(SETTING(PRIO_LOW_SIZE) < 0)
		settings->set(SettingsManager::PRIO_LOW_SIZE, 0);
	if(SETTING(PRIO_LOWEST) < 0)
		settings->set(SettingsManager::PRIO_LOWEST, 0);
	if(SETTING(PRIO_LOWEST) > 1)
		settings->set(SettingsManager::PRIO_LOWEST, 1);
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
 * $Id: QueuePage.cpp,v 1.2 2005/04/24 08:13:03 arnetheduck Exp $
 */

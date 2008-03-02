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

#include "QueuePage.h"

#include <dcpp/SettingsManager.h>

PropPage::TextItem QueuePage::texts[] = {
	{ IDC_SETTINGS_AUTOPRIO, N_("Auto priority settings") },
	{ IDC_SETTINGS_PRIO_HIGHEST, N_("Highest prio max size") },
	{ IDC_SETTINGS_KB3, N_("KiB") },
	{ IDC_SETTINGS_PRIO_HIGH, N_("High prio max size") },
	{ IDC_SETTINGS_KB4, N_("KiB") },
	{ IDC_SETTINGS_PRIO_NORMAL, N_("Normal prio max size") },
	{ IDC_SETTINGS_KB5, N_("KiB") },
	{ IDC_SETTINGS_PRIO_LOW, N_("Low prio max size") },
	{ IDC_SETTINGS_KB6, N_("KiB") },
	{ IDC_SETTINGS_AUTODROP, N_("Autodrop settings") },
	{ IDC_SETTINGS_AUTODROP_SPEED, N_("Drop sources below") },
	{ IDC_SETTINGS_BPS, N_("B/s") },
	{ IDC_SETTINGS_AUTODROP_INTERVAL, N_("Check every") },
	{ IDC_SETTINGS_S1, N_("s") },
	{ IDC_SETTINGS_AUTODROP_ELAPSED, N_("Min elapsed") },
	{ IDC_SETTINGS_S2, N_("s") },
	{ IDC_SETTINGS_AUTODROP_INACTIVITY, N_("Max inactivity") },
	{ IDC_SETTINGS_S3, N_("s") },
	{ IDC_SETTINGS_AUTODROP_MINSOURCES, N_("Min sources online") },
	{ IDC_SETTINGS_AUTODROP_FILESIZE, N_("Min filesize") },
	{ IDC_SETTINGS_KB7, N_("KiB") },
	{ IDC_SETTINGS_OTHER_QUEUE_OPTIONS, N_("Other queue options") },
	{ 0, 0 }
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
	{ SettingsManager::PRIO_LOWEST, N_("Set lowest prio for newly added files larger than Low prio size") },
	{ SettingsManager::AUTODROP_ALL, N_("Autodrop slow sources for all queue items (except filelists)") },
	{ SettingsManager::AUTODROP_FILELISTS, N_("Remove slow filelists") },
	{ SettingsManager::AUTODROP_DISCONNECT, N_("Don't remove the source when autodropping, only disconnect") },
	{ SettingsManager::AUTO_SEARCH, N_("Automatically search for alternative download locations") },
	{ SettingsManager::AUTO_SEARCH_AUTO_MATCH, N_("Automatically match queue for auto search hits") },
	{ SettingsManager::SKIP_ZERO_BYTE, N_("Skip zero-byte files") },
	{ SettingsManager::DONT_DL_ALREADY_SHARED, N_("Don't download files already in share") },
	{ SettingsManager::DONT_DL_ALREADY_QUEUED, N_("Don't download files already in the queue") },
	{ SettingsManager::ANTI_FRAG, N_("Use antifragmentation method for downloads") },
	{ 0, 0 }
};

QueuePage::QueuePage(SmartWin::Widget* parent) : PropPage(parent) {
	createDialog(IDD_QUEUEPAGE);

	PropPage::translate(handle(), texts);
	PropPage::read(handle(), items, 0, 0);
	PropPage::read(handle(), items, optionItems, ::GetDlgItem(handle(), IDC_OTHER_QUEUE_OPTIONS));

	attachTextBox(IDC_PRIO_HIGHEST_SIZE);
	attachTextBox(IDC_PRIO_NORMAL_SIZE);
	attachTextBox(IDC_PRIO_HIGH_SIZE);
	attachTextBox(IDC_PRIO_LOW_SIZE);
	attachTextBox(IDC_AUTODROP_SPEED);
	attachTextBox(IDC_AUTODROP_ELAPSED);
	attachTextBox(IDC_AUTODROP_MINSOURCES);
	attachTextBox(IDC_AUTODROP_INTERVAL);
	attachTextBox(IDC_AUTODROP_INACTIVITY);
	attachTextBox(IDC_AUTODROP_FILESIZE);
}

QueuePage::~QueuePage() {
}

void QueuePage::write() {
	PropPage::write(handle(), items, 0, 0);
	PropPage::write(handle(), items, optionItems, ::GetDlgItem(handle(), IDC_OTHER_QUEUE_OPTIONS));

	SettingsManager* settings = SettingsManager::getInstance();
	if(SETTING(AUTODROP_INTERVAL) < 1)
		settings->set(SettingsManager::AUTODROP_INTERVAL, 1);
	if(SETTING(AUTODROP_ELAPSED) < 1)
		settings->set(SettingsManager::AUTODROP_ELAPSED, 1);
}

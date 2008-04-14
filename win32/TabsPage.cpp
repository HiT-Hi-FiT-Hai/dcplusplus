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

#include "TabsPage.h"

#include <dcpp/SettingsManager.h>

PropPage::Item TabsPage::items[] = {
	{ 0, 0, PropPage::T_END }
};

PropPage::TextItem TabsPage::texts[] = {
	{ IDC_SETTINGS_BOLD_CONTENTS, N_("Tab highlight on content change") },
	{ 0, 0 }
};

PropPage::ListItem TabsPage::listItems[] = {
	{ SettingsManager::BOLD_HUB, N_("Hub") },
	{ SettingsManager::BOLD_PM, N_("Private message") },
	{ SettingsManager::BOLD_SEARCH, N_("Search") },
	{ SettingsManager::BOLD_SYSTEM_LOG, N_("System Log") },
	{ SettingsManager::BOLD_QUEUE, N_("Download Queue") },
	{ SettingsManager::BOLD_FINISHED_DOWNLOADS, N_("Finished Downloads") },
	{ SettingsManager::BOLD_WAITING_USERS, N_("Waiting Users") },
	{ SettingsManager::BOLD_FINISHED_UPLOADS, N_("Finished Uploads") },
	{ 0, 0 }
};

TabsPage::TabsPage(dwt::Widget* parent) : PropPage(parent) {
	createDialog(IDD_TABSPAGE);
	setHelpId(IDH_TABSPAGE);

	PropPage::translate(handle(), texts);
	PropPage::read(handle(), items, listItems,::GetDlgItem(handle(), IDC_BOLD_BOOLEANS));
}

TabsPage::~TabsPage() {
}

void TabsPage::write() {
	PropPage::write(handle(), items, listItems,::GetDlgItem(handle(), IDC_BOLD_BOOLEANS));
}

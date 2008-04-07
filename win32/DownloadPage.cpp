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

#include "DownloadPage.h"

#include <dcpp/SettingsManager.h>
#include "WinUtil.h"
#include "HubListsDlg.h"

#include <smartwin/widgets/Spinner.h>

static const WinUtil::HelpItem helpItems[] = {
	{ IDC_SETTINGS_DOWNLOAD_DIRECTORY, IDH_SETTINGS_DOWNLOAD_DOWNLOADDIR },
	{ IDC_DOWNLOADDIR, IDH_SETTINGS_DOWNLOAD_DOWNLOADDIR },
	{ IDC_BROWSEDIR, IDH_SETTINGS_DOWNLOAD_DOWNLOADDIR },
	{ IDC_SETTINGS_UNFINISHED_DOWNLOAD_DIRECTORY, IDH_SETTINGS_DOWNLOAD_TEMP_DOWNLOAD_DIRECTORY },
	{ IDC_TEMP_DOWNLOAD_DIRECTORY, IDH_SETTINGS_DOWNLOAD_TEMP_DOWNLOAD_DIRECTORY },
	{ IDC_BROWSETEMPDIR, IDH_SETTINGS_DOWNLOAD_TEMP_DOWNLOAD_DIRECTORY },
	{ IDC_SETTINGS_DOWNLOAD_LIMITS, IDH_SETTINGS_DOWNLOAD_LIMITS },
	{ IDC_DOWNLOADS, IDH_SETTINGS_DOWNLOAD_DOWNLOADS },
	{ IDC_SLOTSSPIN, IDH_SETTINGS_DOWNLOAD_DOWNLOADS },
	{ IDC_SETTINGS_DOWNLOADS_MAX, IDH_SETTINGS_DOWNLOAD_DOWNLOADS },
	{ IDC_MAXSPEED, IDH_SETTINGS_DOWNLOAD_MAXSPEED },
	{ IDC_SPEEDSPIN, IDH_SETTINGS_DOWNLOAD_MAXSPEED },
	{ IDC_SETTINGS_DOWNLOADS_SPEED_PAUSE, IDH_SETTINGS_DOWNLOAD_MAXSPEED },
	{ IDC_SETTINGS_SPEEDS_NOT_ACCURATE, IDH_SETTINGS_DOWNLOAD_LIMITS },
	{ IDC_SETTINGS_PUBLIC_HUB_LIST_HTTP_PROXY, IDH_SETTINGS_DOWNLOAD_PROXY },
	{ IDC_PROXY, IDH_SETTINGS_DOWNLOAD_PROXY },
	{ 0, 0 }
};

PropPage::TextItem DownloadPage::texts[] = {
	{ IDC_SETTINGS_DIRECTORIES, N_("Directories") },
	{ IDC_SETTINGS_DOWNLOAD_DIRECTORY, N_("Default download directory") },
	{ IDC_BROWSEDIR, N_("&Browse...") },
	{ IDC_SETTINGS_UNFINISHED_DOWNLOAD_DIRECTORY, N_("Unfinished downloads directory") },
	{ IDC_BROWSETEMPDIR, N_("Browse...") },
	{ IDC_SETTINGS_DOWNLOAD_LIMITS, N_("Limits") },
	{ IDC_SETTINGS_DOWNLOADS_MAX, N_("Maximum simultaneous downloads (0 = infinite)") },
	{ IDC_SETTINGS_DOWNLOADS_SPEED_PAUSE, N_("No new downloads if speed exceeds (KiB/s, 0 = disable)") },
/*xgettext:no-c-format*/{ IDC_SETTINGS_SPEEDS_NOT_ACCURATE, N_("Note; because of changing download speeds, this is not 100% accurate...") },
	{ IDC_SETTINGS_PUBLIC_HUB_LIST, N_("Public Hubs list") },
	{ IDC_SETTINGS_PUBLIC_HUB_LIST_URL, N_("Public Hubs list URL") },
	{ IDC_SETTINGS_LIST_CONFIG, N_("Configure Public Hub Lists") },
	{ IDC_SETTINGS_PUBLIC_HUB_LIST_HTTP_PROXY, N_("HTTP Proxy (for hublist only)") },
	{ 0, 0 }
};

PropPage::Item DownloadPage::items[] = {
	{ IDC_TEMP_DOWNLOAD_DIRECTORY, SettingsManager::TEMP_DOWNLOAD_DIRECTORY, PropPage::T_STR },
	{ IDC_DOWNLOADDIR,	SettingsManager::DOWNLOAD_DIRECTORY, PropPage::T_STR },
	{ IDC_DOWNLOADS, SettingsManager::DOWNLOAD_SLOTS, PropPage::T_INT },
	{ IDC_MAXSPEED, SettingsManager::MAX_DOWNLOAD_SPEED, PropPage::T_INT },
	{ IDC_PROXY, SettingsManager::HTTP_PROXY, PropPage::T_STR },
	{ 0, 0, PropPage::T_END }
};

DownloadPage::DownloadPage(SmartWin::Widget* parent) : PropPage(parent) {
	createDialog(IDD_DOWNLOADPAGE);
	setHelpId(IDH_DOWNLOADPAGE);

	WinUtil::setHelpIds(this, helpItems);
	PropPage::translate(handle(), texts);
	PropPage::read(handle(), items);

	attachChild<Button>(IDC_BROWSEDIR)->onClicked(std::tr1::bind(&DownloadPage::handleBrowseDir, this));

	attachChild<Button>(IDC_BROWSETEMPDIR)->onClicked(std::tr1::bind(&DownloadPage::handleBrowseTempDir, this));

	attachChild<Button>(IDC_SETTINGS_LIST_CONFIG)->onClicked(std::tr1::bind(&DownloadPage::handleConfigHubLists, this));

	SpinnerPtr spinner = attachChild<Spinner>(IDC_SLOTSSPIN);
	spinner->setRange(0, 100);

	attachChild(spinner, IDC_SPEEDSPIN);
	spinner->setRange(0, 10000);

	attachTextBox(IDC_DOWNLOADDIR);
	attachTextBox(IDC_TEMP_DOWNLOAD_DIRECTORY);
	attachTextBox(IDC_DOWNLOADS);
	attachTextBox(IDC_MAXSPEED);
	attachTextBox(IDC_PROXY);
}

DownloadPage::~DownloadPage() {
}

void DownloadPage::write()
{

	PropPage::write(handle(), items);

	const string& s = SETTING(DOWNLOAD_DIRECTORY);
	if(s.length() > 0 && s[s.length() - 1] != '\\') {
		SettingsManager::getInstance()->set(SettingsManager::DOWNLOAD_DIRECTORY, s + '\\');
	}
	const string& t = SETTING(TEMP_DOWNLOAD_DIRECTORY);
	if(t.length() > 0 && t[t.length() - 1] != '\\') {
		SettingsManager::getInstance()->set(SettingsManager::TEMP_DOWNLOAD_DIRECTORY, t + '\\');
	}

}

void DownloadPage::handleBrowseDir() {
	tstring dir = Text::toT(SETTING(DOWNLOAD_DIRECTORY));
	if(createFolderDialog().open(dir))
	{
		// Adjust path string
		if(dir.size() > 0 && dir[dir.size() - 1] != '\\')
			dir += '\\';

		setItemText(IDC_DOWNLOADDIR, dir);
	}
}

void DownloadPage::handleBrowseTempDir() {
	tstring dir = Text::toT(SETTING(TEMP_DOWNLOAD_DIRECTORY));
	if(createFolderDialog().open(dir))
	{
		// Adjust path string
		if(dir.size() > 0 && dir[dir.size() - 1] != '\\')
			dir += '\\';

		setItemText(IDC_TEMP_DOWNLOAD_DIRECTORY, dir);
	}
}

void DownloadPage::handleConfigHubLists() {
	HubListsDlg(this).run();
}

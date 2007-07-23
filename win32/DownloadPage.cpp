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
#include <dcpp/DCPlusPlus.h>

#include "resource.h"

#include "DownloadPage.h"

#include <dcpp/SettingsManager.h>
#include "WinUtil.h"

PropPage::TextItem DownloadPage::texts[] = {
	{ IDC_SETTINGS_DIRECTORIES, ResourceManager::SETTINGS_DIRECTORIES },
	{ IDC_SETTINGS_DOWNLOAD_DIRECTORY, ResourceManager::SETTINGS_DOWNLOAD_DIRECTORY },
	{ IDC_BROWSEDIR, ResourceManager::BROWSE_ACCEL },
	{ IDC_SETTINGS_UNFINISHED_DOWNLOAD_DIRECTORY, ResourceManager::SETTINGS_UNFINISHED_DOWNLOAD_DIRECTORY },
	{ IDC_BROWSETEMPDIR, ResourceManager::BROWSE },
	{ IDC_SETTINGS_DOWNLOAD_LIMITS, ResourceManager::SETTINGS_DOWNLOAD_LIMITS },
	{ IDC_SETTINGS_DOWNLOADS_MAX, ResourceManager::SETTINGS_DOWNLOADS_MAX },
	{ IDC_SETTINGS_DOWNLOADS_SPEED_PAUSE, ResourceManager::SETTINGS_DOWNLOADS_SPEED_PAUSE },
	{ IDC_SETTINGS_SPEEDS_NOT_ACCURATE, ResourceManager::SETTINGS_SPEEDS_NOT_ACCURATE },
	{ IDC_SETTINGS_PUBLIC_HUB_LIST, ResourceManager::SETTINGS_PUBLIC_HUB_LIST },
	{ IDC_SETTINGS_PUBLIC_HUB_LIST_URL, ResourceManager::SETTINGS_PUBLIC_HUB_LIST_URL },
	{ IDC_SETTINGS_LIST_CONFIG, ResourceManager::SETTINGS_CONFIGURE_HUB_LISTS },
	{ IDC_SETTINGS_PUBLIC_HUB_LIST_HTTP_PROXY, ResourceManager::SETTINGS_PUBLIC_HUB_LIST_HTTP_PROXY },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
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

	PropPage::translate(handle(), texts);
	PropPage::read(handle(), items);

	subclassButton(IDC_BROWSEDIR)->onClicked(std::tr1::bind(&DownloadPage::handleBrowseDir, this));

	subclassButton(IDC_BROWSETEMPDIR)->onClicked(std::tr1::bind(&DownloadPage::handleBrowseTempDir, this));

	subclassButton(IDC_SETTINGS_LIST_CONFIG)->onClicked(std::tr1::bind(&DownloadPage::handleConfigHubLists, this));

	WidgetSpinnerPtr spinner = subclassSpinner(IDC_SLOTSSPIN);
	spinner->setRange(0, 100);

	spinner = subclassSpinner(IDC_SPEEDSPIN);
	spinner->setRange(0, 10000);
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
	if(WinUtil::browseDirectory(dir, handle()))
	{
		// Adjust path string
		if(dir.size() > 0 && dir[dir.size() - 1] != '\\')
			dir += '\\';

		::SetDlgItemText(handle(), IDC_DOWNLOADDIR, dir.c_str());
	}
}

void DownloadPage::handleBrowseTempDir() {
	tstring dir = Text::toT(SETTING(TEMP_DOWNLOAD_DIRECTORY));
	if(WinUtil::browseDirectory(dir, handle()))
	{
		// Adjust path string
		if(dir.size() > 0 && dir[dir.size() - 1] != '\\')
			dir += '\\';

		::SetDlgItemText(handle(), IDC_TEMP_DOWNLOAD_DIRECTORY, dir.c_str());
	}
}

void DownloadPage::handleConfigHubLists() {
#ifdef PORT_ME
	PublicHubListDlg dlg;
	dlg.DoModal(m_hWnd);
#endif
}

#ifdef PORT_ME

LRESULT DownloadPage::onHelpInfo(LPNMHDR /*pnmh*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_DOWNLOADPAGE);
	return 0;
}

LRESULT DownloadPage::onHelp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_DOWNLOADPAGE);
	return 0;
}
#endif

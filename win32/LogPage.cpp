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

#include "LogPage.h"

#include <dcpp/SettingsManager.h>

#include <dcpp/LogManager.h>
#include "WinUtil.h"

PropPage::TextItem LogPage::texts[] = {
	{ IDC_SETTINGS_LOGGING, ResourceManager::SETTINGS_LOGGING },
	{ IDC_SETTINGS_LOG_DIR, ResourceManager::DIRECTORY },
	{ IDC_BROWSE_LOG, ResourceManager::BROWSE_ACCEL },
	{ IDC_SETTINGS_FORMAT, ResourceManager::SETTINGS_FORMAT },
	{ IDC_SETTINGS_FILE_NAME, ResourceManager::SETTINGS_FILE_NAME },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

PropPage::Item LogPage::items[] = {
	{ IDC_LOG_DIRECTORY, SettingsManager::LOG_DIRECTORY, PropPage::T_STR },
	{ 0, 0, PropPage::T_END }
};

PropPage::ListItem LogPage::listItems[] = {
	{ SettingsManager::LOG_MAIN_CHAT, ResourceManager::SETTINGS_LOG_MAIN_CHAT },
	{ SettingsManager::LOG_PRIVATE_CHAT, ResourceManager::SETTINGS_LOG_PRIVATE_CHAT },
	{ SettingsManager::LOG_DOWNLOADS, ResourceManager::SETTINGS_LOG_DOWNLOADS },
	{ SettingsManager::LOG_UPLOADS, ResourceManager::SETTINGS_LOG_UPLOADS },
	{ SettingsManager::LOG_SYSTEM, ResourceManager::SETTINGS_LOG_SYSTEM_MESSAGES },
	{ SettingsManager::LOG_STATUS_MESSAGES, ResourceManager::SETTINGS_LOG_STATUS_MESSAGES },
	{ SettingsManager::LOG_FILELIST_TRANSFERS, ResourceManager::SETTINGS_LOG_FILELIST_TRANSFERS },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

LogPage::LogPage(SmartWin::Widget* parent) : PropPage(parent) {
	createDialog(IDD_LOGPAGE);

	PropPage::translate(handle(), texts);
	PropPage::read(handle(), items, listItems, ::GetDlgItem(handle(), IDC_LOG_OPTIONS));

	for(int i = 0; i < LogManager::LAST; ++i) {
		TStringPair pair;
		pair.first = Text::toT(LogManager::getInstance()->getSetting(i, LogManager::FILE));
		pair.second = Text::toT(LogManager::getInstance()->getSetting(i, LogManager::FORMAT));
		options.push_back(pair);
	}

	::EnableWindow(::GetDlgItem(handle(), IDC_LOG_FORMAT), false);
	::EnableWindow(::GetDlgItem(handle(), IDC_LOG_FILE), false);

	oldSelection = -1;

	subclassButton(IDC_BROWSE_LOG)->onClicked(std::tr1::bind(&LogPage::handleBrowseClicked, this));

	WidgetDataGridPtr dataGrid = subclassList(IDC_LOG_OPTIONS);
	dataGrid->onRaw(std::tr1::bind(&LogPage::handleItemChanged, this, dataGrid, _1, _2), SmartWin::Message(WM_NOTIFY, LVN_ITEMCHANGED));
}

LogPage::~LogPage() {
}

void LogPage::write()
{
	PropPage::write(handle(), items, listItems, ::GetDlgItem(handle(), IDC_LOG_OPTIONS));

	const string& s = SETTING(LOG_DIRECTORY);
	if(s.length() > 0 && s[s.length() - 1] != '\\') {
		SettingsManager::getInstance()->set(SettingsManager::LOG_DIRECTORY, s + '\\');
	}
	File::ensureDirectory(SETTING(LOG_DIRECTORY));

	//make sure we save the last edit too, the user
	//might not have changed the selection
	getValues();

	for(int i = 0; i < LogManager::LAST; ++i) {
		string tmp = Text::fromT(options[i].first);
		if(Util::stricmp(Util::getFileExt(tmp), ".log") != 0)
			tmp += ".log";

		LogManager::getInstance()->saveSetting(i, LogManager::FILE, tmp);
		LogManager::getInstance()->saveSetting(i, LogManager::FORMAT, Text::fromT(options[i].second));
	}
}

void LogPage::handleBrowseClicked() {
	tstring dir = Text::toT(SETTING(LOG_DIRECTORY));
	if(WinUtil::browseDirectory(dir, handle()))
	{
		// Adjust path string
		if(dir.size() > 0 && dir[dir.size() - 1] != '\\')
			dir += '\\';

		::SetDlgItemText(handle(), IDC_LOG_DIRECTORY, dir.c_str());
	}
}

HRESULT LogPage::handleItemChanged(WidgetDataGridPtr dataGrid, WPARAM wParam, LPARAM lParam) {
	getValues();

	int sel = dataGrid->getSelectedIndex();

	if(sel >= 0 && sel < LogManager::LAST) {
		BOOL checkState = dataGrid->getIsRowChecked(sel) ? TRUE : FALSE;
		::EnableWindow(::GetDlgItem(handle(), IDC_LOG_FORMAT), checkState);
		::EnableWindow(::GetDlgItem(handle(), IDC_LOG_FILE), checkState);

		::SetDlgItemText(handle(), IDC_LOG_FILE, options[sel].first.c_str());
		::SetDlgItemText(handle(), IDC_LOG_FORMAT, options[sel].second.c_str());

		//save the old selection so we know where to save the values
		oldSelection = sel;
	} else {
		::EnableWindow(::GetDlgItem(handle(), IDC_LOG_FORMAT), FALSE);
		::EnableWindow(::GetDlgItem(handle(), IDC_LOG_FILE), FALSE);

		::SetDlgItemText(handle(), IDC_LOG_FILE, _T(""));
		::SetDlgItemText(handle(), IDC_LOG_FORMAT, _T(""));
	}

	return 0;
}

void LogPage::getValues() {
	if(oldSelection >= 0) {
		TCHAR buf[512];

		if(::GetDlgItemText(handle(), IDC_LOG_FILE, buf, 512) > 0)
			options[oldSelection].first = buf;
		if(::GetDlgItemText(handle(), IDC_LOG_FORMAT, buf, 512) > 0)
			options[oldSelection].second = buf;
	}
}

#ifdef PORT_ME

LRESULT LogPage::onHelpInfo(LPNMHDR /*pnmh*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_LOGPAGE);
	return 0;
}

LRESULT LogPage::onHelp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_LOGPAGE);
	return 0;
}
#endif

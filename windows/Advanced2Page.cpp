/* 
 * Copyright (C) 2001-2004 Jacek Sieka, j_s at telia com
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

#include "Advanced2Page.h"
#include "../client/SettingsManager.h"
#include "WinUtil.h"

PropPage::TextItem Advanced2Page::texts[] = {
	{ IDC_SETTINGS_LOGGING, ResourceManager::SETTINGS_LOGGING },
	{ IDC_SETTINGS_LOG_DIR, ResourceManager::DIRECTORY},
	{ IDC_BROWSE_LOG, ResourceManager::BROWSE_ACCEL },
	{ IDC_LOG_MAIN_CHAT, ResourceManager::SETTINGS_LOG_MAIN_CHAT },
	{ IDC_LOG_PRIVATE_CHAT, ResourceManager::SETTINGS_LOG_PRIVATE_CHAT },
	{ IDC_LOG_DOWNLOADS, ResourceManager::SETTINGS_LOG_DOWNLOADS }, 
	{ IDC_LOG_UPLOADS, ResourceManager::SETTINGS_LOG_UPLOADS },
	{ IDC_LOG_SYSTEM, ResourceManager::SETTINGS_LOG_SYSTEM_MESSAGES },
	{ IDC_SETTINGS_FORMAT1, ResourceManager::SETTINGS_FORMAT },
	{ IDC_SETTINGS_FORMAT2, ResourceManager::SETTINGS_FORMAT },
	{ IDC_SETTINGS_FORMAT3, ResourceManager::SETTINGS_FORMAT },
	{ IDC_SETTINGS_FORMAT4, ResourceManager::SETTINGS_FORMAT },
	{ IDC_LOG_STATUS_MESSAGES, ResourceManager::SETTINGS_LOG_STATUS_MESSAGES },
	{ IDC_SETTINGS_FILE_NAME1, ResourceManager::SETTINGS_FILE_NAME },
	{ IDC_SETTINGS_FILE_NAME2, ResourceManager::SETTINGS_FILE_NAME },
	{ IDC_SETTINGS_FILE_NAME3, ResourceManager::SETTINGS_FILE_NAME },
	{ IDC_SETTINGS_FILE_NAME4, ResourceManager::SETTINGS_FILE_NAME },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

PropPage::Item Advanced2Page::items[] = {
	{ IDC_LOG_MAIN_CHAT, SettingsManager::LOG_MAIN_CHAT, PropPage::T_BOOL },
	{ IDC_LOG_PRIVATE_CHAT, SettingsManager::LOG_PRIVATE_CHAT, PropPage::T_BOOL },
	{ IDC_LOG_DOWNLOADS, SettingsManager::LOG_DOWNLOADS, PropPage::T_BOOL },
	{ IDC_LOG_UPLOADS, SettingsManager::LOG_UPLOADS, PropPage::T_BOOL },
	{ IDC_LOG_DIRECTORY, SettingsManager::LOG_DIRECTORY, PropPage::T_STR },
	{ IDC_LOG_SYSTEM, SettingsManager::LOG_SYSTEM, PropPage::T_BOOL },
	{ IDC_POST_DOWNLOAD, SettingsManager::LOG_FORMAT_POST_DOWNLOAD, PropPage::T_STR },
	{ IDC_POST_UPLOAD, SettingsManager::LOG_FORMAT_POST_UPLOAD, PropPage::T_STR },
	{ IDC_MAIN_CHAT, SettingsManager::LOG_FORMAT_MAIN_CHAT, PropPage::T_STR },
	{ IDC_PRIVATE_CHAT, SettingsManager::LOG_FORMAT_PRIVATE_CHAT, PropPage::T_STR },
	{ IDC_LOG_STATUS_MESSAGES, SettingsManager::LOG_STATUS_MESSAGES, PropPage::T_BOOL },
	{ IDC_MAIN_CHAT_FILE, SettingsManager::LOG_FILE_MAIN_CHAT, PropPage::T_STR },
	{ IDC_PRIVATE_CHAT_FILE, SettingsManager::LOG_FILE_PRIVATE_CHAT, PropPage::T_STR },
	{ IDC_UPLOAD_FILE, SettingsManager::LOG_FILE_UPLOAD, PropPage::T_STR },
	{ IDC_DOWNLOAD_FILE, SettingsManager::LOG_FILE_DOWNLOAD, PropPage::T_STR },
	{ 0, 0, PropPage::T_END }
};

LRESULT Advanced2Page::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	PropPage::translate((HWND)(*this), texts);
	PropPage::read((HWND)*this, items);

	updateEdits();
	// Do specialized reading here
	return TRUE;
}

void Advanced2Page::updateEdits() {
	bool btnState = (IsDlgButtonChecked(IDC_LOG_MAIN_CHAT) != 0);
	::EnableWindow(GetDlgItem(IDC_MAIN_CHAT), btnState);
	::EnableWindow(GetDlgItem(IDC_SETTINGS_FORMAT1), btnState);
	::EnableWindow(GetDlgItem(IDC_SETTINGS_FILE_NAME1), btnState);
	btnState = (IsDlgButtonChecked(IDC_LOG_PRIVATE_CHAT) != 0);
	::EnableWindow(GetDlgItem(IDC_PRIVATE_CHAT), btnState);
	::EnableWindow(GetDlgItem(IDC_SETTINGS_FORMAT2), btnState);
	::EnableWindow(GetDlgItem(IDC_SETTINGS_FILE_NAME2), btnState);
	btnState = (IsDlgButtonChecked(IDC_LOG_DOWNLOADS) != 0);
	::EnableWindow(GetDlgItem(IDC_POST_DOWNLOAD), btnState);
	::EnableWindow(GetDlgItem(IDC_SETTINGS_FORMAT3), btnState);
	::EnableWindow(GetDlgItem(IDC_SETTINGS_FILE_NAME4), btnState);
	btnState = (IsDlgButtonChecked(IDC_LOG_UPLOADS) != 0);
	::EnableWindow(GetDlgItem(IDC_POST_UPLOAD), btnState);
	::EnableWindow(GetDlgItem(IDC_SETTINGS_FORMAT4), btnState);
	::EnableWindow(GetDlgItem(IDC_SETTINGS_FILE_NAME3), btnState);
}

void Advanced2Page::write()
{
	PropPage::write((HWND)*this, items);

	const string& s = SETTING(LOG_DIRECTORY);
	if(s.length() > 0 && s[s.length() - 1] != '\\') {
		SettingsManager::getInstance()->set(SettingsManager::LOG_DIRECTORY, s + '\\');
	}
	File::ensureDirectory(SETTING(LOG_DIRECTORY));
	// Do specialized writing here
	// settings->set(XX, YY);
	// Since the dir might change while running we don't call
	// File::ensureDirectory() here
	const string& t = SETTING(LOG_FILE_MAIN_CHAT);
	if(Util::getFileExt(Util::getFileName(t)) == Util::emptyString) {
		SettingsManager::getInstance()->set(SettingsManager::LOG_FILE_MAIN_CHAT, t + ".log");
	} else {
		SettingsManager::getInstance()->set(SettingsManager::LOG_FILE_MAIN_CHAT, t);
	}
	string tmp = t;
	string::size_type i = tmp.rfind('.');
	tmp.insert(i, "_status");
	SettingsManager::getInstance()->set(SettingsManager::LOG_FILE_STATUS, tmp);
	
	const string& u = SETTING(LOG_FILE_PRIVATE_CHAT);
	if(Util::getFileExt(Util::getFileName(u)) == Util::emptyString)
		SettingsManager::getInstance()->set(SettingsManager::LOG_FILE_PRIVATE_CHAT, u + ".log");
	else
		SettingsManager::getInstance()->set(SettingsManager::LOG_FILE_PRIVATE_CHAT, u);

	const string& w = SETTING(LOG_FILE_UPLOAD);
	if(Util::getFileExt(Util::getFileName(w)) == Util::emptyString)
		SettingsManager::getInstance()->set(SettingsManager::LOG_FILE_UPLOAD, w + ".log");
	else
		SettingsManager::getInstance()->set(SettingsManager::LOG_FILE_UPLOAD, w);

	const string& x = SETTING(LOG_FILE_DOWNLOAD);
	if(Util::getFileExt(Util::getFileName(x)) == Util::emptyString)
		SettingsManager::getInstance()->set(SettingsManager::LOG_FILE_DOWNLOAD, x + ".log");
	else
		SettingsManager::getInstance()->set(SettingsManager::LOG_FILE_DOWNLOAD, x);
}

LRESULT Advanced2Page::onClickedBrowseDir(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	tstring dir = Text::toT(SETTING(LOG_DIRECTORY));
	if(WinUtil::browseDirectory(dir, m_hWnd))
	{
		// Adjust path string
		if(dir.size() > 0 && dir[dir.size() - 1] != '\\')
			dir += '\\';
		
		SetDlgItemText(IDC_LOG_DIRECTORY, dir.c_str());
	}
	return 0;
}

LRESULT Advanced2Page::onHelpInfo(LPNMHDR /*pnmh*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_ADVANCED2PAGE);
	return 0;
}

LRESULT Advanced2Page::onHelp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_ADVANCED2PAGE);
	return 0;
}

/**
 * @file
 * $Id: Advanced2Page.cpp,v 1.17 2004/12/05 15:51:03 arnetheduck Exp $
 */


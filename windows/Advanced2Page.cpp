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

#include "Advanced2Page.h"
#include "../client/SettingsManager.h"
#include "WinUtil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

PropPage::Item Advanced2Page::items[] = {
	{ IDC_LOG_MAIN_CHAT, SettingsManager::LOG_MAIN_CHAT, PropPage::T_BOOL },
	{ IDC_LOG_PRIVATE_CHAT, SettingsManager::LOG_PRIVATE_CHAT, PropPage::T_BOOL },
	{ IDC_LOG_DOWNLOADS, SettingsManager::LOG_DOWNLOADS, PropPage::T_BOOL },
	{ IDC_LOG_UPLOADS, SettingsManager::LOG_UPLOADS, PropPage::T_BOOL },
	{ IDC_LOG_DIRECTORY, SettingsManager::LOG_DIRECTORY, PropPage::T_STR },
	{ IDC_PRIVATE_MESSAGE_BEEP, SettingsManager::PRIVATE_MESSAGE_BEEP, PropPage::T_BOOL },
	{ IDC_PRIVATE_MESSAGE_BEEP_OPEN, SettingsManager::PRIVATE_MESSAGE_BEEP_OPEN, PropPage::T_BOOL },
	{ IDC_POST_DOWNLOAD, SettingsManager::LOG_FORMAT_POST_DOWNLOAD, PropPage::T_STR },
	{ IDC_POST_UPLOAD, SettingsManager::LOG_FORMAT_POST_UPLOAD, PropPage::T_STR },
	{ IDC_MAIN_CHAT, SettingsManager::LOG_FORMAT_MAIN_CHAT, PropPage::T_STR },
	{ IDC_PRIVATE_CHAT, SettingsManager::LOG_FORMAT_PRIVATE_CHAT, PropPage::T_STR },
	{ 0, 0, PropPage::T_END }
};

LRESULT Advanced2Page::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	PropPage::read((HWND)*this, items);

	updateEdits();
	// Do specialized reading here
	return TRUE;
}

void Advanced2Page::updateEdits() {
	CButton btn = ::GetDlgItem(m_hWnd, IDC_LOG_MAIN_CHAT);
	::EnableWindow(::GetDlgItem(m_hWnd, IDC_MAIN_CHAT), btn.GetCheck());
	btn = ::GetDlgItem(m_hWnd, IDC_LOG_PRIVATE_CHAT);
	::EnableWindow(::GetDlgItem(m_hWnd, IDC_PRIVATE_CHAT), btn.GetCheck());
	btn = ::GetDlgItem(m_hWnd, IDC_LOG_DOWNLOADS);
	::EnableWindow(::GetDlgItem(m_hWnd, IDC_POST_DOWNLOAD), btn.GetCheck());
	btn = ::GetDlgItem(m_hWnd, IDC_LOG_UPLOADS);
	::EnableWindow(::GetDlgItem(m_hWnd, IDC_POST_UPLOAD), btn.GetCheck());
}

void Advanced2Page::write()
{
	PropPage::write((HWND)*this, items);

	const string& s = SETTING(LOG_DIRECTORY);
	if(s.length() > 0 && s[s.length() - 1] != '\\') {
		SettingsManager::getInstance()->set(SettingsManager::LOG_DIRECTORY, s + '\\');
	}
	Util::ensureDirectory(SETTING(LOG_DIRECTORY));
	// Do specialized writing here
	// settings->set(XX, YY);
}

LRESULT Advanced2Page::onClickedBrowseDir(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	string dir;
	if(WinUtil::browseDirectory(dir))
	{
		// Adjust path string
		if(dir.size() > 0 && dir[dir.size() - 1] != '\\')
			dir += '\\';
		
		SetDlgItemText(IDC_LOG_DIRECTORY, dir.c_str());
	}
	return 0;
}

/**
 * @file
 * $Id: Advanced2Page.cpp,v 1.6 2003/04/15 10:14:00 arnetheduck Exp $
 */


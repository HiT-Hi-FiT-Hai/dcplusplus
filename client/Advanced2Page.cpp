/* 
 * Copyright (C) 2001 Jacek Sieka, j_s@telia.com
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
#include "dcplusplus.h"
#include "Advanced2Page.h"
#include "SettingsManager.h"

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
	{ 0, 0, PropPage::T_END }
};

LRESULT Advanced2Page::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	PropPage::read((HWND)*this, items);

	// Do specialized reading here
	return TRUE;
}

void Advanced2Page::write()
{
	PropPage::write((HWND)*this, items);

	const string& s = SETTING(LOG_DIRECTORY);
	if(s.length() > 0 && s[s.length() - 1] != '\\') {
		SettingsManager::getInstance()->set(SettingsManager::LOG_DIRECTORY, s + '\\');
	}
	// Do specialized writing here
	// settings->set(XX, YY);
}

LRESULT Advanced2Page::onClickedBrowseDir(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	string dir;
	if(Util::browseDirectory(dir))
	{
		// Adjust path string
		if(dir.size() > 0 && dir[dir.size() - 1] != '\\')
			dir += '\\';
		
		SetDlgItemText(IDC_LOG_DIRECTORY, dir.c_str());
	}
	return 0;
}

/**
 * @file Advanced2Page.cpp
 * $Id: Advanced2Page.cpp,v 1.1 2002/04/03 23:20:35 arnetheduck Exp $
 */


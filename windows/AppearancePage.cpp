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

#include "AppearancePage.h"
#include "../client/SettingsManager.h"
#include "../client/StringTokenizer.h"

#include "WinUtil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

PropPage::Item AppearancePage::items[] = {
	{ IDC_FULLROW, SettingsManager::FULL_ROW_SELECT, PropPage::T_BOOL },
	{ IDC_FILTERKICK, SettingsManager::FILTER_MESSAGES, PropPage::T_BOOL }, 
	{ IDC_MINIMIZETRAY, SettingsManager::MINIMIZE_TRAY, PropPage::T_BOOL },
	{ IDC_TIMESTAMPS, SettingsManager::TIME_STAMPS, PropPage::T_BOOL },
	{ IDC_CONFIRMEXIT, SettingsManager::CONFIRM_EXIT, PropPage::T_BOOL },
	{ IDC_DEFAULT_AWAY_MESSAGE, SettingsManager::DEFAULT_AWAY_MESSAGE, PropPage::T_STR },
	{ IDC_LANGUAGE, SettingsManager::LANGUAGE_FILE, PropPage::T_STR },
	{ IDC_STATUS_IN_CHAT, SettingsManager::STATUS_IN_CHAT, PropPage::T_BOOL },
	{ IDC_SHOW_JOINS, SettingsManager::SHOW_JOINS, PropPage::T_BOOL },
	{ IDC_USE_SYSTEM_ICONS, SettingsManager::USE_SYSTEM_ICONS, PropPage::T_BOOL },
	{ 0, 0, PropPage::T_END }
};

PropPage::ListItem AppearancePage::listItems[] = {
	{ SettingsManager::FULL_ROW_SELECT, ResourceManager::SETTINGS_APPEARANCE_FULL_ROW_SELECT },
	{ SettingsManager::FILTER_MESSAGES, ResourceManager::SETTINGS_APPEARANCE_FILTER_MESSAGES },
	{ SettingsManager::MINIMIZE_TRAY, ResourceManager::SETTINGS_APPEARANCE_MINIMIZE_TRAY },
	{ SettingsManager::TIME_STAMPS, ResourceManager::SETTINGS_APPEARANCE_TIME_STAMPS },
	{ SettingsManager::CONFIRM_EXIT, ResourceManager::SETTINGS_APPEARANCE_CONFIRM_EXIT },
	{ SettingsManager::STATUS_IN_CHAT, ResourceManager::SETTINGS_APPEARANCE_STATUS_IN_CHAT },
	{ SettingsManager::SHOW_JOINS, ResourceManager::SETTINGS_APPEARANCE_SHOW_JOINS },
	{ SettingsManager::USE_SYSTEM_ICONS, ResourceManager::SETTINGS_APPEARANCE_USE_SYSTEM_ICONS },
	{ 0, ResourceManager::SETTINGS_ADVANCED_AUTO_AWAY }
};

AppearancePage::~AppearancePage()
{
	::DeleteObject(bgbrush);
	::DeleteObject(fontObj);
}

LRESULT AppearancePage::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	ctrlExample.Attach(GetDlgItem(IDC_COLOREXAMPLE));

	PropPage::read((HWND)*this, items, listItems, GetDlgItem(IDC_APPEARANCE_BOOLEANS));
	WinUtil::decodeFont(SETTING(TEXT_FONT), font);

	// Do specialized reading here
	fg = SETTING(TEXT_COLOR);
	bg = SETTING(BACKGROUND_COLOR);
	bgbrush = ::CreateSolidBrush(bg);
	fontObj = ::CreateFontIndirect(&font);
	return TRUE;
}

void AppearancePage::write()
{
	PropPage::write((HWND)*this, items, listItems, GetDlgItem(IDC_APPEARANCE_BOOLEANS));

	settings->set(SettingsManager::TEXT_COLOR, (int)fg);
	settings->set(SettingsManager::BACKGROUND_COLOR, (int)bg);

	string f = WinUtil::encodeFont(font);
	settings->set(SettingsManager::TEXT_FONT, f);
}

LRESULT AppearancePage::onClickedBackground(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CColorDialog d(SETTING(BACKGROUND_COLOR), 0, *this);
	if(d.DoModal() == IDOK)
	{
		::DeleteObject(bgbrush);
		bg = d.GetColor();
		bgbrush = CreateSolidBrush(bg);
		ctrlExample.Invalidate();
	}
	return TRUE;
}


LRESULT AppearancePage::onClickedText(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	LOGFONT tmp = font;
	CFontDialog d(&tmp, CF_EFFECTS | CF_SCREENFONTS, NULL, *this);
	d.m_cf.rgbColors = fg;
	if(d.DoModal() == IDOK)
	{
		font = tmp;
		fg = d.GetColor();
		::DeleteObject(fontObj);
		fontObj = ::CreateFontIndirect(&font);
		ctrlExample.Invalidate();
	}
	return TRUE;
}

LRESULT AppearancePage::onCtlColor(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	HWND hwnd = (HWND)lParam;

	if(hwnd == (HWND)ctrlExample)
	{
		HDC hdc = (HDC)wParam;
		::SetBkMode(hdc, TRANSPARENT);
		::SetTextColor(hdc, fg);
		::SelectObject(hdc, fontObj);
		return (LRESULT)bgbrush;
	}
	else
		return FALSE;
}

LRESULT AppearancePage::onBrowse(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	char buf[MAX_PATH];

	GetDlgItemText(IDC_LANGUAGE, buf, MAX_PATH);
	string x = buf;

	if(WinUtil::browseFile(x, m_hWnd, false) == IDOK) {
		SetDlgItemText(IDC_LANGUAGE, x.c_str());
	}
	return 0;
}

/**
 * @file AppearancePage.cpp
 * $Id: AppearancePage.cpp,v 1.4 2003/03/13 13:31:45 arnetheduck Exp $
 */

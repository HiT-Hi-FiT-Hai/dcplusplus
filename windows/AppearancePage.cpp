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

PropPage::TextItem AppearancePage::texts[] = {
	{ IDC_SETTINGS_COLORS, ResourceManager::SETTINGS_COLORS },
	{ IDC_SELWINCOLOR, ResourceManager::SETTINGS_SELECT_WINDOW_COLOR },
	{ IDC_SELTEXT, ResourceManager::SETTINGS_SELECT_TEXT_FACE },
	{ IDC_COLOREXAMPLE, ResourceManager::SETTINGS_EXAMPLE_TEXT },
	{ IDC_SETTINGS_APPEARANCE_OPTIONS, ResourceManager::SETTINGS_OPTIONS },
	{ IDC_SETTINGS_DEFAULT_AWAY_MSG, ResourceManager::SETTINGS_DEFAULT_AWAY_MSG },
	{ IDC_SETTINGS_LANGUAGE_FILE, ResourceManager::SETTINGS_LANGUAGE_FILE },
	{ IDC_BROWSE, ResourceManager::BROWSE_ACCEL },
	{ IDC_SETTINGS_REQUIRES_RESTART, ResourceManager::SETTINGS_REQUIRES_RESTART },
	{ IDC_SETTINGS_UPLOAD_BAR_COLOR, ResourceManager::UPLOADS },
	{ IDC_SETTINGS_DOWNLOAD_BAR_COLOR, ResourceManager::DOWNLOADS }, 
	{ IDC_SETTINGS_GET_USER_COUNTRY, ResourceManager::SETTINGS_GET_USER_COUNTRY }, 
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

PropPage::Item AppearancePage::items[] = {
	{ IDC_DEFAULT_AWAY_MESSAGE, SettingsManager::DEFAULT_AWAY_MESSAGE, PropPage::T_STR },
	{ IDC_LANGUAGE, SettingsManager::LANGUAGE_FILE, PropPage::T_STR },
	{ 0, 0, PropPage::T_END }
};

PropPage::ListItem AppearancePage::listItems[] = {
	{ SettingsManager::FILTER_MESSAGES, ResourceManager::SETTINGS_FILTER_MESSAGES },
	{ SettingsManager::MINIMIZE_TRAY, ResourceManager::SETTINGS_MINIMIZE_TRAY },
	{ SettingsManager::TIME_STAMPS, ResourceManager::SETTINGS_TIME_STAMPS },
	{ SettingsManager::CONFIRM_EXIT, ResourceManager::SETTINGS_CONFIRM_EXIT },
	{ SettingsManager::STATUS_IN_CHAT, ResourceManager::SETTINGS_STATUS_IN_CHAT },
	{ SettingsManager::SHOW_JOINS, ResourceManager::SETTINGS_SHOW_JOINS },
	{ SettingsManager::FAV_SHOW_JOINS, ResourceManager::SETTINGS_FAV_SHOW_JOINS },
	{ SettingsManager::USE_SYSTEM_ICONS, ResourceManager::SETTINGS_USE_SYSTEM_ICONS },
	{ SettingsManager::USE_OEM_MONOFONT, ResourceManager::SETTINGS_USE_OEM_MONOFONT },
	{ SettingsManager::FINISHED_DIRTY, ResourceManager::SETTINGS_FINISHED_DIRTY },
	{ SettingsManager::QUEUE_DIRTY, ResourceManager::SETTINGS_QUEUE_DIRTY },
	{ SettingsManager::TAB_DIRTY, ResourceManager::SETTINGS_TAB_DIRTY },
	{ SettingsManager::GET_USER_COUNTRY, ResourceManager::SETTINGS_GET_USER_COUNTRY },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

AppearancePage::~AppearancePage()
{
	::DeleteObject(bgbrush);
	::DeleteObject(fontObj);
}

LRESULT AppearancePage::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	PropPage::translate((HWND)(*this), texts);
	ctrlExample.Attach(GetDlgItem(IDC_COLOREXAMPLE));

	PropPage::read((HWND)*this, items, listItems, GetDlgItem(IDC_APPEARANCE_BOOLEANS));
	WinUtil::decodeFont(SETTING(TEXT_FONT), font);

	// Do specialized reading here
	fg = SETTING(TEXT_COLOR);
	bg = SETTING(BACKGROUND_COLOR);
	bgbrush = ::CreateSolidBrush(bg);
	fontObj = ::CreateFontIndirect(&font);
	upBar = SETTING(UPLOAD_BAR_COLOR);
	downBar = SETTING(DOWNLOAD_BAR_COLOR);
	return TRUE;
}

void AppearancePage::write()
{
	PropPage::write((HWND)*this, items, listItems, GetDlgItem(IDC_APPEARANCE_BOOLEANS));

	settings->set(SettingsManager::TEXT_COLOR, (int)fg);
	settings->set(SettingsManager::BACKGROUND_COLOR, (int)bg);
	settings->set(SettingsManager::UPLOAD_BAR_COLOR, (int)upBar);
	settings->set(SettingsManager::DOWNLOAD_BAR_COLOR, (int)downBar);

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
	static const char types[] = "Language Files\0*.xml\0All Files\0*.*\0";

	GetDlgItemText(IDC_LANGUAGE, buf, MAX_PATH);
	string x = buf;

	if(WinUtil::browseFile(x, m_hWnd, false, Util::getAppPath(), types) == IDOK) {
		SetDlgItemText(IDC_LANGUAGE, x.c_str());
	}
	return 0;
}

LRESULT AppearancePage::onPickColor(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	switch (wID) {
		case IDC_SETTINGS_UPLOAD_BAR_COLOR: 
			{
				CColorDialog colPicker(upBar, 0, *this);
				if(colPicker.DoModal() == IDOK) 
				{
					upBar = colPicker.GetColor();
				}
			}
			break;
		case IDC_SETTINGS_DOWNLOAD_BAR_COLOR:
			{
				CColorDialog colPicker(downBar, 0, *this);
				if(colPicker.DoModal() == IDOK) 
				{
					downBar = colPicker.GetColor();
				}
			}
			break;
		default:
			break;
	};
	return true;
}

/**
 * @file
 * $Id: AppearancePage.cpp,v 1.16 2004/07/26 20:01:21 arnetheduck Exp $
 */

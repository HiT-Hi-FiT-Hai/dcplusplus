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
#include "AppearancePage.h"
#include "SettingsManager.h"
#include "StringTokenizer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

PropPage::Item AppearancePage::items[] = {
	{ IDC_FULLROW, SettingsManager::FULL_ROW_SELECT, PropPage::T_BOOL },
	{ IDC_FILTERKICK, SettingsManager::FILTER_KICKMSGS, PropPage::T_BOOL }, 
	{ IDC_MINIMIZETRAY, SettingsManager::MINIMIZE_TRAY, PropPage::T_BOOL },
	{ IDC_TIMESTAMPS, SettingsManager::TIME_STAMPS, PropPage::T_BOOL },
	{ IDC_CONFIRMEXIT, SettingsManager::CONFIRM_EXIT, PropPage::T_BOOL },
	{ IDC_LANGUAGE, SettingsManager::LANGUAGE_FILE, PropPage::T_STR },
	{ IDC_STATUS_IN_CHAT, SettingsManager::STATUS_IN_CHAT, PropPage::T_BOOL },
	{ IDC_SHOW_JOINS, SettingsManager::SHOW_JOINS, PropPage::T_BOOL },
	{ 0, 0, PropPage::T_END }
};

AppearancePage::~AppearancePage()
{
	::DeleteObject(bgbrush);
	::DeleteObject(fontObj);
}

LRESULT AppearancePage::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	ctrlExample.Attach(GetDlgItem(IDC_COLOREXAMPLE));

	PropPage::read((HWND)*this, items);
	Util::decodeFont(SETTING(TEXT_FONT), font);

	// Do specialized reading here
	fg = SETTING(TEXT_COLOR);
	bg = SETTING(BACKGROUND_COLOR);
	bgbrush = ::CreateSolidBrush(bg);
	fontObj = ::CreateFontIndirect(&font);
	return TRUE;
}

void AppearancePage::write()
{
	PropPage::write((HWND)*this, items);

	settings->set(SettingsManager::TEXT_COLOR, (int)fg);
	settings->set(SettingsManager::BACKGROUND_COLOR, (int)bg);

	string f = Util::encodeFont(font);
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

	if(Util::browseFile(x, m_hWnd, false) == IDOK) {
		SetDlgItemText(IDC_LANGUAGE, x.c_str());
	}
	return 0;
}

/**
 * @file AppearancePage.cpp
 * $Id: AppearancePage.cpp,v 1.7 2002/04/07 16:08:14 arnetheduck Exp $
 * @if LOG
 * $Log: AppearancePage.cpp,v $
 * Revision 1.7  2002/04/07 16:08:14  arnetheduck
 * Fixes and additions
 *
 * Revision 1.6  2002/03/15 15:12:35  arnetheduck
 * 0.16
 *
 * Revision 1.5  2002/03/13 20:35:25  arnetheduck
 * Release canditate...internationalization done as far as 0.155 is concerned...
 * Also started using mirrors of the public hub lists
 *
 * Revision 1.4  2002/02/25 15:39:28  arnetheduck
 * Release 0.154, lot of things fixed...
 *
 * Revision 1.3  2002/02/09 18:13:51  arnetheduck
 * Fixed level 4 warnings and started using new stl
 *
 * Revision 1.2  2002/01/26 21:09:51  arnetheduck
 * Release 0.14
 *
 * Revision 1.1  2002/01/26 16:34:00  arnetheduck
 * Colors dialog added, as well as some other options
 *
 * Revision 1.4  2002/01/26 12:52:51  arnetheduck
 * More minor fixes
 *
 * @endif
 */

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

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

PropPage::Item AppearancePage::items[] = {
	{ 0, 0, PropPage::T_END }
};

AppearancePage::AppearancePage(SettingsManager *s) : PropPage(s)
{
}

AppearancePage::~AppearancePage()
{
	::DeleteObject(bgbrush);
}

LRESULT AppearancePage::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	ctrlExample.Attach(GetDlgItem(IDC_COLOREXAMPLE));

	PropPage::read((HWND)*this, items);

	// Do specialized reading here
	fg = SETTING(TEXT_COLOR);
	bg = SETTING(BACKGROUND_COLOR);
	bgbrush = ::CreateSolidBrush(bg);
	return TRUE;
}

void AppearancePage::write()
{
	PropPage::write((HWND)*this, items);

	settings->set(SettingsManager::TEXT_COLOR, (int)fg);
	settings->set(SettingsManager::BACKGROUND_COLOR, (int)bg);
}

LRESULT AppearancePage::onClickedColor(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	CColorDialog d(SETTING(TEXT_COLOR));
	if(d.DoModal() == IDOK)
	{
		fg = d.GetColor();
		ctrlExample.Invalidate();
	}
	return TRUE;
}

LRESULT AppearancePage::onClickedBackground(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	CColorDialog d(SETTING(BACKGROUND_COLOR));
	if(d.DoModal() == IDOK)
	{
		::DeleteObject(bgbrush);
		bg = d.GetColor();
		bgbrush = CreateSolidBrush(bg);
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
		return (LRESULT)bgbrush;
	}
	else
		return FALSE;
}

/**
 * @file AppearancePage.cpp
 * $Id: AppearancePage.cpp,v 1.1 2002/01/26 16:34:00 arnetheduck Exp $
 * @if LOG
 * $Log: AppearancePage.cpp,v $
 * Revision 1.1  2002/01/26 16:34:00  arnetheduck
 * Colors dialog added, as well as some other options
 *
 * Revision 1.4  2002/01/26 12:52:51  arnetheduck
 * More minor fixes
 *
 * @endif
 */

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
#include <client/DCPlusPlus.h>

#include "resource.h"

#include "GeneralPage.h"

#include <client/SettingsManager.h>

PropPage::TextItem GeneralPage::texts[] = {
	{ IDC_SETTINGS_PERSONAL_INFORMATION, ResourceManager::SETTINGS_PERSONAL_INFORMATION },
	{ IDC_SETTINGS_NICK, ResourceManager::NICK },
	{ IDC_SETTINGS_EMAIL, ResourceManager::EMAIL },
	{ IDC_SETTINGS_DESCRIPTION, ResourceManager::DESCRIPTION },
	{ IDC_SETTINGS_UPLOAD_LINE_SPEED, ResourceManager::SETTINGS_UPLOAD_LINE_SPEED },
	{ IDC_SETTINGS_MEBIBITS, ResourceManager::MiBITSPS },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

PropPage::Item GeneralPage::items[] = {
	{ IDC_NICK,			SettingsManager::NICK,			PropPage::T_STR },
	{ IDC_EMAIL,		SettingsManager::EMAIL,			PropPage::T_STR },
	{ IDC_DESCRIPTION,	SettingsManager::DESCRIPTION,	PropPage::T_STR },
	{ IDC_CONNECTION,	SettingsManager::UPLOAD_SPEED,	PropPage::T_STR },
	{ 0, 0, PropPage::T_END }
};

GeneralPage::GeneralPage(SmartWin::Widget* parent) : SmartWin::Widget(parent), PropPage() {
	createDialog(IDD_GENERALPAGE);

	PropPage::translate(handle(), texts);
	PropPage::read(handle(), items);

	WidgetComboBoxPtr connections = subclassComboBox(IDC_CONNECTION);

	int selected = 0, j = 0;
	for(StringIter i = SettingsManager::connectionSpeeds.begin(); i != SettingsManager::connectionSpeeds.end(); ++i, ++j) {
		connections->addValue(Text::toT(*i).c_str());
		if(selected == 0 && SETTING(UPLOAD_SPEED) == *i) {
			selected = j; 
		}
	}

	connections->setSelectedIndex(selected);
		
#ifdef PORT_ME
	nick.Attach(GetDlgItem(IDC_NICK));
	nick.LimitText(35);
	desc.Attach(GetDlgItem(IDC_DESCRIPTION));
	desc.LimitText(35);
	desc.Detach();
	return TRUE;
#endif
}

GeneralPage::~GeneralPage() {
	
}

void GeneralPage::write() {
	PropPage::write(handle(), items);
}

#ifdef PORT_ME

LRESULT GeneralPage::onTextChanged(WORD /*wNotifyCode*/, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/)
{
	TCHAR buf[SETTINGS_BUF_LEN];

	GetDlgItemText(wID, buf, SETTINGS_BUF_LEN);
	tstring old = buf;

	// Strip ' ' from nick
	TCHAR *b = buf, *f = buf, c;
	while( (c = *b++) != 0 )
	{
		if(c != ' ')
			*f++ = c;
	}

	*f = '\0';

	if(old != buf)
	{
		// Something changed; update window text without changing cursor pos
		CEdit tmp;
		tmp.Attach(hWndCtl);
		int start, end;
		tmp.GetSel(start, end);
		tmp.SetWindowText(buf);
		if(start > 0) start--;
		if(end > 0) end--;
		tmp.SetSel(start, end);
		tmp.Detach();
	}

	return TRUE;
}

LRESULT GeneralPage::onHelp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_GENERALPAGE);
	return 0;
}

LRESULT GeneralPage::onHelpInfo(LPNMHDR /*pnmh*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_GENERALPAGE);
	return 0;
}
#endif

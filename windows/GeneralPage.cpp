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

#include "GeneralPage.h"
#include "../client/SettingsManager.h"
#include "../client/Socket.h"
#include "WinUtil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

PropPage::TextItem GeneralPage::texts[] = {
	{ IDC_SETTINGS_PERSONAL_INFORMATION, ResourceManager::SETTINGS_PERSONAL_INFORMATION },
	{ IDC_SETTINGS_NICK, ResourceManager::NICK },
	{ IDC_SETTINGS_EMAIL, ResourceManager::EMAIL },
	{ IDC_SETTINGS_DESCRIPTION, ResourceManager::DESCRIPTION },
	{ IDC_SETTINGS_CONNECTION_TYPE, ResourceManager::SETTINGS_CONNECTION_TYPE },
	{ IDC_SETTINGS_CONNECTION_SETTINGS, ResourceManager::SETTINGS_CONNECTION_SETTINGS },
	{ IDC_ACTIVE, ResourceManager::ACTIVE },
	{ IDC_PASSIVE, ResourceManager::SETTINGS_PASSIVE },
	{ IDC_SOCKS5, ResourceManager::SETTINGS_SOCKS5 }, 
	{ IDC_SETTINGS_IP, ResourceManager::SETTINGS_IP },
	{ IDC_SETTINGS_PORT, ResourceManager::SETTINGS_PORT },
	{ IDC_SETTINGS_SOCKS5_IP, ResourceManager::SETTINGS_SOCKS5_IP },
	{ IDC_SETTINGS_SOCKS5_PORT, ResourceManager::SETTINGS_SOCKS5_PORT },
	{ IDC_SETTINGS_SOCKS5_USERNAME, ResourceManager::SETTINGS_SOCKS5_USERNAME },
	{ IDC_SETTINGS_SOCKS5_PASSWORD, ResourceManager::PASSWORD },
	{ IDC_SOCKS_RESOLVE, ResourceManager::SETTINGS_SOCKS5_RESOLVE },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

PropPage::Item GeneralPage::items[] = {
	{ IDC_NICK,			SettingsManager::NICK,			PropPage::T_STR }, 
	{ IDC_EMAIL,		SettingsManager::EMAIL,			PropPage::T_STR }, 
	{ IDC_DESCRIPTION,	SettingsManager::DESCRIPTION,	PropPage::T_STR }, 
	{ IDC_CONNECTION,	SettingsManager::CONNECTION,	PropPage::T_STR }, 
	{ IDC_SERVER,		SettingsManager::SERVER,		PropPage::T_STR }, 
	{ IDC_PORT,			SettingsManager::IN_PORT,		PropPage::T_INT }, 
	{ IDC_SOCKS_SERVER, SettingsManager::SOCKS_SERVER,	PropPage::T_STR },
	{ IDC_SOCKS_PORT,	SettingsManager::SOCKS_PORT,	PropPage::T_INT },
	{ IDC_SOCKS_USER,	SettingsManager::SOCKS_USER,	PropPage::T_STR },
	{ IDC_SOCKS_PASSWORD, SettingsManager::SOCKS_PASSWORD, PropPage::T_STR },
	{ IDC_SOCKS_RESOLVE, SettingsManager::SOCKS_RESOLVE, PropPage::T_BOOL },
	{ 0, 0, PropPage::T_END }
};

void GeneralPage::write()
{
	TCHAR tmp[1024];
	GetDlgItemText(IDC_SOCKS_SERVER, tmp, 1024);
	tstring x = tmp;
	tstring::size_type i;

	while((i = x.find(' ')) != string::npos)
		x.erase(i, 1);
	SetDlgItemText(IDC_SOCKS_SERVER, x.c_str());

	GetDlgItemText(IDC_SERVER, tmp, 1024);
	x = tmp;

	while((i = x.find(' ')) != string::npos)
		x.erase(i, 1);

	SetDlgItemText(IDC_SERVER, x.c_str());
	
	PropPage::write((HWND)(*this), items);

	// Set connection active/passive
	int ct = -1;
	if(IsDlgButtonChecked(IDC_ACTIVE))
		ct = SettingsManager::CONNECTION_ACTIVE;
	else if(IsDlgButtonChecked(IDC_PASSIVE))
		ct = SettingsManager::CONNECTION_PASSIVE;
	else if(IsDlgButtonChecked(IDC_SOCKS5))
		ct = SettingsManager::CONNECTION_SOCKS5;

	if(SETTING(CONNECTION_TYPE) != ct) {
		settings->set(SettingsManager::CONNECTION_TYPE, ct);
		Socket::socksUpdated();
	}
}

LRESULT GeneralPage::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	PropPage::translate((HWND)(*this), texts);
	ctrlConnection.Attach(GetDlgItem(IDC_CONNECTION));
	
	for(int i = 0; i < SettingsManager::SPEED_LAST; i++)
		ctrlConnection.AddString(Text::toT(SettingsManager::connectionSpeeds[i]).c_str());

	int const connType = settings->get(SettingsManager::CONNECTION_TYPE);
	if(connType == SettingsManager::CONNECTION_ACTIVE)
		CheckRadioButton(IDC_ACTIVE, IDC_SOCKS5, IDC_ACTIVE);
	else if(connType == SettingsManager::CONNECTION_PASSIVE)
		CheckRadioButton(IDC_ACTIVE, IDC_SOCKS5, IDC_PASSIVE);
	else if(connType == SettingsManager::CONNECTION_SOCKS5)
		CheckRadioButton(IDC_ACTIVE, IDC_SOCKS5, IDC_SOCKS5);

	PropPage::read((HWND)(*this), items);

	fixControls();

	ctrlConnection.SetCurSel(ctrlConnection.FindString(0, Text::toT(SETTING(CONNECTION)).c_str()));

	nick.Attach(GetDlgItem(IDC_NICK));
	nick.LimitText(35);
	CEdit desc;
	desc.Attach(GetDlgItem(IDC_DESCRIPTION));
	desc.LimitText(35);
	desc.Detach();
	desc.Attach(GetDlgItem(IDC_SOCKS_SERVER));
	desc.LimitText(250);
	desc.Detach();
	desc.Attach(GetDlgItem(IDC_SOCKS_PORT));
	desc.LimitText(5);
	desc.Detach();
	desc.Attach(GetDlgItem(IDC_SOCKS_USER));
	desc.LimitText(250);
	desc.Detach();
	desc.Attach(GetDlgItem(IDC_SOCKS_PASSWORD));
	desc.LimitText(250);
	desc.Detach();
	return TRUE;
}

void GeneralPage::fixControls() {
	BOOL checked = IsDlgButtonChecked(IDC_ACTIVE);
	::EnableWindow(GetDlgItem(IDC_SERVER), checked);
	::EnableWindow(GetDlgItem(IDC_PORT), checked);

	checked = IsDlgButtonChecked(IDC_SOCKS5);
	::EnableWindow(GetDlgItem(IDC_SOCKS_SERVER), checked);
	::EnableWindow(GetDlgItem(IDC_SOCKS_PORT), checked);
	::EnableWindow(GetDlgItem(IDC_SOCKS_USER), checked);
	::EnableWindow(GetDlgItem(IDC_SOCKS_PASSWORD), checked);
	::EnableWindow(GetDlgItem(IDC_SOCKS_RESOLVE), checked);

}

LRESULT GeneralPage::onClickedActive(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	fixControls();
	return 0;
}

LRESULT GeneralPage::onTextChanged(WORD /*wNotifyCode*/, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/)
{
	TCHAR buf[SETTINGS_BUF_LEN];

	GetDlgItemText(wID, buf, SETTINGS_BUF_LEN);
	tstring old = buf;

	// Strip '$', '|', '<', '>' and ' ' from text
	TCHAR *b = buf, *f = buf, c;
	while( (c = *b++) != 0 )
	{
		if(c != '$' && c != '|' && (wID == IDC_DESCRIPTION || c != ' ') && ( (wID != IDC_NICK) || (c != '<' && c != '>')) )
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

/**
 * @file
 * $Id: GeneralPage.cpp,v 1.10 2004/09/10 14:44:17 arnetheduck Exp $
 */


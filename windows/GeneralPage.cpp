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
#include "../client/DCPlusPlus.h"
#include "Resource.h"

#include "GeneralPage.h"
#include "../client/SettingsManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

PropPage::Item GeneralPage::items[] = {
	{ IDC_NICK,			SettingsManager::NICK,			PropPage::T_STR }, 
	{ IDC_EMAIL,		SettingsManager::EMAIL,			PropPage::T_STR }, 
	{ IDC_DESCRIPTION,	SettingsManager::DESCRIPTION,	PropPage::T_STR }, 
	{ IDC_CONNECTION,	SettingsManager::CONNECTION,	PropPage::T_STR }, 
	{ IDC_SERVER,		SettingsManager::SERVER,		PropPage::T_STR }, 
	{ IDC_PORT,			SettingsManager::PORT,			PropPage::T_INT }, 
	{ 0, 0, PropPage::T_END }
};

void GeneralPage::write()
{
	PropPage::write((HWND)(*this), items);

	// Set connection active/passive
	int ct = -1;
	if(IsDlgButtonChecked(IDC_ACTIVE))
		ct = SettingsManager::CONNECTION_ACTIVE;
	else if(IsDlgButtonChecked(IDC_PASSIVE))
		ct = SettingsManager::CONNECTION_PASSIVE;
	settings->set(SettingsManager::CONNECTION_TYPE, ct);
}

LRESULT GeneralPage::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	ctrlConnection.Attach(GetDlgItem(IDC_CONNECTION));
	
	for(int i = 0; i < SettingsManager::SPEED_LAST; i++)
		ctrlConnection.AddString(SettingsManager::connectionSpeeds[i].c_str());

	PropPage::read((HWND)(*this), items);
	if(SettingsManager::getInstance()->get(SettingsManager::SERVER, false).empty()) {
		SetDlgItemText(IDC_SERVER, "");
	}
	
	SetDlgItemText(IDC_MODEHELP, "This is the most common configuration. Select this if you have a direct connection to the Internet or if you're not using a firewall that's configured to forward incoming connections. Sometimes, DC++ might not be able to correctly detect your ip address. Frequent connection timeouts and absence of search results are typical indications of this problem, and to make it work you should either try entering your ip in the box below or use passive mode.");
	int const connType = settings->get(SettingsManager::CONNECTION_TYPE);
	if(connType == SettingsManager::CONNECTION_ACTIVE)
		CheckRadioButton(IDC_ACTIVE, IDC_PASSIVE, IDC_ACTIVE);
	else if(connType == SettingsManager::CONNECTION_PASSIVE)
		CheckRadioButton(IDC_ACTIVE, IDC_PASSIVE, IDC_PASSIVE);

	ctrlConnection.SetCurSel(ctrlConnection.FindString(0, SETTING(CONNECTION).c_str()));

	BOOL dummy;
	onClickedActive(0,0,0,dummy);	// Update enable/disable for server/port controls

	nick.Attach(GetDlgItem(IDC_NICK));
	nick.LimitText(35);
	CEdit desc;
	desc.Attach(GetDlgItem(IDC_DESCRIPTION));
	desc.LimitText(35);
	desc.Detach();
	return TRUE;
}

LRESULT GeneralPage::onClickedActive(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	BOOL const checked = IsDlgButtonChecked(IDC_ACTIVE);
	::EnableWindow(GetDlgItem(IDC_SERVER), checked);
	::EnableWindow(GetDlgItem(IDC_PORT), checked);
	return 0;
}

LRESULT GeneralPage::onTextChanged(WORD /*wNotifyCode*/, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/)
{
	char buf[SETTINGS_BUF_LEN];

	GetDlgItemText(wID, buf, SETTINGS_BUF_LEN);
	string old = buf;

	// Strip '$', '|', '<', '>' and ' ' from text
	char *b = buf, *f = buf, c;
	while( (c = *b++) != 0 )
	{
		if(c != '$' && c != '|' && (wID == IDC_DESCRIPTION || c != ' ') && ( (wID != IDC_NICK) || c != '<' || c != '>') )
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
 * @file GeneralPage.cpp
 * $Id: GeneralPage.cpp,v 1.3 2002/05/25 16:10:16 arnetheduck Exp $
 */


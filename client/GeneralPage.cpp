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
#include "GeneralPage.h"
#include "SettingsManager.h"

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

GeneralPage::GeneralPage(SettingsManager *s) : PropPage(s)
{
}

GeneralPage::~GeneralPage()
{
}

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
		ctrlConnection.AddString(SettingsManager::connectionSpeeds[i]);

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

	return TRUE;
}

LRESULT GeneralPage::onClickedActive(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	BOOL const checked = IsDlgButtonChecked(IDC_ACTIVE);
	::EnableWindow(GetDlgItem(IDC_SERVER), checked);
	::EnableWindow(GetDlgItem(IDC_PORT), checked);
	return 0;
}
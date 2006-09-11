/*
 * Copyright (C) 2001-2006 Jacek Sieka, arnetheduck on gmail point com
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

#include "NetworkPage.h"
#include "../client/SettingsManager.h"
#include "../client/Socket.h"
#include "WinUtil.h"

PropPage::TextItem NetworkPage::texts[] = {
	{ IDC_DIRECT, ResourceManager::SETTINGS_DIRECT },
	{ IDC_DIRECT_OUT, ResourceManager::SETTINGS_DIRECT },
	{ IDC_FIREWALL_UPNP, ResourceManager::SETTINGS_FIREWALL_UPNP },
	{ IDC_FIREWALL_NAT, ResourceManager::SETTINGS_FIREWALL_NAT },
	{ IDC_FIREWALL_PASSIVE, ResourceManager::SETTINGS_FIREWALL_PASSIVE },
	{ IDC_OVERRIDE, ResourceManager::SETTINGS_OVERRIDE },
	{ IDC_SOCKS5, ResourceManager::SETTINGS_SOCKS5 }, 
	{ IDC_SETTINGS_PORTS, ResourceManager::SETTINGS_PORTS },
	{ IDC_SETTINGS_IP, ResourceManager::SETTINGS_EXTERNAL_IP },
	{ IDC_SETTINGS_PORT_TCP, ResourceManager::SETTINGS_TCP_PORT },
	{ IDC_SETTINGS_PORT_UDP, ResourceManager::SETTINGS_UDP_PORT },
	{ IDC_SETTINGS_PORT_TLS, ResourceManager::SETTINGS_TLS_PORT },
	{ IDC_SETTINGS_SOCKS5_IP, ResourceManager::SETTINGS_SOCKS5_IP },
	{ IDC_SETTINGS_SOCKS5_PORT, ResourceManager::SETTINGS_SOCKS5_PORT },
	{ IDC_SETTINGS_SOCKS5_USERNAME, ResourceManager::SETTINGS_SOCKS5_USERNAME },
	{ IDC_SETTINGS_SOCKS5_PASSWORD, ResourceManager::PASSWORD },
	{ IDC_SOCKS_RESOLVE, ResourceManager::SETTINGS_SOCKS5_RESOLVE },
	{ IDC_SETTINGS_INCOMING, ResourceManager::SETTINGS_INCOMING },
	{ IDC_SETTINGS_OUTGOING, ResourceManager::SETTINGS_OUTGOING },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

PropPage::Item NetworkPage::items[] = {
	{ IDC_EXTERNAL_IP,	SettingsManager::EXTERNAL_IP,	PropPage::T_STR }, 
	{ IDC_PORT_TCP,		SettingsManager::TCP_PORT,		PropPage::T_INT }, 
	{ IDC_PORT_UDP,		SettingsManager::UDP_PORT,		PropPage::T_INT }, 
	{ IDC_PORT_TLS,		SettingsManager::TLS_PORT,		PropPage::T_INT },
	{ IDC_OVERRIDE,		SettingsManager::NO_IP_OVERRIDE, PropPage::T_BOOL },
	{ IDC_SOCKS_SERVER, SettingsManager::SOCKS_SERVER,	PropPage::T_STR },
	{ IDC_SOCKS_PORT,	SettingsManager::SOCKS_PORT,	PropPage::T_INT },
	{ IDC_SOCKS_USER,	SettingsManager::SOCKS_USER,	PropPage::T_STR },
	{ IDC_SOCKS_PASSWORD, SettingsManager::SOCKS_PASSWORD, PropPage::T_STR },
	{ IDC_SOCKS_RESOLVE, SettingsManager::SOCKS_RESOLVE, PropPage::T_BOOL },
	{ 0, 0, PropPage::T_END }
};

void NetworkPage::write()
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
	int ct = SettingsManager::INCOMING_DIRECT;

	if(IsDlgButtonChecked(IDC_FIREWALL_UPNP))
		ct = SettingsManager::INCOMING_FIREWALL_UPNP;
	else if(IsDlgButtonChecked(IDC_FIREWALL_NAT))
		ct = SettingsManager::INCOMING_FIREWALL_NAT;
	else if(IsDlgButtonChecked(IDC_FIREWALL_PASSIVE))
		ct = SettingsManager::INCOMING_FIREWALL_PASSIVE;

	if(SETTING(INCOMING_CONNECTIONS) != ct) {
		settings->set(SettingsManager::INCOMING_CONNECTIONS, ct);
	}

	ct = SettingsManager::OUTGOING_DIRECT;
	
	if(IsDlgButtonChecked(IDC_SOCKS5))
		ct = SettingsManager::OUTGOING_SOCKS5;

	if(SETTING(OUTGOING_CONNECTIONS) != ct) {
		settings->set(SettingsManager::OUTGOING_CONNECTIONS, ct);
		Socket::socksUpdated();
	}

}

LRESULT NetworkPage::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	PropPage::translate((HWND)(*this), texts);
	
	if(!(WinUtil::getOsMajor() >= 5 && WinUtil::getOsMinor() >= 1  //WinXP & WinSvr2003
		|| WinUtil::getOsMajor() >= 6 )) //Vista
	{
		::EnableWindow(GetDlgItem(IDC_FIREWALL_UPNP), FALSE);
	}
	switch(SETTING(INCOMING_CONNECTIONS)) {
		case SettingsManager::INCOMING_DIRECT: CheckDlgButton(IDC_DIRECT, BST_CHECKED); break;
		case SettingsManager::INCOMING_FIREWALL_UPNP: CheckDlgButton(IDC_FIREWALL_UPNP, BST_CHECKED); break;
		case SettingsManager::INCOMING_FIREWALL_NAT: CheckDlgButton(IDC_FIREWALL_NAT, BST_CHECKED); break;
		case SettingsManager::INCOMING_FIREWALL_PASSIVE: CheckDlgButton(IDC_FIREWALL_PASSIVE, BST_CHECKED); break;
		default: CheckDlgButton(IDC_DIRECT, BST_CHECKED); break;
	}

	switch(SETTING(OUTGOING_CONNECTIONS)) {
		case SettingsManager::OUTGOING_DIRECT: CheckDlgButton(IDC_DIRECT_OUT, BST_CHECKED); break;
		case SettingsManager::OUTGOING_SOCKS5: CheckDlgButton(IDC_SOCKS5, BST_CHECKED); break;
		default: CheckDlgButton(IDC_DIRECT_OUT, BST_CHECKED); break;
	}

	PropPage::read((HWND)(*this), items);

	fixControls();

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

void NetworkPage::fixControls() {
	BOOL direct = IsDlgButtonChecked(IDC_DIRECT) == BST_CHECKED;
	BOOL upnp = IsDlgButtonChecked(IDC_FIREWALL_UPNP) == BST_CHECKED;
	BOOL nat = IsDlgButtonChecked(IDC_FIREWALL_NAT) == BST_CHECKED;

	::EnableWindow(GetDlgItem(IDC_EXTERNAL_IP), direct || upnp || nat);
	::EnableWindow(GetDlgItem(IDC_OVERRIDE), direct || upnp || nat);

	::EnableWindow(GetDlgItem(IDC_PORT_TCP), direct || upnp || nat);
	::EnableWindow(GetDlgItem(IDC_PORT_UDP), direct || upnp || nat);

	BOOL socks = IsDlgButtonChecked(IDC_SOCKS5);
	::EnableWindow(GetDlgItem(IDC_SOCKS_SERVER), socks);
	::EnableWindow(GetDlgItem(IDC_SOCKS_PORT), socks);
	::EnableWindow(GetDlgItem(IDC_SOCKS_USER), socks);
	::EnableWindow(GetDlgItem(IDC_SOCKS_PASSWORD), socks);
	::EnableWindow(GetDlgItem(IDC_SOCKS_RESOLVE), socks);

}

LRESULT NetworkPage::onClickedActive(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	fixControls();
	return 0;
}

LRESULT NetworkPage::onHelp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_NETWORKPAGE);
	return 0;
}

LRESULT NetworkPage::onHelpInfo(LPNMHDR /*pnmh*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_NETWORKPAGE);
	return 0;
}

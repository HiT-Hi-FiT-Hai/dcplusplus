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

#include "resource.h"

#include "NetworkPage.h"

#include <dcpp/SettingsManager.h>
#include <dcpp/Socket.h>
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

NetworkPage::NetworkPage(SmartWin::Widget* parent) : PropPage(parent) {
	createDialog(IDD_NETWORKPAGE);

	PropPage::translate(handle(), texts);

	if(!(WinUtil::getOsMajor() >= 5 && WinUtil::getOsMinor() >= 1 //WinXP & WinSvr2003
		|| WinUtil::getOsMajor() >= 6 )) //Vista
	{
		::EnableWindow(::GetDlgItem(handle(), IDC_FIREWALL_UPNP), FALSE);
	}
	switch(SETTING(INCOMING_CONNECTIONS)) {
		case SettingsManager::INCOMING_DIRECT: ::CheckDlgButton(handle(), IDC_DIRECT, BST_CHECKED); break;
		case SettingsManager::INCOMING_FIREWALL_UPNP: ::CheckDlgButton(handle(), IDC_FIREWALL_UPNP, BST_CHECKED); break;
		case SettingsManager::INCOMING_FIREWALL_NAT: ::CheckDlgButton(handle(), IDC_FIREWALL_NAT, BST_CHECKED); break;
		case SettingsManager::INCOMING_FIREWALL_PASSIVE: ::CheckDlgButton(handle(), IDC_FIREWALL_PASSIVE, BST_CHECKED); break;
		default: ::CheckDlgButton(handle(), IDC_DIRECT, BST_CHECKED); break;
	}

	switch(SETTING(OUTGOING_CONNECTIONS)) {
		case SettingsManager::OUTGOING_DIRECT: ::CheckDlgButton(handle(), IDC_DIRECT_OUT, BST_CHECKED); break;
		case SettingsManager::OUTGOING_SOCKS5: ::CheckDlgButton(handle(), IDC_SOCKS5, BST_CHECKED); break;
		default: ::CheckDlgButton(handle(), IDC_DIRECT_OUT, BST_CHECKED); break;
	}

	PropPage::read(handle(), items);

	fixControls();

#define RADIO_ATTACH(id) subclassRadioButton(id)->onClicked((std::tr1::bind(&NetworkPage::fixControls, this)))
	RADIO_ATTACH(IDC_DIRECT);
	RADIO_ATTACH(IDC_FIREWALL_UPNP);
	RADIO_ATTACH(IDC_FIREWALL_NAT);
	RADIO_ATTACH(IDC_FIREWALL_PASSIVE);
	RADIO_ATTACH(IDC_DIRECT_OUT);
	RADIO_ATTACH(IDC_SOCKS5);
#undef RADIO_ATTACH

#define TEXTBOX_ATTACH(id) \
	static_cast<WidgetTextBoxPtr>(subclassTextBox(id))->setTextLimit(250)
	TEXTBOX_ATTACH(IDC_SOCKS_SERVER);
	TEXTBOX_ATTACH(IDC_SOCKS_PORT);
	TEXTBOX_ATTACH(IDC_SOCKS_USER);
	TEXTBOX_ATTACH(IDC_SOCKS_PASSWORD);
#undef TEXTBOX_ATTACH
}

NetworkPage::~NetworkPage() {
}

void NetworkPage::write()
{
	TCHAR tmp[1024];
	::GetDlgItemText(handle(), IDC_EXTERNAL_IP, tmp, 1024);
	tstring x = tmp;
	tstring::size_type i;

	while((i = x.find(' ')) != string::npos)
		x.erase(i, 1);
	::SetDlgItemText(handle(), IDC_EXTERNAL_IP, x.c_str());

	::GetDlgItemText(handle(), IDC_SOCKS_SERVER, tmp, 1024);
	x = tmp;

	while((i = x.find(' ')) != string::npos)
		x.erase(i, 1);
	::SetDlgItemText(handle(), IDC_SOCKS_SERVER, x.c_str());

	PropPage::write(handle(), items);

	SettingsManager* settings = SettingsManager::getInstance();

	// Set connection active/passive
	int ct = SettingsManager::INCOMING_DIRECT;

	if(::IsDlgButtonChecked(handle(), IDC_FIREWALL_UPNP))
		ct = SettingsManager::INCOMING_FIREWALL_UPNP;
	else if(::IsDlgButtonChecked(handle(), IDC_FIREWALL_NAT))
		ct = SettingsManager::INCOMING_FIREWALL_NAT;
	else if(::IsDlgButtonChecked(handle(), IDC_FIREWALL_PASSIVE))
		ct = SettingsManager::INCOMING_FIREWALL_PASSIVE;

	if(SETTING(INCOMING_CONNECTIONS) != ct) {
		settings->set(SettingsManager::INCOMING_CONNECTIONS, ct);
	}

	ct = SettingsManager::OUTGOING_DIRECT;

	if(::IsDlgButtonChecked(handle(), IDC_SOCKS5))
		ct = SettingsManager::OUTGOING_SOCKS5;

	if(SETTING(OUTGOING_CONNECTIONS) != ct) {
		settings->set(SettingsManager::OUTGOING_CONNECTIONS, ct);
		Socket::socksUpdated();
	}
}

void NetworkPage::fixControls() {
	BOOL direct = ::IsDlgButtonChecked(handle(), IDC_DIRECT) == BST_CHECKED;
	BOOL upnp = ::IsDlgButtonChecked(handle(), IDC_FIREWALL_UPNP) == BST_CHECKED;
	BOOL nat = ::IsDlgButtonChecked(handle(), IDC_FIREWALL_NAT) == BST_CHECKED;

	::EnableWindow(::GetDlgItem(handle(), IDC_EXTERNAL_IP), direct || upnp || nat);
	::EnableWindow(::GetDlgItem(handle(), IDC_OVERRIDE), direct || upnp || nat);

	::EnableWindow(::GetDlgItem(handle(), IDC_PORT_TCP), direct || upnp || nat);
	::EnableWindow(::GetDlgItem(handle(), IDC_PORT_UDP), direct || upnp || nat);
	::EnableWindow(::GetDlgItem(handle(), IDC_PORT_TLS), direct || upnp || nat);

	BOOL socks = ::IsDlgButtonChecked(handle(), IDC_SOCKS5);
	::EnableWindow(::GetDlgItem(handle(), IDC_SOCKS_SERVER), socks);
	::EnableWindow(::GetDlgItem(handle(), IDC_SOCKS_PORT), socks);
	::EnableWindow(::GetDlgItem(handle(), IDC_SOCKS_USER), socks);
	::EnableWindow(::GetDlgItem(handle(), IDC_SOCKS_PASSWORD), socks);
	::EnableWindow(::GetDlgItem(handle(), IDC_SOCKS_RESOLVE), socks);

}

#ifdef PORT_ME

LRESULT NetworkPage::onHelp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_NETWORKPAGE);
	return 0;
}

LRESULT NetworkPage::onHelpInfo(LPNMHDR /*pnmh*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_NETWORKPAGE);
	return 0;
}
#endif

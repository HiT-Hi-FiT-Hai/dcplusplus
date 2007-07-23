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
#include <dcpp/DCPlusPlus.h>

#include "resource.h"

#include "CertificatesPage.h"

#include <dcpp/SettingsManager.h>
#include <dcpp/CryptoManager.h>
#include "WinUtil.h"

PropPage::TextItem CertificatesPage::texts[] = {
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

PropPage::Item CertificatesPage::items[] = {
	{ IDC_TLS_CERTIFICATE_FILE, SettingsManager::TLS_CERTIFICATE_FILE, PropPage::T_STR },
	{ IDC_TLS_PRIVATE_KEY_FILE, SettingsManager::TLS_PRIVATE_KEY_FILE, PropPage::T_STR },
	{ IDC_TLS_TRUSTED_CERTIFICATES_PATH, SettingsManager::TLS_TRUSTED_CERTIFICATES_PATH, PropPage::T_STR },
	{ 0, 0, PropPage::T_END }
};

PropPage::ListItem CertificatesPage::listItems[] = {
	{ SettingsManager::USE_TLS, ResourceManager::SETTINGS_USE_TLS },
	{ SettingsManager::ALLOW_UNTRUSTED_HUBS, ResourceManager::SETTINGS_ALLOW_UNTRUSTED_HUBS	},
	{ SettingsManager::ALLOW_UNTRUSTED_CLIENTS, ResourceManager::SETTINGS_ALLOW_UNTRUSTED_CLIENTS, },
	{ 0, ResourceManager::SETTINGS_ALLOW_UNTRUSTED_CLIENTS, },
};

CertificatesPage::CertificatesPage(SmartWin::Widget* parent) : PropPage(parent) {
	createDialog(IDD_CERTIFICATESPAGE);

	PropPage::translate(handle(), texts);
	PropPage::read(handle(), items, listItems, ::GetDlgItem(handle(), IDC_TLS_OPTIONS));

	subclassButton(IDC_BROWSE_PRIVATE_KEY)->onClicked(std::tr1::bind(&CertificatesPage::handleBrowsePrivateKeyClicked, this));

	subclassButton(IDC_BROWSE_CERTIFICATE)->onClicked(std::tr1::bind(&CertificatesPage::handleBrowseCertificateClicked, this));

	subclassButton(IDC_BROWSE_TRUSTED_PATH)->onClicked(std::tr1::bind(&CertificatesPage::handleBrowseTrustedPathClicked, this));

	subclassButton(IDC_GENERATE_CERTS)->onClicked(std::tr1::bind(&CertificatesPage::handleGenerateCertsClicked, this));
}

CertificatesPage::~CertificatesPage() {
}

void CertificatesPage::write() {
	PropPage::write(handle(), items, listItems, ::GetDlgItem(handle(), IDC_TLS_OPTIONS));
}

void CertificatesPage::handleBrowsePrivateKeyClicked() {
	tstring target = Text::toT(SETTING(TLS_PRIVATE_KEY_FILE));
	if(WinUtil::browseFile(target, handle(), false, target))
		::SetDlgItemText(handle(), IDC_TLS_PRIVATE_KEY_FILE, &target[0]);
}

void CertificatesPage::handleBrowseCertificateClicked() {
	tstring target = Text::toT(SETTING(TLS_CERTIFICATE_FILE));
	if(WinUtil::browseFile(target, handle(), false, target))
		::SetDlgItemText(handle(), IDC_TLS_CERTIFICATE_FILE, &target[0]);
}

void CertificatesPage::handleBrowseTrustedPathClicked() {
	tstring target = Text::toT(SETTING(TLS_TRUSTED_CERTIFICATES_PATH));
	if(WinUtil::browseDirectory(target, handle()))
		::SetDlgItemText(handle(), IDC_TLS_TRUSTED_CERTIFICATES_PATH, &target[0]);
}

void CertificatesPage::handleGenerateCertsClicked() {
	try {
		CryptoManager::getInstance()->generateCertificate();
	} catch(const CryptoException& e) {
		createMessageBox().show(Text::toT(e.getError()), _T("Error generating certificate"), WidgetMessageBox::BOX_OK, WidgetMessageBox::BOX_ICONSTOP);
	}
}

#ifdef PORT_ME

LRESULT CertificatesPage::onHelpInfo(LPNMHDR /*pnmh*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_CERTIFICATESPAGE);
	return 0;
}

LRESULT CertificatesPage::onHelp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_CERTIFICATESPAGE);
	return 0;
}
#endif

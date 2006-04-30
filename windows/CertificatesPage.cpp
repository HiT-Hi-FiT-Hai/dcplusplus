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

#include "CertificatesPage.h"
#include "CommandDlg.h"

#include "../client/SettingsManager.h"
#include "../client/FavoriteManager.h"

#include "WinUtil.h"

PropPage::TextItem CertificatesPage::texts[] = {
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

PropPage::Item CertificatesPage::items[] = {
	{ IDC_SSL_CERTIFICATE_FILE, SettingsManager::SSL_CERTIFICATE_FILE, PropPage::T_STR },
	{ IDC_SSL_PRIVATE_KEY_FILE, SettingsManager::SSL_PRIVATE_KEY_FILE, PropPage::T_STR },
	{ IDC_SSL_TRUSTED_CERTIFICATES_PATH, SettingsManager::SSL_TRUSTED_CERTIFICATES_PATH, PropPage::T_STR },
	{ 0, 0, PropPage::T_END }
};

LRESULT CertificatesPage::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	PropPage::translate((HWND)(*this), texts);
	PropPage::read((HWND)*this, items, 0, 0);

	// Do specialized reading here
	return TRUE;
}

void CertificatesPage::write() {
	PropPage::write((HWND)*this, items, 0, 0);
}

LRESULT CertificatesPage::onHelpInfo(LPNMHDR /*pnmh*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_CERTIFICATESPAGE);
	return 0;
}

LRESULT CertificatesPage::onHelp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_CERTIFICATESPAGE);
	return 0;
}

/**
 * @file
 * $Id: CertificatesPage.cpp,v 1.1 2005/12/03 20:36:50 arnetheduck Exp $
 */

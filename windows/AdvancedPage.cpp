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

#include "AdvancedPage.h"
#include "../client/SettingsManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

PropPage::Item AdvancedPage::items[] = {
	{ IDC_ROLLBACK, SettingsManager::ROLLBACK, PropPage::T_INT }, 
	{ IDC_CLVERSION, SettingsManager::CLIENTVERSION, PropPage::T_STR }, 
	{ IDC_AUTOFOLLOW, SettingsManager::AUTO_FOLLOW, PropPage::T_BOOL },
	{ IDC_CLEARSEARCH, SettingsManager::CLEAR_SEARCH, PropPage::T_BOOL },
	{ IDC_OPENPUBLIC, SettingsManager::OPEN_PUBLIC, PropPage::T_BOOL },
	{ IDC_OPENQUEUE, SettingsManager::OPEN_QUEUE, PropPage::T_BOOL },
	{ IDC_AUTOSEARCH, SettingsManager::AUTO_SEARCH, PropPage::T_BOOL },
	{ IDC_IGNOREOFFLINE, SettingsManager::IGNORE_OFFLINE, PropPage::T_BOOL },
	{ IDC_POPUPOFFLINE, SettingsManager::POPUP_OFFLINE, PropPage::T_BOOL },
	{ IDC_REMOVEDUPES, SettingsManager::REMOVE_DUPES, PropPage::T_BOOL },
	{ IDC_BUFFERSIZE, SettingsManager::BUFFER_SIZE, PropPage::T_INT },
	{ IDC_PUBLIC_HUBS, SettingsManager::HUBLIST_SERVERS, PropPage::T_STR },
	{ IDC_POPUP_PMS, SettingsManager::POPUP_PMS, PropPage::T_BOOL },
	{ 0, 0, PropPage::T_END }
};

LRESULT AdvancedPage::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	PropPage::read((HWND)*this, items);

	// Do specialized reading here
	return TRUE;
}

void AdvancedPage::write()
{
	PropPage::write((HWND)*this, items);

	// Do specialized writing here
	// settings->set(XX, YY);
}

/**
 * @file AdvancedPage.cpp
 * $Id: AdvancedPage.cpp,v 1.6 2002/06/08 09:34:34 arnetheduck Exp $
 */


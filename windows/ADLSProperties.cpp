/* 
 * Copyright (C) 2001-2005 Jacek Sieka, arnetheduck on gmail point com
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

#include "ADLSProperties.h"
#include "../client/ADLSearch.h"
#include "../client/HubManager.h"
#include "WinUtil.h"

// Initialize dialog
LRESULT ADLSProperties::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
{
	// Translate the texts
	SetWindowText(CTSTRING(ADLS_PROPERTIES));
	SetDlgItemText(IDC_ADLSP_SEARCH, CTSTRING(ADLS_SEARCH_STRING));
	SetDlgItemText(IDC_ADLSP_TYPE, CTSTRING(ADLS_TYPE));
	SetDlgItemText(IDC_ADLSP_SIZE_MIN, CTSTRING(ADLS_SIZE_MIN));
	SetDlgItemText(IDC_ADLSP_SIZE_MAX, CTSTRING(ADLS_SIZE_MAX));
	SetDlgItemText(IDC_ADLSP_UNITS, CTSTRING(ADLS_UNITS));
	SetDlgItemText(IDC_ADLSP_DESTINATION, CTSTRING(ADLS_DESTINATION));
	SetDlgItemText(IDC_IS_ACTIVE, CTSTRING(ADLS_ENABLED));
	SetDlgItemText(IDC_AUTOQUEUE, CTSTRING(ADLS_DOWNLOAD));

	// Initialize combo boxes
	::SendMessage(GetDlgItem(IDC_SOURCE_TYPE), CB_ADDSTRING, 0, 
		(LPARAM)search->SourceTypeToDisplayString(ADLSearch::OnlyFile).c_str());
	::SendMessage(GetDlgItem(IDC_SOURCE_TYPE), CB_ADDSTRING, 0, 
		(LPARAM)search->SourceTypeToDisplayString(ADLSearch::OnlyDirectory).c_str());
	::SendMessage(GetDlgItem(IDC_SOURCE_TYPE), CB_ADDSTRING, 0, 
		(LPARAM)search->SourceTypeToDisplayString(ADLSearch::FullPath).c_str());

	::SendMessage(GetDlgItem(IDC_SIZE_TYPE), CB_ADDSTRING, 0, 
		(LPARAM)search->SizeTypeToDisplayString(ADLSearch::SizeBytes).c_str());
	::SendMessage(GetDlgItem(IDC_SIZE_TYPE), CB_ADDSTRING, 0, 
		(LPARAM)search->SizeTypeToDisplayString(ADLSearch::SizeKiloBytes).c_str());
	::SendMessage(GetDlgItem(IDC_SIZE_TYPE), CB_ADDSTRING, 0, 
		(LPARAM)search->SizeTypeToDisplayString(ADLSearch::SizeMegaBytes).c_str());
	::SendMessage(GetDlgItem(IDC_SIZE_TYPE), CB_ADDSTRING, 0, 
		(LPARAM)search->SizeTypeToDisplayString(ADLSearch::SizeGigaBytes).c_str());

	// Load search data
	SetDlgItemText(IDC_SEARCH_STRING, Text::toT(search->searchString).c_str());
	SetDlgItemText(IDC_DEST_DIR,      Text::toT(search->destDir).c_str());
	SetDlgItemText(IDC_MIN_FILE_SIZE, Text::toT(search->minFileSize > 0 ? Util::toString(search->minFileSize) : "").c_str());
	SetDlgItemText(IDC_MAX_FILE_SIZE, Text::toT(search->maxFileSize > 0 ? Util::toString(search->maxFileSize) : "").c_str());
	::SendMessage(GetDlgItem(IDC_IS_ACTIVE), BM_SETCHECK, search->isActive ? 1 : 0, 0L);
	::SendMessage(GetDlgItem(IDC_SOURCE_TYPE), CB_SETCURSEL, search->sourceType, 0L);
	::SendMessage(GetDlgItem(IDC_SIZE_TYPE), CB_SETCURSEL, search->typeFileSize, 0L);
	::SendMessage(GetDlgItem(IDC_AUTOQUEUE), BM_SETCHECK, search->isAutoQueue ? 1 : 0, 0L);

	// Center dialog
	CenterWindow(GetParent());

	return FALSE;
}

// Exit dialog
LRESULT ADLSProperties::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if(wID == IDOK)
	{
		// Update search
		TCHAR buf[256];

		GetDlgItemText(IDC_SEARCH_STRING, buf, 256);
		search->searchString = Text::fromT(buf);
		GetDlgItemText(IDC_DEST_DIR, buf, 256);
		search->destDir = Text::fromT(buf);

		GetDlgItemText(IDC_MIN_FILE_SIZE, buf, 256);
		search->minFileSize = (_tcslen(buf) == 0 ? -1 : Util::toInt64(Text::fromT(buf)));
		GetDlgItemText(IDC_MAX_FILE_SIZE, buf, 256);
		search->maxFileSize = (_tcslen(buf) == 0 ? -1 : Util::toInt64(Text::fromT(buf)));

		search->sourceType = (ADLSearch::SourceType)::SendMessage(GetDlgItem(IDC_SOURCE_TYPE), CB_GETCURSEL, 0, 0L);
		search->typeFileSize = (ADLSearch::SizeType)::SendMessage(GetDlgItem(IDC_SIZE_TYPE), CB_GETCURSEL, 0, 0L);
		search->isActive = (::SendMessage(GetDlgItem(IDC_IS_ACTIVE), BM_GETCHECK, 0, 0L) != 0);
		search->isAutoQueue = (::SendMessage(GetDlgItem(IDC_AUTOQUEUE), BM_GETCHECK, 0, 0L) != 0);
	}

	EndDialog(wID);
	return 0;
}

/**
 * @file
 * $Id: ADLSProperties.cpp,v 1.7 2005/01/05 19:30:19 arnetheduck Exp $
 */

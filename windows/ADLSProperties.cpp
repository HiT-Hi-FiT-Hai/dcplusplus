/* 
 * Copyright (C) 2001-2003 Jacek Sieka, j_s@telia.com
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

// Initialize dialog
LRESULT ADLSProperties::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
{
	// Initialize combo boxes
	::SendMessage(GetDlgItem(IDC_SOURCE_TYPE), CB_ADDSTRING, 0, 
		(LPARAM)search->SourceTypeToString(ADLSearch::OnlyFile).c_str());
	::SendMessage(GetDlgItem(IDC_SOURCE_TYPE), CB_ADDSTRING, 0, 
		(LPARAM)search->SourceTypeToString(ADLSearch::OnlyDirectory).c_str());
	::SendMessage(GetDlgItem(IDC_SOURCE_TYPE), CB_ADDSTRING, 0, 
		(LPARAM)search->SourceTypeToString(ADLSearch::FullPath).c_str());

	::SendMessage(GetDlgItem(IDC_SIZE_TYPE), CB_ADDSTRING, 0, 
		(LPARAM)search->SizeTypeToStringInternational(ADLSearch::SizeBytes).c_str());
	::SendMessage(GetDlgItem(IDC_SIZE_TYPE), CB_ADDSTRING, 0, 
		(LPARAM)search->SizeTypeToStringInternational(ADLSearch::SizeKiloBytes).c_str());
	::SendMessage(GetDlgItem(IDC_SIZE_TYPE), CB_ADDSTRING, 0, 
		(LPARAM)search->SizeTypeToStringInternational(ADLSearch::SizeMegaBytes).c_str());
	::SendMessage(GetDlgItem(IDC_SIZE_TYPE), CB_ADDSTRING, 0, 
		(LPARAM)search->SizeTypeToStringInternational(ADLSearch::SizeGigaBytes).c_str());

	// Load search data
	char buf[32];
	SetDlgItemText(IDC_SEARCH_STRING, search->searchString.c_str());
	SetDlgItemText(IDC_DEST_DIR,      search->destDir.c_str());
	SetDlgItemText(IDC_MIN_FILE_SIZE, search->minFileSize > 0 ? _i64toa(search->minFileSize, buf, 10) : "");
	SetDlgItemText(IDC_MAX_FILE_SIZE, search->maxFileSize > 0 ? _i64toa(search->maxFileSize, buf, 10) : "");
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
		char buf[256];

		GetDlgItemText(IDC_SEARCH_STRING, buf, 256);
		search->searchString = buf;		
		GetDlgItemText(IDC_DEST_DIR, buf, 256);
		search->destDir = buf;		

		GetDlgItemText(IDC_MIN_FILE_SIZE, buf, 256);
		search->minFileSize = (strlen(buf) == 0 ? -1 : Util::toInt64((string)buf));
		GetDlgItemText(IDC_MAX_FILE_SIZE, buf, 256);
		search->maxFileSize = (strlen(buf) == 0 ? -1 : Util::toInt64((string)buf));

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
 * $Id: ADLSProperties.cpp,v 1.2 2004/01/04 16:34:38 arnetheduck Exp $
 */

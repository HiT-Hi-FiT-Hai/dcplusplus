/* 
 * Copyright (C) 2001 Jacek Sieka, jacek@creatio.se
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
#include "DCPlusPlus.h"

#include "SearchFrm.h"

LRESULT SearchFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	
	CreateSimpleStatusBar(ATL_IDS_IDLEMESSAGE, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SBARS_SIZEGRIP);
	ctrlStatus.Attach(m_hWndStatusBar);
	
	ctrlSearch.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		ES_AUTOHSCROLL, WS_EX_CLIENTEDGE);
	
	ctrlSearchContainer.SubclassWindow(ctrlSearch.m_hWnd);
	
	ctrlResults.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS, WS_EX_CLIENTEDGE, IDC_RESULTS);

	ctrlSearch.SetFont(ctrlResults.GetFont());
	
	ctrlResults.InsertColumn(0, _T("User"), LVCFMT_LEFT, 100, 0);
	ctrlResults.InsertColumn(1, _T("File"), LVCFMT_LEFT, 200, 1);
	ctrlResults.InsertColumn(2, _T("Size"), LVCFMT_RIGHT, 100, 2);
	ctrlResults.InsertColumn(3, _T("Slots"), LVCFMT_LEFT, 75, 3);
	ctrlResults.InsertColumn(4, _T("Hub"), LVCFMT_LEFT, 100, 4);

	SetWindowText("Search all connected hubs");

	
	return 1;
}

/**
 * @file SearchFrm.cpp
 * $Id: SearchFrm.cpp,v 1.3 2001/12/15 17:01:06 arnetheduck Exp $
 * @if LOG
 * $Log: SearchFrm.cpp,v $
 * Revision 1.3  2001/12/15 17:01:06  arnetheduck
 * Passive mode searching as well as some searching code added
 *
 * Revision 1.2  2001/12/13 19:21:57  arnetheduck
 * A lot of work done almost everywhere, mainly towards a friendlier UI
 * and less bugs...time to release 0.06...
 *
 * Revision 1.1  2001/12/10 10:50:10  arnetheduck
 * Oops, forgot the search frame...
 *
 * @endif
 */


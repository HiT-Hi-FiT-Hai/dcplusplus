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

#include "HubFrame.h"

LRESULT HubFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	ctrlClient.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		WS_HSCROLL | WS_VSCROLL | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_NOHIDESEL | ES_READONLY, WS_EX_CLIENTEDGE);

	ctrlMessage.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		ES_AUTOHSCROLL, WS_EX_CLIENTEDGE);

	ctrlMessageContainer.SubclassWindow(ctrlMessage.m_hWnd);
	
	ctrlUsers.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS, WS_EX_CLIENTEDGE);

	ctrlUsers.InsertColumn(0, _T("Name"), LVCFMT_LEFT, 100, 0);
	ctrlUsers.InsertColumn(1, _T("Shared"), LVCFMT_LEFT, 50, 1);
	ctrlUsers.InsertColumn(2, _T("Description"), LVCFMT_LEFT, 100, 2);
	ctrlUsers.InsertColumn(3, _T("Connection"), LVCFMT_LEFT, 75, 3);
	ctrlUsers.InsertColumn(4, _T("E-Mail"), LVCFMT_LEFT, 100, 4);

	bHandled = FALSE;

	client->connect(server);
	return 1;
}

LRESULT HubFrame::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CRect rc;
	GetClientRect(rc);
	rc.bottom -=28;
	rc.right = rc.left + ((rc.right - rc.left)*3/4) - 2;
	ctrlClient.MoveWindow(rc);

	GetClientRect(rc);
	rc.bottom -=28;
	rc.left = rc.left + ((rc.right - rc.left)*3/4) + 2;
	rc.right -=2;
	ctrlUsers.MoveWindow(rc);

	GetClientRect(rc);
	rc.bottom -= 2;
	rc.top = rc.bottom - 22;
	rc.left +=2;
	rc.right -=2;
	ctrlMessage.MoveWindow(rc);
	
	return 0;
}

/**
 * @file HubFrame.cpp
 * $Id: HubFrame.cpp,v 1.1 2001/11/21 17:33:20 arnetheduck Exp $
 * @if LOG
 * $Log: HubFrame.cpp,v $
 * Revision 1.1  2001/11/21 17:33:20  arnetheduck
 * Initial revision
 *
 * @endif
 */


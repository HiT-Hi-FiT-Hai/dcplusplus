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

#include "SpyFrame.h"
#include "WinUtil.h"

SpyFrame* SpyFrame::frame = NULL;

LRESULT SpyFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	CreateSimpleStatusBar(ATL_IDS_IDLEMESSAGE, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SBARS_SIZEGRIP);
	ctrlStatus.Attach(m_hWndStatusBar);

	ctrlSearches.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS, WS_EX_CLIENTEDGE, IDC_RESULTS);

	if(BOOLSETTING(FULL_ROW_SELECT)) {
		ctrlSearches.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);
	}

	ctrlSearches.SetBkColor(WinUtil::bgColor);
	ctrlSearches.SetTextBkColor(WinUtil::bgColor);
	ctrlSearches.SetTextColor(WinUtil::textColor);

	ctrlSearches.AddColumn(CSTRING(SEARCH_STRING), COLUMN_STRING, COLUMN_STRING);
	ctrlSearches.AddColumn(CSTRING(COUNT), COLUMN_COUNT, COLUMN_COUNT);

	ctrlSearches.setSort(COLUMN_COUNT, ExListViewCtrl::SORT_INT, false);

	SetWindowText(CSTRING(SEARCH_SPY));

	bHandled = FALSE;
	return 1;
}

void SpyFrame::UpdateLayout(BOOL bResizeBars /* = TRUE */) {
	RECT rect;
	GetClientRect(&rect);
	// position bars and offset their dimensions
	UpdateBarsPosition(rect, bResizeBars);

	if(ctrlStatus.IsWindow()) {
		CRect sr;
		int w[3];
		ctrlStatus.GetClientRect(sr);

		int tmp = (sr.Width()) > 316 ? 216 : ((sr.Width() > 116) ? sr.Width()-100 : 16);

		w[0] = sr.right - tmp;
		w[1] = w[0] + (tmp-16)*1/2;
		w[2] = w[0] + (tmp-16)*2/2;

		ctrlStatus.SetParts(3, w);
	}

	ctrlSearches.MoveWindow(&rect);
}

LRESULT SpyFrame::onSpeaker(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	if(wParam == SEARCH) {
		string* x = (string*)lParam;
		SearchIter i = searches.find(*x);
		int n;
		if(i == searches.end()) {
			n = searches[*x] = 1;

		} else {
			n = ++i->second;
		}

		total++;

		// Not thread safe, but who cares really...
		perSecond[cur]++;

		int j = ctrlSearches.find(*x);
		if(j == -1) {
			StringList a;
			a.push_back(*x);
			a.push_back(Util::toString(n));
			ctrlSearches.insert(a);
			if(ctrlSearches.GetItemCount() > 500) {
				ctrlSearches.DeleteItem(ctrlSearches.GetItemCount() - 1);
			}
		} else {
			ctrlSearches.SetItemText(j, COLUMN_COUNT, Util::toString(n).c_str());
			if(ctrlSearches.getSortColumn() == COLUMN_COUNT )
				ctrlSearches.resort();
		}
		delete x;

		ctrlStatus.SetText(1, (STRING(TOTAL) + Util::toString(total)).c_str());

	} else if(wParam == TICK_AVG) {
		float* x = (float*)lParam;
		ctrlStatus.SetText(2, (STRING(AVERAGE) + Util::toString(*x)).c_str());
		delete x;
	}

	return 0;
}

/**
 * @file SpyFrame.cpp
 * $Id: SpyFrame.cpp,v 1.4 2002/05/01 21:22:08 arnetheduck Exp $
 */

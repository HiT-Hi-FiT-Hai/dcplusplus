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

#include "SpyFrame.h"
#include "SearchFrm.h"
#include "WinUtil.h"

#include "../client/ShareManager.h"
#include "../client/ResourceManager.h"

LRESULT SpyFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	CreateSimpleStatusBar(ATL_IDS_IDLEMESSAGE, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SBARS_SIZEGRIP);
	ctrlStatus.Attach(m_hWndStatusBar);

	ctrlSearches.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL, WS_EX_CLIENTEDGE, IDC_RESULTS);

	if(BOOLSETTING(FULL_ROW_SELECT)) {
		ctrlSearches.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);
	}

	ctrlSearches.SetBkColor(WinUtil::bgColor);
	ctrlSearches.SetTextBkColor(WinUtil::bgColor);
	ctrlSearches.SetTextColor(WinUtil::textColor);

	ctrlSearches.AddColumn(CSTRING(SEARCH_STRING), COLUMN_STRING, COLUMN_STRING);
	ctrlSearches.AddColumn(CSTRING(COUNT), COLUMN_COUNT, COLUMN_COUNT);

	ctrlSearches.setSort(COLUMN_COUNT, ExListViewCtrl::SORT_INT, false);

	ShareManager::getInstance()->setHits(0);

	m_hMenu = WinUtil::mainMenu;

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
		int w[5];
		ctrlStatus.GetClientRect(sr);

		int tmp = (sr.Width()) > 616 ? 516 : ((sr.Width() > 116) ? sr.Width()-100 : 16);

		w[0] = sr.right - tmp;
		w[1] = w[0] + (tmp-16)*1/4;
		w[2] = w[0] + (tmp-16)*2/4;
		w[3] = w[0] + (tmp-16)*3/4;
		w[4] = w[0] + (tmp-16)*4/4;

		ctrlStatus.SetParts(5, w);
	}

	ctrlSearches.MoveWindow(&rect);
}

LRESULT SpyFrame::onSpeaker(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	if(wParam == SEARCH) {
		string* x = (string*)lParam;

		total++;

		// Not thread safe, but who cares really...
		perSecond[cur]++;

		int j = ctrlSearches.find(*x);
		if(j == -1) {
			StringList a;
			a.push_back(*x);
			a.push_back(Util::toString(1));
			ctrlSearches.insert(a);
			if(ctrlSearches.GetItemCount() > 500) {
				ctrlSearches.DeleteItem(ctrlSearches.GetItemCount() - 1);
			}
		} else {
			char tmp[32];
			ctrlSearches.GetItemText(j, COLUMN_COUNT, tmp, 32);
			ctrlSearches.SetItemText(j, COLUMN_COUNT, Util::toString(Util::toInt(tmp)+1).c_str());
			if(ctrlSearches.getSortColumn() == COLUMN_COUNT )
				ctrlSearches.resort();
		}
		delete x;

		ctrlStatus.SetText(1, (STRING(TOTAL) + Util::toString(total)).c_str());
		ctrlStatus.SetText(3, (STRING(HITS) + Util::toString(ShareManager::getInstance()->getHits())).c_str());
		double ratio = total > 0 ? ((double)ShareManager::getInstance()->getHits()) / (double)total : 0.0;
		ctrlStatus.SetText(4, (STRING(HIT_RATIO) + Util::toString(ratio)).c_str());
	} else if(wParam == TICK_AVG) {
		float* x = (float*)lParam;
		ctrlStatus.SetText(2, (STRING(AVERAGE) + Util::toString(*x)).c_str());
		delete x;
	}

	return 0;
}

LRESULT SpyFrame::onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	RECT rc;                    // client area of window 
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };        // location of mouse click 

	// Get the bounding rectangle of the client area. 
	ctrlSearches.GetClientRect(&rc);
	ctrlSearches.ScreenToClient(&pt); 

	if (PtInRect(&rc, pt) && ctrlSearches.GetSelectedCount() == 1) {
		int i = ctrlSearches.GetNextItem(-1, LVNI_SELECTED);

		CMenu mnu;
		mnu.CreatePopupMenu();
		mnu.AppendMenu(MF_STRING, IDC_SEARCH, CSTRING(SEARCH));
		char* buf = new char[256];
		ctrlSearches.GetItemText(i, COLUMN_STRING, buf, 256);
		searchString = buf;
		delete buf;

		ctrlSearches.ClientToScreen(&pt);
		mnu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
		
		return TRUE; 
	}

	return FALSE; 
}

LRESULT SpyFrame::onSearch(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	SearchFrame::openWindow(searchString);
	return 0;
};

void SpyFrame::onAction(ClientManagerListener::Types type, const string& s) throw() {
	switch(type) {
	case ClientManagerListener::INCOMING_SEARCH:
		{
			string* x = new string(s);
			string::size_type i = string::npos;
			while( (i=x->find('$')) != string::npos) {
				(*x)[i] = ' ';
			}
			PostMessage(WM_SPEAKER, SEARCH, (LPARAM)x);
		}
		break;
	}
}

void SpyFrame::onAction(TimerManagerListener::Types type, u_int32_t) throw() {
	switch(type) {
	case TimerManagerListener::SECOND: 
		float* f = new float(0.0);
		for(int i = 0; i < AVG_TIME; ++i) {
			(*f) += (float)perSecond[i];
		}
		(*f) /= AVG_TIME;
		
		perSecond[++cur] = 0;
		PostMessage(WM_SPEAKER, TICK_AVG, (LPARAM)f);
	}
}

/**
 * @file
 * $Id: SpyFrame.cpp,v 1.17 2003/11/11 20:31:57 arnetheduck Exp $
 */

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

#if !defined(AFX_SPYFRAME_H__19A67830_B811_4672_BBC2_3D793E0342E8__INCLUDED_)
#define AFX_SPYFRAME_H__19A67830_B811_4672_BBC2_3D793E0342E8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../client/ClientManager.h"

#include "FlatTabCtrl.h"
#include "ExListViewCtrl.h"

class SpyFrame : public MDITabChildWindowImpl<SpyFrame>, private ClientManagerListener
{
public:

	static SpyFrame* frame;
	
	SpyFrame() {
		ClientManager::getInstance()->addListener(this);
	}
	virtual ~SpyFrame() {
	}

	enum {
		COLUMN_FIRST,
		COLUMN_STRING = COLUMN_FIRST,
		COLUMN_COUNT,
		COLUMN_LAST
	};

	DECLARE_FRAME_WND_CLASS("SpyFrame", IDR_SPY)

	virtual void OnFinalMessage(HWND /*hWnd*/) { delete this; }

	typedef MDITabChildWindowImpl<SpyFrame> baseClass;
	BEGIN_MSG_MAP(SpyFrame)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_FORWARDMSG, OnForwardMsg)
		MESSAGE_HANDLER(WM_SPEAKER, onSpeaker)
		MESSAGE_HANDLER(WM_CLOSE, onClose)
		NOTIFY_HANDLER(IDC_RESULTS, LVN_COLUMNCLICK, onColumnClickResults)
		CHAIN_MSG_MAP(baseClass)
	END_MSG_MAP()

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);

	LRESULT onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
		ClientManager::getInstance()->removeListener(this);
		bHandled = FALSE;
		return 0;
	}
	
	LRESULT OnForwardMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
		return baseClass::PreTranslateMessage((LPMSG)lParam);
	}

	LRESULT onColumnClickResults(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
		NMLISTVIEW* l = (NMLISTVIEW*)pnmh;
		if(l->iSubItem == ctrlSearches.getSortColumn()) {
			ctrlSearches.setSortDirection(!ctrlSearches.getSortDirection());
		} else {
			if(l->iSubItem == COLUMN_COUNT) {
				ctrlSearches.setSort(l->iSubItem, ExListViewCtrl::SORT_INT);
			} else {
				ctrlSearches.setSort(l->iSubItem, ExListViewCtrl::SORT_STRING_NOCASE);
			}
		}
		return 0;
	}

	LRESULT onSpeaker(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/){
		string* x = (string*)lParam;
		SearchIter i = searches.find(*x);
		int n;
		if(i == searches.end()) {
			n = searches[*x] = 1;
		
		} else {
			n = ++i->second;
		}

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
			if(ctrlSearches.getSortColumn() == COLUMN_COUNT)
				ctrlSearches.resort();
		}
		
		delete x;

		return 0;
	}

	void UpdateLayout(BOOL bResizeBars = TRUE);
	
private:

	ExListViewCtrl ctrlSearches;
	CStatusBarCtrl ctrlStatus;
	
	typedef HASH_MAP<string, int> SearchMap;
	typedef SearchMap::iterator SearchIter;

	SearchMap searches;

	// ClientManagerListener
	virtual void onAction(ClientManagerListener::Types type, const string& s) {
		switch(type) {
		case ClientManagerListener::INCOMING_SEARCH:
			{
				string* x = new string(s);
				int i = -1;
				while( (i=x->find('$')) != string::npos) {
					(*x)[i] = ' ';
				}
				PostMessage(WM_SPEAKER, 0, (LPARAM)x);
			}
			break;
		}
	}
	
};

#endif // !defined(AFX_SPYFRAME_H__19A67830_B811_4672_BBC2_3D793E0342E8__INCLUDED_)

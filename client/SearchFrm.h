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

#if !defined(AFX_SEARCHFRM_H__A7078724_FD85_4F39_8463_5A08A5F45E33__INCLUDED_)
#define AFX_SEARCHFRM_H__A7078724_FD85_4F39_8463_5A08A5F45E33__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "AtlCmdBar2.h"
#include "DownloadManager.h"
#include "SearchManager.h"
#include "ExListViewCtrl.h"

#define SEARCH_MESSAGE_MAP 6		// This could be any number, really...

class SearchFrame : public CMDIChildWindowImpl2<SearchFrame>, private SearchManagerListener
{
public:
	DECLARE_FRAME_WND_CLASS("Search", IDR_MDICHILD)

	virtual void OnFinalMessage(HWND /*hWnd*/)
	{
		delete this;
	}

	BEGIN_MSG_MAP(SearchFrame)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_FORWARDMSG, OnForwardMsg)
		CHAIN_MSG_MAP(CMDIChildWindowImpl2<SearchFrame>)
	ALT_MSG_MAP(SEARCH_MESSAGE_MAP)
		MESSAGE_HANDLER(WM_CHAR, OnChar)
	END_MSG_MAP()

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);

	LRESULT OnForwardMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
	{
		LPMSG pMsg = (LPMSG)lParam;

		return CMDIChildWindowImpl2<SearchFrame>::PreTranslateMessage(pMsg);
	}

	void UpdateLayout(BOOL bResizeBars = TRUE)
	{
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
			w[1] = w[0] + (tmp-16)/2;
			w[2] = w[0] + (tmp-16);
			
			ctrlStatus.SetParts(3, w);
		}
		
		CRect rc = rect;
		rc.top +=22;
		ctrlResults.MoveWindow(rc);

		rc.bottom = rc.top - 2;
		rc.top -= 20;
		ctrlSearch.MoveWindow(rc);
		
	}

	LRESULT OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		char* message;
		
		if(wParam == VK_RETURN && ctrlSearch.GetWindowTextLength() > 0) {
			message = new char[ctrlSearch.GetWindowTextLength()+1];
			ctrlSearch.GetWindowText(message, ctrlSearch.GetWindowTextLength()+1);
			string s(message, ctrlSearch.GetWindowTextLength());
			delete message;
			ctrlResults.DeleteAllItems();
			SearchManager::getInstance()->search(s);
			//client->sendMessage(s);
			ctrlSearch.SetWindowText("");
		} else {
			bHandled = FALSE;
		}
		return 0;
	}
	
	SearchFrame() : ctrlSearchContainer("edit", this, SEARCH_MESSAGE_MAP) {
		SearchManager::getInstance()->addListener(this);
	}

	~SearchFrame() {
		SearchManager::getInstance()->removeListener(this);
	}


private:
	CStatusBarCtrl ctrlStatus;
	CEdit ctrlSearch;
	CContainedWindow ctrlSearchContainer;
	ExListViewCtrl ctrlResults;


	virtual void onSearchResult(SearchResult* aResult) {
		int i = ctrlResults.insert(ctrlResults.GetItemCount(), aResult->getNick());
		ctrlResults.SetItemText(i, 1, aResult->getFile().c_str());
		ctrlResults.SetItemText(i, 2, Util::shortenBytes(aResult->getSize()).c_str());
		ctrlResults.SetItemText(i, 3, aResult->getSlotString().c_str());
		ctrlResults.SetItemText(i, 4, aResult->getHubName().c_str());
	}
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHILDFRM_H__A7078724_FD85_4F39_8463_5A08A5F45E33__INCLUDED_)

/**
 * @file SearchFrm.h
 * $Id: SearchFrm.h,v 1.1 2001/12/10 10:50:10 arnetheduck Exp $
 * @if LOG
 * $Log: SearchFrm.h,v $
 * Revision 1.1  2001/12/10 10:50:10  arnetheduck
 * Oops, forgot the search frame...
 *
 * @endif
 */


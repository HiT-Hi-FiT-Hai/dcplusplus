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

#include "FlatTabCtrl.h"
#include "DownloadManager.h"
#include "SearchManager.h"
#include "ExListViewCtrl.h"
#include "StringTokenizer.h"

#define SEARCH_MESSAGE_MAP 6		// This could be any number, really...

#define WM_ENTER (WM_USER + 120)
#define WM_TAB (WM_ENTER + 1)

class SearchFrame : public MDITabChildWindowImpl<SearchFrame>, private SearchManagerListener
{
public:

	enum {
		COLUMN_NICK,
		COLUMN_FILENAME,
		COLUMN_TYPE,
		COLUMN_SIZE,
		COLUMN_PATH,
		COLUMN_SLOTS,
		COLUMN_CONNECTION,
		COLUMN_HUB
	};

	DECLARE_FRAME_WND_CLASS("SearchFrame", IDR_SEARCH)

	virtual void OnFinalMessage(HWND /*hWnd*/)
	{
		delete this;
	}

	BEGIN_MSG_MAP(SearchFrame)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_FORWARDMSG, OnForwardMsg)
		MESSAGE_HANDLER(WM_PAINT, onPaint)
		MESSAGE_HANDLER(WM_SETFOCUS, OnFocus)
		MESSAGE_HANDLER(WM_DESTROY, onDestroy)
		MESSAGE_HANDLER(WM_ERASEBKGND, onEraseBackground)
		MESSAGE_HANDLER(WM_CONTEXTMENU, onContextMenu)
		MESSAGE_HANDLER(WM_ENTER, onEnter)
		MESSAGE_HANDLER(WM_TAB, onTab)
		MESSAGE_HANDLER(WM_SPEAKER, onSpeaker)
		NOTIFY_HANDLER(IDC_RESULTS, NM_DBLCLK, onDoubleClickResults)
		NOTIFY_HANDLER(IDC_RESULTS, LVN_COLUMNCLICK, onColumnClickResults)
		COMMAND_ID_HANDLER(IDC_DOWNLOAD, onDownload)
		COMMAND_ID_HANDLER(IDC_GETLIST, onGetList)
		COMMAND_ID_HANDLER(IDC_DOWNLOADTO, onDownloadTo)
		CHAIN_MSG_MAP(MDITabChildWindowImpl<SearchFrame>)
	ALT_MSG_MAP(SEARCH_MESSAGE_MAP)
		MESSAGE_HANDLER(WM_CHAR, onChar)
		MESSAGE_HANDLER(WM_KEYDOWN, onChar)
		MESSAGE_HANDLER(WM_KEYUP, onChar)
	END_MSG_MAP()

	static int sortSize(LPARAM a, LPARAM b) {
		LONGLONG* c = (LONGLONG*)a;
		LONGLONG* d = (LONGLONG*)b;
		
		if(*c < *d) {
			return -1;
		} else if(*c == *d) {
			return 0;
		} else {
			return 1;
		}
	}

	LRESULT onColumnClickResults(int idCtrl, LPNMHDR pnmh, BOOL& bHandled) {
		NMLISTVIEW* l = (NMLISTVIEW*)pnmh;
		if(l->iSubItem == ctrlResults.getSortColumn()) {
			ctrlResults.setSortDirection(!ctrlResults.getSortDirection());
		} else {
			if(l->iSubItem == 3) {
				ctrlResults.setSort(l->iSubItem, ExListViewCtrl::SORT_FUNC, true, sortSize);
			} else {
				ctrlResults.setSort(l->iSubItem, ExListViewCtrl::SORT_STRING_NOCASE);
			}
		}
		return 0;
	}
	
	LRESULT onDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled) {
		for(int i = 0; i != ctrlResults.GetItemCount(); i++) {
			delete (LONGLONG*)ctrlResults.GetItemData(i);
		}
		bHandled = FALSE;
		return 0;
	}				

	LRESULT onDownload(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		downloadSelected(SETTING(DOWNLOAD_DIRECTORY));
		return 0;
	}
	
	LRESULT onDownloadTo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	
	void downloadSelected(const string& aDir) {
		int i=-1;
		char buf[256];
		
		while( (i = ctrlResults.GetNextItem(i, LVNI_SELECTED)) != -1) {
			ctrlResults.GetItemText(i, COLUMN_NICK, buf, 256);
			string user = buf;
			ctrlResults.GetItemText(i, COLUMN_FILENAME, buf, 256);
			string file = buf;
			LONGLONG size = *(LONGLONG*)ctrlResults.GetItemData(i);
			ctrlResults.GetItemText(i, COLUMN_PATH, buf, 256);
			string path = buf;
			try {
				DownloadManager::getInstance()->download(path + file, size, user, aDir + file);
			} catch(Exception e) {
				MessageBox(e.getError().c_str());
			}
		}
	}

	LRESULT onGetList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		int i=-1;
		char buf[256];
		while( (i = ctrlResults.GetNextItem(i, LVNI_SELECTED)) != -1) {
			ctrlResults.GetItemText(i, COLUMN_NICK, buf, 256);
			try {
				DownloadManager::getInstance()->downloadList(buf);
			} catch(...) {
				// ...
			}
		}
		return 0;
	}
	
	LRESULT onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled) {
		RECT rc;                    // client area of window 
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };        // location of mouse click 
		
		// Get the bounding rectangle of the client area. 
		ctrlResults.GetClientRect(&rc);
		ctrlResults.ScreenToClient(&pt); 
		
		if (PtInRect(&rc, pt)) 
		{ 
			ctrlResults.ClientToScreen(&pt);
			resultsMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
			
			return TRUE; 
		}
		
		return FALSE; 
	}

	LRESULT onDoubleClickResults(int idCtrl, LPNMHDR pnmh, BOOL& bHandled) {
		NMITEMACTIVATE* item = (NMITEMACTIVATE*)pnmh;
		char buf[256];
		
		if(item->iItem != -1) {
			ctrlResults.GetItemText(item->iItem, COLUMN_NICK, buf, 256);
			string user = buf;
			ctrlResults.GetItemText(item->iItem, COLUMN_FILENAME, buf, 256);
			string file = buf;
			LONGLONG size = *(LONGLONG*)ctrlResults.GetItemData(item->iItem);
			ctrlResults.GetItemText(item->iItem, COLUMN_PATH, buf, 256);
			string path = buf;
			
			try { 
				DownloadManager::getInstance()->download(path + file, size, user, SETTING(DOWNLOAD_DIRECTORY) + file);
			} catch(Exception e) {
				MessageBox(e.getError().c_str());
			}
		}
		return 0;
		
	}

	LRESULT OnFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
		ctrlSearch.SetFocus();
		return 0;
	}

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);

	LRESULT onEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		return 0;
	}
		
	LRESULT onPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(&ps);
		FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_ACTIVEBORDER+1));
		EndPaint(&ps);
		return 0;
	}

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
		rc.top +=25;
		ctrlResults.MoveWindow(rc);

		rc.bottom = rc.top - 2;
		rc.top -= 23;
		rc.right -= 200;
		ctrlSearch.MoveWindow(rc);
		
		rc.left = rc.right;
		rc.right += 73;
		rc.bottom += 50;
		ctrlMode.MoveWindow(rc);
		
		rc.bottom -= 50;
		rc.left = rc.right + 2;
		rc.right += 75;
		ctrlSize.MoveWindow(rc);

		rc.left = rc.right + 2;
		rc.right += 50;
		rc.bottom += 70;
		ctrlSizeMode.MoveWindow(rc);
	}

	LRESULT onSpeaker(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		ctrlResults.insert(*(StringList*)wParam, 0, lParam);
		delete (StringList*)wParam;
		return 0;
	}

	LRESULT onChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		switch (uMsg) 
		{ 
        case WM_KEYDOWN: 
            switch (wParam) { 
			case VK_TAB: 
				SendMessage(WM_TAB); 
				return 0; 
			case VK_RETURN: 
				SendMessage(WM_ENTER); 
				return 0; 
			default:
				bHandled = FALSE;
            } 
            break; 
			
		case WM_KEYUP: 
		case WM_CHAR: 
			switch (wParam) { 
            case VK_TAB:		// Fall through
            case VK_RETURN: 
                return 0; 
			default: 
				bHandled = FALSE;
			} 
			break;
		default:
			bHandled = FALSE;
		}
		return 0;
	}
	
	LRESULT onEnter(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	
	LRESULT onTab(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		HWND focus = GetFocus();
		if(focus == ctrlSearch.m_hWnd) {
			ctrlMode.SetFocus();
		} else if(focus == ctrlMode.m_hWnd) {
			ctrlSize.SetFocus();
		} else if(focus == ctrlSize.m_hWnd) {
			ctrlSizeMode.SetFocus();
		} else if(focus == ctrlSizeMode.m_hWnd) {
			ctrlSearch.SetFocus();
		}
		return 0;
	}

	SearchFrame() : searchContainer("edit", this, SEARCH_MESSAGE_MAP),  sizeContainer("edit", this, SEARCH_MESSAGE_MAP), 
		modeContainer("COMBOBOX", this, SEARCH_MESSAGE_MAP), sizeModeContainer("COMBOBOX", this, SEARCH_MESSAGE_MAP) {
		
		SearchManager::getInstance()->addListener(this);
	}

	~SearchFrame() {
		SearchManager::getInstance()->removeListener(this);
	}


private:
	CStatusBarCtrl ctrlStatus;
	CEdit ctrlSearch;
	CEdit ctrlSize;
	CComboBox ctrlMode;
	CComboBox ctrlSizeMode;
	
	CContainedWindow searchContainer;
	CContainedWindow sizeContainer;
	CContainedWindow modeContainer;
	CContainedWindow sizeModeContainer;
	
	ExListViewCtrl ctrlResults;
	CMenu resultsMenu;
	
	StringList search;

	// SearchManagerListener
	virtual void onAction(SearchManagerListener::Types type, SearchResult* sr) {
		switch(type) {
		case SearchManagerListener::SEARCH_RESULT:
			onSearchResult(sr); break;
		}
	}
	
	void onSearchResult(SearchResult* aResult);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHILDFRM_H__A7078724_FD85_4F39_8463_5A08A5F45E33__INCLUDED_)

/**
 * @file SearchFrm.h
 * $Id: SearchFrm.h,v 1.19 2002/01/15 21:57:53 arnetheduck Exp $
 * @if LOG
 * $Log: SearchFrm.h,v $
 * Revision 1.19  2002/01/15 21:57:53  arnetheduck
 * Hopefully fixed the two annoying bugs...
 *
 * Revision 1.18  2002/01/13 22:50:48  arnetheduck
 * Time for 0.12, added favorites, a bunch of new icons and lot's of other stuff
 *
 * Revision 1.17  2002/01/11 16:13:33  arnetheduck
 * Fixed some locks and bugs, added type field to the search frame
 *
 * Revision 1.16  2002/01/11 14:52:57  arnetheduck
 * Huge changes in the listener code, replaced most of it with templates,
 * also moved the getinstance stuff for the managers to a template
 *
 * Revision 1.15  2002/01/10 12:33:14  arnetheduck
 * Various fixes
 *
 * Revision 1.14  2002/01/07 23:05:48  arnetheduck
 * Resume rollback implemented
 *
 * Revision 1.13  2002/01/06 21:55:20  arnetheduck
 * Some minor bugs fixed, but there remains one strange thing, the reconnect
 * button doesn't work...
 *
 * Revision 1.12  2002/01/06 00:14:54  arnetheduck
 * Incoming searches almost done, just need some testing...
 *
 * Revision 1.11  2002/01/05 19:06:09  arnetheduck
 * Added user list images, fixed bugs and made things more effective
 *
 * Revision 1.9  2002/01/05 10:13:40  arnetheduck
 * Automatic version detection and some other updates
 *
 * Revision 1.8  2002/01/02 16:55:56  arnetheduck
 * Time for 0.09
 *
 * Revision 1.7  2002/01/02 16:12:33  arnetheduck
 * Added code for multiple download sources
 *
 * Revision 1.6  2001/12/29 13:47:14  arnetheduck
 * Fixing bugs and UI work
 *
 * Revision 1.5  2001/12/27 12:05:00  arnetheduck
 * Added flat tabs, fixed sorting and a StringTokenizer bug
 *
 * Revision 1.4  2001/12/21 20:21:17  arnetheduck
 * Private messaging added, and a lot of other updates as well...
 *
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


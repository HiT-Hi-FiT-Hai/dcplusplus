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

class SearchFrame : public MDITabChildWindowImpl<SearchFrame>, private SearchManagerListener
{
public:
	DECLARE_FRAME_WND_CLASS("SearchFrame", IDR_MDICHILD)

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
		NOTIFY_HANDLER(IDC_RESULTS, NM_DBLCLK, onDoubleClickResults)
		NOTIFY_HANDLER(IDC_RESULTS, LVN_COLUMNCLICK, onColumnClickResults)
		COMMAND_ID_HANDLER(IDC_DOWNLOAD, onDownload)
		COMMAND_ID_HANDLER(IDC_GETLIST, onGetList)
		COMMAND_ID_HANDLER(IDC_DOWNLOADTO, onDownloadTo)
		CHAIN_MSG_MAP(MDITabChildWindowImpl<SearchFrame>)
	ALT_MSG_MAP(SEARCH_MESSAGE_MAP)
		MESSAGE_HANDLER(WM_CHAR, OnChar)
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
			if(l->iSubItem == 2) {
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
		downloadSelected(Settings::getDownloadDirectory());
		return 0;
	}
	
	LRESULT onDownloadTo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		char buf[512];
		if(ctrlResults.GetSelectedCount() == 1) {
			int i = ctrlResults.GetNextItem(-1, LVNI_SELECTED);
			ctrlResults.GetItemText(i, 1, buf, 512);
			string file = buf;
			string target = Settings::getDownloadDirectory() + buf;
			if(Util::browseSaveFile(target)) {
				ctrlResults.GetItemText(i, 0, buf, 512);
				string user = buf;
				LONGLONG size = *(LONGLONG*)ctrlResults.GetItemData(i);
				ctrlResults.GetItemText(i, 3, buf, 512);
				string path = buf;
				
				DownloadManager::getInstance()->download(path + file, size, user, target);
			}
		} else {
			string target = Settings::getDownloadDirectory();
			if(Util::browseDirectory(target, m_hWnd)) {
				downloadSelected(target);
			}
		}
		return 0;
	}
	
	void downloadSelected(const string& aDir) {
		int i=-1;
		char buf[256];
		
		while( (i = ctrlResults.GetNextItem(i, LVNI_SELECTED)) != -1) {
			ctrlResults.GetItemText(i, 0, buf, 256);
			string user = buf;
			ctrlResults.GetItemText(i, 1, buf, 256);
			string file = buf;
			LONGLONG size = *(LONGLONG*)ctrlResults.GetItemData(i);
			ctrlResults.GetItemText(i, 3, buf, 256);
			string path = buf;
			
			DownloadManager::getInstance()->download(path + file, size, user, aDir + file);
		}
	}

	LRESULT onGetList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		int i=-1;
		char buf[256];
		while( (i = ctrlResults.GetNextItem(i, LVNI_SELECTED)) != -1) {
			ctrlResults.GetItemText(i, 0, buf, 256);
			DownloadManager::getInstance()->downloadList(buf);
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
			resultsMenu.TrackPopupMenuEx(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
			
			return TRUE; 
		}
		
		return FALSE; 
	}

	LRESULT onDoubleClickResults(int idCtrl, LPNMHDR pnmh, BOOL& bHandled) {
		NMITEMACTIVATE* item = (NMITEMACTIVATE*)pnmh;
		char buf[256];
		
		if(item->iItem != -1) {
			ctrlResults.GetItemText(item->iItem, 0, buf, 256);
			string user = buf;
			ctrlResults.GetItemText(item->iItem, 1, buf, 256);
			string file = buf;
			LONGLONG size = *(LONGLONG*)ctrlResults.GetItemData(item->iItem);
			ctrlResults.GetItemText(item->iItem, 3, buf, 256);
			string path = buf;
			
			DownloadManager::getInstance()->download(path + file, size, user, Settings::getDownloadDirectory() + file);
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
		rc.top +=24;
		ctrlResults.MoveWindow(rc);

		rc.bottom = rc.top - 2;
		rc.top -= 22;
		ctrlSearch.MoveWindow(rc);
		
	}

	LRESULT OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		char* message;
		
		if(wParam == VK_RETURN && ctrlSearch.GetWindowTextLength() > 0) {
			message = new char[ctrlSearch.GetWindowTextLength()+1];
			ctrlSearch.GetWindowText(message, ctrlSearch.GetWindowTextLength()+1);
			string s(message, ctrlSearch.GetWindowTextLength());
			delete message;
			for(int i = 0; i != ctrlResults.GetItemCount(); i++) {
				delete (LONGLONG*)ctrlResults.GetItemData(i);
			}
			ctrlResults.DeleteAllItems();
			SearchManager::getInstance()->search(s);
			//client->sendMessage(s);
			ctrlSearch.SetWindowText("");
			
			ctrlStatus.SetText(0, ("Searching for " + s + "...").c_str());
			search = StringTokenizer(s, ' ').getTokens();

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
	CMenu resultsMenu;
	
	StringList search;

	virtual void onSearchResult(SearchResult* aResult) {
		// Check that this is really a relevant search result...
		for(StringIter j = search.begin(); j != search.end(); ++j) {
			if(Util::findSubString(aResult->getFile(), *j) == -1) {
				return;
			}
		}
		LONGLONG* psize = new LONGLONG;
		*psize = aResult->getSize();

		int i = ctrlResults.insert(ctrlResults.GetItemCount(), aResult->getNick(), 0, (LPARAM)psize);
		string file, path;
		
		if(aResult->getFile().rfind('\\') == string::npos) {
			file = aResult->getFile();
		} else {
			file = aResult->getFile().substr(aResult->getFile().rfind('\\')+1);
			path = aResult->getFile().substr(0, aResult->getFile().rfind('\\')+1);
		}
		ctrlResults.SetItemText(i, 1, file.c_str());
		ctrlResults.SetItemText(i, 2, Util::formatBytes(aResult->getSize()).c_str());
		ctrlResults.SetItemText(i, 3, path.c_str());
		ctrlResults.SetItemText(i, 4, aResult->getSlotString().c_str());
		ctrlResults.SetItemText(i, 5, aResult->getHubName().c_str());


	}
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHILDFRM_H__A7078724_FD85_4F39_8463_5A08A5F45E33__INCLUDED_)

/**
 * @file SearchFrm.h
 * $Id: SearchFrm.h,v 1.9 2002/01/05 10:13:40 arnetheduck Exp $
 * @if LOG
 * $Log: SearchFrm.h,v $
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


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
	searchContainer.SubclassWindow(ctrlSearch.m_hWnd);
	
	ctrlMode.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		WS_HSCROLL | WS_VSCROLL | CBS_DROPDOWNLIST, WS_EX_CLIENTEDGE, IDC_RESULTS);
	modeContainer.SubclassWindow(ctrlMode.m_hWnd);

	ctrlSize.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		ES_AUTOHSCROLL, WS_EX_CLIENTEDGE);
	sizeContainer.SubclassWindow(ctrlSize.m_hWnd);
	
	ctrlSizeMode.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		WS_HSCROLL | WS_VSCROLL | CBS_DROPDOWNLIST, WS_EX_CLIENTEDGE, IDC_RESULTS);
	sizeModeContainer.SubclassWindow(ctrlSizeMode.m_hWnd);

	ctrlResults.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS, WS_EX_CLIENTEDGE, IDC_RESULTS);

	ctrlSearch.SetFont(ctrlResults.GetFont(), FALSE);
	ctrlSize.SetFont(ctrlResults.GetFont(), FALSE);
	ctrlMode.SetFont(ctrlResults.GetFont(), FALSE);
	ctrlSizeMode.SetFont(ctrlResults.GetFont(), FALSE);
	
	ctrlMode.AddString("Normal");
	ctrlMode.AddString("At Least");
	ctrlMode.AddString("At Most");
	ctrlMode.SetCurSel(1);
	
	ctrlSizeMode.AddString("B");
	ctrlSizeMode.AddString("kB");
	ctrlSizeMode.AddString("MB");
	ctrlSizeMode.AddString("GB");
	
	ctrlSizeMode.SetCurSel(2);

	ctrlResults.InsertColumn(0, _T("User"), LVCFMT_LEFT, 100, 0);
	ctrlResults.InsertColumn(1, _T("File"), LVCFMT_LEFT, 200, 1);
	ctrlResults.InsertColumn(2, _T("Type"), LVCFMT_LEFT, 50, 2);
	ctrlResults.InsertColumn(3, _T("Size"), LVCFMT_RIGHT, 80, 3);
	ctrlResults.InsertColumn(4, _T("Path"), LVCFMT_LEFT, 100, 4);
	ctrlResults.InsertColumn(5, _T("Slots"), LVCFMT_LEFT, 40, 5);
	ctrlResults.InsertColumn(6, _T("Connection"), LVCFMT_LEFT, 70, 6);
	ctrlResults.InsertColumn(7, _T("Hub"), LVCFMT_LEFT, 150, 7);

	SetWindowText("Search");

	resultsMenu.CreatePopupMenu();
	
	CMenuItemInfo mi;
	mi.fMask = MIIM_ID | MIIM_TYPE;
	mi.fType = MFT_STRING;
	mi.cch = 13;
	mi.dwTypeData = "Get File List";
	mi.wID = IDC_GETLIST;
	resultsMenu.InsertMenuItem(0, TRUE, &mi);

	mi.fMask = MIIM_ID | MIIM_TYPE;
	mi.fType = MFT_STRING;
	mi.cch = 16;
	mi.dwTypeData = "Download file(s)";
	mi.wID = IDC_DOWNLOAD;
	resultsMenu.InsertMenuItem(1, TRUE, &mi);

	mi.fMask = MIIM_ID | MIIM_TYPE;
	mi.fType = MFT_STRING;
	mi.cch = 22;
	mi.dwTypeData = "Download file(s) to...";
	mi.wID = IDC_DOWNLOADTO;
	resultsMenu.InsertMenuItem(2, TRUE, &mi);
	
	bHandled = FALSE;
	
	return 1;
}

LRESULT SearchFrame::onDownloadTo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	char buf[512];
	if(ctrlResults.GetSelectedCount() == 1) {
		int i = ctrlResults.GetNextItem(-1, LVNI_SELECTED);
		ctrlResults.GetItemText(i, 1, buf, 512);
		string file = buf;
		string target = SETTING(DOWNLOAD_DIRECTORY) + buf;
		if(Util::browseSaveFile(target)) {
			ctrlResults.GetItemText(i, 0, buf, 512);
			string user = buf;
			LONGLONG size = *(LONGLONG*)ctrlResults.GetItemData(i);
			ctrlResults.GetItemText(i, 3, buf, 512);
			string path = buf;
			
			try {
				DownloadManager::getInstance()->download(path + file, size, user, target);
			} catch(Exception e) {
				MessageBox(e.getError().c_str());
			}
		}
	} else {
		string target = SETTING(DOWNLOAD_DIRECTORY);
		if(Util::browseDirectory(target, m_hWnd)) {
			downloadSelected(target);
		}
	}
	return 0;
}

LRESULT SearchFrame::onEnter(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	char* message;
	
	if(ctrlSearch.GetWindowTextLength() > 0) {
		message = new char[ctrlSearch.GetWindowTextLength()+1];
		ctrlSearch.GetWindowText(message, ctrlSearch.GetWindowTextLength()+1);
		string s(message, ctrlSearch.GetWindowTextLength());
		delete message;
		
		message = new char[ctrlSize.GetWindowTextLength()+1];
		ctrlSize.GetWindowText(message, ctrlSize.GetWindowTextLength()+1);
		string size(message, ctrlSize.GetWindowTextLength());
		delete message;
		
		double lsize = Util::toInt64(size);
		switch(ctrlSizeMode.GetCurSel()) {
		case 1:
			lsize*=1024I64; break;
		case 2:
			lsize*=1024I64*1024I64; break;
		case 3:
			lsize*=1024I64*1024I64*1024I64; break;
		}
		
		for(int i = 0; i != ctrlResults.GetItemCount(); i++) {
			delete (LONGLONG*)ctrlResults.GetItemData(i);
		}
		ctrlResults.DeleteAllItems();
		
		SearchManager::getInstance()->search(s, (LONGLONG)lsize, 0, ctrlMode.GetCurSel());
		//client->sendMessage(s);
		ctrlSearch.SetWindowText("");
		
		ctrlStatus.SetText(0, ("Searching for " + s + "...").c_str());
		search = StringTokenizer(s, ' ').getTokens();
		
	}
	return 0;
}

void SearchFrame::onSearchResult(SearchResult* aResult) {
	// Check that this is really a relevant search result...
	for(StringIter j = search.begin(); j != search.end(); ++j) {
		if(Util::findSubString(aResult->getFile(), *j) == -1) {
			return;
		}
	}
	LONGLONG* psize = new LONGLONG;
	*psize = aResult->getSize();
	
	string file, path;
	if(aResult->getFile().rfind('\\') == string::npos) {
		file = aResult->getFile();
	} else {
		file = aResult->getFile().substr(aResult->getFile().rfind('\\')+1);
		path = aResult->getFile().substr(0, aResult->getFile().rfind('\\')+1);
	}
	
	StringList* l = new StringList();
	l->push_back(aResult->getNick());
	l->push_back(file);
	int i = file.rfind('.');
	if(i != string::npos) {
		l->push_back(file.substr(i + 1));
	} else {
		l->push_back("");
	}
	l->push_back(Util::formatBytes(aResult->getSize()));
	l->push_back(path);
	l->push_back(aResult->getSlotString());
	if(aResult->getUser()) {
		l->push_back(aResult->getUser()->getConnection());
	} else {
		l->push_back("");
	}
	l->push_back(aResult->getHubName());
	PostMessage(WM_SPEAKER, (WPARAM)l, (LPARAM)psize);	
}

/**
 * @file SearchFrm.cpp
 * $Id: SearchFrm.cpp,v 1.13 2002/01/13 22:50:48 arnetheduck Exp $
 * @if LOG
 * $Log: SearchFrm.cpp,v $
 * Revision 1.13  2002/01/13 22:50:48  arnetheduck
 * Time for 0.12, added favorites, a bunch of new icons and lot's of other stuff
 *
 * Revision 1.12  2002/01/11 16:13:33  arnetheduck
 * Fixed some locks and bugs, added type field to the search frame
 *
 * Revision 1.11  2002/01/11 14:52:57  arnetheduck
 * Huge changes in the listener code, replaced most of it with templates,
 * also moved the getinstance stuff for the managers to a template
 *
 * Revision 1.10  2002/01/09 19:01:35  arnetheduck
 * Made some small changed to the key generation and search frame...
 *
 * Revision 1.9  2002/01/07 20:17:59  arnetheduck
 * Finally fixed the reconnect bug that's been annoying me for a whole day...
 * Hopefully the app works better in w95 now too...
 *
 * Revision 1.8  2002/01/05 19:06:09  arnetheduck
 * Added user list images, fixed bugs and made things more effective
 *
 * Revision 1.6  2002/01/05 10:13:40  arnetheduck
 * Automatic version detection and some other updates
 *
 * Revision 1.5  2001/12/29 13:47:14  arnetheduck
 * Fixing bugs and UI work
 *
 * Revision 1.4  2001/12/27 12:05:00  arnetheduck
 * Added flat tabs, fixed sorting and a StringTokenizer bug
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


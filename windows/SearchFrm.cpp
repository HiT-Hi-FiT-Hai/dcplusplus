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

#include "SearchFrm.h"
#include "LineDlg.h"
#include "PrivateFrame.h"

#include "../client/QueueManager.h"
#include "../client/StringTokenizer.h"
#include "../client/HubManager.h"

StringList SearchFrame::lastSearches;

int SearchFrame::columnIndexes[] = { COLUMN_NICK, COLUMN_FILENAME, COLUMN_TYPE, COLUMN_SIZE,
	COLUMN_PATH, COLUMN_SLOTS, COLUMN_CONNECTION, COLUMN_HUB, COLUMN_EXACT_SIZE };
int SearchFrame::columnSizes[] = { 100, 200, 50, 80, 100, 40, 70, 150, 80 };

static ResourceManager::Strings columnNames[] = { ResourceManager::USER, ResourceManager::FILE, ResourceManager::TYPE, ResourceManager::SIZE, 
	ResourceManager::PATH, ResourceManager::SLOTS, ResourceManager::CONNECTION, 
	ResourceManager::HUB, ResourceManager::EXACT_SIZE };

LRESULT SearchFrame::onCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	CreateSimpleStatusBar(ATL_IDS_IDLEMESSAGE, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SBARS_SIZEGRIP);
	ctrlStatus.Attach(m_hWndStatusBar);

	ctrlSearchBox.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		WS_VSCROLL | CBS_DROPDOWN | CBS_AUTOHSCROLL, 0);
	for(StringIter i = lastSearches.begin(); i != lastSearches.end(); ++i) {
		ctrlSearchBox.InsertString(0, i->c_str());
	}
	searchBoxContainer.SubclassWindow(ctrlSearchBox.m_hWnd);
	ctrlSearchBox.SetExtendedUI();
	
	ctrlMode.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		WS_HSCROLL | WS_VSCROLL | CBS_DROPDOWNLIST, WS_EX_CLIENTEDGE);
	modeContainer.SubclassWindow(ctrlMode.m_hWnd);

	ctrlSize.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		ES_AUTOHSCROLL, WS_EX_CLIENTEDGE);
	sizeContainer.SubclassWindow(ctrlSize.m_hWnd);
	
	ctrlSizeMode.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		WS_HSCROLL | WS_VSCROLL | CBS_DROPDOWNLIST, WS_EX_CLIENTEDGE);
	sizeModeContainer.SubclassWindow(ctrlSizeMode.m_hWnd);

	ctrlFiletype.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		WS_HSCROLL | WS_VSCROLL | CBS_DROPDOWNLIST, WS_EX_CLIENTEDGE);
	fileTypeContainer.SubclassWindow(ctrlFiletype.m_hWnd);

	ctrlResults.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS, WS_EX_CLIENTEDGE, IDC_RESULTS);
	resultsContainer.SubclassWindow(ctrlResults.m_hWnd);
	
	if(BOOLSETTING(FULL_ROW_SELECT)) {
		ctrlResults.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP);
	} else {
		ctrlResults.SetExtendedListViewStyle(LVS_EX_HEADERDRAGDROP);
	}
	
	ctrlResults.SetImageList(WinUtil::fileImages, LVSIL_SMALL);

	searchLabel.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	searchLabel.SetFont(WinUtil::systemFont, FALSE);
	searchLabel.SetWindowText(CSTRING(SEARCH_FOR));

	sizeLabel.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	sizeLabel.SetFont(WinUtil::systemFont, FALSE);
	sizeLabel.SetWindowText(CSTRING(SIZE));

	typeLabel.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	typeLabel.SetFont(WinUtil::systemFont, FALSE);
	typeLabel.SetWindowText(CSTRING(FILE_TYPE));

	optionLabel.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	optionLabel.SetFont(WinUtil::systemFont, FALSE);
	optionLabel.SetWindowText(CSTRING(SEARCH_OPTIONS));

	ctrlSlots.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, NULL, IDC_FREESLOTS);
	ctrlSlots.SetButtonStyle(BS_AUTOCHECKBOX, FALSE);
	ctrlSlots.SetFont(WinUtil::systemFont, FALSE);
	ctrlSlots.SetWindowText(CSTRING(ONLY_FREE_SLOTS));
	slotsContainer.SubclassWindow(ctrlSlots.m_hWnd);

	ctrlShowUI.Create(ctrlStatus.m_hWnd, rcDefault, "+/-", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	ctrlShowUI.SetButtonStyle(BS_AUTOCHECKBOX, false);
	ctrlShowUI.SetCheck(1);
	showUIContainer.SubclassWindow(ctrlShowUI.m_hWnd);

	ctrlDoSearch.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
		BS_PUSHBUTTON , 0, IDC_SEARCH);
	ctrlDoSearch.SetWindowText(CSTRING(SEARCH));
	ctrlDoSearch.SetFont(WinUtil::systemFont);
	doSearchContainer.SubclassWindow(ctrlDoSearch.m_hWnd);

	ctrlSearchBox.SetFont(WinUtil::systemFont, FALSE);
	ctrlSize.SetFont(WinUtil::systemFont, FALSE);
	ctrlMode.SetFont(WinUtil::systemFont, FALSE);
	ctrlSizeMode.SetFont(WinUtil::systemFont, FALSE);
	ctrlFiletype.SetFont(WinUtil::systemFont, FALSE);

	ctrlMode.AddString(CSTRING(NORMAL));
	ctrlMode.AddString(CSTRING(AT_LEAST));
	ctrlMode.AddString(CSTRING(AT_MOST));
	ctrlMode.SetCurSel(1);
	
	ctrlSizeMode.AddString(CSTRING(B));
	ctrlSizeMode.AddString(CSTRING(KB));
	ctrlSizeMode.AddString(CSTRING(MB));
	ctrlSizeMode.AddString(CSTRING(GB));
	if(initialSize == 0)
		ctrlSizeMode.SetCurSel(2);
	else
		ctrlSizeMode.SetCurSel(0);

	ctrlFiletype.AddString(CSTRING(ANY));
	ctrlFiletype.AddString(CSTRING(AUDIO));
	ctrlFiletype.AddString(CSTRING(COMPRESSED));
	ctrlFiletype.AddString(CSTRING(DOCUMENT));
	ctrlFiletype.AddString(CSTRING(EXECUTABLE));
	ctrlFiletype.AddString(CSTRING(PICTURE));
	ctrlFiletype.AddString(CSTRING(VIDEO));
	ctrlFiletype.AddString(CSTRING(DIRECTORY));
	ctrlFiletype.SetCurSel(0);
	
	// Create listview columns
	WinUtil::splitTokens(columnIndexes, SETTING(SEARCHFRAME_ORDER), COLUMN_LAST);
	WinUtil::splitTokens(columnSizes, SETTING(SEARCHFRAME_WIDTHS), COLUMN_LAST);

	for(int j=0; j<COLUMN_LAST; j++) {
		int fmt = (j == COLUMN_SIZE || j == COLUMN_EXACT_SIZE) ? LVCFMT_RIGHT : LVCFMT_LEFT;
		ctrlResults.InsertColumn(j, CSTRING_I(columnNames[j]), fmt, columnSizes[j], j);
	}

	ctrlResults.SetColumnOrderArray(COLUMN_LAST, columnIndexes);

	ctrlResults.SetBkColor(WinUtil::bgColor);
	ctrlResults.SetTextBkColor(WinUtil::bgColor);
	ctrlResults.SetTextColor(WinUtil::textColor);
	ctrlResults.SetFont(WinUtil::systemFont, FALSE);	// use Util::font instead to obey Appearace settings
	
	SetWindowText(CSTRING(SEARCH));

	targetDirMenu.CreatePopupMenu();
	targetMenu.CreatePopupMenu();
	resultsMenu.CreatePopupMenu();
	opMenu.CreatePopupMenu();
	
	resultsMenu.AppendMenu(MF_STRING, IDC_DOWNLOAD, CSTRING(DOWNLOAD));
	resultsMenu.AppendMenu(MF_POPUP, (UINT)(HMENU)targetMenu, CSTRING(DOWNLOAD_TO));
	resultsMenu.AppendMenu(MF_STRING, IDC_DOWNLOADDIR, CSTRING(DOWNLOAD_WHOLE_DIR));
	resultsMenu.AppendMenu(MF_POPUP, (UINT)(HMENU)targetDirMenu, CSTRING(DOWNLOAD_WHOLE_DIR_TO));
	resultsMenu.AppendMenu(MF_STRING, IDC_VIEW_AS_TEXT, CSTRING(VIEW_AS_TEXT));
	resultsMenu.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)NULL);
	resultsMenu.AppendMenu(MF_STRING, IDC_GETLIST, CSTRING(GET_FILE_LIST));
	resultsMenu.AppendMenu(MF_STRING, IDC_PRIVATEMESSAGE, CSTRING(SEND_PRIVATE_MESSAGE));
	resultsMenu.AppendMenu(MF_STRING, IDC_ADD_TO_FAVORITES, CSTRING(ADD_TO_FAVORITES));
	resultsMenu.AppendMenu(MF_STRING, IDC_REMOVE, CSTRING(REMOVE));

	opMenu.AppendMenu(MF_STRING, IDC_DOWNLOAD, CSTRING(DOWNLOAD));
	opMenu.AppendMenu(MF_POPUP, (UINT)(HMENU)targetMenu, CSTRING(DOWNLOAD_TO));
	opMenu.AppendMenu(MF_STRING, IDC_DOWNLOADDIR, CSTRING(DOWNLOAD_WHOLE_DIR));
	opMenu.AppendMenu(MF_POPUP, (UINT)(HMENU)targetDirMenu, CSTRING(DOWNLOAD_WHOLE_DIR_TO));
	opMenu.AppendMenu(MF_STRING, IDC_VIEW_AS_TEXT, CSTRING(VIEW_AS_TEXT));
	opMenu.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)NULL);
	opMenu.AppendMenu(MF_STRING, IDC_GETLIST, CSTRING(GET_FILE_LIST));
	opMenu.AppendMenu(MF_STRING, IDC_PRIVATEMESSAGE, CSTRING(SEND_PRIVATE_MESSAGE));
	opMenu.AppendMenu(MF_STRING, IDC_ADD_TO_FAVORITES, CSTRING(ADD_TO_FAVORITES));
	opMenu.AppendMenu(MF_STRING, IDC_REMOVE, CSTRING(REMOVE));
	opMenu.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)NULL);
	opMenu.AppendMenu(MF_STRING, IDC_KICK, CSTRING(KICK_USER));
	opMenu.AppendMenu(MF_STRING, IDC_REDIRECT, CSTRING(REDIRECT));

	if(!initialString.empty()) {
		search = StringTokenizer(initialString, ' ').getTokens();
		lastSearches.push_back(initialString);
		ctrlSearchBox.InsertString(0, initialString.c_str());
		ctrlSearchBox.SetCurSel(0);
		ctrlMode.SetCurSel(initialMode);
		ctrlSize.SetWindowText(Util::toString(initialSize).c_str());
		ctrlFiletype.SetCurSel(initialType);
		SearchManager::getInstance()->search(initialString, initialSize, initialType, initialMode);
		ctrlStatus.SetText(1, (STRING(SEARCHING_FOR) + initialString + "...").c_str());
	}

	bHandled = FALSE;
	return 1;
}

LRESULT SearchFrame::onDownloadTo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(ctrlResults.GetSelectedCount() == 1) {
		int i = ctrlResults.GetNextItem(-1, LVNI_SELECTED);
		dcassert(i != -1);
		SearchResult* sr = (SearchResult*)ctrlResults.GetItemData(i);

		if(sr->getType() == SearchResult::TYPE_FILE) {
			string target = SETTING(DOWNLOAD_DIRECTORY) + sr->getFileName();
			if(WinUtil::browseFile(target, m_hWnd)) {
				WinUtil::addLastDir(Util::getFilePath(target));
				try {
					QueueManager::getInstance()->add(sr->getFile(), sr->getSize(), sr->getUser(), target);
				} catch(const Exception& e) {
					ctrlStatus.SetText(1, e.getError().c_str());
				}
			}
		} else {
			string target = SETTING(DOWNLOAD_DIRECTORY);
			if(WinUtil::browseDirectory(target, m_hWnd)) {
				WinUtil::addLastDir(target);
		
				downloadSelected(target);
			}
		}
	} else {
		string target = SETTING(DOWNLOAD_DIRECTORY);
		if(WinUtil::browseDirectory(target, m_hWnd)) {
			WinUtil::addLastDir(target);
			downloadSelected(target);
		}
	}
	return 0;
}

LRESULT SearchFrame::onDownloadWholeTo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	string target = SETTING(DOWNLOAD_DIRECTORY);
	if(WinUtil::browseDirectory(target, m_hWnd)) {
		WinUtil::addLastDir(target);
	
		downloadWholeSelected(target);
	}
	return 0;
}

void SearchFrame::onEnter() {
	char* message;
	
	if(ctrlSearch.GetWindowTextLength() > 0 && lastSearch + 3*1000 < TimerManager::getInstance()->getTick()) {
		message = new char[ctrlSearch.GetWindowTextLength()+1];
		ctrlSearch.GetWindowText(message, ctrlSearch.GetWindowTextLength()+1);
		string s(message, ctrlSearch.GetWindowTextLength());
		delete[] message;
		
		message = new char[ctrlSize.GetWindowTextLength()+1];
		ctrlSize.GetWindowText(message, ctrlSize.GetWindowTextLength()+1);
		string size(message, ctrlSize.GetWindowTextLength());
		delete[] message;
		
		double lsize = Util::toDouble(size);
		switch(ctrlSizeMode.GetCurSel()) {
		case 1:
			lsize*=1024.0; break;
		case 2:
			lsize*=1024.0*1024.0; break;
		case 3:
			lsize*=1024.0*1024.0*1024.0; break;
		}

		int64_t llsize = (int64_t)lsize;

		for(int i = 0; i != ctrlResults.GetItemCount(); i++) {
			delete (SearchResult*)ctrlResults.GetItemData(i);
		}
		ctrlResults.DeleteAllItems();
		
		SearchManager::SizeModes mode((SearchManager::SizeModes)ctrlMode.GetCurSel());
		if(llsize == 0)
			mode = SearchManager::SIZE_DONTCARE;

		SearchManager::getInstance()->search(s, llsize, 
			(SearchManager::TypeModes)ctrlFiletype.GetCurSel(), mode);

		if(BOOLSETTING(CLEAR_SEARCH)){
			ctrlSearch.SetWindowText("");
		} else {
			lastSearch = TimerManager::getInstance()->getTick();
		}

		// Add new searches to the last-search dropdown list
		if(find(lastSearches.begin(), lastSearches.end(), s) == lastSearches.end()) 
		{
			if(ctrlSearchBox.GetCount() > 9)
				ctrlSearchBox.DeleteString(9);
			ctrlSearchBox.InsertString(0, s.c_str());

			while(lastSearches.size() > 9) {
				lastSearches.erase(lastSearches.begin());
			}
			lastSearches.push_back(s);
		}
		
		ctrlStatus.SetText(1, (STRING(SEARCHING_FOR) + s + "...").c_str());
		{
			Lock l(cs);
			search = StringTokenizer(s, ' ').getTokens();
		}

		SetWindowText((STRING(SEARCH) + " - " + s).c_str());
	}
}

void SearchFrame::onSearchResult(SearchResult* aResult) {
	// Check that this is really a relevant search result...
	{
		Lock l(cs);

		if(search.empty()) {
			return;
		}
		for(StringIter j = search.begin(); j != search.end(); ++j) {
			if(Util::findSubString(aResult->getFile(), *j) == -1) {
				return;
			}
		}
	}

	// Reject results without free slots if selected
	if(onlyFree && aResult->getFreeSlots() < 1)
		return;

	SearchResult* copy = new SearchResult(*aResult);
	
	string file, path;
	if(aResult->getType() == SearchResult::TYPE_FILE) {
		if(aResult->getFile().rfind('\\') == string::npos) {
			file = aResult->getFile();
		} else {
			file = Util::getFileName(aResult->getFile());
			path = Util::getFilePath(aResult->getFile());
		}
	} else {
		file = aResult->getFileName();
		path = aResult->getFile();
	}
	
	StringList* l = new StringList();
	l->push_back(aResult->getUser()->getNick());
	l->push_back(file);
	if(aResult->getType() == SearchResult::TYPE_FILE) {
		int i = file.rfind('.');
		if(i != string::npos) {
			l->push_back(file.substr(i + 1));
		} else {
			l->push_back(Util::emptyString);
		}
		l->push_back(Util::formatBytes(aResult->getSize()));
	} else {
		l->push_back(STRING(DIRECTORY));
		l->push_back(Util::emptyString);
	}

	l->push_back(path);
	l->push_back(aResult->getSlotString());
	if(aResult->getUser()) {
		l->push_back(aResult->getUser()->getConnection());
	} else {
		l->push_back(Util::emptyString);
	}
	l->push_back(aResult->getHubName());
	l->push_back((aResult->getType() == SearchResult::TYPE_FILE) ? Util::formatNumber(aResult->getSize()) : Util::emptyString);
	PostMessage(WM_SPEAKER, (WPARAM)l, (LPARAM)copy);	
}

LRESULT SearchFrame::onKick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) { 
	LineDlg dlg;
	dlg.title = STRING(KICK_USER);
	dlg.description = STRING(ENTER_REASON);
	dlg.line = WinUtil::lastKick;
	if(dlg.DoModal() == IDOK) {
		WinUtil::lastKick = dlg.line;
		int i = -1;
		User::List kicked;
		kicked.reserve(ctrlResults.GetSelectedCount());
		while( (i = ctrlResults.GetNextItem(i, LVNI_SELECTED)) != -1) {
			SearchResult* sr = (SearchResult*)ctrlResults.GetItemData(i);
			if(find(kicked.begin(), kicked.end(), sr->getUser()) != kicked.end()) {
				continue;
			}
			kicked.push_back(sr->getUser());
			if(sr->getUser() && sr->getUser()->isOnline()) {
				sr->getUser()->kick(dlg.line);
			}
		}
	}
	
	return 0; 
};

LRESULT SearchFrame::onRedirect(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) { 
	LineDlg dlg1, dlg2;
	dlg1.title = STRING(REDIRECT_USER);
	dlg1.description = STRING(ENTER_REASON);
	dlg1.line = WinUtil::lastRedirect;
	if(dlg1.DoModal() == IDOK) {
		dlg2.title = STRING(REDIRECT_USER);
		dlg2.description = STRING(ENTER_SERVER);
		dlg2.line = WinUtil::lastServer;
		if(dlg2.DoModal() == IDOK) {
			WinUtil::lastRedirect = dlg1.line;
			WinUtil::lastServer = dlg2.line;
			
			int i = -1;
			User::List kicked;
			kicked.reserve(ctrlResults.GetSelectedCount());
			while( (i = ctrlResults.GetNextItem(i, LVNI_SELECTED)) != -1) {
				SearchResult* sr = (SearchResult*)ctrlResults.GetItemData(i);
				if(find(kicked.begin(), kicked.end(), sr->getUser()) != kicked.end()) {
					continue;
				}
				kicked.push_back(sr->getUser());
				if(sr->getUser() && sr->getUser()->isOnline()) {
					sr->getUser()->redirect(dlg2.line, STRING(YOU_ARE_BEING_REDIRECTED) + dlg2.line + ": " + dlg1.line);
				}
			}
		}
	}
	
	return 0; 
};

LRESULT SearchFrame::onGetList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int i=-1;
	while( (i = ctrlResults.GetNextItem(i, LVNI_SELECTED)) != -1) {
		SearchResult* sr = (SearchResult*)ctrlResults.GetItemData(i);
		try {
			QueueManager::getInstance()->addList(sr->getUser());
		} catch(...) {
			// ...
		}
	}
	return 0;
}

LRESULT SearchFrame::onDoubleClickResults(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NMITEMACTIVATE* item = (NMITEMACTIVATE*)pnmh;
	
	if(item->iItem != -1) {
		SearchResult* sr = (SearchResult*)ctrlResults.GetItemData(item->iItem);
		try {
			if(sr->getType() == SearchResult::TYPE_FILE) {
				QueueManager::getInstance()->add(sr->getFile(), sr->getSize(), sr->getUser(), SETTING(DOWNLOAD_DIRECTORY) + sr->getFileName());
			} else {
				dcassert(sr->getType() == SearchResult::TYPE_DIRECTORY);
				QueueManager::getInstance()->addDirectory(sr->getFile(), sr->getUser(), SETTING(DOWNLOAD_DIRECTORY) + Util::getLastDir(sr->getFile()));
			}
		} catch(const Exception& e) {
			ctrlStatus.SetText(1, e.getError().c_str());
		}
	}
	return 0;
}

void SearchFrame::downloadSelected(const string& aDir, bool view /* = false */) {
	int i=-1;
	
	while( (i = ctrlResults.GetNextItem(i, LVNI_SELECTED)) != -1) {
		SearchResult* sr = (SearchResult*)ctrlResults.GetItemData(i);
		try { 
			if(sr->getType() == SearchResult::TYPE_FILE) {
				QueueManager::getInstance()->add(sr->getFile(), sr->getSize(), sr->getUser(), aDir + sr->getFileName(), 
					(view ? (QueueItem::FLAG_CLIENT_VIEW | QueueItem::FLAG_TEXT) : QueueItem::FLAG_RESUME));
			} else if(!view) {
				dcassert(sr->getType() == SearchResult::TYPE_DIRECTORY);
				QueueManager::getInstance()->addDirectory(sr->getFile(), sr->getUser(), aDir);
			}
		} catch(const Exception& e) {
			ctrlStatus.SetText(1, e.getError().c_str());
		}
	}
}

void SearchFrame::downloadWholeSelected(const string& aDir) {
	int i=-1;
	
	while( (i = ctrlResults.GetNextItem(i, LVNI_SELECTED)) != -1) {
		SearchResult* sr = (SearchResult*)ctrlResults.GetItemData(i);
		try {
			if(sr->getType() == SearchResult::TYPE_FILE) {
				QueueManager::getInstance()->addDirectory(Util::getFilePath(sr->getFile()), sr->getUser(), aDir);
			} else {
				QueueManager::getInstance()->addDirectory(sr->getFile(), sr->getUser(), aDir);
			}
		} catch(const Exception& e) {
			ctrlStatus.SetText(1, e.getError().c_str());
		}
	}
}

LRESULT SearchFrame::onPrivateMessage(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int i=-1;
	while( (i = ctrlResults.GetNextItem(i, LVNI_SELECTED)) != -1) {
		SearchResult* sr = (SearchResult*)ctrlResults.GetItemData(i);
		PrivateFrame::openWindow(sr->getUser(), m_hWndMDIClient, getTab());
	}
	return 0;
}

LRESULT SearchFrame::onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	if(!closed) {
		SearchManager::getInstance()->removeListener(this);

		closed = true;
		PostMessage(WM_CLOSE);
		return 0;
	} else {
		for(int i = 0; i < ctrlResults.GetItemCount(); i++) {
			delete (SearchResult*)ctrlResults.GetItemData(i);
		}

		WinUtil::saveHeaderOrder(ctrlResults, SettingsManager::SEARCHFRAME_ORDER,
			SettingsManager::SEARCHFRAME_WIDTHS, COLUMN_LAST, columnIndexes, columnSizes);

		MDIDestroy(m_hWnd);
		return 0;
	}
}

void SearchFrame::UpdateLayout(BOOL bResizeBars)
{
	RECT rect;
	GetClientRect(&rect);
	// position bars and offset their dimensions
	UpdateBarsPosition(rect, bResizeBars);
	
	if(ctrlStatus.IsWindow()) {
		CRect sr;
		int w[4];
		ctrlStatus.GetClientRect(sr);
		int tmp = (sr.Width()) > 316 ? 216 : ((sr.Width() > 116) ? sr.Width()-100 : 16);
		
		w[0] = 15;
		w[1] = sr.right - tmp;
		w[2] = w[1] + (tmp-16)/2;
		w[3] = w[1] + (tmp-16);
		
		ctrlStatus.SetParts(4, w);

		// Layout showUI button in statusbar part #0
		ctrlStatus.GetRect(0, sr);
		ctrlShowUI.MoveWindow(sr);
	}

	if(showUI)
	{
		int const width = 220, spacing = 50, labelH = 16, comboH = 140, lMargin = 2, rMargin = 4;
		CRect rc = rect;

		rc.left += width;
		ctrlResults.MoveWindow(rc);

		// "Search for"
		rc.right = width - rMargin;
		rc.left = lMargin;
		rc.top += 25;
		rc.bottom = rc.top + comboH + 21;
		ctrlSearchBox.MoveWindow(rc);

		searchLabel.MoveWindow(rc.left + lMargin, rc.top - labelH, width - rMargin, labelH-1);

		// "Size"
		int w2 = width - rMargin - lMargin;
		rc.top += spacing;
		rc.bottom += spacing;
		rc.right = w2/3;
		ctrlMode.MoveWindow(rc);

		sizeLabel.MoveWindow(rc.left + lMargin, rc.top - labelH, width - rMargin, labelH-1);

		rc.left = rc.right + lMargin;
		rc.right += w2/3;
		rc.bottom -= comboH;
		ctrlSize.MoveWindow(rc);

		rc.left = rc.right + lMargin;
		rc.right = width - rMargin;
		rc.bottom += comboH;
		ctrlSizeMode.MoveWindow(rc);
		rc.bottom -= comboH;

		// "File type"
		rc.left = lMargin;
		rc.right = width - rMargin;
		rc.top += spacing;
		rc.bottom = rc.top + comboH + 21;
		ctrlFiletype.MoveWindow(rc);
		rc.bottom -= comboH;

		typeLabel.MoveWindow(rc.left + lMargin, rc.top - labelH, width - rMargin, labelH-1);

		// "Search options"
		rc.left = lMargin;
		rc.right = width - rMargin;
		rc.top += spacing;
		rc.bottom += spacing;
		ctrlSlots.MoveWindow(rc);

		optionLabel.MoveWindow(rc.left + lMargin, rc.top - labelH, width - rMargin, labelH-1);

		// "Search"
		rc.right = width - rMargin;
		rc.left = rc.right - 100;
		rc.top += spacing/2;
		rc.bottom += spacing/2;
		ctrlDoSearch.MoveWindow(rc);
	}
	else
	{
		CRect rc = rect;
		ctrlResults.MoveWindow(rc);

		rc.SetRect(0,0,0,0);
		ctrlSearchBox.MoveWindow(rc);
		ctrlMode.MoveWindow(rc);
		ctrlSize.MoveWindow(rc);
		ctrlSizeMode.MoveWindow(rc);
		ctrlFiletype.MoveWindow(rc);
	}

	POINT pt;
	pt.x = 10; 
	pt.y = 10;
	HWND hWnd = ctrlSearchBox.ChildWindowFromPoint(pt);
	if(hWnd != NULL && !ctrlSearch.IsWindow() && hWnd != ctrlSearchBox.m_hWnd) {
		ctrlSearch.Attach(hWnd); 
		searchContainer.SubclassWindow(ctrlSearch.m_hWnd);
	}	
}

LRESULT SearchFrame::onUserCommand(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	dcassert(wID >= IDC_USER_COMMAND);
	size_t n = (size_t)wID - IDC_USER_COMMAND;

	UserCommand::List& ul = HubManager::getInstance()->getUserCommands();
	dcassert(n < ul.size());

	UserCommand& uc = ul[n];

	if(!WinUtil::getUCParams(m_hWnd, uc, ucParams))
		return 0;

	int sel = -1;
	while((sel = ctrlResults.GetNextItem(sel, LVNI_SELECTED)) != -1) {
		SearchResult* sr = (SearchResult*) ctrlResults.GetItemData(sel);
		ucParams["nick"] = sr->getUser()->getNick();
		ucParams["mynick"] = sr->getUser()->getClientNick();
		ucParams["file"] = sr->getFile();
		if(uc.getNick().empty()) {
			sr->getUser()->clientMessage(Util::formatParams(uc.getCommand(), ucParams));
		} else {
			sr->getUser()->clientPM(Util::formatParams(uc.getNick(), ucParams), Util::formatParams(uc.getCommand(), ucParams));
		}
	}
	return 0;
};

LRESULT SearchFrame::onAddToFavorites(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int i=-1;
	while( (i = ctrlResults.GetNextItem(i, LVNI_SELECTED)) != -1) {
		HubManager::getInstance()->addFavoriteUser(((SearchResult*)ctrlResults.GetItemData(i))->getUser());
	}
	return 0;
}

LRESULT SearchFrame::onCtlColor(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	HWND hWnd = (HWND)lParam;
	HDC hDC = (HDC)wParam;

	if(hWnd == searchLabel.m_hWnd || hWnd == sizeLabel.m_hWnd || hWnd == optionLabel.m_hWnd || hWnd == typeLabel.m_hWnd
		|| hWnd == ctrlSlots.m_hWnd) {
		::SetBkColor(hDC, ::GetSysColor(COLOR_3DFACE));
		::SetTextColor(hDC, ::GetSysColor(COLOR_BTNTEXT));
		return (LRESULT)::GetSysColorBrush(COLOR_3DFACE);
	} else {
		::SetBkColor(hDC, WinUtil::bgColor);
		::SetTextColor(hDC, WinUtil::textColor);
		return (LRESULT)WinUtil::bgBrush;
	}
};

LRESULT SearchFrame::onColumnClickResults(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NMLISTVIEW* l = (NMLISTVIEW*)pnmh;
	if(l->iSubItem == ctrlResults.getSortColumn()) {
		if (!ctrlResults.getSortDirection())
			ctrlResults.setSort(-1, ctrlResults.getSortType());
		else
			ctrlResults.setSortDirection(false);
	} else {
		if( (l->iSubItem == COLUMN_SIZE) || (l->iSubItem == COLUMN_EXACT_SIZE) ) {
			ctrlResults.setSort(l->iSubItem, ExListViewCtrl::SORT_FUNC, true, sortSize);
		} else if(l->iSubItem == COLUMN_SLOTS) {
			ctrlResults.setSort(l->iSubItem, ExListViewCtrl::SORT_FUNC, true, sortSlots);
		} else {
			ctrlResults.setSort(l->iSubItem, ExListViewCtrl::SORT_STRING_NOCASE);
		}
	}
	return 0;
}

LRESULT SearchFrame::onChar(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled) {
	switch(wParam) {
	case VK_TAB:
		if(uMsg == WM_KEYDOWN) {
			onTab((GetKeyState(VK_SHIFT) & 0x8000) > 0);
		}
		break;
	case VK_RETURN:
		if( (GetKeyState(VK_SHIFT) & 0x8000) || 
			(GetKeyState(VK_CONTROL) & 0x8000) || 
			(GetKeyState(VK_MENU) & 0x8000) ) {
			bHandled = FALSE;
		} else {
			if(uMsg == WM_KEYDOWN) {
				onEnter();
			}
		}
		break;
	default:
		bHandled = FALSE;
	}
	return 0;
}

void SearchFrame::onTab(bool shift) {
	HWND wnds[] = {
		ctrlSearch.m_hWnd, ctrlMode.m_hWnd, ctrlSize.m_hWnd, ctrlSizeMode.m_hWnd, 
		ctrlFiletype.m_hWnd, ctrlSlots.m_hWnd, ctrlDoSearch.m_hWnd, ctrlSearch.m_hWnd, 
		ctrlResults.m_hWnd
	};
	
	HWND focus = GetFocus();
	if(focus == ctrlSearchBox.m_hWnd)
		focus = ctrlSearch.m_hWnd;
	
	static const int size = sizeof(wnds) / sizeof(wnds[0]);
	int i;
	for(i = 0; i < size; i++) {
		if(wnds[i] == focus)
			break;
	}

	::SetFocus(wnds[(i + (shift ? -1 : 1)) % size]);
}

LRESULT SearchFrame::onSpeaker(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	SearchResult* sr = (SearchResult*)lParam;

	// Check previous search results for dupes
	for(int i = 0, j = ctrlResults.GetItemCount(); i < j; ++i) {
		SearchResult* sr2 = (SearchResult*)ctrlResults.GetItemData(i);
		if((sr->getUser()->getNick() == sr2->getUser()->getNick()) && (sr->getFile() == sr2->getFile())) {
			delete (StringList*)wParam;
			delete sr;
			return 0;
		}
	}

#if 0
	int image = 0;
	const string& tmp = sr->getUser()->getConnection();
	if( (tmp == SettingsManager::connectionSpeeds[SettingsManager::SPEED_288K]) ||
		(tmp == SettingsManager::connectionSpeeds[SettingsManager::SPEED_576K]) ||
		(tmp == SettingsManager::connectionSpeeds[SettingsManager::SPEED_SATELLITE]) ||
		(tmp == SettingsManager::connectionSpeeds[SettingsManager::SPEED_ISDN]) ) {
		image = 1;
	} else if( (tmp == SettingsManager::connectionSpeeds[SettingsManager::SPEED_CABLE]) ||
		(tmp == SettingsManager::connectionSpeeds[SettingsManager::SPEED_DSL]) ) {
		image = 2;
	} else if( (tmp == SettingsManager::connectionSpeeds[SettingsManager::SPEED_T1]) ||
		(tmp == SettingsManager::connectionSpeeds[SettingsManager::SPEED_T3]) ) {
		image = 3;
	}
#endif
	int image = sr->getType() == SearchResult::TYPE_FILE ? WinUtil::getIconIndex(sr->getFile()) : WinUtil::getDirIconIndex();
	ctrlResults.insert(*(StringList*)wParam, image, lParam);
//	ctrlResults.SetItemState(i, INDEXTOOVERLAYMASK(image), LVIS_OVERLAYMASK)
	delete (StringList*)wParam;
	return 0;
}

LRESULT SearchFrame::onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	RECT rc;                    // client area of window 
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };        // location of mouse click 
	
	// Get the bounding rectangle of the client area. 
	ctrlResults.GetClientRect(&rc);
	ctrlResults.ScreenToClient(&pt); 
	int64_t cmpSize;

	bool op = true;
	string oneHub;
	
	if (PtInRect(&rc, pt) && ctrlResults.GetSelectedCount() > 0) {
		ctrlResults.ClientToScreen(&pt);

		while(targetMenu.GetMenuItemCount() > 0) {
			targetMenu.DeleteMenu(0, MF_BYPOSITION);
		}
		while(targetDirMenu.GetMenuItemCount() > 0) {
			targetDirMenu.DeleteMenu(0, MF_BYPOSITION);
		}

		int n = 0;

		targetDirMenu.AppendMenu(MF_STRING, IDC_DOWNLOADTO, CSTRING(BROWSE));
		if(WinUtil::lastDirs.size() > 0) {
			targetDirMenu.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)NULL);
			for(StringIter i = WinUtil::lastDirs.begin(); i != WinUtil::lastDirs.end(); ++i) {
				targetDirMenu.AppendMenu(MF_STRING, IDC_DOWNLOAD_WHOLE_TARGET + n, i->c_str());
				n++;
			}
			
			n = 0;
		}

		if(ctrlResults.GetSelectedCount() == 1) {
			targetMenu.AppendMenu(MF_STRING, IDC_DOWNLOADTO, CSTRING(BROWSE));
			
			int pos = ctrlResults.GetNextItem(-1, LVNI_SELECTED);
			dcassert(pos != -1);
			SearchResult* sr = (SearchResult*)ctrlResults.GetItemData(pos);

			if(sr->getType() == SearchResult::TYPE_FILE) {
				targets = QueueManager::getInstance()->getTargetsBySize(sr->getSize(), Util::getFileExt(sr->getFile()));
				if(targets.size() > 0) {
					targetMenu.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)NULL);
					for(StringIter i = targets.begin(); i != targets.end(); ++i) {
						targetMenu.AppendMenu(MF_STRING, IDC_DOWNLOAD_TARGET + n, i->c_str());
						n++;
					}
				}
			} else {
				// Keep on counting on recent dirs
				if(WinUtil::lastDirs.size() > 0) {
					targetMenu.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)NULL);
					
					for(StringIter i = WinUtil::lastDirs.begin(); i != WinUtil::lastDirs.end(); ++i) {
						targetMenu.AppendMenu(MF_STRING, IDC_DOWNLOAD_TARGET + n, i->c_str());
						n++;
					}
				}
			}
			
			op = sr->getUser()->isClientOp();
			oneHub = sr->getUser()->getClientServer();

		} else if(ctrlResults.GetSelectedCount() > 1) {

			targetMenu.AppendMenu(MF_STRING, IDC_DOWNLOADTO, CSTRING(BROWSE));

			// Keep on counting on recent dirs
			if(WinUtil::lastDirs.size() > 0) {
				targetMenu.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)NULL);
				
				for(StringIter i = WinUtil::lastDirs.begin(); i != WinUtil::lastDirs.end(); ++i) {
					targetMenu.AppendMenu(MF_STRING, IDC_DOWNLOAD_TARGET + n, i->c_str());
					n++;
				}
			}
			
			cmpSize = 0;
			string ext;
			int pos = -1;
			while( (pos = ctrlResults.GetNextItem(pos, LVNI_SELECTED)) != -1) {
				SearchResult* sr = (SearchResult*)ctrlResults.GetItemData(pos);
				if(sr->getType() == SearchResult::TYPE_DIRECTORY) {
					cmpSize = 0;
					break;
				}
				if (cmpSize == 0) {
					if(ext.empty()) {
						ext = Util::getFileExt(sr->getFile());
					}
					if(Util::stricmp(ext, Util::getFileExt(sr->getFile())) == 0) {
						cmpSize = sr->getSize();
					} else {
						cmpSize = 0;
						break;
					}
				} else if(cmpSize != sr->getSize() || Util::stricmp(ext, Util::getFileExt(sr->getFile())) != 0) {
					cmpSize = 0;
					break;
				}
			}
			
			if(cmpSize != 0) {
				// All selected files are same size, see if we have a match in queue
				targets = QueueManager::getInstance()->getTargetsBySize(cmpSize, ext);
				if(targets.size() > 0) {
					targetMenu.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)NULL);
					for(StringIter i = targets.begin(); i != targets.end(); ++i) {
						targetMenu.AppendMenu(MF_STRING, IDC_DOWNLOAD_TARGET + n, i->c_str());
						n++;
					}
				}
			}

			pos = -1;
			bool one = true;

			while( (pos = ctrlResults.GetNextItem(pos, LVNI_SELECTED)) != -1) {
				SearchResult* sr = (SearchResult*) ctrlResults.GetItemData(pos);
				if(!sr->getUser()->isClientOp()) {
					op = false;
					break;
				}
				if(one) {
					if(oneHub.empty()) {
						oneHub = sr->getUser()->getClientServer();
					} else {
						if(Util::stricmp(sr->getUser()->getClientServer(), oneHub) != 0) {
							oneHub.clear();
							one = false;
						}
					}
				}
			}
		}

		if(op) {
			// Alrite, now add the special menu items...
			int added = 0;
			int n = 0;
			UserCommand::List& ul = HubManager::getInstance()->getUserCommands();
			for(UserCommand::Iter ui = ul.begin(); ui != ul.end(); ++ui) {
				UserCommand& uc = *ui;
				if(uc.getHub().empty() || uc.getHub() == "op" || 
					Util::stricmp(uc.getHub(), oneHub) == 0) {
						// We add!
						if(added == 0) {
							opMenu.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)0);
							added++;
						}
						opMenu.AppendMenu(MF_STRING, IDC_USER_COMMAND+n, uc.getName().c_str());
						added++;
					}
					n++;
			}
			commands = ul.size();
			opMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
			while(added > 0) {
				opMenu.DeleteMenu(opMenu.GetMenuItemCount()-1, MF_BYPOSITION);
				added--;
			}
		} else {
			int added = 0;
			int n = 0;
			UserCommand::List& ul = HubManager::getInstance()->getUserCommands();
			for(UserCommand::Iter ui = ul.begin(); ui != ul.end(); ++ui) {
				UserCommand& uc = *ui;
				if(uc.getHub().empty() || 
					Util::stricmp(uc.getHub(), oneHub) == 0) {
						// We add!
						if(added == 0) {
							resultsMenu.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)0);
							added++;
						}
						resultsMenu.AppendMenu(MF_STRING, IDC_USER_COMMAND+n, uc.getName().c_str());
						added++;
					}
					n++;
			}
			commands = ul.size();
			resultsMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
			while(added > 0) {
				resultsMenu.DeleteMenu(resultsMenu.GetMenuItemCount()-1, MF_BYPOSITION);
				added--;
			}
		}
		return TRUE; 
	}
	return FALSE; 
}

LRESULT SearchFrame::onDownloadTarget(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int newId = wID - IDC_DOWNLOAD_TARGET;
	dcassert(newId >= 0);
	
	if(ctrlResults.GetSelectedCount() == 1) {
		int i = ctrlResults.GetNextItem(-1, LVNI_SELECTED);
		dcassert(i != -1);
		SearchResult* sr = (SearchResult*)ctrlResults.GetItemData(i);
		try {
			if(sr->getType() == SearchResult::TYPE_FILE) {
				dcassert(newId < (int)targets.size());
				QueueManager::getInstance()->add(sr->getFile(), sr->getSize(), sr->getUser(), targets[newId]);
			} else {
				dcassert(sr->getType() == SearchResult::TYPE_DIRECTORY);
				dcassert(newId < (int)WinUtil::lastDirs.size());
				QueueManager::getInstance()->addDirectory(sr->getFile(), sr->getUser(), WinUtil::lastDirs[newId]);
			}
		} catch(const Exception& e) {
			ctrlStatus.SetText(1, e.getError().c_str());
		}
	} else if(ctrlResults.GetSelectedCount() > 1) {
		if(newId < (int)WinUtil::lastDirs.size()) {
			//Original Handler
			downloadSelected(WinUtil::lastDirs[newId]);
		} else {
			newId -= WinUtil::lastDirs.size();
			//Add all files as alternates to selected file
			int pos = -1;
			while( (pos = ctrlResults.GetNextItem(pos, LVNI_SELECTED)) != -1) {
     			SearchResult* sr = (SearchResult*)ctrlResults.GetItemData(pos);
				dcassert(sr->getType() == SearchResult::TYPE_FILE);
				try {				
					dcassert((newId < (int)targets.size()) && (newId >= 0));
					QueueManager::getInstance()->add(sr->getFile(), sr->getSize(), sr->getUser(), targets[newId]);
				} catch(const Exception& e) {
					ctrlStatus.SetText(1, e.getError().c_str());
				}
			}
		} 
	}
	return 0;
}

LRESULT SearchFrame::onDownloadWholeTarget(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	dcassert((wID-IDC_DOWNLOAD_WHOLE_TARGET) < (int)WinUtil::lastDirs.size());
	downloadWholeSelected(WinUtil::lastDirs[wID-IDC_DOWNLOAD_WHOLE_TARGET]);
	return 0;
}

/**
 * @file
 * $Id: SearchFrm.cpp,v 1.24 2003/10/07 00:35:08 arnetheduck Exp $
 */

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

#include "SearchFrm.h"
#include "LineDlg.h"
#include "PrivateFrame.h"

#include "../client/QueueManager.h"
#include "../client/StringTokenizer.h"

StringList SearchFrame::lastSearches;

int SearchFrame::columnIndexes[] = { COLUMN_NICK, COLUMN_FILENAME, COLUMN_TYPE, COLUMN_SIZE,
	COLUMN_PATH, COLUMN_SLOTS, COLUMN_CONNECTION, COLUMN_HUB };

int SearchFrame::columnSizes[] = { 100, 200, 50, 80, 100, 40, 70, 150 };

static ResourceManager::Strings columnNames[] = { ResourceManager::USER, ResourceManager::FILE, ResourceManager::TYPE, ResourceManager::SIZE, 
	ResourceManager::PATH, ResourceManager::SLOTS, ResourceManager::CONNECTION, ResourceManager::HUB };


LRESULT SearchFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	HFONT const uiFont = (HFONT)::GetStockObject(DEFAULT_GUI_FONT);

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
		WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS, WS_EX_CLIENTEDGE, IDC_RESULTS);

	if(BOOLSETTING(FULL_ROW_SELECT)) {
		ctrlResults.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP);
	}
	else {
		ctrlResults.SetExtendedListViewStyle(LVS_EX_HEADERDRAGDROP);
	}

	images.CreateFromImage(IDB_SPEEDS, 16, 3, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION | LR_SHARED);
	
	ctrlResults.SetImageList(images, LVSIL_SMALL);

	searchLabel.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	searchLabel.SetFont(uiFont, FALSE);
	searchLabel.SetWindowText(CSTRING(SEARCH_FOR));

	sizeLabel.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	sizeLabel.SetFont(uiFont, FALSE);
	sizeLabel.SetWindowText(CSTRING(SIZE));

	typeLabel.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	typeLabel.SetFont(uiFont, FALSE);
	typeLabel.SetWindowText(CSTRING(FILE_TYPE));

	optionLabel.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	optionLabel.SetFont(uiFont, FALSE);
	optionLabel.SetWindowText(CSTRING(SEARCH_OPTIONS));

	ctrlSlots.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	ctrlSlots.SetButtonStyle(BS_AUTOCHECKBOX, FALSE);
	ctrlSlots.SetFont(uiFont, FALSE);
	ctrlSlots.SetWindowText(CSTRING(ONLY_FREE_SLOTS));
	slotsContainer.SubclassWindow(ctrlSlots.m_hWnd);

	ctrlShowUI.Create(ctrlStatus, rcDefault, "+/-", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	ctrlShowUI.SetButtonStyle(BS_AUTOCHECKBOX, false);
	ctrlShowUI.SetFont(ctrlStatus.GetFont());
	ctrlShowUI.SetCheck(1);
	showUIContainer.SubclassWindow(ctrlShowUI.m_hWnd);

	ctrlDoSearch.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
		BS_PUSHBUTTON , 0, IDC_SEARCH);
	ctrlDoSearch.SetWindowText(CSTRING(SEARCH));
	ctrlDoSearch.SetFont(ctrlStatus.GetFont());
	doSearchContainer.SubclassWindow(ctrlDoSearch.m_hWnd);

	ctrlSearchBox.SetFont(uiFont, FALSE);
	ctrlSize.SetFont(uiFont, FALSE);
	ctrlMode.SetFont(uiFont, FALSE);
	ctrlSizeMode.SetFont(uiFont, FALSE);
	ctrlFiletype.SetFont(uiFont, FALSE);

	ctrlMode.AddString(CSTRING(NORMAL));
	ctrlMode.AddString(CSTRING(AT_LEAST));
	ctrlMode.AddString(CSTRING(AT_MOST));
	ctrlMode.SetCurSel(1);
	
	ctrlSizeMode.AddString("B");
	ctrlSizeMode.AddString("kB");
	ctrlSizeMode.AddString("MB");
	ctrlSizeMode.AddString("GB");
	ctrlSizeMode.SetCurSel(2);

	ctrlFiletype.AddString(CSTRING(ANY));
	ctrlFiletype.AddString(CSTRING(AUDIO));
	ctrlFiletype.AddString(CSTRING(COMPRESSED));
	ctrlFiletype.AddString(CSTRING(DOCUMENT));
	ctrlFiletype.AddString(CSTRING(EXECUTABLE));
	ctrlFiletype.AddString(CSTRING(PICTURE));
	ctrlFiletype.AddString(CSTRING(VIDEO));
	//ctrlFiletype.AddString("Folder");
	ctrlFiletype.SetCurSel(0);
	
	// Create listview columns
	StringList l = StringTokenizer(SETTING(SEARCHFRAME_ORDER), ',').getTokens();
	{
		int k = 0;
		for(StringIter i = l.begin(); i != l.end(); ++i) {
			if(k >= COLUMN_LAST)
				break;
			columnIndexes[k++] = Util::toInt(*i);
		}
	}
	
	l = StringTokenizer(SETTING(SEARCHFRAME_WIDTHS), ',').getTokens();
	{
		int k = 0;
		for(StringIter i = l.begin(); i != l.end(); ++i) {
			if(k >= COLUMN_LAST)
				break;
			columnSizes[k++] = Util::toInt(*i);
		}
	}

	LV_COLUMN lvc;
	ZeroMemory(&lvc, sizeof(lvc));
	lvc.mask = LVCF_FMT | LVCF_ORDER | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	
	for(int j=0; j<COLUMN_LAST; j++)
	{
		lvc.pszText = const_cast<char*>(ResourceManager::getInstance()->getString(columnNames[j]).c_str());
		lvc.fmt = j == COLUMN_SIZE ? LVCFMT_RIGHT : LVCFMT_LEFT;
		lvc.cx = columnSizes[j];
		lvc.iOrder = columnIndexes[j];
		lvc.iSubItem = j;
		ctrlResults.InsertColumn(j, &lvc);
	}

	ctrlResults.SetBkColor(WinUtil::bgColor);
	ctrlResults.SetTextBkColor(WinUtil::bgColor);
	ctrlResults.SetTextColor(WinUtil::textColor);
	ctrlResults.SetFont(uiFont, FALSE);	// use Util::font instead to obey Appearace settings
	
	SetWindowText(CSTRING(SEARCH));

	targetMenu.CreatePopupMenu();
	resultsMenu.CreatePopupMenu();
	opMenu.CreatePopupMenu();
	
	resultsMenu.AppendMenu(MF_STRING, IDC_DOWNLOAD, CSTRING(DOWNLOAD));
	resultsMenu.AppendMenu(MF_POPUP, (UINT)(HMENU)targetMenu, CSTRING(DOWNLOAD_TO));
	resultsMenu.AppendMenu(MF_STRING, IDC_GETLIST, CSTRING(GET_FILE_LIST));
	resultsMenu.AppendMenu(MF_STRING, IDC_PRIVATEMESSAGE, CSTRING(SEND_PRIVATE_MESSAGE));
	resultsMenu.AppendMenu(MF_STRING, IDC_REMOVE, CSTRING(REMOVE));

	opMenu.AppendMenu(MF_STRING, IDC_DOWNLOAD, CSTRING(DOWNLOAD));
	opMenu.AppendMenu(MF_POPUP, (UINT)(HMENU)targetMenu, CSTRING(DOWNLOAD_TO));
	opMenu.AppendMenu(MF_STRING, IDC_GETLIST, CSTRING(GET_FILE_LIST));
	opMenu.AppendMenu(MF_STRING, IDC_PRIVATEMESSAGE, CSTRING(SEND_PRIVATE_MESSAGE));
	opMenu.AppendMenu(MF_STRING, IDC_REMOVE, CSTRING(REMOVE));
	opMenu.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)NULL);
	opMenu.AppendMenu(MF_STRING, IDC_KICK, CSTRING(KICK_USER));
	opMenu.AppendMenu(MF_STRING, IDC_REDIRECT, CSTRING(REDIRECT));

	if(!initialString.empty()) {
		search = StringTokenizer(initialString, ' ').getTokens();
		lastSearches.push_back(initialString);
		ctrlSearchBox.InsertString(0, initialString.c_str());
		ctrlSearchBox.SetCurSel(0);
		ctrlSizeMode.SetCurSel(0);
		ctrlSize.SetWindowText(Util::toString(initialSize).c_str());
		ctrlFiletype.SetCurSel(0);
		SearchManager::getInstance()->search(initialString, initialSize, SearchManager::TYPE_ANY, initialMode);
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

		string target = SETTING(DOWNLOAD_DIRECTORY) + sr->getFileName();

		if(WinUtil::browseFile(target, m_hWnd)) {
			try {
				QueueManager::getInstance()->add(sr->getFile(), sr->getSize(), sr->getUser(), target);
			} catch(QueueException e) {
				ctrlStatus.SetText(1, e.getError().c_str());
			} catch(FileException e) {
				//..
			}
		}
	} else {
		string target = SETTING(DOWNLOAD_DIRECTORY);
		if(WinUtil::browseDirectory(target, m_hWnd)) {
			if(lastDirs.size() > 10)
				lastDirs.erase(lastDirs.begin());
			lastDirs.push_back(target);

			downloadSelected(target);
		}
	}
	return 0;
}

LRESULT SearchFrame::onDownloadTarget(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(ctrlResults.GetSelectedCount() == 1) {
		int i = ctrlResults.GetNextItem(-1, LVNI_SELECTED);
		dcassert(i != -1);
		SearchResult* sr = (SearchResult*)ctrlResults.GetItemData(i);
		dcassert((wID - IDC_DOWNLOAD_TARGET) <	(WORD)targets.size());
		try {
			QueueManager::getInstance()->add(sr->getFile(), sr->getSize(), sr->getUser(), targets[(wID - IDC_DOWNLOAD_TARGET)]);
		} catch(QueueException e) {
			ctrlStatus.SetText(1, e.getError().c_str());
		} catch(FileException e) {
			//..
		}
	} else if(ctrlResults.GetSelectedCount() > 1) {
		dcassert((wID - IDC_DOWNLOAD_TARGET) < (WORD)lastDirs.size());
		downloadSelected(lastDirs[wID-IDC_DOWNLOAD_TARGET]);
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
			lsize*=1024I64; break;
		case 2:
			lsize*=1024I64*1024I64; break;
		case 3:
			lsize*=1024I64*1024I64*1024I64; break;
		}
		
		for(int i = 0; i != ctrlResults.GetItemCount(); i++) {
			delete (SearchResult*)ctrlResults.GetItemData(i);
		}
		ctrlResults.DeleteAllItems();
		
		SearchManager::getInstance()->search(s, (LONGLONG)lsize, (SearchManager::TypeModes)ctrlFiletype.GetCurSel(),
			(SearchManager::SizeModes)ctrlMode.GetCurSel());

		if(BOOLSETTING(CLEAR_SEARCH)){
			ctrlSearch.SetWindowText("");
		} else {
			lastSearch = TimerManager::getInstance()->getTick();
		}
		
		if(ctrlSearchBox.GetCount() >= 10)
			ctrlSearchBox.DeleteString(10);
		ctrlSearchBox.InsertString(0, s.c_str());
		
		while(lastSearches.size() >= 10) {
			lastSearches.erase(lastSearches.begin());
		}

		if(find(lastSearches.begin(), lastSearches.end(), s) == lastSearches.end()) 
			lastSearches.push_back(s);
		
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
	if(ctrlSlots.GetCheck() == 1 && aResult->getFreeSlots() < 1)
		return;

	SearchResult* copy = new SearchResult(*aResult);
	
	string file, path;
	if(aResult->getFile().rfind('\\') == string::npos) {
		file = aResult->getFile();
	} else {
		file = aResult->getFile().substr(aResult->getFile().rfind('\\')+1);
		path = aResult->getFile().substr(0, aResult->getFile().rfind('\\')+1);
	}
	
	StringList* l = new StringList();
	l->push_back(aResult->getUser()->getNick());
	l->push_back(file);
	int i = file.rfind('.');
	if(i != string::npos) {
		l->push_back(file.substr(i + 1));
	} else {
		l->push_back(Util::emptyString);
	}
	l->push_back(Util::formatBytes(aResult->getSize()));
	l->push_back(path);
	l->push_back(aResult->getSlotString());
	if(aResult->getUser()) {
		l->push_back(aResult->getUser()->getConnection());
	} else {
		l->push_back(Util::emptyString);
	}
	l->push_back(aResult->getHubName());
	PostMessage(WM_SPEAKER, (WPARAM)l, (LPARAM)copy);	
}

LRESULT SearchFrame::onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	RECT rc;                    // client area of window 
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };        // location of mouse click 
	
	// Get the bounding rectangle of the client area. 
	ctrlResults.GetClientRect(&rc);
	ctrlResults.ScreenToClient(&pt); 
	
	if (PtInRect(&rc, pt) && ctrlResults.GetSelectedCount() > 0) 
	{
		int n = 0;

		ctrlResults.ClientToScreen(&pt);

		while(targetMenu.GetMenuItemCount() > 0) {
			targetMenu.DeleteMenu(0, MF_BYPOSITION);
		}
		
		if(ctrlResults.GetSelectedCount() == 1) {
			int pos = ctrlResults.GetNextItem(-1, LVNI_SELECTED);
			dcassert(pos != -1);
			SearchResult* sr = (SearchResult*)ctrlResults.GetItemData(pos);

			
			targets = QueueManager::getInstance()->getTargetsBySize(sr->getSize());
			for(StringIter i = targets.begin(); i != targets.end(); ++i) {
				targetMenu.AppendMenu(MF_STRING, IDC_DOWNLOAD_TARGET + n, i->c_str());
				n++;
			}

			targetMenu.AppendMenu(MF_STRING, IDC_DOWNLOADTO, CSTRING(BROWSE));

			if(sr->getUser() && sr->getUser()->isClientOp()) {
				opMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
			} else {
				resultsMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
			}
		} else {
			
			for(StringIter i = lastDirs.begin(); i != lastDirs.end(); ++i) {
				targetMenu.AppendMenu(MF_STRING, IDC_DOWNLOAD_TARGET + n, i->c_str());
				n++;
			}
			targetMenu.AppendMenu(MF_STRING, IDC_DOWNLOADTO, CSTRING(BROWSE));

			int pos = -1;
			bool op = true;
			while( (pos = ctrlResults.GetNextItem(pos, LVNI_SELECTED)) != -1) {
				SearchResult* sr = (SearchResult*) ctrlResults.GetItemData(pos);
				if(!sr->getUser() || !sr->getUser()->isClientOp()) {
					op = false;
					break;
				}
			}
			if(op)
				opMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
			else
				resultsMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
		}
		
		return TRUE; 
	}
	
	return FALSE; 
}

LRESULT SearchFrame::onKick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) { 
	LineDlg dlg;
	dlg.title = STRING(KICK_USER);
	dlg.description = STRING(ENTER_REASON);
	dlg.line = lastKick;
	if(dlg.DoModal() == IDOK) {
		lastKick = dlg.line;
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
	dlg1.line = lastRedirect;
	if(dlg1.DoModal() == IDOK) {
		dlg2.title = STRING(REDIRECT_USER);
		dlg2.description = STRING(ENTER_SERVER);
		dlg2.line = lastServer;
		if(dlg2.DoModal() == IDOK) {
			lastRedirect = dlg1.line;
			lastServer = dlg2.line;
			
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
			QueueManager::getInstance()->add(sr->getFile(), sr->getSize(), sr->getUser(), SETTING(DOWNLOAD_DIRECTORY) + sr->getFileName());
		} catch(QueueException e) {
			ctrlStatus.SetText(1, e.getError().c_str());
		} catch(FileException e) {
			//..
		}
	}
	return 0;
	
}

void SearchFrame::downloadSelected(const string& aDir) {
	int i=-1;
	
	while( (i = ctrlResults.GetNextItem(i, LVNI_SELECTED)) != -1) {
		SearchResult* sr = (SearchResult*)ctrlResults.GetItemData(i);
		try { 
			QueueManager::getInstance()->add(sr->getFile(), sr->getSize(), sr->getUser(), aDir + sr->getFileName());
		} catch(Exception e) {
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

LRESULT SearchFrame::onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	for(int i = 0; i < ctrlResults.GetItemCount(); i++) {
		delete (SearchResult*)ctrlResults.GetItemData(i);
	}

	string tmp1;
	string tmp2;
	
	ctrlResults.GetColumnOrderArray(COLUMN_LAST, columnIndexes);
	for(int j = COLUMN_FIRST; j != COLUMN_LAST; j++) {
		columnSizes[j] = ctrlResults.GetColumnWidth(j);
		tmp1 += Util::toString(columnIndexes[j]) + ",";
		tmp2 += Util::toString(columnSizes[j]) + ",";
	}
	tmp1.erase(tmp1.size()-1, 1);
	tmp2.erase(tmp2.size()-1, 1);
	
	SettingsManager::getInstance()->set(SettingsManager::SEARCHFRAME_ORDER, tmp1);
	SettingsManager::getInstance()->set(SettingsManager::SEARCHFRAME_WIDTHS, tmp2);

	bHandled = FALSE;
	return 0;
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
		int const width = 220, spacing = 50, labelH = 16, comboH = 120, lMargin = 2, rMargin = 4;
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
		ctrlResults.setSortDirection(!ctrlResults.getSortDirection());
	} else {
		if(l->iSubItem == COLUMN_SIZE) {
			ctrlResults.setSort(l->iSubItem, ExListViewCtrl::SORT_FUNC, true, sortSize);
		} else if(l->iSubItem == COLUMN_SLOTS) {
			ctrlResults.setSort(l->iSubItem, ExListViewCtrl::SORT_INT);
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
			onTab();
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

void SearchFrame::onTab() {
	HWND focus = GetFocus();
	if(focus == ctrlSearch.m_hWnd || focus == ctrlSearchBox.m_hWnd) {
		ctrlMode.SetFocus();
	} else if(focus == ctrlMode.m_hWnd) {
		ctrlSize.SetFocus();
	} else if(focus == ctrlSize.m_hWnd) {
		ctrlSizeMode.SetFocus();
	} else if(focus == ctrlSizeMode.m_hWnd) {
		ctrlFiletype.SetFocus();
	} else if(focus == ctrlFiletype.m_hWnd) {
		ctrlSlots.SetFocus();
	} else if(focus == ctrlSlots.m_hWnd) {
		ctrlDoSearch.SetFocus();
	} else if(focus == ctrlDoSearch.m_hWnd) {
		ctrlSearch.SetFocus();
	}
}

LRESULT SearchFrame::onSpeaker(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	SearchResult* sr = (SearchResult*)lParam;

	// Check previous search results for dupes
	for(int i = 0, j = ctrlResults.GetItemCount(); i < j; ++i) {
		SearchResult* sr2 = (SearchResult*)ctrlResults.GetItemData(i);
		if((sr->getUser() == sr2->getUser()) && (sr->getFile() == sr2->getFile())) {
			delete (StringList*)wParam;
			delete sr;
			return 0;
		}
	}

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
	ctrlResults.insert(*(StringList*)wParam, image, lParam);
	delete (StringList*)wParam;
	return 0;
}

/**
 * @file SearchFrm.cpp
 * $Id: SearchFrm.cpp,v 1.8 2002/05/12 21:54:08 arnetheduck Exp $
 */


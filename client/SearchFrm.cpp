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
#include "DCPlusPlus.h"

#include "SearchFrm.h"
#include "LineDlg.h"

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

	ctrlResults.InsertColumn(COLUMN_NICK, _T("User"), LVCFMT_LEFT, 100, COLUMN_NICK);
	ctrlResults.InsertColumn(COLUMN_FILENAME, _T("File"), LVCFMT_LEFT, 200, COLUMN_FILENAME);
	ctrlResults.InsertColumn(COLUMN_TYPE, _T("Type"), LVCFMT_LEFT, 50, COLUMN_TYPE);
	ctrlResults.InsertColumn(COLUMN_SIZE, _T("Size"), LVCFMT_RIGHT, 80, COLUMN_SIZE);
	ctrlResults.InsertColumn(COLUMN_PATH, _T("Path"), LVCFMT_LEFT, 100, COLUMN_PATH);
	ctrlResults.InsertColumn(COLUMN_SLOTS, _T("Slots"), LVCFMT_LEFT, 40, COLUMN_SLOTS);
	ctrlResults.InsertColumn(COLUMN_CONNECTION, _T("Connection"), LVCFMT_LEFT, 70, COLUMN_CONNECTION);
	ctrlResults.InsertColumn(COLUMN_HUB, _T("Hub"), LVCFMT_LEFT, 150, COLUMN_HUB);

	SetWindowText("Search");

	targetMenu.CreatePopupMenu();

	resultsMenu.CreatePopupMenu();
	opMenu.CreatePopupMenu();
	
	int n = 0;

	CMenuItemInfo mi;

	mi.fMask = MIIM_ID | MIIM_TYPE;
	mi.fType = MFT_STRING;
	mi.dwTypeData = "Get File List";
	mi.wID = IDC_GETLIST;
	resultsMenu.InsertMenuItem(n, TRUE, &mi);
	opMenu.InsertMenuItem(n++, TRUE, &mi);

	mi.fMask = MIIM_ID | MIIM_TYPE;
	mi.fType = MFT_STRING;
	mi.dwTypeData = "Download";
	mi.wID = IDC_DOWNLOAD;
	resultsMenu.InsertMenuItem(n, TRUE, &mi);
	opMenu.InsertMenuItem(n++, TRUE, &mi);
	
	mi.fMask = MIIM_TYPE | MIIM_SUBMENU;
	mi.fType = MFT_STRING;
	mi.dwTypeData = "Download to...";
	mi.hSubMenu = targetMenu;
	resultsMenu.InsertMenuItem(n, TRUE, &mi);
	opMenu.InsertMenuItem(n++, TRUE, &mi);
	
	mi.fMask = MIIM_TYPE;
	mi.fType = MFT_SEPARATOR;
	opMenu.InsertMenuItem(n++, TRUE, &mi);
	
	mi.fMask = MIIM_ID | MIIM_TYPE;
	mi.fType = MFT_STRING;
	mi.dwTypeData = "Kick User";
	mi.wID = IDC_KICK;
	opMenu.InsertMenuItem(n++, TRUE, &mi);
	
	mi.fMask = MIIM_ID | MIIM_TYPE;
	mi.fType = MFT_STRING;
	mi.dwTypeData = "Redirect";
	mi.wID = IDC_REDIRECT;
	opMenu.InsertMenuItem(n++, TRUE, &mi);
	
	bHandled = FALSE;
	
	return 1;
}

LRESULT SearchFrame::onDownloadTo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(ctrlResults.GetSelectedCount() == 1) {
		int i = ctrlResults.GetNextItem(-1, LVNI_SELECTED);
		dcassert(i != -1);
		SearchResult* sr = (SearchResult*)ctrlResults.GetItemData(i);

		string target = SETTING(DOWNLOAD_DIRECTORY) + sr->getFileName();
		if(Util::browseSaveFile(target)) {
			try {
				if(sr->getUser())
					DownloadManager::getInstance()->download(sr->getFile(), sr->getSize(), sr->getUser(), target);
				else
					DownloadManager::getInstance()->download(sr->getFile(), sr->getSize(), sr->getNick(), target);
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

LRESULT SearchFrame::onDownloadTarget(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(ctrlResults.GetSelectedCount() == 1) {
		int i = ctrlResults.GetNextItem(-1, LVNI_SELECTED);
		dcassert(i != -1);
		SearchResult* sr = (SearchResult*)ctrlResults.GetItemData(i);
		dcassert((wID - IDC_DOWNLOAD_TARGET) < targets.size());
		try {
			if(sr->getUser())
				DownloadManager::getInstance()->download(sr->getFile(), sr->getSize(), sr->getUser(), targets[(wID - IDC_DOWNLOAD_TARGET)]);
			else
				DownloadManager::getInstance()->download(sr->getFile(), sr->getSize(), sr->getNick(), targets[(wID - IDC_DOWNLOAD_TARGET)]);
		} catch(Exception e) {
			MessageBox(e.getError().c_str());
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
			delete (SearchResult*)ctrlResults.GetItemData(i);
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
	SearchResult* copy = new SearchResult(*aResult);
	
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
	PostMessage(WM_SPEAKER, (WPARAM)l, (LPARAM)copy);	
}

LRESULT SearchFrame::onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled) {
	RECT rc;                    // client area of window 
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };        // location of mouse click 
	
	// Get the bounding rectangle of the client area. 
	ctrlResults.GetClientRect(&rc);
	ctrlResults.ScreenToClient(&pt); 
	
	if (PtInRect(&rc, pt) && ctrlResults.GetSelectedCount() > 0) 
	{
		CMenuItemInfo mi;
		ctrlResults.ClientToScreen(&pt);

		while(targetMenu.GetMenuItemCount() > 0) {
			targetMenu.DeleteMenu(0, MF_BYPOSITION);
		}
		
		if(ctrlResults.GetSelectedCount() == 1) {
			int pos = ctrlResults.GetNextItem(-1, LVNI_SELECTED);
			dcassert(pos != -1);
			SearchResult* sr = (SearchResult*)ctrlResults.GetItemData(pos);

			int n = 0;
			
			targets = DownloadManager::getInstance()->getTargetsBySize(sr->getSize());
			for(StringIter i = targets.begin(); i != targets.end(); ++i) {
				mi.fMask = MIIM_ID | MIIM_TYPE;
				mi.fType = MFT_STRING;
				mi.dwTypeData = const_cast<LPSTR>(i->c_str());
				mi.wID = IDC_DOWNLOAD_TARGET + n;
				targetMenu.InsertMenuItem(n++, TRUE, &mi);
			}

			mi.fMask = MIIM_ID | MIIM_TYPE;
			mi.fType = MFT_STRING;
			mi.dwTypeData = "Browse...";
			mi.wID = IDC_DOWNLOADTO;
			targetMenu.InsertMenuItem(n++, TRUE, &mi);

			if(sr->getUser() && sr->getUser()->isClientOp()) {
				opMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
			} else {
				resultsMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
			}
		} else {
			mi.fMask = MIIM_ID | MIIM_TYPE;
			mi.fType = MFT_STRING;
			mi.dwTypeData = "Browse...";
			mi.wID = IDC_DOWNLOADTO;
			targetMenu.InsertMenuItem(0, TRUE, &mi);
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
	dlg.title = "Kick user(s)";
	dlg.description = "Please enter a reason";
	if(dlg.DoModal() == IDOK) {
		int i = -1;
		while( (i = ctrlResults.GetNextItem(i, LVNI_SELECTED)) != -1) {
			SearchResult* sr = (SearchResult*)ctrlResults.GetItemData(i);
			if(sr->getUser() && sr->getUser()->isOnline()) {
				sr->getUser()->clientMessage(sr->getUser()->getClientNick() + " is kicking " + sr->getUser()->getNick() + " because: " + dlg.line);
				sr->getUser()->privateMessage("You are being kicked because: " + dlg.line);
				sr->getUser()->kick();
			}
		}
	}
	
	return 0; 
};

LRESULT SearchFrame::onRedirect(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) { 
	LineDlg dlg1, dlg2;
	dlg1.title = "Redirect user(s)";
	dlg1.description = "Please enter a reason";
	if(dlg1.DoModal() == IDOK) {
		dlg2.title = "Redirect user(s)";
		dlg2.description = "Please enter destination server";
		if(dlg2.DoModal() == IDOK) {
			int i = -1;
			while( (i = ctrlResults.GetNextItem(i, LVNI_SELECTED)) != -1) {
				SearchResult* sr = (SearchResult*)ctrlResults.GetItemData(i);
				if(sr->getUser() && sr->getUser()->isOnline()) {
					sr->getUser()->redirect(dlg2.line, "You are being redirected to " + dlg2.line + ": " + dlg1.line);
				}
			}
		}
	}
	
	return 0; 
};

/**
 * @file SearchFrm.cpp
 * $Id: SearchFrm.cpp,v 1.18 2002/01/20 22:54:46 arnetheduck Exp $
 * @if LOG
 * $Log: SearchFrm.cpp,v $
 * Revision 1.18  2002/01/20 22:54:46  arnetheduck
 * Bugfixes to 0.131 mainly...
 *
 * Revision 1.17  2002/01/19 13:09:10  arnetheduck
 * Added a file class to hide ugly file code...and fixed a small resume bug (I think...)
 *
 * Revision 1.16  2002/01/18 17:41:43  arnetheduck
 * Reworked many right button menus, adding op commands and making more easy to use
 *
 * Revision 1.15  2002/01/16 20:56:27  arnetheduck
 * Bug fixes, file listing sort and some other small changes
 *
 * Revision 1.14  2002/01/15 21:57:53  arnetheduck
 * Hopefully fixed the two annoying bugs...
 *
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

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

#include "DirectoryListingFrm.h"
#include "WinUtil.h"

StringList DirectoryListingFrame::lastDirs;

LRESULT DirectoryListingFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	
	CreateSimpleStatusBar(ATL_IDS_IDLEMESSAGE, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SBARS_SIZEGRIP);
	ctrlStatus.Attach(m_hWndStatusBar);
	statusContainer.SubclassWindow(ctrlStatus.m_hWnd);

	ctrlTree.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_HASLINES | TVS_SHOWSELALWAYS | TVS_DISABLEDRAGDROP, WS_EX_CLIENTEDGE, IDC_DIRECTORIES);
	ctrlList.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS, WS_EX_CLIENTEDGE, IDC_FILES);
	if(BOOLSETTING(FULL_ROW_SELECT)) {
		ctrlList.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);
	}
	
	ctrlList.SetBkColor(WinUtil::bgColor);
	ctrlList.SetTextBkColor(WinUtil::bgColor);
	ctrlList.SetTextColor(WinUtil::textColor);
	
	ctrlTree.SetBkColor(WinUtil::bgColor);
	ctrlTree.SetTextColor(WinUtil::textColor);
	
	ctrlList.InsertColumn(COLUMN_FILENAME, CSTRING(FILENAME), LVCFMT_LEFT, 350, COLUMN_FILENAME);
	ctrlList.InsertColumn(COLUMN_TYPE, CSTRING(FILE_TYPE), LVCFMT_LEFT, 60, COLUMN_TYPE);
	ctrlList.InsertColumn(COLUMN_SIZE, CSTRING(SIZE), LVCFMT_RIGHT, 100, COLUMN_SIZE);

	ctrlList.setSort(COLUMN_FILENAME, ExListViewCtrl::SORT_FUNC_ITEM, true, sortFile);
	
	ctrlTree.SetImageList(WinUtil::fileImages, TVSIL_NORMAL);
	ctrlList.SetImageList(WinUtil::fileImages, LVSIL_SMALL);

	ctrlFind.Create(ctrlStatus.m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
		BS_PUSHBUTTON, 0, IDC_FIND);
	ctrlFind.SetWindowText(CSTRING(FIND));
	ctrlFind.SetFont(ctrlStatus.GetFont());

	ctrlFindNext.Create(ctrlStatus.m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
		BS_PUSHBUTTON, 0, IDC_NEXT);
	ctrlFindNext.SetWindowText(CSTRING(NEXT));
	ctrlFindNext.SetFont(ctrlStatus.GetFont());

	SetSplitterExtendedStyle(SPLIT_PROPORTIONAL);
	SetSplitterPanes(ctrlTree.m_hWnd, ctrlList.m_hWnd);
	m_nProportionalPos = 2500;
	
	updateTree(dl->getRoot(), NULL);
	files = dl->getTotalFileCount();
	size = Util::formatBytes(dl->getTotalSize());

	int w[5] = { 0, 1, 2, 3, 4 };
	ctrlStatus.SetParts(5, w);
	ctrlStatus.SetText(1, (STRING(FILES) + ": " + Util::toString(dl->getTotalFileCount())).c_str());
	ctrlStatus.SetText(2, (STRING(SIZE) + ": " + Util::formatBytes(dl->getTotalSize())).c_str());
	
	fileMenu.CreatePopupMenu();
	targetMenu.CreatePopupMenu();
	directoryMenu.CreatePopupMenu();
	targetDirMenu.CreatePopupMenu();
	
	fileMenu.AppendMenu(MF_STRING, IDC_DOWNLOAD, CSTRING(DOWNLOAD));
	fileMenu.AppendMenu(MF_POPUP, (UINT)(HMENU)targetMenu, CSTRING(DOWNLOAD_TO));
	
	directoryMenu.AppendMenu(MF_STRING, IDC_DOWNLOADDIR, CSTRING(DOWNLOAD));
	directoryMenu.AppendMenu(MF_POPUP, (UINT)(HMENU)targetDirMenu, CSTRING(DOWNLOAD_TO));
	
	bHandled = FALSE;
	return 1;
}

void DirectoryListingFrame::updateTree(DirectoryListing::Directory* aTree, HTREEITEM aParent) {
	for(DirectoryListing::Directory::Iter i = aTree->directories.begin(); i != aTree->directories.end(); ++i) {
		HTREEITEM ht = ctrlTree.InsertItem(TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM, (*i)->getName().c_str(), WinUtil::getDirIconIndex(), WinUtil::getDirIconIndex(), 0, 0, (LPARAM)*i, aParent, TVI_SORT);;
		updateTree(*i, ht);
	}
}

LRESULT DirectoryListingFrame::onSelChangedDirectories(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NMTREEVIEW* p = (NMTREEVIEW*) pnmh;

	if(p->itemNew.state & TVIS_SELECTED) {
		DirectoryListing::Directory* d = (DirectoryListing::Directory*)p->itemNew.lParam;
		ctrlList.SetRedraw(FALSE);
		clearList();

		for(DirectoryListing::Directory::Iter i = d->directories.begin(); i != d->directories.end(); ++i) {
			DirectoryListing::Directory* d = *i;
			StringList l;
			l.push_back(d->getName());
			l.push_back(Util::emptyString);
			l.push_back(Util::formatBytes(d->getTotalSize()));
			ctrlList.insert(ctrlList.GetItemCount(), l, WinUtil::getDirIconIndex(), (LPARAM)new ItemInfo(d));
		}
		for(DirectoryListing::File::Iter j = d->files.begin(); j != d->files.end(); ++j) {
			string::size_type k = (*j)->getName().rfind('.');
			string suffix = (k != string::npos) ? (*j)->getName().substr(k + 1) : Util::emptyString;
			StringList l;
			l.push_back((*j)->getName());
			l.push_back(suffix);
			l.push_back(Util::formatBytes((*j)->getSize()));

			ctrlList.insert(ctrlList.GetItemCount(), l, WinUtil::getIconIndex((*j)->getName()), (LPARAM)new ItemInfo(*j));
		}
		ctrlList.SetRedraw(TRUE);
		ctrlList.resort();
	}
	return 0;
}

LRESULT DirectoryListingFrame::onDoubleClickFiles(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NMITEMACTIVATE* item = (NMITEMACTIVATE*) pnmh;
	char buf[MAX_PATH];

	HTREEITEM t = ctrlTree.GetSelectedItem();
	if(t != NULL && item->iItem != -1) {
		ItemInfo* ii = (ItemInfo*) ctrlList.GetItemData(item->iItem);

		if(ii->type == ItemInfo::FILE) {
			try {
				dl->download(ii->file, user, SETTING(DOWNLOAD_DIRECTORY) + ii->file->getName());
			} catch(Exception e) {
				ctrlStatus.SetText(0, e.getError().c_str());
			}
		} else {
			HTREEITEM ht = ctrlTree.GetChildItem(t);
			while(ht != NULL) {
				ctrlTree.GetItemText(ht, buf, sizeof(buf));
				if((DirectoryListing::Directory*)ctrlTree.GetItemData(ht) == ii->dir) {
					ctrlTree.SelectItem(ht);
					break;
				}
				ht = ctrlTree.GetNextSiblingItem(ht);
			}
		} 
	}
	return 0;
}

LRESULT DirectoryListingFrame::onDownloadDir(WORD , WORD , HWND , BOOL& ) {
	HTREEITEM t = ctrlTree.GetSelectedItem();
	if(t != NULL) {
		DirectoryListing::Directory* dir = (DirectoryListing::Directory*)ctrlTree.GetItemData(t);
		string target = SETTING(DOWNLOAD_DIRECTORY);
		try {
			dl->download(dir, user, target);
		} catch(Exception e) {
			ctrlStatus.SetText(0, e.getError().c_str());
		}
	}
	return 0;
}

LRESULT DirectoryListingFrame::onDownloadDirTo(WORD , WORD , HWND , BOOL& ) {
	HTREEITEM t = ctrlTree.GetSelectedItem();
	if(t != NULL) {
		DirectoryListing::Directory* dir = (DirectoryListing::Directory*)ctrlTree.GetItemData(t);
		string target = SETTING(DOWNLOAD_DIRECTORY);
		if(WinUtil::browseDirectory(target, m_hWnd)) {
			if(lastDirs.size() > 10)
				lastDirs.erase(lastDirs.begin());
			lastDirs.push_back(target);
			
			try {
				dl->download(dir, user, target);
			} catch(Exception e) {
				ctrlStatus.SetText(0, e.getError().c_str());
			}
		}
	}
	return 0;
}

void DirectoryListingFrame::downloadList(const string& aTarget) {
	int i=-1;
	while( (i = ctrlList.GetNextItem(i, LVNI_SELECTED)) != -1) {
		ItemInfo* ii = (ItemInfo*)ctrlList.GetItemData(i);

		string target = aTarget.empty() ? SETTING(DOWNLOAD_DIRECTORY) : aTarget;

		try {
			if(ii->type == ItemInfo::FILE) {
				dl->download(ii->file, user, target + ii->file->getName());
			} else {
				dl->download(ii->dir, user, target);
			} 
		} catch(Exception e) {
			ctrlStatus.SetText(0, e.getError().c_str());
		}
	}
}

LRESULT DirectoryListingFrame::onDownload(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	downloadList(SETTING(DOWNLOAD_DIRECTORY));
	return 0;
}

LRESULT DirectoryListingFrame::onDownloadTo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(ctrlList.GetSelectedCount() == 1) {
		ItemInfo* ii = (ItemInfo*)ctrlList.GetItemData(ctrlList.GetNextItem(-1, LVNI_SELECTED));

		try {
			if(ii->type == ItemInfo::FILE) {
				string target = ii->file->getName();
				if(WinUtil::browseFile(target, m_hWnd))
					dl->download(ii->file, user, target);
			} else {
				string target = SETTING(DOWNLOAD_DIRECTORY);
				if(WinUtil::browseDirectory(target, m_hWnd)) {
					if(lastDirs.size() > 10)
						lastDirs.erase(lastDirs.begin());
					lastDirs.push_back(target);

					dl->download(ii->dir, user, target);
				}
			} 
		} catch(Exception e) {
			ctrlStatus.SetText(0, e.getError().c_str());
		}
	} else {
		string target = SETTING(DOWNLOAD_DIRECTORY);
		if(WinUtil::browseDirectory(target, m_hWnd)) {
			if(lastDirs.size() > 10)
				lastDirs.erase(lastDirs.begin());
			lastDirs.push_back(target);
			
			downloadList(target);
		}
	}
	return 0;
}

LRESULT DirectoryListingFrame::onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	RECT rc;                    // client area of window 
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };        // location of mouse click 
	
	// Get the bounding rectangle of the client area. 
	ctrlList.GetClientRect(&rc);
	ctrlList.ScreenToClient(&pt); 
	
	if (PtInRect(&rc, pt) && ctrlList.GetSelectedCount() > 0) {
		int n = 0;
		ctrlList.ClientToScreen(&pt);

		ItemInfo* ii = (ItemInfo*)ctrlList.GetItemData(ctrlList.GetNextItem(-1, LVNI_SELECTED));

		while(targetMenu.GetMenuItemCount() > 0) {
			targetMenu.DeleteMenu(0, MF_BYPOSITION);
		}

		if(ctrlList.GetSelectedCount() == 1 && ii->type == ItemInfo::FILE) {

			targets = QueueManager::getInstance()->getTargetsBySize(ii->file->getSize());
			for(StringIter i = targets.begin(); i != targets.end(); ++i) {
				targetMenu.AppendMenu(MF_STRING, IDC_DOWNLOAD_TARGET + (n++), i->c_str());
			}
			targetMenu.AppendMenu(MF_STRING, IDC_DOWNLOADTO, CSTRING(BROWSE));

			fileMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
		} else {
			for(StringIter i = lastDirs.begin(); i != lastDirs.end(); ++i) {
				targetMenu.AppendMenu(MF_STRING, IDC_DOWNLOAD_TARGET + (n++), i->c_str());
			}
			targetMenu.AppendMenu(MF_STRING, IDC_DOWNLOADTO, CSTRING(BROWSE));
			fileMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
		}
		
		return TRUE; 
	} else { 
		
		ctrlList.ClientToScreen(&pt);
		
		ctrlTree.GetClientRect(&rc);
		ctrlTree.ScreenToClient(&pt); 
		
		if (PtInRect(&rc, pt) && ctrlTree.GetSelectedItem() != NULL) 
		{ 
			while(targetDirMenu.GetMenuItemCount() > 0) {
				targetDirMenu.DeleteMenu(0, MF_BYPOSITION);
			}
			int n = 0;
			for(StringIter i = lastDirs.begin(); i != lastDirs.end(); ++i) {
				targetDirMenu.AppendMenu(MF_STRING, IDC_DOWNLOAD_TARGET_DIR + (n++), i->c_str());
			}
			targetDirMenu.AppendMenu(MF_STRING, IDC_DOWNLOADDIRTO, CSTRING(BROWSE));
			
			ctrlTree.ClientToScreen(&pt);
			directoryMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
			
			return TRUE; 
		} 
	}
	
	return FALSE; 
}

LRESULT DirectoryListingFrame::onDownloadTarget(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(ctrlList.GetSelectedCount() == 1) {
		ItemInfo* ii = (ItemInfo*)ctrlList.GetItemData(ctrlList.GetNextItem(-1, LVNI_SELECTED));

		if(ii->type == ItemInfo::FILE) {
			dcassert((wID - IDC_DOWNLOAD_TARGET) < (WORD)targets.size());

			try {
				dl->download(ii->file, user, targets[wID-IDC_DOWNLOAD_TARGET]);
			} catch(Exception e) {
				ctrlStatus.SetText(0, e.getError().c_str());
			} 
		} else {
			dcassert((StringList::size_type)(wID-IDC_DOWNLOAD_TARGET) < lastDirs.size());
			downloadList(lastDirs[wID-IDC_DOWNLOAD_TARGET]);
		}
	} else if(ctrlList.GetSelectedCount() > 1) {
		dcassert((StringList::size_type)(wID-IDC_DOWNLOAD_TARGET) < lastDirs.size());
		downloadList(lastDirs[wID-IDC_DOWNLOAD_TARGET]);
	}
	return 0;
}

LRESULT DirectoryListingFrame::onDownloadTargetDir(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	HTREEITEM t = ctrlTree.GetSelectedItem();
	if(t != NULL) {
		DirectoryListing::Directory* dir = (DirectoryListing::Directory*)ctrlTree.GetItemData(t);
		string target = SETTING(DOWNLOAD_DIRECTORY);
		try {
			dl->download(dir, user, lastDirs[wID - IDC_DOWNLOAD_TARGET_DIR]);
		} catch(Exception e) {
			ctrlStatus.SetText(0, e.getError().c_str());
		}
	}
	return 0;
}

int DirectoryListingFrame::sortFile(LPARAM a, LPARAM b) {
	ItemInfo* c = (ItemInfo*)((LVITEM*)a)->lParam;
	ItemInfo* d = (ItemInfo*)((LVITEM*)b)->lParam;
	
	if(c->type == ItemInfo::DIRECTORY) {
		if(d->type == ItemInfo::FILE) {
			return -1;
		}
		return Util::stricmp(c->dir->getName().c_str(), d->dir->getName().c_str());
	} else {
		if(d->type == ItemInfo::DIRECTORY) {
			return 1;
		}
		return Util::stricmp(c->file->getName().c_str(), d->file->getName().c_str());
	}
}

int DirectoryListingFrame::sortType(LPARAM a, LPARAM b) {
	ItemInfo* c = (ItemInfo*)((LVITEM*)a)->lParam;
	ItemInfo* d = (ItemInfo*)((LVITEM*)b)->lParam;
	
	if(c->type == ItemInfo::DIRECTORY) {
		if(d->type == ItemInfo::FILE) {
			return -1;
		}
		return Util::stricmp(c->dir->getName().c_str(), d->dir->getName().c_str());
	} else {
		if(d->type == ItemInfo::DIRECTORY) {
			return 1;
		}

		string::size_type k = c->file->getName().rfind('.');
		string suffix1 = (k != string::npos) ? c->file->getName().substr(k + 1) : Util::emptyString;
		k = d->file->getName().rfind('.');
		string suffix2 = (k != string::npos) ? d->file->getName().substr(k + 1) : Util::emptyString;
		
		return Util::stricmp(suffix1.c_str(), suffix2.c_str());
	}
}

int DirectoryListingFrame::sortSize(LPARAM a, LPARAM b) {
	ItemInfo* c = (ItemInfo*)((LVITEM*)a)->lParam;
	ItemInfo* d = (ItemInfo*)((LVITEM*)b)->lParam;
	
	if(c->type == ItemInfo::DIRECTORY) {
		if(d->type == ItemInfo::FILE) {
			return -1;
		}
		LONGLONG g = c->dir->getTotalSize();
		LONGLONG h = d->dir->getTotalSize();
		
		return (g < h) ? -1 : ((g == h) ? 0 : 1);
	} else {
		if(d->type == ItemInfo::DIRECTORY) {
			return 1;
		}
		
		LONGLONG g = c->file->getSize();
		LONGLONG h = d->file->getSize();
		return (g < h) ? -1 : ((g == h) ? 0 : 1);
	}
}

void DirectoryListingFrame::UpdateLayout(BOOL bResizeBars /* = TRUE */) {
	RECT rect;
	GetClientRect(&rect);
	// position bars and offset their dimensions
	UpdateBarsPosition(rect, bResizeBars);

	if(ctrlStatus.IsWindow()) {
		CRect sr;
		int w[5];
		ctrlStatus.GetClientRect(sr);
		int tmp = (sr.Width()) > 516 ? 416 : ((sr.Width() > 116) ? sr.Width()-100 : 16);

		w[0] = sr.right - tmp;
		w[1] = w[0] + (tmp-16)*3/8;
		w[2] = w[0] + (tmp-16)*6/8;
		w[3] = w[0] + (tmp-16)*7/8;
		w[4] = w[0] + (tmp-16)*8/8;

		ctrlStatus.SetParts(5, w);

		ctrlStatus.GetRect(3, sr);
		sr.left = w[2];
		sr.right = w[3];
		ctrlFind.MoveWindow(sr);

		sr.left = w[3];
		sr.right = w[4];
		ctrlFindNext.MoveWindow(sr);
	}

	SetSplitterRect(&rect);
}

#include "LineDlg.h"

DirectoryListing::Directory *DirectoryListingFrame::findFile(string const& str,
															 DirectoryListing::Directory *root, DirectoryListing::File *&foundFile, int &skipHits)
{
	// Check dir name
	if(Util::findSubString(root->getName(), str) != string::npos)
	{
		if(skipHits == 0)
		{
			foundFile = 0;
			return root;
		}
		else
			skipHits--;
	}

	// Check file names in dir
	DirectoryListing::File::List::const_iterator it = root->files.begin();
	DirectoryListing::File::List::const_iterator const end = root->files.end();

	for(; it != end; it++)
	{
		if(Util::findSubString((*it)->getName(), str) != string::npos)
		{
			if(skipHits == 0)
			{
				foundFile = *it;
				return (*it)->getParent();
			}
			else
				skipHits--;
		}
	}

	// Check subdirs
	DirectoryListing::Directory::List::const_iterator it2 = root->directories.begin();
	DirectoryListing::Directory::List::const_iterator const end2 = root->directories.end();

	for(; it2 != end2; it2++)
	{
		DirectoryListing::Directory *srch = findFile(str, *it2, foundFile, skipHits);
		if(srch)
			return srch;
	}

	// No match
	return 0;
}

void DirectoryListingFrame::findFile(bool findNext)
{
	if(!findNext)
	{
		LineDlg dlg;
		dlg.title = STRING(SEARCH_FOR_FILE);
		dlg.description = STRING(ENTER_SEARCH_STRING);
		dlg.line = Util::emptyString;

		if(dlg.DoModal() != IDOK)
			return;

		findStr = dlg.line;
		skipHits = 0;
	}
	else
		skipHits++;

	if(findStr.size() == 0)
		return;

	DirectoryListing::File *foundFile;
	int skipHitsTmp = skipHits;
	DirectoryListing::Directory *foundDir = findFile(findStr, dl->getRoot(), foundFile,
		skipHitsTmp);

	if(foundDir)
	{
		StringList path;
		DirectoryListing::Directory *parent = foundDir;

		// Make path as reversed directory list from the file found
		while(parent)
		{
			path.push_back(parent->getName());
			parent = parent->getParent();

			// Skip meaningless top directory added by DC++
			if(!parent)
				path.pop_back();
		}

		// Locate tree item from path
		HTREEITEM item = ctrlTree.GetRootItem();
		char buf[128];
		for(int i=path.size()-1; item && i>=0; i--)
		{
			//dcdebug("locating path fragment \"%s\"...\n", path[i].c_str());
			do
			{
				ctrlTree.GetItemText(item, buf, 127);
				if(path[i] == buf)
				{
					// Enter directory if not at last level already
					if(i > 0)
						item = ctrlTree.GetChildItem(item);
					break;
				}

				item = ctrlTree.GetNextSiblingItem(item);
			}
			while(item != NULL);
			dcassert(item != NULL);
		}

		// Highlight the directory tree and list if the parent dir/a matched dir was found
		if(item)
		{
			string foundName;
			if(foundFile)
			{
				// Got a file; select the parent dir in the tree and highlight the file
				foundName = foundFile->getName();
			}
			else
			{
				HTREEITEM parentItem = ctrlTree.GetParentItem(item);
				if(parentItem)
				{
					// A dir; select the parent dir in the tree and highlight the dir
					foundName = foundDir->getName();
					item = parentItem;
				}
				// If no parent exists, just the dir tree item and skip the list highlighting
			}

			// Select directory in tree
			ctrlTree.SelectItem(item);

			// Unselect all items in list view
			if(ctrlList.GetSelectedCount() > 0)
				for(int i=0; i<ctrlList.GetItemCount(); i++)
					ctrlList.SetItemState(i, 0, LVIS_SELECTED);

			// Highlight item in list
			if(foundName.size() > 0)
			{
				int listPos = ctrlList.find(foundName, -1, true);
				if(listPos >= 0)
				{
					ctrlList.SetFocus();
					ctrlList.EnsureVisible(listPos, FALSE);
					ctrlList.SetItemState(listPos, LVIS_SELECTED | LVIS_FOCUSED, (UINT)-1);
				}
			}
			else
				ctrlTree.SetFocus();
		}
	}
	else		
		MessageBox(CSTRING(NO_MATCHES), CSTRING(SEARCH_FOR_FILE));
}

/**
 * @file DirectoryListingFrm.cpp
 * $Id: DirectoryListingFrm.cpp,v 1.10 2002/05/30 19:09:33 arnetheduck Exp $
 */

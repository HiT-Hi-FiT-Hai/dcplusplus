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

	SetSplitterExtendedStyle(SPLIT_PROPORTIONAL);
	SetSplitterPanes(ctrlTree.m_hWnd, ctrlList.m_hWnd);
	m_nProportionalPos = 2500;
	
	updateTree(dl->getRoot(), NULL);
	files = dl->getTotalFileCount();
	size = Util::formatBytes(dl->getTotalSize());
	
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
			ctrlList.insert(l, WinUtil::getDirIconIndex(), (LPARAM)new ItemInfo(d));
		}
		for(DirectoryListing::File::Iter j = d->files.begin(); j != d->files.end(); ++j) {
			string::size_type k = (*j)->getName().rfind('.');
			string suffix = (k != string::npos) ? (*j)->getName().substr(k + 1) : Util::emptyString;
			StringList l;
			l.push_back((*j)->getName());
			l.push_back(suffix);
			l.push_back(Util::formatBytes((*j)->getSize()));

			ctrlList.insert(l, WinUtil::getIconIndex((*j)->getName()), (LPARAM)new ItemInfo(*j));
		}
		ctrlList.SetRedraw(TRUE);
		ctrlList.Invalidate();
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
		return stricmp(c->dir->getName().c_str(), d->dir->getName().c_str());
	} else {
		if(d->type == ItemInfo::DIRECTORY) {
			return 1;
		}
		return stricmp(c->file->getName().c_str(), d->file->getName().c_str());
	}
}

int DirectoryListingFrame::sortType(LPARAM a, LPARAM b) {
	ItemInfo* c = (ItemInfo*)((LVITEM*)a)->lParam;
	ItemInfo* d = (ItemInfo*)((LVITEM*)b)->lParam;
	
	if(c->type == ItemInfo::DIRECTORY) {
		if(d->type == ItemInfo::FILE) {
			return -1;
		}
		return stricmp(c->dir->getName().c_str(), d->dir->getName().c_str());
	} else {
		if(d->type == ItemInfo::DIRECTORY) {
			return 1;
		}

		string::size_type k = c->file->getName().rfind('.');
		string suffix1 = (k != string::npos) ? c->file->getName().substr(k + 1) : Util::emptyString;
		k = d->file->getName().rfind('.');
		string suffix2 = (k != string::npos) ? d->file->getName().substr(k + 1) : Util::emptyString;
		
		return stricmp(suffix1.c_str(), suffix2.c_str());
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
		int w[3];
		ctrlStatus.GetClientRect(sr);
		int tmp = (sr.Width()) > 316 ? 216 : ((sr.Width() > 116) ? sr.Width()-100 : 16);
		
		w[0] = sr.right - tmp;
		w[1] = w[0] + (tmp-16)/2;
		w[2] = w[0] + (tmp-16);
		
		ctrlStatus.SetParts(3, w);
		ctrlStatus.SetText(1, (STRING(FILES) + ": " + Util::toString(files)).c_str());
		ctrlStatus.SetText(2, (STRING(FILES) + ": " + size).c_str());
	}
	
	SetSplitterRect(&rect);
}

/**
 * @file DirectoryListingFrm.cpp
 * $Id: DirectoryListingFrm.cpp,v 1.7 2002/04/28 08:25:50 arnetheduck Exp $
 */

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

#include "../client/File.h"
#include "../client/QueueManager.h"
#include "../client/StringTokenizer.h"

#include "Resource.h"

#include "DirectoryListingFrm.h"
#include "WinUtil.h"
#include "LineDlg.h"

void DirectoryListingFrame::openWindow(const string& aFile, const User::Ptr& aUser, const string& start) {
	DirectoryListingFrame* frame = new DirectoryListingFrame(aFile, aUser, start);
	frame->CreateEx(WinUtil::mdiClient);
}

DirectoryListingFrame::DirectoryListingFrame(const string& aFile, const User::Ptr& aUser, const string& s) :
	statusContainer(STATUSCLASSNAME, this, STATUS_MESSAGE_MAP),
	treeRoot(NULL), skipHits(0), updating(false), dl(NULL), searching(false), start(s)
{
	string tmp;
	if(aFile.size() < 4) {
		error = aUser->getFullNick() + ": " + STRING(UNSUPPORTED_FILELIST_FORMAT);
		return;
	}

	dl = new DirectoryListing(aUser);
	try {
		dl->loadFile(aFile, true);
	} catch(const Exception& e) {
		error = aUser->getFullNick() + ": " + e.getError();
	}
}

LRESULT DirectoryListingFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	
	CreateSimpleStatusBar(ATL_IDS_IDLEMESSAGE, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SBARS_SIZEGRIP);
	ctrlStatus.Attach(m_hWndStatusBar);
	statusContainer.SubclassWindow(ctrlStatus.m_hWnd);

	ctrlTree.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_HASLINES | TVS_SHOWSELALWAYS | TVS_DISABLEDRAGDROP, WS_EX_CLIENTEDGE, IDC_DIRECTORIES);
	ctrlList.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS, WS_EX_CLIENTEDGE, IDC_FILES);
	ctrlList.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);
	
	ctrlList.SetBkColor(WinUtil::bgColor);
	ctrlList.SetTextBkColor(WinUtil::bgColor);
	ctrlList.SetTextColor(WinUtil::textColor);
	
	ctrlTree.SetBkColor(WinUtil::bgColor);
	ctrlTree.SetTextColor(WinUtil::textColor);
	
	ctrlList.InsertColumn(COLUMN_FILENAME, CSTRING(FILENAME), LVCFMT_LEFT, 300, COLUMN_FILENAME);
	ctrlList.InsertColumn(COLUMN_TYPE, CSTRING(FILE_TYPE), LVCFMT_LEFT, 60, COLUMN_TYPE);
	ctrlList.InsertColumn(COLUMN_SIZE, CSTRING(SIZE), LVCFMT_RIGHT, 100, COLUMN_SIZE);
	ctrlList.InsertColumn(COLUMN_TTH, CSTRING(TTH_ROOT), LVCFMT_LEFT, 200, COLUMN_TTH);

	ctrlList.setSortColumn(COLUMN_FILENAME);
	
	ctrlTree.SetImageList(WinUtil::fileImages, TVSIL_NORMAL);
	ctrlList.SetImageList(WinUtil::fileImages, LVSIL_SMALL);

	ctrlFind.Create(ctrlStatus.m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
		BS_PUSHBUTTON, 0, IDC_FIND);
	ctrlFind.SetWindowText(CSTRING(FIND));
	ctrlFind.SetFont(WinUtil::systemFont);

	ctrlFindNext.Create(ctrlStatus.m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
		BS_PUSHBUTTON, 0, IDC_NEXT);
	ctrlFindNext.SetWindowText(CSTRING(NEXT));
	ctrlFindNext.SetFont(WinUtil::systemFont);

	ctrlMatchQueue.Create(ctrlStatus.m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
		BS_PUSHBUTTON, 0, IDC_MATCH_QUEUE);
	ctrlMatchQueue.SetWindowText(CSTRING(MATCH_QUEUE));
	ctrlMatchQueue.SetFont(WinUtil::systemFont);

	SetSplitterExtendedStyle(SPLIT_PROPORTIONAL);
	SetSplitterPanes(ctrlTree.m_hWnd, ctrlList.m_hWnd);
	m_nProportionalPos = 2500;
	
	if(dl != NULL)
		treeRoot = ctrlTree.InsertItem(TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM, dl->getUser()->getNick().c_str(), WinUtil::getDirIconIndex(), WinUtil::getDirIconIndex(), 0, 0, (LPARAM)dl->getRoot(), NULL, TVI_SORT);;

	updateTree(dl->getRoot(), treeRoot);
	files = dl->getTotalFileCount();
	size = Util::formatBytes(dl->getTotalSize());

	memset(statusSizes, 0, sizeof(statusSizes));
	string tmp1 = STRING(FILES) + ": " + Util::toString(dl->getTotalFileCount(true));
	string tmp2 = STRING(SIZE) + ": " + Util::formatBytes(dl->getTotalSize(true));
	statusSizes[2] = WinUtil::getTextWidth(tmp1, m_hWnd);
	statusSizes[3] = WinUtil::getTextWidth(tmp2, m_hWnd);
	statusSizes[4] = WinUtil::getTextWidth(STRING(MATCH_QUEUE), m_hWnd) + 8;
	statusSizes[5] = WinUtil::getTextWidth(STRING(FIND), m_hWnd) + 8;
	statusSizes[6] = WinUtil::getTextWidth(STRING(NEXT), m_hWnd) + 8;

	ctrlStatus.SetParts(8, statusSizes);
	ctrlStatus.SetText(3, tmp1.c_str());
	ctrlStatus.SetText(4, tmp2.c_str());

	if(!start.empty()) {
		StringTokenizer tok(start, '\\');
		StringIter i = tok.getTokens().begin();
		GoToDirectory(treeRoot, i, tok.getTokens().end());
	} else {
		ctrlTree.SelectItem(treeRoot);
	}
	
	fileMenu.CreatePopupMenu();
	targetMenu.CreatePopupMenu();
	directoryMenu.CreatePopupMenu();
	targetDirMenu.CreatePopupMenu();
	
	fileMenu.AppendMenu(MF_STRING, IDC_DOWNLOAD, CSTRING(DOWNLOAD));
	fileMenu.AppendMenu(MF_POPUP, (UINT)(HMENU)targetMenu, CSTRING(DOWNLOAD_TO));
	fileMenu.AppendMenu(MF_STRING, IDC_VIEW_AS_TEXT, CSTRING(VIEW_AS_TEXT));

	directoryMenu.AppendMenu(MF_STRING, IDC_DOWNLOADDIR, CSTRING(DOWNLOAD));
	directoryMenu.AppendMenu(MF_POPUP, (UINT)(HMENU)targetDirMenu, CSTRING(DOWNLOAD_TO));
	
	setWindowTitle();

	m_hMenu = WinUtil::mainMenu;

	bHandled = FALSE;
	return 1;
}

void DirectoryListingFrame::updateTree(DirectoryListing::Directory* aTree, HTREEITEM aParent) {
	string tmp;
	for(DirectoryListing::Directory::Iter i = aTree->directories.begin(); i != aTree->directories.end(); ++i) {
		const string& name = dl->getUtf8() ? Util::toAcp((*i)->getName(), tmp) : (*i)->getName();
		HTREEITEM ht = ctrlTree.InsertItem(TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM, name.c_str(), WinUtil::getDirIconIndex(), WinUtil::getDirIconIndex(), 0, 0, (LPARAM)*i, aParent, TVI_SORT);;
		if((*i)->getAdls())
			ctrlTree.SetItemState(ht, TVIS_BOLD, TVIS_BOLD);
		updateTree(*i, ht);
	}
}

void DirectoryListingFrame::updateStatus() {
	if(!searching && !updating && ctrlStatus.IsWindow()) {
		int cnt = ctrlList.GetSelectedCount();
		int64_t total = 0;
		if(cnt == 0) {
			cnt = ctrlList.GetItemCount ();
			total = ctrlList.forEachT(ItemInfo::TotalSize()).total;
		} else {
			total = ctrlList.forEachSelectedT(ItemInfo::TotalSize()).total;
		}

		string tmp1 = STRING(ITEMS) + ": " + Util::toString(cnt);
		string tmp2 = STRING(SIZE) + ": " + Util::formatBytes(total);
		bool u = false;

		int w = WinUtil::getTextWidth(tmp1, ctrlStatus.m_hWnd);
		if(statusSizes[0] < w) {
			statusSizes[0] = w;
			u = true;
		}
		ctrlStatus.SetText(1, tmp1.c_str());
		w = WinUtil::getTextWidth(tmp2, ctrlStatus.m_hWnd);
		if(statusSizes[1] < w) {
			statusSizes[1] = w;
			u = true;
		}
		ctrlStatus.SetText(2, tmp2.c_str());

		if(u)
			UpdateLayout(TRUE);
	}
}

LRESULT DirectoryListingFrame::onSelChangedDirectories(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NMTREEVIEW* p = (NMTREEVIEW*) pnmh;

	if(p->itemNew.state & TVIS_SELECTED) {
		DirectoryListing::Directory* d = (DirectoryListing::Directory*)p->itemNew.lParam;
		changeDir(d, TRUE);
	}
	return 0;
}


void DirectoryListingFrame::changeDir(DirectoryListing::Directory* d, BOOL enableRedraw)
{
	ctrlList.SetRedraw(FALSE);
	updating = true;
	clearList();

	for(DirectoryListing::Directory::Iter i = d->directories.begin(); i != d->directories.end(); ++i) {
		ctrlList.insertItem(ctrlList.GetItemCount(), new ItemInfo(*i, dl->getUtf8()), WinUtil::getDirIconIndex());
	}
	for(DirectoryListing::File::Iter j = d->files.begin(); j != d->files.end(); ++j) {
		ItemInfo* ii = new ItemInfo(*j, dl->getUtf8());
		ctrlList.insertItem(ctrlList.GetItemCount(), ii, WinUtil::getIconIndex(ii->getText(COLUMN_FILENAME)));
	}
	ctrlList.resort();
	ctrlList.SetRedraw(enableRedraw);
	updating = false;
	updateStatus();
}

LRESULT DirectoryListingFrame::onDoubleClickFiles(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NMITEMACTIVATE* item = (NMITEMACTIVATE*) pnmh;

	HTREEITEM t = ctrlTree.GetSelectedItem();
	if(t != NULL && item->iItem != -1) {
		ItemInfo* ii = (ItemInfo*) ctrlList.GetItemData(item->iItem);

		if(ii->type == ItemInfo::FILE) {
			try {
				dl->download(ii->file, SETTING(DOWNLOAD_DIRECTORY) + ii->getText(COLUMN_FILENAME));
			} catch(const Exception& e) {
				ctrlStatus.SetText(0, e.getError().c_str());
			}
		} else {
			HTREEITEM ht = ctrlTree.GetChildItem(t);
			while(ht != NULL) {
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
		try {
			dl->download(dir, SETTING(DOWNLOAD_DIRECTORY));
		} catch(const Exception& e) {
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
			WinUtil::addLastDir(target);
			
			try {
				dl->download(dir, target);
			} catch(const Exception& e) {
				ctrlStatus.SetText(0, e.getError().c_str());
			}
		}
	}
	return 0;
}

void DirectoryListingFrame::downloadList(const string& aTarget, bool view /* = false */) {
	int i=-1;
	while( (i = ctrlList.GetNextItem(i, LVNI_SELECTED)) != -1) {
		ItemInfo* ii = (ItemInfo*)ctrlList.GetItemData(i);

		string target = aTarget.empty() ? SETTING(DOWNLOAD_DIRECTORY) : aTarget;

		try {
			if(ii->type == ItemInfo::FILE) {
				if(view) {
					File::deleteFile(target + Util::validateFileName(ii->file->getName()));
				}
				dl->download(ii->file, target + ii->getText(COLUMN_FILENAME), view);
			} else if(!view) {
				dl->download(ii->dir, target);
			} 
		} catch(const Exception& e) {
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
				string target = SETTING(DOWNLOAD_DIRECTORY) + ii->getText(COLUMN_FILENAME);
				if(WinUtil::browseFile(target, m_hWnd)) {
					WinUtil::addLastDir(Util::getFilePath(target));
					dl->download(ii->file, target);
				}
			} else {
				string target = SETTING(DOWNLOAD_DIRECTORY);
				if(WinUtil::browseDirectory(target, m_hWnd)) {
					WinUtil::addLastDir(target);
					dl->download(ii->dir, target);
				}
			} 
		} catch(const Exception& e) {
			ctrlStatus.SetText(0, e.getError().c_str());
		}
	} else {
		string target = SETTING(DOWNLOAD_DIRECTORY);
		if(WinUtil::browseDirectory(target, m_hWnd)) {
			WinUtil::addLastDir(target);			
			downloadList(target);
		}
	}
	return 0;
}

LRESULT DirectoryListingFrame::onViewAsText(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	downloadList(Util::getTempPath(), true);
	return 0;
}

LRESULT DirectoryListingFrame::onCopyTTH(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	ItemInfo* ii = ctrlList.getSelectedItem();
	if(ii != NULL)
		WinUtil::setClipboard(ii->getText(COLUMN_TTH));

	return 0;
}

LRESULT DirectoryListingFrame::onMatchQueue(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int x = QueueManager::getInstance()->matchListing(dl);
	char* buf = new char[STRING(MATCHED_FILES).length() + 32];
	sprintf(buf, CSTRING(MATCHED_FILES), x);
	ctrlStatus.SetText(0, buf);
	delete[] buf;
	return 0;
}

LRESULT DirectoryListingFrame::onGoToDirectory(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(ctrlList.GetSelectedCount() != 1) 
		return 0;

	string fullPath;
	ItemInfo* ii = (ItemInfo*)ctrlList.GetItemData(ctrlList.GetNextItem(-1, LVNI_SELECTED));
	if(ii->type == ItemInfo::FILE) {
		if(!ii->file->getAdls())
			return 0;
		DirectoryListing::Directory* pd = ii->file->getParent();
		while(pd != NULL && pd != dl->getRoot()) {
			fullPath = (string)"\\" + pd->getName() + fullPath;
			pd = pd->getParent();
		}
	} else if(ii->type == ItemInfo::DIRECTORY) {
		if(!(ii->dir->getAdls() && ii->dir->getParent() != dl->getRoot()))
			return 0;
		fullPath = ((DirectoryListing::AdlDirectory*)ii->dir)->getFullPath();
	}

	// Break full path
	StringList brokenPath;
	while(1) {
		if(fullPath.size() == 0 || fullPath[0] != '\\') 
			break;
		fullPath.erase(0, 1);
		string subPath = fullPath.substr(0, fullPath.find_first_of('\\'));
		fullPath.erase(0, subPath.size());
		brokenPath.push_back(subPath);
	}
	
	// Go to directory (recursive)
	StringList::iterator iPath = brokenPath.begin();
	GoToDirectory(ctrlTree.GetRootItem(), iPath, brokenPath.end());
	
	return 0;
}


void DirectoryListingFrame::GoToDirectory(
	HTREEITEM hItem, 
	StringList::iterator& iPath, 
	const StringList::iterator& iPathEnd)
{
	if(iPath == iPathEnd)
		return;	// unexpected
	if(!ctrlTree.ItemHasChildren(hItem))
		return; // unexpected

	// Check on tree children
	HTREEITEM hChild = ctrlTree.GetChildItem(hItem);
	char itemText[256];
	while(hChild != NULL) {
		if(!ctrlTree.GetItemText(hChild, itemText, 255))
			return; // unexpected
		if(Util::stricmp(*iPath, (string)itemText) == 0) {
			++iPath;
			if(iPath == iPathEnd) {
				ctrlTree.SelectItem(hChild);
				ctrlTree.EnsureVisible(hChild);
				return;
			}
			GoToDirectory(hChild, iPath, iPathEnd);
			return;
		}
		hChild = ctrlTree.GetNextItem(hChild, TVGN_NEXT);
	}
}

LRESULT DirectoryListingFrame::onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	RECT rc;                    // client area of window 
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };        // location of mouse click 
	
	fileMenu.RemoveMenu(IDC_GO_TO_DIRECTORY, MF_BYCOMMAND);

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
			targetMenu.AppendMenu(MF_STRING, IDC_DOWNLOADTO, CSTRING(BROWSE));
			targets.clear();
			QueueManager::getInstance()->getTargetsBySize(targets, ii->file->getSize(), Util::getFileExt(ii->file->getName()));
			if(targets.size() > 0) {
				targetMenu.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)NULL);
				for(StringIter i = targets.begin(); i != targets.end(); ++i) {
					targetMenu.AppendMenu(MF_STRING, IDC_DOWNLOAD_TARGET + (n++), i->c_str());
				}
			}
			if(WinUtil::lastDirs.size() > 0) {
				targetMenu.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)NULL);
				for(StringIter i = WinUtil::lastDirs.begin(); i != WinUtil::lastDirs.end(); ++i) {
					targetMenu.AppendMenu(MF_STRING, IDC_DOWNLOAD_TARGET + (n++), i->c_str());
				}
			}
			if(ii->file->getAdls())	{
				fileMenu.AppendMenu(MF_STRING, IDC_GO_TO_DIRECTORY, CSTRING(GO_TO_DIRECTORY));
			}
			fileMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
		} else {
			targetMenu.AppendMenu(MF_STRING, IDC_DOWNLOADTO, CSTRING(BROWSE));
			if(WinUtil::lastDirs.size() > 0) {
				targetMenu.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)NULL);
				for(StringIter i = WinUtil::lastDirs.begin(); i != WinUtil::lastDirs.end(); ++i) {
					targetMenu.AppendMenu(MF_STRING, IDC_DOWNLOAD_TARGET + (n++), i->c_str());
				}
			}
			if(ii->dir->getAdls() && ii->dir->getParent() != dl->getRoot()) {
				fileMenu.AppendMenu(MF_STRING, IDC_GO_TO_DIRECTORY, CSTRING(GO_TO_DIRECTORY));
			}
			fileMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
		}
		
		return TRUE; 
	} else { 
		
		ctrlList.ClientToScreen(&pt);
		
		ctrlTree.GetClientRect(&rc);
		ctrlTree.ScreenToClient(&pt); 
		
		if (PtInRect(&rc, pt) && ctrlTree.GetSelectedItem() != NULL) { 
			// Strange, windows doesn't change the selection on right-click... (!)
			UINT a = 0;
			HTREEITEM ht = ctrlTree.HitTest(pt, &a);
			if(ht != NULL && ht != ctrlTree.GetSelectedItem())
				ctrlTree.SelectItem(ht);
			
			while(targetDirMenu.GetMenuItemCount() > 0) {
				targetDirMenu.DeleteMenu(0, MF_BYPOSITION);
			}

			targetDirMenu.AppendMenu(MF_STRING, IDC_DOWNLOADDIRTO, CSTRING(BROWSE));

			if(WinUtil::lastDirs.size() > 0) {
				targetDirMenu.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)NULL);
				int n = 0;
				for(StringIter i = WinUtil::lastDirs.begin(); i != WinUtil::lastDirs.end(); ++i) {
					targetDirMenu.AppendMenu(MF_STRING, IDC_DOWNLOAD_TARGET_DIR + (n++), i->c_str());
				}
			}
			
			ctrlTree.ClientToScreen(&pt);
			directoryMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
			
			return TRUE; 
		} 
	}
	
	return FALSE; 
}

LRESULT DirectoryListingFrame::onDownloadTarget(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int newId = wID - IDC_DOWNLOAD_TARGET;
	dcassert(newId >= 0);
	
	if(ctrlList.GetSelectedCount() == 1) {
		ItemInfo* ii = (ItemInfo*)ctrlList.GetItemData(ctrlList.GetNextItem(-1, LVNI_SELECTED));

		if(ii->type == ItemInfo::FILE) {
			dcassert(newId < (int)targets.size());

			try {
				dl->download(ii->file, targets[newId]);
			} catch(const Exception& e) {
				ctrlStatus.SetText(0, e.getError().c_str());
			} 
		} else {
			dcassert(newId < (int)WinUtil::lastDirs.size());
			downloadList(WinUtil::lastDirs[newId]);
		}
	} else if(ctrlList.GetSelectedCount() > 1) {
		dcassert(newId < (int)WinUtil::lastDirs.size());
		downloadList(WinUtil::lastDirs[newId]);
	}
	return 0;
}

LRESULT DirectoryListingFrame::onDownloadTargetDir(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int newId = wID - IDC_DOWNLOAD_TARGET_DIR;
	dcassert(newId >= 0);
	
	HTREEITEM t = ctrlTree.GetSelectedItem();
	if(t != NULL) {
		DirectoryListing::Directory* dir = (DirectoryListing::Directory*)ctrlTree.GetItemData(t);
		string target = SETTING(DOWNLOAD_DIRECTORY);
		try {
			dcassert(newId < (int)WinUtil::lastDirs.size());
			dl->download(dir, WinUtil::lastDirs[newId]);
		} catch(const Exception& e) {
			ctrlStatus.SetText(0, e.getError().c_str());
		}
	}
	return 0;
}

LRESULT DirectoryListingFrame::onKeyDown(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NMLVKEYDOWN* kd = (NMLVKEYDOWN*) pnmh;
	if(kd->wVKey == VK_BACK) {
		HTREEITEM cur = ctrlTree.GetSelectedItem();
		if(cur != NULL)
		{
			HTREEITEM parent = ctrlTree.GetParentItem(cur);
			if(parent != NULL)
				ctrlTree.SelectItem(parent);
		}
	} else if(kd->wVKey == VK_TAB) {
		onTab();
	} else if(kd->wVKey == VK_RETURN) {
		if(ctrlList.GetSelectedCount() == 1) {
			ItemInfo* ii = (ItemInfo*)ctrlList.GetItemData(ctrlList.GetNextItem(-1, LVNI_SELECTED));
			if(ii->type == ItemInfo::DIRECTORY) {
				HTREEITEM ht = ctrlTree.GetChildItem(ctrlTree.GetSelectedItem());
				while(ht != NULL) {
					if((DirectoryListing::Directory*)ctrlTree.GetItemData(ht) == ii->dir) {
						ctrlTree.SelectItem(ht);
						break;
					}
					ht = ctrlTree.GetNextSiblingItem(ht);
				}
			} else {
				downloadList(SETTING(DOWNLOAD_DIRECTORY));
			}
		} else {
			downloadList(SETTING(DOWNLOAD_DIRECTORY));
		}
	}
	return 0;
}

void DirectoryListingFrame::UpdateLayout(BOOL bResizeBars /* = TRUE */) {
	RECT rect;
	GetClientRect(&rect);
	// position bars and offset their dimensions
	UpdateBarsPosition(rect, bResizeBars);

	if(ctrlStatus.IsWindow()) {
		CRect sr;
		int w[8];
		ctrlStatus.GetClientRect(sr);
		w[7] = sr.right - 16;
#define setw(x) w[x] = max(w[x+1] - statusSizes[x], 0)
		setw(6); setw(5); setw(4); setw(3); setw(2); setw(1); setw(0);

		ctrlStatus.SetParts(8, w);

		ctrlStatus.GetRect(6, sr);

		sr.left = w[4];
		sr.right = w[5];
		ctrlMatchQueue.MoveWindow(sr);

		sr.left = w[5];
		sr.right = w[6];
		ctrlFind.MoveWindow(sr);

		sr.left = w[6];
		sr.right = w[7];
		ctrlFindNext.MoveWindow(sr);
	}

	SetSplitterRect(&rect);
}

HTREEITEM DirectoryListingFrame::findFile(const StringSearch& str, HTREEITEM root,
										  int &foundFile, int &skipHits)
{
	// Check dir name for match
	DirectoryListing::Directory* dir = (DirectoryListing::Directory*)ctrlTree.GetItemData(root);
	if(str.match(dir->getName()))
	{
		if(skipHits == 0)
		{
			foundFile = -1;
			return root;
		}
		else
			skipHits--;
	}

	// Force list pane to contain files of current dir
	changeDir(dir, FALSE);

	// Check file names in list pane
	for(int i=0; i<ctrlList.GetItemCount(); i++)
	{
		ItemInfo* ii = (ItemInfo*)ctrlList.GetItemData(i);
		if(ii->type == ItemInfo::FILE)
		{
			if(str.match(ii->file->getName()))
			{
				if(skipHits == 0)
				{
					foundFile = i;
					return root;
				}
				else
					skipHits--;
			}
		}
	}

	dcdebug("looking for directories...\n");
	// Check subdirs recursively
	HTREEITEM item = ctrlTree.GetChildItem(root);
	while(item != NULL)
	{
		HTREEITEM srch = findFile(str, item, foundFile, skipHits);
		if(srch)
			return srch;
		else
			item = ctrlTree.GetNextSiblingItem(item);
	}

	return 0;
}

void DirectoryListingFrame::findFile(bool findNext)
{
	if(!findNext) {
		// Prompt for substring to find
		LineDlg dlg;
		dlg.title = STRING(SEARCH_FOR_FILE);
		dlg.description = STRING(ENTER_SEARCH_STRING);
		dlg.line = Util::emptyString;

		if(dlg.DoModal() != IDOK)
			return;

		findStr = dlg.line;
		skipHits = 0;
	} else {
		skipHits++;
	}

	if(findStr.empty())
		return;

	// Do a search
	int foundFile = -1, skipHitsTmp = skipHits;
	HTREEITEM const oldDir = ctrlTree.GetSelectedItem();
	HTREEITEM const foundDir = findFile(StringSearch(findStr), ctrlTree.GetRootItem(), foundFile, skipHitsTmp);
	ctrlTree.SetRedraw(TRUE);

	if(foundDir) {
		// Highlight the directory tree and list if the parent dir/a matched dir was found
		if(foundFile >= 0) {
			// SelectItem won't update the list if SetRedraw was set to FALSE and then
			// to TRUE and the item selected is the same as the last one... workaround:
			if(oldDir == foundDir)
				ctrlTree.SelectItem(NULL);

			ctrlTree.SelectItem(foundDir);
		} else {
			// Got a dir; select its parent directory in the tree if there is one
			HTREEITEM parentItem = ctrlTree.GetParentItem(foundDir);
			if(parentItem) {
				// Go to parent file list
				ctrlTree.SelectItem(parentItem);

				// Locate the dir in the file list
				DirectoryListing::Directory* dir = (DirectoryListing::Directory*)ctrlTree.GetItemData(foundDir);
				
				foundFile = ctrlList.findItem(dir->getName(), -1, false);
			} else {
				// If no parent exists, just the dir tree item and skip the list highlighting
				ctrlTree.SelectItem(foundDir);
			}
		}

		// Remove prev. selection from file list
		if(ctrlList.GetSelectedCount() > 0) {
			for(int i=0; i<ctrlList.GetItemCount(); i++)
				ctrlList.SetItemState(i, 0, LVIS_SELECTED);
		}

		// Highlight and focus the dir/file if possible
		if(foundFile >= 0) {
			ctrlList.SetFocus();
			ctrlList.EnsureVisible(foundFile, FALSE);
			ctrlList.SetItemState(foundFile, LVIS_SELECTED | LVIS_FOCUSED, (UINT)-1);
		} else {
			ctrlTree.SetFocus();
		}
	} else {
		ctrlTree.SelectItem(oldDir);
		MessageBox(CSTRING(NO_MATCHES), CSTRING(SEARCH_FOR_FILE));
	}
}

/**
 * @file
 * $Id: DirectoryListingFrm.cpp,v 1.33 2004/07/05 16:02:43 arnetheduck Exp $
 */

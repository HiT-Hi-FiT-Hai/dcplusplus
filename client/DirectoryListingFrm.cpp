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

#include "DirectoryListingFrm.h"
#include "DirectoryListing.h"
#include "QueueManager.h"

void DirectoryListingFrame::updateTree(DirectoryListing::Directory* aTree, HTREEITEM aParent) {
	for(DirectoryListing::Directory::Iter i = aTree->directories.begin(); i != aTree->directories.end(); ++i) {
		HTREEITEM ht = ctrlTree.InsertItem(TVIF_IMAGE | TVIF_TEXT | TVIF_PARAM, (*i)->getName().c_str(), I_IMAGECALLBACK, I_IMAGECALLBACK, 0, 0, (LPARAM)*i, aParent, TVI_SORT);;
		updateTree(*i, ht);
	}
}

LRESULT DirectoryListingFrame::onGetDispInfoDirectories(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NMTVDISPINFO* p = (NMTVDISPINFO*)pnmh;
	TVITEM t;
	t.hItem = p->item.hItem;
	t.mask = TVIF_STATE;
	ctrlTree.GetItem(&t);
	
	if(p->item.mask & TVIF_IMAGE) {
		p->item.iImage = (t.state & TVIS_EXPANDED) ? 1 : 0;
	}
	if(p->item.mask & TVIF_SELECTEDIMAGE) {
		p->item.iSelectedImage = (t.state & TVIS_EXPANDED) ? 1 : 0;
	}

	return 0;
}

LRESULT DirectoryListingFrame::onSelChangedDirectories(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NMTREEVIEW* p = (NMTREEVIEW*) pnmh;

	if(p->itemNew.state & TVIS_SELECTED) {
		DirectoryListing::Directory* d = (DirectoryListing::Directory*)p->itemNew.lParam;
		ctrlList.SetRedraw(FALSE);
		ctrlList.DeleteAllItems();
		for(DirectoryListing::Directory::Iter i = d->directories.begin(); i != d->directories.end(); ++i) {
			DirectoryListing::Directory* d = *i;
			StringList l;
			l.push_back(d->getName());
			l.push_back(Util::formatBytes(d->getTotalSize()));
			ctrlList.insert(l, IMAGE_DIRECTORY, (LPARAM)d);
		}
		for(DirectoryListing::File::Iter j = d->files.begin(); j != d->files.end(); ++j) {
			
			StringList l;
			l.push_back((*j)->getName());
			l.push_back(Util::formatBytes((*j)->getSize()));
			ctrlList.insert(l, IMAGE_FILE, (LPARAM)*j);
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
		
		LVITEM lvi;
		lvi.iItem = item->iItem;
		lvi.iSubItem = 0;
		lvi.mask = LVIF_PARAM | LVIF_IMAGE;

		ctrlList.GetItem(&lvi);

		if(lvi.iImage == IMAGE_FILE) {
			DirectoryListing::File* file = (DirectoryListing::File*) lvi.lParam;
			try {
				dl->download(file, user, SETTING(DOWNLOAD_DIRECTORY) + file->getName());
			} catch(Exception e) {
				ctrlStatus.SetText(0, e.getError().c_str());
			}
		} else {
			DirectoryListing::Directory* d = (DirectoryListing::Directory*) lvi.lParam;

			HTREEITEM ht = ctrlTree.GetChildItem(t);
			while(ht != NULL) {
				ctrlTree.GetItemText(ht, buf, sizeof(buf));
				if((DirectoryListing::Directory*)ctrlTree.GetItemData(ht) == d) {
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
		if(Util::browseDirectory(target, m_hWnd)) {
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
		LVITEM lvi;
		lvi.iItem = i;
		lvi.iSubItem = 0;
		lvi.mask = LVIF_PARAM | LVIF_IMAGE;
		string target;
		ctrlList.GetItem(&lvi);
		if(aTarget.empty()) {
			target = SETTING(DOWNLOAD_DIRECTORY);
		} else {
			target = aTarget;
		}
		try {
			if(lvi.iImage == 2) {
				DirectoryListing::File* file = (DirectoryListing::File*) lvi.lParam;
				dl->download(file, user, target + file->getName());
			} else {
				DirectoryListing::Directory* d = (DirectoryListing::Directory*) lvi.lParam;
				dl->download(d, user, target);
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
		LVITEM lvi;
		lvi.iItem = ctrlList.GetNextItem(-1, LVNI_SELECTED);
		lvi.iSubItem = 0;
		lvi.mask = LVIF_PARAM | LVIF_IMAGE;
		ctrlList.GetItem(&lvi);

		try {
			if(lvi.iImage == 2) {
				DirectoryListing::File* file = (DirectoryListing::File*) lvi.lParam;
				string target = file->getName();
				if(Util::browseSaveFile(target))
					dl->download(file, user, target);
			} else {
				DirectoryListing::Directory* d = (DirectoryListing::Directory*) lvi.lParam;
				string target;
				if(Util::browseDirectory(target)) {
					dl->download(d, user, target);
				}
			} 
		} catch(Exception e) {
			ctrlStatus.SetText(0, e.getError().c_str());
		}
	} else {
		string target = SETTING(DOWNLOAD_DIRECTORY);
		if(Util::browseDirectory(target, m_hWnd)) {
			downloadList(target);
		}
		
	}
	return 0;
}

LRESULT DirectoryListingFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	char buf[256];

	CreateSimpleStatusBar(ATL_IDS_IDLEMESSAGE, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SBARS_SIZEGRIP);
	ctrlStatus.Attach(m_hWndStatusBar);

	ctrlTree.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_HASLINES | TVS_SHOWSELALWAYS | TVS_DISABLEDRAGDROP, WS_EX_CLIENTEDGE, IDC_DIRECTORIES);
	ctrlList.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS, WS_EX_CLIENTEDGE, IDC_FILES);
	if(BOOLSETTING(FULL_ROW_SELECT)) {
		ctrlList.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);
	}

	ctrlList.SetBkColor(Util::bgColor);
	ctrlList.SetTextBkColor(Util::bgColor);
	ctrlList.SetTextColor(Util::textColor);

	ctrlTree.SetBkColor(Util::bgColor);
	ctrlTree.SetTextColor(Util::textColor);
	
	ctrlList.InsertColumn(COLUMN_FILENAME, _T("Filename"), LVCFMT_LEFT, 400, COLUMN_FILENAME);
	ctrlList.InsertColumn(COLUMN_SIZE, _T("Size"), LVCFMT_RIGHT, 100, COLUMN_SIZE);
	
	ctrlImages.CreateFromImage(IDB_FOLDERS, 16, 3, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION | LR_SHARED);
	ctrlTree.SetImageList(ctrlImages, TVSIL_NORMAL);
	ctrlList.SetImageList(ctrlImages, LVSIL_SMALL);
	SetSplitterExtendedStyle(SPLIT_PROPORTIONAL);
	SetSplitterPanes(ctrlTree.m_hWnd, ctrlList.m_hWnd);
	m_nProportionalPos = 2500;
	
	updateTree(dl->getRoot(), NULL);
	files = dl->getTotalFileCount();
	size = Util::formatBytes(dl->getTotalSize());
	
	sprintf(buf, "Files: %d\n", dl->getTotalFileCount());
	ctrlStatus.SetText(1, buf);
	sprintf(buf, "Size: %s\n", Util::formatBytes(dl->getTotalSize()).c_str());
	ctrlStatus.SetText(2, buf);

	fileMenu.CreatePopupMenu();
	oneFileMenu.CreatePopupMenu();
	targetMenu.CreatePopupMenu();
	directoryMenu.CreatePopupMenu();

	CMenuItemInfo mi;
	mi.fMask = MIIM_ID | MIIM_TYPE;
	mi.fType = MFT_STRING;
	mi.dwTypeData = "Download";
	mi.wID = IDC_DOWNLOAD;
	oneFileMenu.InsertMenuItem(0, TRUE, &mi);
	fileMenu.InsertMenuItem(0, TRUE, &mi);
	
	mi.fMask = MIIM_ID | MIIM_TYPE;
	mi.fType = MFT_STRING;
	mi.dwTypeData = "Download to...";
	mi.wID = IDC_DOWNLOADTO;
	fileMenu.InsertMenuItem(1, TRUE, &mi);

	mi.fMask = MIIM_TYPE | MIIM_SUBMENU;
	mi.fType = MFT_STRING;
	mi.dwTypeData = "Download to...";
	mi.hSubMenu = targetMenu;
	oneFileMenu.InsertMenuItem(1, TRUE, &mi);

	mi.fMask = MIIM_ID | MIIM_TYPE;
	mi.fType = MFT_STRING;
	mi.dwTypeData = "Download";
	mi.wID = IDC_DOWNLOADDIR;
	directoryMenu.InsertMenuItem(0, TRUE, &mi);

	mi.fMask = MIIM_ID | MIIM_TYPE;
	mi.fType = MFT_STRING;
	mi.dwTypeData = "Download to...";
	mi.wID = IDC_DOWNLOADDIRTO;
	directoryMenu.InsertMenuItem(1, TRUE, &mi);
	
	bHandled = FALSE;
	return 1;
}

LRESULT DirectoryListingFrame::onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	RECT rc;                    // client area of window 
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };        // location of mouse click 
	
	// Get the bounding rectangle of the client area. 
	ctrlList.GetClientRect(&rc);
	ctrlList.ScreenToClient(&pt); 
	
	if (PtInRect(&rc, pt) && ctrlList.GetSelectedCount() > 0) {
		ctrlList.ClientToScreen(&pt);

		LVITEM lvi;
		lvi.iItem = ctrlList.GetNextItem(-1, LVNI_SELECTED);
		lvi.iSubItem = 0;
		lvi.mask = LVIF_PARAM | LVIF_IMAGE;
		ctrlList.GetItem(&lvi);

		if(ctrlList.GetSelectedCount() == 1 && lvi.iImage == IMAGE_FILE) {
			while(targetMenu.GetMenuItemCount() > 0) {
				targetMenu.DeleteMenu(0, MF_BYPOSITION);
			}

			int pos = ctrlList.GetNextItem(-1, LVNI_SELECTED);
			dcassert(pos != -1);
			DirectoryListing::File* f = (DirectoryListing::File*)ctrlList.GetItemData(pos);
			
			int n = 0;
			CMenuItemInfo mi;

			targets = QueueManager::getInstance()->getTargetsBySize(f->getSize());
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
			
			oneFileMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
		} else {
			fileMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
		}
		
		return TRUE; 
	} else { 
		
		ctrlList.ClientToScreen(&pt);
		
		ctrlTree.GetClientRect(&rc);
		ctrlTree.ScreenToClient(&pt); 
		
		if (PtInRect(&rc, pt) && ctrlTree.GetSelectedItem() != NULL) 
		{ 
			ctrlTree.ClientToScreen(&pt);
			directoryMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
			
			return TRUE; 
		} 
	}
	
	return FALSE; 
}

LRESULT DirectoryListingFrame::onDownloadTarget(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(ctrlList.GetSelectedCount() == 1) {
		int i = ctrlList.GetNextItem(-1, LVNI_SELECTED);
		dcassert(i != -1);
		DirectoryListing::File* f = (DirectoryListing::File*)ctrlList.GetItemData(i);
		dcassert((wID - IDC_DOWNLOAD_TARGET) < (WORD)targets.size());
		try {
			if(user->isOnline())
				QueueManager::getInstance()->add(f->getName(), f->getSize(), user, targets[(wID - IDC_DOWNLOAD_TARGET)]);
			else
				QueueManager::getInstance()->add(f->getName(), f->getSize(), user->getNick(), targets[(wID - IDC_DOWNLOAD_TARGET)]);
		} catch(QueueException e) {
			ctrlStatus.SetText(0, e.getError().c_str());
		} catch(FileException e) {
			//..
		}
	}
	return 0;
}

/**
 * @file DirectoryListingFrm.cpp
 * $Id: DirectoryListingFrm.cpp,v 1.24 2002/02/09 18:13:51 arnetheduck Exp $
 * @if LOG
 * $Log: DirectoryListingFrm.cpp,v $
 * Revision 1.24  2002/02/09 18:13:51  arnetheduck
 * Fixed level 4 warnings and started using new stl
 *
 * Revision 1.23  2002/02/07 22:12:22  arnetheduck
 * Last fixes before 0.152
 *
 * Revision 1.22  2002/02/04 01:10:29  arnetheduck
 * Release 0.151...a lot of things fixed
 *
 * Revision 1.21  2002/02/01 02:00:26  arnetheduck
 * A lot of work done on the new queue manager, hopefully this should reduce
 * the number of crashes...
 *
 * Revision 1.20  2002/01/26 12:38:50  arnetheduck
 * Added some user options
 *
 * Revision 1.19  2002/01/20 22:54:46  arnetheduck
 * Bugfixes to 0.131 mainly...
 *
 * Revision 1.18  2002/01/19 19:07:39  arnetheduck
 * Last fixes before 0.13
 *
 * Revision 1.17  2002/01/16 20:56:26  arnetheduck
 * Bug fixes, file listing sort and some other small changes
 *
 * Revision 1.16  2002/01/13 22:50:48  arnetheduck
 * Time for 0.12, added favorites, a bunch of new icons and lot's of other stuff
 *
 * Revision 1.15  2002/01/07 20:17:59  arnetheduck
 * Finally fixed the reconnect bug that's been annoying me for a whole day...
 * Hopefully the app works better in w95 now too...
 *
 * Revision 1.14  2002/01/06 21:55:20  arnetheduck
 * Some minor bugs fixed, but there remains one strange thing, the reconnect
 * button doesn't work...
 *
 * Revision 1.13  2002/01/05 19:06:09  arnetheduck
 * Added user list images, fixed bugs and made things more effective
 *
 * Revision 1.11  2002/01/02 16:12:32  arnetheduck
 * Added code for multiple download sources
 *
 * Revision 1.10  2001/12/29 13:47:14  arnetheduck
 * Fixing bugs and UI work
 *
 * Revision 1.9  2001/12/19 23:07:59  arnetheduck
 * Added directory downloading from the directory tree (although it hasn't been
 * tested at all) and password support.
 *
 * Revision 1.8  2001/12/18 12:32:18  arnetheduck
 * Stability fixes
 *
 * Revision 1.7  2001/12/13 19:21:57  arnetheduck
 * A lot of work done almost everywhere, mainly towards a friendlier UI
 * and less bugs...time to release 0.06...
 *
 * Revision 1.6  2001/12/12 00:06:04  arnetheduck
 * Updated the public hub listings, fixed some minor transfer bugs, reworked the
 * sockets to use only one thread (instead of an extra thread for sending files),
 * and fixed a major bug in the client command decoding (still have to fix this
 * one for the userconnections...)
 *
 * Revision 1.5  2001/12/02 23:47:35  arnetheduck
 * Added the framework for uploading and file sharing...although there's something strange about
 * the file lists...my client takes them, but not the original...
 *
 * Revision 1.4  2001/12/02 11:16:46  arnetheduck
 * Optimised hub listing, removed a few bugs and leaks, and added a few small
 * things...downloads are now working, time to start writing the sharing
 * code...
 *
 * Revision 1.3  2001/12/01 17:15:03  arnetheduck
 * Added a crappy version of huffman encoding, and some other minor changes...
 *
 * Revision 1.2  2001/11/29 19:10:54  arnetheduck
 * Refactored down/uploading and some other things completely.
 * Also added download indicators and download resuming, along
 * with some other stuff.
 *
 * Revision 1.1  2001/11/26 23:40:36  arnetheduck
 * Downloads!! Now downloads are possible, although the implementation is
 * likely to change in the future...more UI work (splitters...) and some bug
 * fixes. Only user file listings are downloadable, but at least it's something...
 *
 * @endif
 */

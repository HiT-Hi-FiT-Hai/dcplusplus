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

#include "DirectoryListingFrm.h"
#include "DirectoryListing.h"

void DirectoryListingFrame::updateTree(DirectoryListing::Directory* aTree, HTREEITEM aParent) {
	for(DirectoryListing::Directory::Iter i = aTree->directories.begin(); i != aTree->directories.end(); ++i) {
		HTREEITEM ht = ctrlTree.InsertItem(TVIF_IMAGE | TVIF_TEXT | TVIF_PARAM, (*i)->name.c_str(), I_IMAGECALLBACK, I_IMAGECALLBACK, 0, 0, (LPARAM)*i, aParent, TVI_SORT);;
		updateTree(*i, ht);
	}
}
LRESULT DirectoryListingFrame::onGetDispInfoDirectories(int idCtrl, LPNMHDR pnmh, BOOL& bHandled) {
	NMTVDISPINFO* p = (NMTVDISPINFO*)pnmh;
	TVITEM t;
	t.hItem = p->item.hItem;
	t.mask = TVIF_STATE;
	ctrlTree.GetItem(&t);
	if(p->item.mask && (TVIF_IMAGE || TVIF_SELECTEDIMAGE)) {
		if(t.state && TVIS_EXPANDED) {
			p->item.iImage = 1;
			p->item.iSelectedImage = 1;
		} else {
			p->item.iImage = 0;
			p->item.iSelectedImage = 0;
		}
	} 
	return 0;
}
	
LRESULT DirectoryListingFrame::onSelChangedDirectories(int idCtrl, LPNMHDR pnmh, BOOL& bHandled) {
	NMTREEVIEW* p = (NMTREEVIEW*) pnmh;

	if(p->itemNew.state & TVIS_SELECTED) {
		DirectoryListing::Directory* d = (DirectoryListing::Directory*)p->itemNew.lParam;
		ctrlList.DeleteAllItems();
		for(DirectoryListing::File::Iter i = d->files.begin(); i != d->files.end(); ++i) {
			char buf[20];
			_i64toa((*i)->size, buf, 10);
			
			StringList l;
			l.push_back((*i)->name);
			l.push_back(buf);
			ctrlList.insertItem(l, 2);
		}
	}
	return 0;
}

LRESULT DirectoryListingFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	ctrlTree.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_HASLINES | TVS_SHOWSELALWAYS | TVS_DISABLEDRAGDROP, WS_EX_CLIENTEDGE, IDC_DIRECTORIES);
	ctrlList.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS, WS_EX_CLIENTEDGE, IDC_FILES);
	
	ctrlList.InsertColumn(0, _T("Filename"), LVCFMT_LEFT, 400, 0);
	ctrlList.InsertColumn(1, _T("Size"), LVCFMT_RIGHT, 100, 1);
	
	ctrlImages.CreateFromImage(IDB_FOLDERS, 16, 3, CLR_DEFAULT, IMAGE_BITMAP, LR_DEFAULTCOLOR);
	ctrlTree.SetImageList(ctrlImages, TVSIL_NORMAL);
	ctrlList.SetImageList(ctrlImages, LVSIL_SMALL);
	SetSplitterExtendedStyle(SPLIT_PROPORTIONAL);
	SetSplitterPanes(ctrlTree.m_hWnd, ctrlList.m_hWnd);
	m_nProportionalPos = 2500;
	
	updateTree(dl->getRoot(), NULL);
	
	bHandled = FALSE;
	return 1;
}

/**
 * @file DirectoryListingFrm.cpp
 * $Id: DirectoryListingFrm.cpp,v 1.1 2001/11/26 23:40:36 arnetheduck Exp $
 * @if LOG
 * $Log: DirectoryListingFrm.cpp,v $
 * Revision 1.1  2001/11/26 23:40:36  arnetheduck
 * Downloads!! Now downloads are possible, although the implementation is
 * likely to change in the future...more UI work (splitters...) and some bug
 * fixes. Only user file listings are downloadable, but at least it's something...
 *
 * @endif
 */

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

#if !defined(AFX_DIRECTORYFRM_H__A7078724_FD85_4F39_8463_5A08A5F45E33__INCLUDED_)
#define AFX_DIRECTORYFRM_H__A7078724_FD85_4F39_8463_5A08A5F45E33__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "User.h"

#include "FlatTabCtrl.h"

#include "DirectoryListing.h"
#include "ExListViewCtrl.h"

class User;

class DirectoryListingFrame : public MDITabChildWindowImpl<DirectoryListingFrame>, CSplitterImpl<DirectoryListingFrame>
{
public:
	
	enum {
		COLUMN_FILENAME,
		COLUMN_SIZE
	};
	
	enum {
		IMAGE_DIRECTORY = 0,
		IMAGE_FILE = 2
	};
	DirectoryListingFrame(DirectoryListing* aList, const string& aNick) :  dl(aList), user(aNick) { };

	~DirectoryListingFrame() {
		ctrlImages.Destroy();
	}

	DECLARE_FRAME_WND_CLASS("DirectoryListingFrame", IDR_DIRECTORY)

	virtual void OnFinalMessage(HWND /*hWnd*/)
	{
		delete dl;
		delete this;
	}

	BEGIN_MSG_MAP(DirectoryListingFrame)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_FORWARDMSG, OnForwardMsg)
		MESSAGE_HANDLER(WM_CONTEXTMENU, onContextMenu)
		NOTIFY_HANDLER(IDC_FILES, NM_DBLCLK, onDoubleClickFiles)
		NOTIFY_HANDLER(IDC_DIRECTORIES, TVN_SELCHANGED, onSelChangedDirectories)
		NOTIFY_HANDLER(IDC_DIRECTORIES, TVN_GETDISPINFO, onGetDispInfoDirectories)
		COMMAND_ID_HANDLER(IDC_DOWNLOAD, onDownload)
		COMMAND_ID_HANDLER(IDC_DOWNLOADDIR, onDownloadDir)
		COMMAND_ID_HANDLER(IDC_DOWNLOADDIRTO, onDownloadDirTo)
		COMMAND_ID_HANDLER(IDC_DOWNLOADTO, onDownloadTo)
		NOTIFY_HANDLER(IDC_FILES, LVN_COLUMNCLICK, onColumnClickFiles)
		CHAIN_MSG_MAP(MDITabChildWindowImpl<DirectoryListingFrame>)
		CHAIN_MSG_MAP(CSplitterImpl<DirectoryListingFrame>)
	END_MSG_MAP()

	static int sortFile(LPARAM a, LPARAM b) {
		LVITEM* c = (LVITEM*)a;
		LVITEM* d = (LVITEM*)b;
		
		if(c->iImage == IMAGE_DIRECTORY) {
			if(d->iImage == IMAGE_FILE) {
				return -1;
			}
			dcassert(c->iImage == IMAGE_DIRECTORY);

			DirectoryListing::Directory* e = (DirectoryListing::Directory*)c->lParam;
			DirectoryListing::Directory* f = (DirectoryListing::Directory*)d->lParam;
			
			return strnicmp(e->getName().c_str(), f->getName().c_str(), min(e->getName().size(), f->getName().size()));
		} else {
			if(d->iImage == IMAGE_DIRECTORY) {
				return 1;
			}
			dcassert(c->iImage == IMAGE_FILE);
			
			DirectoryListing::File* e = (DirectoryListing::File*)c->lParam;
			DirectoryListing::File* f = (DirectoryListing::File*)d->lParam;
			
			return strnicmp(e->getName().c_str(), f->getName().c_str(), min(e->getName().size(), f->getName().size()));
		}
	}
	
	static int sortSize(LPARAM a, LPARAM b) {
		LVITEM* c = (LVITEM*)a;
		LVITEM* d = (LVITEM*)b;
		
		if(c->iImage == IMAGE_DIRECTORY) {
			if(d->iImage == IMAGE_FILE) {
				return -1;
			}
			dcassert(c->iImage == IMAGE_DIRECTORY);
			
			DirectoryListing::Directory* e = (DirectoryListing::Directory*)c->lParam;
			DirectoryListing::Directory* f = (DirectoryListing::Directory*)d->lParam;
			LONGLONG g = e->getTotalSize();
			LONGLONG h = f->getTotalSize();
			
			return (g < h) ? -1 : ((g == h) ? 0 : 1);
		} else {
			if(d->iImage == IMAGE_DIRECTORY) {
				return 1;
			}
			dcassert(c->iImage == IMAGE_FILE);
			
			DirectoryListing::File* e = (DirectoryListing::File*)c->lParam;
			DirectoryListing::File* f = (DirectoryListing::File*)d->lParam;
			LONGLONG g = e->getSize();
			LONGLONG h = f->getSize();
			return (g < h) ? -1 : ((g == h) ? 0 : 1);
		}
	}

	LRESULT onColumnClickFiles(int idCtrl, LPNMHDR pnmh, BOOL& bHandled) {
		NMLISTVIEW* l = (NMLISTVIEW*)pnmh;

		if(l->iSubItem == ctrlList.getSortColumn()) {
			ctrlList.setSortDirection(!ctrlList.getSortDirection());
		} else {
			if(l->iSubItem == COLUMN_FILENAME) {
				ctrlList.setSort(l->iSubItem, ExListViewCtrl::SORT_FUNC_ITEM, true, sortFile);
			} else if(l->iSubItem == COLUMN_SIZE) {
				ctrlList.setSort(l->iSubItem, ExListViewCtrl::SORT_FUNC_ITEM, true, sortSize);
			}
		}
		return 0;
	}
	
	LRESULT onDownload(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onDownloadDir(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onDownloadDirTo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onDownloadTo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	
	void downloadList(const string& aTarget);
	
	LRESULT onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled) {
		RECT rc;                    // client area of window 
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };        // location of mouse click 
		
		// Get the bounding rectangle of the client area. 
		ctrlList.GetClientRect(&rc);
		ctrlList.ScreenToClient(&pt); 
		
		if (PtInRect(&rc, pt)) 
		{ 
			ctrlList.ClientToScreen(&pt);
			fileMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
			
			return TRUE; 
		} else { 
		
			ctrlList.ClientToScreen(&pt);

			ctrlTree.GetClientRect(&rc);
			ctrlTree.ScreenToClient(&pt); 
			
			if (PtInRect(&rc, pt)) 
			{ 
				ctrlTree.ClientToScreen(&pt);
				directoryMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
				
				return TRUE; 
			} 
		}
		
		return FALSE; 
	}
		
	LRESULT OnForwardMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
		return 0;
	}
	
	void setWindowTitle() {
		SetWindowText(user.c_str());
	}

	LRESULT onDoubleClickFiles(int idCtrl, LPNMHDR pnmh, BOOL& bHandled); 
	LRESULT onSelChangedDirectories(int idCtrl, LPNMHDR pnmh, BOOL& bHandled); 
	LRESULT onGetDispInfoDirectories(int idCtrl, LPNMHDR pnmh, BOOL& bHandled); 
	
	void updateTree(DirectoryListing::Directory* tree, HTREEITEM treeItem);

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);

	LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		// handled, no background painting needed
		return 1;
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
			
			char buf[512];
			sprintf(buf, "Files: %d", files);
			ctrlStatus.SetText(1, buf);
			sprintf(buf, "Size: %s", size.c_str());
			ctrlStatus.SetText(2, buf);
		}
		
		SetSplitterRect(&rect);
	}
	
private:
	CImageList ctrlImages;
	
	CMenu fileMenu;
	CMenu directoryMenu;
	
	string user;
	CTreeViewCtrl ctrlTree;
	ExListViewCtrl ctrlList;
	CStatusBarCtrl ctrlStatus;
	
	int files;
	string size;
	
	DirectoryListing* dl;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHILDFRM_H__A7078724_FD85_4F39_8463_5A08A5F45E33__INCLUDED_)

/**
 * @file DirectoryListingFrm.h
 * $Id: DirectoryListingFrm.h,v 1.11 2002/01/16 20:56:26 arnetheduck Exp $
 * @if LOG
 * $Log: DirectoryListingFrm.h,v $
 * Revision 1.11  2002/01/16 20:56:26  arnetheduck
 * Bug fixes, file listing sort and some other small changes
 *
 * Revision 1.10  2002/01/13 22:50:48  arnetheduck
 * Time for 0.12, added favorites, a bunch of new icons and lot's of other stuff
 *
 * Revision 1.9  2002/01/10 12:33:14  arnetheduck
 * Various fixes
 *
 * Revision 1.8  2001/12/27 12:05:00  arnetheduck
 * Added flat tabs, fixed sorting and a StringTokenizer bug
 *
 * Revision 1.7  2001/12/21 20:21:17  arnetheduck
 * Private messaging added, and a lot of other updates as well...
 *
 * Revision 1.6  2001/12/19 23:07:59  arnetheduck
 * Added directory downloading from the directory tree (although it hasn't been
 * tested at all) and password support.
 *
 * Revision 1.5  2001/12/13 19:21:57  arnetheduck
 * A lot of work done almost everywhere, mainly towards a friendlier UI
 * and less bugs...time to release 0.06...
 *
 * Revision 1.4  2001/12/12 00:06:04  arnetheduck
 * Updated the public hub listings, fixed some minor transfer bugs, reworked the
 * sockets to use only one thread (instead of an extra thread for sending files),
 * and fixed a major bug in the client command decoding (still have to fix this
 * one for the userconnections...)
 *
 * Revision 1.3  2001/12/02 23:47:35  arnetheduck
 * Added the framework for uploading and file sharing...although there's something strange about
 * the file lists...my client takes them, but not the original...
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

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

#if !defined(AFX_DIRECTORYFRM_H__A7078724_FD85_4F39_8463_5A08A5F45E33__INCLUDED_)
#define AFX_DIRECTORYFRM_H__A7078724_FD85_4F39_8463_5A08A5F45E33__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "../client/User.h"

#include "FlatTabCtrl.h"
#include "ExListViewCtrl.h"

#include "../client/DirectoryListing.h"
#include "../client/CryptoManager.h"

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
	DirectoryListingFrame(const string& aFile, const User::Ptr& aUser) : user(aUser) { 

		File f(aFile, File::READ, File::OPEN);
		dl = new DirectoryListing();
		DWORD size = (DWORD)f.getSize();
		
		string tmp;
		if(size > 16) {
			BYTE* buf = new BYTE[size];
			f.read(buf, size);
			CryptoManager::getInstance()->decodeHuffman(buf, tmp);
			delete[] buf;
		} else {
			tmp = "";
		}
		dl->load(tmp);
	};

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
		COMMAND_RANGE_HANDLER(IDC_DOWNLOAD_TARGET, IDC_DOWNLOAD_TARGET + targets.size(), onDownloadTarget)
		NOTIFY_HANDLER(IDC_FILES, LVN_COLUMNCLICK, onColumnClickFiles)
		CHAIN_MSG_MAP(MDITabChildWindowImpl<DirectoryListingFrame>)
		CHAIN_MSG_MAP(CSplitterImpl<DirectoryListingFrame>)
	END_MSG_MAP()

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onDownload(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onDownloadDir(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onDownloadDirTo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onDownloadTo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onDownloadTarget(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onDoubleClickFiles(int idCtrl, LPNMHDR pnmh, BOOL& bHandled); 
	LRESULT onSelChangedDirectories(int idCtrl, LPNMHDR pnmh, BOOL& bHandled); 
	LRESULT onGetDispInfoDirectories(int idCtrl, LPNMHDR pnmh, BOOL& bHandled); 
	LRESULT onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled);
	
	void downloadList(const string& aTarget);
	static int sortFile(LPARAM a, LPARAM b);
	static int sortSize(LPARAM a, LPARAM b);
	void updateTree(DirectoryListing::Directory* tree, HTREEITEM treeItem);
	void UpdateLayout(BOOL bResizeBars = TRUE);
	
	LRESULT OnForwardMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		return 0;
	}
	
	void setWindowTitle() {
		SetWindowText((user->getNick() + " (" + user->getClientName() + ")").c_str());
	}

	LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		return 1;
	}

	LRESULT onColumnClickFiles(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
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
	
private:
	CImageList ctrlImages;
	
	CMenu targetMenu;
	CMenu fileMenu;
	CMenu directoryMenu;
	
	StringList targets;
	StringList lastDirs;
	
	User::Ptr user;
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
 * $Id: DirectoryListingFrm.h,v 1.1 2002/04/09 18:46:32 arnetheduck Exp $
 * @if LOG
 * $Log: DirectoryListingFrm.h,v $
 * Revision 1.1  2002/04/09 18:46:32  arnetheduck
 * New files of the major reorganization
 *
 * Revision 1.18  2002/04/03 23:20:35  arnetheduck
 * ...
 *
 * Revision 1.17  2002/03/13 20:35:25  arnetheduck
 * Release canditate...internationalization done as far as 0.155 is concerned...
 * Also started using mirrors of the public hub lists
 *
 * Revision 1.16  2002/03/04 23:52:30  arnetheduck
 * Updates and bugfixes, new user handling almost finished...
 *
 * Revision 1.15  2002/02/12 00:35:37  arnetheduck
 * 0.153
 *
 * Revision 1.14  2002/02/09 18:13:51  arnetheduck
 * Fixed level 4 warnings and started using new stl
 *
 * Revision 1.13  2002/01/20 22:54:46  arnetheduck
 * Bugfixes to 0.131 mainly...
 *
 * Revision 1.12  2002/01/19 19:07:39  arnetheduck
 * Last fixes before 0.13
 *
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

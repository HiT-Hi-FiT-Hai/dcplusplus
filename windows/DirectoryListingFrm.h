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
		IDC_DOWNLOAD_TARGET = 5000,
		IDC_DOWNLOAD_TARGET_DIR = 5100
	};
	
	enum {
		COLUMN_FILENAME,
		COLUMN_TYPE,
		COLUMN_SIZE
	};
	
	DirectoryListingFrame(const string& aFile, const User::Ptr& aUser) : user(aUser) { 
		string tmp;
		try{
			File f(aFile, File::READ, File::OPEN);
			dl = new DirectoryListing();
			DWORD size = (DWORD)f.getSize();
			
			if(size > 16) {
				BYTE* buf = new BYTE[size];
				f.read(buf, size);
				CryptoManager::getInstance()->decodeHuffman(buf, tmp);
				delete[] buf;
			} else {
				tmp = Util::emptyString;
			}
		} catch(FileException) {
			string file = aFile.substr(0, aFile.size() - 5) + "bz2";
			
			File f(file, File::READ, File::OPEN);
			dl = new DirectoryListing();
			DWORD size = (DWORD)f.getSize();
			
			if(size > 16) {
				BYTE* buf = new BYTE[size];
				f.read(buf, size);
				CryptoManager::getInstance()->decodeBZ2(buf, size, tmp);
				delete[] buf;
			} else {
				tmp = Util::emptyString;
			}
		}

		dl->load(tmp);
	};

	~DirectoryListingFrame() { }

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
		MESSAGE_HANDLER(WM_CLOSE, onClose)
		NOTIFY_HANDLER(IDC_FILES, NM_DBLCLK, onDoubleClickFiles)
		NOTIFY_HANDLER(IDC_DIRECTORIES, TVN_SELCHANGED, onSelChangedDirectories)
		COMMAND_ID_HANDLER(IDC_DOWNLOAD, onDownload)
		COMMAND_ID_HANDLER(IDC_DOWNLOADDIR, onDownloadDir)
		COMMAND_ID_HANDLER(IDC_DOWNLOADDIRTO, onDownloadDirTo)
		COMMAND_ID_HANDLER(IDC_DOWNLOADTO, onDownloadTo)
		COMMAND_RANGE_HANDLER(IDC_DOWNLOAD_TARGET, IDC_DOWNLOAD_TARGET + max(targets.size(), lastDirs.size()), onDownloadTarget)
		COMMAND_RANGE_HANDLER(IDC_DOWNLOAD_TARGET, IDC_DOWNLOAD_TARGET_DIR + lastDirs.size(), onDownloadTargetDir)
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
	LRESULT onDownloadTargetDir(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onDoubleClickFiles(int idCtrl, LPNMHDR pnmh, BOOL& bHandled); 
	LRESULT onSelChangedDirectories(int idCtrl, LPNMHDR pnmh, BOOL& bHandled); 
	LRESULT onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled);
	
	void downloadList(const string& aTarget);
	static int sortFile(LPARAM a, LPARAM b);
	static int sortSize(LPARAM a, LPARAM b);
	static int sortType(LPARAM a, LPARAM b);
	void updateTree(DirectoryListing::Directory* tree, HTREEITEM treeItem);
	void UpdateLayout(BOOL bResizeBars = TRUE);
	
	LRESULT OnForwardMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		return 0;
	}

	LRESULT onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
		clearList();
		bHandled = FALSE;
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
			} else if(l->iSubItem == COLUMN_TYPE) {
				ctrlList.setSort(l->iSubItem, ExListViewCtrl::SORT_FUNC_ITEM, true, sortType);
			}
		}
		return 0;
	}
	
	void clearList() {
		int j = ctrlList.GetItemCount();
		for(int i = 0; i < j; i++) {
			delete (ItemInfo*)ctrlList.GetItemData(i);
		}
		ctrlList.DeleteAllItems();
	}
	
private:
	
	class ItemInfo {
	public:
		enum ItemType {
			FILE,
			DIRECTORY
		} type;
		
		union {
			DirectoryListing::File* file;
			DirectoryListing::Directory* dir;
		};

		ItemInfo(DirectoryListing::File* f) : type(FILE), file(f) { };
		ItemInfo(DirectoryListing::Directory* d) : type(DIRECTORY), dir(d) { };
	};
	
	CMenu targetMenu;
	CMenu targetDirMenu;
	CMenu fileMenu;
	CMenu directoryMenu;
	
	StringList targets;
	static StringList lastDirs;
	
	User::Ptr user;
	CTreeViewCtrl ctrlTree;
	ExListViewCtrl ctrlList;
	CStatusBarCtrl ctrlStatus;
	
	int files;
	string size;
	
	DirectoryListing* dl;
};

#endif // !defined(AFX_CHILDFRM_H__A7078724_FD85_4F39_8463_5A08A5F45E33__INCLUDED_)

/**
 * @file DirectoryListingFrm.h
 * $Id: DirectoryListingFrm.h,v 1.6 2002/04/28 08:25:50 arnetheduck Exp $
 */

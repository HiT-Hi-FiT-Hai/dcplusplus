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

#if !defined(AFX_DIRECTORYFRM_H__A7078724_FD85_4F39_8463_5A08A5F45E33__INCLUDED_)
#define AFX_DIRECTORYFRM_H__A7078724_FD85_4F39_8463_5A08A5F45E33__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "../client/User.h"

#include "FlatTabCtrl.h"
#include "ExListViewCtrl.h"
#include "WinUtil.h"

#include "../client/DirectoryListing.h"
#include "../client/StringSearch.h"

#define STATUS_MESSAGE_MAP 9

class DirectoryListingFrame : public MDITabChildWindowImpl<DirectoryListingFrame, RGB(255, 0, 255)>, public CSplitterImpl<DirectoryListingFrame>
{
public:
	static void openWindow(const string& aFile, const User::Ptr& aUser);

	typedef MDITabChildWindowImpl<DirectoryListingFrame, RGB(255, 0, 255)> baseClass;

	enum {
		COLUMN_FILENAME,
		COLUMN_TYPE,
		COLUMN_SIZE
	};
	
	DirectoryListingFrame(const string& aFile, const User::Ptr& aUser);
	~DirectoryListingFrame() { 
		delete dl; 
	}

	DECLARE_FRAME_WND_CLASS("DirectoryListingFrame", IDR_DIRECTORY)

	virtual void OnFinalMessage(HWND /*hWnd*/) {
		delete this;
	}

	BEGIN_MSG_MAP(DirectoryListingFrame)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_CONTEXTMENU, onContextMenu)
		MESSAGE_HANDLER(WM_CLOSE, onClose)
		MESSAGE_HANDLER(WM_SETFOCUS, onSetFocus)
		NOTIFY_HANDLER(IDC_FILES, LVN_COLUMNCLICK, onColumnClickFiles)
		NOTIFY_HANDLER(IDC_FILES, LVN_KEYDOWN, onKeyDown)
		NOTIFY_HANDLER(IDC_FILES, NM_DBLCLK, onDoubleClickFiles)
		NOTIFY_HANDLER(IDC_FILES, LVN_ITEMCHANGED, onItemChanged)
		NOTIFY_HANDLER(IDC_DIRECTORIES, TVN_KEYDOWN, onKeyDownDirs)
		NOTIFY_HANDLER(IDC_DIRECTORIES, TVN_SELCHANGED, onSelChangedDirectories)
		COMMAND_ID_HANDLER(IDC_DOWNLOAD, onDownload)
		COMMAND_ID_HANDLER(IDC_DOWNLOADDIR, onDownloadDir)
		COMMAND_ID_HANDLER(IDC_DOWNLOADDIRTO, onDownloadDirTo)
		COMMAND_ID_HANDLER(IDC_DOWNLOADTO, onDownloadTo)
		COMMAND_ID_HANDLER(IDC_GO_TO_DIRECTORY, onGoToDirectory)
		COMMAND_ID_HANDLER(IDC_VIEW_AS_TEXT, onViewAsText)
		COMMAND_RANGE_HANDLER(IDC_DOWNLOAD_TARGET, IDC_DOWNLOAD_TARGET + max(targets.size(), WinUtil::lastDirs.size()), onDownloadTarget)
		COMMAND_RANGE_HANDLER(IDC_DOWNLOAD_TARGET, IDC_DOWNLOAD_TARGET_DIR + WinUtil::lastDirs.size(), onDownloadTargetDir)
		CHAIN_MSG_MAP(baseClass)
		CHAIN_MSG_MAP(CSplitterImpl<DirectoryListingFrame>)
	ALT_MSG_MAP(STATUS_MESSAGE_MAP)
		COMMAND_ID_HANDLER(IDC_FIND, onFind)
		COMMAND_ID_HANDLER(IDC_NEXT, onNext)
		COMMAND_ID_HANDLER(IDC_MATCH_QUEUE, onMatchQueue)
	END_MSG_MAP()

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onDownload(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onDownloadDir(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onDownloadDirTo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onDownloadTo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onViewAsText(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onGoToDirectory(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onDownloadTarget(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onDownloadTargetDir(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onDoubleClickFiles(int idCtrl, LPNMHDR pnmh, BOOL& bHandled); 
	LRESULT onSelChangedDirectories(int idCtrl, LPNMHDR pnmh, BOOL& bHandled); 
	LRESULT onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled);
	
	void downloadList(const string& aTarget, bool view = false);
	static int sortFile(LPARAM a, LPARAM b);
	static int sortSize(LPARAM a, LPARAM b);
	static int sortType(LPARAM a, LPARAM b);
	void updateTree(DirectoryListing::Directory* tree, HTREEITEM treeItem);
	void UpdateLayout(BOOL bResizeBars = TRUE);
	void findFile(bool findNext);
	
	LRESULT onItemChanged(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/) {
		updateStatus();
		return 0;
	}

	LRESULT onSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /* bHandled */) {
		ctrlList.SetFocus();
		return 0;
	}

	LRESULT onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		ctrlList.SetRedraw(FALSE);
		clearList();
		MDIDestroy(m_hWnd);
		return 0;
	}
	
	void setWindowTitle() {
		if(error.empty())
			SetWindowText(dl->getUser()->getFullNick().c_str());
		else
			SetWindowText(error.c_str());		
	}

	LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		return 1;
	}

	LRESULT onColumnClickFiles(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
		NMLISTVIEW* l = (NMLISTVIEW*)pnmh;
		
		if(l->iSubItem == ctrlList.getSortColumn()) {
			if (!ctrlList.isAscending())
				ctrlList.setSort(-1, ctrlList.getSortType());
			else
				ctrlList.setSortDirection(false);
		} else {
			if(l->iSubItem == COLUMN_FILENAME) {
				ctrlList.setSort(l->iSubItem, ExListViewCtrl::SORT_FUNC, true, sortFile);
			} else if(l->iSubItem == COLUMN_SIZE) {
				ctrlList.setSort(l->iSubItem, ExListViewCtrl::SORT_FUNC, true, sortSize);
			} else if(l->iSubItem == COLUMN_TYPE) {
				ctrlList.setSort(l->iSubItem, ExListViewCtrl::SORT_FUNC, true, sortType);
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

	LRESULT onFind(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		searching = true;
		findFile(false);
		searching = false;
		updateStatus();
		return 0;
	}
	LRESULT onNext(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		searching = true;
		findFile(true);
		searching = false;
		updateStatus();
		return 0;
	}

	LRESULT onMatchQueue(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	LRESULT onKeyDown(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);

	LRESULT onKeyDownDirs(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
		NMTVKEYDOWN* kd = (NMTVKEYDOWN*) pnmh;
		if(kd->wVKey == VK_TAB) {
			onTab();
		}
		return 0;
	}

	void onTab() {
		HWND focus = ::GetFocus();
		if(focus == ctrlTree.m_hWnd) {
			ctrlList.SetFocus();
		} else if(focus == ctrlList.m_hWnd) {
			ctrlTree.SetFocus();
		}
	}
private:
	void changeDir(DirectoryListing::Directory* d, BOOL enableRedraw);
	HTREEITEM findFile(const StringSearch& str, HTREEITEM root, int &foundFile, int &skipHits);
	void updateStatus();
	void GoToDirectory(HTREEITEM hItem, StringList::iterator& iPath, const StringList::iterator& iPathEnd);

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
	CContainedWindow statusContainer;

	StringList targets;
	
	CTreeViewCtrl ctrlTree;
	ExListViewCtrl ctrlList;
	CStatusBarCtrl ctrlStatus;
	HTREEITEM treeRoot;
	
	CButton ctrlFind, ctrlFindNext;
	CButton ctrlMatchQueue;

	string findStr;
	string error;
	string size;

	int skipHits;

	int files;

	bool updating;
	bool searching;

	int statusSizes[8];
	
	DirectoryListing* dl;
};

#endif // !defined(AFX_CHILDFRM_H__A7078724_FD85_4F39_8463_5A08A5F45E33__INCLUDED_)

/**
 * @file
 * $Id: DirectoryListingFrm.h,v 1.23 2003/11/12 21:45:00 arnetheduck Exp $
 */

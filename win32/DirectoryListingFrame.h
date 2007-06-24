/*
 * Copyright (C) 2001-2007 Jacek Sieka, arnetheduck on gmail point com
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

#ifndef DCPLUSPLUS_WIN32_DIRECTORY_LISTING_FRAME_H
#define DCPLUSPLUS_WIN32_DIRECTORY_LISTING_FRAME_H

#include "MDIChildFrame.h"
#include "TypedListViewCtrl.h"
#include "TypedTreeView.h"

#include <client/forward.h>
#include <client/FastAlloc.h>
#include <client/DirectoryListing.h>
#include <client/User.h>

class DirectoryListingFrame : public MDIChildFrame<DirectoryListingFrame> {
public:
	enum Status {
		STATUS_STATUS,
		STATUS_SPEED,
		STATUS_TOTAL_FILES,
		STATUS_TOTAL_SIZE,
		STATUS_SELECTED_FILES,
		STATUS_SELECTED_SIZE,
		STATUS_FILE_LIST_DIFF,
		STATUS_MATCH_QUEUE,
		STATUS_FIND,
		STATUS_NEXT,
		STATUS_DUMMY,
		STATUS_LAST
	};

	static void openWindow(SmartWin::Widget* mdiParent, const tstring& aFile, const tstring& aDir, const User::Ptr& aUser, int64_t aSpeed);
	static void openWindow(SmartWin::Widget* mdiParent, const User::Ptr& aUser, const string& txt, int64_t aSpeed);
	static void closeAll();

protected:
	typedef MDIChildFrame<DirectoryListingFrame> Base;
	friend class MDIChildFrame<DirectoryListingFrame>;
	
	void layout();
	void postClosing();

private:
	enum {
		COLUMN_FILENAME,
		COLUMN_TYPE,
		COLUMN_EXACTSIZE,
		COLUMN_SIZE,
		COLUMN_TTH,
		COLUMN_LAST
	};

	class ItemInfo : public FastAlloc<ItemInfo> {
	public:
		enum ItemType {
			FILE,
			DIRECTORY,
			USER
		} type;

		union {
			DirectoryListing::File* file;
			DirectoryListing::Directory* dir;
		};
		
		ItemInfo(const tstring& nick, DirectoryListing::Directory* d) : type(USER), dir(d) {
			columns[COLUMN_FILENAME] = nick;
		}

		ItemInfo(DirectoryListing::File* f) : type(FILE), file(f) {
			columns[COLUMN_FILENAME] = Text::toT(f->getName());
			columns[COLUMN_TYPE] = Util::getFileExt(columns[COLUMN_FILENAME]);
			if(columns[COLUMN_TYPE].size() > 0 && columns[COLUMN_TYPE][0] == '.')
				columns[COLUMN_TYPE].erase(0, 1);

			columns[COLUMN_EXACTSIZE] = Text::toT(Util::formatExactSize(f->getSize()));
			columns[COLUMN_SIZE] = Text::toT(Util::formatBytes(f->getSize()));
			columns[COLUMN_TTH] = Text::toT(f->getTTH().toBase32());
		}
		ItemInfo(DirectoryListing::Directory* d) : type(DIRECTORY), dir(d) {
			columns[COLUMN_FILENAME] = Text::toT(d->getName());			
			columns[COLUMN_EXACTSIZE] = Text::toT(Util::formatExactSize(d->getTotalSize()));
			columns[COLUMN_SIZE] = Text::toT(Util::formatBytes(d->getTotalSize()));
		}
		
		const tstring& getText() const {
			return columns[COLUMN_FILENAME];
		}
		
		int getImage() const;
		
		int getSelectedImage() const {
			return getImage();
		}

		const tstring& getText(int col) const {
			return columns[col];
		}

		struct TotalSize {
			TotalSize() : total(0) { }
			void operator()(ItemInfo* a) { total += a->type == DIRECTORY ? a->dir->getTotalSize() : a->file->getSize(); }
			int64_t total;
		};

		static int compareItems(ItemInfo* a, ItemInfo* b, int col);
	private:
		tstring columns[COLUMN_LAST];
	};
	
	typedef TypedTreeView<DirectoryListingFrame, ItemInfo> WidgetDirs;
	typedef WidgetDirs* WidgetDirsPtr;
	WidgetDirsPtr dirs;
	typedef TypedListViewCtrl<DirectoryListingFrame, ItemInfo> WidgetFiles;
	typedef WidgetFiles* WidgetFilesPtr;
	
	WidgetFilesPtr files;
	WidgetVPanedPtr paned;

	WidgetButtonPtr find;
	WidgetButtonPtr findNext;
	WidgetButtonPtr listDiff;
	WidgetButtonPtr matchQueue;
	
	int64_t speed;		/**< Speed at which this file list was downloaded */

	std::auto_ptr<DirectoryListing> dl;
	
	std::string error;
	bool usingDirMenu;
	StringList targets;
	deque<string> history;
	size_t historyIndex;

	HTREEITEM treeRoot;

	string findStr;
	string size;

	int skipHits;
	bool updating;
	bool searching;

	static int columnIndexes[COLUMN_LAST];
	static int columnSizes[COLUMN_LAST];

	typedef HASH_MAP_X(UserPtr, DirectoryListingFrame*, User::HashFunction, equal_to<UserPtr>, less<UserPtr>) UserMap;
	typedef UserMap::iterator UserIter;

	static UserMap lists;
	
	DirectoryListingFrame(SmartWin::Widget* mdiParent, const User::Ptr& aUser, int64_t aSpeed);
	virtual ~DirectoryListingFrame();

	WidgetPopupMenuPtr makeSingleMenu(ItemInfo* ii);
	WidgetPopupMenuPtr makeMultiMenu();
	WidgetPopupMenuPtr makeDirMenu();
	
	void addTargets(const WidgetPopupMenuPtr& menu, ItemInfo* ii = 0);
	void addUserCommands(const WidgetPopupMenuPtr& menu);
	
	void handleFind(WidgetButtonPtr);
	void handleFindNext(WidgetButtonPtr);
	void handleListDiff(WidgetButtonPtr);
	void handleMatchQueue(WidgetButtonPtr);
	
	void handleDownload(WidgetMenuPtr, unsigned id);
	void handleViewAsText(WidgetMenuPtr, unsigned id);
	void handleSearchAlternates(WidgetMenuPtr, unsigned id);
	void handleLookupBitzi(WidgetMenuPtr, unsigned id);
	void handleCopyMagnet(WidgetMenuPtr, unsigned id);
	void handleGoToDirectory(WidgetMenuPtr, unsigned id);
	void handleDownloadLastDir(WidgetMenuPtr, unsigned id);
	void handleDownloadTarget(WidgetMenuPtr, unsigned id);
	void handleDownloadFavorite(WidgetMenuPtr, unsigned id);
	void handleDownloadBrowse(WidgetMenuPtr, unsigned id);
	
	void handleSelectionChanged(WidgetTreeViewPtr);
	
	void download(const string& aDir);
	void download(ItemInfo* ii, const string& aDir, bool view = false);
	void downloadFiles(const string& aTarget, bool view = false);
	
	HRESULT handleContextMenu(LPARAM lParam, WPARAM wParam);

	void changeDir(DirectoryListing::Directory* d, BOOL enableRedraw);
	void updateTree(DirectoryListing::Directory* tree, HTREEITEM treeItem);
	HTREEITEM findItem(HTREEITEM ht, const tstring& name);
	void selectItem(const tstring& name);
	void clearList();
	void setWindowTitle();
	
	void loadFile(const tstring& name, const tstring& dir);
	void loadXML(const string& txt);
	void refreshTree(const tstring& root);

	void addHistory(const string& name);
	void up();
	void back();
	void forward();

	void initStatus();
	void updateStatus();
	
	void findFile(bool findNext);
	HTREEITEM findFile(const StringSearch& str, HTREEITEM root, int &foundFile, int &skipHits);

};


#ifdef PORT_ME

#include "FlatTabCtrl.h"
#include "WinUtil.h"
#include "UCHandler.h"

#include "../client/StringSearch.h"
#include "../client/FavoriteManager.h"

#define STATUS_MESSAGE_MAP 9
#define CONTROL_MESSAGE_MAP 10
class DirectoryListingFrame : public MDITabChildWindowImpl<DirectoryListingFrame, RGB(255, 0, 255)>,
	public CSplitterImpl<DirectoryListingFrame>, public UCHandler<DirectoryListingFrame>

{
public:

	typedef MDITabChildWindowImpl<DirectoryListingFrame, RGB(255, 0, 255)> baseClass;
	typedef UCHandler<DirectoryListingFrame> ucBase;


	DECLARE_FRAME_WND_CLASS(_T("DirectoryListingFrame"), IDR_DIRECTORY)

	BEGIN_MSG_MAP(DirectoryListingFrame)
		NOTIFY_HANDLER(IDC_FILES, LVN_GETDISPINFO, ctrlList.onGetDispInfo)
		NOTIFY_HANDLER(IDC_FILES, LVN_COLUMNCLICK, ctrlList.onColumnClick)
		NOTIFY_HANDLER(IDC_FILES, LVN_KEYDOWN, onKeyDown)
		NOTIFY_HANDLER(IDC_FILES, NM_DBLCLK, onDoubleClickFiles)
		NOTIFY_HANDLER(IDC_FILES, LVN_ITEMCHANGED, onItemChanged)
		NOTIFY_HANDLER(IDC_DIRECTORIES, TVN_KEYDOWN, onKeyDownDirs)
		NOTIFY_HANDLER(IDC_DIRECTORIES, TVN_SELCHANGED, onSelChangedDirectories)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_CONTEXTMENU, onContextMenu)
		MESSAGE_HANDLER(WM_CLOSE, onClose)
		MESSAGE_HANDLER(WM_SETFOCUS, onSetFocus)
		COMMAND_ID_HANDLER(IDC_DOWNLOAD, onDownload)
		COMMAND_ID_HANDLER(IDC_DOWNLOADDIR, onDownloadDir)
		COMMAND_ID_HANDLER(IDC_DOWNLOADDIRTO, onDownloadDirTo)
		COMMAND_ID_HANDLER(IDC_DOWNLOADTO, onDownloadTo)
		COMMAND_ID_HANDLER(IDC_GO_TO_DIRECTORY, onGoToDirectory)
		COMMAND_ID_HANDLER(IDC_VIEW_AS_TEXT, onViewAsText)
		COMMAND_ID_HANDLER(IDC_SEARCH_ALTERNATES, onSearchByTTH)
		COMMAND_ID_HANDLER(IDC_BITZI_LOOKUP, onBitziLookup)
		COMMAND_ID_HANDLER(IDC_COPY_MAGNET, onCopyMagnet)
		COMMAND_RANGE_HANDLER(IDC_DOWNLOAD_TARGET, IDC_DOWNLOAD_TARGET + targets.size() + WinUtil::lastDirs.size(), onDownloadTarget)
		COMMAND_RANGE_HANDLER(IDC_DOWNLOAD_TARGET_DIR, IDC_DOWNLOAD_TARGET_DIR + WinUtil::lastDirs.size(), onDownloadTargetDir)
		COMMAND_RANGE_HANDLER(IDC_DOWNLOAD_FAVORITE_DIRS, IDC_DOWNLOAD_FAVORITE_DIRS + FavoriteManager::getInstance()->getFavoriteDirs().size(), onDownloadFavoriteDirs)
		COMMAND_RANGE_HANDLER(IDC_DOWNLOAD_WHOLE_FAVORITE_DIRS, IDC_DOWNLOAD_WHOLE_FAVORITE_DIRS + FavoriteManager::getInstance()->getFavoriteDirs().size(), onDownloadWholeFavoriteDirs)
		CHAIN_COMMANDS(ucBase)
		CHAIN_MSG_MAP(baseClass)
		CHAIN_MSG_MAP(CSplitterImpl<DirectoryListingFrame>)
	ALT_MSG_MAP(STATUS_MESSAGE_MAP)
		COMMAND_ID_HANDLER(IDC_FIND, onFind)
		COMMAND_ID_HANDLER(IDC_NEXT, onNext)
		COMMAND_ID_HANDLER(IDC_MATCH_QUEUE, onMatchQueue)
		COMMAND_ID_HANDLER(IDC_FILELIST_DIFF, onListDiff)
	ALT_MSG_MAP(CONTROL_MESSAGE_MAP)
		MESSAGE_HANDLER(WM_XBUTTONUP, onXButtonUp)
	END_MSG_MAP()

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onDownload(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onDownloadDir(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onDownloadDirTo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onDownloadTo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onViewAsText(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onSearchByTTH(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onBitziLookup(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onCopyMagnet(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onGoToDirectory(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onDownloadTarget(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onDownloadTargetDir(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onDoubleClickFiles(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT onSelChangedDirectories(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled);
	LRESULT onXButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled);
	LRESULT onDownloadFavoriteDirs(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onDownloadWholeFavoriteDirs(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	void runUserCommand(UserCommand& uc);

	LRESULT onItemChanged(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/) {
		updateStatus();
		return 0;
	}

	LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		return 1;
	}
	LRESULT onKeyDown(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);

	LRESULT onKeyDownDirs(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
		NMTVKEYDOWN* kd = (NMTVKEYDOWN*) pnmh;
		if(kd->wVKey == VK_TAB) {
			onTab();
		}
		return 0;
	}

private:

	StringMap ucLineParams;

};

#endif // !defined(DIRECTORY_LISTING_FRM_H)
#endif

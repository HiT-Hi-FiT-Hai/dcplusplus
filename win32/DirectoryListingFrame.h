/*
 * Copyright (C) 2001-2008 Jacek Sieka, arnetheduck on gmail point com
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
#include "TypedTable.h"
#include "TypedTree.h"
#include "AspectUserCommand.h"

#include <dcpp/forward.h>
#include <dcpp/FastAlloc.h>
#include <dcpp/DirectoryListing.h>
#include <dcpp/User.h>

class DirectoryListingFrame : 
	public MDIChildFrame<DirectoryListingFrame>,
	public AspectUserCommand<DirectoryListingFrame>
{
	typedef MDIChildFrame<DirectoryListingFrame> BaseType;
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
		STATUS_LAST
	};

	static void openWindow(dwt::TabView* mdiParent, const tstring& aFile, const tstring& aDir, const UserPtr& aUser, int64_t aSpeed);
	static void openWindow(dwt::TabView* mdiParent, const UserPtr& aUser, const string& txt, int64_t aSpeed);
	static void closeAll();

protected:
	friend class MDIChildFrame<DirectoryListingFrame>;
	friend class AspectUserCommand<DirectoryListingFrame>;
	
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
			columns[COLUMN_EXACTSIZE] = d->getComplete() ? Text::toT(Util::formatExactSize(d->getTotalSize())) : _T("?");
			columns[COLUMN_SIZE] = d->getComplete() ? Text::toT(Util::formatBytes(d->getTotalSize())) : _T("?");
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
	
	typedef TypedTree<ItemInfo> WidgetDirs;
	typedef WidgetDirs* WidgetDirsPtr;
	WidgetDirsPtr dirs;
	typedef TypedTable<ItemInfo> WidgetFiles;
	typedef WidgetFiles* WidgetFilesPtr;
	
	WidgetFilesPtr files;
	WidgetVPanedPtr paned;

	ButtonPtr find;
	ButtonPtr findNext;
	ButtonPtr listDiff;
	ButtonPtr matchQueue;
	
	int64_t speed;		/**< Speed at which this file list was downloaded */

	std::auto_ptr<DirectoryListing> dl;
	
	tstring error;
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

	StringMap ucLineParams;

	static int columnIndexes[COLUMN_LAST];
	static int columnSizes[COLUMN_LAST];

	typedef unordered_map<UserPtr, DirectoryListingFrame*, User::Hash> UserMap;
	typedef UserMap::iterator UserIter;

	static UserMap lists;
	
	DirectoryListingFrame(dwt::TabView* mdiParent, const UserPtr& aUser, int64_t aSpeed);
	virtual ~DirectoryListingFrame();

	MenuPtr makeSingleMenu(ItemInfo* ii);
	MenuPtr makeMultiMenu();
	MenuPtr makeDirMenu();
	
	void runUserCommand(const UserCommand& uc);
		
	void addTargets(const MenuPtr& menu, ItemInfo* ii = 0);
	void addUserCommands(const MenuPtr& menu);
	
	void handleFind();
	void handleFindNext();
	void handleListDiff();
	void handleMatchQueue();
	
	void handleDownload();
	void handleViewAsText();
	void handleGoToDirectory();
	void handleDownloadLastDir(unsigned id);
	void handleDownloadTarget(unsigned id);
	void handleDownloadFavorite(unsigned id);
	void handleDownloadBrowse();
	bool handleKeyDownFiles(int c);
	
	void handleDoubleClickFiles();
	void handleSelectionChanged();
	
	void download(const string& aDir);
	void download(ItemInfo* ii, const string& aDir, bool view = false);
	void downloadFiles(const string& aTarget, bool view = false);
	
	bool handleDirsContextMenu(dwt::ScreenCoordinate pt);
	bool handleFilesContextMenu(dwt::ScreenCoordinate pt);
	LRESULT handleXButtonUp(WPARAM wParam, LPARAM lParam);
	
	void changeDir(DirectoryListing::Directory* d);
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

	void initStatusText();
	void updateStatus();
	
	void findFile(bool findNext);
	HTREEITEM findFile(const StringSearch& str, HTREEITEM root, int &foundFile, int &skipHits);

};

#endif

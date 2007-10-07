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

#include "stdafx.h"

#include "DirectoryListingFrame.h"
#include "LineDlg.h"
#include "HoldRedraw.h"
#include "ShellContextMenu.h"

#include "resource.h"

#include <dcpp/ResourceManager.h>
#include <dcpp/ADLSearch.h>
#include <dcpp/FavoriteManager.h>
#include <dcpp/File.h>
#include <dcpp/QueueManager.h>
#include <dcpp/StringSearch.h>
#include <dcpp/ClientManager.h>
#include <dcpp/ShareManager.h>

int DirectoryListingFrame::columnIndexes[] = { COLUMN_FILENAME, COLUMN_TYPE, COLUMN_EXACTSIZE, COLUMN_SIZE, COLUMN_TTH };
int DirectoryListingFrame::columnSizes[] = { 300, 60, 100, 100, 200 };

static ResourceManager::Strings columnNames[] = { ResourceManager::FILE, ResourceManager::TYPE, ResourceManager::EXACT_SIZE, ResourceManager::SIZE, ResourceManager::TTH_ROOT };

DirectoryListingFrame::UserMap DirectoryListingFrame::lists;

int DirectoryListingFrame::ItemInfo::getImage() const {
	if(type == DIRECTORY || type == USER) {
		return dir->getComplete() ? WinUtil::getDirIconIndex() : WinUtil::getDirMaskedIndex();
	}
	
	return WinUtil::getIconIndex(getText(COLUMN_FILENAME));
}

int DirectoryListingFrame::ItemInfo::compareItems(ItemInfo* a, ItemInfo* b, int col) {
	if(a->type == DIRECTORY) {
		if(b->type == DIRECTORY) {
			switch(col) {
			case COLUMN_EXACTSIZE: return compare(a->dir->getTotalSize(), b->dir->getTotalSize());
			case COLUMN_SIZE: return compare(a->dir->getTotalSize(), b->dir->getTotalSize());
			default: return lstrcmpi(a->columns[col].c_str(), b->columns[col].c_str());
			}
		} else {
			return -1;
		}
	} else if(b->type == DIRECTORY) {
		return 1;
	} else {
		switch(col) {
		case COLUMN_EXACTSIZE: return compare(a->file->getSize(), b->file->getSize());
		case COLUMN_SIZE: return compare(a->file->getSize(), b->file->getSize());
		default: return lstrcmp(a->columns[col].c_str(), b->columns[col].c_str());
		}
	}
}

void DirectoryListingFrame::openWindow(SmartWin::WidgetTabView* mdiParent, const tstring& aFile, const tstring& aDir, const UserPtr& aUser, int64_t aSpeed) {
	UserIter i = lists.find(aUser);
	if(i != lists.end()) {
		i->second->speed = aSpeed;
		if(!BOOLSETTING(POPUNDER_FILELIST)) {
			//i->second->activate();
		}
	} else {
		DirectoryListingFrame* frame = new DirectoryListingFrame(mdiParent, aUser, aSpeed);
		frame->loadFile(aFile, aDir);
	}
}

void DirectoryListingFrame::closeAll(){
	for(UserIter i = lists.begin(); i != lists.end(); ++i)
		::PostMessage(i->second->handle(), WM_CLOSE, 0, 0);
}

void DirectoryListingFrame::openWindow(SmartWin::WidgetTabView* mdiParent, const UserPtr& aUser, const string& txt, int64_t aSpeed) {
	UserIter i = lists.find(aUser);
	if(i != lists.end()) {
		i->second->speed = aSpeed;
		i->second->loadXML(txt);
	} else {
		DirectoryListingFrame* frame = new DirectoryListingFrame(mdiParent, aUser, aSpeed);
		frame->loadXML(txt);
	}
}

DirectoryListingFrame::DirectoryListingFrame(SmartWin::WidgetTabView* mdiParent, const UserPtr& aUser, int64_t aSpeed) :
	BaseType(mdiParent, _T(""), SmartWin::IconPtr(new SmartWin::Icon(IDR_DIRECTORY)), !BOOLSETTING(POPUNDER_FILELIST)),
	dirs(0),
	files(0),
	paned(0),
	find(0),
	findNext(0),
	listDiff(0),
	matchQueue(0),
	speed(aSpeed),
	dl(new DirectoryListing(aUser)),
	usingDirMenu(false),
	historyIndex(0),
	treeRoot(NULL),
	skipHits(0),
	updating(false),
	searching(false)
{
	paned = createVPaned();
	paned->setRelativePos(0.3);

	{
		WidgetTreeView::Seed cs;
		
		cs.style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_HASLINES | TVS_SHOWSELALWAYS | TVS_DISABLEDRAGDROP;
		cs.exStyle = WS_EX_CLIENTEDGE;
		dirs = SmartWin::WidgetCreator<WidgetDirs>::create(this, cs);
		addWidget(dirs);
		paned->setFirst(dirs);
		dirs->setColor(WinUtil::textColor, WinUtil::bgColor);
		dirs->setNormalImageList(WinUtil::fileImages);
		dirs->onSelectionChanged(std::tr1::bind(&DirectoryListingFrame::handleSelectionChanged, this));
	}
	
	{
		WidgetFiles::Seed cs;
		cs.style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_NOSORTHEADER | LVS_SHAREIMAGELISTS;
		cs.exStyle = WS_EX_CLIENTEDGE;
		files = SmartWin::WidgetCreator<WidgetFiles>::create(this, cs);
		files->setListViewStyle(LVS_EX_LABELTIP | LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT);
		files->setFont(WinUtil::font);
		addWidget(files);
		paned->setSecond(files);

		files->setSmallImageList(WinUtil::fileImages);
		files->createColumns(ResourceManager::getInstance()->getStrings(columnNames));
		files->setColumnOrder(WinUtil::splitTokens(SETTING(QUEUEFRAME_ORDER), columnIndexes));
		files->setColumnWidths(WinUtil::splitTokens(SETTING(QUEUEFRAME_WIDTHS), columnSizes));
		files->setColor(WinUtil::textColor, WinUtil::bgColor);
		files->setSortColumn(COLUMN_FILENAME);
		
		files->onSelectionChanged(std::tr1::bind(&DirectoryListingFrame::updateStatus, this));
		files->onDblClicked(std::tr1::bind(&DirectoryListingFrame::handleDoubleClickFiles, this));
		files->onKeyDown(std::tr1::bind(&DirectoryListingFrame::handleKeyDownFiles, this, _1));
	}
	
	{
		WidgetButton::Seed cs;
		cs.style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON;
		
		cs.caption = TSTRING(FIND);
		find = createButton(cs);
		find->onClicked(std::tr1::bind(&DirectoryListingFrame::handleFind, this));
		
		cs.caption = TSTRING(NEXT);
		findNext = createButton(cs);
		findNext->onClicked(std::tr1::bind(&DirectoryListingFrame::handleFindNext, this));

		cs.caption = TSTRING(MATCH_QUEUE);
		matchQueue = createButton(cs);
		matchQueue->onClicked(std::tr1::bind(&DirectoryListingFrame::handleMatchQueue, this));
		
		cs.caption = TSTRING(FILE_LIST_DIFF);
		listDiff = createButton(cs);
		listDiff->onClicked(std::tr1::bind(&DirectoryListingFrame::handleListDiff, this));
	}
	
	initStatus();
	
	// This will set the widths correctly
	setStatus(STATUS_FILE_LIST_DIFF, TSTRING(FILE_LIST_DIFF));
	setStatus(STATUS_MATCH_QUEUE, TSTRING(MATCH_QUEUE));
	setStatus(STATUS_FIND, TSTRING(FIND));
	setStatus(STATUS_NEXT, TSTRING(NEXT));

	onRaw(std::tr1::bind(&DirectoryListingFrame::handleContextMenu, this, _1, _2), SmartWin::Message(WM_CONTEXTMENU));
	files->onRaw(std::tr1::bind(&DirectoryListingFrame::handleXButtonUp, this, _1, _2), SmartWin::Message(WM_XBUTTONUP));
	dirs->onRaw(std::tr1::bind(&DirectoryListingFrame::handleXButtonUp, this, _1, _2), SmartWin::Message(WM_XBUTTONUP));
	string nick = ClientManager::getInstance()->getNicks(dl->getUser()->getCID())[0];
	treeRoot = dirs->insert(NULL, new ItemInfo(Text::toT(nick), dl->getRoot()));

	setWindowTitle();

	layout();
	
	lists.insert(std::make_pair(aUser, this));
}

DirectoryListingFrame::~DirectoryListingFrame() {
	dcassert(lists.find(dl->getUser()) != lists.end());
	lists.erase(dl->getUser());
}

void DirectoryListingFrame::loadFile(const tstring& name, const tstring& dir) {
	try {
		dl->loadFile(Text::fromT(name));
		ADLSearchManager::getInstance()->matchListing(*dl);
		refreshTree(dir);
	} catch(const Exception& e) {
		error = WinUtil::getNicks(dl->getUser()) + Text::toT(": " + e.getError());
	}

	initStatus();
}

void DirectoryListingFrame::loadXML(const string& txt) {
	try {
		refreshTree(Text::toT(Util::toNmdcFile(dl->loadXML(txt, true))));
	} catch(const Exception& e) {
		error = WinUtil::getNicks(dl->getUser()) + Text::toT(": " + e.getError());
	}

	initStatus();
}


void DirectoryListingFrame::initStatusText() {
	setStatus(STATUS_TOTAL_FILES, Text::toT(STRING(FILES) + ": " + Util::toString(dl->getTotalFileCount(true))));
	setStatus(STATUS_TOTAL_SIZE, Text::toT(STRING(SIZE) + ": " + Util::formatBytes(dl->getTotalSize(true))));
	setStatus(STATUS_SPEED, Text::toT(STRING(SPEED) + ": " + Util::formatBytes(speed) + "/s"));
}

void DirectoryListingFrame::layout() {
	SmartWin::Rectangle r(getClientAreaSize()); 

	layoutStatus(r);

	mapWidget(STATUS_FILE_LIST_DIFF, listDiff);
	mapWidget(STATUS_MATCH_QUEUE, matchQueue);
	mapWidget(STATUS_FIND, find);
	mapWidget(STATUS_NEXT, findNext);
	
	paned->setRect(r);
}

void DirectoryListingFrame::postClosing() {
	clearList();

	SettingsManager::getInstance()->set(SettingsManager::DIRECTORLISTINGFRAME_ORDER, WinUtil::toString(files->getColumnOrder()));
	SettingsManager::getInstance()->set(SettingsManager::DIRECTORLISTINGFRAME_WIDTHS, WinUtil::toString(files->getColumnWidths()));
}

void DirectoryListingFrame::handleFind() {
	searching = true;
	findFile(false);
	searching = false;
	updateStatus();
}

void DirectoryListingFrame::handleFindNext() {
	searching = true;
	findFile(true);
	searching = false;
	updateStatus();
}

void DirectoryListingFrame::handleMatchQueue() {
	int matched = QueueManager::getInstance()->matchListing(*dl);
	setStatus(STATUS_STATUS, Text::tformat(TSTRING(MATCHED_FILES), matched));
}

void DirectoryListingFrame::handleListDiff() {
	tstring file;
	if(WinUtil::browseFile(file, handle(), false, Text::toT(Util::getListPath()), _T("File Lists\0*.xml.bz2\0All Files\0*.*\0"))) {
		DirectoryListing dirList(dl->getUser());
		try {
			dirList.loadFile(Text::fromT(file));
			dl->getRoot()->filterList(dirList);
			refreshTree(Util::emptyStringT);
			initStatus();
			updateStatus();
		} catch(const Exception&) {
			/// @todo report to user?
		}
	}
}

void DirectoryListingFrame::refreshTree(const tstring& root) {
	HoldRedraw hold(dirs);
	
	HTREEITEM ht = findItem(treeRoot, root);
	if(ht == NULL) {
		ht = treeRoot;
	}

	DirectoryListing::Directory* d = dirs->getData(ht)->dir;

	HTREEITEM next = NULL;
	while((next = dirs->getChild(ht)) != NULL) {
		dirs->erase(next);
	}

	updateTree(d, ht);
	
	dirs->select(NULL);
	selectItem(root);

	dirs->expand(treeRoot);
}

void DirectoryListingFrame::setWindowTitle() {
	if(error.empty())
		setText(WinUtil::getNicks(dl->getUser()) + _T(" - ") + WinUtil::getHubNames(dl->getUser()).first);
	else
		setText(error);
}

DirectoryListingFrame::WidgetMenuPtr DirectoryListingFrame::makeSingleMenu(ItemInfo* ii) {
	WidgetMenuPtr menu = createMenu(true);
	
	menu->appendItem(IDC_DOWNLOAD, TSTRING(DOWNLOAD), std::tr1::bind(&DirectoryListingFrame::handleDownload, this));
	addTargets(menu, ii);
	
	if(ii->type == ItemInfo::FILE) {
		menu->appendItem(IDC_VIEW_AS_TEXT, TSTRING(VIEW_AS_TEXT), std::tr1::bind(&DirectoryListingFrame::handleViewAsText, this));
		
		menu->appendSeparatorItem();
		
		menu->appendItem(IDC_SEARCH_ALTERNATES, TSTRING(SEARCH_FOR_ALTERNATES), std::tr1::bind(&DirectoryListingFrame::handleSearchAlternates, this));
		menu->appendItem(IDC_BITZI_LOOKUP, TSTRING(LOOKUP_AT_BITZI), std::tr1::bind(&DirectoryListingFrame::handleLookupBitzi, this));
		menu->appendItem(IDC_COPY_MAGNET, TSTRING(COPY_MAGNET), std::tr1::bind(&DirectoryListingFrame::handleCopyMagnet, this));
	}

	if((ii->type == ItemInfo::FILE && ii->file->getAdls()) ||
		(ii->type == ItemInfo::DIRECTORY && ii->dir->getAdls() && ii->dir->getParent() != dl->getRoot()) )	{
		menu->appendSeparatorItem();
		menu->appendItem(IDC_GO_TO_DIRECTORY, TSTRING(GO_TO_DIRECTORY), std::tr1::bind(&DirectoryListingFrame::handleGoToDirectory, this));
	}
	
	addUserCommands(menu);
	menu->setDefaultItem(IDC_DOWNLOAD);
	return menu;
}

DirectoryListingFrame::WidgetMenuPtr DirectoryListingFrame::makeMultiMenu() {
	WidgetMenuPtr menu = createMenu(true);
	
	menu->appendItem(IDC_DOWNLOAD, TSTRING(DOWNLOAD), std::tr1::bind(&DirectoryListingFrame::handleDownload, this));
	addTargets(menu);
	addUserCommands(menu);
	
	menu->setDefaultItem(IDC_DOWNLOAD);

	return menu;
}

DirectoryListingFrame::WidgetMenuPtr DirectoryListingFrame::makeDirMenu() {
	WidgetMenuPtr menu = createMenu(true);
	
	menu->appendItem(IDC_DOWNLOAD, TSTRING(DOWNLOAD), std::tr1::bind(&DirectoryListingFrame::handleDownload, this));
	addTargets(menu);
	return menu;
}

void DirectoryListingFrame::addUserCommands(const WidgetMenuPtr& parent) {
	prepareMenu(parent, UserCommand::CONTEXT_FILELIST, ClientManager::getInstance()->getHubs(dl->getUser()->getCID()));
}

void DirectoryListingFrame::addTargets(const WidgetMenuPtr& parent, ItemInfo* ii) {
	WidgetMenuPtr menu = parent->appendPopup(TSTRING(DOWNLOAD_TO));
	
	StringPairList spl = FavoriteManager::getInstance()->getFavoriteDirs();
	size_t i = 0;
	for(; i < spl.size(); ++i) {
		menu->appendItem(IDC_DOWNLOAD_FAVORITE_DIRS + i, Text::toT(spl[i].second), std::tr1::bind(&DirectoryListingFrame::handleDownloadFavorite, this, _1));
	}
	
	if(i > 0) {
		menu->appendSeparatorItem();
	}

	menu->appendItem(IDC_DOWNLOAD_BROWSE, TSTRING(BROWSE), std::tr1::bind(&DirectoryListingFrame::handleDownloadBrowse, this));

	targets.clear();
	
	if(ii && ii->type == ItemInfo::FILE) {
		QueueManager::getInstance()->getTargets(ii->file->getTTH(), targets);
		if(!targets.empty()) {
			menu->appendSeparatorItem();
			for(i = 0; i < targets.size(); ++i) {
				menu->appendItem(IDC_DOWNLOAD_TARGET + i, Text::toT(targets[i]), std::tr1::bind(&DirectoryListingFrame::handleDownloadTarget, this, _1));
			}
		}
	}
	
	if(WinUtil::lastDirs.size() > 0) {
		menu->appendSeparatorItem();
		
		for(i = 0; i < WinUtil::lastDirs.size(); ++i) {
			menu->appendItem(IDC_DOWNLOAD_LASTDIR + i, WinUtil::lastDirs[i], std::tr1::bind(&DirectoryListingFrame::handleDownloadLastDir, this, _1));
		}
	}
}

LRESULT DirectoryListingFrame::handleContextMenu(WPARAM wParam, LPARAM lParam) {
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
	
	WidgetMenuPtr contextMenu;
	if(reinterpret_cast<HWND>(wParam) == files->handle() && files->hasSelection()) {
		if(pt.x == -1 && pt.y == -1) {
			pt = files->getContextMenuPos();
		}
		
		if(files->getSelectedCount() == 1) {
			if(BOOLSETTING(SHOW_SHELL_MENU) && (dl->getUser() == ClientManager::getInstance()->getMe())) {
				string path;
				ItemInfo* ii = files->getSelectedData();
				try {
					path = ShareManager::getInstance()->toReal(Util::toAdcFile(dl->getPath(ii->file) + ii->file->getName()));
				} catch(const ShareException&) {
					// Ignore
				}
				if(!path.empty() && (File::getSize(path) != -1)) {
					WidgetMenuPtr menu = this->createMenu(true);
					CShellContextMenu shellMenu;
					shellMenu.SetPath(Text::utf8ToWide(path));
					shellMenu.ShowContextMenu(menu, this, pt);
					return TRUE;
				}
			}
			ItemInfo* ii = files->getSelectedData();
			
			contextMenu = makeSingleMenu(ii);
		} else {
			contextMenu = makeMultiMenu();
		}
		usingDirMenu = false;
		contextMenu->trackPopupMenu(this, pt.x, pt.y, TPM_LEFTALIGN | TPM_RIGHTBUTTON);
		return TRUE;
	} else if(reinterpret_cast<HWND>(wParam) == dirs->handle()) {
		if(dirs->getSelection() == NULL) {
			return FALSE;
		}

		if(pt.x == -1 && pt.y == -1) {
			pt = dirs->getContextMenuPos();
		} else {
			dirs->select(pt);
		}

		contextMenu = makeDirMenu();
		usingDirMenu = true;
		
		contextMenu->trackPopupMenu(this, pt.x, pt.y, TPM_LEFTALIGN | TPM_RIGHTBUTTON);

		return TRUE;
	}

	return FALSE;
}

void DirectoryListingFrame::downloadFiles(const string& aTarget, bool view /* = false */) {
	int i=-1;
	
	while( (i = files->getNext(i, LVNI_SELECTED)) != -1) {
		download(files->getData(i), aTarget, view);
	}
}

void DirectoryListingFrame::download(ItemInfo* ii, const string& dir, bool view) {
	try {
		if(ii->type == ItemInfo::FILE) {
			if(view) {
				File::deleteFile(dir + Util::validateFileName(ii->file->getName()));
			}
			dl->download(ii->file, dir + Text::fromT(ii->getText(COLUMN_FILENAME)), view, WinUtil::isShift() || view);
		} else if(!view) {
			dl->download(ii->dir, dir, WinUtil::isShift());
		}
			
	} catch(const Exception& e) {
		setStatus(STATUS_STATUS, Text::toT(e.getError()));
	}
}

void DirectoryListingFrame::handleSearchAlternates() {
	ItemInfo* ii = files->getSelectedData();
	if(ii != NULL && ii->type == ItemInfo::FILE) {
		WinUtil::searchHash(ii->file->getTTH());
	}
}

void DirectoryListingFrame::handleLookupBitzi() {
	ItemInfo* ii = files->getSelectedData();
	if(ii != NULL && ii->type == ItemInfo::FILE) {
		WinUtil::bitziLink(ii->file->getTTH());
	}
}

void DirectoryListingFrame::handleCopyMagnet() {
	ItemInfo* ii = files->getSelectedData();
	if(ii != NULL && ii->type == ItemInfo::FILE) {
		WinUtil::copyMagnet(ii->file->getTTH(), ii->getText(COLUMN_FILENAME));
	}
}

void DirectoryListingFrame::handleDownload() {
	if(usingDirMenu) {
		ItemInfo* ii = dirs->getSelectedData();
		if(ii) {
			download(ii, SETTING(DOWNLOAD_DIRECTORY));
		}
	} else {
		downloadFiles(SETTING(DOWNLOAD_DIRECTORY));
	}
}

void DirectoryListingFrame::handleDownloadBrowse() {
	if(usingDirMenu) {
		ItemInfo* ii = dirs->getSelectedData();
		if(ii) {
			tstring target = Text::toT(SETTING(DOWNLOAD_DIRECTORY));
			if(WinUtil::browseDirectory(target, handle())) {
				WinUtil::addLastDir(target);
				download(ii, Text::fromT(target));
			}
		}
	} else {
		if(files->getSelectedCount() == 1) {
			ItemInfo* ii = files->getSelectedData();
			try {
				if(ii->type == ItemInfo::FILE) {
					tstring target = Text::toT(SETTING(DOWNLOAD_DIRECTORY)) + ii->getText(COLUMN_FILENAME);
					if(WinUtil::browseFile(target, handle())) {
						WinUtil::addLastDir(Util::getFilePath(target));
						dl->download(ii->file, Text::fromT(target), false, WinUtil::isShift());
					}
				} else {
					tstring target = Text::toT(SETTING(DOWNLOAD_DIRECTORY));
					if(WinUtil::browseDirectory(target, handle())) {
						WinUtil::addLastDir(target);
						dl->download(ii->dir, Text::fromT(target), WinUtil::isShift());
					}
				}
			} catch(const Exception& e) {
				setStatus(STATUS_STATUS, Text::toT(e.getError()).c_str());
			}
		} else {
			tstring target = Text::toT(SETTING(DOWNLOAD_DIRECTORY));
			if(WinUtil::browseDirectory(target, handle())) {
				WinUtil::addLastDir(target);
				downloadFiles(Text::fromT(target));
			}
		}
	}
}

void DirectoryListingFrame::handleDownloadLastDir(unsigned id) {
	size_t n = id - IDC_DOWNLOAD_LASTDIR;
	if(n >= WinUtil::lastDirs.size()) {
		return;
	}
	download(Text::fromT(WinUtil::lastDirs[n]));
}

void DirectoryListingFrame::handleDownloadFavorite(unsigned id) {
	size_t n = id - IDC_DOWNLOAD_FAVORITE_DIRS;
	StringPairList spl = FavoriteManager::getInstance()->getFavoriteDirs();
	if(n >= spl.size()) {
		return;
	}
	download(spl[n].first);
}

void DirectoryListingFrame::handleDownloadTarget(unsigned id) {
	size_t n = id - IDC_DOWNLOAD_TARGET;
	if(n >= targets.size()) {
		return;
	}
	
	if(files->getSelectedCount() != 1) {
		return;
	}

	const string& target = targets[n];
	ItemInfo* ii = files->getSelectedData();
	try {
		dl->download(ii->file, target, false, WinUtil::isShift());
	} catch(const Exception& e) {
		setStatus(STATUS_STATUS, Text::toT(e.getError()).c_str());
	}
}


void DirectoryListingFrame::handleGoToDirectory() {
	if(files->getSelectedCount() != 1)
		return;

	tstring fullPath;
	ItemInfo* ii = files->getSelectedData();
	
	if(ii->type == ItemInfo::FILE) {
		if(!ii->file->getAdls())
			return;
		
		DirectoryListing::Directory* pd = ii->file->getParent();
		while(pd != NULL && pd != dl->getRoot()) {
			fullPath = Text::toT(pd->getName()) + _T("\\") + fullPath;
			pd = pd->getParent();
		}
	} else if(ii->type == ItemInfo::DIRECTORY) {
		if(!(ii->dir->getAdls() && ii->dir->getParent() != dl->getRoot()))
			return;
		fullPath = Text::toT(((DirectoryListing::AdlDirectory*)ii->dir)->getFullPath());
	}

	selectItem(fullPath);
}

void DirectoryListingFrame::download(const string& target) {
	if(usingDirMenu) {
		ItemInfo* ii = dirs->getSelectedData();
		if(ii) {
			download(ii, target);
		}
	} else {
		if(files->getSelectedCount() == 1) {
			ItemInfo* ii = files->getSelectedData();
			download(ii, target);
		} else {
			downloadFiles(target);
		}
	}
}

void DirectoryListingFrame::handleViewAsText() {
	downloadFiles(Util::getTempPath(), true);
}

HTREEITEM DirectoryListingFrame::findItem(HTREEITEM ht, const tstring& name) {
	string::size_type i = name.find(_T('\\'));
	if(i == string::npos)
		return ht;

	for(HTREEITEM child = dirs->getChild(ht); child != NULL; child = dirs->getNextSibling(child)) {
		DirectoryListing::Directory* d = dirs->getData(child)->dir;
		if(Text::toT(d->getName()) == name.substr(0, i)) {
			return findItem(child, name.substr(i+1));
		}
	}
	return NULL;
}

void DirectoryListingFrame::selectItem(const tstring& name) {
	HTREEITEM ht = findItem(treeRoot, name);
	if(ht != NULL) {
		dirs->ensureVisible(ht);
		dirs->select(ht);
	}
}

void DirectoryListingFrame::updateTree(DirectoryListing::Directory* aTree, HTREEITEM aParent) {
	for(DirectoryListing::Directory::Iter i = aTree->directories.begin(); i != aTree->directories.end(); ++i) {
		HTREEITEM ht = dirs->insert(aParent, new ItemInfo(*i));
		if((*i)->getAdls())
			dirs->setItemState(ht, TVIS_BOLD, TVIS_BOLD);
		updateTree(*i, ht);
	}
}

void DirectoryListingFrame::updateStatus() {
	if(!searching && !updating) {
		int cnt = files->getSelectedCount();
		int64_t total = 0;
		if(cnt == 0) {
			cnt = files->size();
			total = files->forEachT(ItemInfo::TotalSize()).total;
		} else {
			total = files->forEachSelectedT(ItemInfo::TotalSize()).total;
		}

		tstring tmp = Text::toT(STRING(ITEMS) + ": " + Util::toString(cnt));
		setStatus(STATUS_SELECTED_FILES, tmp.c_str());

		tmp = Text::toT(STRING(SIZE) + ": " + Util::formatBytes(total));
		setStatus(STATUS_SELECTED_SIZE, tmp.c_str());
	}
}

void DirectoryListingFrame::handleSelectionChanged() {
	
	DirectoryListing::Directory* d = dirs->getSelectedData()->dir;
	if(d == 0) {
		return;
	}
	HoldRedraw hold(files);
	changeDir(d);
	addHistory(dl->getPath(d));
}

void DirectoryListingFrame::changeDir(DirectoryListing::Directory* d) {

	updating = true;
	clearList();

	for(DirectoryListing::Directory::Iter i = d->directories.begin(); i != d->directories.end(); ++i) {
		files->insert(files->size(), new ItemInfo(*i));
	}
	for(DirectoryListing::File::Iter j = d->files.begin(); j != d->files.end(); ++j) {
		ItemInfo* ii = new ItemInfo(*j);
		files->insert(files->size(), ii);
	}
	files->resort();
	
	updating = false;
	updateStatus();

	if(!d->getComplete()) {
		if(dl->getUser()->isOnline()) {
			try {
				QueueManager::getInstance()->addPfs(dl->getUser(), dl->getPath(d));
				setStatus(STATUS_STATUS, CTSTRING(DOWNLOADING_LIST));
			} catch(const QueueException& e) {
				setStatus(STATUS_STATUS, Text::toT(e.getError()).c_str());
			}
		} else {
			setStatus(STATUS_STATUS, CTSTRING(USER_OFFLINE));
		}
	}
}

void DirectoryListingFrame::clearList() {
	int j = files->size();
	for(int i = 0; i < j; i++) {
		delete files->getData(i);
	}
	files->clear();
}

void DirectoryListingFrame::addHistory(const string& name) {
	history.erase(history.begin() + historyIndex, history.end());
	while(history.size() > 25)
		history.pop_front();
	history.push_back(name);
	historyIndex = history.size();
}

void DirectoryListingFrame::up() {
	HTREEITEM t = dirs->getSelection();
	if(t == NULL)
		return;
	t = dirs->getParent(t);
	if(t == NULL)
		return;
	dirs->select(t);
}

void DirectoryListingFrame::back() {
	if(history.size() > 1 && historyIndex > 1) {
		size_t n = min(historyIndex, history.size()) - 1;
		deque<string> tmp = history;
		selectItem(Text::toT(history[n - 1]));
		historyIndex = n;
		history = tmp;
	}
}

void DirectoryListingFrame::forward() {
	if(history.size() > 1 && historyIndex < history.size()) {
		size_t n = min(historyIndex, history.size() - 1);
		deque<string> tmp = history;
		selectItem(Text::toT(history[n]));
		historyIndex = n + 1;
		history = tmp;
	}
}

HTREEITEM DirectoryListingFrame::findFile(const StringSearch& str, HTREEITEM root,
										  int &foundFile, int &skipHits)
{
	// Check dir name for match
	DirectoryListing::Directory* dir = dirs->getData(root)->dir;
	if(str.match(dir->getName())) {
		if(skipHits == 0) {
			foundFile = -1;
			return root;
		} else {
			skipHits--;
		}
	}

	// Force list pane to contain files of current dir
	changeDir(dir);

	// Check file names in list pane
	for(int i=0; i<files->size(); i++) {
		ItemInfo* ii = files->getData(i);
		if(ii->type == ItemInfo::FILE) {
			if(str.match(ii->file->getName())) {
				if(skipHits == 0) {
					foundFile = i;
					return root;
				} else {
					skipHits--;
				}
			}
		}
	}

	dcdebug("looking for directories...\n");
	// Check subdirs recursively
	HTREEITEM item = dirs->getChild(root);
	while(item != NULL) {
		HTREEITEM srch = findFile(str, item, foundFile, skipHits);
		if(srch)
			return srch;
		else
			item = dirs->getNextSibling(item);
	}

	return 0;
}

void DirectoryListingFrame::findFile(bool findNext)
{
	if(!findNext) {
		// Prompt for substring to find
		LineDlg dlg(this, TSTRING(SEARCH_FOR_FILE), TSTRING(ENTER_SEARCH_STRING));

		if(dlg.run() != IDOK)
			return;

		findStr = Text::fromT(dlg.getLine());
		skipHits = 0;
	} else {
		skipHits++;
	}

	if(findStr.empty())
		return;

	HoldRedraw hold(files);
	HoldRedraw hold2(dirs);
	
	// Do a search
	int foundFile = -1, skipHitsTmp = skipHits;
	HTREEITEM const oldDir = dirs->getSelection();
	HTREEITEM const foundDir = findFile(StringSearch(findStr), treeRoot, foundFile, skipHitsTmp);

	if(foundDir) {
		// Highlight the directory tree and list if the parent dir/a matched dir was found
		if(foundFile >= 0) {
			// SelectItem won't update the list if SetRedraw was set to FALSE and then
			// to TRUE and the item selected is the same as the last one... workaround:
			if(oldDir == foundDir)
				dirs->select(NULL);

			dirs->select(foundDir);
		} else {
			// Got a dir; select its parent directory in the tree if there is one
			HTREEITEM parentItem = dirs->getParent(foundDir);
			if(parentItem) {
				// Go to parent file list
				dirs->select(parentItem);

				// Locate the dir in the file list
				DirectoryListing::Directory* dir = dirs->getData(foundDir)->dir;

				foundFile = files->find(Text::toT(dir->getName()), -1, false);
			} else {
				// If no parent exists, just the dir tree item and skip the list highlighting
				dirs->select(foundDir);
			}
		}

		// Remove prev. selection from file list
		if(files->hasSelection()) {
			files->clearSelection();
		}

		// Highlight and focus the dir/file if possible
		if(foundFile >= 0) {
			files->setFocus();
			files->setSelectedIndex(foundFile);
			files->ensureVisible(foundFile);
		} else {
			dirs->setFocus();
		}
	} else {
		dirs->select(oldDir);
		createMessageBox().show(TSTRING(NO_MATCHES), TSTRING(SEARCH_FOR_FILE));
	}
}

void DirectoryListingFrame::runUserCommand(const UserCommand& uc) {
	if(!WinUtil::getUCParams(this, uc, ucLineParams))
		return;

	StringMap ucParams = ucLineParams;

	set<UserPtr> users;

	int sel = -1;
	while((sel = files->getNext(sel, LVNI_SELECTED)) != -1) {
		ItemInfo* ii = files->getData(sel);
		if(uc.getType() == UserCommand::TYPE_RAW_ONCE) {
			if(users.find(dl->getUser()) != users.end())
				continue;
			users.insert(dl->getUser());
		}
		if(!dl->getUser()->isOnline())
			return;
		ucParams["fileTR"] = "NONE";
		if(ii->type == ItemInfo::FILE) {
			ucParams["type"] = "File";
			ucParams["fileFN"] = dl->getPath(ii->file) + ii->file->getName();
			ucParams["fileSI"] = Util::toString(ii->file->getSize());
			ucParams["fileSIshort"] = Util::formatBytes(ii->file->getSize());
			ucParams["fileTR"] = ii->file->getTTH().toBase32();
		} else {
			ucParams["type"] = "Directory";
			ucParams["fileFN"] = dl->getPath(ii->dir) + ii->dir->getName();
			ucParams["fileSI"] = Util::toString(ii->dir->getTotalSize());
			ucParams["fileSIshort"] = Util::formatBytes(ii->dir->getTotalSize());
		}

		// compatibility with 0.674 and earlier
		ucParams["file"] = ucParams["fileFN"];
		ucParams["filesize"] = ucParams["fileSI"];
		ucParams["filesizeshort"] = ucParams["fileSIshort"];
		ucParams["tth"] = ucParams["fileTR"];

		StringMap tmp = ucParams;
		ClientManager::getInstance()->userCommand(dl->getUser(), uc, tmp, true);
	}
}

void DirectoryListingFrame::handleDoubleClickFiles() {

	HTREEITEM t = dirs->getSelection();
	int i = files->getSelectedIndex();
	if(t != NULL && i != -1) {
		ItemInfo* ii = files->getData(i);

		if(ii->type == ItemInfo::FILE) {
			try {
				dl->download(ii->file, SETTING(DOWNLOAD_DIRECTORY) + Text::fromT(ii->getText(COLUMN_FILENAME)), false, WinUtil::isShift());
			} catch(const Exception& e) {
				setStatus(STATUS_STATUS, Text::toT(e.getError()));
			}
		} else {
			HTREEITEM ht = dirs->getChild(t);
			while(ht != NULL) {
				if(dirs->getData(ht)->dir == ii->dir) {
					dirs->select(ht);
					break;
				}
				ht = dirs->getNextSibling(ht);
			}
		}
	}
}

LRESULT DirectoryListingFrame::handleXButtonUp(WPARAM wParam, LPARAM lParam) {
	if(HIWORD(wParam) & XBUTTON1) {
		back();
		return TRUE;
	} else if(HIWORD(wParam) & XBUTTON2) {
		forward();
		return TRUE;
	}

	return FALSE;
}

bool DirectoryListingFrame::handleKeyDownFiles(int c) {
	if(c == VK_BACK) {
		up();
	} else if(c == VK_LEFT && WinUtil::isAlt()) {
		back();
	} else if(c == VK_RIGHT && WinUtil::isAlt()) {
		forward();
	} else if(c == VK_RETURN) {
		if(files->getSelectedCount() == 1) {
			ItemInfo* ii = files->getSelectedData();
			if(ii->type == ItemInfo::DIRECTORY) {
				HTREEITEM ht = dirs->getChild(dirs->getSelection());
				while(ht != NULL) {
					if(dirs->getData(ht)->dir == ii->dir) {
						dirs->select(ht);
						break;
					}
					ht = dirs->getNextSibling(ht);
				}
			} else {
				downloadFiles(SETTING(DOWNLOAD_DIRECTORY));
			}
		} else {
			downloadFiles(SETTING(DOWNLOAD_DIRECTORY));
		}
	}
	return true;
}

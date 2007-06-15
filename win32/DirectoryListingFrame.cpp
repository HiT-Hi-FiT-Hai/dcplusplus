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
#include <client/DCPlusPlus.h>

#include "DirectoryListingFrame.h"

#include "resource.h"

#include <client/ResourceManager.h>
#include <client/ADLSearch.h>
#include <client/QueueManager.h>
#include <client/FavoriteManager.h>
#include <client/File.h>

int DirectoryListingFrame::columnIndexes[] = { COLUMN_FILENAME, COLUMN_TYPE, COLUMN_EXACTSIZE, COLUMN_SIZE, COLUMN_TTH };
int DirectoryListingFrame::columnSizes[] = { 300, 60, 100, 100, 200 };

static ResourceManager::Strings columnNames[] = { ResourceManager::FILE, ResourceManager::TYPE, ResourceManager::EXACT_SIZE, ResourceManager::SIZE, ResourceManager::TTH_ROOT };

DirectoryListingFrame::UserMap DirectoryListingFrame::lists;

void DirectoryListingFrame::openWindow(SmartWin::Widget* mdiParent, const tstring& aFile, const tstring& aDir, const User::Ptr& aUser, int64_t aSpeed) {
	UserIter i = lists.find(aUser);
	if(i != lists.end()) {
		i->second->speed = aSpeed;
		if(!BOOLSETTING(POPUNDER_FILELIST)) {
#ifdef PORT_ME
			i->second->MDIActivate(i->second->m_hWnd);
#endif
		}
	} else {
		DirectoryListingFrame* frame = new DirectoryListingFrame(mdiParent, aUser, aSpeed);
#ifdef PORT_ME
		if(BOOLSETTING(POPUNDER_FILELIST)) {
			WinUtil::hiddenCreateEx(frame);
		} else {
			frame->CreateEx(WinUtil::mdiClient);
		}
#endif
		
		frame->loadFile(aFile, aDir);
	}
}

void DirectoryListingFrame::openWindow(SmartWin::Widget* mdiParent, const User::Ptr& aUser, const string& txt, int64_t aSpeed) {
	UserIter i = lists.find(aUser);
	if(i != lists.end()) {
		i->second->speed = aSpeed;
		i->second->loadXML(txt);
	} else {
		DirectoryListingFrame* frame = new DirectoryListingFrame(mdiParent, aUser, aSpeed);
#ifdef PORT_ME
		if(BOOLSETTING(POPUNDER_FILELIST)) {
			WinUtil::hiddenCreateEx(frame);
		} else {
			frame->CreateEx(WinUtil::mdiClient);
		}
#endif
		frame->loadXML(txt);
	}
}

DirectoryListingFrame::DirectoryListingFrame(SmartWin::Widget* mdiParent, const UserPtr& aUser, int64_t aSpeed) :
	SmartWin::Widget(mdiParent),
	status(0),
	dirs(0),
	files(0),
	splitter(0),
	find(0),
	findNext(0),
	listDiff(0),
	matchQueue(0),
	speed(aSpeed),
	dl(new DirectoryListing(aUser)),
	usingDirMenu(false),
	treeRoot(NULL)
{
	splitter = createSplitterCool();
	splitter->onMoved(&DirectoryListingFrame::splitterMoved);

	{
		WidgetTreeView::Seed cs;
		
		cs.style = WS_CHILD | WS_VISIBLE | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_HASLINES | TVS_SHOWSELALWAYS | TVS_DISABLEDRAGDROP;
		cs.exStyle = WS_EX_CLIENTEDGE;
		dirs = SmartWin::WidgetCreator<WidgetDirs>::create(this, cs);
		add_widget(dirs);
		dirs->setColor(WinUtil::textColor, WinUtil::bgColor);
		dirs->setNormalImageList(WinUtil::fileImages);
	}
	
	{
		WidgetFiles::Seed cs;
		cs.style = WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_NOSORTHEADER | LVS_SHAREIMAGELISTS;
		cs.exStyle = WS_EX_CLIENTEDGE;
		files = SmartWin::WidgetCreator<WidgetFiles>::create(this, cs);
		files->setListViewStyle(LVS_EX_LABELTIP | LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT);
		files->setFont(WinUtil::font);
		add_widget(files);

		files->setSmallImageList(WinUtil::fileImages);
		files->createColumns(ResourceManager::getInstance()->getStrings(columnNames));
		files->setColumnOrder(WinUtil::splitTokens(SETTING(QUEUEFRAME_ORDER), columnIndexes));
		files->setColumnWidths(WinUtil::splitTokens(SETTING(QUEUEFRAME_WIDTHS), columnSizes));
		files->setColor(WinUtil::textColor, WinUtil::bgColor);
	}
	
	{
		WidgetButton::Seed cs;
		cs.style = WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON;
		
		cs.caption = TSTRING(FIND);
		find = createButton(cs);
		find->onClicked(&DirectoryListingFrame::handleFind);
		
		cs.caption = TSTRING(NEXT);
		findNext = createButton(cs);
		findNext->onClicked(&DirectoryListingFrame::handleFindNext);

		cs.caption = TSTRING(MATCH_QUEUE);
		matchQueue = createButton(cs);
		matchQueue->onClicked(&DirectoryListingFrame::handleMatchQueue);
		
		cs.caption = TSTRING(FILE_LIST_DIFF);
		listDiff = createButton(cs);
		listDiff->onClicked(&DirectoryListingFrame::handleListDiff);
	}
	
	status = createStatusBarSections();
	memset(statusSizes, 0, sizeof(statusSizes));
	///@todo get real resizer width
	statusSizes[STATUS_DUMMY] = 16;
	
	// This will set the widths correctly
	setStatus(STATUS_FILE_LIST_DIFF, TSTRING(FILE_LIST_DIFF));
	setStatus(STATUS_MATCH_QUEUE, TSTRING(MATCH_QUEUE));
	setStatus(STATUS_FIND, TSTRING(FIND));
	setStatus(STATUS_NEXT, TSTRING(NEXT));

	string nick = ClientManager::getInstance()->getNicks(dl->getUser()->getCID())[0];
	treeRoot = dirs->insert(NULL, new ItemInfo(nick, dl->getRoot()));

	lists.insert(std::make_pair(aUser, this));
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


void DirectoryListingFrame::initStatus() {
	setStatus(STATUS_TOTAL_FILES, Text::toT(STRING(FILES) + ": " + Util::toString(dl->getTotalFileCount(true))));
	setStatus(STATUS_TOTAL_SIZE, Text::toT(STRING(SIZE) + ": " + Util::formatBytes(dl->getTotalSize(true))));
	setStatus(STATUS_SPEED, Text::toT(STRING(SPEED) + ": " + Util::formatBytes(speed) + "/s"));
}

void DirectoryListingFrame::setStatus(Status s, const tstring& text) {
	int w = status->getTextSize(text).x + 12;
	if(w > static_cast<int>(statusSizes[s])) {
		dcdebug("Setting status size %d to %d\n", s, w);
		statusSizes[s] = w;
		layout();
	}
	status->setText(text, s);
}

void DirectoryListingFrame::layout() {
	SmartWin::Rectangle r(getClientAreaSize()); 
	status->refresh();

	SmartWin::Rectangle rs(status->getClientAreaSize());

	{
		std::vector<unsigned> w(STATUS_LAST);

		w[0] = rs.size.x - rs.pos.x - std::accumulate(statusSizes+1, statusSizes+STATUS_LAST, 0); 
		std::copy(statusSizes+1, statusSizes + STATUS_LAST, w.begin()+1);

		status->setSections(w);
	
		RECT sr;
		
		::SendMessage(status->handle(), SB_GETRECT, STATUS_FILE_LIST_DIFF, reinterpret_cast<LPARAM>(&sr));
		listDiff->setBounds(SmartWin::Rectangle::FromRECT(sr));

		::SendMessage(status->handle(), SB_GETRECT, STATUS_MATCH_QUEUE, reinterpret_cast<LPARAM>(&sr));
		matchQueue->setBounds(SmartWin::Rectangle::FromRECT(sr));
		
		::SendMessage(status->handle(), SB_GETRECT, STATUS_FIND, reinterpret_cast<LPARAM>(&sr));
		find->setBounds(SmartWin::Rectangle::FromRECT(sr));
		
		::SendMessage(status->handle(), SB_GETRECT, STATUS_NEXT, reinterpret_cast<LPARAM>(&sr));
		findNext->setBounds(SmartWin::Rectangle::FromRECT(sr));
	
		
#ifdef PORT_ME
		ctrlLastLines.SetMaxTipWidth(w[0]);
#endif
	}
	
	r.size.y -= rs.size.y;
	
	SmartWin::Rectangle rsplit(splitter->getBounds());
	
	dirs->setBounds(0, 0, rsplit.pos.x, r.size.y);
	files->setBounds(rsplit.pos.x + rsplit.size.x, 0, r.size.x - (rsplit.pos.x + rsplit.size.x), r.size.y);
}

void DirectoryListingFrame::postClosing() {
#ifdef PORT_ME
	clearList();
	frames.erase(m_hWnd);
#endif

	SettingsManager::getInstance()->set(SettingsManager::DIRECTORLISTINGFRAME_ORDER, WinUtil::toString(files->getColumnOrder()));
	SettingsManager::getInstance()->set(SettingsManager::DIRECTORLISTINGFRAME_WIDTHS, WinUtil::toString(files->getColumnWidths()));
}

void DirectoryListingFrame::splitterMoved(WidgetSplitterCool*, const SmartWin::Point& pt) {
	layout();
}


void DirectoryListingFrame::handleFind(WidgetButtonPtr) {
#ifdef PORT_ME
	searching = true;
	findFile(false);
	searching = false;
	updateStatus();
#endif
}

void DirectoryListingFrame::handleFindNext(WidgetButtonPtr) {
#ifdef PORT_ME
	searching = true;
	findFile(true);
	searching = false;
	updateStatus();
#endif
}

void DirectoryListingFrame::handleMatchQueue(WidgetButtonPtr) {
	int x = QueueManager::getInstance()->matchListing(*dl);
	AutoArray<TCHAR> buf(STRING(MATCHED_FILES).length() + 32);
	_stprintf(buf, CTSTRING(MATCHED_FILES), x);
	setStatus(STATUS_TEXT, (TCHAR*)buf);
}

void DirectoryListingFrame::handleListDiff(WidgetButtonPtr) {
	tstring file;
#ifdef PORT_ME
	if(WinUtil::browseFile(file, m_hWnd, false, Text::toT(Util::getListPath()), _T("File Lists\0*.xml.bz2\0All Files\0*.*\0"))) {
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
#endif
}

void DirectoryListingFrame::refreshTree(const tstring& root) {
#ifdef PORT_ME
	ctrlTree.SetRedraw(FALSE);

	HTREEITEM ht = findItem(treeRoot, root);
	if(ht == NULL) {
		ht = treeRoot;
	}

	DirectoryListing::Directory* d = (DirectoryListing::Directory*)ctrlTree.GetItemData(ht);

	HTREEITEM next = NULL;
	while((next = ctrlTree.GetChildItem(ht)) != NULL) {
		ctrlTree.DeleteItem(next);
	}

	updateTree(d, ht);

	ctrlTree.Expand(treeRoot);

	int index = d->getComplete() ? WinUtil::getDirIconIndex() : WinUtil::getDirMaskedIndex();
	ctrlTree.SetItemImage(ht, index, index);

	ctrlTree.SelectItem(NULL);
	selectItem(root);

	ctrlTree.SetRedraw(TRUE);
#endif
}

DirectoryListingFrame::WidgetPopupMenuPtr DirectoryListingFrame::makeSingleMenu(ItemInfo* ii) {
	WidgetPopupMenuPtr menu = createPopupMenu();
	
	menu->appendItem(IDC_DOWNLOAD, TSTRING(DOWNLOAD), &DirectoryListingFrame::handleDownload);
	addTargets(menu, ii);
	
	if(ii->type == ItemInfo::FILE) {
		menu->appendItem(IDC_VIEW_AS_TEXT, TSTRING(VIEW_AS_TEXT), &DirectoryListingFrame::handleViewAsText);
		
		menu->appendSeparatorItem();
		
		menu->appendItem(IDC_SEARCH_ALTERNATES, TSTRING(SEARCH_FOR_ALTERNATES), &DirectoryListingFrame::handleSearchAlternates);
		menu->appendItem(IDC_BITZI_LOOKUP, TSTRING(LOOKUP_AT_BITZI), &DirectoryListingFrame::handleLookupBitzi);
		menu->appendItem(IDC_COPY_MAGNET, TSTRING(COPY_MAGNET), &DirectoryListingFrame::handleCopyMagnet);
	}

	if((ii->type == ItemInfo::FILE && ii->file->getAdls()) ||
		(ii->type == ItemInfo::DIRECTORY && ii->dir->getAdls() && ii->dir->getParent() != dl->getRoot()) )	{
		menu->appendSeparatorItem();
		menu->appendItem(IDC_GO_TO_DIRECTORY, TSTRING(GO_TO_DIRECTORY), &DirectoryListingFrame::handleGoToDirectory);
	}
	
#ifdef PORT_ME
	fileMenu.SetMenuDefaultItem(IDC_DOWNLOAD);
#endif
	return menu;
}


DirectoryListingFrame::WidgetPopupMenuPtr DirectoryListingFrame::makeMultiMenu() {
	WidgetPopupMenuPtr menu = createPopupMenu();
	
	menu->appendItem(IDC_DOWNLOAD, TSTRING(DOWNLOAD), &DirectoryListingFrame::handleDownload);
	addTargets(menu);

#ifdef PORT_ME
	fileMenu.SetMenuDefaultItem(IDC_DOWNLOAD);
#endif
	return menu;
}

DirectoryListingFrame::WidgetPopupMenuPtr DirectoryListingFrame::makeDirMenu() {
	WidgetPopupMenuPtr menu = createPopupMenu();
	
	menu->appendItem(IDC_DOWNLOAD, TSTRING(DOWNLOAD), &DirectoryListingFrame::handleDownload);
	addTargets(menu);
}

void DirectoryListingFrame::addUserCommands(const WidgetPopupMenuPtr& parent) {
#ifdef PORT_ME
	prepareMenu(fileMenu, UserCommand::CONTEXT_FILELIST, ClientManager::getInstance()->getHubs(dl->getUser()->getCID()));
#endif
}

void DirectoryListingFrame::addTargets(const WidgetPopupMenuPtr& parent, ItemInfo* ii) {
	WidgetMenuPtr menu = parent->appendPopup(TSTRING(DOWNLOAD_TO));
	
	StringPairList spl = FavoriteManager::getInstance()->getFavoriteDirs();
	size_t i = 0;
	for(; i < spl.size(); ++i) {
		menu->appendItem(IDC_DOWNLOAD_FAVORITE_DIRS + i, Text::toT(spl[i].second), &DirectoryListingFrame::handleDownloadFavorite);
	}
	
	if(i > 0) {
		menu->appendSeparatorItem();
	}

	menu->appendItem(IDC_DOWNLOAD_BROWSE, TSTRING(BROWSE), &DirectoryListingFrame::handleDownloadBrowse);

	targets.clear();
	
	if(ii && ii->type == ItemInfo::FILE) {
		QueueManager::getInstance()->getTargets(ii->file->getTTH(), targets);
		if(!targets.empty()) {
			menu->appendSeparatorItem();
			for(i = 0; i < targets.size(); ++i) {
				menu->appendItem(IDC_DOWNLOAD_TARGET + i, Text::toT(targets[i]), &DirectoryListingFrame::handleDownloadTarget);
			}
		}
	}
	
	if(WinUtil::lastDirs.size() > 0) {
		menu->appendSeparatorItem();
		
		for(i = 0; i < WinUtil::lastDirs.size(); ++i) {
			menu->appendItem(IDC_DOWNLOAD_LASTDIR + i, WinUtil::lastDirs[i], &DirectoryListingFrame::handleDownloadLastDir);
		}
	}
}

HRESULT DirectoryListingFrame::handleContextMenu(LPARAM lParam, WPARAM wParam) {
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
	
	WidgetPopupMenuPtr contextMenu;
	if(reinterpret_cast<HWND>(wParam) == files->handle() && files->hasSelection()) {
		if(pt.x == -1 && pt.y == -1) {
			pt = files->getContextMenuPos();
		}
		
		if(files->getSelectedCount() == 1) {
#ifdef PORT_ME
			if(BOOLSETTING(SHOW_SHELL_MENU) && (dl->getUser() == ClientManager::getInstance()->getMe())) {
				string path;
				try {
					path = ShareManager::getInstance()->toReal(Util::toAdcFile(dl->getPath(ii->file) + ii->file->getName()));
				} catch(const ShareException&) {
					// Ignore
				}
				if(!path.empty() && (File::getSize(path) != -1)) {
					CShellContextMenu shellMenu;
					shellMenu.SetPath(Text::toT(path));
					shellMenu.ShowContextMenu(m_hWnd, pt);
					return TRUE;
				}
			}
#endif
			ItemInfo* ii = files->getSelectedItem();
			
			contextMenu = makeSingleMenu(ii);
		} else {
			contextMenu = makeMultiMenu();
		}
		usingDirMenu = false;
		contextMenu->trackPopupMenu(this, pt.x, pt.y, TPM_LEFTALIGN | TPM_RIGHTBUTTON);
		return TRUE;
	} else if(reinterpret_cast<HWND>(wParam) == dirs->handle()) {
		if(pt.x == -1 && pt.y == -1) {
			pt = dirs->getContextMenuPos();
		} else {
			dirs->select(pt);
		}

		if(dirs->getSelected() == NULL) {
			return FALSE;
		}

		contextMenu = makeDirMenu();
		usingDirMenu = true;
		
		contextMenu->trackPopupMenu(this, pt.x, pt.y, TPM_LEFTALIGN | TPM_RIGHTBUTTON);

		return TRUE;
	}

	return FALSE;
}

void DirectoryListingFrame::downloadFiles(const tstring& aTarget, bool view /* = false */) {
	int i=-1;
	
	while( (i = files->getNextItem(i, LVNI_SELECTED)) != -1) {
		download(files->getItemData(i), aTarget, view);
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
		setStatus(STATUS_TEXT, Text::toT(e.getError()));
	}
}

void DirectoryListingFrame::handleSearchAlternates(WidgetMenuPtr, unsigned id) {
	ItemInfo* ii = files->getSelectedItem();
	if(ii != NULL && ii->type == ItemInfo::FILE) {
		WinUtil::searchHash(ii->file->getTTH());
	}
}

void DirectoryListingFrame::handleLookupBitzi(WidgetMenuPtr, unsigned id) {
	ItemInfo* ii = files->getSelectedItem();
	if(ii != NULL && ii->type == ItemInfo::FILE) {
		WinUtil::bitziLink(ii->file->getTTH());
	}
}

void DirectoryListingFrame::handleCopyMagnet(WidgetMenuPtr, unsigned id) {
	ItemInfo* ii = files->getSelectedItem();
	if(ii != NULL && ii->type == ItemInfo::FILE) {
		WinUtil::copyMagnet(ii->file->getTTH(), ii->getText(COLUMN_FILENAME));
	}
}

void DirectoryListingFrame::handleDownload(WidgetMenuPtr, unsigned id) {
	if(usingDirMenu) {
		ItemInfo* ii = dirs->getSelectedData();
		if(ii) {
			download(ii, SETTING(DOWNLOAD_DIRECTORY));
		}
	} else {
		downloadFiles(SETTING(DOWNLOAD_DIRECTORY));
	}
}

void DirectoryListingFrame::handleDownloadBrowse(WidgetMenuPtr, unsigned id) {
	if(usingDirMenu) {
		ItemInfo* ii = dirs->getSelectedData();
		if(ii) {
			tstring target = SETTING(DOWNLOAD_DIRECTORY);
			if(WinUtil::browseDirectory(target, handle())) {
				WinUtil::addLastDir(target);
				download(ii, Text::fromT(target));
			}
		}
	} else {
		if(files->getSelectedCount() == 1) {
			ItemInfo* ii = files->getSelectedItem();
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
				setStatus(STATUS_TEXT, Text::toT(e.getError()).c_str());
			}
		} else {
			tstring target = SETTING(DOWNLOAD_DIRECTORY);
			if(WinUtil::browseDirectory(target, handle())) {
				WinUtil::addLastDir(target);
				downloadFiles(Text::fromT(target));
			}
		}
	}
}

void DirectoryListingFrame::handleDownloadLastDir(WidgetMenuPtr, unsigned id) {
	size_t n = id - IDC_DOWNLOAD_LASTDIR;
	if(n >= WinUtil::lastDirs.size()) {
		return;
	}
	download(WinUtil::lastDirs[n]);
}

void DirectoryListingFrame::handleDownloadFavorite(WidgetMenuPtr, unsigned id) {
	size_t n = id - IDC_DOWNLOAD_FAVORITE_DIRS;
	StringPairList spl = FavoriteManager::getInstance()->getFavoriteDirs();
	if(n >= spl.size()) {
		return;
	}
	download(spl[n].first);
}

void DirectoryListingFrame::handleDownloadTarget(WidgetMenuPtr, unsigned id) {
	size_t n = id - IDC_DOWNLOAD_TARGET;
	if(n >= targets.size()) {
		return;
	}
	
	if(files->getSelectedCount() != 1) {
		return;
	}

	const string& target = targets[n];
	ItemInfo* ii = files->getSelectedItem();
	try {
		dl->download(ii->file, target, false, WinUtil::isShift());
	} catch(const Exception& e) {
		setStatus(STATUS_TEXT, Text::toT(e.getError()).c_str());
	}
}


void DirectoryListingFrame::handleGoToDirectory(WidgetMenuPtr, unsigned id) {
	if(files->getSelectedCount() != 1)
		return;

	tstring fullPath;
	ItemInfo* ii = files->getSelectedItem();
	
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

#ifdef PORT_ME
	selectItem(fullPath);
#endif
}

void DirectoryListingFrame::download(const string& target) {
	if(usingDirMenu) {
		ItemInfo* ii = dirs->getSelectedData();
		if(ii) {
			download(ii, target);
		}
	} else {
		if(files->getSelectedCount() == 1) {
			ItemInfo* ii = files->getSelectedItem();
			download(ii, target);
		} else {
			downloadFiles(target);
		}
	}
}

void DirectoryListingFrame::handleViewAsText(WidgetMenuPtr, unsigned id) {
	downloadFiles(Text::toT(Util::getTempPath()), true);
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
#ifdef PORT_ME
		ctrlTree.EnsureVisible(ht);
#endif
		dirs->select(ht);
	}
}

DirectoryListingFrame::~DirectoryListingFrame() {
	dcassert(lists.find(dl->getUser()) != lists.end());
	lists.erase(dl->getUser());
}

#ifdef PORT_ME

#include "stdafx.h"
#include "../client/DCPlusPlus.h"

#include "Resource.h"

#include "DirectoryListingFrm.h"
#include "ShellContextMenu.h"
#include "WinUtil.h"
#include "LineDlg.h"

#include "../client/File.h"
#include "../client/QueueManager.h"
#include "../client/ShareManager.h"
#include "../client/StringTokenizer.h"
#include "../client/ADLSearch.h"
#include "../client/MerkleTree.h"
#include "../client/User.h"
#include "../client/ClientManager.h"

LRESULT DirectoryListingFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {

	ctrlStatus.Attach(m_hWndStatusBar);
	statusContainer.SubclassWindow(ctrlStatus.m_hWnd);

	ctrlList.setSortColumn(COLUMN_FILENAME);

	SetSplitterExtendedStyle(SPLIT_PROPORTIONAL);
	SetSplitterPanes(ctrlTree.m_hWnd, ctrlList.m_hWnd);
	m_nProportionalPos = 2500;

	setWindowTitle();

	bHandled = FALSE;
	return 1;
}

void DirectoryListingFrame::updateTree(DirectoryListing::Directory* aTree, HTREEITEM aParent) {
	for(DirectoryListing::Directory::Iter i = aTree->directories.begin(); i != aTree->directories.end(); ++i) {
		tstring name = Text::toT((*i)->getName());
		int index = (*i)->getComplete() ? WinUtil::getDirIconIndex() : WinUtil::getDirMaskedIndex();
		HTREEITEM ht = ctrlTree.InsertItem(TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM, name.c_str(), index, index, 0, 0, (LPARAM)*i, aParent, TVI_SORT);
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

		tstring tmp = Text::toT(STRING(ITEMS) + ": " + Util::toString(cnt));
		bool u = false;

		int w = WinUtil::getTextWidth(tmp, ctrlStatus.m_hWnd);
		if(statusSizes[STATUS_SELECTED_FILES] < w) {
			statusSizes[STATUS_SELECTED_FILES] = w;
			u = true;
		}
		ctrlStatus.SetText(STATUS_SELECTED_FILES, tmp.c_str());

		tmp = Text::toT(STRING(SIZE) + ": " + Util::formatBytes(total));
		w = WinUtil::getTextWidth(tmp, ctrlStatus.m_hWnd);
		if(statusSizes[STATUS_SELECTED_SIZE] < w) {
			statusSizes[STATUS_SELECTED_SIZE] = w;
			u = true;
		}
		ctrlStatus.SetText(STATUS_SELECTED_SIZE, tmp.c_str());

		if(u)
			UpdateLayout(TRUE);
	}
}

LRESULT DirectoryListingFrame::onSelChangedDirectories(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NMTREEVIEW* p = (NMTREEVIEW*) pnmh;

	if(p->itemNew.state & TVIS_SELECTED) {
		DirectoryListing::Directory* d = (DirectoryListing::Directory*)p->itemNew.lParam;
		changeDir(d, TRUE);
		addHistory(dl->getPath(d));
	}
	return 0;
}

void DirectoryListingFrame::addHistory(const string& name) {
	history.erase(history.begin() + historyIndex, history.end());
	while(history.size() > 25)
		history.pop_front();
	history.push_back(name);
	historyIndex = history.size();
}

void DirectoryListingFrame::changeDir(DirectoryListing::Directory* d, BOOL enableRedraw)
{
	ctrlList.SetRedraw(FALSE);
	updating = true;
	clearList();

	for(DirectoryListing::Directory::Iter i = d->directories.begin(); i != d->directories.end(); ++i) {
		ctrlList.insertItem(ctrlList.GetItemCount(), new ItemInfo(*i), (*i)->getComplete() ? WinUtil::getDirIconIndex() : WinUtil::getDirMaskedIndex());
	}
	for(DirectoryListing::File::Iter j = d->files.begin(); j != d->files.end(); ++j) {
		ItemInfo* ii = new ItemInfo(*j);
		ctrlList.insertItem(ctrlList.GetItemCount(), ii, WinUtil::getIconIndex(ii->getText(COLUMN_FILENAME)));
	}
	ctrlList.resort();
	ctrlList.SetRedraw(enableRedraw);
	updating = false;
	updateStatus();

	if(!d->getComplete()) {
		if(dl->getUser()->isOnline()) {
			try {
				QueueManager::getInstance()->addPfs(dl->getUser(), dl->getPath(d));
				ctrlStatus.SetText(STATUS_TEXT, CTSTRING(DOWNLOADING_LIST));
			} catch(const QueueException& e) {
				ctrlStatus.SetText(STATUS_TEXT, Text::toT(e.getError()).c_str());
			}
		} else {
			ctrlStatus.SetText(STATUS_TEXT, CTSTRING(USER_OFFLINE));
		}
	}
}

void DirectoryListingFrame::up() {
	HTREEITEM t = ctrlTree.GetSelectedItem();
	if(t == NULL)
		return;
	t = ctrlTree.GetParentItem(t);
	if(t == NULL)
		return;
	ctrlTree.SelectItem(t);
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

LRESULT DirectoryListingFrame::onDoubleClickFiles(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NMITEMACTIVATE* item = (NMITEMACTIVATE*) pnmh;

	HTREEITEM t = ctrlTree.GetSelectedItem();
	if(t != NULL && item->iItem != -1) {
		ItemInfo* ii = ctrlList.getItemData(item->iItem);

		if(ii->type == ItemInfo::FILE) {
			try {
				dl->download(ii->file, SETTING(DOWNLOAD_DIRECTORY) + Text::fromT(ii->getText(COLUMN_FILENAME)), false, WinUtil::isShift());
			} catch(const Exception& e) {
				ctrlStatus.SetText(STATUS_TEXT, Text::toT(e.getError()).c_str());
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





HRESULT DirectoryListingFrame::onXButtonUp(UINT /*uMsg*/, WPARAM wParam, LPARAM /* lParam */, BOOL& /* bHandled */) {
	if(GET_XBUTTON_WPARAM(wParam) & XBUTTON1) {
		back();
		return TRUE;
	} else if(GET_XBUTTON_WPARAM(wParam) & XBUTTON2) {
		forward();
		return TRUE;
	}

	return FALSE;
}


LRESULT DirectoryListingFrame::onKeyDown(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NMLVKEYDOWN* kd = (NMLVKEYDOWN*) pnmh;
	if(kd->wVKey == VK_BACK) {
		up();
	} else if(kd->wVKey == VK_TAB) {
		onTab();
	} else if(kd->wVKey == VK_LEFT && WinUtil::isAlt()) {
		back();
	} else if(kd->wVKey == VK_RIGHT && WinUtil::isAlt()) {
		forward();
	} else if(kd->wVKey == VK_RETURN) {
		if(ctrlList.GetSelectedCount() == 1) {
			ItemInfo* ii = ctrlList.getItemData(ctrlList.GetNextItem(-1, LVNI_SELECTED));
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
				downloadList(Text::toT(SETTING(DOWNLOAD_DIRECTORY)));
			}
		} else {
			downloadList(Text::toT(SETTING(DOWNLOAD_DIRECTORY)));
		}
	}
	return 0;
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
		ItemInfo* ii = ctrlList.getItemData(i);
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
		dlg.title = TSTRING(SEARCH_FOR_FILE);
		dlg.description = TSTRING(ENTER_SEARCH_STRING);
		dlg.line = Util::emptyStringT;

		if(dlg.DoModal() != IDOK)
			return;

		findStr = Text::fromT(dlg.line);
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

				foundFile = ctrlList.findItem(Text::toT(dir->getName()), -1, false);
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
		MessageBox(CTSTRING(NO_MATCHES), CTSTRING(SEARCH_FOR_FILE));
	}
}

void DirectoryListingFrame::runUserCommand(UserCommand& uc) {
	if(!WinUtil::getUCParams(m_hWnd, uc, ucLineParams))
		return;

	StringMap ucParams = ucLineParams;

	set<User::Ptr> nicks;

	int sel = -1;
	while((sel = ctrlList.GetNextItem(sel, LVNI_SELECTED)) != -1) {
		ItemInfo* ii = ctrlList.getItemData(sel);
		if(uc.getType() == UserCommand::TYPE_RAW_ONCE) {
			if(nicks.find(dl->getUser()) != nicks.end())
				continue;
			nicks.insert(dl->getUser());
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
		User::Ptr tmpPtr = dl->getUser();
		ClientManager::getInstance()->userCommand(dl->getUser(), uc, tmp, true);
	}
}

void DirectoryListingFrame::closeAll(){
	for(FrameIter i = frames.begin(); i != frames.end(); ++i)
		i->second->PostMessage(WM_CLOSE, 0, 0);
}
#endif

/*
 * Copyright (C) 2001-2005 Jacek Sieka, arnetheduck on gmail point com
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
#include "../client/DCPlusPlus.h"
#include "Resource.h"

#include "QueueFrame.h"
#include "SearchFrm.h"
#include "PrivateFrame.h"
#include "LineDlg.h"

#include "../client/StringTokenizer.h"
#include "../client/ShareManager.h"

#define FILE_LIST_NAME _T("File Lists")

int QueueFrame::columnIndexes[] = { COLUMN_TARGET, COLUMN_STATUS, COLUMN_SIZE, COLUMN_DOWNLOADED, COLUMN_PRIORITY,
COLUMN_USERS, COLUMN_PATH, COLUMN_EXACT_SIZE, COLUMN_ERRORS, COLUMN_ADDED, COLUMN_TTH, COLUMN_TYPE };

int QueueFrame::columnSizes[] = { 200, 300, 75, 110, 75, 200, 200, 75, 200, 100, 125, 75 };

static ResourceManager::Strings columnNames[] = { ResourceManager::FILENAME, ResourceManager::STATUS, ResourceManager::SIZE, ResourceManager::DOWNLOADED,
ResourceManager::PRIORITY, ResourceManager::USERS, ResourceManager::PATH, ResourceManager::EXACT_SIZE, ResourceManager::ERRORS,
ResourceManager::ADDED, ResourceManager::TTH_ROOT, ResourceManager::TYPE };

LRESULT QueueFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	showTree = BOOLSETTING(QUEUEFRAME_SHOW_TREE);
	
	CreateSimpleStatusBar(ATL_IDS_IDLEMESSAGE, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SBARS_SIZEGRIP);
	ctrlStatus.Attach(m_hWndStatusBar);
	
	ctrlQueue.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS, WS_EX_CLIENTEDGE, IDC_QUEUE);
	ctrlQueue.SetExtendedListViewStyle(LVS_EX_LABELTIP | LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT);
	

	ctrlDirs.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
		TVS_HASBUTTONS | TVS_LINESATROOT | TVS_HASLINES | TVS_SHOWSELALWAYS | TVS_DISABLEDRAGDROP, 
		 WS_EX_CLIENTEDGE, IDC_DIRECTORIES);
	
	ctrlDirs.SetImageList(WinUtil::fileImages, TVSIL_NORMAL);
	ctrlQueue.SetImageList(WinUtil::fileImages, LVSIL_SMALL);
	
	m_nProportionalPos = 2500;
	SetSplitterPanes(ctrlDirs.m_hWnd, ctrlQueue.m_hWnd);

	// Create listview columns
	WinUtil::splitTokens(columnIndexes, SETTING(QUEUEFRAME_ORDER), COLUMN_LAST);
	WinUtil::splitTokens(columnSizes, SETTING(QUEUEFRAME_WIDTHS), COLUMN_LAST);
	
	for(int j=0; j<COLUMN_LAST; j++) {
		int fmt = (j == COLUMN_SIZE || j == COLUMN_DOWNLOADED || j == COLUMN_EXACT_SIZE) ? LVCFMT_RIGHT : LVCFMT_LEFT;
		ctrlQueue.InsertColumn(j, CTSTRING_I(columnNames[j]), fmt, columnSizes[j], j);
	}
	
	ctrlQueue.SetColumnOrderArray(COLUMN_LAST, columnIndexes);
	ctrlQueue.setSortColumn(COLUMN_TARGET);
	
	ctrlQueue.SetBkColor(WinUtil::bgColor);
	ctrlQueue.SetTextBkColor(WinUtil::bgColor);
	ctrlQueue.SetTextColor(WinUtil::textColor);

	ctrlDirs.SetBkColor(WinUtil::bgColor);
	ctrlDirs.SetTextColor(WinUtil::textColor);

	ctrlShowTree.Create(ctrlStatus.m_hWnd, rcDefault, _T("+/-"), WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	ctrlShowTree.SetButtonStyle(BS_AUTOCHECKBOX, false);
	ctrlShowTree.SetCheck(showTree);
	showTreeContainer.SubclassWindow(ctrlShowTree.m_hWnd);
	
	singleMenu.CreatePopupMenu();
	multiMenu.CreatePopupMenu();
	browseMenu.CreatePopupMenu();
	removeMenu.CreatePopupMenu();
	removeAllMenu.CreatePopupMenu();
	pmMenu.CreatePopupMenu();
	priorityMenu.CreatePopupMenu();
	dirMenu.CreatePopupMenu();
	readdMenu.CreatePopupMenu();

	singleMenu.AppendMenu(MF_STRING, IDC_SEARCH_ALTERNATES, CTSTRING(SEARCH_FOR_ALTERNATES));
	singleMenu.AppendMenu(MF_STRING, IDC_BITZI_LOOKUP, CTSTRING(LOOKUP_AT_BITZI));
	singleMenu.AppendMenu(MF_STRING, IDC_COPY_MAGNET, CTSTRING(COPY_MAGNET));
	singleMenu.AppendMenu(MF_STRING, IDC_MOVE, CTSTRING(MOVE));
	singleMenu.AppendMenu(MF_POPUP, (UINT_PTR)(HMENU)priorityMenu, CTSTRING(SET_PRIORITY));
	singleMenu.AppendMenu(MF_POPUP, (UINT_PTR)(HMENU)browseMenu, CTSTRING(GET_FILE_LIST));
	singleMenu.AppendMenu(MF_POPUP, (UINT_PTR)(HMENU)pmMenu, CTSTRING(SEND_PRIVATE_MESSAGE));
	singleMenu.AppendMenu(MF_POPUP, (UINT_PTR)(HMENU)readdMenu, CTSTRING(READD_SOURCE));
	singleMenu.AppendMenu(MF_SEPARATOR);
	singleMenu.AppendMenu(MF_POPUP, (UINT_PTR)(HMENU)removeMenu, CTSTRING(REMOVE_SOURCE));
	singleMenu.AppendMenu(MF_POPUP, (UINT_PTR)(HMENU)removeAllMenu, CTSTRING(REMOVE_FROM_ALL));
	singleMenu.AppendMenu(MF_STRING, IDC_REMOVE, CTSTRING(REMOVE));

	multiMenu.AppendMenu(MF_POPUP, (UINT_PTR)(HMENU)priorityMenu, CTSTRING(SET_PRIORITY));
	multiMenu.AppendMenu(MF_STRING, IDC_MOVE, CTSTRING(MOVE));
	multiMenu.AppendMenu(MF_SEPARATOR);
	multiMenu.AppendMenu(MF_STRING, IDC_REMOVE, CTSTRING(REMOVE));
	
	priorityMenu.AppendMenu(MF_STRING, IDC_PRIORITY_PAUSED, CTSTRING(PAUSED));
	priorityMenu.AppendMenu(MF_STRING, IDC_PRIORITY_LOWEST, CTSTRING(LOWEST));
	priorityMenu.AppendMenu(MF_STRING, IDC_PRIORITY_LOW, CTSTRING(LOW));
	priorityMenu.AppendMenu(MF_STRING, IDC_PRIORITY_NORMAL, CTSTRING(NORMAL));
	priorityMenu.AppendMenu(MF_STRING, IDC_PRIORITY_HIGH, CTSTRING(HIGH));
	priorityMenu.AppendMenu(MF_STRING, IDC_PRIORITY_HIGHEST, CTSTRING(HIGHEST));

	dirMenu.AppendMenu(MF_POPUP, (UINT_PTR)(HMENU)priorityMenu, CTSTRING(SET_PRIORITY));
	dirMenu.AppendMenu(MF_STRING, IDC_MOVE, CTSTRING(MOVE));
	dirMenu.AppendMenu(MF_SEPARATOR);
	dirMenu.AppendMenu(MF_STRING, IDC_REMOVE, CTSTRING(REMOVE));

	removeMenu.AppendMenu(MF_STRING, IDC_REMOVE_SOURCES, CTSTRING(ALL));
	removeMenu.AppendMenu(MF_SEPARATOR);

	readdMenu.AppendMenu(MF_STRING, IDC_READD, CTSTRING(ALL));
	readdMenu.AppendMenu(MF_SEPARATOR);

	addQueueList(QueueManager::getInstance()->lockQueue());
	QueueManager::getInstance()->unlockQueue();
	QueueManager::getInstance()->addListener(this);

	memset(statusSizes, 0, sizeof(statusSizes));
	statusSizes[0] = 16;
	ctrlStatus.SetParts(6, statusSizes);
	updateStatus();

	bHandled = FALSE;
	return 1;
}

void QueueFrame::QueueItemInfo::update() {
	if(display != NULL) {
		int colMask = updateMask;
		updateMask = 0;

		if(colMask & MASK_TARGET) {
			display->columns[COLUMN_TARGET] = Util::getFileName(getTarget());
		}
		int online = 0;
		if(colMask & MASK_USERS || colMask & MASK_STATUS) {
			tstring tmp;

			SourceIter j;
			for(j = getSources().begin(); j != getSources().end(); ++j) {
				if(tmp.size() > 0)
					tmp += _T(", ");

				if(j->getUser()->isOnline())
					online++;

				tmp += Text::toT(j->getUser()->getFirstNick());
			}
			display->columns[COLUMN_USERS] = tmp.empty() ? TSTRING(NO_USERS) : tmp;
		}
		if(colMask & MASK_STATUS) {
			if(getStatus() == QueueItem::STATUS_WAITING) {

				TCHAR buf[64];
				if(online > 0) {
					if(getSources().size() == 1) {
						display->columns[COLUMN_STATUS] = TSTRING(WAITING_USER_ONLINE);
					} else {
						_stprintf(buf, CTSTRING(WAITING_USERS_ONLINE), online, getSources().size());
						display->columns[COLUMN_STATUS] = buf;
					}
				} else {
					if(getSources().size() == 0) {
						display->columns[COLUMN_STATUS] = TSTRING(NO_USERS_TO_DOWNLOAD_FROM);
					} else if(getSources().size() == 1) {
						display->columns[COLUMN_STATUS] = TSTRING(USER_OFFLINE);
					} else if(getSources().size() == 2) {
						display->columns[COLUMN_STATUS] = TSTRING(BOTH_USERS_OFFLINE);
					} else if(getSources().size() == 3) {
						display->columns[COLUMN_STATUS] = TSTRING(ALL_3_USERS_OFFLINE);
					} else if(getSources().size() == 4) {
						display->columns[COLUMN_STATUS] = TSTRING(ALL_4_USERS_OFFLINE);
					} else {
						_stprintf(buf, CTSTRING(ALL_USERS_OFFLINE), getSources().size());
						display->columns[COLUMN_STATUS] = buf;
					}
				}
			} else if(getStatus() == QueueItem::STATUS_RUNNING) {
				display->columns[COLUMN_STATUS] = TSTRING(RUNNING);
			} 
		}
		if(colMask & MASK_SIZE) {
			display->columns[COLUMN_SIZE] = (getSize() == -1) ? TSTRING(UNKNOWN) : Text::toT(Util::formatBytes(getSize()));
			display->columns[COLUMN_EXACT_SIZE] = (getSize() == -1) ? TSTRING(UNKNOWN) : Text::toT(Util::formatExactSize(getSize()));
		}
		if(colMask & MASK_DOWNLOADED) {
			if(getSize() > 0)
				display->columns[COLUMN_DOWNLOADED] = Text::toT(Util::formatBytes(getDownloadedBytes()) + " (" + Util::toString((double)getDownloadedBytes()*100.0/(double)getSize()) + "%)");
			else
				display->columns[COLUMN_DOWNLOADED].clear();
		}
		if(colMask & MASK_PRIORITY) {
			switch(getPriority()) {
		case QueueItem::PAUSED: display->columns[COLUMN_PRIORITY] = TSTRING(PAUSED); break;
		case QueueItem::LOWEST: display->columns[COLUMN_PRIORITY] = TSTRING(LOWEST); break;
		case QueueItem::LOW: display->columns[COLUMN_PRIORITY] = TSTRING(LOW); break;
		case QueueItem::NORMAL: display->columns[COLUMN_PRIORITY] = TSTRING(NORMAL); break;
		case QueueItem::HIGH: display->columns[COLUMN_PRIORITY] = TSTRING(HIGH); break;
		case QueueItem::HIGHEST: display->columns[COLUMN_PRIORITY] = TSTRING(HIGHEST); break;
		default: dcasserta(0); break;
			}
		}

		if(colMask & MASK_PATH) {
			display->columns[COLUMN_PATH] = Util::getFilePath(getTarget());
		}

		if(colMask & MASK_ERRORS) {
			tstring tmp;
			SourceIter j;
			for(j = getBadSources().begin(); j != getBadSources().end(); ++j) {
				if(!j->isSet(QueueItem::Source::FLAG_REMOVED)) {
					if(tmp.size() > 0)
						tmp += _T(", ");
						tmp += Text::toT(j->getUser()->getFirstNick());
					tmp += _T(" (");
					if(j->isSet(QueueItem::Source::FLAG_FILE_NOT_AVAILABLE)) {
						tmp += TSTRING(FILE_NOT_AVAILABLE);
					} else if(j->isSet(QueueItem::Source::FLAG_PASSIVE)) {
						tmp += TSTRING(PASSIVE_USER);
					} else if(j->isSet(QueueItem::Source::FLAG_ROLLBACK_INCONSISTENCY)) {
						tmp += TSTRING(ROLLBACK_INCONSISTENCY);
					} else if(j->isSet(QueueItem::Source::FLAG_CRC_FAILED)) {
						tmp += TSTRING(SFV_INCONSISTENCY);
					} else if(j->isSet(QueueItem::Source::FLAG_BAD_TREE)) {
						tmp += TSTRING(INVALID_TREE);
					} else if(j->isSet(QueueItem::Source::FLAG_SLOW_SOURCE)) {
						tmp += TSTRING(SOURCE_TOO_SLOW);
					}
					tmp += ')';
				}
			}
			display->columns[COLUMN_ERRORS] = tmp.empty() ? TSTRING(NO_ERRORS) : tmp;
		}

		if(colMask & MASK_ADDED) {
			display->columns[COLUMN_ADDED] = Text::toT(Util::formatTime("%Y-%m-%d %H:%M", getAdded()));
		}
		if(colMask & MASK_TTH && getTTH() != NULL) {
			display->columns[COLUMN_TTH] = Text::toT(getTTH()->toBase32());
		}
		if(colMask & MASK_TYPE) {
			display->columns[COLUMN_TYPE] = Util::getFileExt(getTarget());
			if(display->columns[COLUMN_TYPE].size() > 0 && display->columns[COLUMN_TYPE][0] == '.')
				display->columns[COLUMN_TYPE].erase(0, 1);
		}
	}
}

void QueueFrame::on(QueueManagerListener::Added, QueueItem* aQI) {
	QueueItemInfo* ii = new QueueItemInfo(aQI);
	{
		Lock l(cs);
		dcassert(queue.find(aQI) == queue.end());
		queue[aQI] = ii;
	}

	speak(ADD_ITEM,	ii);
}

void QueueFrame::addQueueItem(QueueItemInfo* ii, bool noSort) {
	if(!ii->isSet(QueueItem::FLAG_USER_LIST)) {
		queueSize+=ii->getSize();
	}
	queueItems++;
	dirty = true;
	
	const tstring& dir = ii->getPath();
	
	bool updateDir = (directories.find(dir) == directories.end());
	directories.insert(make_pair(dir, ii));
	
	if(updateDir) {
		addDirectory(dir, ii->isSet(QueueItem::FLAG_USER_LIST));
	} 
	if(!showTree || isCurDir(dir)) {
		ii->update();
		if(noSort)
			ctrlQueue.insertItem(ctrlQueue.GetItemCount(), ii, WinUtil::getIconIndex(ii->getTarget()));
		else
			ctrlQueue.insertItem(ii, WinUtil::getIconIndex(ii->getTarget()));
	}
}

void QueueFrame::addQueueList(const QueueItem::StringMap& li) {
	ctrlQueue.SetRedraw(FALSE);
	ctrlDirs.SetRedraw(FALSE);
	for(QueueItem::StringMap::const_iterator j = li.begin(); j != li.end(); ++j) {
		QueueItem* aQI = j->second;
		QueueItemInfo* ii = new QueueItemInfo(aQI);
		dcassert(queue.find(aQI) == queue.end());
		queue[aQI] = ii;
		addQueueItem(ii, true);
	}
	ctrlQueue.resort();
	ctrlQueue.SetRedraw(TRUE);
	ctrlDirs.SetRedraw(TRUE);
	ctrlDirs.Invalidate();
}

LRESULT QueueFrame::onKeyDown(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NMLVKEYDOWN* kd = (NMLVKEYDOWN*) pnmh;
	if(kd->wVKey == VK_DELETE) {
		removeSelected();
	} else if(kd->wVKey == VK_ADD){
		// Increase Item priority
		changePriority(true);
	} else if(kd->wVKey == VK_SUBTRACT){
		// Decrease item priority
		changePriority(false);
	} else if(kd->wVKey == VK_TAB) {
		onTab();
	}
	return 0;
}


HTREEITEM QueueFrame::addDirectory(const tstring& dir, bool isFileList /* = false */, HTREEITEM startAt /* = NULL */) {
	TVINSERTSTRUCT tvi;
	tvi.hInsertAfter = TVI_SORT;
	tvi.item.mask = TVIF_IMAGE | TVIF_PARAM | TVIF_SELECTEDIMAGE | TVIF_TEXT;
	tvi.item.iImage = tvi.item.iSelectedImage = WinUtil::getDirIconIndex();

	if(isFileList) {
		// We assume we haven't added it yet, and that all filelists go to the same
		// directory...
		dcassert(fileLists == NULL);
		tvi.hParent = NULL;
		tvi.item.pszText = FILE_LIST_NAME;
		tvi.item.lParam = (LPARAM) new tstring(dir);
		fileLists = ctrlDirs.InsertItem(&tvi);
		return fileLists;
	} 

	// More complicated, we have to find the last available tree item and then see...
	string::size_type i = 0;
	string::size_type j;

	HTREEITEM next = NULL;
	HTREEITEM parent = NULL;

	if(startAt == NULL) {
		// First find the correct drive letter
		dcassert(dir[1] == ':');
		dcassert(dir[2] == '\\');

		next = ctrlDirs.GetRootItem();

		while(next != NULL) {
			if(next != fileLists) {
				tstring* stmp = (tstring*)ctrlDirs.GetItemData(next);
				if(Util::strnicmp(*stmp, dir, 3) == 0)
					break;
			}
			next = ctrlDirs.GetNextSiblingItem(next);
		}

		if(next == NULL) {
			// First addition, set commonStart to the dir minus the last part...
			i = dir.rfind('\\', dir.length()-2);
			if(i != tstring::npos) {
				tstring name = dir.substr(0, i);
				tvi.hParent = NULL;
				tvi.item.pszText = const_cast<TCHAR*>(name.c_str());
				tvi.item.lParam = (LPARAM)new tstring(dir.substr(0, i+1));
				next = ctrlDirs.InsertItem(&tvi);
			} else {
				dcassert(dir.length() == 3);
				tvi.hParent = NULL;
				tvi.item.pszText = const_cast<TCHAR*>(dir.c_str());
				tvi.item.lParam = (LPARAM)new tstring(dir);
				next = ctrlDirs.InsertItem(&tvi);
			}
		} 
		
		// Ok, next now points to x:\... find how much is common

		tstring* rootStr = (tstring*)ctrlDirs.GetItemData(next);
		
		i = 0;

		for(;;) {
			j = dir.find('\\', i);
			if(j == string::npos)
				break;
			if(Util::strnicmp(dir.c_str() + i, rootStr->c_str() + i, j - i + 1) != 0)
				break;
			i = j + 1;
		}
		
		if(i < rootStr->length()) {
			HTREEITEM oldRoot = next;

			// Create a new root
			tstring name = rootStr->substr(0, i-1);
			tvi.hParent = NULL;
			tvi.item.pszText = const_cast<TCHAR*>(name.c_str());
			tvi.item.lParam = (LPARAM)new tstring(rootStr->substr(0, i));
			HTREEITEM newRoot = ctrlDirs.InsertItem(&tvi);

			parent = addDirectory(*rootStr, false, newRoot);

			next = ctrlDirs.GetChildItem(oldRoot);
			while(next != NULL) {
				moveNode(next, parent);
				next = ctrlDirs.GetChildItem(oldRoot);
			}
			delete rootStr;
			ctrlDirs.DeleteItem(oldRoot);
			parent = newRoot;
		} else {
			// Use this root as parent
			parent = next;
			next = ctrlDirs.GetChildItem(parent);
		}
	} else {
		parent = startAt;
		next = ctrlDirs.GetChildItem(parent);
		i = getDir(parent).length();
		dcassert(Util::strnicmp(getDir(parent), dir, getDir(parent).length()) == 0);
	}

	HTREEITEM firstParent = parent;

	while( i < dir.length() ) {
		while(next != NULL) {
			if(next != fileLists) {
				const tstring& n = getDir(next);
				if(Util::strnicmp(n.c_str()+i, dir.c_str()+i, n.length()-i) == 0) {
					// Found a part, we assume it's the best one we can find...
					i = n.length();

					parent = next;
					next = ctrlDirs.GetChildItem(next);
					break;
				}
			}
			next = ctrlDirs.GetNextSiblingItem(next);
		}

		if(next == NULL) {
			// We didn't find it, add...
			j = dir.find('\\', i);
			dcassert(j != string::npos);
			tstring name = dir.substr(i, j-i);
			tvi.hParent = parent;
			tvi.item.pszText = const_cast<TCHAR*>(name.c_str());
			tvi.item.lParam = (LPARAM) new tstring(dir.substr(0, j+1));
			
			parent = ctrlDirs.InsertItem(&tvi);
			
			i = j + 1;
		}
	}

	if(firstParent != NULL)
		ctrlDirs.Expand(firstParent);

	return parent;
}

void QueueFrame::removeDirectory(const tstring& dir, bool isFileList /* = false */) {

	// First, find the last name available
	string::size_type i = 0;

	HTREEITEM next = ctrlDirs.GetRootItem();
	HTREEITEM parent = NULL;
	
	if(isFileList) {
		dcassert(fileLists != NULL);
		delete (string*)ctrlDirs.GetItemData(fileLists);
		ctrlDirs.DeleteItem(fileLists);
		fileLists = NULL;
		return;
	} else {
		while(i < dir.length()) {
			while(next != NULL) {
				if(next != fileLists) {
					const tstring& n = getDir(next);
					if(Util::strnicmp(n.c_str()+i, dir.c_str()+i, n.length()-i) == 0) {
						// Match!
						parent = next;
						next = ctrlDirs.GetChildItem(next);
						i = n.length();
						break;
					}
				}
				next = ctrlDirs.GetNextSiblingItem(next);
			}
			if(next == NULL)
				break;
		}
	}

	next = parent;

	while((ctrlDirs.GetChildItem(next) == NULL) && (directories.find(getDir(next)) == directories.end())) {
		delete (string*)ctrlDirs.GetItemData(next);
		parent = ctrlDirs.GetParentItem(next);
		
		ctrlDirs.DeleteItem(next);
		if(parent == NULL)
			break;
		next = parent;
	}
}

void QueueFrame::removeDirectories(HTREEITEM ht) {
	HTREEITEM next = ctrlDirs.GetChildItem(ht);
	while(next != NULL) {
		removeDirectories(next);
		next = ctrlDirs.GetNextSiblingItem(ht);
	}
	delete (string*)ctrlDirs.GetItemData(ht);
	ctrlDirs.DeleteItem(ht);
}

void QueueFrame::on(QueueManagerListener::Removed, QueueItem* aQI) {
	QueueItemInfo* qi = NULL;
	{
		Lock l(cs);
		QueueIter i = queue.find(aQI);
		dcassert(i != queue.end());
		qi = i->second;
		queue.erase(i);

		dirty = true;
	}

	speak(REMOVE_ITEM, qi);
}

void QueueFrame::on(QueueManagerListener::Moved, QueueItem* aQI) {
	QueueItemInfo* qi = NULL;
	QueueItemInfo* qi2 = new QueueItemInfo(aQI);
	{
		Lock l(cs);
		dcassert(queue.find(aQI) != queue.end());
		QueueIter i = queue.find(aQI);
		qi = i->second;
		i->second = qi2;
	}
	
	speak(REMOVE_ITEM, qi);
	speak(ADD_ITEM,	qi2);
}

void QueueFrame::on(QueueManagerListener::SourcesUpdated, QueueItem* aQI) {
	QueueItemInfo* ii = NULL;
	{
		Lock l(cs);
		dcassert(queue.find(aQI) != queue.end());
		ii = queue[aQI];

		ii->setPriority(aQI->getPriority());
		ii->setStatus(aQI->getStatus());
		ii->setDownloadedBytes(aQI->getDownloadedBytes());
		ii->setTTH(aQI->getTTH());

		{
			for(QueueItemInfo::SourceIter i = ii->getSources().begin(); i != ii->getSources().end(); ) {
				if(!aQI->isSource(i->getUser())) {
					i = ii->getSources().erase(i);
				} else {
					++i;
				}
			}
			for(QueueItem::Source::Iter j = aQI->getSources().begin(); j != aQI->getSources().end(); ++j) {
				if(!ii->isSource((*j)->getUser())) {
					ii->getSources().push_back(QueueItemInfo::SourceInfo(*(*j)));
				}
			}
		}
		{
			for(QueueItemInfo::SourceIter i = ii->getBadSources().begin(); i != ii->getBadSources().end(); ) {
				if(!aQI->isBadSource(i->getUser())) {
					i = ii->getBadSources().erase(i);
				} else {
					++i;
				}
			}
			for(QueueItem::Source::Iter j = aQI->getBadSources().begin(); j != aQI->getBadSources().end(); ++j) {
				if(!ii->isBadSource((*j)->getUser())) {
					ii->getBadSources().push_back(QueueItemInfo::SourceInfo(*(*j)));
				}
			}
		}
		ii->updateMask |= QueueItemInfo::MASK_PRIORITY | QueueItemInfo::MASK_USERS | QueueItemInfo::MASK_ERRORS | QueueItemInfo::MASK_STATUS | QueueItemInfo::MASK_DOWNLOADED | QueueItemInfo::MASK_TTH;
	}

	speak(UPDATE_ITEM, ii);
}

LRESULT QueueFrame::onSpeaker(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	Lock l(cs);
	spoken = false;

	for(TaskIter ti = tasks.begin(); ti != tasks.end(); ++ti) {
		if(ti->first == ADD_ITEM) {
			QueueItemInfo* ii = (QueueItemInfo*)ti->second;
			dcassert(ctrlQueue.findItem(ii) == -1);
			addQueueItem(ii, false);
			updateStatus();
		} else if(ti->first == REMOVE_ITEM) {
			QueueItemInfo* ii = (QueueItemInfo*)ti->second;
			
			if(!showTree || isCurDir(ii->getPath()) ) {
				dcassert(ctrlQueue.findItem(ii) != -1);
				ctrlQueue.deleteItem(ii);
			}

			if(!ii->isSet(QueueItem::FLAG_USER_LIST)) {
				queueSize-=ii->getSize();
				dcassert(queueSize >= 0);
			}
			queueItems--;
			dcassert(queueItems >= 0);
			
			pair<DirectoryIter, DirectoryIter> i = directories.equal_range(ii->getPath());
			DirectoryIter j;
			for(j = i.first; j != i.second; ++j) {
				if(j->second == ii)
					break;
			}
			dcassert(j != i.second);
			directories.erase(j);
			if(directories.count(ii->getPath()) == 0) {
				removeDirectory(ii->getPath(), ii->isSet(QueueItem::FLAG_USER_LIST));
				if(isCurDir(ii->getPath()))
					curDir = Util::emptyStringT;
			}
			
			delete ii;
			updateStatus();
			if (BOOLSETTING(QUEUE_DIRTY)) {
				setDirty();
			}
			dirty = true;
		} else if(ti->first == UPDATE_ITEM) {
			QueueItemInfo* ii = (QueueItemInfo*)ti->second;
			if(!showTree || isCurDir(ii->getPath())) {
				dcassert(ctrlQueue.findItem(ii) != -1);
				ii->update();
				ctrlQueue.updateItem(ii);
			}
		}
	}
	if(tasks.size() > 0) {
		tasks.clear();
	}

	return 0;
}

void QueueFrame::moveSelected() {

	int n = ctrlQueue.GetSelectedCount();
	if(n == 1) {
		// Single file, get the full filename and move...
		QueueItemInfo* ii = ctrlQueue.getItemData(ctrlQueue.GetNextItem(-1, LVNI_SELECTED));
		tstring name = ii->getTarget();
		tstring ext = Util::getFileExt(name);
		tstring ext2;
		if (!ext.empty())
		{
			ext = ext.substr(1); // remove leading dot so default extension works when browsing for file
			ext2 = _T("*.") + ext;
		ext2 += (TCHAR)0;
		ext2 += _T("*.") + ext;
		}
		ext2 += _T("*.*");
		ext2 += (TCHAR)0;
		ext2 += _T("*.*");
		ext2 += (TCHAR)0;

		if(WinUtil::browseFile(name, m_hWnd, true, ii->getPath(), ext2.c_str(), ext.empty() ? NULL : ext.c_str())) {
			QueueManager::getInstance()->move(Text::fromT(ii->getTarget()), Text::fromT(name));
		}
	} else if(n > 1) {
		tstring name;
		if(showTree) {
			name = curDir;
		}

		if(WinUtil::browseDirectory(name, m_hWnd)) {
			int i = -1;
			while( (i = ctrlQueue.GetNextItem(i, LVNI_SELECTED)) != -1) {
				QueueItemInfo* ii = ctrlQueue.getItemData(i);
				QueueManager::getInstance()->move(Text::fromT(ii->getTarget()), Text::fromT(name + ii->getTargetFileName()));
			}			
		}
	}
}

void QueueFrame::moveSelectedDir() {
	if(ctrlDirs.GetSelectedItem() == NULL)
		return;

	dcassert(!curDir.empty());
	tstring name = curDir;
	
	if(WinUtil::browseDirectory(name, m_hWnd)) {
		moveDir(ctrlDirs.GetSelectedItem(), name);
	}
}

void QueueFrame::moveDir(HTREEITEM ht, const tstring& target) {
	HTREEITEM next = ctrlDirs.GetChildItem(ht);
	while(next != NULL) {
		moveDir(next, target + Util::getLastDir(getDir(next)) + _T(PATH_SEPARATOR_STR));		
		next = ctrlDirs.GetNextSiblingItem(next);
	}
	tstring* s = (tstring*)ctrlDirs.GetItemData(ht);

	DirectoryPair p = directories.equal_range(*s);
	
	for(DirectoryIter i = p.first; i != p.second; ++i) {
		QueueItemInfo* qi = i->second;
		QueueManager::getInstance()->move(Text::fromT(qi->getTarget()), Text::fromT(target + qi->getTargetFileName()));
	}			
}

LRESULT QueueFrame::onContextMenu(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	if (reinterpret_cast<HWND>(wParam) == ctrlQueue && ctrlQueue.GetSelectedCount() > 0) { 
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		
		if(pt.x == -1 && pt.y == -1) {
			WinUtil::getContextMenuPos(ctrlQueue, pt);
		}

		usingDirMenu = false;
		CMenuItemInfo mi;
		
		while(browseMenu.GetMenuItemCount() > 0) {
			browseMenu.RemoveMenu(0, MF_BYPOSITION);
		}
		while(removeMenu.GetMenuItemCount() > 2) {
			removeMenu.RemoveMenu(2, MF_BYPOSITION);
		}
		while(removeAllMenu.GetMenuItemCount() > 0) {
			removeAllMenu.RemoveMenu(0, MF_BYPOSITION);
		}
		while(pmMenu.GetMenuItemCount() > 0) {
			pmMenu.RemoveMenu(0, MF_BYPOSITION);
		}
		while(readdMenu.GetMenuItemCount() > 2) {
			readdMenu.RemoveMenu(2, MF_BYPOSITION);
		}

		if(ctrlQueue.GetSelectedCount() == 1) {
			QueueItemInfo* ii = ctrlQueue.getItemData(ctrlQueue.GetNextItem(-1, LVNI_SELECTED));
			menuItems = 0;
			int pmItems = 0;
			QueueItemInfo::SourceIter i;
			for(i = ii->getSources().begin(); i != ii->getSources().end(); ++i) {
				tstring nick = Text::toT(i->getUser()->getFirstNick());
				mi.fMask = MIIM_ID | MIIM_TYPE | MIIM_DATA;
				mi.fType = MFT_STRING;
				mi.dwTypeData = (LPTSTR)nick.c_str();
				mi.dwItemData = (ULONG_PTR)&(*i);
				mi.wID = IDC_BROWSELIST + menuItems;
				browseMenu.InsertMenuItem(menuItems, TRUE, &mi);
				mi.wID = IDC_REMOVE_SOURCE + 1 + menuItems; // "All" is before sources
				removeMenu.InsertMenuItem(menuItems + 2, TRUE, &mi); // "All" and separator come first
				mi.wID = IDC_REMOVE_SOURCES + menuItems;
				removeAllMenu.InsertMenuItem(menuItems, TRUE, &mi);
				if(i->getUser()->isOnline()) {
					mi.wID = IDC_PM + menuItems;
					pmMenu.InsertMenuItem(menuItems, TRUE, &mi);
					pmItems++;
				}
				menuItems++;
			}
			readdItems = 0;
			for(i = ii->getBadSources().begin(); i != ii->getBadSources().end(); ++i) {
				tstring nick = Text::toT(i->getUser()->getFirstNick());
				mi.fMask = MIIM_ID | MIIM_TYPE | MIIM_DATA;
				mi.fType = MFT_STRING;
				mi.dwTypeData = (LPTSTR)nick.c_str();
				mi.dwItemData = (ULONG_PTR)&(*i);
				mi.wID = IDC_READD + 1 + readdItems;  // "All" is before sources
				readdMenu.InsertMenuItem(readdItems + 2, TRUE, &mi);  // "All" and separator come first
				readdItems++;
			}

			if(menuItems == 0) {
				singleMenu.EnableMenuItem((UINT_PTR)(HMENU)browseMenu, MFS_GRAYED);
				singleMenu.EnableMenuItem((UINT_PTR)(HMENU)removeMenu, MFS_GRAYED);
				singleMenu.EnableMenuItem((UINT_PTR)(HMENU)removeAllMenu, MFS_GRAYED);
			}
			else {
				singleMenu.EnableMenuItem((UINT_PTR)(HMENU)browseMenu, MFS_ENABLED);
				singleMenu.EnableMenuItem((UINT_PTR)(HMENU)removeMenu, MFS_ENABLED);
				singleMenu.EnableMenuItem((UINT_PTR)(HMENU)removeAllMenu, MFS_ENABLED);
			}

			if(pmItems == 0) {
				singleMenu.EnableMenuItem((UINT_PTR)(HMENU)pmMenu, MFS_GRAYED);
			}
			else {
				singleMenu.EnableMenuItem((UINT_PTR)(HMENU)pmMenu, MFS_ENABLED);
			}

			if(readdItems == 0) {
				singleMenu.EnableMenuItem((UINT_PTR)(HMENU)readdMenu, MFS_GRAYED);
			}
			else {
				singleMenu.EnableMenuItem((UINT_PTR)(HMENU)readdMenu, MFS_ENABLED);
 			}

			if(ii->getTTH() == NULL) {
				singleMenu.EnableMenuItem(IDC_BITZI_LOOKUP, MFS_GRAYED);
				singleMenu.EnableMenuItem(IDC_COPY_MAGNET, MFS_GRAYED);
			} else {
				singleMenu.EnableMenuItem(IDC_BITZI_LOOKUP, MFS_ENABLED);
				singleMenu.EnableMenuItem(IDC_COPY_MAGNET, MFS_ENABLED);
			}
			
			singleMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
		} else {
			multiMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
		}
		
		return TRUE; 
	} else if (reinterpret_cast<HWND>(wParam) == ctrlDirs && ctrlDirs.GetSelectedItem() != NULL) { 
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		
		if(pt.x == -1 && pt.y == -1) {
			WinUtil::getContextMenuPos(ctrlDirs, pt);
		} else {
			// Strange, windows doesn't change the selection on right-click... (!)
			UINT a = 0;
			ctrlDirs.ScreenToClient(&pt);
			HTREEITEM ht = ctrlDirs.HitTest(pt, &a);
			if(ht != NULL && ht != ctrlDirs.GetSelectedItem())
				ctrlDirs.SelectItem(ht);
			ctrlDirs.ClientToScreen(&pt);
		}
		usingDirMenu = true;
		
		dirMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
	
		return TRUE;
	}

	bHandled = FALSE;
	return FALSE; 
}

LRESULT QueueFrame::onSearchAlternates(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(ctrlQueue.GetSelectedCount() == 1) {
		int i = ctrlQueue.GetNextItem(-1, LVNI_SELECTED);
		QueueItemInfo* ii = ctrlQueue.getItemData(i);

		if(ii->getTTH() != NULL) {
			WinUtil::searchHash(ii->getTTH());
		} else {
			tstring searchString = Text::toT(SearchManager::clean(Text::fromT(ii->getTargetFileName())));

			if(!searchString.empty()) {
				bool bigFile = (ii->getSize() > 10*1024*1024);
				if(bigFile) {
					SearchFrame::openWindow(searchString, ii->getSize()-1, SearchManager::SIZE_ATLEAST, ShareManager::getInstance()->getType(Text::fromT(ii->getTargetFileName())));
				} else {
					SearchFrame::openWindow(searchString, ii->getSize()+1, SearchManager::SIZE_ATMOST, ShareManager::getInstance()->getType(Text::fromT(ii->getTargetFileName())));
				}
			}
		}
	}
	return 0;
}

LRESULT QueueFrame::onBitziLookup(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(ctrlQueue.GetSelectedCount() == 1) {
		int i = ctrlQueue.GetNextItem(-1, LVNI_SELECTED);
		QueueItemInfo* ii = ctrlQueue.getItemData(i);
		WinUtil::bitziLink(ii->getTTH());
	}
	return 0;
}

LRESULT QueueFrame::onCopyMagnet(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(ctrlQueue.GetSelectedCount() == 1) {
		int i = ctrlQueue.GetNextItem(-1, LVNI_SELECTED);
		QueueItemInfo* ii = ctrlQueue.getItemData(i);
		WinUtil::copyMagnet(ii->getTTH(), ii->getTargetFileName());
	}
	return 0;
}

LRESULT QueueFrame::onBrowseList(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	
	if(ctrlQueue.GetSelectedCount() == 1) {
		CMenuItemInfo mi;
		mi.fMask = MIIM_DATA;
		
		browseMenu.GetMenuItemInfo(wID, FALSE, &mi);
		QueueItemInfo::SourceInfo* s = (QueueItemInfo::SourceInfo*)mi.dwItemData;
		try {
			QueueManager::getInstance()->addList(s->getUser(), QueueItem::FLAG_CLIENT_VIEW);
		} catch(const Exception&) {
		}
	}
	return 0;
}

LRESULT QueueFrame::onReadd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	
	if(ctrlQueue.GetSelectedCount() == 1) {
		int i = ctrlQueue.GetNextItem(-1, LVNI_SELECTED);
		QueueItemInfo* ii = ctrlQueue.getItemData(i);

		CMenuItemInfo mi;
		mi.fMask = MIIM_DATA;
		
		readdMenu.GetMenuItemInfo(wID, FALSE, &mi);
		if(wID == IDC_READD) {
			// re-add all sources
			for(QueueItemInfo::SourceIter s = ii->getBadSources().begin(); s != ii->getBadSources().end(); ) {
				QueueManager::getInstance()->readd(Text::fromT(ii->getTarget()), s->getUser());
			}
		} else {
			QueueItemInfo::SourceInfo* s = (QueueItemInfo::SourceInfo*)mi.dwItemData;
			try {
				QueueManager::getInstance()->readd(Text::fromT(ii->getTarget()), s->getUser());
			} catch(const Exception& e) {
				ctrlStatus.SetText(0, Text::toT(e.getError()).c_str());
			}
		}
	}
	return 0;
}

LRESULT QueueFrame::onRemoveSource(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	
	if(ctrlQueue.GetSelectedCount() == 1) {
		int i = ctrlQueue.GetNextItem(-1, LVNI_SELECTED);
		QueueItemInfo* ii = ctrlQueue.getItemData(i);
		if(wID == IDC_REMOVE_SOURCE) {
			for(QueueItemInfo::SourceIter si = ii->getSources().begin(); si != ii->getSources().end(); ) {
				QueueManager::getInstance()->removeSource(Text::fromT(ii->getTarget()), si->getUser(), QueueItem::Source::FLAG_REMOVED);
			}
		} else {
			CMenuItemInfo mi;
			mi.fMask = MIIM_DATA;

			removeMenu.GetMenuItemInfo(wID, FALSE, &mi);
			QueueItemInfo::SourceInfo* s = (QueueItemInfo::SourceInfo*)mi.dwItemData;
			QueueManager::getInstance()->removeSource(Text::fromT(ii->getTarget()), s->getUser(), QueueItem::Source::FLAG_REMOVED);
		}
	}
	return 0;
}

LRESULT QueueFrame::onRemoveSources(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	CMenuItemInfo mi;
	mi.fMask = MIIM_DATA;
	removeAllMenu.GetMenuItemInfo(wID, FALSE, &mi);
	QueueItemInfo::SourceInfo* s = (QueueItemInfo::SourceInfo*)mi.dwItemData;
	QueueManager::getInstance()->removeSource(s->getUser(), QueueItem::Source::FLAG_REMOVED);
	return 0;
}

LRESULT QueueFrame::onPM(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(ctrlQueue.GetSelectedCount() == 1) {
		CMenuItemInfo mi;
		mi.fMask = MIIM_DATA;
		
		pmMenu.GetMenuItemInfo(wID, FALSE, &mi);
		QueueItemInfo::SourceInfo* s = (QueueItemInfo::SourceInfo*)mi.dwItemData;
		PrivateFrame::openWindow(s->getUser());
	}
	return 0;
}
LRESULT QueueFrame::onPriority(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	QueueItem::Priority p;

	switch(wID) {
		case IDC_PRIORITY_PAUSED: p = QueueItem::PAUSED; break;
		case IDC_PRIORITY_LOWEST: p = QueueItem::LOWEST; break;
		case IDC_PRIORITY_LOW: p = QueueItem::LOW; break;
		case IDC_PRIORITY_NORMAL: p = QueueItem::NORMAL; break;
		case IDC_PRIORITY_HIGH: p = QueueItem::HIGH; break;
		case IDC_PRIORITY_HIGHEST: p = QueueItem::HIGHEST; break;
		default: p = QueueItem::DEFAULT; break;
	}

	if(usingDirMenu) {
		setPriority(ctrlDirs.GetSelectedItem(), p);
	} else {
		int i = -1;
		while( (i = ctrlQueue.GetNextItem(i, LVNI_SELECTED)) != -1) {
			QueueManager::getInstance()->setPriority(Text::fromT(ctrlQueue.getItemData(i)->getTarget()), p);
		}
	}

	return 0;
}

void QueueFrame::removeDir(HTREEITEM ht) {
	if(ht == NULL)
		return;
	HTREEITEM child = ctrlDirs.GetChildItem(ht);
	while(child) {
		removeDir(child);
		child = ctrlDirs.GetNextSiblingItem(child);
	}
	const tstring& name = getDir(ht);
	DirectoryPair dp = directories.equal_range(name);
	for(DirectoryIter i = dp.first; i != dp.second; ++i) {
		QueueManager::getInstance()->remove(Text::fromT(i->second->getTarget()));
	}
}

/*
 * @param inc True = increase, False = decrease
 */
void QueueFrame::changePriority(bool inc){
	int i = -1;
	while( (i = ctrlQueue.GetNextItem(i, LVNI_SELECTED)) != -1){
		QueueItem::Priority p = ctrlQueue.getItemData(i)->getPriority();

		if ((inc && p == QueueItem::HIGHEST) || (!inc && p == QueueItem::PAUSED)){
			// Trying to go higher than HIGHEST or lower than PAUSED
			// so do nothing
			continue;
		}

		switch(p){
			case QueueItem::HIGHEST: p = QueueItem::HIGH; break;
			case QueueItem::HIGH:    p = inc ? QueueItem::HIGHEST : QueueItem::NORMAL; break;
			case QueueItem::NORMAL:  p = inc ? QueueItem::HIGH    : QueueItem::LOW; break;
			case QueueItem::LOW:     p = inc ? QueueItem::NORMAL  : QueueItem::LOWEST; break;
			case QueueItem::LOWEST:  p = inc ? QueueItem::LOW     : QueueItem::PAUSED; break;
			case QueueItem::PAUSED:  p = QueueItem::LOWEST; break;
		}

		QueueManager::getInstance()->setPriority(Text::fromT(ctrlQueue.getItemData(i)->getTarget()), p);
	}
}

void QueueFrame::setPriority(HTREEITEM ht, const QueueItem::Priority& p) {
	if(ht == NULL)
		return;
	HTREEITEM child = ctrlDirs.GetChildItem(ht);
	while(child) {
		setPriority(child, p);
		child = ctrlDirs.GetNextSiblingItem(child);
	}
	const tstring& name = getDir(ht);
	DirectoryPair dp = directories.equal_range(name);
	for(DirectoryIter i = dp.first; i != dp.second; ++i) {
		QueueManager::getInstance()->setPriority(Text::fromT(i->second->getTarget()), p);
	}
}

void QueueFrame::updateStatus() {
	if(ctrlStatus.IsWindow()) {
		int64_t total = 0;
		int cnt = ctrlQueue.GetSelectedCount();
		if(cnt == 0) {
			cnt = ctrlQueue.GetItemCount();
			if(showTree) {
				for(int i = 0; i < cnt; ++i) {
					QueueItemInfo* ii = ctrlQueue.getItemData(i);
					total += (ii->getSize() > 0) ? ii->getSize() : 0;
				}
			} else {
				total = queueSize;
			}
		} else {
			int i = -1;
			while( (i = ctrlQueue.GetNextItem(i, LVNI_SELECTED)) != -1) {
				QueueItemInfo* ii = ctrlQueue.getItemData(i);
				total += (ii->getSize() > 0) ? ii->getSize() : 0;
			}

		}

		tstring tmp1 = Text::toT(STRING(ITEMS) + ": " + Util::toString(cnt));
		tstring tmp2 = Text::toT(STRING(SIZE) + ": " + Util::formatBytes(total));
		bool u = false;

		int w = WinUtil::getTextWidth(tmp1, ctrlStatus.m_hWnd);
		if(statusSizes[1] < w) {
			statusSizes[1] = w;
			u = true;
		}
		ctrlStatus.SetText(2, tmp1.c_str());
		w = WinUtil::getTextWidth(tmp2, ctrlStatus.m_hWnd);
		if(statusSizes[2] < w) {
			statusSizes[2] = w;
			u = true;
		}
		ctrlStatus.SetText(3, tmp2.c_str());

		if(dirty) {
			tmp1 = Text::toT(STRING(FILES) + ": " + Util::toString(queueItems));
			tmp2 = Text::toT(STRING(SIZE) + ": " + Util::formatBytes(queueSize));

			w = WinUtil::getTextWidth(tmp2, ctrlStatus.m_hWnd);
			if(statusSizes[3] < w) {
				statusSizes[3] = w;
				u = true;
			}
			ctrlStatus.SetText(4, tmp1.c_str());

			w = WinUtil::getTextWidth(tmp2, ctrlStatus.m_hWnd);
			if(statusSizes[4] < w) {
				statusSizes[4] = w;
				u = true;
			}
			ctrlStatus.SetText(5, tmp2.c_str());

			dirty = false;
		}

		if(u)
			UpdateLayout(TRUE);
	}
}

void QueueFrame::UpdateLayout(BOOL bResizeBars /* = TRUE */) {
	RECT rect;
	GetClientRect(&rect);
	// position bars and offset their dimensions
	UpdateBarsPosition(rect, bResizeBars);

	if(ctrlStatus.IsWindow()) {
		CRect sr;
		int w[6];
		ctrlStatus.GetClientRect(sr);
		w[5] = sr.right - 16;
#define setw(x) w[x] = max(w[x+1] - statusSizes[x], 0)
		setw(4); setw(3); setw(2); setw(1);

		w[0] = 16;

		ctrlStatus.SetParts(6, w);

		ctrlStatus.GetRect(0, sr);
		ctrlShowTree.MoveWindow(sr);
	}

	if(showTree) {
		if(GetSinglePaneMode() != SPLIT_PANE_NONE) {
			SetSinglePaneMode(SPLIT_PANE_NONE);
			updateQueue();
		}
	} else {
		if(GetSinglePaneMode() != SPLIT_PANE_RIGHT) {
			SetSinglePaneMode(SPLIT_PANE_RIGHT);
			updateQueue();
		}
	}
	
	CRect rc = rect;
	SetSplitterRect(rc);
}

LRESULT QueueFrame::onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	if(!closed) {
		QueueManager::getInstance()->removeListener(this);

		closed = true;
		PostMessage(WM_CLOSE);
		return 0;
	} else {
		HTREEITEM ht = ctrlDirs.GetRootItem();
		while(ht != NULL) {
			clearTree(ht);
			ht = ctrlDirs.GetNextSiblingItem(ht);
		}

		SettingsManager::getInstance()->set(SettingsManager::QUEUEFRAME_SHOW_TREE, ctrlShowTree.GetCheck() == BST_CHECKED);
		{
			Lock l(cs);
			for(QueueIter i = queue.begin(); i != queue.end(); ++i) {
				delete i->second;
			}
			queue.clear();
		}
		ctrlQueue.DeleteAllItems();

		WinUtil::saveHeaderOrder(ctrlQueue, SettingsManager::QUEUEFRAME_ORDER, 
			SettingsManager::QUEUEFRAME_WIDTHS, COLUMN_LAST, columnIndexes, columnSizes);

		bHandled = FALSE;
		return 0;
	}
}

LRESULT QueueFrame::onItemChanged(int /*idCtrl*/, LPNMHDR /* pnmh */, BOOL& /*bHandled*/) {
	updateQueue();
	return 0;
}

void QueueFrame::updateQueue() {
	Lock l(cs);

	ctrlQueue.DeleteAllItems();
	pair<DirectoryIter, DirectoryIter> i;
	if(showTree) {
		i = directories.equal_range(getSelectedDir());
	} else {
		i.first = directories.begin();
		i.second = directories.end();
	}

	ctrlQueue.SetRedraw(FALSE);
	for(DirectoryIter j = i.first; j != i.second; ++j) {
		QueueItemInfo* ii = j->second;
		ii->update();
		ctrlQueue.insertItem(ctrlQueue.GetItemCount(), ii, WinUtil::getIconIndex(ii->getTarget()));
	}
	ctrlQueue.resort();
	ctrlQueue.SetRedraw(TRUE);
	curDir = getSelectedDir();
	updateStatus();
}

// Put it here to avoid a copy for each recursion...
static TCHAR tmpBuf[1024];
void QueueFrame::moveNode(HTREEITEM item, HTREEITEM parent) {
	TVINSERTSTRUCT tvis;
	memset(&tvis, 0, sizeof(tvis));
	tvis.itemex.hItem = item;
	tvis.itemex.mask = TVIF_CHILDREN | TVIF_HANDLE | TVIF_IMAGE | TVIF_INTEGRAL | TVIF_PARAM |
		TVIF_SELECTEDIMAGE | TVIF_STATE | TVIF_TEXT;
	tvis.itemex.pszText = tmpBuf;
	tvis.itemex.cchTextMax = 1024;
	ctrlDirs.GetItem((TVITEM*)&tvis.itemex);
	tvis.hInsertAfter =	TVI_SORT;
	tvis.hParent = parent;
	HTREEITEM ht = ctrlDirs.InsertItem(&tvis);
	HTREEITEM next = ctrlDirs.GetChildItem(item);
	while(next != NULL) {
		moveNode(next, ht);
		next = ctrlDirs.GetChildItem(item);
	}
	ctrlDirs.DeleteItem(item);
}

/**
 * @file
 * $Id: QueueFrame.cpp,v 1.80 2005/07/21 00:02:04 arnetheduck Exp $
 */

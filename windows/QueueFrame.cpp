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

#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "Resource.h"

#include "QueueFrame.h"
#include "SearchFrm.h"
#include "PrivateFrame.h"
#include "LineDlg.h"

#include "../client/StringTokenizer.h"
#include "../client/ShareManager.h"

#define FILE_LIST_NAME "File Lists"

int QueueFrame::columnIndexes[] = { COLUMN_TARGET, COLUMN_STATUS, COLUMN_SIZE, COLUMN_DOWNLOADED, COLUMN_PRIORITY,
COLUMN_USERS, COLUMN_PATH, COLUMN_EXACT_SIZE, COLUMN_ERRORS, COLUMN_SEARCHSTRING, COLUMN_ADDED, COLUMN_TTH };

int QueueFrame::columnSizes[] = { 200, 300, 75, 110, 75, 200, 200, 75, 200, 200, 100, 125 };

static ResourceManager::Strings columnNames[] = { ResourceManager::FILENAME, ResourceManager::STATUS, ResourceManager::SIZE, ResourceManager::DOWNLOADED,
ResourceManager::PRIORITY, ResourceManager::USERS, ResourceManager::PATH, ResourceManager::EXACT_SIZE, ResourceManager::ERRORS, ResourceManager::SEARCH_STRING,
ResourceManager::ADDED, ResourceManager::TTH_ROOT };

LRESULT QueueFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	showTree = BOOLSETTING(QUEUEFRAME_SHOW_TREE);
	
	CreateSimpleStatusBar(ATL_IDS_IDLEMESSAGE, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SBARS_SIZEGRIP);
	ctrlStatus.Attach(m_hWndStatusBar);
	
	ctrlQueue.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS, WS_EX_CLIENTEDGE, IDC_QUEUE);
	ctrlQueue.SetExtendedListViewStyle(LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT);
	

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
		ctrlQueue.InsertColumn(j, CSTRING_I(columnNames[j]), fmt, columnSizes[j], j);
	}
	
	ctrlQueue.SetColumnOrderArray(COLUMN_LAST, columnIndexes);
	ctrlQueue.setSortColumn(COLUMN_TARGET);
	
	ctrlQueue.SetBkColor(WinUtil::bgColor);
	ctrlQueue.SetTextBkColor(WinUtil::bgColor);
	ctrlQueue.SetTextColor(WinUtil::textColor);

	ctrlDirs.SetBkColor(WinUtil::bgColor);
	ctrlDirs.SetTextColor(WinUtil::textColor);

	ctrlShowTree.Create(ctrlStatus.m_hWnd, rcDefault, "+/-", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
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

	singleMenu.AppendMenu(MF_STRING, IDC_SEARCH_ALTERNATES, CSTRING(SEARCH_FOR_ALTERNATES));
	singleMenu.AppendMenu(MF_STRING, IDC_SEARCH_BY_TTH, CSTRING(SEARCH_BY_TTH));
	singleMenu.AppendMenu(MF_STRING, IDC_BITZI_LOOKUP, CSTRING(LOOKUP_AT_BITZI));
	singleMenu.AppendMenu(MF_STRING, IDC_COPY_MAGNET, CSTRING(COPY_MAGNET));
	singleMenu.AppendMenu(MF_STRING, IDC_SEARCH_STRING, CSTRING(ENTER_SEARCH_STRING));
	singleMenu.AppendMenu(MF_STRING, IDC_MOVE, CSTRING(MOVE));
	singleMenu.AppendMenu(MF_POPUP, (UINT)(HMENU)priorityMenu, CSTRING(SET_PRIORITY));
	singleMenu.AppendMenu(MF_POPUP, (UINT)(HMENU)browseMenu, CSTRING(GET_FILE_LIST));
	singleMenu.AppendMenu(MF_POPUP, (UINT)(HMENU)pmMenu, CSTRING(SEND_PRIVATE_MESSAGE));
	singleMenu.AppendMenu(MF_POPUP, (UINT)(HMENU)readdMenu, CSTRING(READD_SOURCE));
	singleMenu.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)NULL);
	singleMenu.AppendMenu(MF_POPUP, (UINT)(HMENU)removeMenu, CSTRING(REMOVE_SOURCE));
	singleMenu.AppendMenu(MF_POPUP, (UINT)(HMENU)removeAllMenu, CSTRING(REMOVE_FROM_ALL));
	singleMenu.AppendMenu(MF_STRING, IDC_REMOVE, CSTRING(REMOVE));

	multiMenu.AppendMenu(MF_POPUP, (UINT)(HMENU)priorityMenu, CSTRING(SET_PRIORITY));
	multiMenu.AppendMenu(MF_STRING, IDC_SEARCH_STRING, CSTRING(ENTER_SEARCH_STRING));
	multiMenu.AppendMenu(MF_STRING, IDC_MOVE, CSTRING(MOVE));
	multiMenu.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)NULL);
	multiMenu.AppendMenu(MF_STRING, IDC_REMOVE, CSTRING(REMOVE));
	
	priorityMenu.AppendMenu(MF_STRING, IDC_PRIORITY_PAUSED, CSTRING(PAUSED));
	priorityMenu.AppendMenu(MF_STRING, IDC_PRIORITY_LOWEST, CSTRING(LOWEST));
	priorityMenu.AppendMenu(MF_STRING, IDC_PRIORITY_LOW, CSTRING(LOW));
	priorityMenu.AppendMenu(MF_STRING, IDC_PRIORITY_NORMAL, CSTRING(NORMAL));
	priorityMenu.AppendMenu(MF_STRING, IDC_PRIORITY_HIGH, CSTRING(HIGH));
	priorityMenu.AppendMenu(MF_STRING, IDC_PRIORITY_HIGHEST, CSTRING(HIGHEST));

	dirMenu.AppendMenu(MF_POPUP, (UINT)(HMENU)priorityMenu, CSTRING(SET_PRIORITY));
	dirMenu.AppendMenu(MF_STRING, IDC_SEARCH_STRING, CSTRING(ENTER_SEARCH_STRING));
	dirMenu.AppendMenu(MF_STRING, IDC_MOVE, CSTRING(MOVE));
	dirMenu.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)NULL);
	dirMenu.AppendMenu(MF_STRING, IDC_REMOVE, CSTRING(REMOVE));

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
			string tmp;

			SourceIter j;
			for(j = getSources().begin(); j != getSources().end(); ++j) {
				if(tmp.size() > 0)
					tmp += ", ";

				if(j->getUser()->isOnline())
					online++;

				tmp += j->getUser()->getFullNick();
			}
			display->columns[COLUMN_USERS] = tmp.empty() ? STRING(NO_USERS) : tmp;
		}
		if(colMask & MASK_STATUS) {
			if(getStatus() == QueueItem::STATUS_WAITING) {

				char buf[64];
				if(online > 0) {
					if(getSources().size() == 1) {
						display->columns[COLUMN_STATUS] = STRING(WAITING_USER_ONLINE);
					} else {
						sprintf(buf, CSTRING(WAITING_USERS_ONLINE), online, getSources().size());
						display->columns[COLUMN_STATUS] = buf;
					}
				} else {
					if(getSources().size() == 0) {
						display->columns[COLUMN_STATUS] = STRING(NO_USERS_TO_DOWNLOAD_FROM);
					} else if(getSources().size() == 1) {
						display->columns[COLUMN_STATUS] = STRING(USER_OFFLINE);
					} else if(getSources().size() == 2) {
						display->columns[COLUMN_STATUS] = STRING(BOTH_USERS_OFFLINE);
					} else if(getSources().size() == 3) {
						display->columns[COLUMN_STATUS] = STRING(ALL_3_USERS_OFFLINE);
					} else if(getSources().size() == 4) {
						display->columns[COLUMN_STATUS] = STRING(ALL_4_USERS_OFFLINE);
					} else {
						sprintf(buf, CSTRING(ALL_USERS_OFFLINE), getSources().size());
						display->columns[COLUMN_STATUS] = buf;
					}
				}
			} else if(getStatus() == QueueItem::STATUS_RUNNING) {
				display->columns[COLUMN_STATUS] = STRING(RUNNING);
			} 
		}
		if(colMask & MASK_SIZE) {
			display->columns[COLUMN_SIZE] = (getSize() == -1) ? STRING(UNKNOWN) : Util::formatBytes(getSize());
			display->columns[COLUMN_EXACT_SIZE] = (getSize() == -1) ? STRING(UNKNOWN) : Util::formatExactSize(getSize());
		}
		if(colMask & MASK_DOWNLOADED) {
			if(getSize() > 0)
				display->columns[COLUMN_DOWNLOADED] = Util::formatBytes(getDownloadedBytes()) + " (" + Util::toString((double)getDownloadedBytes()*100.0/(double)getSize()) + "%)";
			else
				display->columns[COLUMN_DOWNLOADED].clear();
		}
		if(colMask & MASK_PRIORITY) {
			switch(getPriority()) {
		case QueueItem::PAUSED: display->columns[COLUMN_PRIORITY] = STRING(PAUSED); break;
		case QueueItem::LOWEST: display->columns[COLUMN_PRIORITY] = STRING(LOWEST); break;
		case QueueItem::LOW: display->columns[COLUMN_PRIORITY] = STRING(LOW); break;
		case QueueItem::NORMAL: display->columns[COLUMN_PRIORITY] = STRING(NORMAL); break;
		case QueueItem::HIGH: display->columns[COLUMN_PRIORITY] = STRING(HIGH); break;
		case QueueItem::HIGHEST: display->columns[COLUMN_PRIORITY] = STRING(HIGHEST); break;
		default: dcasserta(0); break;
			}
		}

		if(colMask & MASK_PATH) {
			display->columns[COLUMN_PATH] = Util::getFilePath(getTarget());
		}

		if(colMask & MASK_ERRORS) {
			string tmp;
			SourceIter j;
			for(j = getBadSources().begin(); j != getBadSources().end(); ++j) {
				if(!j->isSet(QueueItem::Source::FLAG_REMOVED)) {
					if(tmp.size() > 0)
						tmp += ", ";
					tmp += j->getUser()->getNick();
					tmp += " (";
					if(j->isSet(QueueItem::Source::FLAG_FILE_NOT_AVAILABLE)) {
						tmp += STRING(FILE_NOT_AVAILABLE);
					} else if(j->isSet(QueueItem::Source::FLAG_PASSIVE)) {
						tmp += STRING(PASSIVE_USER);
					} else if(j->isSet(QueueItem::Source::FLAG_ROLLBACK_INCONSISTENCY)) {
						tmp += STRING(ROLLBACK_INCONSISTENCY);
					} else if(j->isSet(QueueItem::Source::FLAG_CRC_FAILED)) {
						tmp += STRING(SFV_INCONSISTENCY);
					} else if(j->isSet(QueueItem::Source::FLAG_BAD_TREE)) {
						tmp += STRING(INVALID_TREE);
					}
					tmp += ')';
				}
			}
			display->columns[COLUMN_ERRORS] = tmp.empty() ? STRING(NO_ERRORS) : tmp;
		}

		if(colMask & MASK_SEARCHSTRING) {
			display->columns[COLUMN_SEARCHSTRING] = getSearchString();
		}

		if(colMask & MASK_ADDED) {
			display->columns[COLUMN_ADDED] = Util::formatTime("%Y-%m-%d %H:%M", getAdded());
		}
		if(colMask & MASK_TTH && getTTH() != NULL) {
			display->columns[COLUMN_TTH] = getTTH()->toBase32();
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
	
	const string& dir = ii->getPath();
	
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

HTREEITEM QueueFrame::addDirectory(const string& dir, bool isFileList /* = false */, HTREEITEM startAt /* = NULL */) {
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
		tvi.item.lParam = (LPARAM) new string(dir);
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
				string* stmp = (string*)ctrlDirs.GetItemData(next);
				if(Util::strnicmp(*stmp, dir, 3) == 0)
					break;
			}
			next = ctrlDirs.GetNextSiblingItem(next);
		}

		if(next == NULL) {
			// First addition, set commonStart to the dir minus the last part...
			i = dir.rfind('\\', dir.length()-2);
			if(i != string::npos) {
				string name = dir.substr(0, i);
				tvi.hParent = NULL;
				tvi.item.pszText = const_cast<char*>(name.c_str());
				tvi.item.lParam = (LPARAM)new string(dir.substr(0, i+1));
				next = ctrlDirs.InsertItem(&tvi);
			} else {
				dcassert(dir.length() == 3);
				tvi.hParent = NULL;
				tvi.item.pszText = const_cast<char*>(dir.c_str());
				tvi.item.lParam = (LPARAM)new string(dir);
				next = ctrlDirs.InsertItem(&tvi);
			}
		} 
		
		// Ok, next now points to x:\... find how much is common

		string* rootStr = (string*)ctrlDirs.GetItemData(next);
		
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
			string name = rootStr->substr(0, i-1);
			tvi.hParent = NULL;
			tvi.item.pszText = const_cast<char*>(name.c_str());
			tvi.item.lParam = (LPARAM)new string(rootStr->substr(0, i));
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

	while( i < dir.length() ) {
		while(next != NULL) {
			if(next != fileLists) {
				const string& n = getDir(next);
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
			string name = dir.substr(i, j-i);
			tvi.hParent = parent;
			tvi.item.pszText = const_cast<char*>(name.c_str());
			tvi.item.lParam = (LPARAM) new string(dir.substr(0, j+1));
			
			parent = ctrlDirs.InsertItem(&tvi);
			
			ctrlDirs.Expand(ctrlDirs.GetParentItem(parent), TVE_EXPAND);
			i = j + 1;
		}
	}
	return parent;
}

void QueueFrame::removeDirectory(const string& dir, bool isFileList /* = false */) {

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
					const string& n = getDir(next);
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

		if(!aQI->isSet(QueueItem::FLAG_USER_LIST)) {
			queueSize-=aQI->getSize();
			dcassert(queueSize >= 0);
		}
		queueItems--;
		dcassert(queueItems >= 0);
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

void QueueFrame::on(QueueManagerListener::SearchStringUpdated, QueueItem* aQI) {
	QueueItemInfo* ii = NULL;
	{
		Lock l(cs);
		QueueIter i = queue.find(aQI);
		dcassert(i != queue.end());
		ii = i->second;
		ii->setSearchString(aQI->getSearchString());
		ii->updateMask |= QueueItemInfo::MASK_SEARCHSTRING;
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
					curDir = Util::emptyString;
			}
			
			delete ii;
			updateStatus();
			if (BOOLSETTING(QUEUE_DIRTY)) {
				setDirty();
			}
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
		string name = ii->getTarget();
		string ext = Util::getFileExt(name);
		string ext2;
		if (!ext.empty())
		{
			ext = ext.substr(1); // remove leading dot so default extension works when browsing for file
			ext2 = "*." + ext;
		ext2 += (char)0;
		ext2 += "*." + ext;
		}
		ext2 += "*.*";
		ext2 += (char)0;
		ext2 += "*.*";
		ext2 += (char)0;

		if(WinUtil::browseFile(name, m_hWnd, true, ii->getPath(), ext2.c_str(), ext.empty() ? NULL : ext.c_str())) {
			QueueManager::getInstance()->move(ii->getTarget(), name);
		}
	} else if(n > 1) {
		string name;
		if(showTree) {
			name = curDir;
		}

		if(WinUtil::browseDirectory(name, m_hWnd)) {
			int i = -1;
			while( (i = ctrlQueue.GetNextItem(i, LVNI_SELECTED)) != -1) {
				QueueItemInfo* ii = ctrlQueue.getItemData(i);
				QueueManager::getInstance()->move(ii->getTarget(), name + ii->getTargetFileName());
			}			
		}
	}
}

void QueueFrame::moveSelectedDir() {
	if(ctrlDirs.GetSelectedItem() == NULL)
		return;

	dcassert(!curDir.empty());
	string name = curDir;
	
	if(WinUtil::browseDirectory(name, m_hWnd)) {
		moveDir(ctrlDirs.GetSelectedItem(), name);
	}
}

void QueueFrame::moveDir(HTREEITEM ht, const string& target) {
	HTREEITEM next = ctrlDirs.GetChildItem(ht);
	while(next != NULL) {
		moveDir(next, target + Util::getLastDir(getDir(next)));
		next = ctrlDirs.GetNextSiblingItem(next);
	}
	string* s = (string*)ctrlDirs.GetItemData(ht);

	DirectoryPair p = directories.equal_range(*s);
	
	for(DirectoryIter i = p.first; i != p.second; ++i) {
		QueueItemInfo* qi = i->second;
		QueueManager::getInstance()->move(qi->getTarget(), target + qi->getTargetFileName());
	}			
}

void QueueFrame::setSearchStringForSelected() {
	LineDlg dlg;
	dlg.title = STRING(SEARCH_STRING);
	dlg.description = STRING(ENTER_SEARCH_STRING);

	int n = ctrlQueue.GetSelectedCount();
	if(n == 1) {
		// Single item, fill in the current search string
		QueueItemInfo* ii = ctrlQueue.getItemData(ctrlQueue.GetNextItem(-1, LVNI_SELECTED));
		dlg.line = ii->getSearchString();
		if(dlg.line.empty() && BOOLSETTING(AUTO_SEARCH_AUTO_STRING))
			dlg.line = SearchManager::getInstance()->clean(ii->getTargetFileName());
		if(dlg.DoModal() == IDOK)
			QueueManager::getInstance()->setSearchString(ii->getTarget(), dlg.line);
	} else if(n > 1) {
		if(SETTING(CONNECTION_TYPE) == SettingsManager::CONNECTION_ACTIVE) {
			if(n > 10) {
				if(MessageBox(CSTRING(SEARCH_STRING_INEFFICIENT), CSTRING(SEARCH_STRING), MB_YESNO|MB_ICONWARNING) != IDYES)
					return;
			}
		} else {
			if(n > 5) {
				if(MessageBox(CSTRING(SEARCH_STRING_INEFFICIENT), CSTRING(SEARCH_STRING), MB_YESNO|MB_ICONWARNING) != IDYES)
					return;
			}
		}

		// Multiple items. TODO: Could check if all search strings are the same and fill in the search string
		if(dlg.DoModal() == IDOK) {
			int i = -1;
			while( (i = ctrlQueue.GetNextItem(i, LVNI_SELECTED)) != -1) {
				QueueItemInfo* ii = ctrlQueue.getItemData(i);
				QueueManager::getInstance()->setSearchString(ii->getTarget(), dlg.line);
			}						
		}
	}
}

void QueueFrame::setSearchStringForSelectedDir() {
	HTREEITEM ht = ctrlDirs.GetSelectedItem();
	if(ht == NULL)
		return;

	unsigned int maxItemCount;
	if(SETTING(CONNECTION_TYPE) == SettingsManager::CONNECTION_ACTIVE)
		maxItemCount = 10;
	else
		maxItemCount = 5;

	if (isItemCountAtLeast(ht, maxItemCount + 1)) {
		if(MessageBox(CSTRING(SEARCH_STRING_INEFFICIENT), CSTRING(SEARCH_STRING), MB_YESNO|MB_ICONWARNING) != IDYES)
			return;
	}

	LineDlg dlg;
	dlg.title = STRING(SEARCH_STRING);
	dlg.description = STRING(ENTER_SEARCH_STRING);
	if(dlg.DoModal() == IDOK)
		setSearchStringForDir(ht, dlg.line);
}

void QueueFrame::setSearchStringForDir(HTREEITEM ht, const string& searchString) {
	string* s = (string*)ctrlDirs.GetItemData(ht);
	DirectoryPair p = directories.equal_range(*s);

	for(DirectoryIter i = p.first; i != p.second; ++i) {
		QueueItemInfo* ii = i->second;
		QueueManager::getInstance()->setSearchString(ii->getTarget(), searchString);
	}			

	HTREEITEM next = ctrlDirs.GetChildItem(ht);
	while(next != NULL) {
		setSearchStringForDir(next, searchString);
		next = ctrlDirs.GetNextSiblingItem(next);
	}
}

bool QueueFrame::isItemCountAtLeast(HTREEITEM ht, unsigned int minItemCount) {
	if (ht == NULL)
		return false;

	return isItemCountAtLeastRecursive(ht, minItemCount);
}

bool QueueFrame::isItemCountAtLeastRecursive(HTREEITEM ht, unsigned int& minItemCount) {
	string* s = (string*)ctrlDirs.GetItemData(ht);
	DirectoryPair p = directories.equal_range(*s);

	for(DirectoryIter i = p.first; i != p.second && minItemCount; ++i, --minItemCount)
		;

	if (!minItemCount)
		return true;

	HTREEITEM next = ctrlDirs.GetChildItem(ht);
	while(next != NULL) {
		if (isItemCountAtLeastRecursive(next, minItemCount))
			return true;

		next = ctrlDirs.GetNextSiblingItem(next);
	}

	return false;
}

LRESULT QueueFrame::onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	RECT rc;                    // client area of window 
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };        // location of mouse click 
	
	// Get the bounding rectangle of the client area. 
	ctrlQueue.GetClientRect(&rc);
	ctrlQueue.ScreenToClient(&pt); 
	if (PtInRect(&rc, pt) && ctrlQueue.GetSelectedCount() > 0) { 
		usingDirMenu = false;
		CMenuItemInfo mi;
		
		while(browseMenu.GetMenuItemCount() > 0) {
			browseMenu.RemoveMenu(0, MF_BYPOSITION);
		}
		while(removeMenu.GetMenuItemCount() > 0) {
			removeMenu.RemoveMenu(0, MF_BYPOSITION);
		}
		while(removeAllMenu.GetMenuItemCount() > 0) {
			removeAllMenu.RemoveMenu(0, MF_BYPOSITION);
		}
		while(pmMenu.GetMenuItemCount() > 0) {
			pmMenu.RemoveMenu(0, MF_BYPOSITION);
		}
		while(readdMenu.GetMenuItemCount() > 0) {
			readdMenu.RemoveMenu(0, MF_BYPOSITION);
		}

		ctrlQueue.ClientToScreen(&pt);
		
		if(ctrlQueue.GetSelectedCount() == 1) {
			QueueItemInfo* ii = ctrlQueue.getItemData(ctrlQueue.GetNextItem(-1, LVNI_SELECTED));
			menuItems = 0;
			QueueItemInfo::SourceIter i;
			for(i = ii->getSources().begin(); i != ii->getSources().end(); ++i) {

				mi.fMask = MIIM_ID | MIIM_TYPE | MIIM_DATA;
				mi.fType = MFT_STRING;
				mi.dwTypeData = (LPSTR)i->getUser()->getNick().c_str();
				mi.dwItemData = (DWORD)&(*i);
				mi.wID = IDC_BROWSELIST + menuItems;
				browseMenu.InsertMenuItem(menuItems, TRUE, &mi);
				mi.wID = IDC_REMOVE_SOURCE + menuItems;
				removeMenu.InsertMenuItem(menuItems, TRUE, &mi);
				mi.wID = IDC_REMOVE_SOURCES + menuItems;
				removeAllMenu.InsertMenuItem(menuItems, TRUE, &mi);
				if(i->getUser()->isOnline()) {
					mi.wID = IDC_PM + menuItems;
					pmMenu.InsertMenuItem(menuItems, TRUE, &mi);
				}
				menuItems++;
			}
			readdItems = 0;
			for(i = ii->getBadSources().begin(); i != ii->getBadSources().end(); ++i) {
				mi.fMask = MIIM_ID | MIIM_TYPE | MIIM_DATA;
				mi.fType = MFT_STRING;
				mi.dwTypeData = (LPSTR)i->getUser()->getNick().c_str();
				mi.dwItemData = (DWORD)&(*i);
				mi.wID = IDC_READD + readdItems;
				readdMenu.InsertMenuItem(readdItems, TRUE, &mi);
				readdItems++;
			}

			if(ii->getTTH() == NULL) {
				singleMenu.EnableMenuItem(IDC_SEARCH_BY_TTH, MF_GRAYED);
				singleMenu.EnableMenuItem(IDC_BITZI_LOOKUP, MF_GRAYED);
				singleMenu.EnableMenuItem(IDC_COPY_MAGNET, MF_GRAYED);
			} else {
				singleMenu.EnableMenuItem(IDC_SEARCH_BY_TTH, MF_ENABLED);
				singleMenu.EnableMenuItem(IDC_BITZI_LOOKUP, MF_ENABLED);
				singleMenu.EnableMenuItem(IDC_COPY_MAGNET, MF_ENABLED);
			}
			
			singleMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
		} else {
			multiMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
		}
		
		return TRUE; 
	}

	ctrlQueue.ClientToScreen(&pt);

	ctrlDirs.GetClientRect(&rc);
	ctrlDirs.ScreenToClient(&pt);

	if (PtInRect(&rc, pt) && ctrlDirs.GetSelectedItem() != NULL) { 
		usingDirMenu = true;
		// Strange, windows doesn't change the selection on right-click... (!)
		UINT a = 0;
		HTREEITEM ht = ctrlDirs.HitTest(pt, &a);
		if(ht != NULL && ht != ctrlDirs.GetSelectedItem())
			ctrlDirs.SelectItem(ht);
		
		ctrlDirs.ClientToScreen(&pt);
		dirMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
	
		return TRUE;
	}

	return FALSE; 
}

LRESULT QueueFrame::onSearchAlternates(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(ctrlQueue.GetSelectedCount() == 1) {
		string tmp;

		int i = ctrlQueue.GetNextItem(-1, LVNI_SELECTED);
		QueueItemInfo* ii = ctrlQueue.getItemData(i);
		
		string searchString = SearchManager::clean(ii->getSearchString());
		if (searchString.size() < 1)
			searchString = SearchManager::clean(ii->getTargetFileName());
		StringList tok = StringTokenizer(searchString, ' ').getTokens();
		
		for(StringIter si = tok.begin(); si != tok.end(); ++si) {
			bool found = false;
			
			for(StringIter j = searchFilter.begin(); j != searchFilter.end(); ++j) {
				if(Util::stricmp(si->c_str(), j->c_str()) == 0) {
					found = true;
				}
			}
			
			if(!found && !si->empty()) {
				tmp += *si + ' ';
			}
		}
		if(!tmp.empty()) {
			bool bigFile = (ii->getSize() > 10*1024*1024);
			if(bigFile) {
				SearchFrame::openWindow(tmp, ii->getSize()-1, SearchManager::SIZE_ATLEAST, ShareManager::getInstance()->getType(ii->getTargetFileName()));
			} else {
				SearchFrame::openWindow(tmp, ii->getSize()+1, SearchManager::SIZE_ATMOST, ShareManager::getInstance()->getType(ii->getTargetFileName()));
			}
		}
	} 
	
	return 0;
}

LRESULT QueueFrame::onSearchByTTH(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(ctrlQueue.GetSelectedCount() == 1) {
		int i = ctrlQueue.GetNextItem(-1, LVNI_SELECTED);
		QueueItemInfo* ii = ctrlQueue.getItemData(i);

		if(ii->getTTH() != NULL) {
			WinUtil::searchHash(ii->getTTH());
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
		QueueItemInfo::SourceInfo* s = (QueueItemInfo::SourceInfo*)mi.dwItemData;
		try {
			QueueManager::getInstance()->readd(ii->getTarget(), s->getUser());
		} catch(const Exception& e) {
			ctrlStatus.SetText(0, e.getError().c_str());
		}
	}
	return 0;
}

LRESULT QueueFrame::onRemoveSource(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	
	if(ctrlQueue.GetSelectedCount() == 1) {
		int i = ctrlQueue.GetNextItem(-1, LVNI_SELECTED);
		QueueItemInfo* ii = ctrlQueue.getItemData(i);
		CMenuItemInfo mi;
		mi.fMask = MIIM_DATA;
		
		removeMenu.GetMenuItemInfo(wID, FALSE, &mi);
		QueueItemInfo::SourceInfo* s = (QueueItemInfo::SourceInfo*)mi.dwItemData;
		QueueManager::getInstance()->removeSource(ii->getTarget(), s->getUser(), QueueItem::Source::FLAG_REMOVED);
	}
	return 0;
}

LRESULT QueueFrame::onRemoveSources(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	CMenuItemInfo mi;
	mi.fMask = MIIM_DATA;
	removeAllMenu.GetMenuItemInfo(wID, FALSE, &mi);
	QueueItemInfo::SourceInfo* s = (QueueItemInfo::SourceInfo*)mi.dwItemData;
	QueueManager::getInstance()->removeSources(s->getUser(), QueueItem::Source::FLAG_REMOVED);
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
			QueueManager::getInstance()->setPriority(ctrlQueue.getItemData(i)->getTarget(), p);
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
	const string& name = getDir(ht);
	DirectoryPair dp = directories.equal_range(name);
	for(DirectoryIter i = dp.first; i != dp.second; ++i) {
		QueueManager::getInstance()->remove(i->second->getTarget());
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
			return;
		}

		switch(p){
			case QueueItem::HIGHEST: p = QueueItem::HIGH; break;
			case QueueItem::HIGH:    p = inc ? QueueItem::HIGHEST : QueueItem::NORMAL; break;
			case QueueItem::NORMAL:  p = inc ? QueueItem::HIGH    : QueueItem::LOW; break;
			case QueueItem::LOW:     p = inc ? QueueItem::NORMAL  : QueueItem::LOWEST; break;
			case QueueItem::LOWEST:  p = inc ? QueueItem::LOW     : QueueItem::PAUSED; break;
			case QueueItem::PAUSED:  p = QueueItem::LOWEST; break;
		}

		QueueManager::getInstance()->setPriority(ctrlQueue.getItemData(i)->getTarget(), p);
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
	string name = getDir(ht);
	DirectoryPair dp = directories.equal_range(name);
	for(DirectoryIter i = dp.first; i != dp.second; ++i) {
		QueueManager::getInstance()->setPriority(i->second->getTarget(), p);
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

		string tmp1 = STRING(ITEMS) + ": " + Util::toString(cnt);
		string tmp2 = STRING(SIZE) + ": " + Util::formatBytes(total);
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
			tmp1 = STRING(FILES) + ": " + Util::toString(queueItems);
			tmp2 = STRING(SIZE) + ": " + Util::formatBytes(queueSize);

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
static char tmpBuf[1024];
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
 * $Id: QueueFrame.cpp,v 1.58 2004/08/02 15:29:19 arnetheduck Exp $
 */



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

#include "../client/SimpleXML.h"
#include "../client/StringTokenizer.h"

QueueFrame* QueueFrame::frame = NULL;

#define FILE_LIST_NAME "File Lists"

int QueueFrame::columnIndexes[] = { COLUMN_TARGET, COLUMN_STATUS, COLUMN_SIZE, COLUMN_PRIORITY,
COLUMN_USERS, COLUMN_PATH, COLUMN_ERRORS };

int QueueFrame::columnSizes[] = { 200, 300, 75, 75, 200, 200, 200 };

static ResourceManager::Strings columnNames[] = { ResourceManager::FILENAME, ResourceManager::STATUS, ResourceManager::SIZE, 
ResourceManager::PRIORITY, ResourceManager::USERS, ResourceManager::PATH, ResourceManager::ERRORS };

LRESULT QueueFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	dcassert(frame == NULL);
	frame = this;

	showTree = BOOLSETTING(QUEUEFRAME_SHOW_TREE);
	
	CreateSimpleStatusBar(ATL_IDS_IDLEMESSAGE, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SBARS_SIZEGRIP);
	ctrlStatus.Attach(m_hWndStatusBar);
	
	ctrlQueue.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS, WS_EX_CLIENTEDGE, IDC_QUEUE);

	if(BOOLSETTING(FULL_ROW_SELECT)) {
		ctrlQueue.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP);
	}
	else {
		ctrlQueue.SetExtendedListViewStyle(LVS_EX_HEADERDRAGDROP);
	}

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
		int fmt = (j == COLUMN_SIZE) ? LVCFMT_RIGHT : LVCFMT_LEFT;
		ctrlQueue.InsertColumn(j, CSTRING_I(columnNames[j]), fmt, columnSizes[j], j);
	}
	
	ctrlQueue.SetColumnOrderArray(COLUMN_LAST, columnIndexes);
	
	ctrlQueue.SetBkColor(WinUtil::bgColor);
	ctrlQueue.SetTextBkColor(WinUtil::bgColor);
	ctrlQueue.SetTextColor(WinUtil::textColor);

	ctrlDirs.SetBkColor(WinUtil::bgColor);
	ctrlDirs.SetTextColor(WinUtil::textColor);

	ctrlShowTree.Create(ctrlStatus.m_hWnd, rcDefault, "+/-", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	ctrlShowTree.SetButtonStyle(BS_AUTOCHECKBOX, false);
	ctrlShowTree.SetFont(ctrlStatus.GetFont());
	ctrlShowTree.SetCheck(showTree);
	showTreeContainer.SubclassWindow(ctrlShowTree.m_hWnd);
	
	singleMenu.CreatePopupMenu();
	multiMenu.CreatePopupMenu();
	browseMenu.CreatePopupMenu();
	removeMenu.CreatePopupMenu();
	pmMenu.CreatePopupMenu();
	priorityMenu.CreatePopupMenu();
	dirMenu.CreatePopupMenu();
	readdMenu.CreatePopupMenu();

	singleMenu.AppendMenu(MF_STRING, IDC_SEARCH_ALTERNATES, CSTRING(SEARCH_FOR_ALTERNATES));
	singleMenu.AppendMenu(MF_STRING, IDC_MOVE, CSTRING(MOVE));
	singleMenu.AppendMenu(MF_POPUP, (UINT)(HMENU)priorityMenu, CSTRING(SET_PRIORITY));
	singleMenu.AppendMenu(MF_POPUP, (UINT)(HMENU)browseMenu, CSTRING(GET_FILE_LIST));
	singleMenu.AppendMenu(MF_POPUP, (UINT)(HMENU)pmMenu, CSTRING(SEND_PRIVATE_MESSAGE));
	singleMenu.AppendMenu(MF_POPUP, (UINT)(HMENU)readdMenu, CSTRING(READD_SOURCE));
	singleMenu.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)NULL);
	singleMenu.AppendMenu(MF_POPUP, (UINT)(HMENU)removeMenu, CSTRING(REMOVE_SOURCE));
	singleMenu.AppendMenu(MF_STRING, IDC_REMOVE, CSTRING(REMOVE));

	multiMenu.AppendMenu(MF_POPUP, (UINT)(HMENU)priorityMenu, CSTRING(SET_PRIORITY));
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
	dirMenu.AppendMenu(MF_STRING, IDC_MOVE, CSTRING(MOVE));
	dirMenu.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)NULL);
	dirMenu.AppendMenu(MF_STRING, IDC_REMOVE, CSTRING(REMOVE));

	SetWindowText(CSTRING(DOWNLOAD_QUEUE));

	addQueueList(QueueManager::getInstance()->lockQueue());
	QueueManager::getInstance()->unlockQueue();
	QueueManager::getInstance()->addListener(this);

	int w[] = { 1, 2, 3, 4};
	ctrlStatus.SetParts(4, w);
	updateStatus();

	bHandled = FALSE;
	return 1;
}

QueueFrame::StringListInfo::StringListInfo(QueueItem* qi) {

	columns[COLUMN_TARGET] = qi->getTargetFileName();
	columns[COLUMN_SIZE] = (qi->getSize() == -1) ? STRING(UNKNOWN) : Util::formatBytes(qi->getSize());
	string tmp;
	
	int online = 0;
	QueueItem::Source::Iter j;
	for(j = qi->getSources().begin(); j != qi->getSources().end(); ++j) {
		if(tmp.size() > 0)
			tmp += ", ";
		
		QueueItem::Source::Ptr sr = *j;
		if(sr->getUser()->isOnline())
			online++;
		
		tmp += sr->getUser()->getNick() + " (" + sr->getUser()->getClientName() + ")";
	}
	columns[COLUMN_USERS] = tmp.empty() ? STRING(NO_USERS) : tmp;
	tmp = Util::emptyString;
	for(j = qi->getBadSources().begin(); j != qi->getBadSources().end(); ++j) {
		QueueItem::Source::Ptr sr = *j;
		if(!sr->isSet(QueueItem::Source::FLAG_REMOVED)) {
			if(tmp.size() > 0)
				tmp += ", ";
			tmp += sr->getUser()->getNick();
			tmp += " (";
			if(sr->isSet(QueueItem::Source::FLAG_FILE_NOT_AVAILABLE)) {
				tmp += STRING(FILE_NOT_AVAILABLE);
			} else if(sr->isSet(QueueItem::Source::FLAG_PASSIVE)) {
				tmp += STRING(PASSIVE_USER);
			} else if(sr->isSet(QueueItem::Source::FLAG_ROLLBACK_INCONSISTENCY)) {
				tmp += STRING(ROLLBACK_INCONSISTENCY);
			} else if(sr->isSet(QueueItem::Source::FLAG_CRC_FAILED)) {
				tmp += STRING(SFV_INCONSISTENCY);
			}
			tmp += ')';
		}
	}
	columns[COLUMN_ERRORS] = tmp.empty() ? STRING(NO_ERRORS) : tmp;
	
	if(qi->getStatus() == QueueItem::STATUS_WAITING) {
		
		char buf[64];
		if(online > 0) {
			
			if(qi->getSources().size() == 1) {
				columns[COLUMN_STATUS] = STRING(WAITING_USER_ONLINE);
			} else {
				sprintf(buf, CSTRING(WAITING_USERS_ONLINE), online, qi->getSources().size());
				columns[COLUMN_STATUS] = buf;
			}
		} else {
			if(qi->getSources().size() == 0) {
				columns[COLUMN_STATUS] = STRING(NO_USERS_TO_DOWNLOAD_FROM);
			} else if(qi->getSources().size() == 1) {
				columns[COLUMN_STATUS] = STRING(USER_OFFLINE);
			} else if(qi->getSources().size() == 2) {
				columns[COLUMN_STATUS] = STRING(BOTH_USERS_OFFLINE);
			} else {
				sprintf(buf, CSTRING(ALL_USERS_OFFLINE), qi->getSources().size());
				columns[COLUMN_STATUS] = buf;
			}
		}
	} else if(qi->getStatus() == QueueItem::STATUS_RUNNING) {
		columns[COLUMN_STATUS] = STRING(RUNNING);
	} 
	
	switch(qi->getPriority()) {
	case QueueItem::PAUSED: columns[COLUMN_PRIORITY] = STRING(PAUSED); break;
	case QueueItem::LOWEST: columns[COLUMN_PRIORITY] = STRING(LOWEST); break;
	case QueueItem::LOW: columns[COLUMN_PRIORITY] = STRING(LOW); break;
	case QueueItem::NORMAL: columns[COLUMN_PRIORITY] = STRING(NORMAL); break;
	case QueueItem::HIGH: columns[COLUMN_PRIORITY] = STRING(HIGH); break;
	case QueueItem::HIGHEST: columns[COLUMN_PRIORITY] = STRING(HIGHEST); break;
	default: dcassert(0); break;
	}
	columns[COLUMN_PATH] = qi->getTarget();
}

void QueueFrame::onQueueAdded(QueueItem* aQI) {
	QueueItem* qi = new QueueItem(*aQI);
	{
		Lock l(cs);
		dcassert(queue.find(aQI) == queue.end());
		queue[aQI] = qi;
	}

	speak(ADD_ITEM,	qi);
}

void QueueFrame::addQueueItem(QueueItem* qi) {
	if(!qi->isSet(QueueItem::FLAG_USER_LIST)) {
		queueSize+=qi->getSize();
	}
	queueItems++;
	dirty = true;
	
	string dir = Util::getFilePath(qi->getTarget());
	
	bool updateDir = (directories.find(dir) == directories.end());
	directories.insert(make_pair(dir, qi));
	
	if(updateDir) {
		addDirectory(dir, qi->isSet(QueueItem::FLAG_USER_LIST));
	} 
	if(!showTree || (curDir == dir)) {
		StringListInfo sli(qi);
		StringList l;
		
		for(int j = 0; j < COLUMN_LAST; j++) {
			l.push_back(sli.columns[j]);
		}
		dcassert(ctrlQueue.find((LPARAM)qi) == -1);
		ctrlQueue.insert(l, WinUtil::getIconIndex(qi->getTarget()), (LPARAM)qi);
	}
}

void QueueFrame::addQueueList(const QueueItem::StringMap& li) {
	ctrlQueue.SetRedraw(FALSE);
	ctrlDirs.SetRedraw(FALSE);
	for(QueueItem::StringMap::const_iterator j = li.begin(); j != li.end(); ++j) {
		QueueItem* aQI = j->second;
		QueueItem* qi = new QueueItem(*aQI);
		dcassert(queue.find(aQI) == queue.end());
		queue[aQI] = qi;
		addQueueItem(qi);
	}
	ctrlQueue.SetRedraw(TRUE);
	ctrlQueue.resort();
	ctrlDirs.SetRedraw(TRUE);
	ctrlDirs.Invalidate();
}

void QueueFrame::addDirectory(const string& dir, bool isFileList /* = false */, string* s /* = NULL*/ ) {
	TV_INSERTSTRUCT tvi;
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
		return;
	} 

	// More complicated, we have to find the last available tree item and then see...
	string::size_type i = 0;
	string::size_type j;

	HTREEITEM next = ctrlDirs.GetRootItem();
	HTREEITEM parent = NULL;
	
	while( i < dir.length() ) {
		while(next != NULL) {
			if(next == fileLists) {
				next = ctrlDirs.GetNextSiblingItem(next);
				continue;
			}
			const string& n = getDir(next);
			if(Util::strnicmp(n.c_str()+i, dir.c_str()+i, n.length()-i) == 0) {
				// Found a part, we assume it's the best one we can find...
				i = n.length();
				
				parent = next;
				next = ctrlDirs.GetChildItem(next);
				break;
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
			tvi.item.lParam = (LPARAM) ((s == NULL) ? new string(dir.substr(0, j+1)) : s);
			
			parent = ctrlDirs.InsertItem(&tvi);
			
			if(directories.find(getDir(parent)) == directories.end()) {
				ctrlDirs.Expand(ctrlDirs.GetParentItem(parent), TVE_EXPAND);
			}
			i = j + 1;
		}
	}
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
				if(next == fileLists) {
					next = ctrlDirs.GetNextSiblingItem(next);
					continue;
				}
				const string& n = getDir(next);
				if(Util::strnicmp(n.c_str()+i, dir.c_str()+i, n.length()-i) == 0) {
					// Match!
					parent = next;
					next = ctrlDirs.GetChildItem(next);
					i = n.length();
					break;
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

void QueueFrame::onQueueRemoved(QueueItem* aQI) {
	QueueItem* qi = NULL;
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

void QueueFrame::onQueueMoved(QueueItem* aQI) {
	QueueItem* qi = NULL;
	QueueItem* qi2 = new QueueItem(*aQI);
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

void QueueFrame::onQueueUpdated(QueueItem* aQI) {
	QueueItem* qi = NULL;
	{
		Lock l(cs);
		dcassert(queue.find(aQI) != queue.end());
		qi = queue[aQI];
		{
			for(QueueItem::Source::Iter i = qi->getSources().begin(); i != qi->getSources().end(); ++i) {
				delete *i;
			}
			qi->getSources().clear();
			for(QueueItem::Source::Iter j = aQI->getSources().begin(); j != aQI->getSources().end(); ++j) {
				qi->getSources().push_back(new QueueItem::Source(*(*j)));
			}
		}
		{
			for(QueueItem::Source::Iter i = qi->getBadSources().begin(); i != qi->getBadSources().end(); ++i) {
				delete *i;
			}
			qi->getBadSources().clear();
			for(QueueItem::Source::Iter j = aQI->getBadSources().begin(); j != aQI->getBadSources().end(); ++j) {
				qi->getBadSources().push_back(new QueueItem::Source(*(*j)));
			}
		}

		qi->setPriority(aQI->getPriority());
		qi->setStatus(aQI->getStatus());
	}
	
	speak(SET_TEXT, qi);
}

LRESULT QueueFrame::onSpeaker(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	Lock l(cs);
	spoken = false;

	for(TaskIter ti = tasks.begin(); ti != tasks.end(); ++ti) {
		if(ti->first == ADD_ITEM) {
			QueueItem* qi = (QueueItem*)ti->second;
			addQueueItem(qi);

			updateStatus();
		} else if(ti->first == REMOVE_ITEM) {
			QueueItem* qi = (QueueItem*)ti->second;
			string dir = Util::getFilePath(qi->getTarget());
			
			if(!showTree || (dir == curDir)) {
				dcassert(ctrlQueue.find((LPARAM)qi) != -1);
				ctrlQueue.DeleteItem(ctrlQueue.find((LPARAM)qi));
				updateStatus();
			}
			
			pair<DirectoryIter, DirectoryIter> i = directories.equal_range(dir);
			DirectoryIter j;
			for(j = i.first; j != i.second; ++j) {
				if(j->second == qi)
					break;
			}
			dcassert(j != i.second);
			directories.erase(j);
			if(directories.count(dir) == 0) {
				removeDirectory(dir, qi->isSet(QueueItem::FLAG_USER_LIST));
				if(curDir == dir)
					curDir = Util::emptyString;
			}
			
			delete qi;
			updateStatus();
			setDirty();
			
		} else if(ti->first == SET_TEXT) {
			QueueItem* qi = (QueueItem*)ti->second;
			
			string dir = Util::getFilePath(qi->getTarget());
			if(!showTree || (dir == curDir) ) {
				StringListInfo sli(qi);
				int n = ctrlQueue.find((LPARAM)qi);
				dcassert(n != -1);
				ctrlQueue.SetRedraw(FALSE);
				for(int i = 0; i < COLUMN_LAST; i++) {
					if(!sli.columns[i].empty()) {
						ctrlQueue.SetItemText(n, i, sli.columns[i].c_str());
					}
				}
				ctrlQueue.SetRedraw(TRUE);
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
		QueueItem* qi = (QueueItem*)ctrlQueue.GetItemData(ctrlQueue.GetNextItem(-1, LVNI_SELECTED));
		string name = qi->getTarget();
		if(WinUtil::browseFile(name, m_hWnd, true, Util::getFilePath(name))) {
			QueueManager::getInstance()->move(qi->getTarget(), name);
		}
	} else if(n > 1) {
		string name;
		if(showTree) {
			name = curDir;
		}

		if(WinUtil::browseDirectory(name, m_hWnd)) {
			int i = -1;
			while( (i = ctrlQueue.GetNextItem(i, LVNI_SELECTED)) != -1) {
				QueueItem* qi = (QueueItem*)ctrlQueue.GetItemData(i);
				QueueManager::getInstance()->move(qi->getTarget(), name + qi->getTargetFileName());
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
		moveDir(next, target);
		next = ctrlDirs.GetNextSiblingItem(next);
	}
	string* s = (string*)ctrlDirs.GetItemData(ht);
	string name = target + s->substr(curDir.length());
	
	DirectoryPair p = directories.equal_range(*s);
	
	for(DirectoryIter i = p.first; i != p.second; ++i) {
		QueueItem* qi = i->second;
		QueueManager::getInstance()->move(qi->getTarget(), name + qi->getTargetFileName());
	}			
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
		while(pmMenu.GetMenuItemCount() > 0) {
			pmMenu.RemoveMenu(0, MF_BYPOSITION);
		}
		while(readdMenu.GetMenuItemCount() > 0) {
			readdMenu.RemoveMenu(0, MF_BYPOSITION);
		}

		ctrlQueue.ClientToScreen(&pt);
		
		if(ctrlQueue.GetSelectedCount() == 1) {
			LVITEM lvi;
			lvi.iItem = ctrlQueue.GetNextItem(-1, LVNI_SELECTED);
			lvi.iSubItem = 0;
			lvi.mask = LVIF_IMAGE | LVIF_PARAM;
			
			ctrlQueue.GetItem(&lvi);
			menuItems = 0;
			
			QueueItem::Source::Iter i;
			QueueItem* q = (QueueItem*)lvi.lParam;
			for(i = q->getSources().begin(); i != q->getSources().end(); ++i) {

				mi.fMask = MIIM_ID | MIIM_TYPE | MIIM_DATA;
				mi.fType = MFT_STRING;
				mi.dwTypeData = (LPSTR)(*i)->getUser()->getNick().c_str();
				mi.dwItemData = (DWORD)*i;
				mi.wID = IDC_BROWSELIST + menuItems;
				browseMenu.InsertMenuItem(menuItems, TRUE, &mi);
				mi.wID = IDC_REMOVE_SOURCE + menuItems;
				removeMenu.InsertMenuItem(menuItems, TRUE, &mi);
				if((*i)->getUser()->isOnline()) {
					mi.wID = IDC_PM + menuItems;
					pmMenu.InsertMenuItem(menuItems, TRUE, &mi);
				}
				menuItems++;
			}
			readdItems = 0;
			for(i = q->getBadSources().begin(); i != q->getBadSources().end(); ++i) {
				mi.fMask = MIIM_ID | MIIM_TYPE | MIIM_DATA;
				mi.fType = MFT_STRING;
				mi.dwTypeData = (LPSTR)(*i)->getUser()->getNick().c_str();
				mi.dwItemData = (DWORD)*i;
				mi.wID = IDC_READD + readdItems;
				readdMenu.InsertMenuItem(readdItems, TRUE, &mi);
				readdItems++;
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
		QueueItem* qi = (QueueItem*)ctrlQueue.GetItemData(i);
		
		StringList tok = StringTokenizer(SearchManager::clean(qi->getTargetFileName()), ' ').getTokens();
		
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
		int64_t size = qi->getSize();
		if(!tmp.empty()) {
			SearchFrame* pChild = new SearchFrame();
			pChild->setTab(getTab());
			bool bigFile = (size > 10*1024*1024);
			pChild->setInitial(tmp, (bigFile ? size-1: size + 1), (bigFile ? SearchManager::SIZE_ATLEAST : SearchManager::SIZE_ATMOST) );
			pChild->CreateEx(m_hWndMDIClient);
			
		}
	} 
	
	return 0;
}

LRESULT QueueFrame::onBrowseList(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	
	if(ctrlQueue.GetSelectedCount() == 1) {
		CMenuItemInfo mi;
		mi.fMask = MIIM_DATA;
		
		browseMenu.GetMenuItemInfo(wID, FALSE, &mi);
		QueueItem::Source* s = (QueueItem::Source*)mi.dwItemData;
		try {
			QueueManager::getInstance()->addList(s->getUser());
		} catch(Exception) {
		}
	}
	return 0;
}

LRESULT QueueFrame::onReadd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	
	if(ctrlQueue.GetSelectedCount() == 1) {
		int i = ctrlQueue.GetNextItem(-1, LVNI_SELECTED);
		QueueItem* q = (QueueItem*)ctrlQueue.GetItemData(i);

		CMenuItemInfo mi;
		mi.fMask = MIIM_DATA;
		
		readdMenu.GetMenuItemInfo(wID, FALSE, &mi);
		QueueItem::Source* s = (QueueItem::Source*)mi.dwItemData;
		try {
			QueueManager::getInstance()->add(s->getPath(), q->getSize(), s->getUser(), q->getTarget());
		} catch(Exception e) {
			ctrlStatus.SetText(0, e.getError().c_str());
		}
	}
	return 0;
}

LRESULT QueueFrame::onRemoveSource(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	
	if(ctrlQueue.GetSelectedCount() == 1) {
		int i = ctrlQueue.GetNextItem(-1, LVNI_SELECTED);
		QueueItem* q = (QueueItem*)ctrlQueue.GetItemData(i);
		CMenuItemInfo mi;
		mi.fMask = MIIM_DATA;
		
		removeMenu.GetMenuItemInfo(wID, FALSE, &mi);
		QueueItem::Source* s = (QueueItem::Source*)mi.dwItemData;
		QueueManager::getInstance()->removeSource(q->getTarget(), s->getUser(), QueueItem::Source::FLAG_REMOVED);
	}
	return 0;
}

LRESULT QueueFrame::onPM(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	
	if(ctrlQueue.GetSelectedCount() == 1) {
		CMenuItemInfo mi;
		mi.fMask = MIIM_DATA;
		
		pmMenu.GetMenuItemInfo(wID, FALSE, &mi);
		QueueItem::Source* s = (QueueItem::Source*)mi.dwItemData;
		PrivateFrame::openWindow(s->getUser(), m_hWndMDIClient, getTab());
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
			QueueManager::getInstance()->setPriority(((QueueItem*)ctrlQueue.GetItemData(i))->getTarget(), p);
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

void QueueFrame::UpdateLayout(BOOL bResizeBars /* = TRUE */) {
	RECT rect;
	GetClientRect(&rect);
	// position bars and offset their dimensions
	UpdateBarsPosition(rect, bResizeBars);
	
	if(ctrlStatus.IsWindow()) {
		CRect sr;
		int w[4];
		ctrlStatus.GetClientRect(sr);
		int tmp = (sr.Width()) > 316 ? 216 : ((sr.Width() > 116) ? sr.Width()-100 : 16);
		
		w[0] = 15;
		w[1] = sr.right - tmp;
		w[2] = w[1] + (tmp-16)/2;
		w[3] = w[1] + (tmp-16);
		
		ctrlStatus.SetParts(4, w);
		// Layout showUI button in statusbar part #0
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
	
	QueueManager::getInstance()->removeListener(this);
	QueueFrame::frame = NULL;
	
	SettingsManager::getInstance()->set(SettingsManager::QUEUEFRAME_SHOW_TREE, ctrlShowTree.GetCheck() == BST_CHECKED);
	{
		Lock l(cs);
		for(QueueIter i = queue.begin(); i != queue.end(); ++i) {
			delete i->second;
		}
		queue.clear();
	}
	ctrlQueue.DeleteAllItems();
	
	string tmp1;
	string tmp2;
	
	ctrlQueue.GetColumnOrderArray(COLUMN_LAST, columnIndexes);
	for(int j = COLUMN_FIRST; j != COLUMN_LAST; j++) {
		columnSizes[j] = ctrlQueue.GetColumnWidth(j);
		tmp1 += Util::toString(columnIndexes[j]) + ",";
		tmp2 += Util::toString(columnSizes[j]) + ",";
	}
	tmp1.erase(tmp1.size()-1, 1);
	tmp2.erase(tmp2.size()-1, 1);
	
	SettingsManager::getInstance()->set(SettingsManager::QUEUEFRAME_ORDER, tmp1);
	SettingsManager::getInstance()->set(SettingsManager::QUEUEFRAME_WIDTHS, tmp2);
	
	bHandled = FALSE;		
	return 0;
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
		QueueItem* qi = j->second;

		StringList sl;
		StringListInfo li(qi);
		for(int k = 0; k < COLUMN_LAST; k++) {
			sl.push_back(li.columns[k]);
		}
		
		ctrlQueue.insert(ctrlQueue.GetItemCount(), sl, WinUtil::getIconIndex(qi->getTarget()), (LPARAM)qi);
	}
	ctrlQueue.SetRedraw(TRUE);
	ctrlQueue.resort();
	curDir = getSelectedDir();
}

void QueueFrame::onAction(QueueManagerListener::Types type, QueueItem* aQI) throw() { 
	switch(type) {
	case QueueManagerListener::ADDED: onQueueAdded(aQI); break;
	case QueueManagerListener::QUEUE_ITEM: onQueueAdded(aQI); onQueueUpdated(aQI); break;
	case QueueManagerListener::REMOVED: onQueueRemoved(aQI); break;
	case QueueManagerListener::MOVED: onQueueMoved(aQI); break;
	case QueueManagerListener::SOURCES_UPDATED: onQueueUpdated(aQI); break;
	case QueueManagerListener::STATUS_UPDATED: onQueueUpdated(aQI); break;
	default: break;
	}
};

/**
 * @file
 * $Id: QueueFrame.cpp,v 1.19 2003/04/15 10:14:03 arnetheduck Exp $
 */



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

#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "Resource.h"

#include "QueueFrame.h"
#include "SearchFrm.h"
#include "PrivateFrame.h"

#include "../client/SimpleXML.h"
#include "../client/StringTokenizer.h"

QueueFrame* QueueFrame::frame = NULL;

int QueueFrame::columnIndexes[] = { COLUMN_TARGET, COLUMN_STATUS, COLUMN_SIZE, COLUMN_PRIORITY,
COLUMN_USERS, COLUMN_PATH };

int QueueFrame::columnSizes[] = { 200, 300, 75, 75, 200, 200 };

static ResourceManager::Strings columnNames[] = { ResourceManager::FILENAME, ResourceManager::STATUS, ResourceManager::SIZE, 
	ResourceManager::PRIORITY, ResourceManager::USERS, ResourceManager::PATH };

LRESULT QueueFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{

	dcassert(frame == NULL);
	frame = this;
	
	CreateSimpleStatusBar(ATL_IDS_IDLEMESSAGE, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SBARS_SIZEGRIP);
	ctrlStatus.Attach(m_hWndStatusBar);
	
	ctrlQueue.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS, WS_EX_CLIENTEDGE, IDC_QUEUE);

	ctrlQueue.SetImageList(WinUtil::fileImages, LVSIL_SMALL);
	if(BOOLSETTING(FULL_ROW_SELECT)) {
		ctrlQueue.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP);
	}
	else {
		ctrlQueue.SetExtendedListViewStyle(LVS_EX_HEADERDRAGDROP);
	}

	ctrlDirectories.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS | LVS_SINGLESEL, WS_EX_CLIENTEDGE, IDC_DIRECTORIES);
	ctrlDirectories.SetImageList(WinUtil::fileImages, LVSIL_SMALL);

	m_nProportionalPos = 2500;
	SetSplitterPanes(ctrlDirectories.m_hWnd, ctrlQueue.m_hWnd);

	// Create listview columns
	StringList l = StringTokenizer(SETTING(QUEUEFRAME_ORDER), ',').getTokens();
	{
		int k = 0;
		for(StringIter i = l.begin(); i != l.end(); ++i) {
			if(k >= COLUMN_LAST)
				break;
			columnIndexes[k++] = Util::toInt(*i);
		}
	}
	
	l = StringTokenizer(SETTING(QUEUEFRAME_WIDTHS), ',').getTokens();
	{
		int k = 0;
		for(StringIter i = l.begin(); i != l.end(); ++i) {
			if(k >= COLUMN_LAST)
				break;
			columnSizes[k++] = Util::toInt(*i);
		}
	}
	
	LV_COLUMN lvc;
	ZeroMemory(&lvc, sizeof(lvc));
	lvc.mask = LVCF_FMT | LVCF_ORDER | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	
	for(int j=0; j<COLUMN_LAST; j++)
	{
		lvc.pszText = const_cast<char*>(ResourceManager::getInstance()->getString(columnNames[j]).c_str());
		lvc.fmt = j == COLUMN_SIZE ? LVCFMT_RIGHT : LVCFMT_LEFT;
		lvc.cx = columnSizes[j];
		lvc.iOrder = columnIndexes[j];
		lvc.iSubItem = j;
		ctrlQueue.InsertColumn(j, &lvc);
	}
	
	ctrlQueue.SetBkColor(WinUtil::bgColor);
	ctrlQueue.SetTextBkColor(WinUtil::bgColor);
	ctrlQueue.SetTextColor(WinUtil::textColor);
	ctrlDirectories.SetBkColor(WinUtil::bgColor);
	ctrlDirectories.SetTextBkColor(WinUtil::bgColor);
	ctrlDirectories.SetTextColor(WinUtil::textColor);
	
	ctrlDirectories.setSort(0, ExListViewCtrl::SORT_STRING_NOCASE);

	ctrlDirectories.InsertColumn(0, CSTRING(PATH), LVCFMT_LEFT, 200, 0);
	
	transferMenu.CreatePopupMenu();
	browseMenu.CreatePopupMenu();
	removeMenu.CreatePopupMenu();
	pmMenu.CreatePopupMenu();
	priorityMenu.CreatePopupMenu();
	
	transferMenu.AppendMenu(MF_STRING, IDC_SEARCH_ALTERNATES, CSTRING(SEARCH_FOR_ALTERNATES));
	transferMenu.AppendMenu(MF_POPUP, (UINT)(HMENU)priorityMenu, CSTRING(SET_PRIORITY));
	transferMenu.AppendMenu(MF_POPUP, (UINT)(HMENU)browseMenu, CSTRING(GET_FILE_LIST));
	transferMenu.AppendMenu(MF_POPUP, (UINT)(HMENU)pmMenu, CSTRING(SEND_PRIVATE_MESSAGE));
	transferMenu.AppendMenu(MF_POPUP, (UINT)(HMENU)removeMenu, CSTRING(REMOVE_SOURCE));
	transferMenu.AppendMenu(MF_STRING, IDC_REMOVE, CSTRING(REMOVE));

	priorityMenu.AppendMenu(MF_STRING, IDC_PRIORITY_PAUSED, CSTRING(PAUSED));
	priorityMenu.AppendMenu(MF_STRING, IDC_PRIORITY_LOW, CSTRING(LOW));
	priorityMenu.AppendMenu(MF_STRING, IDC_PRIORITY_NORMAL, CSTRING(NORMAL));
	priorityMenu.AppendMenu(MF_STRING, IDC_PRIORITY_HIGH, CSTRING(HIGH));

	SetWindowText(CSTRING(DOWNLOAD_QUEUE));

	QueueManager::getInstance()->getQueue();
	
	updateStatus();

	bHandled = FALSE;
	return 1;
}

void QueueFrame::onQueueAdded(QueueItem* aQI) {
	QueueItem* qi = new QueueItem(*aQI);
	{
		Lock l(cs);
		dcassert(queue.find(aQI) == queue.end());
		queue[aQI] = qi;
		if(!aQI->isSet(QueueItem::USER_LIST)) {
			queueSize+=aQI->getSize();
		}
		queueItems++;
	}
	StringListInfo* i = new StringListInfo((LPARAM)aQI);

	i->columns[COLUMN_TARGET] = aQI->getTargetFileName();
	i->columns[COLUMN_STATUS] = STRING(WAITING);
	if(aQI->getSize() == -1) {
		i->columns[COLUMN_SIZE] = STRING(UNKNOWN);
	} else {
		i->columns[COLUMN_SIZE] = Util::formatBytes(aQI->getSize()); 
	}
	switch(aQI->getPriority()) {
	case QueueItem::PAUSED: i->columns[COLUMN_PRIORITY] = STRING(PAUSED); break;
	case QueueItem::LOW: i->columns[COLUMN_PRIORITY] = STRING(LOW); break;
	case QueueItem::NORMAL: i->columns[COLUMN_PRIORITY] = STRING(NORMAL); break;
	case QueueItem::HIGH: i->columns[COLUMN_PRIORITY] = STRING(HIGH); break;
	default: dcassert(0); break;
	}
	i->columns[COLUMN_PATH] = aQI->getTarget();
	PostMessage(WM_SPEAKER, ADD_ITEM, (LPARAM)i);
}

void QueueFrame::onQueueRemoved(QueueItem* aQI) {
	{
		Lock l(cs);
		if(!aQI->isSet(QueueItem::USER_LIST)) {
			queueSize-=aQI->getSize();
			dcassert(queueSize >= 0);
		}
		queueItems--;
		dcassert(queueItems >= 0);
	}

	PostMessage(WM_SPEAKER, REMOVE_ITEM, (LPARAM) aQI);
}

void QueueFrame::onQueueUpdated(QueueItem* aQI) {

	{
		Lock l(cs);
		dcassert(queue.find(aQI) != queue.end());
		QueueItem* q = queue[aQI];
		for(QueueItem::Source::Iter i = q->getSources().begin(); i != q->getSources().end(); ++i) {
			delete *i;
		}
		q->getSources().clear();
		for(QueueItem::Source::Iter j = aQI->getSources().begin(); j != aQI->getSources().end(); ++j) {
			q->getSources().push_back(new QueueItem::Source(*(*j)));
		}
		q->setPriority(aQI->getPriority());
		q->setStatus(aQI->getStatus());
	}
	StringListInfo* i = new StringListInfo((LPARAM)aQI);

	string tmp;
	
	int online = 0;
	for(QueueItem::Source::Iter j = aQI->getSources().begin(); j != aQI->getSources().end(); ++j) {
		if(tmp.size() > 0)
			tmp += ", ";
		
		QueueItem::Source::Ptr sr = *j;
		if(sr->getUser()->isOnline())
			online++;
			
		tmp += sr->getUser()->getNick() + " (" + sr->getUser()->getClientName() + ")";
	}
	if(tmp.empty()) {
		tmp = "No users";
	}
	i->columns[COLUMN_USERS] = tmp;
	
	if(aQI->getStatus() == QueueItem::WAITING) {
		
		char buf[64];
		if(online > 0) {
			
			if(aQI->getSources().size() == 1) {
				i->columns[COLUMN_STATUS] = STRING(WAITING_USER_ONLINE);
			} else {
				sprintf(buf, CSTRING(WAITING_USERS_ONLINE), online, aQI->getSources().size());
				i->columns[COLUMN_STATUS] = buf;
			}
		} else {
			if(aQI->getSources().size() == 0) {
				i->columns[COLUMN_STATUS] = STRING(NO_USERS_TO_DOWNLOAD);
			} else if(aQI->getSources().size() == 1) {
				i->columns[COLUMN_STATUS] = STRING(USER_OFFLINE);
			} else if(aQI->getSources().size() == 2) {
				i->columns[COLUMN_STATUS] = STRING(BOTH_USERS_OFFLINE);
			} else {
				sprintf(buf, CSTRING(ALL_USERS_OFFLINE), aQI->getSources().size());
				i->columns[COLUMN_STATUS] = buf;
			}
		}
	} else if(aQI->getStatus() == QueueItem::RUNNING) {
		i->columns[COLUMN_STATUS] = STRING(RUNNING);
	} 
	
	switch(aQI->getPriority()) {
	case QueueItem::PAUSED: i->columns[COLUMN_PRIORITY] = STRING(PAUSED); break;
	case QueueItem::LOW: i->columns[COLUMN_PRIORITY] = STRING(LOW); break;
	case QueueItem::NORMAL: i->columns[COLUMN_PRIORITY] = STRING(NORMAL); break;
	case QueueItem::HIGH: i->columns[COLUMN_PRIORITY] = STRING(HIGH); break;
	default: dcassert(0); break;
	}
	
	PostMessage(WM_SPEAKER, SET_TEXT, (LPARAM)i);
}

LRESULT QueueFrame::onSpeaker(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
		
	if(wParam == ADD_ITEM) {
		StringListInfo* i = (StringListInfo*)lParam;
		StringList l;
		QueueItem* qi = NULL;
		{
			Lock l(cs);
			dcassert(queue.find((QueueItem*)i->lParam) != queue.end());
			qi = queue[(QueueItem*)i->lParam];
		}
		string dir = getDirectory(qi->getTarget());
		bool updateDir = directories.find(dir) == directories.end();
		directories.insert(make_pair(dir, qi));

		if(!updateDir && curDir == dir) {
			for(int j = 0; j < COLUMN_LAST; j++) {
				l.push_back(i->columns[j]);
			}
			dcassert(ctrlQueue.find(i->lParam) == -1);
			ctrlQueue.insert(l, WinUtil::IMAGE_FILE, (LPARAM)qi);
		} else if(directories.count(dir) == 1) {
			l.push_back(dir);
			ctrlDirectories.insert(l, WinUtil::IMAGE_DIRECTORY);
		}
		delete i;
		updateStatus();
	} else if(wParam == REMOVE_ITEM) {
		QueueItem* qi;
		{
			Lock l(cs);
			dcassert(queue.find((QueueItem*)lParam) != queue.end());
			qi = queue[(QueueItem*)lParam];
		}
		
		string dir = getDirectory(qi->getTarget());

		if(dir == curDir) {
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
			ctrlDirectories.deleteItem(dir);
			if(curDir == dir)
				curDir == Util::emptyString;
		}
	} else if(wParam == SET_TEXT) {
		StringListInfo* li = (StringListInfo*)lParam;
		QueueItem* qi;
		{
			Lock l(cs);
			dcassert(queue.find((QueueItem*)li->lParam) != queue.end());
			qi = queue[(QueueItem*)li->lParam];
		}
		
		string dir = getDirectory(qi->getTarget());
		if(dir == curDir) {
			int n = ctrlQueue.find((LPARAM)qi);
			dcassert(n != -1);
			ctrlQueue.SetRedraw(FALSE);
			for(int i = 0; i < COLUMN_LAST; i++) {
				if(!li->columns[i].empty()) {
					ctrlQueue.SetItemText(n, i, li->columns[i].c_str());
				}
			}
			ctrlQueue.SetRedraw(TRUE);
		}
		delete li;
	}

	return 0;
}

LRESULT QueueFrame::onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	RECT rc;                    // client area of window 
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };        // location of mouse click 
	
	// Get the bounding rectangle of the client area. 
	ctrlQueue.GetClientRect(&rc);
	ctrlQueue.ScreenToClient(&pt); 
	if (PtInRect(&rc, pt) && ctrlQueue.GetSelectedCount() > 0) 
	{ 
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
		
		if(ctrlQueue.GetSelectedCount() == 1) {
			LVITEM lvi;
			lvi.iItem = ctrlQueue.GetNextItem(-1, LVNI_SELECTED);
			lvi.iSubItem = 0;
			lvi.mask = LVIF_IMAGE | LVIF_PARAM;
			
			ctrlQueue.GetItem(&lvi);
			menuItems = 0;
			
			QueueItem* q = (QueueItem*)lvi.lParam;
			for(QueueItem::Source::Iter i = q->getSources().begin(); i != q->getSources().end(); ++i) {

				mi.fMask = MIIM_ID | MIIM_TYPE | MIIM_DATA;
				mi.fType = MFT_STRING;
				mi.dwTypeData = (LPSTR)(*i)->getUser()->getNick().c_str();
				mi.dwItemData = (DWORD)*i;
				mi.wID = IDC_BROWSELIST + menuItems;
				browseMenu.InsertMenuItem(menuItems, TRUE, &mi);
				mi.wID = IDC_REMOVE_SOURCE + menuItems;
				removeMenu.InsertMenuItem(menuItems, TRUE, &mi);
				if((*i)->getUser()) {
					mi.wID = IDC_PM + menuItems;
					pmMenu.InsertMenuItem(menuItems, TRUE, &mi);
				}
				menuItems++;
			}
		}
		
		ctrlQueue.ClientToScreen(&pt);
		
		transferMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);

		return TRUE; 
	}
	return FALSE; 
}

LRESULT QueueFrame::onSearchAlternates(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(ctrlQueue.GetSelectedCount() == 1) {
		string tmp;

		int i = ctrlQueue.GetNextItem(-1, LVNI_SELECTED);
		QueueItem* qi = (QueueItem*)ctrlQueue.GetItemData(i);
		
		StringList tok = StringTokenizer(qi->getTargetFileName(), ' ').getTokens();
		
		for(StringIter si = tok.begin(); si != tok.end(); ++si) {
			bool found = false;
			
			for(StringIter j = searchFilter.begin(); j != searchFilter.end(); ++j) {
				if(stricmp(si->c_str(), j->c_str()) == 0) {
					found = true;
				}
			}
			
			if(!found && !si->empty()) {
				tmp += *si + ' ';
			}
		}
		LONGLONG size = qi->getSize();
		if(!tmp.empty()) {
			SearchFrame* pChild = new SearchFrame();
			pChild->setTab(getTab());
			pChild->setInitial(tmp, ((size > 10*1024*1024) ? size-1: size + 1), ((size > 10*1024*1024) ? SearchManager::SIZE_ATLEAST : SearchManager::SIZE_ATMOST) );
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
		} catch(...) {
			// ...
		}
	}
	return 0;
}

LRESULT QueueFrame::onRemoveSource(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	
	if(ctrlQueue.GetSelectedCount() == 1) {
		string tmp;
		int i = ctrlQueue.GetNextItem(-1, LVNI_SELECTED);
		QueueItem* q = (QueueItem*)ctrlQueue.GetItemData(i);
		CMenuItemInfo mi;
		mi.fMask = MIIM_DATA;
		
		removeMenu.GetMenuItemInfo(wID, FALSE, &mi);
		QueueItem::Source* s = (QueueItem::Source*)mi.dwItemData;
		try {
			QueueManager::getInstance()->removeSource(q->getTarget(), s->getUser());
		} catch(...) {
			// ...
		}
	}
	return 0;
}

LRESULT QueueFrame::onPM(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	
	if(ctrlQueue.GetSelectedCount() == 1) {
		CMenuItemInfo mi;
		mi.fMask = MIIM_DATA;
		
		pmMenu.GetMenuItemInfo(wID, FALSE, &mi);
		QueueItem::Source* s = (QueueItem::Source*)mi.dwItemData;
		try {
			PrivateFrame::openWindow(s->getUser(), m_hWndMDIClient, getTab());
		} catch(...) {
			// ...
		}
	}
	return 0;
}
	
LRESULT QueueFrame::onPriority(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int i = -1;
	while( (i = ctrlQueue.GetNextItem(i, LVNI_SELECTED)) != -1) {
		QueueItem::Priority p;
		
		switch(wID) {
		case IDC_PRIORITY_PAUSED: p = QueueItem::PAUSED; break;
		case IDC_PRIORITY_LOW: p = QueueItem::LOW; break;
		case IDC_PRIORITY_NORMAL: p = QueueItem::NORMAL; break;
		case IDC_PRIORITY_HIGH: p = QueueItem::HIGH; break;
		default: p = QueueItem::NORMAL; break;
		}
		QueueManager::getInstance()->setPriority(((QueueItem*)ctrlQueue.GetItemData(i))->getTarget(), p);
	}
	return 0;
}

void QueueFrame::UpdateLayout(BOOL bResizeBars /* = TRUE */) {
	RECT rect;
	GetClientRect(&rect);
	// position bars and offset their dimensions
	UpdateBarsPosition(rect, bResizeBars);
	
	if(ctrlStatus.IsWindow()) {
		CRect sr;
		int w[3];
		ctrlStatus.GetClientRect(sr);
		int tmp = (sr.Width()) > 316 ? 216 : ((sr.Width() > 116) ? sr.Width()-100 : 16);
		
		w[0] = sr.right - tmp;
		w[1] = w[0] + (tmp-16)/2;
		w[2] = w[0] + (tmp-16);
		
		ctrlStatus.SetParts(3, w);
	}
	
	CRect rc = rect;
	SetSplitterRect(rc);
}

LRESULT QueueFrame::onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	
	QueueManager::getInstance()->removeListener(this);
	QueueFrame::frame = NULL;
	
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

LRESULT QueueFrame::onItemChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NMLISTVIEW* l = (NMLISTVIEW*)pnmh;
	if(l->uChanged == LVIF_STATE && (l->uNewState & LVIS_SELECTED)) {
		ctrlQueue.DeleteAllItems();
		char* buf = new char[MAX_PATH];
		ctrlDirectories.GetItemText(l->iItem, 0, buf, MAX_PATH);
		curDir = buf;

		pair<DirectoryIter, DirectoryIter> i = directories.equal_range(curDir);

		for(DirectoryIter j = i.first; j != i.second; ++j) {
			QueueItem* qi = j->second;

			StringList sl;
			
			sl.push_back(qi->getTargetFileName());

			string tmp;
			int online = 0;
			for(QueueItem::Source::Iter j = qi->getSources().begin(); j != qi->getSources().end(); ++j) {
				if(!tmp.empty())
					tmp += ", ";
				
				QueueItem::Source::Ptr sr = *j;
				if(sr->getUser()->isOnline())
					online++;
				
				tmp += sr->getUser()->getNick() + " (" + sr->getUser()->getClientName() + ")";
			}
			if(tmp.empty()) {
				tmp = "No users";
			}
			
			if(qi->getStatus() == QueueItem::WAITING) {
				char buf[64];
				if(online > 0) {
					if(qi->getSources().size() == 1) {
						sl.push_back(STRING(WAITING_USER_ONLINE));
					} else {
						sprintf(buf, CSTRING(WAITING_USERS_ONLINE), online, qi->getSources().size());
						sl.push_back(buf);
					}
				} else {
					if(qi->getSources().size() == 0) {
						sl.push_back(STRING(NO_USERS_TO_DOWNLOAD));
					} else if(qi->getSources().size() == 1) {
						sl.push_back(STRING(USER_OFFLINE));
					} else if(qi->getSources().size() == 2) {
						sl.push_back(STRING(BOTH_USERS_OFFLINE));
					} else {
						sprintf(buf, CSTRING(ALL_USERS_OFFLINE), qi->getSources().size());
						sl.push_back(buf);
					}
				}
			} else if(qi->getStatus() == QueueItem::RUNNING) {
				sl.push_back(STRING(RUNNING));
			} 
			
			if(qi->getSize() == -1) {
				sl.push_back(STRING(UNKNOWN));
			} else {
				sl.push_back(Util::formatBytes(qi->getSize())); 
			}
			switch(qi->getPriority()) {
			case QueueItem::PAUSED: sl.push_back(STRING(PAUSED)); break;
			case QueueItem::LOW: sl.push_back(STRING(LOW)); break;
			case QueueItem::NORMAL: sl.push_back(STRING(NORMAL)); break;
			case QueueItem::HIGH: sl.push_back(STRING(HIGH)); break;
			default: dcassert(0); break;
			}
			sl.push_back(tmp);
			sl.push_back(qi->getTarget());
			ctrlQueue.insert(sl, WinUtil::IMAGE_FILE, (LPARAM)qi);
		}
	}
	return 0;
}

/**
 * @file QueueFrame.cpp
 * $Id: QueueFrame.cpp,v 1.3 2002/04/16 16:45:55 arnetheduck Exp $
 */



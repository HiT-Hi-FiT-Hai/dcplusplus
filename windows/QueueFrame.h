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

#if !defined(AFX_QUEUEFRAME_H__8F6D05EC_ADCF_4987_8881_6DF3C0E355FA__INCLUDED_)
#define AFX_QUEUEFRAME_H__8F6D05EC_ADCF_4987_8881_6DF3C0E355FA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FlatTabCtrl.h"
#include "ExListViewCtrl.h"

#include "../client/QueueManager.h"
#include "../client/CriticalSection.h"

class QueueFrame : public MDITabChildWindowImpl<QueueFrame>, private QueueManagerListener, public CSplitterImpl<QueueFrame>
{
public:
	enum {
		ADD_ITEM,
		ADD_ITEMS,
		REMOVE_ITEM,
		SET_TEXT
	};
	
	enum {
		IDC_BROWSELIST = 3000,
		IDC_REMOVE_SOURCE = 3400,
		IDC_PM = 3600,
		IDC_PRIORITY_PAUSED = 4000,
		IDC_PRIORITY_LOWEST,
		IDC_PRIORITY_LOW,
		IDC_PRIORITY_NORMAL,
		IDC_PRIORITY_HIGH,
		IDC_PRIORITY_HIGHEST
	};

	DECLARE_FRAME_WND_CLASS_EX("QueueFrame", IDR_QUEUE, 0, COLOR_3DFACE);

	static QueueFrame* frame;

	QueueFrame() : menuItems(0), queueSize(0), queueItems(0), dirty(false), usingDirMenu(false) { 
		QueueManager::getInstance()->addListener(this);
		searchFilter.push_back("the");
		searchFilter.push_back("of");
		searchFilter.push_back("divx");
		searchFilter.push_back("frail");
	}

	~QueueFrame() { }
	
	virtual void OnFinalMessage(HWND /*hWnd*/) {
		
		delete this;
	}

	typedef MDITabChildWindowImpl<QueueFrame> baseClass;
	typedef CSplitterImpl<QueueFrame> splitBase;

	BEGIN_MSG_MAP(QueueFrame)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_FORWARDMSG, OnForwardMsg)
		MESSAGE_HANDLER(WM_CLOSE, onClose)
		MESSAGE_HANDLER(WM_SPEAKER, onSpeaker)
		MESSAGE_HANDLER(WM_CONTEXTMENU, onContextMenu)
		NOTIFY_HANDLER(IDC_QUEUE, LVN_COLUMNCLICK, onColumnClick)
		NOTIFY_HANDLER(IDC_QUEUE, LVN_KEYDOWN, onKeyDown)
		NOTIFY_HANDLER(IDC_DIRECTORIES, LVN_ITEMCHANGED, onItemChanged)
		COMMAND_ID_HANDLER(IDC_SEARCH_ALTERNATES, onSearchAlternates)
		COMMAND_ID_HANDLER(IDC_REMOVE, onRemove)
		COMMAND_RANGE_HANDLER(IDC_PRIORITY_PAUSED, IDC_PRIORITY_HIGHEST, onPriority)
		COMMAND_RANGE_HANDLER(IDC_BROWSELIST, IDC_BROWSELIST + menuItems, onBrowseList)
		COMMAND_RANGE_HANDLER(IDC_REMOVE_SOURCE, IDC_REMOVE_SOURCE + menuItems, onRemoveSource)
		COMMAND_RANGE_HANDLER(IDC_PM, IDC_PM + menuItems, onPM)
		CHAIN_MSG_MAP(splitBase);
		CHAIN_MSG_MAP(baseClass)
	END_MSG_MAP()

	LRESULT onPriority(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onBrowseList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onRemoveSource(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onPM(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onSearchAlternates(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onItemChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled);
	LRESULT onSpeaker(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled);
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	
	void UpdateLayout(BOOL bResizeBars = TRUE);

	LRESULT onRemove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		if(usingDirMenu) {
			removeSelectedDir();
		} else {
			removeSelected();
		}
		return 0;
	}

	LRESULT onKeyDown(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
		NMLVKEYDOWN* kd = (NMLVKEYDOWN*) pnmh;
		
		if(kd->wVKey == VK_DELETE) {
			removeSelected();
		} 
		return 0;
	}

	void removeSelected() {
		int i = -1;
		while( (i = ctrlQueue.GetNextItem(i, LVNI_SELECTED)) != -1) {
			QueueManager::getInstance()->remove(((QueueItem*)ctrlQueue.GetItemData(i))->getTarget());
		}
	}

	void removeSelectedDir() {
		if(ctrlDirectories.GetSelectedCount() != 1) {
			return;
		}
		int n = ctrlDirectories.GetNextItem(-1, LVNI_SELECTED);
		char* buf = new char[MAX_PATH];
		ctrlDirectories.GetItemText(n, 0, buf, MAX_PATH-1);
		DirectoryPair dp = directories.equal_range(buf);
		delete buf;
		for(DirectoryIter i = dp.first; i != dp.second; ++i) {
			QueueManager::getInstance()->remove(i->second->getTarget());
		}
	}
	
	static int sortSize(LPARAM a, LPARAM b) {
		LVITEM* c = (LVITEM*)a;
		LVITEM* d = (LVITEM*)b;
		
		QueueItem* e = (QueueItem*)c->lParam;
		QueueItem* f = (QueueItem*)d->lParam;
		LONGLONG g = e->getSize();
		LONGLONG h = f->getSize();
			
		return (g < h) ? -1 : ((g == h) ? 0 : 1);
	}
	
	LRESULT onColumnClick(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
		NMLISTVIEW* l = (NMLISTVIEW*)pnmh;
		if(l->iSubItem == ctrlQueue.getSortColumn()) {
			ctrlQueue.setSortDirection(!ctrlQueue.getSortDirection());
		} else {
			if(l->iSubItem == COLUMN_SIZE) {
				ctrlQueue.setSort(l->iSubItem, ExListViewCtrl::SORT_FUNC_ITEM, true, sortSize);
			} else {
				ctrlQueue.setSort(l->iSubItem, ExListViewCtrl::SORT_STRING_NOCASE);
			}
		}
		return 0;
	}

	LRESULT OnForwardMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
		LPMSG pMsg = (LPMSG)lParam;
		return baseClass::PreTranslateMessage(pMsg);
	}
	
private:

	enum {
		COLUMN_FIRST,
		COLUMN_TARGET = COLUMN_FIRST,
		COLUMN_STATUS,
		COLUMN_SIZE,
		COLUMN_PRIORITY,
		COLUMN_USERS,
		COLUMN_PATH,
		COLUMN_LAST
	};

	class StringListInfo;
	friend class StringListInfo;
	
	class StringListInfo {
	public:
		StringListInfo(QueueItem* aQi);
		QueueItem* qi;
		string columns[COLUMN_LAST];
	};
	
	CMenu transferMenu;
	CMenu browseMenu;
	CMenu removeMenu;
	CMenu pmMenu;
	CMenu priorityMenu;
	CMenu dirMenu;
	bool usingDirMenu;

	bool dirty;

	int menuItems;
	StringList searchFilter;

	typedef map<QueueItem*, QueueItem*> QueueMap;
	typedef QueueMap::iterator QueueIter;
	QueueMap queue;
	
	typedef HASH_MULTIMAP<string, QueueItem*> DirectoryMap;
	typedef DirectoryMap::iterator DirectoryIter;
	typedef pair<DirectoryIter, DirectoryIter> DirectoryPair;
	DirectoryMap directories;
	string curDir;
	
	CriticalSection cs;
	ExListViewCtrl ctrlQueue;
	ExListViewCtrl ctrlDirectories;
	
	CStatusBarCtrl ctrlStatus;
	
	LONGLONG queueSize;
	int queueItems;

	static int columnIndexes[COLUMN_LAST];
	static int columnSizes[COLUMN_LAST];
	
	void updateStatus() {
		if(dirty) {
			ctrlStatus.SetText(1, ("Items: " + Util::toString(queueItems)).c_str());
			ctrlStatus.SetText(2, ("Size: " + Util::formatBytes(queueSize)).c_str());
			dirty = false;
		}
	}

	string getDirectory(const string& aTarget) {
		string::size_type i, j, k;
		if( (i = aTarget.rfind('\\')) == string::npos) {
			return "\\";
		}
		if( (j = aTarget.rfind('\\', i-1)) == string::npos) {
			return aTarget.substr(0, i+1);
		}
		if( ((k = aTarget.rfind('\\', j-1)) == string::npos) || ((i-j) > 3)) {
			return aTarget.substr(j+1, i-j);
		}
		if(k > 3)
			return aTarget.substr(k+1, i-k);
		else
			return aTarget.substr(0, i+1);
	}

	virtual void onAction(QueueManagerListener::Types type, QueueItem* aQI) { 
		switch(type) {
		case QueueManagerListener::ADDED: onQueueAdded(aQI); break;
		case QueueManagerListener::QUEUE_ITEM: onQueueAdded(aQI); onQueueUpdated(aQI); break;
		case QueueManagerListener::REMOVED: onQueueRemoved(aQI); break;
		case QueueManagerListener::SOURCES_UPDATED: onQueueUpdated(aQI); break;
		case QueueManagerListener::STATUS_UPDATED: onQueueUpdated(aQI); break;
		default: dcassert(0); break;
		}
	};

	virtual void onAction(QueueManagerListener::Types type, const QueueItem::StringMap& l) { 
		switch(type) {
		case QueueManagerListener::QUEUE: onQueueList(l); break;
		default: dcassert(0); break;
		}
	};

	void onQueueList(const QueueItem::StringMap& l);
	void onQueueAdded(QueueItem* aQI);
	void onQueueRemoved(QueueItem* aQI);
	void onQueueUpdated(QueueItem* aQI);
	
};

#endif // !defined(AFX_QUEUEFRAME_H__8F6D05EC_ADCF_4987_8881_6DF3C0E355FA__INCLUDED_)

/**
 * @file QueueFrame.h
 * $Id: QueueFrame.h,v 1.7 2002/05/18 11:20:37 arnetheduck Exp $
 */


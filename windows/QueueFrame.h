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

class QueueFrame : public MDITabChildWindowImpl<QueueFrame>, private QueueManagerListener
{
public:
	enum {
		ADD_ITEM,
		REMOVE_ITEM,
		SET_TEXT
	};
	
	enum {
		IDC_BROWSELIST = 3000,
		IDC_REMOVE_SOURCE = 3400,
		IDC_PM = 3600,
		IDC_PRIORITY_PAUSED = 4000,
		IDC_PRIORITY_LOW = 4001,
		IDC_PRIORITY_NORMAL = 4002,
		IDC_PRIORITY_HIGH = 4003,
		
	};

	DECLARE_FRAME_WND_CLASS("QueueFrame", IDR_QUEUE);

	static QueueFrame* frame;

	QueueFrame() : menuItems(0), queueSize(0), queueItems(0) { 
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

	BEGIN_MSG_MAP(QueueFrame)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_FORWARDMSG, OnForwardMsg)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_CLOSE, onClose)
		MESSAGE_HANDLER(WM_SPEAKER, onSpeaker)
		MESSAGE_HANDLER(WM_CONTEXTMENU, onContextMenu)
		NOTIFY_HANDLER(IDC_QUEUE, LVN_COLUMNCLICK, onColumnClick)
		NOTIFY_HANDLER(IDC_QUEUE, LVN_KEYDOWN, onKeyDown)
		COMMAND_ID_HANDLER(IDC_SEARCH_ALTERNATES, onSearchAlternates)
		COMMAND_ID_HANDLER(IDC_REMOVE, onRemove)
		COMMAND_RANGE_HANDLER(IDC_PRIORITY_PAUSED, IDC_PRIORITY_HIGH, onPriority)
		COMMAND_RANGE_HANDLER(IDC_BROWSELIST, IDC_BROWSELIST + menuItems, onBrowseList)
		COMMAND_RANGE_HANDLER(IDC_REMOVE_SOURCE, IDC_REMOVE_SOURCE + menuItems, onRemoveSource)
		COMMAND_RANGE_HANDLER(IDC_PM, IDC_PM + menuItems, onPM)
		
		CHAIN_MSG_MAP(MDITabChildWindowImpl<QueueFrame>)
	END_MSG_MAP()

	LRESULT onPriority(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onBrowseList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onRemoveSource(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onPM(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onSearchAlternates(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled);
	LRESULT onSpeaker(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled);
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	
	void UpdateLayout(BOOL bResizeBars = TRUE);

	LRESULT onRemove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		removeSelected();
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
			string tmp;
			{
				Lock l(cs);
				QueueIter j = queue.find((QueueItem*)ctrlQueue.GetItemData(i));
				if(j == queue.end())
					continue;
				
				tmp = j->second->getTarget();
			}
			QueueManager::getInstance()->remove(tmp);
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
		return MDITabChildWindowImpl<QueueFrame>::PreTranslateMessage(pMsg);
	}
	
	LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		return 0;
	}
		
	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(&ps);
		FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_ACTIVEBORDER+1));
		EndPaint(&ps);
		return 0;
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
		StringListInfo(LPARAM lp = NULL) : lParam(lp) { };
		LPARAM lParam;
		string columns[COLUMN_LAST];
	};
	
	CMenu transferMenu;
	CMenu browseMenu;
	CMenu removeMenu;
	CMenu pmMenu;
	CMenu priorityMenu;
	
	int menuItems;
	StringList searchFilter;

	typedef map<QueueItem*, QueueItem*> QueueMap;
	typedef QueueMap::iterator QueueIter;
	QueueMap queue;
	
	CriticalSection cs;
	ExListViewCtrl ctrlQueue;
	CStatusBarCtrl ctrlStatus;
	
	LONGLONG queueSize;
	int queueItems;

	static int columnIndexes[COLUMN_LAST];
	static int columnSizes[COLUMN_LAST];
	
	void updateStatus() {
		ctrlStatus.SetText(1, ("Items: " + Util::toString(queueItems)).c_str());
		ctrlStatus.SetText(2, ("Size: " + Util::formatBytes(queueSize)).c_str());
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
	
	void onQueueAdded(QueueItem* aQI);
	void onQueueRemoved(QueueItem* aQI);
	void onQueueUpdated(QueueItem* aQI);
	
};

#endif // !defined(AFX_QUEUEFRAME_H__8F6D05EC_ADCF_4987_8881_6DF3C0E355FA__INCLUDED_)

/**
 * @file QueueFrame.h
 * $Id: QueueFrame.h,v 1.2 2002/04/13 12:57:23 arnetheduck Exp $
 */


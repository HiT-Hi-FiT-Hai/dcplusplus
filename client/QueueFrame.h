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
#include "QueueManager.h"
#include "ExListViewCtrl.h"

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

	QueueFrame() : stopperThread(NULL) { 
		QueueManager::getInstance()->addListener(this);
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
		NOTIFY_HANDLER(IDC_QUEUE, LVN_COLUMNCLICK, onColumnClick)
		NOTIFY_HANDLER(IDC_QUEUE, LVN_KEYDOWN, onKeyDown)
		CHAIN_MSG_MAP(MDITabChildWindowImpl<QueueFrame>)
	END_MSG_MAP()

	LRESULT onKeyDown(int idCtrl, LPNMHDR pnmh, BOOL& bHandled) {
		NMLVKEYDOWN* kd = (NMLVKEYDOWN*) pnmh;
		
		if(kd->wVKey == VK_DELETE) {
			removeSelected();
		} 
		return 0;
	}

	void removeSelected() {
		int i = -1;
		while( (i = ctrlQueue.GetNextItem(i, LVNI_SELECTED)) != -1) {
			QueueManager::getInstance()->remove((QueueItem*)ctrlQueue.GetItemData(i));
		}
	}
	
	
	LRESULT onColumnClick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled) {
		NMLISTVIEW* l = (NMLISTVIEW*)pnmh;
		if(l->iSubItem == ctrlQueue.getSortColumn()) {
			ctrlQueue.setSortDirection(!ctrlQueue.getSortDirection());
		} else {
			ctrlQueue.setSort(l->iSubItem, ExListViewCtrl::SORT_STRING_NOCASE);
		}
		return 0;
	}
	
	void UpdateLayout(BOOL bResizeBars = TRUE)
	{
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

		rc.bottom -= 2;
		rc.top += 2;
		rc.left +=2;
		rc.right -=2;
		ctrlQueue.MoveWindow(rc);

	}
	
	LRESULT onSpeaker(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled);
		
	LRESULT onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled) {
		DWORD id = 0;

		if(stopperThread) {
			if(WaitForSingleObject(stopperThread, 0) == WAIT_TIMEOUT) {
				// Hm, the thread's not finished stopping the client yet...post a close message and continue processing...
				PostMessage(WM_CLOSE);
				return 0;
			}
			CloseHandle(stopperThread);
			stopperThread = FALSE;
			bHandled = FALSE;
		} else {
			stopperThread = CreateThread(NULL, 0, stopper, this, 0, &id);
			
		}
		return 0;
	}

	static DWORD WINAPI stopper(void* p) {
		QueueFrame* f = (QueueFrame*)p;
		
		QueueManager::getInstance()->removeListener(f);
		f->PostMessage(WM_CLOSE);
		return 0;
	}
	
	LRESULT OnForwardMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
	{
		LPMSG pMsg = (LPMSG)lParam;
		
		return MDITabChildWindowImpl<QueueFrame>::PreTranslateMessage(pMsg);
	}
	
	LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
		return 0;
	}
		
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(&ps);
		FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_ACTIVEBORDER+1));
		EndPaint(&ps);
		return 0;
	}
	LRESULT OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	
	
private:

	enum {
		COLUMN_TARGET,
		COLUMN_STATUS,
		COLUMN_SIZE,
		COLUMN_PRIORITY,
		COLUMN_USERS,
		COLUMN_LAST
	};

	class StringListInfo;
	friend class StringListInfo;

	class StringListInfo {
	public:
		StringListInfo(LPARAM lp = NULL) : lParam(lp) { memset(columns, 0, sizeof(columns)); };
		LPARAM lParam;
		string columns[COLUMN_LAST];
	};
	
	
	HANDLE stopperThread;
	CMenu transferMenu;
	CMenu browseMenu;
	CMenu removeMenu;
	CMenu pmMenu;
	CMenu priorityMenu;
	
	virtual void onAction(QueueManagerListener::Types type, QueueItem* aQI) { 
		switch(type) {
		case QueueManagerListener::ADDED: onQueueAdded(aQI); break;
		case QueueManagerListener::QUEUE_ITEM: onQueueAdded(aQI); onQueueUpdated(aQI); break;
		case QueueManagerListener::REMOVED: onQueueRemoved(aQI); break;
		case QueueManagerListener::SOURCES_UPDATED: onQueueUpdated(aQI); break;
		case QueueManagerListener::STATUS_UPDATED: onQueueStatus(aQI); break;
		default: dcassert(0); break;
		}
	};
	
	void onQueueAdded(QueueItem* aQI);
	void onQueueRemoved(QueueItem* aQI);
	void onQueueUpdated(QueueItem* aQI);
	void onQueueStatus(QueueItem* aQI);
	
	ExListViewCtrl ctrlQueue;
	CStatusBarCtrl ctrlStatus;
	
};

#endif // !defined(AFX_QUEUEFRAME_H__8F6D05EC_ADCF_4987_8881_6DF3C0E355FA__INCLUDED_)

/**
 * @file QueueFrame.h
 * $Id: QueueFrame.h,v 1.3 2002/02/03 01:06:56 arnetheduck Exp $
 * @if LOG
 * $Log: QueueFrame.h,v $
 * Revision 1.3  2002/02/03 01:06:56  arnetheduck
 * More bugfixes and some minor changes
 *
 * Revision 1.2  2002/02/02 17:21:27  arnetheduck
 * Fixed search bugs and some other things...
 *
 * Revision 1.1  2002/02/01 02:00:40  arnetheduck
 * A lot of work done on the new queue manager, hopefully this should reduce
 * the number of crashes...
 *
 * Revision 1.2  2002/01/26 21:09:51  arnetheduck
 * Release 0.14
 *
 * Revision 1.1  2002/01/23 08:45:37  arnetheduck
 * New files for the notepad
 *
 * Revision 1.7  2002/01/20 22:54:46  arnetheduck
 * Bugfixes to 0.131 mainly...
 *
 * Revision 1.6  2002/01/14 22:19:43  arnetheduck
 * Commiting minor bugfixes
 *
 * Revision 1.5  2002/01/13 22:50:48  arnetheduck
 * Time for 0.12, added favorites, a bunch of new icons and lot's of other stuff
 *
 * Revision 1.4  2002/01/11 14:52:57  arnetheduck
 * Huge changes in the listener code, replaced most of it with templates,
 * also moved the getinstance stuff for the managers to a template
 *
 * Revision 1.3  2002/01/05 10:13:40  arnetheduck
 * Automatic version detection and some other updates
 *
 * Revision 1.2  2001/12/27 12:05:00  arnetheduck
 * Added flat tabs, fixed sorting and a StringTokenizer bug
 *
 * Revision 1.1  2001/12/21 20:21:17  arnetheduck
 * Private messaging added, and a lot of other updates as well...
 *
 * @endif
 */


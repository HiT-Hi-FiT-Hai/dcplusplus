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

#ifndef FINISHEDULFRAME_H
#define FINISHEDULFRAME_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FlatTabCtrl.h"
#include "ExListViewCtrl.h"

#include "../client/FinishedManager.h"

#define SERVER_MESSAGE_MAP 7

class FinishedULFrame : public MDITabChildWindowImpl<FinishedULFrame>, private FinishedManagerListener
{
public:
	FinishedULFrame() : totalBytes(0), totalTime(0), closed(false) { };
	virtual ~FinishedULFrame() { };

	DECLARE_FRAME_WND_CLASS_EX("FinishedULFrame", IDR_FINISHED_UL, 0, COLOR_3DFACE);
		
	virtual void OnFinalMessage(HWND /*hWnd*/) {
		frame = NULL;
		delete this;
	}

	BEGIN_MSG_MAP(FinishedULFrame)
		MESSAGE_HANDLER(WM_CREATE, onCreate)
		MESSAGE_HANDLER(WM_CLOSE, onClose)
		MESSAGE_HANDLER(WM_CONTEXTMENU, onContextMenu)
		MESSAGE_HANDLER(WM_SETFOCUS, onSetFocus)
		COMMAND_HANDLER(IDC_REMOVE, BN_CLICKED, onRemove)
		COMMAND_HANDLER(IDC_TOTAL, BN_CLICKED, onRemove)
		COMMAND_HANDLER(IDC_OPEN_FILE, BN_CLICKED, onOpenFile)
		COMMAND_HANDLER(IDC_OPEN_FOLDER, BN_CLICKED, onOpenFolder)
		NOTIFY_HANDLER(IDC_FINISHED_UL, LVN_COLUMNCLICK, onColumnClickFinished)
		NOTIFY_HANDLER(IDC_FINISHED_UL, LVN_KEYDOWN, onKeyDown)
		NOTIFY_HANDLER(IDC_FINISHED_UL, NM_DBLCLK, onDoubleClick)
		CHAIN_MSG_MAP(MDITabChildWindowImpl<FinishedULFrame>)
	END_MSG_MAP()
		
	LRESULT onCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onDoubleClick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT onRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT onOpenFile(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT onOpenFolder(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	
	LRESULT onSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /* bHandled */) {
		ctrlList.SetFocus();
		return 0;
	}

	LRESULT onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
		RECT rc;                    // client area of window 
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };        // location of mouse click 
		
		// Get the bounding rectangle of the client area. 
		ctrlList.GetClientRect(&rc);
		ctrlList.ScreenToClient(&pt); 
		
		if (ctrlList.GetSelectedCount() > 0 && PtInRect(&rc, pt)) 
		{ 
			ctrlList.ClientToScreen(&pt);
			ctxMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);			
			return TRUE; 
		}
		return FALSE; 
	}
	
	static int sortSize(LPARAM a, LPARAM b) {
		FinishedItem* c = (FinishedItem*)a;
		FinishedItem* d = (FinishedItem*)b;
		return compare(c->getSize(), d->getSize());
	}

	static int sortSpeed(LPARAM a, LPARAM b) {
		FinishedItem* c = (FinishedItem*)a;
		FinishedItem* d = (FinishedItem*)b;
		return compare(c->getAvgSpeed(), d->getAvgSpeed());
	}

	LRESULT onColumnClickFinished(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
		NMLISTVIEW* const l = (NMLISTVIEW*)pnmh;
		if(l->iSubItem == ctrlList.getSortColumn()) {
			ctrlList.setSortDirection(!ctrlList.getSortDirection());
		} else {
			switch(l->iSubItem) {
			case COLUMN_SIZE:
				ctrlList.setSort(l->iSubItem, ExListViewCtrl::SORT_FUNC, true, sortSize);
				break;
			case COLUMN_SPEED:
				ctrlList.setSort(l->iSubItem, ExListViewCtrl::SORT_FUNC, true, sortSpeed);
				break;
			default:
				ctrlList.setSort(l->iSubItem, ExListViewCtrl::SORT_STRING_NOCASE);
				break;
			}
		}
		return 0;
	}
	
	LRESULT onKeyDown(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
		NMLVKEYDOWN* kd = (NMLVKEYDOWN*) pnmh;
		
		if(kd->wVKey == VK_DELETE) {
			BOOL dummy;
			onRemove(0, IDC_REMOVE, 0, dummy);
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
			w[2] = sr.right - 16;
			w[1] = max(w[2] - 100, 0);
			w[0] = max(w[1] - 100, 0);

			ctrlStatus.SetParts(3, w);
		}
		
		CRect rc(rect);
		ctrlList.MoveWindow(rc);
	}
	
	static FinishedULFrame* frame;
	
private:
	enum {
		COLUMN_FIRST,
		COLUMN_DONE = COLUMN_FIRST,
		COLUMN_PATH,
		COLUMN_NICK,
		COLUMN_SIZE,
		COLUMN_SPEED,
		COLUMN_LAST
	};
	
	CStatusBarCtrl ctrlStatus;
	CMenu ctxMenu;
	
	ExListViewCtrl ctrlList;
	
	int64_t totalBytes;
	int64_t totalTime;

	bool closed;
	
	static int columnSizes[COLUMN_LAST];
	static int columnIndexes[COLUMN_LAST];
	
	void updateStatus() {
		ctrlStatus.SetText(1, Util::formatBytes(totalBytes).c_str());
		ctrlStatus.SetText(2, (Util::formatBytes((totalTime > 0) ? totalBytes * ((int64_t)1000) / totalTime : 0) + "/s").c_str());
	}

	void updateList(const FinishedItem::List& fl) {
		ctrlList.SetRedraw(FALSE);
		for(FinishedItem::List::const_iterator i = fl.begin(); i != fl.end(); ++i) {
			addEntry(*i, false);
		}
		ctrlList.SetRedraw(TRUE);
		ctrlList.Invalidate();
		updateStatus();
	}

	void addEntry(FinishedItem* entry, bool dirty = true);
	
	virtual void onAction(FinishedManagerListener::Types type, FinishedItem* entry) throw();;
};

#endif // FINISHEDULFRAME_H

/**
 * @file
 * $Id: FinishedULFrame.h,v 1.4 2003/05/13 11:34:07 arnetheduck Exp $
 */

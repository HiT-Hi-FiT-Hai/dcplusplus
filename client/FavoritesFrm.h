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

#if !defined(AFX_FAVORITEHUBSFRM_H__F6D75CA8_F229_4E7D_8ADC_0B1F3B0083C4__INCLUDED_)
#define AFX_FAVORITEHUBSFRM_H__F6D75CA8_F229_4E7D_8ADC_0B1F3B0083C4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FlatTabCtrl.h"
#include "HubManager.h"
#include "ExListViewCtrl.h"

#define SERVER_MESSAGE_MAP 7

class FavoriteHubsFrame : public MDITabChildWindowImpl<FavoriteHubsFrame>, private HubManagerListener
{
public:
	enum {
		COLUMN_NAME,
		COLUMN_DESCRIPTION,
		COLUMN_NICK,
		COLUMN_PASSWORD,
		COLUMN_SERVER
	};
	
	FavoriteHubsFrame() : stopperThread(NULL), users(0), hubs(0) {
		
	};

	virtual ~FavoriteHubsFrame() {
	};

	DECLARE_FRAME_WND_CLASS("FavoriteHubsFrame", IDR_FAVORITES);
		
	virtual void OnFinalMessage(HWND /*hWnd*/) {
		frame = NULL;
		delete this;
	}

	BEGIN_MSG_MAP(FavoriteHubsFrame)
		MESSAGE_HANDLER(WM_CREATE, onCreate)
		MESSAGE_HANDLER(WM_FORWARDMSG, OnForwardMsg)
		MESSAGE_HANDLER(WM_CLOSE, onClose)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_CONTEXTMENU, onContextMenu)
		COMMAND_HANDLER(IDC_CONNECT, BN_CLICKED, onClickedConnect)
		COMMAND_HANDLER(IDC_REMOVE, BN_CLICKED, onRemove)
		COMMAND_HANDLER(IDC_EDIT, BN_CLICKED, onEdit)
		NOTIFY_HANDLER(IDC_HUBLIST, LVN_COLUMNCLICK, onColumnClickHublist)
		NOTIFY_HANDLER(IDC_HUBLIST, NM_DBLCLK, onDoubleClickHublist)
		NOTIFY_HANDLER(IDC_HUBLIST, LVN_ITEMCHANGED, onItemChanged)
		CHAIN_MSG_MAP(MDITabChildWindowImpl<FavoriteHubsFrame>)
	END_MSG_MAP()
		

	bool checkNick() {
		if(SETTING(NICK).empty()) {
			MessageBox("Please enter a nickname in the settings dialog!");
			return false;
		}
		return true;
	}
	
	LRESULT onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled) {
		RECT rc;                    // client area of window 
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };        // location of mouse click 
		
		// Get the bounding rectangle of the client area. 
		ctrlHubs.GetClientRect(&rc);
		ctrlHubs.ScreenToClient(&pt); 
		
		if (PtInRect(&rc, pt)) 
		{ 
			ctrlHubs.ClientToScreen(&pt);
			hubsMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
			
			return TRUE; 
		}
		
		return FALSE; 
	}
	
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(&ps);
		FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_ACTIVEBORDER+1));
		EndPaint(&ps);
		return 0;
	}
	
	LRESULT OnForwardMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
	{
		LPMSG pMsg = (LPMSG)lParam;
		
		return CMDIChildWindowImpl2<FavoriteHubsFrame>::PreTranslateMessage(pMsg);
	}
	
	LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
		return 0;
	}
	LRESULT onCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	LRESULT onClose(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled) {
		DWORD id;
		if(stopperThread) {
			if(WaitForSingleObject(stopperThread, 0) == WAIT_TIMEOUT) {
				// Hm, the thread's not finished stopping the client yet...post a close message and continue processing...
				PostMessage(WM_CLOSE);
				return 0;
			}
			CloseHandle(stopperThread);
			stopperThread = NULL;
			bHandled = FALSE;
		} else {
			stopperThread = CreateThread(NULL, 0, stopper, this, 0, &id);
		}
		return 0;
	}

	LRESULT onItemChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled) {
		NMITEMACTIVATE* l = (NMITEMACTIVATE*)pnmh;
		if(l->iItem != -1) {
			FavoriteHubEntry* f = (FavoriteHubEntry*)ctrlHubs.GetItemData(l->iItem);
			f->setConnect(ctrlHubs.GetCheckState(l->iItem) != FALSE);
		}
		return 0;
	}
	
	LRESULT onDoubleClickHublist(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	
	LRESULT onColumnClickHublist(int idCtrl, LPNMHDR pnmh, BOOL& bHandled) {
		NMLISTVIEW* l = (NMLISTVIEW*)pnmh;
		if(l->iSubItem == ctrlHubs.getSortColumn()) {
			ctrlHubs.setSortDirection(!ctrlHubs.getSortDirection());
		} else {
			ctrlHubs.setSort(l->iSubItem, ExListViewCtrl::SORT_STRING_NOCASE);
		}
		return 0;
	}
	
	LRESULT onClickedConnect(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT onEdit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT onRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	
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
		rc.bottom -=28;
		ctrlHubs.MoveWindow(rc);
		
		rc = rect;
		rc.bottom -= 2;
		rc.top = rc.bottom - 22;

		rc.left = rc.right - 96;
		rc.right -= 2;
		ctrlConnect.MoveWindow(rc);
	}
	
	static FavoriteHubsFrame* frame;
	
private:
	int hubs;
	int users;
	CStatusBarCtrl ctrlStatus;
	CButton ctrlConnect;
	CButton ctrlRemove;
	CButton ctrlEdit;
	CMenu hubsMenu;
	
	ExListViewCtrl ctrlHubs;
	HANDLE stopperThread;
	
	static DWORD WINAPI stopper(void* p) {
		FavoriteHubsFrame* frm = (FavoriteHubsFrame*)p;
		HubManager::getInstance()->removeListener(frm);
		HubManager::getInstance()->reset();
		frm->PostMessage(WM_CLOSE);	
		return 0;
	}
	virtual void onAction(HubManagerListener::Types type, const FavoriteHubEntry::List& fl) {
		switch(type) {
		case HubManagerListener::GET_FAVORITE_HUBS: 
			for(FavoriteHubEntry::List::const_iterator i = fl.begin(); i != fl.end(); ++i) {
				FavoriteHubEntry* entry = *i;
				StringList l;
				l.push_back(entry->getName());
				l.push_back(entry->getDescription());
				l.push_back(entry->getNick(false));
				l.push_back(string(entry->getPassword().size(), '*'));
				l.push_back(entry->getServer());
				bool b = entry->getConnect();
				int i = ctrlHubs.insert(l, 0, (LPARAM)entry);
				ctrlHubs.SetCheckState(i, b);
			}
			break;
		}
	}
	
	virtual void onAction(HubManagerListener::Types type, FavoriteHubEntry* entry) {
		switch(type) {
		case HubManagerListener::FAVORITE_ADDED: {
				StringList l;
				l.push_back(entry->getName());
				l.push_back(entry->getDescription());
				l.push_back(entry->getNick(false));
				l.push_back(string(entry->getPassword().size(), '*'));
				l.push_back(entry->getServer());
				bool b = entry->getConnect();
				int i = ctrlHubs.insert(l, 0, (LPARAM)entry);
				ctrlHubs.SetCheckState(i, b);
			}

			break;
		case HubManagerListener::FAVORITE_REMOVED: 
			ctrlHubs.DeleteItem(ctrlHubs.find((LPARAM)entry));
			break;
		}
	};
	
};

#endif // !defined(AFX_FAVORITEHUBSFRM_H__F6D75CA8_F229_4E7D_8ADC_0B1F3B0083C4__INCLUDED_)

/**
 * @file FavoriteHubsFrm.h
 * $Id: FavoritesFrm.h,v 1.2 2002/01/20 22:54:46 arnetheduck Exp $
 * @if LOG
 * $Log: FavoritesFrm.h,v $
 * Revision 1.2  2002/01/20 22:54:46  arnetheduck
 * Bugfixes to 0.131 mainly...
 *
 * Revision 1.1  2002/01/13 22:53:26  arnetheduck
 * Favorites...
 *
 * @endif
 */


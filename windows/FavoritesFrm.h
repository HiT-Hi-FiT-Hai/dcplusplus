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

#if !defined(AFX_FAVORITEHUBSFRM_H__F6D75CA8_F229_4E7D_8ADC_0B1F3B0083C4__INCLUDED_)
#define AFX_FAVORITEHUBSFRM_H__F6D75CA8_F229_4E7D_8ADC_0B1F3B0083C4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FlatTabCtrl.h"
#include "ExListViewCtrl.h"

#include "../client/HubManager.h"

#define SERVER_MESSAGE_MAP 7

class FavoriteHubsFrame : public MDITabChildWindowImpl<FavoriteHubsFrame>, private HubManagerListener
{
public:
	FavoriteHubsFrame() { };
	virtual ~FavoriteHubsFrame() { };

	DECLARE_FRAME_WND_CLASS_EX("FavoriteHubsFrame", IDR_FAVORITES, 0, COLOR_3DFACE);
		
	virtual void OnFinalMessage(HWND /*hWnd*/) {
		frame = NULL;
		delete this;
	}

	BEGIN_MSG_MAP(FavoriteHubsFrame)
		MESSAGE_HANDLER(WM_CREATE, onCreate)
		MESSAGE_HANDLER(WM_FORWARDMSG, OnForwardMsg)
		MESSAGE_HANDLER(WM_CLOSE, onClose)
		MESSAGE_HANDLER(WM_CONTEXTMENU, onContextMenu)
		MESSAGE_HANDLER(WM_SETFOCUS, onSetFocus)
		COMMAND_HANDLER(IDC_CONNECT, BN_CLICKED, onClickedConnect)
		COMMAND_HANDLER(IDC_REMOVE, BN_CLICKED, onRemove)
		COMMAND_HANDLER(IDC_EDIT, BN_CLICKED, onEdit)
		COMMAND_HANDLER(IDC_NEWFAV, BN_CLICKED, onNew)
		NOTIFY_HANDLER(IDC_HUBLIST, LVN_COLUMNCLICK, onColumnClickHublist)
		NOTIFY_HANDLER(IDC_HUBLIST, NM_DBLCLK, onDoubleClickHublist)
		NOTIFY_HANDLER(IDC_HUBLIST, NM_RETURN, onEnter)
		NOTIFY_HANDLER(IDC_HUBLIST, LVN_ITEMCHANGED, onItemChanged)
		CHAIN_MSG_MAP(MDITabChildWindowImpl<FavoriteHubsFrame>)
	END_MSG_MAP()
		
	LRESULT onCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onDoubleClickHublist(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT onEnter(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT onClickedConnect(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT onEdit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT onRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT onNew(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	
	bool checkNick();
	
	LRESULT onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
		RECT rc;                    // client area of window 
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };        // location of mouse click 
		
		if(ctrlHubs.GetSelectedCount() > 0) {
			// Get the bounding rectangle of the client area. 
			ctrlHubs.GetClientRect(&rc);
			ctrlHubs.ScreenToClient(&pt); 

			if (PtInRect(&rc, pt)) 
			{ 
				ctrlHubs.ClientToScreen(&pt);
				hubsMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);

				return TRUE; 
			}
		}
		
		return FALSE; 
	}
	
	LRESULT OnForwardMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
		LPMSG pMsg = (LPMSG)lParam;
		return CMDIChildWindowImpl<FavoriteHubsFrame>::PreTranslateMessage(pMsg);
	}
	
	LRESULT onSetFocus(UINT /* uMsg */, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		ctrlHubs.SetFocus();
		return 0;
	}

	LRESULT onItemChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
		NMITEMACTIVATE* l = (NMITEMACTIVATE*)pnmh;
		if(l->iItem != -1) {
			FavoriteHubEntry* f = (FavoriteHubEntry*)ctrlHubs.GetItemData(l->iItem);
			f->setConnect(ctrlHubs.GetCheckState(l->iItem) != FALSE);
		}
		return 0;
	}
	
	LRESULT onColumnClickHublist(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
		NMLISTVIEW* l = (NMLISTVIEW*)pnmh;
		if(l->iSubItem == ctrlHubs.getSortColumn()) {
			ctrlHubs.setSortDirection(!ctrlHubs.getSortDirection());
		} else {
			ctrlHubs.setSort(l->iSubItem, ExListViewCtrl::SORT_STRING_NOCASE);
		}
		return 0;
	}
	
	void UpdateLayout(BOOL bResizeBars = TRUE);

	static FavoriteHubsFrame* frame;
	
private:
	enum {
		COLUMN_FIRST,
		COLUMN_NAME = COLUMN_FIRST,
		COLUMN_DESCRIPTION,
		COLUMN_NICK,
		COLUMN_PASSWORD,
		COLUMN_SERVER,
		COLUMN_USERDESCRIPTION,
		COLUMN_LAST
	};
	
	CStatusBarCtrl ctrlStatus;
	CButton ctrlConnect;
	CButton ctrlRemove;
	CButton ctrlEdit;
	CButton ctrlNew;
	CButton ctrlProps;
	CMenu hubsMenu;
	
	ExListViewCtrl ctrlHubs;
	
	static int columnSizes[COLUMN_LAST];
	static int columnIndexes[COLUMN_LAST];
	
	void updateList(const FavoriteHubEntry::List& fl) {
		ctrlHubs.SetRedraw(FALSE);
		for(FavoriteHubEntry::List::const_iterator i = fl.begin(); i != fl.end(); ++i) {
			addEntry(*i);
		}
		ctrlHubs.SetRedraw(TRUE);
		ctrlHubs.Invalidate();
	}

	void addEntry(FavoriteHubEntry* entry) {
		StringList l;
		l.push_back(entry->getName());
		l.push_back(entry->getDescription());
		l.push_back(entry->getNick(false));
		l.push_back(string(entry->getPassword().size(), '*'));
		l.push_back(entry->getServer());
		l.push_back(entry->getUserDescription(false));
		bool b = entry->getConnect();
		int i = ctrlHubs.insert(l, 0, (LPARAM)entry);
		ctrlHubs.SetCheckState(i, b);
	}
	
	virtual void onAction(HubManagerListener::Types type, FavoriteHubEntry* entry) throw();
};

#endif // !defined(AFX_FAVORITEHUBSFRM_H__F6D75CA8_F229_4E7D_8ADC_0B1F3B0083C4__INCLUDED_)

/**
 * @file FavoritesFrm.h
 * $Id: FavoritesFrm.h,v 1.8 2003/03/31 11:22:46 arnetheduck Exp $
 */


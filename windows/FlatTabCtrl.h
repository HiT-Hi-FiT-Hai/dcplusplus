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

#if !defined(AFX_FLATTABCTRL_H__FFFCBD5C_891D_44FB_B9F3_1DF83DA3EA83__INCLUDED_)
#define AFX_FLATTABCTRL_H__FFFCBD5C_891D_44FB_B9F3_1DF83DA3EA83__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../client/SettingsManager.h"
#include "../client/ResourceManager.h"

#include "WinUtil.h"

enum {
	FT_FIRST = WM_APP + 700,
	/** This will be sent when the user presses a tab. WPARAM = HWND */
	FTM_SELECTED,
	/** The number of rows changed */
	FTM_ROWS_CHANGED,
	/** Set currently active tab to the HWND pointed by WPARAM */
	FTM_SETACTIVE,
	/** Display context menu and return TRUE, or return FALSE for the default one */
	FTM_CONTEXTMENU,

};

template <class T, class TBase = CWindow, class TWinTraits = CControlWinTraits>
class ATL_NO_VTABLE FlatTabCtrlImpl : public CWindowImpl< T, TBase, TWinTraits> {
public:

	enum { FT_EXTRA_SPACE = 18 };

	FlatTabCtrlImpl() : closing(NULL), rows(1), height(0), active(NULL) { 
		black.CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
	};
	~FlatTabCtrlImpl() { }

	static LPCTSTR GetWndClassName()
	{
		return _T("FlatTabCtrl");
	}

	void addTab(HWND hWnd, COLORREF color = RGB(0, 0, 0)) {
		TabInfo* i = new TabInfo(hWnd, color);
		dcassert(getTabInfo(hWnd) == NULL);
		tabs.push_back(i);
		active = i;
		calcRows(false);
		Invalidate();		
	}

	void removeTab(HWND aWnd) {
		TabInfo::ListIter i;
		for(i = tabs.begin(); i != tabs.end(); ++i) {
			if((*i)->hWnd == aWnd)
				break;
		}

		dcassert(i != tabs.end());
		TabInfo* ti = *i;
		if(active == ti)
			active = NULL;
		delete ti;
		tabs.erase(i);
		calcRows(false);
		Invalidate();
	}

	void setActive(HWND aWnd) {
		TabInfo* ti = getTabInfo(aWnd);
		dcassert(ti != NULL);
		active = ti;
		ti->dirty = false;
		calcRows(false);
		Invalidate();
	}

	void setDirty(HWND aWnd) {
		TabInfo* ti = getTabInfo(aWnd);
		dcassert(ti != NULL);
		bool inval = ti->update();
		
		if(active != ti) {
			if(!ti->dirty) {
				ti->dirty = true;
				inval = true;
			}
		}

		if(inval) {
			calcRows(false);
			Invalidate();
		}
	}

	void setColor(HWND aWnd, COLORREF color) {
		TabInfo* ti = getTabInfo(aWnd);
		if(ti != NULL) {
			ti->pen.DeleteObject();
			ti->pen.CreatePen(PS_SOLID, 1, color);
			Invalidate();
		}
	}

	void updateText(HWND aWnd, LPCTSTR text) {
		TabInfo* ti = getTabInfo(aWnd);
		if(ti != NULL) {
			ti->updateText(text);
			calcRows(false);
			Invalidate();
		}
	}

	BEGIN_MSG_MAP(thisClass)
		MESSAGE_HANDLER(WM_SIZE, onSize)
		MESSAGE_HANDLER(WM_CREATE, onCreate)
		MESSAGE_HANDLER(WM_PAINT, onPaint)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, onLButtonDown)
		MESSAGE_HANDLER(WM_CONTEXTMENU, onContextMenu)
		COMMAND_ID_HANDLER(IDC_CLOSE_WINDOW, onCloseWindow)
		COMMAND_ID_HANDLER(IDC_CHEVRON, onChevron)
		COMMAND_RANGE_HANDLER(IDC_SELECT_WINDOW, IDC_SELECT_WINDOW+tabs.size(), onSelectWindow)
	END_MSG_MAP()

	LRESULT onLButtonDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
		int xPos = GET_X_LPARAM(lParam); 
		int yPos = GET_Y_LPARAM(lParam); 
		int row = getRows() - ((yPos / getTabHeight()) + 1);

		for(TabInfo::ListIter i = tabs.begin(); i != tabs.end(); ++i) {
			TabInfo* t = *i;
			if((row == t->row) && (xPos >= t->xpos) && (xPos < (t->xpos + t->getWidth())) ) {
				// Bingo, this was clicked
				HWND hWnd = GetParent();
				if(hWnd) {
					if(wParam & MK_SHIFT) ::SendMessage(t->hWnd, WM_CLOSE, 0, 0);
					else ::SendMessage(hWnd, FTM_SELECTED, (WPARAM)t->hWnd, 0);
				}
				break;
			}
		}
		return 0;
	}

	LRESULT onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };        // location of mouse click 

		ScreenToClient(&pt); 
		int xPos = pt.x;
		int row = getRows() - ((pt.y / getTabHeight()) + 1);

		for(TabInfo::ListIter i = tabs.begin(); i != tabs.end(); ++i) {
			TabInfo* t = *i;
			if((row == t->row) && (xPos >= t->xpos) && (xPos < (t->xpos + t->getWidth())) ) {
				// Bingo, this was clicked, check if the owner wants to handle it...
				if(!::SendMessage(t->hWnd, FTM_CONTEXTMENU, 0, lParam)) {
					closing = t->hWnd;
					ClientToScreen(&pt);
					CMenu mnu;
					mnu.CreatePopupMenu();
					mnu.AppendMenu(MF_STRING, IDC_CLOSE_WINDOW, CSTRING(CLOSE));
					mnu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_BOTTOMALIGN, pt.x, pt.y, m_hWnd);
				}
				break;
			}
		}
		return 0;
	}

	LRESULT onCloseWindow(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		::SendMessage(closing, WM_CLOSE, 0, 0);
		return 0;
	}

	int getTabHeight() { return height; };
	int getHeight() { return (getRows() * getTabHeight())+1; };
	int getFill() { return (getTabHeight() + 1) / 2; };

	int getRows() { return rows; };

	void calcRows(bool inval = true) {
		CRect rc;
		GetClientRect(rc);
		int r = 1;
		int w = 0;
		bool notify = false;
		bool needInval = false;

		for(TabInfo::ListIter i = tabs.begin(); i != tabs.end(); ++i) {
			TabInfo* ti = *i;
			if( (r != 0) && ((w + ti->getWidth() + getFill()) > rc.Width()) ) {
				if(r >= SETTING(MAX_TAB_ROWS)) {
					notify |= (rows != r);
					rows = r;
					r = 0;
					chevron.EnableWindow(TRUE);
				} else {
					r++;
					w = 0;
				}
			} 
			ti->xpos = w;
			needInval |= (ti->row != (r-1));
			ti->row = r-1;
			w += ti->getWidth();
		}

		if(r != 0) {
			chevron.EnableWindow(FALSE);
			notify |= (rows != r);
			rows = r;
		}

		if(notify) {
			::SendMessage(GetParent(), FTM_ROWS_CHANGED, 0, 0);
		}
		if(needInval && inval)
			Invalidate();
	}

	LRESULT onCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) { 
		chevron.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
			BS_PUSHBUTTON , 0, IDC_CHEVRON);
		chevron.SetWindowText("»");

		mnu.CreatePopupMenu();

		CDC dc(::GetDC(m_hWnd));
		HFONT oldfont = dc.SelectFont(WinUtil::font);
		height = WinUtil::getTextHeight(dc) + 2;
		dc.SelectFont(oldfont);
		::ReleaseDC(m_hWnd, dc);
		
		return 0;
	}

	LRESULT onSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) { 
		calcRows();
		SIZE sz = { LOWORD(lParam), HIWORD(lParam) };
		chevron.MoveWindow(sz.cx-14, 1, 14, getHeight());
		return 0;
	}
		
	LRESULT onPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		RECT rc;
		bool drawActive = false;
		RECT crc;
		GetClientRect(&crc);

		if(GetUpdateRect(&rc, FALSE)) {
			CPaintDC dc(m_hWnd);
			HFONT oldfont = dc.SelectFont(WinUtil::font);

			//ATLTRACE("%d, %d\n", rc.left, rc.right);
			for(TabInfo::ListIter i = tabs.begin(); i != tabs.end(); ++i) {
				TabInfo* t = *i;
				
				if(t->row != -1 && t->xpos < rc.right && t->xpos + t->getWidth() + getFill() >= rc.left ) {
					if(t != active) {
						drawTab(dc, t, t->xpos, t->row);
					} else {
						drawActive = true;
					}
				}
			}
			HPEN oldpen = dc.SelectPen(black);
			for(int r = 0; r < rows; r++) {
				dc.MoveTo(rc.left, r*getTabHeight());
				dc.LineTo(rc.right, r*getTabHeight());
			}

			if(drawActive) {
				dcassert(active);
				drawTab(dc, active, active->xpos, active->row, true);
				dc.SelectPen(active->pen);
				int y = (rows - active->row -1) * getTabHeight();
				dc.MoveTo(active->xpos, y);
				dc.LineTo(active->xpos + active->getWidth() + getFill(), y);
			}
			dc.SelectPen(oldpen);
			dc.SelectFont(oldfont);
		}
		return 0;
	}

	LRESULT onChevron(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		while(mnu.GetMenuItemCount() > 0) {
			mnu.RemoveMenu(0, MF_BYPOSITION);
		}
		int n = 0;
		CRect rc;
		GetClientRect(&rc);
		CMenuItemInfo mi;
		mi.fMask = MIIM_ID | MIIM_TYPE | MIIM_DATA | MIIM_STATE;
		mi.fType = MFT_STRING | MFT_RADIOCHECK;

		for(TabInfo::ListIter i = tabs.begin(); i != tabs.end(); ++i) {
			TabInfo* ti = *i;
			if(ti->row == -1) {
				mi.dwTypeData = (LPSTR)ti->name;
				mi.dwItemData = (DWORD)ti->hWnd;
				mi.fState = MFS_ENABLED | (ti->dirty ? MFS_CHECKED : 0);
				mi.wID = IDC_SELECT_WINDOW + n;
				mnu.InsertMenuItem(n++, TRUE, &mi);
			} 
		}

		POINT pt;
		chevron.GetClientRect(&rc);
		pt.x = rc.right - rc.left;
		pt.y = 0;
		chevron.ClientToScreen(&pt);
		
		mnu.TrackPopupMenu(TPM_RIGHTALIGN | TPM_BOTTOMALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
		return 0;
	}

	LRESULT onSelectWindow(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		CMenuItemInfo mi;
		mi.fMask = MIIM_DATA;
		
		mnu.GetMenuItemInfo(wID, FALSE, &mi);
		HWND hWnd = GetParent();
		if(hWnd) {
			SendMessage(hWnd, FTM_SELECTED, (WPARAM)mi.dwItemData, 0);
		}
		return 0;		
	}
private:
	class TabInfo {
	public:

		typedef vector<TabInfo*> List;
		typedef typename List::iterator ListIter;

		enum { MAX_LENGTH = 20 };

		TabInfo(HWND aWnd, COLORREF c) : hWnd(aWnd), len(0), xpos(0), row(0), dirty(false) { 
			pen.CreatePen(PS_SOLID, 1, c);
			memset(&size, 0, sizeof(size));
			memset(&boldSize, 0, sizeof(boldSize));
			name[0] = 0;
			update();
		};

		HWND hWnd;
		CPen pen;
		char name[MAX_LENGTH];
		int len;
		SIZE size;
		SIZE boldSize;
		int xpos;
		int row;
		bool dirty;

		bool update() {
			char name2[MAX_LENGTH];
			len = ::GetWindowTextLength(hWnd);
			if(len >= MAX_LENGTH) {
				::GetWindowText(hWnd, name2, MAX_LENGTH - 3);
				name2[MAX_LENGTH - 4] = '.';
				name2[MAX_LENGTH - 3] = '.';
				name2[MAX_LENGTH - 2] = '.';
				name2[MAX_LENGTH - 1] = 0;
				len = MAX_LENGTH - 1;
			} else {
				::GetWindowText(hWnd, name2, MAX_LENGTH);
			}
			if(strcmp(name, name2) == 0) {
				return false;
			}
			strcpy(name, name2);
			CDC dc(::GetDC(hWnd));
			HFONT f = dc.SelectFont(WinUtil::font);
			dc.GetTextExtent(name, len, &size);
			dc.SelectFont(WinUtil::boldFont);
			dc.GetTextExtent(name, len, &boldSize);
			dc.SelectFont(f);		
			::ReleaseDC(hWnd, dc);
			return true;
		};

		bool updateText(LPCTSTR text) {
			len = strlen(text);
			if(len >= MAX_LENGTH) {
				::strncpy(name, text, MAX_LENGTH - 3);
				name[MAX_LENGTH - 4] = '.';
				name[MAX_LENGTH - 3] = '.';
				name[MAX_LENGTH - 2] = '.';
				name[MAX_LENGTH - 1] = 0;
				len = MAX_LENGTH - 1;
			} else {
				strcpy(name, text);
			}
			CDC dc(::GetDC(hWnd));
			HFONT f = dc.SelectFont(WinUtil::font);
			dc.GetTextExtent(name, len, &size);
			dc.SelectFont(WinUtil::boldFont);
			dc.GetTextExtent(name, len, &boldSize);
			dc.SelectFont(f);		
			::ReleaseDC(hWnd, dc);
			return true;
		};

		int getWidth() {
			return (dirty ? boldSize.cx : size.cx) + FT_EXTRA_SPACE;
		}
	};

	HWND closing;
	CButton chevron;
	CMenu mnu;
	
	int rows;
	int height;

	TabInfo* active;
	TabInfo::List tabs;
	CPen black;

	TabInfo* getTabInfo(HWND aWnd) {
		for(TabInfo::ListIter i	= tabs.begin(); i != tabs.end(); ++i) {
			if((*i)->hWnd == aWnd)
				return *i;
		}
		return NULL;
	}

	/**
	 * Draws a tab
	 * @return The width of the tab
	 */
	void drawTab(CDC& dc, TabInfo* tab, int pos, int row, bool aActive = false) {
		
		int ypos = (getRows() - row - 1) * getTabHeight();

		HPEN oldpen = dc.SelectPen(black);
		
		POINT p[4];
		dc.BeginPath();
		dc.MoveTo(pos, ypos);
		p[0].x = pos + tab->getWidth() + getFill();
		p[0].y = ypos;
		p[1].x = pos + tab->getWidth();
		p[1].y = ypos + getTabHeight();
		p[2].x = pos + getFill();
		p[2].y = ypos + getTabHeight();
		p[3].x = pos;
		p[3].y = ypos;
		
		dc.PolylineTo(p, 4);
		dc.CloseFigure();
		dc.EndPath();
		
		HBRUSH oldbrush = dc.SelectBrush(GetSysColorBrush(aActive ? COLOR_WINDOW : COLOR_BTNFACE));
		dc.FillPath();
		
		dc.MoveTo(p[1].x + 1, p[1].y);
		dc.LineTo(p[0].x + 1, p[0].y);
		dc.MoveTo(p[2]);
		dc.LineTo(p[3]);
		if(!active || (tab->row != (rows - 1)) )
			dc.LineTo(p[0]);
		
		dc.SelectPen(tab->pen);
		dc.MoveTo(p[1]);
		dc.LineTo(p[0]);
		dc.MoveTo(p[1]);
		dc.LineTo(p[2]);
		
		dc.SelectPen(oldpen);
		dc.SelectBrush(oldbrush);
		
		dc.SetBkMode(TRANSPARENT);

		if(tab->dirty) {
			HFONT f = dc.SelectFont(WinUtil::boldFont);
			dc.TextOut(pos + getFill() / 2 + FT_EXTRA_SPACE / 2, ypos + 1, tab->name, tab->len);
			dc.SelectFont(f);		
		} else {
			dc.TextOut(pos + getFill() / 2 + FT_EXTRA_SPACE / 2, ypos + 1, tab->name, tab->len);
		}
	};
};

class FlatTabCtrl : public FlatTabCtrlImpl<FlatTabCtrl> {
public:
	DECLARE_FRAME_WND_CLASS_EX(GetWndClassName(), IDR_QUEUE, 0, COLOR_3DFACE);
};

template <class T, int C = RGB(128, 128, 128), class TBase = CMDIWindow, class TWinTraits = CMDIChildWinTraits>
class ATL_NO_VTABLE MDITabChildWindowImpl : public CMDIChildWindowImpl<T, TBase, TWinTraits> {
public:

	MDITabChildWindowImpl() : created(false) { };
	FlatTabCtrl* getTab() { return WinUtil::tabCtrl; };

 	typedef MDITabChildWindowImpl<T, C, TBase, TWinTraits> thisClass;
	typedef CMDIChildWindowImpl<T, TBase, TWinTraits> baseClass;
	BEGIN_MSG_MAP(thisClass>)
		MESSAGE_HANDLER(WM_FORWARDMSG, onForwardMsg)
		MESSAGE_HANDLER(WM_CREATE, onCreate)
		MESSAGE_HANDLER(WM_MDIACTIVATE, onMDIActivate)
		MESSAGE_HANDLER(WM_DESTROY, onDestroy)
		MESSAGE_HANDLER(WM_SETTEXT, onSetText)
		CHAIN_MSG_MAP(baseClass)
	END_MSG_MAP()
		
	// Fix window maximization state issues...code taken from www.codeproject.com, article by david bowen
	HWND Create(HWND hWndParent, WTL::_U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
	DWORD dwStyle = 0, DWORD dwExStyle = 0,
	UINT nMenuID = 0, LPVOID lpCreateParam = NULL)
	{
		// NOTE: hWndParent is going to become m_hWndMDIClient
		//  in CMDIChildWindowImpl::Create
		ATLASSERT(::IsWindow(hWndParent));
		
		BOOL bMaximized = FALSE;
		if(::SendMessage(hWndParent, WM_MDIGETACTIVE, 0, (LPARAM)&bMaximized) == NULL)
			bMaximized = BOOLSETTING(MDI_MAXIMIZED);
		
		if(bMaximized == TRUE) {
			::SendMessage(hWndParent, WM_SETREDRAW, FALSE, 0);
		}
		
		HWND hWnd = baseClass::Create(hWndParent, rect, szWindowName, dwStyle, dwExStyle, nMenuID, lpCreateParam);
		
		if(bMaximized == TRUE) {
			::ShowWindow(hWnd, SW_SHOWMAXIMIZED);
			
			::SendMessage(hWndParent, WM_SETREDRAW, TRUE, 0);
			::RedrawWindow(hWndParent, NULL, NULL,
				RDW_INVALIDATE | RDW_ALLCHILDREN);
		}
		
		return hWnd;
	}

	// All MDI windows must have this in wtl it seems to handle ctrl-tab and so on...
	LRESULT onForwardMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
		return baseClass::PreTranslateMessage((LPMSG)lParam);
	}

	LRESULT onCreate(UINT /* uMsg */, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
		bHandled = FALSE;
		dcassert(getTab());
		getTab()->addTab(m_hWnd, C);
		created = true;
		return 0;
	}
	
	LRESULT onMDIActivate(UINT /*uMsg*/, WPARAM /*wParam */, LPARAM lParam, BOOL& bHandled) {
		dcassert(getTab());
		if((m_hWnd == (HWND)lParam))
			getTab()->setActive(m_hWnd);

		bHandled = FALSE;
		return 1; 
	}

	LRESULT onDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
		bHandled = FALSE;
		dcassert(getTab());
		getTab()->removeTab(m_hWnd);

		BOOL bMaximized = FALSE;
		if(::SendMessage(m_hWndMDIClient, WM_MDIGETACTIVE, 0, (LPARAM)&bMaximized) != NULL)
			SettingsManager::getInstance()->set(SettingsManager::MDI_MAXIMIZED, (bMaximized>0));

		return 0;
	}

	LRESULT onSetText(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled) {
		bHandled = FALSE;
		dcassert(getTab());
		if(created) {
			getTab()->updateText(m_hWnd, (LPCTSTR)lParam);
		}
		return 0;
	}

	void setDirty() {
		dcassert(getTab());
		getTab()->setDirty(m_hWnd);
	}
	void setTabColor(COLORREF color) {
		dcassert(getTab());
		getTab()->setColor(m_hWnd, color);
	}

private:
	bool created;
};

#endif // !defined(AFX_FLATTABCTRL_H__FFFCBD5C_891D_44FB_B9F3_1DF83DA3EA83__INCLUDED_)

/**
 * @file
 * $Id: FlatTabCtrl.h,v 1.22 2003/11/27 10:33:15 arnetheduck Exp $
 */

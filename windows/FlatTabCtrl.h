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

#if !defined(AFX_FLATTABCTRL_H__FFFCBD5C_891D_44FB_B9F3_1DF83DA3EA83__INCLUDED_)
#define AFX_FLATTABCTRL_H__FFFCBD5C_891D_44FB_B9F3_1DF83DA3EA83__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/** This will be sent when the user presses a tab. WPARAM = HWND */
#define FTN_SELECTED (WM_APP + 700)
/** Set currently active tab to the HWND pointed by WPARAM */
#define FTM_SETACTIVE (WM_APP + 701)

#define IDC_SELECT_WINDOW 6000

template <class T, class TBase = CWindow, class TWinTraits = CControlWinTraits>
class ATL_NO_VTABLE FlatTabCtrlImpl : public CWindowImpl< T, TBase, TWinTraits> {
public:
	FlatTabCtrlImpl() : active(NULL), boldFont(NULL), closing(NULL) { };

	static LPCTSTR GetWndClassName()
	{
		return _T("FlatTabCtrl");
	}

	void addTab(HWND hWnd) {
		TabInfo* i = new TabInfo(hWnd);
		tabs.push_back(i);
		active = i;
		Invalidate();		
	}

	void removeTab(HWND aWnd) {
		for(vector<TabInfo*>::iterator i = tabs.begin(); i != tabs.end(); ++i) {
			if((*i)->hWnd == aWnd) {
				if(*i == active)
					active = NULL;
				delete *i;
				tabs.erase(i);
				Invalidate();
				break;
			}
		}
	}

	void setActive(HWND aWnd) {
		for(vector<TabInfo*>::iterator i = tabs.begin(); i != tabs.end(); ++i) {
			if((*i)->hWnd == aWnd) {
				active = *i;
				(*i)->dirty = false;
				Invalidate();
				break;
			}
		}
	}

	void setDirty(HWND aWnd) {
		for(vector<TabInfo*>::iterator i = tabs.begin(); i != tabs.end(); ++i) {
			if((*i)->hWnd == aWnd) {
				if(active != (*i)) {
					(*i)->dirty = true;
					Invalidate();
				}
				break;
			}
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

	LRESULT onLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
		int pos = 0;
		int xPos = GET_X_LPARAM(lParam); 
//		int yPos = GET_Y_LPARAM(lParam); 
		for(vector<TabInfo*>::iterator i = tabs.begin(); i != tabs.end(); ++i) {
			TabInfo* t = *i;
			if(xPos > pos && xPos < pos + t->getWidth()) {
				// Bingo, this was clicked
				HWND hWnd = GetParent();
				if(hWnd) {
					SendMessage(hWnd, FTN_SELECTED, (WPARAM)t->hWnd, 0);
					break;
				}
			}
			pos += t->getWidth();
		}
		return 0;
	}

	LRESULT onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };        // location of mouse click 

		ScreenToClient(&pt); 
		int xPos = pt.x;
		int pos = 0;

		for(vector<TabInfo*>::iterator i = tabs.begin(); i != tabs.end(); ++i) {
			TabInfo* t = *i;
			if(xPos > pos && xPos < pos + t->getWidth()) {
				// Bingo, this was clicked
				closing = t->hWnd;
				ClientToScreen(&pt);
				CMenu mnu;
				mnu.CreatePopupMenu();
				mnu.AppendMenu(MF_STRING, IDC_CLOSE_WINDOW, CSTRING(CLOSE));
				mnu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
				break;
			}
			pos += t->getWidth();
		}
		return 0;
	}

	LRESULT onCloseWindow(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		::SendMessage(closing, WM_CLOSE, 0, 0);
		return 0;
	}

	int getTabHeight() { return 15; };
	int getHeight() { return getTabHeight()+1; };

	LRESULT onCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) { 
		chevron.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
			BS_PUSHBUTTON , 0, IDC_CHEVRON);
		chevron.SetWindowText("»");

		mnu.CreatePopupMenu();
		
		return 0;
	}

	LRESULT onSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) { 
		SIZE sz = { LOWORD(lParam), HIWORD(lParam) };
		chevron.MoveWindow(sz.cx-14, 0, 14, sz.cy);
		return 0;
	}
		
	LRESULT onPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		int pos = 0;
		int activepos = -1;
		RECT rc;
		bool fits = true;
		if(GetUpdateRect(&rc, FALSE)) {
			CPaintDC dc(m_hWnd);
			HFONT oldfont = dc.SelectFont((HFONT)GetStockObject(DEFAULT_GUI_FONT));

			if(boldFont == NULL) {
				LOGFONT lf;
				::GetObject((HFONT)GetStockObject(DEFAULT_GUI_FONT), sizeof(lf), &lf);

				lf.lfWeight = FW_BOLD;
				boldFont = CreateFontIndirect(&lf);
			}
			//ATLTRACE("%d, %d\n", rc.left, rc.right);
			
			for(vector<TabInfo*>::iterator i = tabs.begin(); i != tabs.end(); ++i) {
				TabInfo* t = *i;
				t->update(dc, boldFont);
				if(pos + t->getWidth() + t->getFill() > rc.right) {
					fits = false;
				}
				if(pos <= rc.right && (pos + t->getWidth() + t->getFill()) >= rc.left) {
					if(*i == active) {
						activepos = pos;
						pos+=(*i)->getWidth();
					} else {
						pos += drawTab(dc, *i, pos);
					}
					
				} else {
					pos += t->getWidth();
				}
			}
			chevron.EnableWindow(!fits);

			if(active) {
				if(activepos != -1 && activepos <= rc.right && (activepos + active->getWidth() + active->getFill()) >= rc.left) {
					drawTab(dc, active, activepos, true);
				}
				dc.MoveTo(0, 0);
				dc.LineTo(activepos+1, 0);
				dc.MoveTo(activepos + active->getWidth() + (getTabHeight()+1) / 2, 0);
				dc.LineTo(rc.right, 0);
				dc.SelectFont(oldfont);
			}
		}
		return 0;
	}

	LRESULT onChevron(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		while(mnu.GetMenuItemCount() > 0) {
			mnu.RemoveMenu(0, MF_BYPOSITION);
		}
		int n = 0;
		int pos = 0;
		RECT rc;
		GetClientRect(&rc);
		CMenuItemInfo mi;
		mi.fMask = MIIM_ID | MIIM_TYPE | MIIM_DATA;
		mi.fType = MFT_STRING;
		
		for(vector<TabInfo*>::iterator i = tabs.begin(); i != tabs.end(); ++i) {
			pos += (*i)->getWidth();

			if(pos + (*i)->getFill() > rc.right) {
				mi.dwTypeData = (LPSTR)(*i)->name;
				mi.dwItemData = (DWORD)(*i)->hWnd;
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
			SendMessage(hWnd, FTN_SELECTED, (WPARAM)mi.dwItemData, 0);
		}
		return 0;		
	}
private:
	class TabInfo {
	public:
		enum { MAX_LENGTH = 20 };

		TabInfo(HWND aWnd) : hWnd(aWnd), dirty(false), len(0) { };
		HWND hWnd;
		char name[MAX_LENGTH];
		int len;
		SIZE size;
		bool dirty;

		void update(CDC& dc, HFONT boldFont) {
			len = ::GetWindowTextLength(hWnd);
			if(len >= MAX_LENGTH) {
				::GetWindowText(hWnd, name, MAX_LENGTH - 3);
				name[MAX_LENGTH - 4] = '.';
				name[MAX_LENGTH - 3] = '.';
				name[MAX_LENGTH - 2] = '.';
				name[MAX_LENGTH - 1] = 0;
				len = MAX_LENGTH - 1;
			} else {
				::GetWindowText(hWnd, name, MAX_LENGTH);
			}

			if(dirty) {
				HFONT f = dc.SelectFont(boldFont);
				dc.GetTextExtent(name, len, &size);
				dc.SelectFont(f);		
			} else {
				dc.GetTextExtent(name, len, &size);
			}
			
		};

		int getWidth() {
			return size.cx + 18;
		}
		int getFill() {
			return 10;
		}
	};

	HFONT boldFont;
	HWND closing;
	CButton chevron;
	CMenu mnu;
	
	TabInfo* active;
	vector<TabInfo*> tabs;

	/**
	 * Draws a tab
	 * @return The width of the tab
	 */
	int drawTab(CDC& dc, TabInfo* tab, int pos, bool aActive = false) {
		
		CPen black;
		black.CreatePen(PS_SOLID, tab->dirty ? 1 : 1, RGB(0, 0, 0));
		HPEN oldpen = dc.SelectPen(black);
		
		POINT p[4];
		dc.BeginPath();
		dc.MoveTo(pos, 0);
		p[0].x = pos + tab->getWidth() + (getTabHeight()+1)/2;
		p[0].y = 0;
		p[1].x = pos + tab->getWidth();
		p[1].y = getTabHeight();
		p[2].x = pos + (getTabHeight()+1)/2;
		p[2].y = getTabHeight();
		p[3].x = pos;
		p[3].y = 0;
		
		dc.PolylineTo(p, 4);
		dc.CloseFigure();
		dc.EndPath();
		
		HBRUSH oldbrush = dc.SelectBrush(GetSysColorBrush(aActive ? COLOR_WINDOW : COLOR_BTNFACE));
		dc.FillPath();
		
		dc.MoveTo(p[1].x + 1, p[1].y);
		dc.LineTo(p[0].x + 1, p[0].y);
		dc.MoveTo(p[2]);
		dc.LineTo(p[3]);
		
		CPen grey;
		grey.CreatePen(PS_SOLID, tab->dirty ? 1 : 1, RGB(128,128,128));
		dc.SelectPen(grey);
		dc.MoveTo(p[1]);
		dc.LineTo(p[0]);
		dc.MoveTo(p[1]);
		dc.LineTo(p[2]);
		
		dc.SelectPen(oldpen);
		dc.SelectBrush(oldbrush);
		
		dc.SetBkMode(TRANSPARENT);

		if(tab->dirty) {
			HFONT f = dc.SelectFont(boldFont);
			dc.TextOut(pos + (getTabHeight()+1)/2 + tab->getFill()/2, 0, tab->name, tab->len);
			dc.SelectFont(f);		
		} else {
			dc.TextOut(pos + (getTabHeight()+1)/2 + tab->getFill()/2, 0, tab->name, tab->len);
		}
		
		return tab->getWidth();
	};

};

class FlatTabCtrl : public FlatTabCtrlImpl<FlatTabCtrl> {
public:
	DECLARE_FRAME_WND_CLASS_EX(GetWndClassName(), IDR_QUEUE, 0, COLOR_3DFACE);
};

template <class T, class TBase = CMDIWindow, class TWinTraits = CMDIChildWinTraits>
class ATL_NO_VTABLE MDITabChildWindowImpl : public CMDIChildWindowImpl<T, TBase, TWinTraits> {
public:

	MDITabChildWindowImpl() : tab(NULL) { };
	void setTab(FlatTabCtrl* aTab) { tab = aTab; };
	FlatTabCtrl* getTab() { return tab; };

	void setDirty() {
		dcassert(getTab());
		getTab()->setDirty(m_hWnd);
	}
	
 	typedef MDITabChildWindowImpl<T, TBase, TWinTraits> thisClass;
	typedef CMDIChildWindowImpl<T, TBase, TWinTraits> baseClass;
	BEGIN_MSG_MAP(thisClass>)
		MESSAGE_HANDLER(WM_CREATE, onCreate)
		MESSAGE_HANDLER(WM_MDIACTIVATE, onActivate)
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
		::SendMessage(hWndParent, WM_MDIGETACTIVE, 0, (LPARAM)&bMaximized);
		
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

	LRESULT onCreate(UINT /* uMsg */, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
		bHandled = FALSE;
		if(tab)
			tab->addTab(m_hWnd);
		return 0;
	}
	
	LRESULT onActivate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
		bHandled = FALSE;
		if(tab)
			tab->setActive(m_hWnd);
		return 0;
	}
	
	LRESULT onDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
		bHandled = FALSE;
		if(tab)
			tab->removeTab(m_hWnd);
		return 0;
	}

	LRESULT onSetText(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
		bHandled = FALSE;
		if(tab) {
			tab->Invalidate();
		}
		return 0;
	}
	
private:
	FlatTabCtrl* tab;
};

#endif // !defined(AFX_FLATTABCTRL_H__FFFCBD5C_891D_44FB_B9F3_1DF83DA3EA83__INCLUDED_)

/**
 * @file FlatTabCtrl.h
 * $Id: FlatTabCtrl.h,v 1.8 2002/06/03 20:45:38 arnetheduck Exp $
 */

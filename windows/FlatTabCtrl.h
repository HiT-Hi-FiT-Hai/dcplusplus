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

#include "../client/ResourceManager.h"

/** This will be sent when the user presses a tab. WPARAM = HWND */
#define FTN_SELECTED (WM_APP + 700)
/** Set currently active tab to the HWND pointed by WPARAM */
#define FTM_SETACTIVE (WM_APP + 701)

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
		MESSAGE_HANDLER(WM_PAINT, onPaint)
		MESSAGE_HANDLER(WM_WINDOWPOSCHANGING, onWindowPosChanging)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, onLButtonDown)
		MESSAGE_HANDLER(WM_CONTEXTMENU, onContextMenu)
		COMMAND_ID_HANDLER(IDCLOSE, onClose)
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
		
	LRESULT onWindowPosChanging(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) { return 0; };
		
	int getTabHeight() { return 14; };
	int getHeight() { return getTabHeight()+1; };

	LRESULT onPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		int pos = 0;
		int activepos = -1;
		RECT rc;
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

	LRESULT onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };        // location of mouse click 
		int w = 0;

		for(vector<TabInfo*>::iterator i = tabs.begin(); i != tabs.end(); ++i) {
			TabInfo* t = *i;
			if( (pt.x > w) && (pt.x < w + t->getWidth())) {
				closing = t->hWnd;

				CMenu mnu;
				mnu.CreatePopupMenu();
				mnu.AppendMenu(MF_STRING, IDCLOSE, CSTRING(CLOSE));
				mnu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
				break;
			} else {
				w+=t->getWidth();
			}
		}
		return FALSE; 
	}

	LRESULT onClose(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		::SendMessage(closing, WM_CLOSE, 0, 0);
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
	static CWndClassInfo& GetWndClassInfo() { 
		static CWndClassInfo wc = { 
			{ sizeof(WNDCLASSEX), 0, StartWindowProc, 
		  0, 0, NULL, NULL, NULL, (HBRUSH)(COLOR_BTNFACE + 1), NULL, GetWndClassName(), NULL }, 
		  NULL, NULL, IDC_ARROW, TRUE, 0, _T("") }; 
		return wc; 
	}
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
 * $Id: FlatTabCtrl.h,v 1.1 2002/04/09 18:46:32 arnetheduck Exp $
 * @if LOG
 * $Log: FlatTabCtrl.h,v $
 * Revision 1.1  2002/04/09 18:46:32  arnetheduck
 * New files of the major reorganization
 *
 * Revision 1.10  2002/04/07 16:08:14  arnetheduck
 * Fixes and additions
 *
 * Revision 1.9  2002/03/25 22:23:24  arnetheduck
 * Lots of minor updates
 *
 * Revision 1.8  2002/03/04 23:52:31  arnetheduck
 * Updates and bugfixes, new user handling almost finished...
 *
 * Revision 1.7  2002/02/18 23:48:32  arnetheduck
 * New prerelease, bugs fixed and features added...
 *
 * Revision 1.6  2002/02/09 18:13:51  arnetheduck
 * Fixed level 4 warnings and started using new stl
 *
 * Revision 1.5  2002/01/22 00:10:37  arnetheduck
 * Version 0.132, removed extra slots feature for nm dc users...and some bug
 * fixes...
 *
 * Revision 1.4  2002/01/20 22:54:46  arnetheduck
 * Bugfixes to 0.131 mainly...
 *
 * Revision 1.3  2002/01/18 17:41:43  arnetheduck
 * Reworked many right button menus, adding op commands and making more easy to use
 *
 * Revision 1.2  2002/01/05 10:13:39  arnetheduck
 * Automatic version detection and some other updates
 *
 * Revision 1.1  2001/12/27 12:05:00  arnetheduck
 * Added flat tabs, fixed sorting and a StringTokenizer bug
 *
 * @endif
 */
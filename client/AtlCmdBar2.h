// AtlCmdBar2.h
// Copyright (C) 2001 Bjoern Graf
// bjoern.graf@gmx.net
// All rights reserved.
//
// The code and information is provided "as-is" without
// warranty of any kind, either expressed or implied.
//

#ifndef __ATLCMDBAR2_H__
#define __ATLCMDBAR2_H__

#pragma once

#ifndef __cplusplus
	#error ATL requires C++ compilation (use a .cpp suffix)
#endif

#ifndef __ATLAPP_H__
	#error atlcmdbar2.h requires atlapp.h to be included first
#endif

#ifndef __ATLCTRLS_H__
	#error atlcmdbar2.h requires atlctrls.h to be included first
#endif

#if (_WIN32_IE < 0x0400)
	#error atlcmdbar2.h requires _WIN32_IE >= 0x0400
#endif


namespace WTL
{

/////////////////////////////////////////////////////////////////////////////
// CMDIChildWindowImpl2 - Base class for MDI child frame

#define WM_MDICHILDSIZED	(WM_USER + 123)

// Seems that static variables can not be initialized in a template hence we
// need this class...
class CMDIChildWinBaseTraits2
{
public:
	static bool m_bMaximized;
};

__declspec(selectany) bool CMDIChildWinBaseTraits2::m_bMaximized = false;

template <DWORD t_dwStyle, DWORD t_dwExStyle>
class CMDIChildWinImplTraits2 : public CMDIChildWinBaseTraits2
{
public:
	static DWORD GetWndStyle(DWORD dwStyle)
	{
		if(m_bMaximized == true)
			return (dwStyle == 0 ? t_dwStyle : dwStyle) | WS_MAXIMIZE;
		else
			return dwStyle == 0 ? t_dwStyle : dwStyle;
	}
	static DWORD GetWndExStyle(DWORD dwExStyle)
	{
		return dwExStyle == 0 ? t_dwExStyle : dwExStyle;
	}
};

typedef CMDIChildWinImplTraits2<WS_OVERLAPPEDWINDOW | WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, WS_EX_MDICHILD>	CMDIChildWinTraits2;

template <class T, class TBase = CMDIWindow, class TWinTraits = CMDIChildWinTraits2>
class ATL_NO_VTABLE CMDIChildWindowImpl2 : public CMDIChildWindowImpl< T, TBase, TWinTraits >
{
public:

	// Flag to prevent thrashing of maximized state and sys menu
	bool m_fNotifyCmdBar;

	// Overriden to prevent flickering during creation
	HWND Create(HWND hWndParent, _U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
			DWORD dwStyle = 0, DWORD dwExStyle = 0,
			UINT nMenuID = 0, LPVOID lpCreateParam = NULL)
	{
		m_fNotifyCmdBar = false;
		HWND hWndFrame = ::GetParent(hWndParent);
		::SendMessage(hWndFrame, WM_SETREDRAW, (WPARAM)FALSE, 0);
		HWND hWnd = CMDIChildWindowImpl< T, TBase, TWinTraits >::Create(hWndParent, rect, szWindowName,
			dwStyle, dwExStyle, nMenuID, lpCreateParam);
		m_fNotifyCmdBar = true;
		UpdateClientEdge();
		::SendMessage(hWndFrame, WM_SETREDRAW, (WPARAM)TRUE, 0);
		::RedrawWindow(hWndFrame, NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
		return hWnd;
	}
	HWND CreateEx(HWND hWndParent, _U_RECT rect = NULL, LPCTSTR szWindowName = NULL, DWORD dwStyle = 0, DWORD dwExStyle = 0, LPVOID lpCreateParam = NULL)
	{
		m_fNotifyCmdBar = false;
		HWND hWndFrame = ::GetParent(hWndParent);
		::SendMessage(hWndFrame, WM_SETREDRAW, (WPARAM)FALSE, 0);
		HWND hWnd = CMDIChildWindowImpl< T, TBase, TWinTraits >::CreateEx(hWndParent, rect, szWindowName, dwStyle, dwExStyle, lpCreateParam);
		m_fNotifyCmdBar = true;
		UpdateClientEdge();
		::SendMessage(hWndFrame, WM_SETREDRAW, (WPARAM)TRUE, 0);
		::RedrawWindow(hWndFrame, NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
		return hWnd;
	}

// Message map and handlers
	typedef CMDIChildWindowImpl2< T, TBase, TWinTraits >	thisClass;
	typedef CMDIChildWindowImpl< T, TBase, TWinTraits >		baseClass;
	BEGIN_MSG_MAP(thisClass)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_GETICON, OnGetIcon)
		MESSAGE_HANDLER(WM_WINDOWPOSCHANGING, OnWindowPosChanging)
		CHAIN_MSG_MAP(baseClass)
	END_MSG_MAP()

	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		m_fNotifyCmdBar = false;
		bHandled = FALSE;
		return TRUE;
	}

	LRESULT OnGetIcon(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		HICON hIcon;
		UINT nResourceID = T::GetWndClassInfo().m_uCommonResourceID;
		if(wParam == ICON_SMALL)
		{
			hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(),
				MAKEINTRESOURCE(nResourceID), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
		}
		else
		{
			hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(),
				MAKEINTRESOURCE(nResourceID), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR | LR_DEFAULTSIZE);
		}
		return (LRESULT)hIcon;
	}

	LRESULT OnWindowPosChanging(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
	{
		LPWINDOWPOS lpWndPos = (LPWINDOWPOS)lParam;

		if(m_fNotifyCmdBar == true && (lpWndPos->flags & SWP_NOSIZE) == 0)
		{
			// Inform the command bar about the size change
			bool b = (::SendMessage(m_hWndMDIClient, WM_MDICHILDSIZED, 0, (LPARAM)m_hWnd) != 0);
			if( !(lpWndPos->flags & SWP_NOACTIVATE)) {
				TWinTraits::m_bMaximized = b;
			}
		}
		bHandled = FALSE;
		return 1;
	}
};

/////////////////////////////////////////////////////////////////////////////
// CCommandBarCtrl2 - ATL implementation of Command Bars with MDI support

#define IDM_MDI_BASE      (ATL_IDM_FIRST_MDICHILD - 5)
#define IDM_MDI_ICON      (IDM_MDI_BASE + 0)
#define IDM_MDI_GAP       (IDM_MDI_BASE + 1)
#define IDM_MDI_MINIMIZE  (IDM_MDI_BASE + 2)
#define IDM_MDI_RESTORE   (IDM_MDI_BASE + 3)
#define IDM_MDI_CLOSE     (IDM_MDI_BASE + 4)
#define IDM_MDI_FIRST     IDM_MDI_ICON
#define IDM_MDI_LAST      IDM_MDI_CLOSE

template <class T, class TBase = CCommandBarCtrlBase, class TWinTraits = CControlWinTraits>
class ATL_NO_VTABLE CCommandBarCtrlImpl2 : public CCommandBarCtrlImpl< T, TBase, TWinTraits >
{
public:
	DECLARE_WND_SUPERCLASS(NULL, TBase::GetWndClassName())

	bool m_bShowMDIButtons;
	SIZE m_szCaptionIcon;
	SIZE m_szCaptionButton;
	CMenu m_menuDummy;

	CCommandBarCtrlImpl2()
		: CCommandBarCtrlImpl< T, TBase, TWinTraits >()
		, m_bShowMDIButtons(false)
	{
		m_szCaptionIcon.cx = 16;
		m_szCaptionIcon.cy = 16;
		m_szCaptionButton.cx = 16;
		m_szCaptionButton.cy = 14;
	}

	BOOL SetMDIClient(HWND hWndMDIClient)
	{
		GetSystemSettings2();
		return CCommandBarCtrlImpl< T, TBase, TWinTraits >::SetMDIClient(hWndMDIClient);
	}

// Message map and handlers
	typedef CCommandBarCtrlImpl2< T, TBase, TWinTraits >	thisClass;
	typedef CCommandBarCtrlImpl< T, TBase, TWinTraits >		baseClass;
	BEGIN_MSG_MAP(thisClass)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_SETTINGCHANGE, OnSettingChange)
		MESSAGE_HANDLER(WM_INITMENUPOPUP, OnInitMenuPopup)
		MESSAGE_HANDLER(WM_MENUSELECT, OnMenuSelect)
		CHAIN_MSG_MAP(baseClass)
	ALT_MSG_MAP(1)		// Parent window messages
		NOTIFY_CODE_HANDLER(NM_CUSTOMDRAW, OnCustomDraw)
		COMMAND_RANGE_HANDLER(IDM_MDI_FIRST, IDM_MDI_LAST, OnMDICommand)
		CHAIN_MSG_MAP_ALT(baseClass,1)
	ALT_MSG_MAP(2)		// MDI client window messages
		MESSAGE_HANDLER(WM_MDISETMENU, OnMDISetMenu)
		MESSAGE_HANDLER(WM_MDICHILDSIZED, OnMDIChildSized)
		CHAIN_MSG_MAP_ALT(baseClass,2)
	ALT_MSG_MAP(3)		// Message hook messages
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnHookMouseMove)
		CHAIN_MSG_MAP_ALT(baseClass,3)
	END_MSG_MAP()

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		// get and use system settings
		GetSystemSettings2();

		bHandled = FALSE;
		return 0;
	}

	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		::SendMessage(m_hWnd, WM_SETREDRAW, (WPARAM)FALSE, 0);
		int xGapPos = UpdateGapWidth();
		::SendMessage(m_hWnd, WM_SETREDRAW, (WPARAM)TRUE, 0);
		if(xGapPos > 0)
		{
			// Prevent flicker
			// Without this line the menu items flicker slightly
			::RedrawWindow(m_hWnd, NULL, NULL, RDW_VALIDATE);

			RECT rc;
			GetClientRect(&rc);
			rc.left = xGapPos;
			::RedrawWindow(m_hWnd, &rc, NULL, RDW_NOFRAME | RDW_ERASE | RDW_NOINTERNALPAINT | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
		}
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnSettingChange(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		GetSystemSettings2();
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnInitMenuPopup(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if((BOOL)HIWORD(lParam))	// System menu, do nothing
		{
			bHandled = FALSE;
			return 1;
		}

		if((HMENU)wParam != m_menuDummy.m_hMenu)		// Not ours, do nothing
		{
			bHandled = FALSE;
			return 1;
		}

		ATLTRACE2(atlTraceUI, 0, "CmdBar2 - OnInitMenuPopup %d\n", m_menuDummy.GetMenuItemCount());

		// forward to the parent or subclassed window, so it can handle update UI
		LRESULT lRet = 0;
		if(m_bAttachedMenu)
			lRet = DefWindowProc(uMsg, wParam, (lParam || m_bContextMenu) ? lParam : GetHotItem());
		else
			lRet = m_wndParent.DefWindowProc(uMsg, wParam, (lParam || m_bContextMenu) ? lParam : GetHotItem());

		// Copy the system menu to our dummy menu
		HWND hWndActiveMDI = (HWND)m_wndMDIClient.SendMessage(WM_MDIGETACTIVE);
		CMenuHandle sysMenu = ::GetSystemMenu(hWndActiveMDI, FALSE);
		ATLASSERT(sysMenu.m_hMenu != NULL);

		T* pT = static_cast<T*>(this);
		pT;	// avoid level 4 warning
		TCHAR szString[pT->_nMaxMenuItemTextLength];
		BOOL bRet;
		for(int i = 0; i < sysMenu.GetMenuItemCount(); i++)
		{
			CMenuItemInfo mii;
			mii.cch = pT->_nMaxMenuItemTextLength;
			mii.fMask = MIIM_CHECKMARKS | MIIM_DATA | MIIM_ID | MIIM_STATE | MIIM_SUBMENU | MIIM_TYPE;
			mii.fType = MFT_STRING;
			mii.dwTypeData = szString;
			bRet = sysMenu.GetMenuItemInfo(i, TRUE, &mii);
			ATLASSERT(bRet);

			bRet = m_menuDummy.InsertMenuItem(i, TRUE, &mii);
			ATLASSERT(bRet);
		}

//		::GetSystemMenu(hWndActiveMDI, TRUE);

		return lRet;
	}

	LRESULT OnMenuSelect(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if((HMENU)wParam == m_menuDummy.m_hMenu)  // Dummy menu selected, do nothing.
		{
			bHandled = FALSE;
			return 1;
		}

		// Check if a menu is closing, clear the dummy menu
		if(HIWORD(wParam) == 0xFFFF && lParam == NULL &&	// Menu closing
			m_menuDummy.m_hMenu != NULL && m_menuDummy.GetMenuItemCount() > 0)
		{
#ifdef _CMDBAR_EXTRA_TRACE
			ATLTRACE2(atlTraceUI, 0, "CmdBar2 - OnMenuSelect - CLOSING!!!!\n");
#endif
			while(m_menuDummy.GetMenuItemCount() > 0)
			{
				BOOL bRet = m_menuDummy.RemoveMenu(0, MF_BYPOSITION);
				ATLASSERT(bRet);
				bRet;
			}
		}

		bHandled = FALSE;
		return 1;
	}

	LRESULT OnCustomDraw(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled)
	{
		NMCUSTOMDRAW* pCustomDraw = (NMCUSTOMDRAW*)pnmh;

		if(pCustomDraw->dwDrawStage == CDDS_PREPAINT)
			// Request prepaint notifications for each item.
			return CDRF_NOTIFYITEMDRAW;

		else if(pCustomDraw->dwDrawStage == CDDS_ITEMPREPAINT)
		{
			if(pCustomDraw->dwItemSpec >= IDM_MDI_FIRST && pCustomDraw->dwItemSpec <= IDM_MDI_LAST)
			{
				CDCHandle dc(pCustomDraw->hdc);

				int uState = -1;
				if(pCustomDraw->dwItemSpec == IDM_MDI_ICON)
				{
					HWND hWndActiveMDI = (HWND)m_wndMDIClient.SendMessage(WM_MDIGETACTIVE);
					HICON hIcon = (HICON)::SendMessage(hWndActiveMDI, WM_GETICON, ICON_SMALL, 0);
					POINT pos = { pCustomDraw->rc.left, pCustomDraw->rc.top };
					pos.y += (pCustomDraw->rc.bottom - pCustomDraw->rc.top - m_szCaptionIcon.cy) / 2;
					dc.DrawIconEx(pos, hIcon, m_szCaptionIcon);
					::DestroyIcon(hIcon);
				}
				else if(pCustomDraw->dwItemSpec == IDM_MDI_GAP)
				{
				}
				else if(pCustomDraw->dwItemSpec == IDM_MDI_MINIMIZE)
				{
					uState = DFCS_CAPTIONMIN;
				}
				else if(pCustomDraw->dwItemSpec == IDM_MDI_RESTORE)
				{
					uState = DFCS_CAPTIONRESTORE;
				}
				else if(pCustomDraw->dwItemSpec == IDM_MDI_CLOSE)
				{
					uState = DFCS_CAPTIONCLOSE;
				}

				RECT rc = pCustomDraw->rc;
				rc.top += (pCustomDraw->rc.bottom - pCustomDraw->rc.top - m_szCaptionButton.cy) / 2 + 1;
				rc.bottom = rc.top + m_szCaptionButton.cy;
				rc.right = rc.left + m_szCaptionButton.cx;
				if(uState != -1)
					dc.DrawFrameControl(&rc, DFC_CAPTION, uState |
						((pCustomDraw->uItemState & CDIS_SELECTED) != 0 ? DFCS_PUSHED : 0));

				return CDRF_SKIPDEFAULT;
			}
			else
				return CDRF_DODEFAULT;
		}

		bHandled = FALSE;
		return CDRF_DODEFAULT;
	}

	LRESULT OnMDICommand(WORD wNotifyCode, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled)
	{
		if(wNotifyCode != BN_CLICKED)
		{
			bHandled = FALSE;
			return 0;
		}

		HWND hWndActiveMDI = (HWND)m_wndMDIClient.SendMessage(WM_MDIGETACTIVE);
		switch(wID)
		{
		case IDM_MDI_ICON:
		case IDM_MDI_GAP:
			// Nothing to do
			break;
		case IDM_MDI_MINIMIZE:
			::SendMessage(hWndActiveMDI, WM_SYSCOMMAND, SC_MINIMIZE, 0);
			break;
		case IDM_MDI_RESTORE:
			::SendMessage(hWndActiveMDI, WM_SYSCOMMAND, SC_RESTORE, 0);
			break;
		case IDM_MDI_CLOSE:
			::SendMessage(hWndActiveMDI, WM_SYSCOMMAND, SC_CLOSE, 0);
			break;
		default:
			ATLASSERT(FALSE);
			break;
		}
		return 0;
	}

// MDI client window message handlers
	LRESULT OnMDISetMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		BOOL fShow = false;
		m_wndMDIClient.SendMessage(WM_MDIGETACTIVE, 0, (LPARAM)&fShow);

		int xGapPos = UpdateMenu(fShow == TRUE, (HMENU)wParam);
		LRESULT lRes = CCommandBarCtrlImpl< T, TBase, TWinTraits >::OnMDISetMenu(uMsg, wParam, lParam, bHandled);
		UpdateMDIButtons(fShow == TRUE);

#if (_WIN32_IE >= 0x0400)
		// Update rebar band width
		if(xGapPos == 0)
		{
			int nIndex = GetButtonCount();
			RECT rc;
			GetItemRect(nIndex - 1, &rc);
			xGapPos = rc.right;
		}
		REBARBANDINFO rbBand;
		rbBand.cbSize = sizeof(REBARBANDINFO);
		rbBand.fMask = RBBIM_SIZE | RBBIM_IDEALSIZE;
		rbBand.cx = xGapPos;
		rbBand.cxIdeal = rbBand.cx;
		lRes = ::SendMessage(GetParent(), RB_SETBANDINFO, (WPARAM)0, (LPARAM)&rbBand);
		if(lRes == 0)
		{
			ATLTRACE2(atlTraceUI, 0, _T("Failed to modify the menu band size.\n"));
			return FALSE;
		}
#endif //(_WIN32_IE >= 0x0400)
		return lRes;
	}

	LRESULT OnMDIChildSized(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
	{
		MDIChildChanged((HWND)lParam);
		return m_bShowMDIButtons;
	}

// Message hook handlers
	LRESULT OnHookMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		// Fix to deselect the file menu if the MDI child is maximized
		if(m_bMenuActive && m_bShowMDIButtons)
		{
			DWORD dwPoint = ::GetMessagePos();
			POINT point = { GET_X_LPARAM(dwPoint), GET_Y_LPARAM(dwPoint) };

			if(::WindowFromPoint(point) == m_hWnd)
			{
				ScreenToClient(&point);
				int nHit = HitTest(&point);

				if(nHit == 0 && nHit != m_nPopBtn && m_nPopBtn != -1)
				{
					TBBUTTON tbb;
					GetButton(nHit, &tbb);
					if(tbb.idCommand == IDM_MDI_ICON)
					{
						SetHotItem(-1);
					}
				}
			}
		}
		bHandled = FALSE;
		return 0;
	}

// Implementation
	void GetSystemSettings2()
	{
		// refresh our caption button widths
		NONCLIENTMETRICS info;
		info.cbSize = sizeof(info);
		::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(info), &info, 0);

		m_szCaptionIcon.cx = info.iCaptionWidth - ::GetSystemMetrics(SM_CXEDGE);
		m_szCaptionIcon.cy = info.iCaptionHeight - ::GetSystemMetrics(SM_CYEDGE);
		m_szCaptionButton.cx = info.iCaptionWidth - ::GetSystemMetrics(SM_CXEDGE);
		m_szCaptionButton.cy = info.iCaptionHeight - 2  *::GetSystemMetrics(SM_CYEDGE);
	}

	int GetGapWidth()
	{
		// Calc width of gap between last menu item and mdi buttons
		TBBUTTON btn;
		memset(&btn, 0, sizeof(btn));

		RECT rc;
		int nItems = ::GetMenuItemCount(m_hMenu);
		BOOL bRet = GetItemRect(nItems - 1, &rc);
		bRet;
		ATLASSERT(bRet);
		int width = rc.right;
		GetWindowRect(&rc);
		width = rc.right - rc.left - width;
		width -= 3 * m_szCaptionButton.cx + ::GetSystemMetrics(SM_CXEDGE);

		return width;
	}

	// Returns the x position of the gap
	int UpdateGapWidth()
	{
		if(m_bShowMDIButtons == false)
			return 0;

		TBBUTTON btn;
		memset(&btn, 0, sizeof(btn));

		int nItems = GetButtonCount();
		BOOL bRet = GetButton(nItems - 4, &btn);
		ATLASSERT(bRet);

		int nGapWidth = GetGapWidth();
		if(btn.idCommand == IDM_MDI_GAP)
		{
			if(nGapWidth < 0)
			{
				// Toolbar buttons are outside the visible area, remove them
				// to prevent chevron problems
				UpdateMDIButtons(false, false);
				nGapWidth = 0;
			}
			else
			{
				// Update the gap width
				TBBUTTONINFO bi;
				memset(&bi, 0, sizeof(bi));
				bi.cbSize = sizeof(TBBUTTONINFO);
				bi.dwMask = TBIF_SIZE;
				bi.cx = (WORD)nGapWidth;
				bRet = SetButtonInfo(IDM_MDI_GAP, &bi);
				ATLASSERT(bRet);

				RECT rc;
				bRet = GetItemRect(nItems - 4, &rc);
				nGapWidth = rc.left;
				ATLASSERT(bRet);
			}
		}
		else
		{
			if(nGapWidth > 0)
			{
				// Toolbar buttons are visible, insert them
				UpdateMDIButtons(true, false);

				RECT rc;
				bRet = GetItemRect(nItems - 4, &rc);
				nGapWidth = rc.left;
				ATLASSERT(bRet);
			}
			else
			{
				nGapWidth = 0;
			}
		}

		return nGapWidth;
	}

	bool UpdateMenu(bool fShow, HMENU hMenu)
	{
		if(fShow == true)
		{
			m_bShowMDIButtons = true;

			MENUITEMINFO mii;
			TCHAR szString[32];
			memset(&mii, 0, sizeof(mii));
			mii.cbSize = sizeof(mii);
			mii.fMask = MIIM_TYPE;
			mii.fType = MFT_STRING;
			mii.dwTypeData = szString;
			mii.cch = 32;

			BOOL bRet = ::GetMenuItemInfo(hMenu, 0, TRUE, &mii);
			ATLASSERT(bRet);
			if(mii.dwTypeData != NULL && lstrcmp(_T("*"), mii.dwTypeData) == 0)
				return false;

			// Set the dummy menu as placeholder for the system menu
			if(m_menuDummy.m_hMenu == NULL)
				m_menuDummy.CreatePopupMenu();

			memset(&mii, 0, sizeof(mii));
			mii.cbSize = sizeof(mii);
			mii.fMask = MIIM_STATE | MIIM_SUBMENU | MIIM_TYPE;
			mii.fType = MF_STRING;
			mii.fState = MFS_ENABLED;
			mii.hSubMenu = m_menuDummy;
			mii.dwTypeData = _T("*");
			mii.cch = 1;
			bRet = ::InsertMenuItem(hMenu, 0, MF_BYPOSITION, &mii);
			ATLASSERT(bRet);
		}
		else
		{
			MENUITEMINFO mii;
			TCHAR szString[32];

			mii.cbSize = sizeof(mii);
			mii.fMask = MIIM_TYPE;
			mii.fType = MFT_STRING;
			mii.dwTypeData = szString;
			mii.cch = 32;

			BOOL bRet = ::GetMenuItemInfo(hMenu, 0, TRUE, &mii);
			ATLASSERT(bRet);
			if(mii.dwTypeData == NULL || lstrcmp(_T("*"), mii.dwTypeData) != 0)
				return false;

			bRet = ::RemoveMenu(hMenu, 0, MF_BYPOSITION);
			ATLASSERT(bRet);

			if(m_menuDummy.m_hMenu != NULL)
				m_menuDummy.DestroyMenu();
		}
		return true;
	}

	void UpdateMDIButtons(bool fShow, bool fAll = true)
	{
		if(fShow == true)
		{
			TBBUTTON btn;
			TBBUTTONINFO bi;
			BOOL bRet;

			if(fAll == true)
			{
				m_bShowMDIButtons = true;

				memset(&btn, 0, sizeof(btn));

				int nItems = GetButtonCount();
				bRet = GetButton(nItems - 1, &btn);
				ATLASSERT(bRet);
				if(btn.idCommand == IDM_MDI_CLOSE)
					return;

				bRet = GetButton(0, &btn);
				ATLASSERT(bRet);

				memset(&bi, 0, sizeof(bi));
				bi.cbSize = sizeof(TBBUTTONINFO);
				bi.dwMask = TBIF_COMMAND | TBIF_SIZE | TBIF_STYLE;
				bi.fsStyle = TBSTYLE_BUTTON | TBSTYLE_DROPDOWN;
				bi.idCommand = IDM_MDI_ICON;
				bi.cx = (WORD)m_szCaptionIcon.cx;

				bRet = SetButtonInfo(btn.idCommand, &bi);
				ATLASSERT(bRet);
			}

			//
			memset(&btn, 0, sizeof(btn));
			btn.idCommand = IDM_MDI_GAP;
			btn.fsStyle = TBSTYLE_BUTTON;

			bRet = InsertButton(-1, &btn);
			ATLASSERT(bRet);

			memset(&bi, 0, sizeof(bi));
			bi.cbSize = sizeof(TBBUTTONINFO);
			bi.dwMask = TBIF_SIZE;
			bi.cx = (WORD)GetGapWidth();

			bRet = SetButtonInfo(IDM_MDI_GAP, &bi);
			ATLASSERT(bRet);

			for(int i = 0; i < 3; i++)
			{
				memset(&btn, 0, sizeof(btn));
				btn.idCommand = IDM_MDI_MINIMIZE + i;
				btn.fsState = TBSTATE_ENABLED;
				btn.fsStyle = TBSTYLE_BUTTON;

				bRet = InsertButton(-1, &btn);
				ATLASSERT(bRet);

				memset(&bi, 0, sizeof(bi));
				bi.cbSize = sizeof(TBBUTTONINFO);
				bi.dwMask = TBIF_SIZE;
				bi.cx = (WORD)m_szCaptionButton.cx;
				if(i == 1)
					bi.cx = (WORD)(bi.cx + ::GetSystemMetrics(SM_CXEDGE));

				bRet = SetButtonInfo(IDM_MDI_MINIMIZE + i, &bi);
				ATLASSERT(bRet);
			}
		}
		else
		{
			if(fAll == true)
				m_bShowMDIButtons = false;

			TBBUTTON btn;
			memset(&btn, 0, sizeof(btn));

			int nItems = GetButtonCount();
			BOOL bRet = GetButton(nItems - 1, &btn);
			bRet;
			ATLASSERT(bRet);
			if(btn.idCommand != IDM_MDI_CLOSE)
				return;

			SetRedraw(FALSE);

			for(int i = nItems; i > nItems - 4; i--)
			{
				BOOL bRet = DeleteButton(i - 1);
				bRet;
				ATLASSERT(bRet);
			}

			SetRedraw(TRUE);
		}
	}

	void MDIChildChanged(HWND hWndMDIChild)
	{
		DWORD dwStyle = ::GetWindowLong(hWndMDIChild, GWL_STYLE);
		bool fShow = ((dwStyle & WS_MAXIMIZE) != 0);

		if(UpdateMenu(fShow, m_hMenu) == true)
		{
			AttachMenu(m_hMenu);
			UpdateMDIButtons(fShow);
		}
	}
};

class CCommandBarCtrl2 : public CCommandBarCtrlImpl2<CCommandBarCtrl2>
{
public:
	DECLARE_WND_SUPERCLASS(_T("WTL_CommandBar2"), GetWndClassName())
};

}; //namespace WTL

#endif // __ATLCMDBAR2_H__

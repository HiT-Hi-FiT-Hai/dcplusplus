/*
 * Copyright (C) 2001-2005 Jacek Sieka, arnetheduck on gmail point com
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

#if !defined(SYSTEM_FRAME_H)
#define SYSTEM_FRAME_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FlatTabCtrl.h"
#include "../client/Text.h"

#include "../client/LogManager.h"

#define SYSTEM_LOG_MESSAGE_MAP 42

class SystemFrame : public MDITabChildWindowImpl<SystemFrame>, public StaticFrame<SystemFrame, ResourceManager::SYSTEM_LOG>,
	private LogManagerListener
{
public:
	DECLARE_FRAME_WND_CLASS_EX(_T("SystemFrame"), IDR_NOTEPAD, 0, COLOR_3DFACE);

	SystemFrame() : ctrlClientContainer(_T("edit"), this, SYSTEM_LOG_MESSAGE_MAP) { }
	virtual ~SystemFrame() { }
	
	typedef MDITabChildWindowImpl<SystemFrame> baseClass;
	BEGIN_MSG_MAP(SystemFrame)
		MESSAGE_HANDLER(WM_SETFOCUS, OnFocus)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_CLOSE, onClose)
		MESSAGE_HANDLER(WM_CTLCOLOREDIT, onCtlColor)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, onCtlColor)
		MESSAGE_HANDLER(WM_SPEAKER, onSpeaker)
		CHAIN_MSG_MAP(baseClass)
	ALT_MSG_MAP(SYSTEM_LOG_MESSAGE_MAP)
		MESSAGE_HANDLER(WM_LBUTTONDBLCLK, onLButton)
	END_MSG_MAP()

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onSpeaker(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onLButton(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	void UpdateLayout(BOOL bResizeBars = TRUE);
	
	LRESULT onCtlColor(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		HWND hWnd = (HWND)lParam;
		HDC hDC = (HDC)wParam;
		if(hWnd == ctrlPad.m_hWnd) {
			::SetBkColor(hDC, WinUtil::bgColor);
			::SetTextColor(hDC, WinUtil::textColor);
			return (LRESULT)WinUtil::bgBrush;
		}
		bHandled = FALSE;
		return FALSE;
	};
	
	
	LRESULT OnFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		ctrlPad.SetFocus();
		return 0;
	}
	
private:
	CEdit ctrlPad;
	CContainedWindow ctrlClientContainer;

	void addLine(time_t t, const tstring& msg);

	virtual void on(Message, const string& message) { PostMessage(WM_SPEAKER, (WPARAM)(new tstring(Text::toT(message)))); }
};

#endif // !defined(SYSTEM_FRAME_H)

/**
 * @file
 * $Id: SystemFrame.h,v 1.3 2005/12/26 17:16:03 arnetheduck Exp $
 */

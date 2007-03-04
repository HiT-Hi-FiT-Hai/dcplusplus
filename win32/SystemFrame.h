/*
 * Copyright (C) 2001-2007 Jacek Sieka, arnetheduck on gmail point com
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

#ifndef DCPLUSPLUS_WIN32_SYSTEM_FRAME_H
#define DCPLUSPLUS_WIN32_SYSTEM_FRAME_H

#include "StaticFrame.h"

#include <client/LogManager.h>
#include <client/ResourceManager.h>

class SystemFrame : public StaticFrame<SystemFrame>,
	private LogManagerListener
{
public:
	enum { TITLE_RESOURCE = ResourceManager::SYSTEM_LOG };
	
	private:
	WidgetTextBoxPtr log;

	SystemFrame(WidgetMDIParentPtr parent);

	void addLine(time_t t, const tstring& msg);
};

#ifdef PORT_ME
#include "FlatTabCtrl.h"
#include "../client/Text.h"


#define SYSTEM_LOG_MESSAGE_MAP 42

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
	}

	LRESULT OnFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		ctrlPad.SetFocus();
		return 0;
	}

private:
	CContainedWindow ctrlClientContainer;

	virtual void on(Message, time_t t, const string& message) { PostMessage(WM_SPEAKER, (WPARAM)(new pair<time_t, tstring>(t, Text::toT(message)))); }
};

#endif
#endif

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

#if !defined(AFX_PRIVATEFRAME_H__8F6D05EC_ADCF_4987_8881_6DF3C0E355FA__INCLUDED_)
#define AFX_PRIVATEFRAME_H__8F6D05EC_ADCF_4987_8881_6DF3C0E355FA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "User.h"
#include "CriticalSection.h"

#include "FlatTabCtrl.h"

#define PM_MESSAGE_MAP 8		// This could be any number, really...

class PrivateFrame : public MDITabChildWindowImpl<PrivateFrame>
{
public:

	DECLARE_FRAME_WND_CLASS("PrivateFrame", IDR_PRIVATE);

	virtual void OnFinalMessage(HWND /*hWnd*/) {
		delete this;
	}

	BEGIN_MSG_MAP(PrivateFrame)
		MESSAGE_HANDLER(WM_SETFOCUS, OnFocus)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_FORWARDMSG, OnForwardMsg)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_CTLCOLOREDIT, onCtlColor)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, onCtlColor)
		MESSAGE_HANDLER(WM_CLOSE, onClose)
		CHAIN_MSG_MAP(MDITabChildWindowImpl<PrivateFrame>)
	ALT_MSG_MAP(PM_MESSAGE_MAP)
		MESSAGE_HANDLER(WM_CHAR, onChar)
		MESSAGE_HANDLER(WM_KEYDOWN, onChar)
		MESSAGE_HANDLER(WM_KEYUP, onChar)
	END_MSG_MAP()

	LRESULT onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
		Lock l(cs);
		frames.erase(user);
		bHandled = FALSE;
		return FALSE;
	}
	
	LRESULT onCtlColor(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		HWND hWnd = (HWND)lParam;
		HDC hDC = (HDC)wParam;
		if(hWnd == ctrlClient.m_hWnd || hWnd == ctrlMessage.m_hWnd) {
			::SetBkColor(hDC, Util::bgColor);
			::SetTextColor(hDC, Util::textColor);
			return (LRESULT)Util::bgBrush;
		}
		bHandled = FALSE;
		return FALSE;
	};

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
		ctrlClient.MoveWindow(rc);
		
		rc = rect;
		rc.bottom -= 2;
		rc.top = rc.bottom - 22;
		rc.left +=2;
		rc.right -=2;
		ctrlMessage.MoveWindow(rc);

	}
	
	LRESULT OnFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		ctrlMessage.SetFocus();
		return 0;
	}
	
	LRESULT OnForwardMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
	{
		LPMSG pMsg = (LPMSG)lParam;
		
		return MDITabChildWindowImpl<PrivateFrame>::PreTranslateMessage(pMsg);
	}
	
	LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		return 0;
	}
		
	void addLine(const string& aLine);
	void addClientLine(const string& aLine) {
		if(!created) {
			CreateEx(parent);
		}
		ctrlStatus.SetText(0, ("[" + Util::getShortTimeString() + "] " + aLine).c_str());
		setDirty();
	}
	
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(&ps);
		FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_ACTIVEBORDER+1));
		EndPaint(&ps);
		return 0;
	}
	LRESULT onChar(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled) {
		switch(wParam) {
		case VK_RETURN:
			if( (GetKeyState(VK_SHIFT) & 0x8000) || 
				(GetKeyState(VK_CONTROL) & 0x8000) || 
				(GetKeyState(VK_MENU) & 0x8000) ) {
				bHandled = FALSE;
			} else {
				if(uMsg == WM_KEYDOWN) {
					onEnter();
				}
			}
			break;
		default:
			bHandled = FALSE;
		}
		return 0;
	}

	void onEnter();
	
	void setUser(const User::Ptr& aUser) { user = aUser; };
	void sendMessage(const string& msg) {
		if(user && user->isOnline()) {
			string s = "<" + user->getClientNick() + "> " + msg;
			user->privateMessage(s);
			addLine(Util::validateMessage(s));
		}
	}
	static void gotMessage(const User::Ptr& aUser, const string& aMessage, HWND aParent, FlatTabCtrl* aTab);
	static void openWindow(const User::Ptr& aUser, HWND aParent, FlatTabCtrl* aTab);
	
	User::Ptr& getUser() { return user; };
private:
	PrivateFrame(const User::Ptr& aUser, HWND aParent = NULL) : user(aUser), parent(aParent), created(false), ctrlMessageContainer("edit", this, PM_MESSAGE_MAP) {
	}
	
	~PrivateFrame() {
	}
	
	HWND parent;
	bool created;
	typedef map<User::Ptr, PrivateFrame*> FrameMap;
	typedef FrameMap::iterator FrameIter;
	static FrameMap frames;
	CEdit ctrlClient;
	CEdit ctrlMessage;
	CStatusBarCtrl ctrlStatus;
	static CriticalSection cs;

	User::Ptr user;
	CContainedWindow ctrlMessageContainer;
	
};

#endif // !defined(AFX_PRIVATEFRAME_H__8F6D05EC_ADCF_4987_8881_6DF3C0E355FA__INCLUDED_)

/**
 * @file PrivateFrame.h
 * $Id: PrivateFrame.h,v 1.18 2002/04/03 23:20:35 arnetheduck Exp $
 * @if LOG
 * $Log: PrivateFrame.h,v $
 * Revision 1.18  2002/04/03 23:20:35  arnetheduck
 * ...
 *
 * Revision 1.17  2002/03/10 22:41:08  arnetheduck
 * Working on internationalization...
 *
 * Revision 1.16  2002/03/07 19:07:52  arnetheduck
 * Minor fixes + started code review
 *
 * Revision 1.15  2002/03/04 23:52:31  arnetheduck
 * Updates and bugfixes, new user handling almost finished...
 *
 * Revision 1.14  2002/02/25 15:39:29  arnetheduck
 * Release 0.154, lot of things fixed...
 *
 * Revision 1.13  2002/02/18 23:48:32  arnetheduck
 * New prerelease, bugs fixed and features added...
 *
 * Revision 1.12  2002/02/09 18:13:51  arnetheduck
 * Fixed level 4 warnings and started using new stl
 *
 * Revision 1.11  2002/02/07 17:25:28  arnetheduck
 * many bugs fixed, time for 0.152 I think
 *
 * Revision 1.10  2002/02/04 01:10:30  arnetheduck
 * Release 0.151...a lot of things fixed
 *
 * Revision 1.9  2002/01/26 21:09:51  arnetheduck
 * Release 0.14
 *
 * Revision 1.8  2002/01/22 00:10:37  arnetheduck
 * Version 0.132, removed extra slots feature for nm dc users...and some bug
 * fixes...
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


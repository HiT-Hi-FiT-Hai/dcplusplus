/* 
 * Copyright (C) 2001 Jacek Sieka, jacek@creatio.se
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

#include "Client.h"
#include "User.h"
#include "CriticalSection.h"
#include "ClientManager.h"

#include "AtlCmdBar2.h"

#define PM_MESSAGE_MAP 8		// This could be any number, really...

class PrivateFrame : public CMDIChildWindowImpl2<PrivateFrame>
{
public:

	DECLARE_FRAME_WND_CLASS("PrivateFrame", IDR_MDICHILD);

	virtual void OnFinalMessage(HWND /*hWnd*/) {
		cs.enter();
		frames.erase(user);
		cs.leave();
		delete this;
	}

	BEGIN_MSG_MAP(PrivateFrame)
		MESSAGE_HANDLER(WM_SETFOCUS, OnFocus)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_FORWARDMSG, OnForwardMsg)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		CHAIN_MSG_MAP(CMDIChildWindowImpl2<PrivateFrame>)
	ALT_MSG_MAP(PM_MESSAGE_MAP)
		MESSAGE_HANDLER(WM_CHAR, OnChar)
	END_MSG_MAP()

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
	
	LRESULT OnFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
		ctrlMessage.SetFocus();
		return 0;
	}
	
	LRESULT OnForwardMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
	{
		LPMSG pMsg = (LPMSG)lParam;
		
		return CMDIChildWindowImpl2<PrivateFrame>::PreTranslateMessage(pMsg);
	}
	
	LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
		return 0;
	}
		
	void addLine(const string& aLine) {
		if(!created) {
			CreateEx(parent);
		}
		ctrlClient.AppendText(aLine.c_str());
		ctrlClient.AppendText("\r\n");
	}

	void addClientLine(const char* aLine) {
		if(!created) {
			CreateEx(parent);
		}
		ctrlStatus.SetText(0, aLine);
	}
	void addClientLine(const string& aLine) {
		addClientLine(aLine.c_str());
	}

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(&ps);
		FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_ACTIVEBORDER+1));
		EndPaint(&ps);
		return 0;
	}
	LRESULT OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if(user->isOnline()) {
			char* message;
			
			if(wParam == VK_RETURN && ctrlMessage.GetWindowTextLength() > 0) {
				message = new char[ctrlMessage.GetWindowTextLength()+1];
				ctrlMessage.GetWindowText(message, ctrlMessage.GetWindowTextLength()+1);
				string s = "<" + Settings::getNick() + "> " + string(message, ctrlMessage.GetWindowTextLength());
				delete message;
				user->getClient()->privateMessage(user, s);
				ctrlMessage.SetWindowText("");
				addLine(s);
			} else {
				bHandled = FALSE;
			}
		} else {
			ctrlStatus.SetText(0, "User went offline");
			bHandled = FALSE;
		}
		return 0;
	}

	static PrivateFrame* getFrame(User::Ptr& aUser, HWND aParent = NULL);
	
private:
	PrivateFrame(User::Ptr& aUser, HWND aParent = NULL) : user(aUser), parent(aParent), created(false), ctrlMessageContainer("edit", this, PM_MESSAGE_MAP) {
	}
	
	~PrivateFrame() {
	}
	
	HWND parent;
	bool created;
	static map<User::Ptr, PrivateFrame*> frames;
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
 * $Id: PrivateFrame.h,v 1.1 2001/12/21 20:21:17 arnetheduck Exp $
 * @if LOG
 * $Log: PrivateFrame.h,v $
 * Revision 1.1  2001/12/21 20:21:17  arnetheduck
 * Private messaging added, and a lot of other updates as well...
 *
 * @endif
 */


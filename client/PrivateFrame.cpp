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

#include "stdafx.h"
#include "DCPlusPlus.h"

#include "PrivateFrame.h"
#include "Client.h"

CriticalSection PrivateFrame::cs;
map<User::Ptr, PrivateFrame*> PrivateFrame::frames;

LRESULT PrivateFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	
	CreateSimpleStatusBar(ATL_IDS_IDLEMESSAGE, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SBARS_SIZEGRIP);
	ctrlStatus.Attach(m_hWndStatusBar);
	
	ctrlClient.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		WS_VSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_NOHIDESEL | ES_READONLY, WS_EX_CLIENTEDGE);
	
	ctrlClient.FmtLines(TRUE);
	ctrlClient.LimitText(0);
	ctrlMessage.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		ES_AUTOHSCROLL, WS_EX_CLIENTEDGE);
	
	ctrlMessageContainer.SubclassWindow(ctrlMessage.m_hWnd);
	
	ctrlClient.SetFont((HFONT)::GetStockObject(DEFAULT_GUI_FONT));
	ctrlMessage.SetFont((HFONT)::GetStockObject(DEFAULT_GUI_FONT));
	if(user->isOnline()) {
		SetWindowText((user->getNick() + " (" + user->getClient()->getName() + ")").c_str());
	} else {
		SetWindowText((user->getNick() + " (Offline)").c_str());
	}
	
	created = true;

	bHandled = FALSE;
	return 1;
}


PrivateFrame* PrivateFrame::getFrame(const User::Ptr& aUser, HWND aParent) {
	PrivateFrame* p;
	cs.enter();
	map<User::Ptr, PrivateFrame*>::iterator i = frames.find(aUser);
	if(i == frames.end()) {
		p = new PrivateFrame(aUser, aParent);
		frames[aUser] = p;
	} else {
		p = i->second;
	}
	cs.leave();
	return p;
}

LRESULT PrivateFrame::OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
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

/**
 * @file PrivateFrame.cpp
 * $Id: PrivateFrame.cpp,v 1.3 2002/01/11 14:52:57 arnetheduck Exp $
 * @if LOG
 * $Log: PrivateFrame.cpp,v $
 * Revision 1.3  2002/01/11 14:52:57  arnetheduck
 * Huge changes in the listener code, replaced most of it with templates,
 * also moved the getinstance stuff for the managers to a template
 *
 * Revision 1.2  2002/01/05 10:13:40  arnetheduck
 * Automatic version detection and some other updates
 *
 * Revision 1.1  2001/12/21 20:21:17  arnetheduck
 * Private messaging added, and a lot of other updates as well...
 *
 * @endif
 */



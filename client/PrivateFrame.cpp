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

#include "stdafx.h"
#include "DCPlusPlus.h"

#include "PrivateFrame.h"
#include "Client.h"
#include "ClientManager.h"
#include "Util.h"

CriticalSection PrivateFrame::cs;
PrivateFrame::FrameMap PrivateFrame::frames;

LRESULT PrivateFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	
	CreateSimpleStatusBar(ATL_IDS_IDLEMESSAGE, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SBARS_SIZEGRIP);
	ctrlStatus.Attach(m_hWndStatusBar);
	
	ctrlClient.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		WS_VSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_NOHIDESEL | ES_READONLY, WS_EX_CLIENTEDGE);
	
	ctrlClient.FmtLines(TRUE);
	ctrlClient.LimitText(0);
	ctrlClient.SetFont(Util::font);
	ctrlMessage.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		ES_AUTOHSCROLL | ES_MULTILINE | ES_AUTOVSCROLL, WS_EX_CLIENTEDGE);
	
	ctrlMessageContainer.SubclassWindow(ctrlMessage.m_hWnd);
	
	ctrlMessage.SetFont((HFONT)::GetStockObject(DEFAULT_GUI_FONT));
	if(user->isOnline()) {
		SetWindowText((user->getNick() + " (" + user->getClientName() + ")").c_str());
	} else {
		SetWindowText((user->getNick() + " (Offline)").c_str());
	}
	
	created = true;

	bHandled = FALSE;
	return 1;
}

void PrivateFrame::gotMessage(const User::Ptr& aUser, const string& aMessage, HWND aParent, FlatTabCtrl* aTab) {
	PrivateFrame* p = NULL;
	Lock l(cs);
	FrameIter i = frames.find(aUser);
	if(i == frames.end()) {
		bool found = false;
		for(i = frames.begin(); i != frames.end(); ++i) {
			if( (!i->first->isOnline()) && 
				(i->first->getNick() == aUser->getNick()) &&
				(i->first->getLastHubIp() == aUser->getLastHubIp()) ) {
				
				found = true;
				p = i->second;
				frames.erase(i);
				frames[aUser] = p;
				p->setUser(aUser);
				p->addLine(aMessage);
				break;
			}
		}
		if(!found) {
			p = new PrivateFrame(aUser, aParent);
			frames[aUser] = p;
			p->setTab(aTab);
			p->addLine(aMessage);
			if(Util::getAway()) {
				p->sendMessage(Util::getAwayMessage());
			}
		}
	} else {
		i->second->addLine(aMessage);
	}
}

void PrivateFrame::openWindow(const User::Ptr& aUser, HWND aParent, FlatTabCtrl* aTab) {
	PrivateFrame* p = NULL;
	Lock l(cs);
	FrameIter i = frames.find(aUser);
	if(i == frames.end()) {
		bool found = false;
		for(i = frames.begin(); i != frames.end(); ++i) {
			if( (!i->first->isOnline()) && 
				(i->first->getNick() == aUser->getNick()) &&
				(i->first->getLastHubIp() == aUser->getLastHubIp()) ) {

				found = true;
				p = i->second;
				frames.erase(i);
				frames[aUser] = p;
				p->setUser(aUser);
				break;
			}
		}
		if(!found) {
			p = new PrivateFrame(aUser, aParent);
			frames[aUser] = p;
			p->setTab(aTab);
			p->CreateEx(aParent);
		}
	} else {
		i->second->MDIActivate(i->second->m_hWnd);
	}
}

void PrivateFrame::onEnter()
{
	if(user->isOnline()) {
		char* message;
		
		if(ctrlMessage.GetWindowTextLength() > 0) {
			message = new char[ctrlMessage.GetWindowTextLength()+1];
			ctrlMessage.GetWindowText(message, ctrlMessage.GetWindowTextLength()+1);
			sendMessage(string(message, ctrlMessage.GetWindowTextLength()));
			delete message;
			ctrlMessage.SetWindowText("");
		} 
	} else {
		ctrlStatus.SetText(0, "User went offline");
	}
}

/**
 * @file PrivateFrame.cpp
 * $Id: PrivateFrame.cpp,v 1.19 2002/03/04 23:52:31 arnetheduck Exp $
 * @if LOG
 * $Log: PrivateFrame.cpp,v $
 * Revision 1.19  2002/03/04 23:52:31  arnetheduck
 * Updates and bugfixes, new user handling almost finished...
 *
 * Revision 1.18  2002/02/27 12:02:09  arnetheduck
 * Completely new user handling, wonder how it turns out...
 *
 * Revision 1.17  2002/02/18 23:48:32  arnetheduck
 * New prerelease, bugs fixed and features added...
 *
 * Revision 1.16  2002/02/09 18:13:51  arnetheduck
 * Fixed level 4 warnings and started using new stl
 *
 * Revision 1.15  2002/02/07 17:25:28  arnetheduck
 * many bugs fixed, time for 0.152 I think
 *
 * Revision 1.14  2002/02/04 01:10:30  arnetheduck
 * Release 0.151...a lot of things fixed
 *
 * Revision 1.13  2002/01/26 21:09:51  arnetheduck
 * Release 0.14
 *
 * Revision 1.12  2002/01/26 12:06:40  arnetheduck
 * Småsaker
 *
 * Revision 1.11  2002/01/25 00:11:26  arnetheduck
 * New settings dialog and various fixes
 *
 * Revision 1.10  2002/01/22 00:10:37  arnetheduck
 * Version 0.132, removed extra slots feature for nm dc users...and some bug
 * fixes...
 *
 * Revision 1.9  2002/01/20 22:54:46  arnetheduck
 * Bugfixes to 0.131 mainly...
 *
 * Revision 1.8  2002/01/19 19:07:39  arnetheduck
 * Last fixes before 0.13
 *
 * Revision 1.7  2002/01/18 17:41:43  arnetheduck
 * Reworked many right button menus, adding op commands and making more easy to use
 *
 * Revision 1.6  2002/01/17 23:35:59  arnetheduck
 * Reworked threading once more, now it actually seems stable. Also made
 * sure that noone tries to access client objects that have been deleted
 * as well as some other minor updates
 *
 * Revision 1.5  2002/01/14 22:19:43  arnetheduck
 * Commiting minor bugfixes
 *
 * Revision 1.4  2002/01/13 22:50:48  arnetheduck
 * Time for 0.12, added favorites, a bunch of new icons and lot's of other stuff
 *
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



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
#include "../client/DCPlusPlus.h"
#include "Resource.h"

#include "PrivateFrame.h"

#include "../client/Client.h"
#include "../client/ClientManager.h"
#include "../client/Util.h"
#include "../client/ResourceManager.h"
#include "../client/LogManager.h"

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
	ctrlClient.SetFont(WinUtil::font);
	ctrlMessage.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		ES_AUTOHSCROLL | ES_MULTILINE | ES_AUTOVSCROLL, WS_EX_CLIENTEDGE);
	
	ctrlMessageContainer.SubclassWindow(ctrlMessage.m_hWnd);
	
	ctrlMessage.SetFont((HFONT)::GetStockObject(DEFAULT_GUI_FONT));
	if(user->isOnline()) {
		SetWindowText((user->getNick() + " (" + user->getClientName() + ")").c_str());
	} else {
		SetWindowText((user->getNick() + " (" + STRING(OFFLINE) + ")").c_str());
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
		ctrlStatus.SetText(0, CSTRING(USER_WENT_OFFLINE));
	}
}

void PrivateFrame::addLine(const string& aLine) {
	if(!created) {
		CreateEx(parent);
	}
	if(BOOLSETTING(TIME_STAMPS)) {
		ctrlClient.AppendText(("\r\n[" + Util::getShortTimeString() + "] " + aLine).c_str());
		if(BOOLSETTING(LOG_PRIVATE_CHAT)) {
			LOG(user->getNick(), "[" + Util::getShortTimeString() + "] " + aLine);
		}
		
	} else {
		ctrlClient.AppendText(("\r\n" + aLine).c_str());
		if(BOOLSETTING(LOG_PRIVATE_CHAT)) {
			LOG(user->getNick(), aLine);
		}
	}
	addClientLine("Last change: " + Util::getTimeString());
	setDirty();
}

/**
 * @file PrivateFrame.cpp
 * $Id: PrivateFrame.cpp,v 1.2 2002/04/13 12:57:23 arnetheduck Exp $
 */



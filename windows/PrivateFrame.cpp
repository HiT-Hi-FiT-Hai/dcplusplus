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
#include "SearchFrm.h"

#include "../client/Client.h"
#include "../client/ClientManager.h"
#include "../client/Util.h"
#include "../client/LogManager.h"
#include "../client/UploadManager.h"

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
				if(BOOLSETTING(PRIVATE_MESSAGE_BEEP)) {
					MessageBeep(MB_OK);
				}
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

			if(BOOLSETTING(PRIVATE_MESSAGE_BEEP) || BOOLSETTING(PRIVATE_MESSAGE_BEEP_OPEN)) {
				MessageBeep(MB_OK);
			}
		}
	} else {
		if(BOOLSETTING(PRIVATE_MESSAGE_BEEP)) {
			MessageBeep(MB_OK);
		}
		i->second->addLine(aMessage);
	}
}

void PrivateFrame::openWindow(const User::Ptr& aUser, HWND aParent, FlatTabCtrl* aTab) {
	PrivateFrame* p = NULL;
	Lock l(cs);
	FrameIter i = frames.find(aUser);
	if(i == frames.end()) {
		p = new PrivateFrame(aUser, aParent);
		frames[aUser] = p;
		p->setTab(aTab);
		p->CreateEx(aParent);
	} else {
		i->second->MDIActivate(i->second->m_hWnd);
	}
}

void PrivateFrame::onEnter()
{
	char* message;
	
	if(ctrlMessage.GetWindowTextLength() > 0) {
		message = new char[ctrlMessage.GetWindowTextLength()+1];
		ctrlMessage.GetWindowText(message, ctrlMessage.GetWindowTextLength()+1);
		string s(message, ctrlMessage.GetWindowTextLength());
		delete message;

		// Process special commands
		if(s[0] == '/') {
			string param;
			int i = s.find(' ');
			if(i != string::npos) {
				param = s.substr(i+1);
				s = s.substr(1, i - 1);
			} else {
				s = s.substr(1);
			}

			if(Util::stricmp(s.c_str(), "refresh")==0) {
				try {
					ShareManager::getInstance()->setDirty();
					ShareManager::getInstance()->refresh(true);
					addClientLine(STRING(FILE_LIST_REFRESHED));
				} catch(ShareException e) {
					addClientLine(e.getError());
				}
			} else if(Util::stricmp(s.c_str(), "slots")==0) {
				int j = Util::toInt(param);
				if(j > 0) {
					SettingsManager::getInstance()->set(SettingsManager::SLOTS, j);
					addClientLine(STRING(SLOTS_SET));
					ClientManager::getInstance()->infoUpdated();
				} else {
					addClientLine(STRING(INVALID_NUMBER_OF_SLOTS));
				}
			} else if(Util::stricmp(s.c_str(), "search") == 0) {
				if(!param.empty()) {
					SearchFrame* pChild = new SearchFrame();
					pChild->setTab(getTab());
					pChild->setInitial(param, 0, SearchManager::SIZE_ATLEAST);
					pChild->CreateEx(m_hWndMDIClient);
				} else {
					addClientLine(STRING(SPECIFY_SEARCH_STRING));
				}
			} else if(Util::stricmp(s.c_str(), "clear") == 0) {
				ctrlClient.SetWindowText("");
			} else if(Util::stricmp(s.c_str(), "away") == 0) {
				if(Util::getAway()) {
					Util::setAway(false);
					addClientLine(STRING(AWAY_MODE_OFF));
				} else {
					Util::setAway(true);
					Util::setAwayMessage(param);
					addClientLine(STRING(AWAY_MODE_ON) + Util::getAwayMessage());
				}
			} else if(Util::stricmp(s.c_str(), "back") == 0) {
				Util::setAway(false);
				addClientLine(STRING(AWAY_MODE_OFF));
			} else if(Util::stricmp(s.c_str(), "grant") == 0) {
				UploadManager::getInstance()->reserveSlot(getUser());
				addClientLine(STRING(SLOT_GRANTED));
			} else if(Util::stricmp(s.c_str(), "close") == 0) {
				PostMessage(WM_CLOSE);
			} else if(Util::stricmp(s.c_str(), "help") == 0) {
				addLine("/refresh, /slots #, /search <string>, /clear, /away <msg>, /back, /grant, /close, /help");
			}
		} else {
			if(user->isOnline()) {
				sendMessage(s);
			} else {
				ctrlStatus.SetText(0, CSTRING(USER_WENT_OFFLINE));
			}
		}
		ctrlMessage.SetWindowText("");
	} 
}

void PrivateFrame::addLine(const string& aLine) {
	if(!created) {
		CreateEx(parent);
	}

	if(BOOLSETTING(LOG_PRIVATE_CHAT)) {
		StringMap params;
		params["message"] = aLine;
		LOG(user->getNick(), Util::formatParams(SETTING(LOG_FORMAT_PRIVATE_CHAT), params));
	}

	if(BOOLSETTING(TIME_STAMPS)) {
		ctrlClient.AppendText(("\r\n[" + Util::getShortTimeString() + "] " + aLine).c_str());
		
	} else {
		ctrlClient.AppendText(("\r\n" + aLine).c_str());
	}
	addClientLine("Last change: " + Util::getTimeString());
	setDirty();
}

void PrivateFrame::UpdateLayout(BOOL bResizeBars /* = TRUE */) {
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

/**
 * @file PrivateFrame.cpp
 * $Id: PrivateFrame.cpp,v 1.9 2002/05/30 19:09:33 arnetheduck Exp $
 */



/* 
 * Copyright (C) 2001-2004 Jacek Sieka, j_s at telia com
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
#include "WinUtil.h"

#include "../client/Client.h"
#include "../client/ClientManager.h"
#include "../client/Util.h"
#include "../client/LogManager.h"
#include "../client/UploadManager.h"
#include "../client/ShareManager.h"
#include "../client/HubManager.h"
#include "../client/QueueManager.h"

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
	ctrlClientContainer.SubclassWindow(ctrlClient.m_hWnd);

	ctrlMessage.SetFont(WinUtil::font);

	tabMenu.CreatePopupMenu();
	tabMenu.AppendMenu(MF_STRING, IDC_GETLIST, CTSTRING(GET_FILE_LIST));
	tabMenu.AppendMenu(MF_STRING, IDC_MATCH_QUEUE, CTSTRING(MATCH_QUEUE));
	tabMenu.AppendMenu(MF_STRING, IDC_GRANTSLOT, CTSTRING(GRANT_EXTRA_SLOT));
	tabMenu.AppendMenu(MF_STRING, IDC_ADD_TO_FAVORITES, CTSTRING(ADD_TO_FAVORITES));

	PostMessage(WM_SPEAKER, USER_UPDATED);
	created = true;

	ClientManager::getInstance()->addListener(this);

	bHandled = FALSE;
	return 1;
}

void PrivateFrame::gotMessage(const User::Ptr& aUser, const tstring& aMessage) {
	PrivateFrame* p = NULL;
	Lock l(cs);
	FrameIter i = frames.find(aUser);
	if(i == frames.end()) {
		bool found = false;
		for(i = frames.begin(); i != frames.end(); ++i) {
			if( (!i->first->isOnline()) && 
				(i->first->getNick() == aUser->getNick()) &&
				(i->first->getLastHubAddress() == aUser->getLastHubAddress()) ) {
				
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
			p = new PrivateFrame(aUser);
			frames[aUser] = p;
			p->addLine(aMessage);
			if(Util::getAway()) {
				// if no_awaymsg_to_bots is set, and aUser has an empty connection type (i.e. probably is a bot), then don't send
				if(!(BOOLSETTING(NO_AWAYMSG_TO_BOTS) && aUser->getConnection().empty()))
					p->sendMessage(Text::toT(Util::getAwayMessage()));
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

void PrivateFrame::openWindow(const User::Ptr& aUser, const tstring& msg) {
	PrivateFrame* p = NULL;
	Lock l(cs);
	FrameIter i = frames.find(aUser);
	if(i == frames.end()) {
		p = new PrivateFrame(aUser);
		frames[aUser] = p;
		p->CreateEx(WinUtil::mdiClient);
	} else {
		p = i->second;
		if(::IsIconic(p->m_hWnd))
			::ShowWindow(p->m_hWnd, SW_RESTORE);
		p->MDIActivate(p->m_hWnd);
	}
	if(!msg.empty())
		p->sendMessage(msg);
}

LRESULT PrivateFrame::onChar(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled) {
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

void PrivateFrame::onEnter()
{
	TCHAR* message = NULL;
	bool resetText = true;

	if(ctrlMessage.GetWindowTextLength() > 0) {
		message = new TCHAR[ctrlMessage.GetWindowTextLength()+1];
		ctrlMessage.GetWindowText(message, ctrlMessage.GetWindowTextLength()+1);
		tstring s(message, ctrlMessage.GetWindowTextLength());
		delete[] message;

		// Process special commands
		if(s[0] == '/') {
			tstring param;
			tstring message;
			tstring status;
			if(WinUtil::checkCommand(s, param, message, status)) {
				if(!message.empty()) {
					sendMessage(message);
				}
				if(!status.empty()) {
					addClientLine(status);
				}
			} else if(Util::stricmp(s.c_str(), _T("clear")) == 0) {
				ctrlClient.SetWindowText(_T(""));
			} else if(Util::stricmp(s.c_str(), _T("grant")) == 0) {
				UploadManager::getInstance()->reserveSlot(getUser());
				addClientLine(TSTRING(SLOT_GRANTED));
			} else if(Util::stricmp(s.c_str(), _T("close")) == 0) {
				PostMessage(WM_CLOSE);
			} else if((Util::stricmp(s.c_str(), _T("favorite")) == 0) || (Util::stricmp(s.c_str(), _T("fav")) == 0)) {
				HubManager::getInstance()->addFavoriteUser(getUser());
				addLine(TSTRING(FAVORITE_USER_ADDED));
			} else if(Util::stricmp(s.c_str(), _T("getlist")) == 0) {
				BOOL bTmp;
				onGetList(0,0,0,bTmp);
			} else if(Util::stricmp(s.c_str(), _T("log")) == 0) {
				WinUtil::openFile(Text::toT(Util::validateFileName(SETTING(LOG_DIRECTORY) + user->getNick() + ".log")));
			} else if(Util::stricmp(s.c_str(), _T("help")) == 0) {
				addLine(_T("*** ") + WinUtil::commands + _T(", /getlist, /clear, /grant, /close, /favorite, /log  <system, downloads, uploads>"));
			} else {
				if(user->isOnline()) {
					sendMessage(s);
				} else {
					ctrlStatus.SetText(0, CTSTRING(USER_WENT_OFFLINE));
					resetText = false;
				}
			}
		} else {
			if(user->isOnline()) {
				sendMessage(s);
			} else {
				ctrlStatus.SetText(0, CTSTRING(USER_WENT_OFFLINE));
				resetText = false;
			}
		}
		if(resetText)
			ctrlMessage.SetWindowText(_T(""));
	} 
}

LRESULT PrivateFrame::onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	if(!closed) {
		ClientManager::getInstance()->removeListener(this);

		closed = true;
		PostMessage(WM_CLOSE);
		return 0;
	} else {
		Lock l(cs);
		frames.erase(user);

		bHandled = FALSE;
		return 0;
	}
}

void PrivateFrame::addLine(const tstring& aLine) {
	if(!created) {
		if(BOOLSETTING(POPUNDER_PM))
			WinUtil::hiddenCreateEx(this);
		else
			CreateEx(WinUtil::mdiClient);
	}

	if(BOOLSETTING(LOG_PRIVATE_CHAT)) {
		StringMap params;
		params["message"] = Text::fromT(aLine);
		LOG(user->getNick(), Util::formatParams(SETTING(LOG_FORMAT_PRIVATE_CHAT), params));
	}

	if(BOOLSETTING(TIME_STAMPS)) {
		ctrlClient.AppendText((Text::toT("\r\n[" + Util::getShortTimeString() + "] ") + aLine).c_str());
		
	} else {
		ctrlClient.AppendText((_T("\r\n") + aLine).c_str());
	}
	addClientLine(CTSTRING(LAST_CHANGE) + Text::toT(Util::getTimeString()));

	if (BOOLSETTING(TAB_DIRTY)) {
		setDirty();
	}
}

LRESULT PrivateFrame::onTabContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };        // location of mouse click 
	prepareMenu(tabMenu, UserCommand::CONTEXT_CHAT, Text::toT(user->getClientAddressPort()), user->isClientOp());
	tabMenu.AppendMenu(MF_SEPARATOR);
	tabMenu.AppendMenu(MF_STRING, IDC_CLOSE_WINDOW, CTSTRING(CLOSE));
	tabMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_BOTTOMALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
	tabMenu.DeleteMenu(tabMenu.GetMenuItemCount()-1, MF_BYPOSITION);
	tabMenu.DeleteMenu(tabMenu.GetMenuItemCount()-1, MF_BYPOSITION);
	cleanMenu(tabMenu);
	return TRUE;
}

void PrivateFrame::runUserCommand(UserCommand& uc) {
	if(!WinUtil::getUCParams(m_hWnd, uc, ucParams))
		return;

	ucParams["mynick"] = user->getClientNick();
	ucParams["mycid"] = user->getClientCID().toBase32();

	user->getParams(ucParams);
	user->clientEscapeParams(ucParams);

	user->sendUserCmd(Util::formatParams(uc.getCommand(), ucParams));
	return;
};



LRESULT PrivateFrame::onGetList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	try {
		QueueManager::getInstance()->addList(user, QueueItem::FLAG_CLIENT_VIEW);
	} catch(const Exception& e) {
		addClientLine(Text::toT(e.getError()));
	}
	return 0;
}

LRESULT PrivateFrame::onMatchQueue(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	try {
		QueueManager::getInstance()->addList(user, QueueItem::FLAG_MATCH_QUEUE);
	} catch(const Exception& e) {
		addClientLine(Text::toT(e.getError()));
	}
	return 0;
}

LRESULT PrivateFrame::onGrantSlot(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	UploadManager::getInstance()->reserveSlot(user);
	return 0;
}

LRESULT PrivateFrame::onAddToFavorites(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	HubManager::getInstance()->addFavoriteUser(user);
	return 0;
}

void PrivateFrame::UpdateLayout(BOOL bResizeBars /* = TRUE */) {
	RECT rect;
	GetClientRect(&rect);
	// position bars and offset their dimensions
	UpdateBarsPosition(rect, bResizeBars);
	
	if(ctrlStatus.IsWindow()) {
		CRect sr;
		int w[1];
		ctrlStatus.GetClientRect(sr);
		
		w[0] = sr.right - 16;

		ctrlStatus.SetParts(1, w);
	}
	
	int h = WinUtil::fontHeight + 4;

	CRect rc = rect;
	rc.bottom -= h + 10;
	ctrlClient.MoveWindow(rc);
	
	rc = rect;
	rc.bottom -= 2;
	rc.top = rc.bottom - h - 5;
	rc.left +=2;
	rc.right -=2;
	ctrlMessage.MoveWindow(rc);
	
}

LRESULT PrivateFrame::onLButton(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled) {
	HWND focus = GetFocus();
	bHandled = false;
	if(focus == ctrlClient.m_hWnd) {
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		tstring x;
		string::size_type start = (string::size_type)WinUtil::textUnderCursor(pt, ctrlClient, x);
		string::size_type end = x.find(_T(" "), start);

		if(end == string::npos)
			end = x.length();
		
		bHandled = WinUtil::parseDBLClick(x, start, end);
	}
	return 0;
}

/**
 * @file
 * $Id: PrivateFrame.cpp,v 1.38 2004/10/26 13:53:59 arnetheduck Exp $
 */



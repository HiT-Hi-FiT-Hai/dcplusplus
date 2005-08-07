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

#if !defined(PRIVATE_FRAME_H)
#define PRIVATE_FRAME_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../client/User.h"
#include "../client/CriticalSection.h"
#include "../client/ClientManagerListener.h"
#include "../client/ResourceManager.h"

#include "FlatTabCtrl.h"
#include "WinUtil.h"
#include "UCHandler.h"

#define PM_MESSAGE_MAP 8		// This could be any number, really...

class PrivateFrame : public MDITabChildWindowImpl<PrivateFrame, RGB(0, 255, 255)>, 
	private ClientManagerListener, public UCHandler<PrivateFrame>
{
public:
	static void gotMessage(const User::Ptr& aUser, const tstring& aMessage);
	static void openWindow(const User::Ptr& aUser, const tstring& aMessage = Util::emptyStringT);
	static bool isOpen(const User::Ptr u) { return frames.find(u) != frames.end(); };

	enum {
		USER_UPDATED
	};

	DECLARE_FRAME_WND_CLASS_EX(_T("PrivateFrame"), IDR_PRIVATE, 0, COLOR_3DFACE);

	virtual void OnFinalMessage(HWND /*hWnd*/) {
		delete this;
	}

	typedef MDITabChildWindowImpl<PrivateFrame, RGB(0, 255, 255)> baseClass;
	typedef UCHandler<PrivateFrame> ucBase;

	BEGIN_MSG_MAP(PrivateFrame)
		MESSAGE_HANDLER(WM_SETFOCUS, onFocus)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_CTLCOLOREDIT, onCtlColor)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, onCtlColor)
		MESSAGE_HANDLER(WM_CLOSE, onClose)
		MESSAGE_HANDLER(WM_SPEAKER, onSpeaker)
		MESSAGE_HANDLER(FTM_CONTEXTMENU, onTabContextMenu)
		COMMAND_ID_HANDLER(IDC_GETLIST, onGetList)
		COMMAND_ID_HANDLER(IDC_MATCH_QUEUE, onMatchQueue)
		COMMAND_ID_HANDLER(IDC_GRANTSLOT, onGrantSlot)
		COMMAND_ID_HANDLER(IDC_ADD_TO_FAVORITES, onAddToFavorites)
		COMMAND_ID_HANDLER(IDC_SEND_MESSAGE, onSendMessage)
		COMMAND_ID_HANDLER(IDC_CLOSE_WINDOW, onCloseWindow)
		CHAIN_COMMANDS(ucBase)
		CHAIN_MSG_MAP(baseClass)
	ALT_MSG_MAP(PM_MESSAGE_MAP)
		MESSAGE_HANDLER(WM_CHAR, onChar)
		MESSAGE_HANDLER(WM_KEYDOWN, onChar)
		MESSAGE_HANDLER(WM_KEYUP, onChar)
		MESSAGE_HANDLER(WM_LBUTTONDBLCLK, onLButton)
	END_MSG_MAP()

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onChar(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onGetList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onMatchQueue(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onGrantSlot(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onAddToFavorites(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onTabContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT onLButton(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);

	void addLine(const tstring& aLine);
	void onEnter();
	void UpdateLayout(BOOL bResizeBars = TRUE);	
	void runUserCommand(UserCommand& uc);
	void readLog();
	
	LRESULT onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	
	LRESULT onSendMessage(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		onEnter();
		return 0;
	}

	LRESULT onCloseWindow(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		PostMessage(WM_CLOSE);
		return 0;
	}

	LRESULT PrivateFrame::onSpeaker(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /* bHandled */) {
		updateTitle();
		return 0;
	}
	
	LRESULT onCtlColor(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		HWND hWnd = (HWND)lParam;
		HDC hDC = (HDC)wParam;
		if(hWnd == ctrlClient.m_hWnd || hWnd == ctrlMessage.m_hWnd) {
			::SetBkColor(hDC, WinUtil::bgColor);
			::SetTextColor(hDC, WinUtil::textColor);
			return (LRESULT)WinUtil::bgBrush;
		}
		bHandled = FALSE;
		return FALSE;
	};

	LRESULT onFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		ctrlMessage.SetFocus();
		return 0;
	}
	
	void addClientLine(const tstring& aLine) {
		if(!created) {
			CreateEx(WinUtil::mdiClient);
		}
		ctrlStatus.SetText(0, (_T("[") + Text::toT(Util::getShortTimeString()) + _T("] ") + aLine).c_str());
		if (BOOLSETTING(TAB_PM_DIRTY)) {
			setDirty();
		}
	}
	
	void setUser(const User::Ptr& aUser) { user = aUser; };
	void sendMessage(const tstring& msg);
	
	User::Ptr& getUser() { return user; };
private:
	PrivateFrame(const User::Ptr& aUser) : user(aUser), 
		created(false), closed(false), 
		ctrlMessageContainer(_T("edit"), this, PM_MESSAGE_MAP),
		ctrlClientContainer(_T("edit"), this, PM_MESSAGE_MAP) {
	}
	
	~PrivateFrame() {
	}
	
	bool created;
	typedef HASH_MAP<User::Ptr, PrivateFrame*, User::HashFunction> FrameMap;
	typedef FrameMap::iterator FrameIter;
	static FrameMap frames;
	CEdit ctrlClient;
	CEdit ctrlMessage;
	CStatusBarCtrl ctrlStatus;
	static CriticalSection cs;

	CMenu tabMenu;

	StringMap ucParams;

	User::Ptr user;
	CContainedWindow ctrlMessageContainer;
	CContainedWindow ctrlClientContainer;

	bool closed;

	void updateTitle() {
		if(user->isOnline()) {
			/** @todo Find something better here perhaps? */
			SetWindowText(Text::toT(user->getFirstNick()).c_str());
			setTabColor(RGB(0, 255, 255));
		} else {
			/** @todo Find something better here perhaps? */
			SetWindowText(Text::toT(user->getFirstNick() + " [" + STRING(OFFLINE) + "]").c_str());
			setTabColor(RGB(255, 0, 0));
		}
	}
	
	// ClientManagerListener
	virtual void on(ClientManagerListener::UserUpdated, const User::Ptr& aUser) throw() {
		if(aUser == user)
			PostMessage(WM_SPEAKER, USER_UPDATED);
	}
	virtual void on(ClientManagerListener::UserConnected, const User::Ptr& aUser) throw() {
		if(aUser == user)
			PostMessage(WM_SPEAKER, USER_UPDATED);
	}
	virtual void on(ClientManagerListener::UserDisconnected, const User::Ptr& aUser) throw() {
		if(aUser == user)
			PostMessage(WM_SPEAKER, USER_UPDATED);
	}
};

#endif // !defined(PRIVATE_FRAME_H)

/**
 * @file
 * $Id: PrivateFrame.h,v 1.31 2005/08/07 13:05:47 arnetheduck Exp $
 */

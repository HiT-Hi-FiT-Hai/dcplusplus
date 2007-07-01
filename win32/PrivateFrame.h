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

#ifndef DCPLUSPLUS_WIN32_PRIVATE_FRAME_H
#define DCPLUSPLUS_WIN32_PRIVATE_FRAME_H

#include "MDIChildFrame.h"
#include "TableLayout.h"

#include <dcpp/ClientManagerListener.h>
#include <dcpp/User.h>

class PrivateFrame : 
	public MDIChildFrame<PrivateFrame>, 
	private ClientManagerListener
{
public:
	enum Status {
		STATUS_STATUS,
		STATUS_DUMMY,
		STATUS_LAST
	};
	
	static void gotMessage(SmartWin::Widget* mdiParent, const User::Ptr& from, const User::Ptr& to, const User::Ptr& replyTo, const tstring& aMessage);
	static void openWindow(SmartWin::Widget* mdiParent, const UserPtr& replyTo, const tstring& aMessage = Util::emptyStringT);
	static bool isOpen(const User::Ptr u) { return frames.find(u) != frames.end(); }
	static void closeAll();
	static void closeAllOffline();

	void sendMessage(const tstring& msg);

protected:
	typedef MDIChildFrame<PrivateFrame> Base;
	friend class MDIChildFrame<PrivateFrame>;
	
	void layout();
	HRESULT spoken(LPARAM lp, WPARAM wp);
	bool preClosing();
	
	using Base::charred;
	bool charred(WidgetTextBoxPtr w, int c);
	bool enter();

private:
	enum Tasks { USER_UPDATED
	};

	WidgetTextBoxPtr chat;
	WidgetTextBoxPtr message;
	TableLayout layoutTable;
	
	typedef HASH_MAP<UserPtr, PrivateFrame*, User::HashFunction> FrameMap;
	typedef FrameMap::iterator FrameIter;
	static FrameMap frames;

	UserPtr replyTo;

	PrivateFrame(Widget* mdiParent, const UserPtr& replyTo_);
	virtual ~PrivateFrame();
	
	void readLog();
	void addChat(const tstring& aLine);
	void addStatus(const tstring& aLine, bool inChat = true);
	void updateTitle();
	
	// ClientManagerListener
	virtual void on(ClientManagerListener::UserUpdated, const OnlineUser& aUser) throw();
	virtual void on(ClientManagerListener::UserConnected, const UserPtr& aUser) throw();
	virtual void on(ClientManagerListener::UserDisconnected, const UserPtr& aUser) throw();
};

#ifdef PORT_ME
#include "../client/User.h"
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

	DECLARE_FRAME_WND_CLASS_EX(_T("PrivateFrame"), IDR_PRIVATE, 0, COLOR_3DFACE);

	typedef MDITabChildWindowImpl<PrivateFrame, RGB(0, 255, 255)> baseClass;
	typedef UCHandler<PrivateFrame> ucBase;

	BEGIN_MSG_MAP(PrivateFrame)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
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
		MESSAGE_HANDLER(WM_LBUTTONDBLCLK, onLButton)
	END_MSG_MAP()

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onGetList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onMatchQueue(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onGrantSlot(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onAddToFavorites(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onTabContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT onLButton(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);

	void runUserCommand(UserCommand& uc);

	LRESULT onCloseWindow(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		PostMessage(WM_CLOSE);
		return 0;
	}

	User::Ptr& getUser() { return replyTo; }
private:
	CMenu tabMenu;

	CContainedWindow ctrlMessageContainer;
	CContainedWindow ctrlClientContainer;

	StringMap ucLineParams;

	void updateTitle();

};

#endif

#endif // !defined(PRIVATE_FRAME_H)

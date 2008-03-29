/*
 * Copyright (C) 2001-2008 Jacek Sieka, arnetheduck on gmail point com
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
#include "AspectUserCommand.h"

#include <dcpp/ClientManagerListener.h>
#include <dcpp/User.h>

class PrivateFrame : 
	public MDIChildFrame<PrivateFrame>, 
	private ClientManagerListener,
	public AspectUserCommand<PrivateFrame>
{
public:
	enum Status {
		STATUS_STATUS,
		STATUS_LAST
	};
	
	static void gotMessage(SmartWin::WidgetTabView* mdiParent, const UserPtr& from, const UserPtr& to, const UserPtr& replyTo, const tstring& aMessage);
	static void openWindow(SmartWin::WidgetTabView* mdiParent, const UserPtr& replyTo, const tstring& aMessage = Util::emptyStringT);
	static bool isOpen(const UserPtr& u) { return frames.find(u) != frames.end(); }
	static void closeAll();
	static void closeAllOffline();

	void sendMessage(const tstring& msg, bool thirdPerson = false);

private:
	typedef MDIChildFrame<PrivateFrame> BaseType;
	friend class MDIChildFrame<PrivateFrame>;
	friend class AspectUserCommand<PrivateFrame>;
	
	enum Tasks { USER_UPDATED
	};

	TextBoxPtr chat;
	TextBoxPtr message;

	StringMap ucLineParams;
	UserPtr replyTo;

	typedef unordered_map<UserPtr, PrivateFrame*, User::Hash> FrameMap;
	typedef FrameMap::iterator FrameIter;
	static FrameMap frames;

	PrivateFrame(SmartWin::WidgetTabView* mdiParent, const UserPtr& replyTo_, bool activte);
	virtual ~PrivateFrame();
	
	void layout();
	HRESULT handleSpeaker(WPARAM wParam, LPARAM lParam);
	bool preClosing();
	bool handleTabContextMenu(const SmartWin::ScreenCoordinate& pt);
	void handleGetList();
	void handleMatchQueue();
	
	bool handleChar(int c);
	bool handleKeyDown(int c);
	bool enter();

	void readLog();
	void addChat(const tstring& aLine);
	void addStatus(const tstring& aLine, bool inChat = true);
	void updateTitle();
	
	void runUserCommand(const UserCommand& uc);

	// ClientManagerListener
	virtual void on(ClientManagerListener::UserUpdated, const OnlineUser& aUser) throw();
	virtual void on(ClientManagerListener::UserConnected, const UserPtr& aUser) throw();
	virtual void on(ClientManagerListener::UserDisconnected, const UserPtr& aUser) throw();
};

#endif // !defined(PRIVATE_FRAME_H)

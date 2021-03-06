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

#include "stdafx.h"

#include "PrivateFrame.h"
#include "HoldRedraw.h"
#include "resource.h"

#include <dcpp/ClientManager.h>
#include <dcpp/Client.h>
#include <dcpp/LogManager.h>
#include <dcpp/User.h>
#include <dcpp/FavoriteManager.h>
#include <dcpp/UploadManager.h>
#include <dcpp/QueueItem.h>
#include <dcpp/QueueManager.h>

#include <mmsystem.h>

PrivateFrame::FrameMap PrivateFrame::frames;

void PrivateFrame::openWindow(dwt::TabView* mdiParent, const UserPtr& replyTo_, const tstring& msg) {
	PrivateFrame* pf = 0;
	FrameIter i = frames.find(replyTo_);
	if(i == frames.end()) {
		pf = new PrivateFrame(mdiParent, replyTo_, true);
	} else {
		pf = i->second;
		pf->activate();
	}
	if(!msg.empty())
		pf->sendMessage(msg);
	
}

void PrivateFrame::gotMessage(dwt::TabView* mdiParent, const UserPtr& from, const UserPtr& to, const UserPtr& replyTo, const tstring& aMessage) {
	PrivateFrame* p = 0;
	const UserPtr& user = (replyTo == ClientManager::getInstance()->getMe()) ? to : replyTo;

	FrameIter i = frames.find(user);
	if(i == frames.end()) {
		p = new PrivateFrame(mdiParent, user, !BOOLSETTING(POPUNDER_PM));
		p->addChat(aMessage);
		if(Util::getAway()) {
			if(!(BOOLSETTING(NO_AWAYMSG_TO_BOTS) && user->isSet(User::BOT)))
				p->sendMessage(Text::toT(Util::getAwayMessage()));
		}
		WinUtil::playSound(SettingsManager::SOUND_PM_WINDOW);
	} else {
		i->second->addChat(aMessage);
		WinUtil::playSound(SettingsManager::SOUND_PM);
	}
}

void PrivateFrame::closeAll(){
	for(FrameIter i = frames.begin(); i != frames.end(); ++i)
		::PostMessage(i->second->handle(), WM_CLOSE, 0, 0);
}

void PrivateFrame::closeAllOffline() {
	for(FrameIter i = frames.begin(); i != frames.end(); ++i) {
		if(!i->first->isOnline())
			::PostMessage(i->second->handle(), WM_CLOSE, 0, 0);
	}
}

PrivateFrame::PrivateFrame(dwt::TabView* mdiParent, const UserPtr& replyTo_, bool activate) : 
	BaseType(mdiParent, _T(""), IDH_PM, IDR_PRIVATE, activate),
	chat(0),
	message(0),
	replyTo(replyTo_)
{
	{
		TextBox::Seed cs = WinUtil::Seeds::textBox;
		cs.style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE;
		message = addChild(cs);
		message->setHelpId(IDH_PM_MESSAGE);
		addWidget(message, true);
		message->onKeyDown(std::tr1::bind(&PrivateFrame::handleKeyDown, this, _1));
		message->onChar(std::tr1::bind(&PrivateFrame::handleChar, this, _1));
	}
	
	{
		TextBox::Seed cs = WinUtil::Seeds::textBox;
		cs.style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | ES_MULTILINE | ES_NOHIDESEL | ES_READONLY;
		chat = addChild(cs);
		chat->setHelpId(IDH_PM_CHAT);
		chat->setTextLimit(0);
		addWidget(chat);
	}
	
	initStatus();

	updateTitle();
	layout();
	
	readLog();
	
	onSpeaker(std::tr1::bind(&PrivateFrame::handleSpeaker, this, _1, _2));
	onTabContextMenu(std::tr1::bind(&PrivateFrame::handleTabContextMenu, this, _1));

	ClientManager::getInstance()->addListener(this);
	
	speak(USER_UPDATED);
	
	frames.insert(std::make_pair(replyTo, this));
}

PrivateFrame::~PrivateFrame() {
	frames.erase(replyTo);
}

void PrivateFrame::addChat(const tstring& aLine) {
	tstring line;
	if(BOOLSETTING(TIME_STAMPS)) {
		line = Text::toT("\r\n[" + Util::getShortTimeString() + "] ");
	} else {
		line = _T("\r\n");
	}
	line += aLine;

	bool scroll = chat->scrollIsAtEnd();
	HoldRedraw hold(chat, !scroll);

	size_t limit = chat->getTextLimit();
	if(chat->length() + line.size() > limit) {
		HoldRedraw hold2(chat, scroll);
		chat->setSelection(0, chat->lineIndex(chat->lineFromChar(limit / 10)));
		chat->replaceSelection(_T(""));
	}
	if(BOOLSETTING(LOG_PRIVATE_CHAT)) {
		StringMap params;
		params["message"] = Text::fromT(aLine);
		params["hubNI"] = Util::toString(ClientManager::getInstance()->getHubNames(replyTo->getCID()));
		params["hubURL"] = Util::toString(ClientManager::getInstance()->getHubs(replyTo->getCID()));
		params["userCID"] = replyTo->getCID().toBase32();
		params["userNI"] = ClientManager::getInstance()->getNicks(replyTo->getCID())[0];
		params["myCID"] = ClientManager::getInstance()->getMe()->getCID().toBase32();
		LOG(LogManager::PM, params);
	}
	chat->addText(line);

	if(scroll)
		chat->sendMessage(WM_VSCROLL, SB_BOTTOM);

	setDirty(SettingsManager::BOLD_PM);
}

void PrivateFrame::addStatus(const tstring& aLine, bool inChat /* = true */) {
	tstring line = Text::toT("[" + Util::getShortTimeString() + "] ") + aLine;

	setStatus(STATUS_STATUS, line);

	if(BOOLSETTING(STATUS_IN_CHAT) && inChat) {
		addChat(_T("*** ") + aLine);
	}
}

bool PrivateFrame::preClosing() {
	ClientManager::getInstance()->removeListener(this);
	return true;
}

void PrivateFrame::readLog() {
	if(SETTING(SHOW_LAST_LINES_LOG) == 0)
		return;

	StringMap params;
	params["hubNI"] = Util::toString(ClientManager::getInstance()->getHubNames(replyTo->getCID()));
	params["hubURL"] = Util::toString(ClientManager::getInstance()->getHubs(replyTo->getCID()));
	params["userCID"] = replyTo->getCID().toBase32();
	params["userNI"] = ClientManager::getInstance()->getNicks(replyTo->getCID())[0];
	params["myCID"] = ClientManager::getInstance()->getMe()->getCID().toBase32();
	string path = Util::validateFileName(SETTING(LOG_DIRECTORY) + Util::formatParams(SETTING(LOG_FILE_PRIVATE_CHAT), params, true));

	StringList lines;

	try {
		File f(path, File::READ, File::OPEN);
		if(f.getSize() > 32*1024) {
			f.setEndPos(- 32*1024 + 1);
		}

		lines = StringTokenizer<string>(f.read(32*1024), "\r\n").getTokens();

		f.close();
	} catch(const FileException&) { }

	if(lines.empty())
		return;

	// the last line in the log file is an empty line; remove it
	lines.pop_back();

	size_t linesCount = lines.size();
	for(size_t i = std::max(static_cast<int>(linesCount) - SETTING(SHOW_LAST_LINES_LOG), 0); i < linesCount; ++i) {
		addStatus(_T("- ") + Text::toT(lines[i]));
	}
}

void PrivateFrame::layout() {
	bool scroll = chat->scrollIsAtEnd();

	const int border = 2;
	
	dwt::Rectangle r(getClientAreaSize()); 

	layoutStatus(r);
	
	int ymessage = message->getTextSize(_T("A")).y + 10;
	dwt::Rectangle rm(0, r.size.y - ymessage, r.width() , ymessage);
	message->setBounds(rm);
	
	r.size.y -= rm.size.y + border;
	chat->setBounds(r);

	if(scroll)
		chat->sendMessage(WM_VSCROLL, SB_BOTTOM);
}

void PrivateFrame::updateTitle() {
	pair<tstring, bool> hubs = WinUtil::getHubNames(replyTo);
	setText((WinUtil::getNicks(replyTo) + _T(" - ") + hubs.first));
	setIcon(hubs.second ? IDR_PRIVATE : IDR_PRIVATE_OFF);
}

bool PrivateFrame::handleChar(int c) {
	if(c == VK_RETURN) {
		if(!(isShiftPressed() || isControlPressed() || isAltPressed())) {
			return true;
		}
	}
	return false;
}

bool PrivateFrame::enter() {
	if(isShiftPressed() || isControlPressed() || isAltPressed()) {
		return false;
	}
	
	tstring s = message->getText();
	if(s.empty()) {
		::MessageBeep(MB_ICONEXCLAMATION);
		return false;
	}

	bool resetText = true;
	bool send = false;
	// Process special commands
	if(s[0] == '/') {
		tstring cmd = s;
		tstring param;
		tstring message;
		tstring status;
		bool thirdPerson = false;
		if(WinUtil::checkCommand(cmd, param, message, status, thirdPerson)) {
			if(!message.empty()) {
				sendMessage(message, thirdPerson);
			}
			if(!status.empty()) {
				addStatus(status);
			}
		} else if(Util::stricmp(cmd.c_str(), _T("clear")) == 0) {
			chat->setText(Util::emptyStringT);
		} else if(Util::stricmp(cmd.c_str(), _T("grant")) == 0) {
			UploadManager::getInstance()->reserveSlot(replyTo);
			addStatus(T_("Slot granted"));
		} else if(Util::stricmp(cmd.c_str(), _T("close")) == 0) {
			postMessage(WM_CLOSE);
		} else if((Util::stricmp(cmd.c_str(), _T("favorite")) == 0) || (Util::stricmp(cmd.c_str(), _T("fav")) == 0)) {
			FavoriteManager::getInstance()->addFavoriteUser(replyTo);
			addStatus(T_("Favorite user added"));
		} else if(Util::stricmp(cmd.c_str(), _T("getlist")) == 0) {
			// TODO handleGetList();
		} else if(Util::stricmp(cmd.c_str(), _T("log")) == 0) {
			StringMap params;

			params["hubNI"] = Util::toString(ClientManager::getInstance()->getHubNames(replyTo->getCID()));
			params["hubURL"] = Util::toString(ClientManager::getInstance()->getHubs(replyTo->getCID()));
			params["userCID"] = replyTo->getCID().toBase32();
			params["userNI"] = ClientManager::getInstance()->getNicks(replyTo->getCID())[0];
			params["myCID"] = ClientManager::getInstance()->getMe()->getCID().toBase32();
			WinUtil::openFile(Text::toT(Util::validateFileName(SETTING(LOG_DIRECTORY) + Util::formatParams(SETTING(LOG_FILE_PRIVATE_CHAT), params, true))));
		} else if(Util::stricmp(cmd.c_str(), _T("help")) == 0) {
			addStatus(_T("*** ") + WinUtil::commands + _T(", /getlist, /clear, /grant, /close, /favorite, /log <system, downloads, uploads>"));
		} else {
			send = true;
		}
	} else {
		send = true;
	}
	
	if(send) {
		if(replyTo->isOnline()) {
			sendMessage(s);
		} else {
			addStatus(T_("User went offline"));
			resetText = false;
		}
	}
	if(resetText) {
		message->setText(Util::emptyStringT);
	}
	return true;
	
}

void PrivateFrame::sendMessage(const tstring& msg, bool thirdPerson) {
	ClientManager::getInstance()->privateMessage(replyTo, Text::fromT(msg), thirdPerson);
}

HRESULT PrivateFrame::handleSpeaker(WPARAM, LPARAM) {
	updateTitle();
	return 0;
}

bool PrivateFrame::handleKeyDown(int c) {	
	if(c == VK_RETURN && enter()) {
		return true;
	} else if(c == VK_PRIOR) { // page up
		chat->sendMessage(WM_VSCROLL, SB_PAGEUP);
		return true;
	} else if(c == VK_NEXT) { // page down
		chat->sendMessage(WM_VSCROLL, SB_PAGEDOWN);
		return true;
	}

	return false;
}

void PrivateFrame::on(ClientManagerListener::UserUpdated, const OnlineUser& aUser) throw() {
	if(aUser.getUser() == replyTo)
		speak(USER_UPDATED);
}
void PrivateFrame::on(ClientManagerListener::UserConnected, const UserPtr& aUser) throw() {
	if(aUser == replyTo)
		speak(USER_UPDATED);
}
void PrivateFrame::on(ClientManagerListener::UserDisconnected, const UserPtr& aUser) throw() {
	if(aUser == replyTo)
		speak(USER_UPDATED);
}

bool PrivateFrame::handleTabContextMenu(const dwt::ScreenCoordinate& pt) {
	MenuPtr menu = addChild(WinUtil::Seeds::menu);

	menu->setTitle(getParent()->getTabText(this));
	
	menu->appendItem(IDC_GETLIST, T_("&Get file list"), std::tr1::bind(&PrivateFrame::handleGetList, this));
	menu->appendItem(IDC_MATCH_QUEUE, T_("&Match queue"), std::tr1::bind(&PrivateFrame::handleMatchQueue, this));
	menu->appendItem(IDC_GRANTSLOT, T_("Grant &extra slot"), std::tr1::bind(&UploadManager::reserveSlot, UploadManager::getInstance(), replyTo));
	if(!FavoriteManager::getInstance()->isFavoriteUser(replyTo))
		menu->appendItem(IDC_ADD_TO_FAVORITES, T_("Add To &Favorites"), std::tr1::bind(&FavoriteManager::addFavoriteUser, FavoriteManager::getInstance(), replyTo), dwt::BitmapPtr(new dwt::Bitmap(IDB_FAVORITE_USERS)));

	prepareMenu(menu, UserCommand::CONTEXT_CHAT, ClientManager::getInstance()->getHubs(replyTo->getCID()));
	menu->appendSeparatorItem();
	menu->appendItem(IDC_CLOSE_WINDOW, T_("&Close"), std::tr1::bind(&PrivateFrame::close, this, true), dwt::BitmapPtr(new dwt::Bitmap(IDB_EXIT)));

	menu->trackPopupMenu(pt, TPM_LEFTALIGN | TPM_RIGHTBUTTON);
	return true;
}

void PrivateFrame::runUserCommand(const UserCommand& uc) {
	if(!WinUtil::getUCParams(this, uc, ucLineParams))
		return;

	StringMap ucParams = ucLineParams;

	ClientManager::getInstance()->userCommand(replyTo, uc, ucParams, true);
}

void PrivateFrame::handleGetList() {
	try {
		QueueManager::getInstance()->addList(replyTo, QueueItem::FLAG_CLIENT_VIEW);
	} catch(const Exception& e) {
		addStatus(Text::toT(e.getError()));
	}
}

void PrivateFrame::handleMatchQueue() {
	try {
		QueueManager::getInstance()->addList(replyTo, QueueItem::FLAG_MATCH_QUEUE);
	} catch(const Exception& e) {
		addStatus(Text::toT(e.getError()));
	}
}

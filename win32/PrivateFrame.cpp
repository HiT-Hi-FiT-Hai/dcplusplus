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

#include "stdafx.h"

#include "PrivateFrame.h"
#include "HoldRedraw.h"
#include "resource.h"

#include <dcpp/ClientManager.h>
#include <dcpp/Client.h>
#include <dcpp/LogManager.h>
#include <dcpp/User.h>
#include <dcpp/ResourceManager.h>
#include <dcpp/FavoriteManager.h>
#include <dcpp/UploadManager.h>

#include <mmsystem.h>

PrivateFrame::FrameMap PrivateFrame::frames;

void PrivateFrame::openWindow(SmartWin::WidgetMDIParent* mdiParent, const UserPtr& replyTo_, const tstring& msg) {
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

void PrivateFrame::gotMessage(SmartWin::WidgetMDIParent* mdiParent, const User::Ptr& from, const User::Ptr& to, const User::Ptr& replyTo, const tstring& aMessage) {
	PrivateFrame* p = 0;
	const User::Ptr& user = (replyTo == ClientManager::getInstance()->getMe()) ? to : replyTo;

	FrameIter i = frames.find(user);
	if(i == frames.end()) {
		p = new PrivateFrame(mdiParent, user, !BOOLSETTING(POPUNDER_PM));
		p->addChat(aMessage);
		if(Util::getAway()) {
			if(!(BOOLSETTING(NO_AWAYMSG_TO_BOTS) && user->isSet(User::BOT)))
				p->sendMessage(Text::toT(Util::getAwayMessage()));
		}

		if(BOOLSETTING(PRIVATE_MESSAGE_BEEP) || BOOLSETTING(PRIVATE_MESSAGE_BEEP_OPEN)) {
			if (SETTING(BEEPFILE).empty())
				MessageBeep(MB_OK);
			else
				::PlaySound(Text::toT(SETTING(BEEPFILE)).c_str(), NULL, SND_FILENAME | SND_ASYNC);
		}
	} else {
		if(BOOLSETTING(PRIVATE_MESSAGE_BEEP)) {
			if (SETTING(BEEPFILE).empty())
				MessageBeep(MB_OK);
			else
				::PlaySound(Text::toT(SETTING(BEEPFILE)).c_str(), NULL, SND_FILENAME | SND_ASYNC);
		}
		i->second->addChat(aMessage);
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

PrivateFrame::PrivateFrame(SmartWin::WidgetMDIParent* mdiParent, const UserPtr& replyTo_, bool activate) : 
	BaseType(mdiParent, _T(""), SmartWin::IconPtr(new SmartWin::Icon(IDR_PRIVATE)), activate),
	chat(0),
	message(0),
	replyTo(replyTo_)
{
	{
		WidgetTextBox::Seed cs;
		cs.style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | ES_AUTOVSCROLL | ES_MULTILINE;
		cs.exStyle = WS_EX_CLIENTEDGE;
		message = createTextBox(cs);
		message->setFont(WinUtil::font);
		addWidget(message);
		message->onKeyDown(std::tr1::bind(&PrivateFrame::handleKeyDown, this, _1));
		message->onChar(std::tr1::bind(&PrivateFrame::handleChar, this, _1));
	}
	
	{
		WidgetTextBox::Seed cs;
		cs.style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | ES_MULTILINE | ES_NOHIDESEL | ES_READONLY;
		cs.exStyle = WS_EX_CLIENTEDGE;
		chat = createTextBox(cs);
		chat->setTextLimit(0);
		chat->setFont(WinUtil::font);
		addWidget(chat);
		
#ifdef PORT_ME
		/// @todo do we need this?§
		ctrlClient.FmtLines(TRUE);
		
#endif
	}
	
	initStatus();

	statusSizes[STATUS_DUMMY] = 16;

	updateTitle();
	layout();
	
	readLog();
	
	onSpeaker(std::tr1::bind(&PrivateFrame::handleSpeaker, this, _1, _2));

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

	int limit = chat->getTextLimit();
	if(StupidWin::getWindowTextLength(chat) + static_cast<int>(line.size()) > limit) {
		HoldRedraw hold(chat);
		chat->setSelection(0, StupidWin::lineIndex(chat, StupidWin::lineFromChar(chat, limit / 10)));
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
	StringMap params;

	params["hubNI"] = Util::toString(ClientManager::getInstance()->getHubNames(replyTo->getCID()));
	params["hubURL"] = Util::toString(ClientManager::getInstance()->getHubs(replyTo->getCID()));
	params["userCID"] = replyTo->getCID().toBase32();
	params["userNI"] = ClientManager::getInstance()->getNicks(replyTo->getCID())[0];
	params["myCID"] = ClientManager::getInstance()->getMe()->getCID().toBase32();

	string path = Util::validateFileName(SETTING(LOG_DIRECTORY) + Util::formatParams(SETTING(LOG_FILE_PRIVATE_CHAT), params, true));

	try {
		if (SETTING(SHOW_LAST_LINES_LOG) > 0) {
			File f(path, File::READ, File::OPEN);

			int64_t size = f.getSize();

			if(size > 32*1024) {
				f.setPos(size - 32*1024);
			}

			StringList lines = StringTokenizer<string>(f.read(32*1024), "\r\n").getTokens();

			int linesCount = lines.size();

			int i = linesCount > (SETTING(SHOW_LAST_LINES_LOG) + 1) ? linesCount - (SETTING(SHOW_LAST_LINES_LOG) + 1) : 0;

			for(; i < (linesCount - 1); ++i){
				addStatus(_T("- ") + Text::toT(lines[i]));
			}

			f.close();
		}
	} catch(const FileException&){
	}
}

void PrivateFrame::layout() {
	const int border = 2;
	
	SmartWin::Rectangle r(getClientAreaSize()); 

	SmartWin::Rectangle rs = layoutStatus();
	
	r.size.y -= rs.size.y + border;
	int ymessage = message->getTextSize(_T("A")).y + 10;
	SmartWin::Rectangle rm(0, r.size.y - ymessage, r.size.x , ymessage);
	message->setBounds(rm);
	
	r.size.y -= rm.size.y + border;
	chat->setBounds(r);
}

void PrivateFrame::updateTitle() {
	pair<tstring, bool> hubs = WinUtil::getHubNames(replyTo);
#ifdef PORT_ME
	if(hubs.second) {
		setTabColor(RGB(0, 255, 255));
	} else {
		setTabColor(RGB(255, 0, 0));
	}
#endif 
	setText((WinUtil::getNicks(replyTo) + _T(" - ") + hubs.first));
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
		tstring param;
		tstring message;
		tstring status;
		if(WinUtil::checkCommand(s, param, message, status)) {
			if(!message.empty()) {
				sendMessage(message);
			}
			if(!status.empty()) {
				addStatus(status);
			}
		} else if(Util::stricmp(s.c_str(), _T("clear")) == 0) {
			chat->setText(Util::emptyStringT);
		} else if(Util::stricmp(s.c_str(), _T("grant")) == 0) {
			UploadManager::getInstance()->reserveSlot(replyTo);
			addStatus(TSTRING(SLOT_GRANTED));
		} else if(Util::stricmp(s.c_str(), _T("close")) == 0) {
			postMessage(WM_CLOSE);
		} else if((Util::stricmp(s.c_str(), _T("favorite")) == 0) || (Util::stricmp(s.c_str(), _T("fav")) == 0)) {
			FavoriteManager::getInstance()->addFavoriteUser(replyTo);
			addStatus(TSTRING(FAVORITE_USER_ADDED));
		} else if(Util::stricmp(s.c_str(), _T("getlist")) == 0) {
			// TODO handleGetList();
		} else if(Util::stricmp(s.c_str(), _T("log")) == 0) {
			StringMap params;

			params["hubNI"] = Util::toString(ClientManager::getInstance()->getHubNames(replyTo->getCID()));
			params["hubURL"] = Util::toString(ClientManager::getInstance()->getHubs(replyTo->getCID()));
			params["userCID"] = replyTo->getCID().toBase32();
			params["userNI"] = ClientManager::getInstance()->getNicks(replyTo->getCID())[0];
			params["myCID"] = ClientManager::getInstance()->getMe()->getCID().toBase32();
			WinUtil::openFile(Text::toT(Util::validateFileName(SETTING(LOG_DIRECTORY) + Util::formatParams(SETTING(LOG_FILE_PRIVATE_CHAT), params, true))));
		} else if(Util::stricmp(s.c_str(), _T("help")) == 0) {
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
			addStatus(TSTRING(USER_WENT_OFFLINE));
			resetText = false;
		}
	}
	if(resetText) {
		message->setText(Util::emptyStringT);
	}
	return true;
	
}

void PrivateFrame::sendMessage(const tstring& msg) {
	ClientManager::getInstance()->privateMessage(replyTo, Text::fromT(msg));
}

HRESULT PrivateFrame::handleSpeaker(WPARAM, LPARAM) {
	updateTitle();
	return 0;
}

bool PrivateFrame::handleKeyDown(int c) {	
	if(c == VK_RETURN && enter()) {
		return true;
	}
	
	return false;
}

void PrivateFrame::on(ClientManagerListener::UserUpdated, const OnlineUser& aUser) throw() {
	if(aUser.getUser() == replyTo)
		speak(USER_UPDATED);
}
void PrivateFrame::on(ClientManagerListener::UserConnected, const User::Ptr& aUser) throw() {
	if(aUser == replyTo)
		speak(USER_UPDATED);
}
void PrivateFrame::on(ClientManagerListener::UserDisconnected, const User::Ptr& aUser) throw() {
	if(aUser == replyTo)
		speak(USER_UPDATED);
}

#ifdef PORT_ME

LRESULT PrivateFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	tabMenu.CreatePopupMenu();
	tabMenu.AppendMenu(MF_STRING, IDC_GETLIST, CTSTRING(GET_FILE_LIST));
	tabMenu.AppendMenu(MF_STRING, IDC_MATCH_QUEUE, CTSTRING(MATCH_QUEUE));
	tabMenu.AppendMenu(MF_STRING, IDC_GRANTSLOT, CTSTRING(GRANT_EXTRA_SLOT));
	tabMenu.AppendMenu(MF_STRING, IDC_ADD_TO_FAVORITES, CTSTRING(ADD_TO_FAVORITES));

	created = true;

	bHandled = FALSE;
	return 1;
}

LRESULT PrivateFrame::onTabContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };		// location of mouse click
	prepareMenu(tabMenu, UserCommand::CONTEXT_CHAT, ClientManager::getInstance()->getHubs(replyTo->getCID()));
	tabMenu.AppendMenu(MF_SEPARATOR);
	tabMenu.AppendMenu(MF_STRING, IDC_CLOSE_WINDOW, CTSTRING(CLOSE));
	tabMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_BOTTOMALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
	tabMenu.DeleteMenu(tabMenu.GetMenuItemCount()-1, MF_BYPOSITION);
	tabMenu.DeleteMenu(tabMenu.GetMenuItemCount()-1, MF_BYPOSITION);
	cleanMenu(tabMenu);
	return TRUE;
}

void PrivateFrame::runUserCommand(UserCommand& uc) {

	if(!WinUtil::getUCParams(m_hWnd, uc, ucLineParams))
		return;

	StringMap ucParams = ucLineParams;

	ClientManager::getInstance()->userCommand(replyTo, uc, ucParams, true);
}

LRESULT PrivateFrame::onGetList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	try {
		QueueManager::getInstance()->addList(replyTo, QueueItem::FLAG_CLIENT_VIEW);
	} catch(const Exception& e) {
		addClientLine(Text::toT(e.getError()));
	}
	return 0;
}

LRESULT PrivateFrame::onMatchQueue(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	try {
		QueueManager::getInstance()->addList(replyTo, QueueItem::FLAG_MATCH_QUEUE);
	} catch(const Exception& e) {
		addClientLine(Text::toT(e.getError()));
	}
	return 0;
}

LRESULT PrivateFrame::onGrantSlot(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	UploadManager::getInstance()->reserveSlot(replyTo);
	return 0;
}

LRESULT PrivateFrame::onAddToFavorites(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	FavoriteManager::getInstance()->addFavoriteUser(replyTo);
	return 0;
}

LRESULT PrivateFrame::onLButton(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled) {
	HWND focus = GetFocus();
	bHandled = false;
	if(focus == ctrlClient.m_hWnd) {
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		tstring x;
		tstring::size_type start = (tstring::size_type)WinUtil::textUnderCursor(pt, ctrlClient, x);
		tstring::size_type end = x.find(_T(" "), start);

		if(end == string::npos)
			end = x.length();

		bHandled = WinUtil::parseDBLClick(x, start, end);
	}
	return 0;
}
#endif

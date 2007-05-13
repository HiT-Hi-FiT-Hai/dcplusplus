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
#include <client/DCPlusPlus.h>

#include "PrivateFrame.h"

#include <client/ClientManager.h>
#include <client/Client.h>
#include <client/LogManager.h>
#include <client/User.h>
#include <client/ResourceManager.h>

#include <mmsystem.h>

PrivateFrame::FrameMap PrivateFrame::frames;

PrivateFrame::PrivateFrame(SmartWin::Widget* mdiParent, const UserPtr& replyTo_) : 
	SmartWin::Widget(mdiParent), 
	chat(0),
	message(0),
	status(0),
	replyTo(replyTo_),
	layoutTable(1, 2)
{
	{
		WidgetTextBox::Seed cs;
		cs.style = WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_NOHIDESEL | ES_READONLY;
		cs.exStyle = WS_EX_CLIENTEDGE;
		chat = createTextBox(cs);
		chat->setTextLimit(0);
		chat->setFont(WinUtil::font);
		add_widget(chat);
		layoutTable.add(chat, SmartWin::Point(20, 20), 0, 0, 1, 1, TableLayout::FILL, TableLayout::EXPAND);
		
#ifdef PORT_ME
		/// @todo do we need this?§
		ctrlClient.FmtLines(TRUE);
		
#endif
	}
	
	{
		WidgetTextBox::Seed cs;
		cs.style = WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_AUTOVSCROLL | ES_MULTILINE;
		cs.exStyle = WS_EX_CLIENTEDGE;
		message = createTextBox(cs);
		message->setFont(WinUtil::font);
		add_widget(message);
		layoutTable.add(message, SmartWin::Point(20, 20), 0, 1, 1, 1, TableLayout::FILL, TableLayout::EXPAND);
	}
	
	status = createStatusBarSections();
	memset(statusSizes, 0, sizeof(statusSizes));
	///@todo get real resizer width
	statusSizes[STATUS_DUMMY] = 16;

	updateTitle();
	layout();
	
	readLog();
	
	onSpeaker(&PrivateFrame::spoken);

	ClientManager::getInstance()->addListener(this);
	
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
		StupidWin::setRedraw(chat, false);
		chat->setSelection(0, StupidWin::lineIndex(chat, StupidWin::lineFromChar(chat, limit / 10)));
		chat->replaceSelection(_T(""));
		StupidWin::setRedraw(chat, true);
	}
#ifdef PORT_ME
	if(!created) {
		if(BOOLSETTING(POPUNDER_PM))
			WinUtil::hiddenCreateEx(this);
		else
			CreateEx(WinUtil::mdiClient);
	}
#endif
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

#ifdef PORT_ME
	if (BOOLSETTING(BOLD_PM)) {
		setDirty();
	}
#endif
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
	status->refresh();

	SmartWin::Rectangle rs(status->getClientAreaSize());

	{
		std::vector<unsigned> w(STATUS_LAST);

		w[0] = rs.size.x - rs.pos.x - std::accumulate(statusSizes+1, statusSizes+STATUS_LAST, 0); 
		std::copy(statusSizes+1, statusSizes + STATUS_LAST, w.begin()+1);

		status->setSections(w);
#ifdef PORT_ME
		ctrlLastLines.SetMaxTipWidth(w[0]);
		// Strange, can't get the correct width of the last field...
		ctrlStatus.GetRect(2, sr);
		sr.left = sr.right + 2;
		sr.right = sr.left + 16;
		ctrlShowUsers.MoveWindow(sr);
#endif
	}
	r.size.y -= status->getSize().y - border;
	layoutTable.resize(r);
	int ymessage = message->getTextSize("A").y + 10;
	SmartWin::Rectangle rm(0, r.size.y - ymessage, r.size.x, ymessage);
	message->setBounds(rm);
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

void PrivateFrame::openWindow(SmartWin::Widget* mdiParent, const UserPtr& replyTo_, const tstring& msg) {
	PrivateFrame* pf = 0;
	FrameIter i = frames.find(replyTo_);
	if(i == frames.end()) {
		pf = new PrivateFrame(mdiParent, replyTo_);
	} else {
		pf = i->second;
		if(StupidWin::isIconic(pf))
			pf->restore();
	
#ifdef PORT_ME
		i->second->MDIActivate(i->second->m_hWnd);
#endif
	}
	if(!msg.empty())
		pf->sendMessage(msg);
	
}

void PrivateFrame::gotMessage(SmartWin::Widget* mdiParent, const User::Ptr& from, const User::Ptr& to, const User::Ptr& replyTo, const tstring& aMessage) {
	PrivateFrame* p = 0;
	const User::Ptr& user = (replyTo == ClientManager::getInstance()->getMe()) ? to : replyTo;

	FrameIter i = frames.find(user);
	if(i == frames.end()) {
		p = new PrivateFrame(mdiParent, user);
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


bool PrivateFrame::enter() {
	
	tstring s = message->getText();
	if(s.empty()) {
		::MessageBeep(MB_ICONEXCLAMATION);
		return false;
	}

	bool resetText = true;

	// Process special commands
	if(s[0] == '/') {
		tstring param;
		tstring message;
		tstring status;
#ifdef PORT_ME
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
			FavoriteManager::getInstance()->addFavoriteUser(getUser());
			addStatus(TSTRING(FAVORITE_USER_ADDED));
		} else if(Util::stricmp(s.c_str(), _T("getlist")) == 0) {
			BOOL bTmp;
			onGetList(0,0,0,bTmp);
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
			if(replyTo->isOnline()) {
				sendMessage(tstring(msg));
			} else {
				ctrlStatus.SetText(0, CTSTRING(USER_WENT_OFFLINE));
				resetText = false;
			}
		}
#endif
	} else {
		if(replyTo->isOnline()) {
			sendMessage(s);
		} else {
			setStatus(STATUS_STATUS, CTSTRING(USER_WENT_OFFLINE));
			resetText = false;
		}
	}
	
	if(resetText) {
		message->setText(_T(""));
	}
	return true;
	
}

void PrivateFrame::sendMessage(const tstring& msg) {
	ClientManager::getInstance()->privateMessage(replyTo, Text::fromT(msg));
}

void PrivateFrame::setStatus(Status s, const tstring& text) {
	int w = status->getTextSize(text).x + 12;
	if(w > static_cast<int>(statusSizes[s])) {
		dcdebug("Setting status size %d to %d\n", s, w);
		statusSizes[s] = w;
		layout();
	}
	status->setText(text, s);
}

HRESULT PrivateFrame::spoken(LPARAM, WPARAM) {
	updateTitle();
	return 0;
}

bool PrivateFrame::charred(WidgetTextBoxPtr ptr, int c) {
	///@todo Investigate WM_CHAR vs WM_KEYDOWN
	
	switch(c) {
	case VK_RETURN: return enter();
	default:  return Base::charred(ptr, c);
	}
	
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
#include "../client/FavoriteManager.h"
#include "../client/QueueManager.h"
#include "../client/StringTokenizer.h"

LRESULT PrivateFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	tabMenu.CreatePopupMenu();
	tabMenu.AppendMenu(MF_STRING, IDC_GETLIST, CTSTRING(GET_FILE_LIST));
	tabMenu.AppendMenu(MF_STRING, IDC_MATCH_QUEUE, CTSTRING(MATCH_QUEUE));
	tabMenu.AppendMenu(MF_STRING, IDC_GRANTSLOT, CTSTRING(GRANT_EXTRA_SLOT));
	tabMenu.AppendMenu(MF_STRING, IDC_ADD_TO_FAVORITES, CTSTRING(ADD_TO_FAVORITES));

	PostMessage(WM_SPEAKER, USER_UPDATED);
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



void PrivateFrame::closeAll(){
	for(FrameIter i = frames.begin(); i != frames.end(); ++i)
		i->second->PostMessage(WM_CLOSE, 0, 0);
}

void PrivateFrame::closeAllOffline() {
	for(FrameIter i = frames.begin(); i != frames.end(); ++i) {
		if(!i->first->isOnline())
			i->second->PostMessage(WM_CLOSE, 0, 0);
	}
}
#endif

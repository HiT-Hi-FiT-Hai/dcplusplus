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

#include "HubFrame.h"

#include <client/ClientManager.h>
#include <client/Client.h>
#include <client/LogManager.h>
#include <client/User.h>
#include <client/ResourceManager.h>

HubFrame::FrameMap HubFrame::frames;

HubFrame::HubFrame(SmartWin::Widget* mdiParent, const string& url_) : 
	SmartWin::Widget(mdiParent), 
	client(0),
	url(url_),
	timeStamps(BOOLSETTING(TIME_STAMPS)), 
	updateUsers(false), 
	waitingForPW(false), 
	chat(0),
	message(0),
	filter(0),
	filterType(0),
	status(0)
{
	{
		WidgetTextBox::Seed cs;
		cs.style = WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_NOHIDESEL | ES_READONLY;
		cs.exStyle = WS_EX_CLIENTEDGE;
		chat = createTextBox(cs);
		chat->setTextLimit(0);
		chat->setFont(WinUtil::font);
		add_widget(chat);
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
	}
	
	{
		WidgetTextBox::Seed cs;
		cs.style = WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE;
		cs.exStyle = WS_EX_CLIENTEDGE;
		filter = createTextBox(cs);
		filter->setFont(WinUtil::font);
		add_widget(filter);
	}
	{
		WidgetComboBox::Seed cs;
		
		cs.style = WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | CBS_DROPDOWNLIST;
		cs.exStyle =  WS_EX_CLIENTEDGE;
		filterType = createComboBox(cs);
		filterType->setFont(WinUtil::font);
		add_widget(filterType);
	}
	
	status = createStatusBarSections();
	memset(statusSizes, 0, sizeof(statusSizes));
	///@todo get real resizer width
	statusSizes[STATUS_DUMMY] = 16;
	layout();
	
	initSecond();
	
	onSpeaker(&HubFrame::spoken);
	
	client = ClientManager::getInstance()->getClient(url);
	client->addListener(this);
	client->connect();
	
	frames.insert(std::make_pair(url, this));
}

HubFrame::~HubFrame() {
	frames.erase(url);
}

bool HubFrame::preClosing() {
#ifdef PORT_ME
	FavoriteManager::getInstance()->removeListener(this);
#endif
	client->removeListener(this);
	client->disconnect(true);
	return true;
}

void HubFrame::postClosing() {
#ifdef PORT_ME
	SettingsManager::getInstance()->set(SettingsManager::GET_USER_INFO, showUsers);
	FavoriteManager::getInstance()->removeUserCommand(Text::fromT(server));

	clearUserList();
	clearTaskList();

	WinUtil::saveHeaderOrder(ctrlUsers, SettingsManager::HUBFRAME_ORDER,
		SettingsManager::HUBFRAME_WIDTHS, COLUMN_LAST, columnIndexes, columnSizes);

	FavoriteHubEntry *fhe = FavoriteManager::getInstance()->getFavoriteHubEntry(Text::fromT(server));
	if(fhe != NULL && !IsIconic()){
		CRect rc;

		//Get position of window
		GetWindowRect(&rc);

		//convert the position so it's relative to main window
		::ScreenToClient(GetParent(), &rc.TopLeft());
		::ScreenToClient(GetParent(), &rc.BottomRight());

		//save the position
		fhe->setBottom((uint16_t)(rc.bottom > 0 ? rc.bottom : 0));
		fhe->setTop((uint16_t)(rc.top > 0 ? rc.top : 0));
		fhe->setLeft((uint16_t)(rc.left > 0 ? rc.left : 0));
		fhe->setRight((uint16_t)(rc.right > 0 ? rc.right : 0));

		FavoriteManager::getInstance()->save();
	}
#endif	
}

void HubFrame::layout() {
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
	int ymessage = message->getTextSize("A").y + 10;
	int xfilter = std::min(r.size.x / 4, 200l);
	SmartWin::Rectangle rm(0, r.size.y - ymessage, r.size.x - xfilter, ymessage);
	message->setBounds(rm);
	
	rm.pos.x += rm.size.x + border;
	rm.size.x = xfilter * 2 / 3 - border;
	filter->setBounds(rm);
	
	rm.pos.x += rm.size.x + border;
	rm.size.x = xfilter / 3 - border;
	filterType->setBounds(rm);
	r.size.y -= rm.size.y + border;
	chat->setBounds(r);

#ifdef PORT_ME	
	CRect rc = rect;
	rc.bottom -= h + 10;
	if(!showUsers) {
		if(GetSinglePaneMode() == SPLIT_PANE_NONE)
			SetSinglePaneMode(SPLIT_PANE_LEFT);
	} else {
		if(GetSinglePaneMode() != SPLIT_PANE_NONE)
			SetSinglePaneMode(SPLIT_PANE_NONE);
	}
	SetSplitterRect(rc);

	rc = rect;
	rc.bottom -= 2;
	rc.top = rc.bottom - h - 5;
	rc.left +=2;
	rc.right -= showUsers ? 202 : 2;
	ctrlMessage.MoveWindow(rc);

	if(showUsers){
		rc.left = rc.right + 4;
		rc.right = rc.left + 116;
		ctrlFilter.MoveWindow(rc);

		rc.left = rc.right + 4;
		rc.right = rc.left + 76;
		rc.top = rc.top + 0;
		rc.bottom = rc.bottom + 120;
		ctrlFilterSel.MoveWindow(rc);
	}
#endif
}
void HubFrame::updateStatus() {
	setStatus(STATUS_USERS, getStatusUsers());
	setStatus(STATUS_SHARED, getStatusShared());
		
}

void HubFrame::initSecond() {
	SmartWin::Command cmd(_T("1 second"));
	createTimer(&HubFrame::eachSecond, 1000, cmd);	
}

void HubFrame::eachSecond(const SmartWin::CommandPtr&) {
	updateStatus();
	///@todo change smartwin to support recurring timers 
	initSecond();
}


void HubFrame::openWindow(SmartWin::Widget* mdiParent, const string& url) {
	FrameIter i = frames.find(url);
	if(i == frames.end()) {
		new HubFrame(mdiParent, url);
	} else {
		if(StupidWin::isIconic(i->second))
			i->second->restore();
	
#ifdef PORT_ME
		i->second->MDIActivate(i->second->m_hWnd);
#endif
	}
}
bool HubFrame::enter() {
	tstring s = message->getText();
	if(s.empty()) {
		::MessageBeep(MB_ICONEXCLAMATION);
		return false;
	}

#ifdef PORT_ME	
	// save command in history, reset current buffer pointer to the newest command
	curCommandPosition = prevCommands.size();		//this places it one position beyond a legal subscript
	if (!curCommandPosition || curCommandPosition > 0 && prevCommands[curCommandPosition - 1] != s) {
		++curCommandPosition;
		prevCommands.push_back(s);
	}
	currentCommand = _T("");
#endif
	// Special command
	if(s[0] == _T('/')) {
		tstring cmd = s;
		tstring param;
		tstring msg;
		tstring status;
		if(WinUtil::checkCommand(cmd, param, msg, status)) {
			if(!msg.empty()) {
				client->hubMessage(Text::fromT(msg));
			}
			if(!status.empty()) {
				addStatus(status);
			}
		} else if(Util::stricmp(cmd.c_str(), _T("join"))==0) {
#ifdef PORT_ME
			if(!param.empty()) {
				redirect = param;
				if(BOOLSETTING(JOIN_OPEN_NEW_WINDOW)) {
					HubFrame::openWindow(param);
				} else {
					BOOL whatever = FALSE;
					onFollow(0, 0, 0, whatever);
				}
			} else {
				addClientLine(TSTRING(SPECIFY_SERVER));
			}
#endif
			} else if(Util::stricmp(cmd.c_str(), _T("clear")) == 0) {
			chat->setText(_T(""));
		} else if(Util::stricmp(cmd.c_str(), _T("ts")) == 0) {
			timeStamps = !timeStamps;
			if(timeStamps) {
				addStatus(TSTRING(TIMESTAMPS_ENABLED));
			} else {
				addStatus(TSTRING(TIMESTAMPS_DISABLED));
			}
		} else if( (Util::stricmp(cmd.c_str(), _T("password")) == 0) && waitingForPW ) {
			client->setPassword(Text::fromT(param));
			client->password(Text::fromT(param));
			waitingForPW = false;
		} else if( Util::stricmp(cmd.c_str(), _T("showjoins")) == 0 ) {
#ifdef PORT_ME
				showJoins = !showJoins;
				if(showJoins) {
					addClientLine(TSTRING(JOIN_SHOWING_ON));
				} else {
					addClientLine(TSTRING(JOIN_SHOWING_OFF));
				}
#endif
			} else if( Util::stricmp(cmd.c_str(), _T("favshowjoins")) == 0 ) {
#ifdef PORT_ME
				favShowJoins = !favShowJoins;
				if(favShowJoins) {
					addClientLine(TSTRING(FAV_JOIN_SHOWING_ON));
				} else {
					addClientLine(TSTRING(FAV_JOIN_SHOWING_OFF));
				}
#endif
			} else if(Util::stricmp(cmd.c_str(), _T("close")) == 0) {
			StupidWin::postMessage(this, WM_CLOSE);
		} else if(Util::stricmp(cmd.c_str(), _T("userlist")) == 0) {
#ifdef PORT_ME
				ctrlShowUsers.SetCheck(showUsers ? BST_UNCHECKED : BST_CHECKED);
#endif 
			} else if(Util::stricmp(cmd.c_str(), _T("connection")) == 0) {
#ifdef PORT_ME
				addStatus(Text::toT((STRING(IP) + client->getLocalIp() + ", " +
				STRING(PORT) +
				Util::toString(ConnectionManager::getInstance()->getPort()) + "/" +
				Util::toString(SearchManager::getInstance()->getPort()) + "/" +
					Util::toString(ConnectionManager::getInstance()->getSecurePort())
					)));
#endif
			} else if((Util::stricmp(cmd.c_str(), _T("favorite")) == 0) || (Util::stricmp(cmd.c_str(), _T("fav")) == 0)) {
#ifdef PORT_ME
				addAsFavorite();
#endif
			} else if((Util::stricmp(cmd.c_str(), _T("removefavorite")) == 0) || (Util::stricmp(cmd.c_str(), _T("removefav")) == 0)) {
#ifdef PORT_ME
				removeFavoriteHub();
#endif
			} else if(Util::stricmp(cmd.c_str(), _T("getlist")) == 0){
#ifdef PORT_ME
				if( !param.empty() ){
					UserInfo* ui = findUser(param);
					if(ui) {
						ui->getList();
					}
				}
#endif
			} else if(Util::stricmp(cmd.c_str(), _T("log")) == 0) {
#ifdef PORT_ME
				StringMap params;
				params["hubNI"] = client->getHubName();
			params["hubURL"] = client->getHubUrl();
			params["myNI"] = client->getMyNick();
			if(param.empty()) {
				WinUtil::openFile(Text::toT(Util::validateFileName(SETTING(LOG_DIRECTORY) + Util::formatParams(SETTING(LOG_FILE_MAIN_CHAT), params, true))));
			} else if(Util::stricmp(param.c_str(), _T("status")) == 0) {
					WinUtil::openFile(Text::toT(Util::validateFileName(SETTING(LOG_DIRECTORY) + Util::formatParams(SETTING(LOG_FILE_STATUS), params, true))));
				}
#endif
			} else if(Util::stricmp(cmd.c_str(), _T("help")) == 0) {
			addChat(_T("*** ") + WinUtil::commands + _T(", /join <hub-ip>, /clear, /ts, /showjoins, /favshowjoins, /close, /userlist, /connection, /favorite, /pm <user> [message], /getlist <user>, /log <status, system, downloads, uploads>, /removefavorite"));
		} else if(Util::stricmp(cmd.c_str(), _T("pm")) == 0) {
#ifdef PORT_ME
				string::size_type j = param.find(_T(' '));
				if(j != string::npos) {
					tstring nick = param.substr(0, j);
					UserInfo* ui = findUser(nick);

					if(ui) {
						if(param.size() > j + 1)
							PrivateFrame::openWindow(ui->user, param.substr(j+1));
						else
							PrivateFrame::openWindow(ui->user);
					}
				} else if(!param.empty()) {
					UserInfo* ui = findUser(param);
					if(ui) {
						PrivateFrame::openWindow(ui->user);
					}
				}
#endif
			} else {
				if (BOOLSETTING(SEND_UNKNOWN_COMMANDS)) {
					client->hubMessage(Text::fromT(s));
				} else {
					addStatus(TSTRING(UNKNOWN_COMMAND) + cmd);
				}
			}
			message->setText(_T(""));
	} else if(waitingForPW) {
		addStatus(TSTRING(DONT_REMOVE_SLASH_PASSWORD));
		message->setText(_T("/password "));
		message->setFocus();
		message->setSelection(10, 10);
	} else {
		client->hubMessage(Text::fromT(s));
		message->setText(_T(""));
	}
	return true;
}


void HubFrame::setStatus(Status s, const tstring& text) {
	int w = status->getTextSize(text).x + 12;
	if(w > static_cast<int>(statusSizes[s])) {
		dcdebug("Setting status size %d to %d\n", s, w);
		statusSizes[s] = w;
		layout();
	}
	status->setText(text, s);
}

void HubFrame::addChat(const tstring& aLine) {
	tstring line;
	if(timeStamps) {
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
	BOOL noscroll = TRUE;
	POINT p = ctrlClient.PosFromChar(ctrlClient.GetWindowTextLength() - 1);
	CRect r;
	ctrlClient.GetClientRect(r);

	if( r.PtInRect(p) || MDIGetActive() != m_hWnd)
		noscroll = FALSE;
	else {
		ctrlClient.SetRedraw(FALSE); // Strange!! This disables the scrolling...????
	}
	if(BOOLSETTING(LOG_MAIN_CHAT)) {
		StringMap params;
		params["message"] = Text::fromT(aLine);
		client->getHubIdentity().getParams(params, "hub", false);
		params["hubURL"] = client->getHubUrl();
		client->getMyIdentity().getParams(params, "my", true);
		LOG(LogManager::CHAT, params);
	}
#endif
	chat->addText(line);
#ifdef PORT_ME
	if(noscroll) {
		ctrlClient.SetRedraw(TRUE);
	}
	if (BOOLSETTING(BOLD_HUB)) {
		setDirty();
	}
#endif
}

void HubFrame::addStatus(const tstring& aLine, bool inChat /* = true */) {
	tstring line = Text::toT("[" + Util::getShortTimeString() + "] ") + aLine;

	setStatus(STATUS_STATUS, line);

#ifdef PORT_ME
	while(lastLinesList.size() + 1 > MAX_CLIENT_LINES)
		lastLinesList.erase(lastLinesList.begin());
	lastLinesList.push_back(line);

	if (BOOLSETTING(BOLD_HUB)) {
		setDirty();
	}
#endif
	if(BOOLSETTING(STATUS_IN_CHAT) && inChat) {
		addChat(_T("*** ") + aLine);
	}
	if(BOOLSETTING(LOG_STATUS_MESSAGES)) {
		StringMap params;
		client->getHubIdentity().getParams(params, "hub", false);
		params["hubURL"] = client->getHubUrl();
		client->getMyIdentity().getParams(params, "my", true);
		params["message"] = Text::fromT(aLine);
		LOG(LogManager::STATUS, params);
	}
}

HRESULT HubFrame::spoken(LPARAM, WPARAM) {
	updateUsers = false;
	TaskQueue::List t;
	tasks.get(t);

#ifdef PORT_ME
	if(t.size() > 2) {
		ctrlUsers.SetRedraw(FALSE);
	}
#endif

	for(TaskQueue::Iter i = t.begin(); i != t.end(); ++i) {
		if(i->first == UPDATE_USER) {
#ifdef PORT_ME
			updateUser(*static_cast<UserTask*>(i->second));
#endif
		} else if(i->first == UPDATE_USER_JOIN) {
#ifdef PORT_ME
			UserTask& u = *static_cast<UserTask*>(i->second);
			if(updateUser(u)) {
				if (showJoins || (favShowJoins && FavoriteManager::getInstance()->isFavoriteUser(u.user))) {
					addLine(_T("*** ") + TSTRING(JOINS) + Text::toT(u.identity.getNick()));
				}
			}
#endif
		} else if(i->first == REMOVE_USER) {
#ifdef PORT_ME
			UserTask& u = *static_cast<UserTask*>(i->second);
			removeUser(u.user);
			if (showJoins || (favShowJoins && FavoriteManager::getInstance()->isFavoriteUser(u.user))) {
				addLine(Text::toT("*** " + STRING(PARTS) + u.identity.getNick()));
			}
#endif
		} else if(i->first == CONNECTED) {
			addStatus(TSTRING(CONNECTED));
#ifdef PORT_ME
			setTabColor(GREEN);
#endif
		} else if(i->first == DISCONNECTED) {
#ifdef PORT_ME
			clearUserList();
			setTabColor(RED);
#endif
		} else if(i->first == ADD_CHAT_LINE) {
			addChat(Text::toT(static_cast<StringTask*>(i->second)->str));
		} else if(i->first == ADD_STATUS_LINE) {
			addStatus(Text::toT(static_cast<StringTask*>(i->second)->str));
		} else if(i->first == ADD_SILENT_STATUS_LINE) {
			addStatus(Text::toT(static_cast<StringTask*>(i->second)->str), false);
		} else if(i->first == SET_WINDOW_TITLE) {
			setText(Text::toT(static_cast<StringTask*>(i->second)->str));
		} else if(i->first == GET_PASSWORD) {
			if(client->getPassword().size() > 0) {
				client->password(client->getPassword());
				addStatus(TSTRING(STORED_PASSWORD_SENT));
			} else {
				if(!BOOLSETTING(PROMPT_PASSWORD)) {
					message->setText(_T("/password "));
					message->setFocus();
					message->setSelection(10, 10);
					waitingForPW = true;
				} else {
#ifdef PORT_ME
					LineDlg linePwd;
					linePwd.title = CTSTRING(ENTER_PASSWORD);
					linePwd.description = CTSTRING(ENTER_PASSWORD);
					linePwd.password = true;
					if(linePwd.DoModal(m_hWnd) == IDOK) {
						client->setPassword(Text::fromT(linePwd.line));
						client->password(Text::fromT(linePwd.line));
						waitingForPW = false;
					} else {
						client->disconnect(true);
					}
#endif
				}
			}
		} else if(i->first == PRIVATE_MESSAGE) {
#ifdef PORT_ME
			PMTask& pm = *static_cast<PMTask*>(i->second);
			if(pm.hub) {
				if(BOOLSETTING(IGNORE_HUB_PMS)) {
					addStatus(TSTRING(IGNORED_MESSAGE) + Text::toT(pm.str), false);
				} else if(BOOLSETTING(POPUP_HUB_PMS) || PrivateFrame::isOpen(pm.replyTo)) {
					PrivateFrame::gotMessage(pm.from, pm.to, pm.replyTo, Text::toT(pm.str));
				} else {
					addChat(TSTRING(PRIVATE_MESSAGE_FROM) + getNick(pm.from) + _T(": ") + Text::toT(pm.str));
				}
			} else if(pm.bot) {
				if(BOOLSETTING(IGNORE_BOT_PMS)) {
					addStatus(TSTRING(IGNORED_MESSAGE) + Text::toT(pm.str), false);
				} else if(BOOLSETTING(POPUP_BOT_PMS) || PrivateFrame::isOpen(pm.replyTo)) {
					PrivateFrame::gotMessage(pm.from, pm.to, pm.replyTo, Text::toT(pm.str));
				} else {
					addChat(TSTRING(PRIVATE_MESSAGE_FROM) + getNick(pm.from) + _T(": ") + Text::toT(pm.str));
				}
			} else {
				if(BOOLSETTING(POPUP_PMS) || PrivateFrame::isOpen(pm.replyTo) || pm.from == client->getMyIdentity().getUser()) {
					PrivateFrame::gotMessage(pm.from, pm.to, pm.replyTo, Text::toT(pm.str));
				} else {
					addChat(TSTRING(PRIVATE_MESSAGE_FROM) + getNick(pm.from) + _T(": ") + Text::toT(pm.str));
				}
			}
#endif
		}
		delete i->second;
	}
#ifdef PORT_ME
	if(resort && showUsers) {
		ctrlUsers.resort();
		resort = false;
	}

	if(t.size() > 2) {
		ctrlUsers.SetRedraw(TRUE);
	}
#endif
	return 0;
}

bool HubFrame::charred(WidgetTextBoxPtr ptr, int c) {
	///@todo Investigate WM_CHAR vs WM_KEYDOWN
	
	switch(c) {
	case VK_RETURN: return enter();
	default:  return Base::charred(ptr, c);
	}
	
#ifdef PORT_ME
	if(!complete.empty() && wParam != VK_TAB && uMsg == WM_KEYDOWN)
		complete.clear(), inTabComplete = false;

	if (uMsg != WM_KEYDOWN) {
		switch(wParam) {
			case VK_RETURN:
				if( (GetKeyState(VK_CONTROL) & 0x8000) || (GetKeyState(VK_MENU) & 0x8000) ) {
					bHandled = FALSE;
				}
				break;
			case VK_TAB:
				bHandled = TRUE;
				break;
			default:
				bHandled = FALSE;
				break;
		}
		return 0;
	}

	switch(wParam) {
		case VK_TAB:
				onTab();
			break;
		case VK_RETURN:
			if( (GetKeyState(VK_CONTROL) & 0x8000) ||
				(GetKeyState(VK_MENU) & 0x8000) ) {
					bHandled = FALSE;
				} else {
					onEnter();
				}
				break;
		case VK_UP:
			if ( (GetKeyState(VK_MENU) & 0x8000) ||	( ((GetKeyState(VK_CONTROL) & 0x8000) == 0) ^ (BOOLSETTING(USE_CTRL_FOR_LINE_HISTORY) == true) ) ) {
				//scroll up in chat command history
				//currently beyond the last command?
				if (curCommandPosition > 0) {
					//check whether current command needs to be saved
					if (curCommandPosition == prevCommands.size()) {
						auto_ptr<TCHAR> messageContents(new TCHAR[ctrlMessage.GetWindowTextLength()+2]);
						ctrlMessage.GetWindowText(messageContents.get(), ctrlMessage.GetWindowTextLength()+1);
						currentCommand = tstring(messageContents.get());
					}

					//replace current chat buffer with current command
					ctrlMessage.SetWindowText(prevCommands[--curCommandPosition].c_str());
				}
				// move cursor to end of line
				ctrlMessage.SetSel(ctrlMessage.GetWindowTextLength(), ctrlMessage.GetWindowTextLength());
			} else {
				bHandled = FALSE;
			}

			break;
		case VK_DOWN:
			if ( (GetKeyState(VK_MENU) & 0x8000) ||	( ((GetKeyState(VK_CONTROL) & 0x8000) == 0) ^ (BOOLSETTING(USE_CTRL_FOR_LINE_HISTORY) == true) ) ) {
				//scroll down in chat command history

				//currently beyond the last command?
				if (curCommandPosition + 1 < prevCommands.size()) {
					//replace current chat buffer with current command
					ctrlMessage.SetWindowText(prevCommands[++curCommandPosition].c_str());
				} else if (curCommandPosition + 1 == prevCommands.size()) {
					//revert to last saved, unfinished command

					ctrlMessage.SetWindowText(currentCommand.c_str());
					++curCommandPosition;
				}
				// move cursor to end of line
				ctrlMessage.SetSel(ctrlMessage.GetWindowTextLength(), ctrlMessage.GetWindowTextLength());
			} else {
				bHandled = FALSE;
			}

			break;
		case VK_PRIOR: // page up
			ctrlClient.SendMessage(WM_VSCROLL, SB_PAGEUP);

			break;
		case VK_NEXT: // page down
			ctrlClient.SendMessage(WM_VSCROLL, SB_PAGEDOWN);

			break;
		case VK_HOME:
			if (!prevCommands.empty() && (GetKeyState(VK_CONTROL) & 0x8000) || (GetKeyState(VK_MENU) & 0x8000)) {
				curCommandPosition = 0;

				auto_ptr<TCHAR> messageContents(new TCHAR[ctrlMessage.GetWindowTextLength()+2]);
				ctrlMessage.GetWindowText(messageContents.get(), ctrlMessage.GetWindowTextLength()+1);
				currentCommand = tstring(messageContents.get());

				ctrlMessage.SetWindowText(prevCommands[curCommandPosition].c_str());
			} else {
				bHandled = FALSE;
			}

			break;
		case VK_END:
			if ((GetKeyState(VK_CONTROL) & 0x8000) || (GetKeyState(VK_MENU) & 0x8000)) {
				curCommandPosition = prevCommands.size();

				ctrlMessage.SetWindowText(currentCommand.c_str());
			} else {
				bHandled = FALSE;
			}
			break;
		default:
			bHandled = FALSE;
	}
	return 0;
#endif
}


HubFrame::UserTask::UserTask(const OnlineUser& ou) : user(ou.getUser()), identity(ou.getIdentity()) { 

}

HubFrame::PMTask::PMTask(const OnlineUser& from_, const OnlineUser& to_, const OnlineUser& replyTo_, const string& m) : 
	StringTask(m), from(from_.getUser()), to(to_.getUser()), replyTo(replyTo_.getUser()), 
	hub(replyTo_.getIdentity().isHub()), bot(replyTo_.getIdentity().isBot()) 
{

}

void HubFrame::on(Connecting, Client*) throw() {
	speak(ADD_STATUS_LINE, STRING(CONNECTING_TO) + client->getHubUrl() + "...");
	speak(SET_WINDOW_TITLE, client->getHubUrl());
}
void HubFrame::on(Connected, Client*) throw() {
	speak(CONNECTED);
}
void HubFrame::on(UserUpdated, Client*, const OnlineUser& user) throw() {
	speak(UPDATE_USER_JOIN, user);
}
void HubFrame::on(UsersUpdated, Client*, const OnlineUser::List& aList) throw() {
	for(OnlineUser::List::const_iterator i = aList.begin(); i != aList.end(); ++i) {
		tasks.add(UPDATE_USER, new UserTask(*(*i)));
	}
	updateUsers = true;
}

void HubFrame::on(ClientListener::UserRemoved, Client*, const OnlineUser& user) throw() {
	speak(REMOVE_USER, user);
}

void HubFrame::on(Redirect, Client*, const string& line) throw() {
	if(ClientManager::getInstance()->isConnected(line)) {
		speak(ADD_STATUS_LINE, STRING(REDIRECT_ALREADY_CONNECTED));
		return;
	}
#ifdef PORT_ME
	redirect = Text::toT(line);
	if(BOOLSETTING(AUTO_FOLLOW)) {
		PostMessage(WM_COMMAND, IDC_FOLLOW, 0);
	} else {
		speak(ADD_STATUS_LINE, STRING(PRESS_FOLLOW) + line);
	}
#endif
}

void HubFrame::on(Failed, Client*, const string& line) throw() {
	speak(ADD_STATUS_LINE, line);
	speak(DISCONNECTED);
}

void HubFrame::on(GetPassword, Client*) throw() {
	speak(GET_PASSWORD);
}

void HubFrame::on(HubUpdated, Client*) throw() {
	string hubName = client->getHubName();
	if(!client->getHubDescription().empty()) {
		hubName += " - " + client->getHubDescription();
	}
	hubName += " (" + client->getHubUrl() + ")";
#ifdef _DEBUG
	string version = client->getHubIdentity().get("VE");
	if(!version.empty()) {
		hubName += " - " + version;
	}
#endif
	speak(SET_WINDOW_TITLE, hubName);
}

void HubFrame::on(Message, Client*, const OnlineUser& from, const string& msg) throw() {
	speak(ADD_CHAT_LINE, Util::formatMessage(from.getIdentity().getNick(), msg));
}

void HubFrame::on(StatusMessage, Client*, const string& line) throw() {
	if(SETTING(FILTER_MESSAGES)) {
		if((line.find("Hub-Security") != string::npos) && (line.find("was kicked by") != string::npos)) {
			// Do nothing...
		} else if((line.find("is kicking") != string::npos) && (line.find("because:") != string::npos)) {
			speak(ADD_SILENT_STATUS_LINE, Util::toDOS(line));
		} else {
			speak(ADD_CHAT_LINE, Util::toDOS(line));
		}
	} else {
		speak(ADD_CHAT_LINE, Util::toDOS(line));
	}
}

void HubFrame::on(PrivateMessage, Client*, const OnlineUser& from, const OnlineUser& to, const OnlineUser& replyTo, const string& line) throw() {
	speak(from, to, replyTo, Util::formatMessage(from.getIdentity().getNick(), line));
}

void HubFrame::on(NickTaken, Client*) throw() {
	speak(ADD_STATUS_LINE, STRING(NICK_TAKEN));
}

void HubFrame::on(SearchFlood, Client*, const string& line) throw() {
	speak(ADD_STATUS_LINE, STRING(SEARCH_SPAM_FROM) + line);
}

tstring HubFrame::getStatusShared() const {
#ifdef PORT_ME
	if (ctrlUsers.GetSelectedCount() > 1) {
		return ctrlUsers.forEachSelectedT(CountAvailable()).available;
	} else
		return for_each(userMap.begin(), userMap.end(), CountAvailable()).available;
#endif
	return _T("0");
}


tstring HubFrame::getStatusUsers() const {
#ifdef PORT_ME
	size_t userCount = 0;
	for(UserMap::const_iterator i = userMap.begin(); i != userMap.end(); ++i){
		UserInfo* ui = i->second;
		if(!ui->isHidden())
			userCount++;
	}

	tstring textForUsers;
	if (ctrlUsers.GetSelectedCount() > 1)
		textForUsers += Util::toString(ctrlUsers.GetSelectedCount()) + "/";
	if (showUsers && (size_t)ctrlUsers.GetItemCount() < userCount)
		textForUsers += Util::toString(ctrlUsers.GetItemCount()) + "/";
	return textForUsers + Util::toString(userCount) + " " + STRING(HUB_USERS);
#endif
	return _T("Unknown");
}

#ifdef PORT_ME

#include "LineDlg.h"
#include "SearchFrm.h"
#include "PrivateFrame.h"

#include "../client/QueueManager.h"
#include "../client/ShareManager.h"
#include "../client/Util.h"
#include "../client/StringTokenizer.h"
#include "../client/FavoriteManager.h"
#include "../client/LogManager.h"
#include "../client/AdcCommand.h"
#include "../client/ConnectionManager.h"
#include "../client/SearchManager.h"

int HubFrame::columnSizes[] = { 100, 75, 75, 100, 75, 100, 100, 125 };
int HubFrame::columnIndexes[] = { COLUMN_NICK, COLUMN_SHARED, COLUMN_DESCRIPTION, COLUMN_TAG, COLUMN_CONNECTION, COLUMN_IP, COLUMN_EMAIL, COLUMN_CID };
static ResourceManager::Strings columnNames[] = { ResourceManager::NICK, ResourceManager::SHARED,
ResourceManager::DESCRIPTION, ResourceManager::TAG, ResourceManager::CONNECTION, ResourceManager::IP_BARE, ResourceManager::EMAIL, ResourceManager::CID };

LRESULT HubFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	CreateSimpleStatusBar(ATL_IDS_IDLEMESSAGE, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SBARS_SIZEGRIP);
	ctrlStatus.Attach(m_hWndStatusBar);

	ctrlUsers.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
		WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS, WS_EX_CLIENTEDGE, IDC_USERS);
	ctrlUsers.SetExtendedListViewStyle(LVS_EX_LABELTIP | LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT);

	SetSplitterPanes(ctrlClient.m_hWnd, ctrlUsers.m_hWnd, false);
	SetSplitterExtendedStyle(SPLIT_PROPORTIONAL);
	m_nProportionalPos = 7500;

	ctrlShowUsers.Create(ctrlStatus.m_hWnd, rcDefault, _T("+/-"), WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	ctrlShowUsers.SetButtonStyle(BS_AUTOCHECKBOX, false);
	ctrlShowUsers.SetFont(WinUtil::systemFont);
	ctrlShowUsers.SetCheck(showUsers ? BST_CHECKED : BST_UNCHECKED);
	showUsersContainer.SubclassWindow(ctrlShowUsers.m_hWnd);

	WinUtil::splitTokens(columnIndexes, SETTING(HUBFRAME_ORDER), COLUMN_LAST);
	WinUtil::splitTokens(columnSizes, SETTING(HUBFRAME_WIDTHS), COLUMN_LAST);

	for(int j=0; j<COLUMN_LAST; j++) {
		int fmt = (j == COLUMN_SHARED) ? LVCFMT_RIGHT : LVCFMT_LEFT;
		ctrlUsers.InsertColumn(j, CTSTRING_I(columnNames[j]), fmt, columnSizes[j], j);
	}

	ctrlUsers.SetColumnOrderArray(COLUMN_LAST, columnIndexes);

	ctrlUsers.SetBkColor(WinUtil::bgColor);
	ctrlUsers.SetTextBkColor(WinUtil::bgColor);
	ctrlUsers.SetTextColor(WinUtil::textColor);

	ctrlUsers.setSortColumn(COLUMN_NICK);

	ctrlUsers.SetImageList(WinUtil::userImages, LVSIL_SMALL);

	CToolInfo ti(TTF_SUBCLASS, ctrlStatus.m_hWnd);

	ctrlLastLines.Create(ctrlStatus.m_hWnd, rcDefault, NULL, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP, WS_EX_TOPMOST);
	ctrlLastLines.SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	ctrlLastLines.AddTool(&ti);

	userMenu.CreatePopupMenu();
	appendUserItems(userMenu);
	userMenu.AppendMenu(MF_STRING, IDC_COPY_NICK, CTSTRING(COPY_NICK));
	userMenu.SetMenuDefaultItem(IDC_GETLIST);

	tabMenu = CreatePopupMenu();
	tabMenu.AppendMenu(MF_STRING, IDC_ADD_AS_FAVORITE, CTSTRING(ADD_TO_FAVORITES));
	tabMenu.AppendMenu(MF_STRING, ID_FILE_RECONNECT, CTSTRING(MENU_RECONNECT));
	tabMenu.AppendMenu(MF_STRING, IDC_COPY_HUB, CTSTRING(COPY_HUB));

	showJoins = BOOLSETTING(SHOW_JOINS);
	favShowJoins = BOOLSETTING(FAV_SHOW_JOINS);

	for(int j=0; j<COLUMN_LAST; j++) {
		ctrlFilterSel.AddString(CTSTRING_I(columnNames[j]));
	}
	ctrlFilterSel.AddString(CTSTRING(ANY));
	ctrlFilterSel.SetCurSel(COLUMN_LAST);

	bHandled = FALSE;

	FavoriteHubEntry *fhe = FavoriteManager::getInstance()->getFavoriteHubEntry(Text::fromT(server));
	if(fhe != NULL){
		//retrieve window position
		CRect rc(fhe->getLeft(), fhe->getTop(), fhe->getRight(), fhe->getBottom());

		//check that we have a window position stored
		if(! (rc.top == 0 && rc.bottom == 0 && rc.left == 0 && rc.right == 0) )
			MoveWindow(rc, TRUE);
	}

	FavoriteManager::getInstance()->addListener(this);
	TimerManager::getInstance()->addListener(this);

	return 1;
}

LRESULT HubFrame::OnForwardMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	LPMSG pMsg = (LPMSG)lParam;
	if((pMsg->message >= WM_MOUSEFIRST) && (pMsg->message <= WM_MOUSELAST))
		ctrlLastLines.RelayEvent(pMsg);
	return 0;
}

struct CompareItems {
	CompareItems(int aCol) : col(aCol) { }
	bool operator()(const HubFrame::UserInfo& a, const HubFrame::UserInfo& b) const {
		return HubFrame::UserInfo::compareItems(&a, &b, col) < 0;
	}
	const int col;
};

const tstring& HubFrame::getNick(const User::Ptr& aUser) {
	UserMapIter i = userMap.find(aUser);
	if(i == userMap.end())
		return Util::emptyStringT;

	UserInfo* ui = i->second;
	return ui->columns[COLUMN_NICK];
}

void HubFrame::addAsFavorite() {
	FavoriteHubEntry* existingHub = FavoriteManager::getInstance()->getFavoriteHubEntry(client->getHubUrl());
	if(!existingHub) {
		FavoriteHubEntry aEntry;
		TCHAR buf[256];
		this->GetWindowText(buf, 255);
		aEntry.setServer(Text::fromT(server));
		aEntry.setName(Text::fromT(buf));
		aEntry.setDescription(Text::fromT(buf));
		aEntry.setConnect(false);
		aEntry.setNick(client->getMyNick());
		FavoriteManager::getInstance()->addFavorite(aEntry);
		addClientLine(TSTRING(FAVORITE_HUB_ADDED));
	} else {
		addClientLine(TSTRING(FAVORITE_HUB_ALREADY_EXISTS));
	}
}

void HubFrame::removeFavoriteHub() {
	FavoriteHubEntry* removeHub = FavoriteManager::getInstance()->getFavoriteHubEntry(client->getHubUrl());
	if(removeHub) {
		FavoriteManager::getInstance()->removeFavorite(removeHub);
		addClientLine(TSTRING(FAVORITE_HUB_REMOVED));
	} else {
		addClientLine(TSTRING(FAVORITE_HUB_DOES_NOT_EXIST));
	}
}

int HubFrame::getImage(const Identity& u) {
	int image = u.isOp() ? IMAGE_OP : IMAGE_USER;

	if(u.getUser()->isSet(User::DCPLUSPLUS))
		image+=2;
	if(SETTING(INCOMING_CONNECTIONS) == SettingsManager::INCOMING_FIREWALL_PASSIVE && !u.isTcpActive()) {
		// Users we can't connect to...
		image+=4;
	}
	return image;
}

LRESULT HubFrame::onCopyNick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int i=-1;
	string nicks;

	while( (i = ctrlUsers.GetNextItem(i, LVNI_SELECTED)) != -1) {
		nicks += ctrlUsers.getItemData(i)->getNick();
		nicks += ' ';
	}
	if(!nicks.empty()) {
		// remove last space
		nicks.erase(nicks.length() - 1);
		WinUtil::setClipboard(Text::toT(nicks));
	}
	return 0;
}

LRESULT HubFrame::onCopyHub(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	WinUtil::setClipboard(Text::toT(client->getHubUrl()));
	return 0;
}

LRESULT HubFrame::onDoubleClickUsers(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NMITEMACTIVATE* item = (NMITEMACTIVATE*)pnmh;
	if(item->iItem != -1) {
		ctrlUsers.getItemData(item->iItem)->getList();
	}
	return 0;
}


bool HubFrame::updateUser(const UserTask& u) {
	UserMapIter i = userMap.find(u.user);
	if(i == userMap.end()) {
		UserInfo* ui = new UserInfo(u);
		userMap.insert(make_pair(u.user, ui));
		if(!ui->isHidden() && showUsers)
			ctrlUsers.insertItem(ui, getImage(u.identity));

		if(!filter.empty())
			updateUserList(ui);

		return true;
	} else {
		UserInfo* ui = i->second;
		if(!ui->isHidden() && u.identity.isHidden() && showUsers) {
			ctrlUsers.deleteItem(ui);
		}

		resort = ui->update(u.identity, ctrlUsers.getSortColumn()) || resort;
		if(showUsers) {
			int pos = ctrlUsers.findItem(ui);
			if(pos != -1) {
				ctrlUsers.updateItem(pos);
				ctrlUsers.SetItem(pos, 0, LVIF_IMAGE, NULL, getImage(u.identity), 0, 0, NULL);
			}

			updateUserList(ui);
		}

		return false;
	}
}

void HubFrame::removeUser(const User::Ptr& aUser) {
	UserMapIter i = userMap.find(aUser);
	if(i == userMap.end()) {
		// Should never happen?
		dcassert(i != userMap.end());
		return;
	}

	UserInfo* ui = i->second;
	if(!ui->isHidden() && showUsers)
		ctrlUsers.deleteItem(ui);

	userMap.erase(i);
	delete ui;
}

HubFrame::UserInfo* HubFrame::findUser(const tstring& nick) {
	for(UserMapIter i = userMap.begin(); i != userMap.end(); ++i) {
		if(i->second->columns[COLUMN_NICK] == nick)
			return i->second;
	}
	return 0;
}

bool HubFrame::UserInfo::update(const Identity& identity, int sortCol) {
	bool needsSort = (getIdentity().isOp() != identity.isOp());
	tstring old;
	if(sortCol != -1)
		old = columns[sortCol];

	columns[COLUMN_NICK] = Text::toT(identity.getNick());
	columns[COLUMN_SHARED] = Text::toT(Util::formatBytes(identity.getBytesShared()));
	columns[COLUMN_DESCRIPTION] = Text::toT(identity.getDescription());
	columns[COLUMN_TAG] = Text::toT(identity.getTag());
	columns[COLUMN_CONNECTION] = Text::toT(identity.getConnection());
	string ip = identity.getIp();
	string country = ip.empty()?Util::emptyString:Util::getIpCountry(ip);
	if (!country.empty())
		ip = country + " (" + ip + ")";
	columns[COLUMN_IP] = Text::toT(ip);
	columns[COLUMN_EMAIL] = Text::toT(identity.getEmail());
	columns[COLUMN_CID] = Text::toT(identity.getUser()->getCID().toBase32());

	if(sortCol != -1) {
		needsSort = needsSort || (old != columns[sortCol]);
	}

	setIdentity(identity);
	return needsSort;
}

static const COLORREF RED = RGB(255, 0, 0);
static const COLORREF GREEN = RGB(0, 255, 0);

void HubFrame::clearUserList() {
	ctrlUsers.DeleteAllItems();
	for(UserMapIter i = userMap.begin(); i != userMap.end(); ++i) {
		delete i->second;
	}
	userMap.clear();
}

void HubFrame::clearTaskList() {
	tasks.clear();
}

LRESULT HubFrame::onLButton(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	HWND focus = GetFocus();
	bHandled = false;
	if(focus == ctrlClient.m_hWnd) {
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		tstring x;
		tstring::size_type start = (tstring::size_type)WinUtil::textUnderCursor(pt, ctrlClient, x);
		tstring::size_type end = x.find(_T(" "), start);

		if(end == tstring::npos)
			end = x.length();

		bHandled = WinUtil::parseDBLClick(x, start, end);
		if (!bHandled) {
			string::size_type end = x.find_first_of(_T(" >\t"), start+1);

			if(end == tstring::npos) // get EOL as well
				end = x.length();
			else if(end == start + 1)
				return 0;

			// Nickname click, let's see if we can find one like it in the name list...
			tstring nick = x.substr(start, end - start);
			UserInfo* ui = findUser(nick);
			if(ui) {
				bHandled = true;
				if (wParam & MK_CONTROL) { // MK_CONTROL = 0x0008
					PrivateFrame::openWindow(ui->user);
				} else if (wParam & MK_SHIFT) {
					try {
						QueueManager::getInstance()->addList(ui->user, QueueItem::FLAG_CLIENT_VIEW);
					} catch(const Exception& e) {
						addClientLine(Text::toT(e.getError()));
					}
				} else if(showUsers) {
					int items = ctrlUsers.GetItemCount();
					int pos = -1;
					ctrlUsers.SetRedraw(FALSE);
					for(int i = 0; i < items; ++i) {
						if(ctrlUsers.getItemData(i) == ui)
							pos = i;
						ctrlUsers.SetItemState(i, (i == pos) ? LVIS_SELECTED | LVIS_FOCUSED : 0, LVIS_SELECTED | LVIS_FOCUSED);
					}
					ctrlUsers.SetRedraw(TRUE);
					ctrlUsers.EnsureVisible(pos, FALSE);
				}
			}
		}
	}
	return 0;
}

LRESULT HubFrame::onTabContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };		// location of mouse click
	tabMenuShown = true;
	prepareMenu(tabMenu, ::UserCommand::CONTEXT_HUB, client->getHubUrl());
	tabMenu.AppendMenu(MF_SEPARATOR);
	tabMenu.AppendMenu(MF_STRING, IDC_CLOSE_WINDOW, CTSTRING(CLOSE));
	tabMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_BOTTOMALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
	tabMenu.DeleteMenu(tabMenu.GetMenuItemCount()-1, MF_BYPOSITION);
	tabMenu.DeleteMenu(tabMenu.GetMenuItemCount()-1, MF_BYPOSITION);
	cleanMenu(tabMenu);
	return TRUE;
}

LRESULT HubFrame::onCtlColor(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	HWND hWnd = (HWND)lParam;
	HDC hDC = (HDC)wParam;
	if(hWnd == ctrlClient.m_hWnd || hWnd == ctrlMessage.m_hWnd ||
		hWnd == ctrlFilter.m_hWnd || hWnd == ctrlFilterSel.m_hWnd) {
		::SetBkColor(hDC, WinUtil::bgColor);
		::SetTextColor(hDC, WinUtil::textColor);
		return (LRESULT)WinUtil::bgBrush;
	} else {
		return 0;
	}
}

LRESULT HubFrame::onContextMenu(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

	bool doMenu = false;

	if(reinterpret_cast<HWND>(wParam) == ctrlClient) {
		if(pt.x == -1 && pt.y == -1) {
			WinUtil::getContextMenuPos(ctrlClient, pt);
		}

		tstring x;
		ctrlClient.ScreenToClient(&pt);
		tstring::size_type start = (tstring::size_type)WinUtil::textUnderCursor(pt, ctrlClient, x);
		ctrlClient.ClientToScreen(&pt);

		tstring::size_type end = x.find_first_of(_T(" >\t"), start+1);
		if(end == string::npos) // get EOL as well
			end = x.length();
		else if(end == start + 1) {
			bHandled = FALSE;
			return FALSE;
		}

		// Nickname click, let's see if we can find one like it in the name list...

		int pos = ctrlUsers.findItem(x.substr(start, end - start));
		if(pos != -1) {
			int items = ctrlUsers.GetItemCount();
			ctrlUsers.SetRedraw(FALSE);
			for(int i = 0; i < items; ++i) {
				ctrlUsers.SetItemState(i, (i == pos) ? LVIS_SELECTED | LVIS_FOCUSED : 0, LVIS_SELECTED | LVIS_FOCUSED);
			}
			ctrlUsers.SetRedraw(TRUE);
			ctrlUsers.EnsureVisible(pos, FALSE);

			doMenu = true;
		}
	}

	if((doMenu || (reinterpret_cast<HWND>(wParam) == ctrlUsers)) && ctrlUsers.GetSelectedCount() > 0) {
		if(pt.x == -1 && pt.y == -1) {
			WinUtil::getContextMenuPos(ctrlUsers, pt);
		}

		tabMenuShown = false;
		prepareMenu(userMenu, ::UserCommand::CONTEXT_CHAT, client->getHubUrl());
		checkAdcItems(userMenu);
		userMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
		cleanMenu(userMenu);
		return TRUE;
	} else {
		bHandled = FALSE; //needed to popup context menu under userlist
	}
	return FALSE;
}

void HubFrame::runUserCommand(::UserCommand& uc) {
	if(!WinUtil::getUCParams(m_hWnd, uc, ucLineParams))
		return;

	StringMap ucParams = ucLineParams;

	client->getMyIdentity().getParams(ucParams, "my", true);
	client->getHubIdentity().getParams(ucParams, "hub", false);

	if(tabMenuShown) {
		client->escapeParams(ucParams);
		client->sendUserCmd(Util::formatParams(uc.getCommand(), ucParams, false));
	} else {
		int sel = -1;
		while((sel = ctrlUsers.GetNextItem(sel, LVNI_SELECTED)) != -1) {
			UserInfo* u = ctrlUsers.getItemData(sel);
			StringMap tmp = ucParams;

			u->getIdentity().getParams(tmp, "user", true);
			client->escapeParams(tmp);
			client->sendUserCmd(Util::formatParams(uc.getCommand(), tmp, false));
		}
	}
}

string HubFrame::stripNick(const string& nick) const {
	if (nick.substr(0, 1) != "[") return nick;
	string::size_type x = nick.find(']');
	string ret;
	// Avoid full deleting of [IMCOOL][CUSIHAVENOTHINGELSETHANBRACKETS]-type nicks
	if ((x != string::npos) && (nick.substr(x+1).length() > 0)) {
		ret = nick.substr(x+1);
	} else {
		ret = nick;
	}
	return ret;
}

//Has fun side-effects. Otherwise needs reference arguments or multiple-return-values.
tstring HubFrame::scanNickPrefix(const tstring& prefixT) {
	string prefix = Text::fromT(prefixT), maxPrefix;
	tabCompleteNicks.clear();
	for (UserMap::const_iterator i = userMap.begin(); i != userMap.end(); ++i) {
		string prevNick, nick = i->second->getIdentity().getNick(), wholeNick = nick;

		do {
			string::size_type lp = prefix.size(), ln = nick.size();
			if ((ln >= lp) && (!Util::strnicmp(nick, prefix, lp))) {
				if (maxPrefix == Util::emptyString) maxPrefix = nick;	//ugly hack
				tabCompleteNicks.push_back(nick);
				tabCompleteNicks.push_back(wholeNick);
				maxPrefix = maxPrefix.substr(0, mismatch(maxPrefix.begin(),
					maxPrefix.begin()+min(maxPrefix.size(), nick.size()),
					nick.begin(), compareCharsNoCase).first - maxPrefix.begin());
			}

			prevNick = nick;
			nick = stripNick(nick);
		} while (prevNick != nick);
	}

	return Text::toT(maxPrefix);
}

void HubFrame::onTab() {
	if(ctrlMessage.GetWindowTextLength() == 0) {
		handleTab(WinUtil::isShift());
		return;
	}

	HWND focus = GetFocus();
	if( (focus == ctrlMessage.m_hWnd) && !WinUtil::isShift() )
	{
		int n = ctrlMessage.GetWindowTextLength();
		AutoArray<TCHAR> buf(n+1);
		ctrlMessage.GetWindowText(buf, n+1);
		tstring text(buf, n);
		string::size_type textStart = text.find_last_of(_T(" \n\t"));

		if(complete.empty()) {
			if(textStart != string::npos) {
				complete = text.substr(textStart + 1);
			} else {
				complete = text;
			}
			if(complete.empty()) {
				// Still empty, no text entered...
				ctrlUsers.SetFocus();
				return;
			}
			int y = ctrlUsers.GetItemCount();

			for(int x = 0; x < y; ++x)
				ctrlUsers.SetItemState(x, 0, LVNI_FOCUSED | LVNI_SELECTED);
		}

		if(textStart == string::npos)
			textStart = 0;
		else
			textStart++;

		if (inTabComplete) {
			// Already pressed tab once. Output nick candidate list.
			tstring nicks;
			for (StringList::const_iterator i = tabCompleteNicks.begin(); i < tabCompleteNicks.end(); i+=2)
				nicks.append(Text::toT(*i + " "));
			addClientLine(nicks);
			inTabComplete = false;
		} else {
			// First tab. Maximally extend proposed nick.
			tstring nick = scanNickPrefix(complete);
			if (tabCompleteNicks.empty()) return;

			// Maybe it found a unique match. If userlist showing, highlight.
			if (showUsers && tabCompleteNicks.size() == 2) {
				int i = ctrlUsers.findItem(Text::toT(tabCompleteNicks[1]));
				ctrlUsers.SetItemState(i, LVNI_FOCUSED | LVNI_SELECTED, LVNI_FOCUSED | LVNI_SELECTED);
				ctrlUsers.EnsureVisible(i, FALSE);
			}

			ctrlMessage.SetSel(textStart, ctrlMessage.GetWindowTextLength(), TRUE);
			// no shift, use partial nick when appropriate
			if(GetAsyncKeyState(VK_SHIFT) & 0x8000) {
				ctrlMessage.ReplaceSel(nick.c_str());
			} else {
				ctrlMessage.ReplaceSel(Text::toT(stripNick(Text::fromT(nick))).c_str());
			}

			inTabComplete = true;
		}
	}
}

LRESULT HubFrame::onFileReconnect(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	client->reconnect();
	return 0;
}

LRESULT HubFrame::onShowUsers(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled) {
	bHandled = FALSE;
	if((wParam == BST_CHECKED)) {
		showUsers = true;
		ctrlUsers.SetRedraw(FALSE);
		ctrlUsers.DeleteAllItems();

		for(UserMapIter i = userMap.begin(); i != userMap.end(); ++i) {
			UserInfo* ui = i->second;
			if(!ui->isHidden())
				ctrlUsers.insertItem(ui, getImage(ui->getIdentity()));
		}

		ctrlUsers.SetRedraw(TRUE);
		ctrlUsers.resort();
	} else {
		showUsers = false;
		ctrlUsers.DeleteAllItems();
	}

	SettingsManager::getInstance()->set(SettingsManager::GET_USER_INFO, showUsers);

	UpdateLayout(FALSE);
	return 0;
}

LRESULT HubFrame::onFollow(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(!redirect.empty()) {
		if(ClientManager::getInstance()->isConnected(Text::fromT(redirect))) {
			addClientLine(TSTRING(REDIRECT_ALREADY_CONNECTED));
			return 0;
		}

		dcassert(frames.find(server) != frames.end());
		dcassert(frames[server] == this);
		frames.erase(server);
		server = redirect;
		frames[server] = this;

		// the client is dead, long live the client!
		client->removeListener(this);
		ClientManager::getInstance()->putClient(client);
		clearUserList();
		clearTaskList();
		client = ClientManager::getInstance()->getClient(Text::fromT(server));
		client->addListener(this);
		client->connect();
	}
	return 0;
}

LRESULT HubFrame::onEnterUsers(int /*idCtrl*/, LPNMHDR /* pnmh */, BOOL& /*bHandled*/) {
	int item = ctrlUsers.GetNextItem(-1, LVNI_FOCUSED);
	if(item != -1) {
		try {
			QueueManager::getInstance()->addList((ctrlUsers.getItemData(item))->user, QueueItem::FLAG_CLIENT_VIEW);
		} catch(const Exception& e) {
			addClientLine(Text::toT(e.getError()));
		}
	}
	return 0;
}

LRESULT HubFrame::onGetToolTip(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NMTTDISPINFO* nm = (NMTTDISPINFO*)pnmh;
	lastLines.clear();
	for(TStringIter i = lastLinesList.begin(); i != lastLinesList.end(); ++i) {
		lastLines += *i;
		lastLines += _T("\r\n");
	}
	if(lastLines.size() > 2) {
		lastLines.erase(lastLines.size() - 2);
	}
	nm->lpszText = const_cast<TCHAR*>(lastLines.c_str());
	return 0;
}

void HubFrame::resortUsers() {
	for(FrameIter i = frames.begin(); i != frames.end(); ++i)
		i->second->resortForFavsFirst(true);
}

void HubFrame::closeDisconnected() {
	for(FrameIter i=frames.begin(); i!= frames.end(); ++i) {
		if (!(i->second->client->isConnected())) {
			i->second->PostMessage(WM_CLOSE);
		}
	}
}

void HubFrame::on(FavoriteManagerListener::UserAdded, const FavoriteUser& /*aUser*/) throw() {
	resortForFavsFirst();
}
void HubFrame::on(FavoriteManagerListener::UserRemoved, const FavoriteUser& /*aUser*/) throw() {
	resortForFavsFirst();
}

void HubFrame::resortForFavsFirst(bool justDoIt /* = false */) {
	if(justDoIt || BOOLSETTING(SORT_FAVUSERS_FIRST)) {
		resort = true;
		PostMessage(WM_SPEAKER);
	}
}

void HubFrame::on(Second, uint32_t /*aTick*/) throw() {
	if(updateUsers) {
		updateUsers = false;
		PostMessage(WM_SPEAKER);
	}
}

LRESULT HubFrame::onFilterChar(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled) {
	if(uMsg == WM_CHAR && wParam == VK_TAB) {
		handleTab(WinUtil::isShift());
		return 0;
	}

	TCHAR *buf = new TCHAR[ctrlFilter.GetWindowTextLength()+1];
	ctrlFilter.GetWindowText(buf, ctrlFilter.GetWindowTextLength()+1);
	filter = buf;
	delete[] buf;

	updateUserList();

	bHandled = FALSE;

	return 0;
}

LRESULT HubFrame::onSelChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled) {
	TCHAR *buf = new TCHAR[ctrlFilter.GetWindowTextLength()+1];
	ctrlFilter.GetWindowText(buf, ctrlFilter.GetWindowTextLength()+1);
	filter = buf;
	delete[] buf;

	updateUserList();

	bHandled = FALSE;

	return 0;
}

bool HubFrame::parseFilter(FilterModes& mode, int64_t& size) {
	tstring::size_type start = static_cast<tstring::size_type>(tstring::npos);
	tstring::size_type end = static_cast<tstring::size_type>(tstring::npos);
	int64_t multiplier = 1;

	if(filter.empty()) {
		return false;
	}
	if(filter.compare(0, 2, _T(">=")) == 0) {
		mode = GREATER_EQUAL;
		start = 2;
	} else if(filter.compare(0, 2, _T("<=")) == 0) {
		mode = LESS_EQUAL;
		start = 2;
	} else if(filter.compare(0, 2, _T("==")) == 0) {
		mode = EQUAL;
		start = 2;
	} else if(filter.compare(0, 2, _T("!=")) == 0) {
		mode = NOT_EQUAL;
		start = 2;
	} else if(filter[0] == _T('<')) {
		mode = LESS;
		start = 1;
	} else if(filter[0] == _T('>')) {
		mode = GREATER;
		start = 1;
	} else if(filter[0] == _T('=')) {
		mode = EQUAL;
		start = 1;
	}

	if(start == tstring::npos)
		return false;
	if(filter.length() <= start)
		return false;

	if((end = Util::findSubString(filter, _T("TiB"))) != tstring::npos) {
		multiplier = 1024LL * 1024LL * 1024LL * 1024LL;
	} else if((end = Util::findSubString(filter, _T("GiB"))) != tstring::npos) {
		multiplier = 1024*1024*1024;
	} else if((end = Util::findSubString(filter, _T("MiB"))) != tstring::npos) {
		multiplier = 1024*1024;
	} else if((end = Util::findSubString(filter, _T("KiB"))) != tstring::npos) {
		multiplier = 1024;
	} else if((end = Util::findSubString(filter, _T("TB"))) != tstring::npos) {
		multiplier = 1000LL * 1000LL * 1000LL * 1000LL;
	} else if((end = Util::findSubString(filter, _T("GB"))) != tstring::npos) {
		multiplier = 1000*1000*1000;
	} else if((end = Util::findSubString(filter, _T("MB"))) != tstring::npos) {
		multiplier = 1000*1000;
	} else if((end = Util::findSubString(filter, _T("kB"))) != tstring::npos) {
		multiplier = 1000;
	} else if((end = Util::findSubString(filter, _T("B"))) != tstring::npos) {
		multiplier = 1;
	}

	if(end == tstring::npos) {
		end = filter.length();
	}

	tstring tmpSize = filter.substr(start, end-start);
	size = static_cast<int64_t>(Util::toDouble(Text::fromT(tmpSize)) * multiplier);

	return true;
}

void HubFrame::updateUserList(UserInfo* ui) {
	int64_t size = -1;
	FilterModes mode = NONE;

	int sel = ctrlFilterSel.GetCurSel();

	bool doSizeCompare = parseFilter(mode, size) && sel == COLUMN_SHARED;

	//single update?
	//avoid refreshing the whole list and just update the current item
	//instead
	if(ui != NULL) {
		if(ui->isHidden()) {
			return;
		}
		if(filter.empty()) {
			if(ctrlUsers.findItem(ui) == -1) {
				ctrlUsers.insertItem(ui, getImage(ui->getIdentity()));
			}
		} else {
			if(matchFilter(*ui, sel, doSizeCompare, mode, size)) {
				if(ctrlUsers.findItem(ui) == -1) {
					ctrlUsers.insertItem(ui, getImage(ui->getIdentity()));
				}
			} else {
				//deleteItem checks to see that the item exists in the list
				//unnecessary to do it twice.
				ctrlUsers.deleteItem(ui);
			}
		}
	} else {
		ctrlUsers.SetRedraw(FALSE);
		ctrlUsers.DeleteAllItems();

		if(filter.empty()) {
			for(UserMapIter i = userMap.begin(); i != userMap.end(); ++i){
				UserInfo* ui = i->second;
				if(!ui->isHidden())
					ctrlUsers.insertItem(i->second, getImage(i->second->getIdentity()));
			}
		} else {
			for(UserMapIter i = userMap.begin(); i != userMap.end(); ++i) {
				UserInfo* ui = i->second;
				if(!ui->isHidden() && matchFilter(*ui, sel, doSizeCompare, mode, size)) {
					ctrlUsers.insertItem(ui, getImage(ui->getIdentity()));
				}
			}
		}
		ctrlUsers.SetRedraw(TRUE);
	}
}

void HubFrame::handleTab(bool reverse) {
	HWND focus = GetFocus();

	if(reverse) {
		if(focus == ctrlFilterSel.m_hWnd) {
			ctrlFilter.SetFocus();
		} else if(focus == ctrlFilter.m_hWnd) {
			ctrlMessage.SetFocus();
		} else if(focus == ctrlMessage.m_hWnd) {
			ctrlUsers.SetFocus();
		} else if(focus == ctrlUsers.m_hWnd) {
			ctrlClient.SetFocus();
		} else if(focus == ctrlClient.m_hWnd) {
			ctrlFilterSel.SetFocus();
		}
	} else {
		if(focus == ctrlClient.m_hWnd) {
			ctrlUsers.SetFocus();
		} else if(focus == ctrlUsers.m_hWnd) {
			ctrlMessage.SetFocus();
		} else if(focus == ctrlMessage.m_hWnd) {
			ctrlFilter.SetFocus();
		} else if(focus == ctrlFilter.m_hWnd) {
			ctrlFilterSel.SetFocus();
		} else if(focus == ctrlFilterSel.m_hWnd) {
			ctrlClient.SetFocus();
		}
	}
}

bool HubFrame::matchFilter(const UserInfo& ui, int sel, bool doSizeCompare, FilterModes mode, int64_t size) {

	if(filter.empty())
		return true;

	bool insert = false;
	if(doSizeCompare) {
		switch(mode) {
			case EQUAL: insert = (size == ui.getIdentity().getBytesShared()); break;
			case GREATER_EQUAL: insert = (size <= ui.getIdentity().getBytesShared()); break;
			case LESS_EQUAL: insert = (size >= ui.getIdentity().getBytesShared()); break;
			case GREATER: insert = (size < ui.getIdentity().getBytesShared()); break;
			case LESS: insert = (size > ui.getIdentity().getBytesShared()); break;
			case NOT_EQUAL: insert = (size != ui.getIdentity().getBytesShared()); break;
		}
	} else {
		if(sel >= COLUMN_LAST) {
			for(int i = COLUMN_FIRST; i < COLUMN_LAST; ++i) {
				if(Util::findSubString(ui.getText(i), filter) != string::npos) {
					insert = true;
					break;
				}
			}
		} else {
			if(Util::findSubString(ui.getText(sel), filter) != string::npos)
				insert = true;
		}
	}

	return insert;
}

#endif

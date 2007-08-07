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

#include "HubFrame.h"
#include "PrivateFrame.h"
#include "LineDlg.h"
#include "HoldRedraw.h"

#include <dcpp/ClientManager.h>
#include <dcpp/Client.h>
#include <dcpp/LogManager.h>
#include <dcpp/User.h>
#include <dcpp/ResourceManager.h>
#include <dcpp/FavoriteManager.h>
#include <dcpp/ConnectionManager.h>
#include <dcpp/SearchManager.h>

int HubFrame::columnSizes[] = { 100, 75, 75, 100, 75, 100, 100, 125 };
int HubFrame::columnIndexes[] = { COLUMN_NICK, COLUMN_SHARED, COLUMN_DESCRIPTION, COLUMN_TAG, COLUMN_CONNECTION, COLUMN_IP, COLUMN_EMAIL, COLUMN_CID };
static ResourceManager::Strings columnNames[] = { ResourceManager::NICK, ResourceManager::SHARED,
ResourceManager::DESCRIPTION, ResourceManager::TAG, ResourceManager::CONNECTION, ResourceManager::IP_BARE, ResourceManager::EMAIL, ResourceManager::CID };

HubFrame::FrameList HubFrame::frames;

void HubFrame::closeDisconnected() {
	for(FrameIter i=frames.begin(); i!= frames.end(); ++i) {
		HubFrame* frame = *i;
		if (!(frame->client->isConnected())) {
			::PostMessage(frame->handle(), WM_CLOSE, 0, 0);
		}
	}
}

void HubFrame::openWindow(SmartWin::WidgetMDIParent* mdiParent, const string& url) {
	for(FrameIter i = frames.begin(); i!= frames.end(); ++i) {
		HubFrame* frame = *i;
		if(frame->url == url) {
			frame->activate();
			return;
		}
	}

	new HubFrame(mdiParent, url);
}

HubFrame::HubFrame(SmartWin::WidgetMDIParent* mdiParent, const string& url_) : 
	BaseType(mdiParent, Text::toT(url_), SmartWin::IconPtr(new SmartWin::Icon(IDR_HUB))),
	chat(0),
	message(0),
	filter(0),
	filterType(0),
	paned(0),
	showUsers(0),
	users(0),
	client(0),
	url(url_),
	timeStamps(BOOLSETTING(TIME_STAMPS)), 
	updateUsers(false), 
	waitingForPW(false), 
	resort(false),
	showJoins(BOOLSETTING(SHOW_JOINS)),
	favShowJoins(BOOLSETTING(FAV_SHOW_JOINS)),
	curCommandPosition(0),
	inTabMenu(false),
	inTabComplete(false)
{
	paned = createVPaned();
	paned->setRelativePos(0.7);

	{
		WidgetTextBox::Seed cs;
		cs.style = WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_AUTOVSCROLL | ES_MULTILINE;
		cs.exStyle = WS_EX_CLIENTEDGE;
		message = createTextBox(cs);
		message->setFont(WinUtil::font);
		addWidget(message);
		message->onKeyDown(std::tr1::bind(&HubFrame::handleMessageKeyDown, this, _1));
		message->onChar(std::tr1::bind(&HubFrame::handleMessageChar, this, _1));
	}
	
	{
		WidgetTextBox::Seed cs;
		cs.style = WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_NOHIDESEL | ES_READONLY;
		cs.exStyle = WS_EX_CLIENTEDGE;
		chat = createTextBox(cs);
		chat->setTextLimit(0);
		chat->setFont(WinUtil::font);
		addWidget(chat);
		paned->setFirst(chat);
	}
	
	{
		WidgetTextBox::Seed cs;
		cs.style = WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE;
		cs.exStyle = WS_EX_CLIENTEDGE;
		filter = createTextBox(cs);
		filter->setFont(WinUtil::font);
		addWidget(filter);
		filter->onTextChanging(std::tr1::bind(&HubFrame::updateFilter, this, _1));
	}
	{
		WidgetComboBox::Seed cs;
		
		cs.style = WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | CBS_DROPDOWNLIST;
		cs.exStyle =  WS_EX_CLIENTEDGE;
		filterType = createComboBox(cs);
		filterType->setFont(WinUtil::font);
		addWidget(filterType);
		
		for(int j=0; j<COLUMN_LAST; j++) {
			filterType->addValue(TSTRING_I(columnNames[j]));
		}
		filterType->addValue(CTSTRING(ANY));
		filterType->setSelectedIndex(COLUMN_LAST);
		filterType->onSelectionChanged(std::tr1::bind(&HubFrame::updateUserList, this, (UserInfo*)0));
	}
	
	{
		WidgetUsers::Seed cs;
		
		cs.style = WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS;
		cs.exStyle =  WS_EX_CLIENTEDGE;
		users = SmartWin::WidgetCreator<WidgetUsers>::create(this, cs);
		users->setListViewStyle(LVS_EX_LABELTIP | LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT);
		users->setFont(WinUtil::font);
		addWidget(users);
		paned->setSecond(users);
		
		users->setSmallImageList(WinUtil::userImages);
		users->createColumns(ResourceManager::getInstance()->getStrings(columnNames));
		users->setColumnOrder(WinUtil::splitTokens(SETTING(HUBFRAME_ORDER), columnIndexes));
		users->setColumnWidths(WinUtil::splitTokens(SETTING(HUBFRAME_WIDTHS), columnSizes));
		users->setColor(WinUtil::textColor, WinUtil::bgColor);
		users->setSortColumn(COLUMN_NICK);
		
		users->onSelectionChanged(std::tr1::bind(&HubFrame::updateStatus, this));
		users->onDblClicked(std::tr1::bind(&HubFrame::handleDoubleClickUsers, this));
		users->onKeyDown(std::tr1::bind(&HubFrame::handleUsersKeyDown, this, _1));
	}
	
	{
		WidgetCheckBox::Seed cs;
		cs.caption = _T("+/-");
		showUsers = createCheckBox(cs);
		showUsers->setChecked(BOOLSETTING(GET_USER_INFO));
	}

	initStatus();
	///@todo get real resizer width
	statusSizes[STATUS_SHOW_USERS] = 16;
	statusSizes[STATUS_DUMMY] = 16;

	layout();
	
	initSecond();
	
	onSpeaker(std::tr1::bind(&HubFrame::handleSpeaker, this, _1, _2));
	onRaw(std::tr1::bind(&HubFrame::handleContextMenu, this, _1, _2), SmartWin::Message(WM_CONTEXTMENU));
	onTabContextMenu(std::tr1::bind(&HubFrame::handleTabContextMenu, this, _1));
	
	client = ClientManager::getInstance()->getClient(url);
	client->addListener(this);
	client->connect();
	
	frames.push_back(this);
	
	showUsers->onClicked(std::tr1::bind(&HubFrame::handleShowUsersClicked, this));

	BOOL max = FALSE;
	if(this->getParent()->sendMessage(WM_MDIGETACTIVE, 0, reinterpret_cast<LPARAM>(&max)) && !max) {
		FavoriteHubEntry *fhe = FavoriteManager::getInstance()->getFavoriteHubEntry(url);
		if(fhe != NULL){
			//retrieve window position
			SmartWin::Rectangle rc(fhe->getLeft(), fhe->getTop(), fhe->getRight() - fhe->getLeft(), fhe->getBottom() - fhe->getTop());
	
			//check that we have a window position stored
			if(rc.pos.x >= 0 && rc.pos.y >= 0 && rc.size.x > 0 && rc.size.y > 0) {
				setBounds(rc);
			}
		}
	}
	FavoriteManager::getInstance()->addListener(this);
}

HubFrame::~HubFrame() {
	ClientManager::getInstance()->putClient(client);
	frames.erase(std::remove(frames.begin(), frames.end(), this), frames.end());
	clearTaskList();
}

bool HubFrame::preClosing() {
	FavoriteManager::getInstance()->removeListener(this);
	client->removeListener(this);
	client->disconnect(true);
	return true;
}

void HubFrame::postClosing() {
	SettingsManager::getInstance()->set(SettingsManager::GET_USER_INFO, showUsers->getChecked());

	clearUserList();
	clearTaskList();

	SettingsManager::getInstance()->set(SettingsManager::HUBFRAME_ORDER, WinUtil::toString(users->getColumnOrder()));
	SettingsManager::getInstance()->set(SettingsManager::HUBFRAME_WIDTHS, WinUtil::toString(users->getColumnWidths()));
	
	FavoriteHubEntry *fhe = FavoriteManager::getInstance()->getFavoriteHubEntry(url);
	if(fhe != NULL && !this->isIconic()){
		//Get position of window

		//convert the position so it's relative to main window
		SmartWin::Point pos = getPosition();
		SmartWin::Point size = getSize();

		//save the position
		fhe->setBottom((uint16_t)std::max(pos.y + size.y, 0L));
		fhe->setTop((uint16_t)std::max(pos.y, 0L));
		fhe->setLeft((uint16_t)std::max(pos.x, 0L));
		fhe->setRight((uint16_t)std::max(pos.x + size.x, 0L));

		FavoriteManager::getInstance()->save();
	}
}

void HubFrame::layout() {
	const int border = 2;
	
	SmartWin::Rectangle r(getClientAreaSize()); 

	SmartWin::Rectangle rs = layoutStatus();
	mapWidget(STATUS_SHOW_USERS, showUsers);
	
	r.size.y -= rs.size.y + border;
	int ymessage = message->getTextSize(_T("A")).y + 10;
	int xfilter = showUsers->getChecked() ? std::min(r.size.x / 4, 200l) : 0;
	SmartWin::Rectangle rm(0, r.size.y - ymessage, r.size.x - xfilter, ymessage);
	message->setBounds(rm);
	
	rm.pos.x += rm.size.x + border;
	rm.size.x = showUsers->getChecked() ? xfilter * 2 / 3 - border : 0;
	filter->setBounds(rm);
	
	rm.pos.x += rm.size.x + border;
	rm.size.x = showUsers->getChecked() ? xfilter / 3 - border : 0;
	filterType->setBounds(rm);
	
	r.size.y -= rm.size.y + border;

	bool checked = showUsers->getChecked();
	if(checked && !paned->getSecond()) {
		paned->setSecond(users);
	} else if(!checked && paned->getSecond()) {
		paned->setSecond(0);
	}
	paned->setRect(r);
}

void HubFrame::updateStatus() {
	setStatus(STATUS_USERS, getStatusUsers());
	setStatus(STATUS_SHARED, getStatusShared());
}

void HubFrame::initSecond() {
	createTimer(std::tr1::bind(&HubFrame::eachSecond, this), 1000);
}

bool HubFrame::eachSecond() {
	if(updateUsers) {
		updateUsers = false;
		speak();
	}
	
	updateStatus();
	return true;
}

bool HubFrame::enter() {
	if(isShiftPressed() || isControlPressed() || isAltPressed()) {
		return false;
	}
	tstring s = message->getText();
	if(s.empty()) {
		::MessageBeep(MB_ICONEXCLAMATION);
		return false;
	}

	// save command in history, reset current buffer pointer to the newest command
	curCommandPosition = prevCommands.size();		//this places it one position beyond a legal subscript
	if (curCommandPosition == 0 || (curCommandPosition > 0 && prevCommands[curCommandPosition - 1] != s)) {
		++curCommandPosition;
		prevCommands.push_back(s);
	}
	currentCommand = _T("");
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
			if(!param.empty()) {
				redirect = Text::fromT(param);
				if(BOOLSETTING(JOIN_OPEN_NEW_WINDOW)) {
					HubFrame::openWindow(getParent(), Text::fromT(param));
				} else {
					BOOL whatever = FALSE;
#ifdef PORT_ME
					onFollow(0, 0, 0, whatever);
#endif
				}
			} else {
				addStatus(TSTRING(SPECIFY_SERVER));
			}
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
			showJoins = !showJoins;
			if(showJoins) {
				addStatus(TSTRING(JOIN_SHOWING_ON));
			} else {
				addStatus(TSTRING(JOIN_SHOWING_OFF));
			}
		} else if( Util::stricmp(cmd.c_str(), _T("favshowjoins")) == 0 ) {
			favShowJoins = !favShowJoins;
			if(favShowJoins) {
				addStatus(TSTRING(FAV_JOIN_SHOWING_ON));
			} else {
				addStatus(TSTRING(FAV_JOIN_SHOWING_OFF));
			}
		} else if(Util::stricmp(cmd.c_str(), _T("close")) == 0) {
			this->close(true);
		} else if(Util::stricmp(cmd.c_str(), _T("userlist")) == 0) {
			showUsers->setChecked(!showUsers->getChecked());
		} else if(Util::stricmp(cmd.c_str(), _T("connection")) == 0) {
			addStatus(Text::toT((STRING(IP) + client->getLocalIp() + ", " +
			STRING(PORT) +
			Util::toString(ConnectionManager::getInstance()->getPort()) + "/" +
			Util::toString(SearchManager::getInstance()->getPort()) + "/" +
				Util::toString(ConnectionManager::getInstance()->getSecurePort())
				)));
		} else if((Util::stricmp(cmd.c_str(), _T("favorite")) == 0) || (Util::stricmp(cmd.c_str(), _T("fav")) == 0)) {
			addAsFavorite();
		} else if((Util::stricmp(cmd.c_str(), _T("removefavorite")) == 0) || (Util::stricmp(cmd.c_str(), _T("removefav")) == 0)) {
			removeFavoriteHub();
		} else if(Util::stricmp(cmd.c_str(), _T("getlist")) == 0){
			if( !param.empty() ){
				UserInfo* ui = findUser(param);
				if(ui) {
					ui->getList();
				}
			}
		} else if(Util::stricmp(cmd.c_str(), _T("log")) == 0) {
			StringMap params;
			params["hubNI"] = client->getHubName();
			params["hubURL"] = client->getHubUrl();
			params["myNI"] = client->getMyNick();
			if(param.empty()) {
				WinUtil::openFile(Text::toT(Util::validateFileName(SETTING(LOG_DIRECTORY) + Util::formatParams(SETTING(LOG_FILE_MAIN_CHAT), params, true))));
			} else if(Util::stricmp(param.c_str(), _T("status")) == 0) {
				WinUtil::openFile(Text::toT(Util::validateFileName(SETTING(LOG_DIRECTORY) + Util::formatParams(SETTING(LOG_FILE_STATUS), params, true))));
			}
		} else if(Util::stricmp(cmd.c_str(), _T("help")) == 0) {
			addChat(_T("*** ") + WinUtil::commands + _T(", /join <hub-ip>, /clear, /ts, /showjoins, /favshowjoins, /close, /userlist, /connection, /favorite, /pm <user> [message], /getlist <user>, /log <status, system, downloads, uploads>, /removefavorite"));
		} else if(Util::stricmp(cmd.c_str(), _T("pm")) == 0) {
			string::size_type j = param.find(_T(' '));
			if(j != string::npos) {
				tstring nick = param.substr(0, j);
				UserInfo* ui = findUser(nick);

				if(ui) {
					if(param.size() > j + 1)
						PrivateFrame::openWindow(getParent(), ui->user, param.substr(j+1));
					else
						PrivateFrame::openWindow(getParent(), ui->user);
				}
			} else if(!param.empty()) {
				UserInfo* ui = findUser(param);
				if(ui) {
					PrivateFrame::openWindow(getParent(), ui->user);
				}
			}
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

void HubFrame::clearUserList() {
	users->removeAllRows();
	for(UserMapIter i = userMap.begin(); i != userMap.end(); ++i) {
		delete i->second;
	}
	userMap.clear();
}

void HubFrame::clearTaskList() {
	tasks.clear();
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
	if(chat->length() + static_cast<int>(line.size()) > limit) {
		HoldRedraw hold(chat);
		chat->setSelection(0, chat->lineIndex(chat->lineFromChar(limit / 10)));
		chat->replaceSelection(_T(""));
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
#endif
	if(BOOLSETTING(LOG_MAIN_CHAT)) {
		StringMap params;
		params["message"] = Text::fromT(aLine);
		client->getHubIdentity().getParams(params, "hub", false);
		params["hubURL"] = client->getHubUrl();
		client->getMyIdentity().getParams(params, "my", true);
		LOG(LogManager::CHAT, params);
	}
	chat->addText(line);
#ifdef PORT_ME
	if(noscroll) {
		ctrlClient.SetRedraw(TRUE);
	}
#endif
	setDirty(SettingsManager::BOLD_HUB);
}

void HubFrame::addStatus(const tstring& aLine, bool inChat /* = true */) {
	tstring line = Text::toT("[" + Util::getShortTimeString() + "] ") + aLine;

	setStatus(STATUS_STATUS, line);

	while(lastLinesList.size() + 1 > MAX_CLIENT_LINES)
		lastLinesList.erase(lastLinesList.begin());
	lastLinesList.push_back(line);

	setDirty(SettingsManager::BOLD_HUB);

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

HRESULT HubFrame::handleSpeaker(WPARAM, LPARAM) {
	updateUsers = false;
	TaskQueue::List t;
	tasks.get(t);

	HoldRedraw hold(users);

	for(TaskQueue::Iter i = t.begin(); i != t.end(); ++i) {
		if(i->first == UPDATE_USER) {
			updateUser(*static_cast<UserTask*>(i->second));
		} else if(i->first == UPDATE_USER_JOIN) {
			UserTask& u = *static_cast<UserTask*>(i->second);
			if(updateUser(u)) {
				if (showJoins || (favShowJoins && FavoriteManager::getInstance()->isFavoriteUser(u.user))) {
					addStatus(_T("*** ") + TSTRING(JOINS) + Text::toT(u.identity.getNick()));
				}
			}
		} else if(i->first == REMOVE_USER) {
			UserTask& u = *static_cast<UserTask*>(i->second);
			removeUser(u.user);
			if (showJoins || (favShowJoins && FavoriteManager::getInstance()->isFavoriteUser(u.user))) {
				addStatus(Text::toT("*** " + STRING(PARTS) + u.identity.getNick()));
			}
		} else if(i->first == CONNECTED) {
			addStatus(TSTRING(CONNECTED));
#ifdef PORT_ME
			setTabColor(GREEN);
#endif
		} else if(i->first == DISCONNECTED) {
			clearUserList();
#ifdef PORT_ME
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
					LineDlg linePwd(this, TSTRING(ENTER_PASSWORD), TSTRING(ENTER_PASSWORD), Util::emptyStringT, true);
					if(linePwd.run() == IDOK) {
						client->setPassword(Text::fromT(linePwd.getLine()));
						client->password(Text::fromT(linePwd.getLine()));
						waitingForPW = false;
					} else {
						client->disconnect(true);
					}
				}
			}
		} else if(i->first == PRIVATE_MESSAGE) {
			PMTask& pm = *static_cast<PMTask*>(i->second);
			if(pm.hub) {
				if(BOOLSETTING(IGNORE_HUB_PMS)) {
					addStatus(TSTRING(IGNORED_MESSAGE) + Text::toT(pm.str), false);
				} else if(BOOLSETTING(POPUP_HUB_PMS) || PrivateFrame::isOpen(pm.replyTo)) {
					PrivateFrame::gotMessage(getParent(), pm.from, pm.to, pm.replyTo, Text::toT(pm.str));
				} else {
					addChat(TSTRING(PRIVATE_MESSAGE_FROM) + getNick(pm.from) + _T(": ") + Text::toT(pm.str));
				}
			} else if(pm.bot) {
				if(BOOLSETTING(IGNORE_BOT_PMS)) {
					addStatus(TSTRING(IGNORED_MESSAGE) + Text::toT(pm.str), false);
				} else if(BOOLSETTING(POPUP_BOT_PMS) || PrivateFrame::isOpen(pm.replyTo)) {
					PrivateFrame::gotMessage(getParent(), pm.from, pm.to, pm.replyTo, Text::toT(pm.str));
				} else {
					addChat(TSTRING(PRIVATE_MESSAGE_FROM) + getNick(pm.from) + _T(": ") + Text::toT(pm.str));
				}
			} else {
				if(BOOLSETTING(POPUP_PMS) || PrivateFrame::isOpen(pm.replyTo) || pm.from == client->getMyIdentity().getUser()) {
					PrivateFrame::gotMessage(getParent(), pm.from, pm.to, pm.replyTo, Text::toT(pm.str));
				} else {
					addChat(TSTRING(PRIVATE_MESSAGE_FROM) + getNick(pm.from) + _T(": ") + Text::toT(pm.str));
				}
			}
		}
		delete i->second;
	}
	if(resort && showUsers->getChecked()) {
		users->resort();
		resort = false;
	}

	return 0;
}

HubFrame::UserInfo* HubFrame::findUser(const tstring& nick) {
	for(UserMapIter i = userMap.begin(); i != userMap.end(); ++i) {
		if(i->second->columns[COLUMN_NICK] == nick)
			return i->second;
	}
	return 0;
}

const tstring& HubFrame::getNick(const User::Ptr& aUser) {
	UserMapIter i = userMap.find(aUser);
	if(i == userMap.end())
		return Util::emptyStringT;

	UserInfo* ui = i->second;
	return ui->columns[COLUMN_NICK];
}

bool HubFrame::updateUser(const UserTask& u) {
	UserMapIter i = userMap.find(u.user);
	if(i == userMap.end()) {
		UserInfo* ui = new UserInfo(u);
		userMap.insert(make_pair(u.user, ui));
		if(!ui->isHidden() && showUsers->getChecked())
			users->insertItem(ui);

		if(!filterString.empty())
			updateUserList(ui);
		return true;
	} else {
		UserInfo* ui = i->second;
		if(!ui->isHidden() && u.identity.isHidden() && showUsers->getChecked()) {
			users->deleteItem(ui);
		}

		resort = ui->update(u.identity, users->getSortColumn()) || resort;
		if(showUsers->getChecked()) {
			int pos = users->findItem(ui);
			if(pos != -1) {
				users->updateItem(pos);
			}
			updateUserList(ui);
		}

		return false;
	}
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

void HubFrame::removeUser(const User::Ptr& aUser) {
	UserMapIter i = userMap.find(aUser);
	if(i == userMap.end()) {
		// Should never happen?
		dcassert(i != userMap.end());
		return;
	}

	UserInfo* ui = i->second;
	if(!ui->isHidden() && showUsers->getChecked())
		users->deleteItem(ui);

	userMap.erase(i);
	delete ui;
}

bool HubFrame::historyActive() {
	return isAltPressed() || (isControlPressed() && BOOLSETTING(USE_CTRL_FOR_LINE_HISTORY));
}

bool HubFrame::handleUsersKeyDown(int c) {
	if(c == VK_RETURN) {
		int item = users->getNextItem(-1, LVNI_FOCUSED);
		if(item != -1) {
			users->getItemData(item)->getList();
		}
		return true;
	}
	return false;
}

bool HubFrame::handleMessageChar(int c) {
	switch(c) {
	case VK_TAB:
	case VK_RETURN: {
		if(!(isShiftPressed() || isControlPressed() || isAltPressed())) {
			return true;
		}
	} break;
	case VK_UP:
	case VK_DOWN:
	case VK_END:
	case VK_HOME:
	{
		if(historyActive()) {
			return true;
		}
	} break;
	case VK_PRIOR:
	case VK_NEXT: {
		return true;
	}
	}
	return false;
}

bool HubFrame::handleMessageKeyDown(int c) {
	if(!complete.empty() && c != VK_TAB)
		complete.clear(), inTabComplete = false;

	switch(c) {
	case VK_TAB: 
		if(tab())
			return true;
		break;
	case VK_RETURN: 
		if(enter())
			return true;
		break;
	case VK_UP:
		if ( historyActive() ) {
			//scroll up in chat command history
			//currently beyond the last command?
			if (curCommandPosition > 0) {
				//check whether current command needs to be saved
				if (curCommandPosition == prevCommands.size()) {
					currentCommand = message->getText();
				}

				//replace current chat buffer with current command
				message->setText(prevCommands[--curCommandPosition]);
			}
			// move cursor to end of line
			message->setSelection(message->length(), message->length());
			return true;
		} 
		break;
	case VK_DOWN:
		if ( historyActive() ) {
			//scroll down in chat command history

			//currently beyond the last command?
			if (curCommandPosition + 1 < prevCommands.size()) {
				//replace current chat buffer with current command
				message->setText(prevCommands[++curCommandPosition]);
			} else if (curCommandPosition + 1 == prevCommands.size()) {
				//revert to last saved, unfinished command

				message->setText(currentCommand);
				++curCommandPosition;
			}
			// move cursor to end of line
			message->setSelection(message->length(), message->length());
			return true;
		} 
		break;
	case VK_PRIOR: // page up
		{
			chat->sendMessage(WM_VSCROLL, SB_PAGEUP);
			return true;
		} break;
	case VK_NEXT: // page down
		{
			chat->sendMessage(WM_VSCROLL, SB_PAGEDOWN);
			return true;
		} break;
	case VK_HOME:
		if (!prevCommands.empty() && historyActive() ) {
			curCommandPosition = 0;
			currentCommand = message->getText();

			message->setText(prevCommands[curCommandPosition]);
			return true;
		} 
		break;
	case VK_END:
		if (historyActive()) {
			curCommandPosition = prevCommands.size();

			message->setText(currentCommand);
			return true;
		} 
		break;
	}
	return false;
}

int HubFrame::UserInfo::getImage() const {
	int image = identity.isOp() ? IMAGE_OP : IMAGE_USER;

	if(identity.getUser()->isSet(User::DCPLUSPLUS))
		image+=2;
	if(SETTING(INCOMING_CONNECTIONS) == SettingsManager::INCOMING_FIREWALL_PASSIVE && !identity.isTcpActive()) {
		// Users we can't connect to...
		image+=4;
	}
	return image;
}

HubFrame::UserTask::UserTask(const OnlineUser& ou) : user(ou.getUser()), identity(ou.getIdentity()) { 

}

HubFrame::PMTask::PMTask(const OnlineUser& from_, const OnlineUser& to_, const OnlineUser& replyTo_, const string& m) : 
	StringTask(m), from(from_.getUser()), to(to_.getUser()), replyTo(replyTo_.getUser()), 
	hub(replyTo_.getIdentity().isHub()), bot(replyTo_.getIdentity().isBot()) 
{

}

int HubFrame::UserInfo::compareItems(const HubFrame::UserInfo* a, const HubFrame::UserInfo* b, int col) {
	if(col == COLUMN_NICK) {
		bool a_isOp = a->getIdentity().isOp(),
			b_isOp = b->getIdentity().isOp();
		if(a_isOp && !b_isOp)
			return -1;
		if(!a_isOp && b_isOp)
			return 1;
		if(BOOLSETTING(SORT_FAVUSERS_FIRST)) {
			bool a_isFav = FavoriteManager::getInstance()->isFavoriteUser(a->getIdentity().getUser()),
				b_isFav = FavoriteManager::getInstance()->isFavoriteUser(b->getIdentity().getUser());
			if(a_isFav && !b_isFav)
				return -1;
			if(!a_isFav && b_isFav)
				return 1;
		}
	}
	if(col == COLUMN_SHARED) {
		return compare(a->identity.getBytesShared(), b->identity.getBytesShared());;
	}
	return lstrcmpi(a->columns[col].c_str(), b->columns[col].c_str());
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
	redirect = line;
#ifdef PORT_ME
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
			speak(ADD_SILENT_STATUS_LINE, Text::toDOS(line));
		} else {
			speak(ADD_CHAT_LINE, Text::toDOS(line));
		}
	} else {
		speak(ADD_CHAT_LINE, Text::toDOS(line));
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
	int64_t available;
	if (users->getSelectedCount() > 1) {
		available = users->forEachSelectedT(CountAvailable()).available;
	} else {
		available = std::for_each(userMap.begin(), userMap.end(), CountAvailable()).available;
	}
	return Text::toT(Util::formatBytes(available));
}

tstring HubFrame::getStatusUsers() const {
	size_t userCount = 0;
	for(UserMap::const_iterator i = userMap.begin(); i != userMap.end(); ++i){
		UserInfo* ui = i->second;
		if(!ui->isHidden())
			userCount++;
	}

	tstring textForUsers;
	if (users->getSelectedCount() > 1)
		textForUsers += Text::toT(Util::toString(users->getSelectedCount()) + "/");
	if (showUsers->getChecked() && users->getRowCount() < userCount)
		textForUsers += Text::toT(Util::toString(users->getRowCount()) + "/");
	return textForUsers + Text::toT(Util::toString(userCount) + " " + STRING(HUB_USERS));
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
		speak();
	}
}

void HubFrame::addAsFavorite() {
	FavoriteHubEntry* existingHub = FavoriteManager::getInstance()->getFavoriteHubEntry(client->getHubUrl());
	if(!existingHub) {
		FavoriteHubEntry aEntry;
		aEntry.setServer(url);
		aEntry.setName(Text::fromT(getText()));
		aEntry.setDescription(Text::fromT(getText()));
		aEntry.setConnect(false);
		aEntry.setNick(client->getMyNick());
		FavoriteManager::getInstance()->addFavorite(aEntry);
		addStatus(TSTRING(FAVORITE_HUB_ADDED));
	} else {
		addStatus(TSTRING(FAVORITE_HUB_ALREADY_EXISTS));
	}
}

void HubFrame::removeFavoriteHub() {
	FavoriteHubEntry* removeHub = FavoriteManager::getInstance()->getFavoriteHubEntry(client->getHubUrl());
	if(removeHub) {
		FavoriteManager::getInstance()->removeFavorite(removeHub);
		addStatus(TSTRING(FAVORITE_HUB_REMOVED));
	} else {
		addStatus(TSTRING(FAVORITE_HUB_DOES_NOT_EXIST));
	}
}

bool HubFrame::parseFilter(FilterModes& mode, int64_t& size) {
	tstring::size_type start = static_cast<tstring::size_type>(tstring::npos);
	tstring::size_type end = static_cast<tstring::size_type>(tstring::npos);
	int64_t multiplier = 1;

	if(filterString.empty()) {
		return false;
	}
	if(filterString.compare(0, 2, _T(">=")) == 0) {
		mode = GREATER_EQUAL;
		start = 2;
	} else if(filterString.compare(0, 2, _T("<=")) == 0) {
		mode = LESS_EQUAL;
		start = 2;
	} else if(filterString.compare(0, 2, _T("==")) == 0) {
		mode = EQUAL;
		start = 2;
	} else if(filterString.compare(0, 2, _T("!=")) == 0) {
		mode = NOT_EQUAL;
		start = 2;
	} else if(filterString[0] == _T('<')) {
		mode = LESS;
		start = 1;
	} else if(filterString[0] == _T('>')) {
		mode = GREATER;
		start = 1;
	} else if(filterString[0] == _T('=')) {
		mode = EQUAL;
		start = 1;
	}

	if(start == tstring::npos)
		return false;
	if(filterString.length() <= start)
		return false;

	if((end = Util::findSubString(filterString, _T("TiB"))) != tstring::npos) {
		multiplier = 1024LL * 1024LL * 1024LL * 1024LL;
	} else if((end = Util::findSubString(filterString, _T("GiB"))) != tstring::npos) {
		multiplier = 1024*1024*1024;
	} else if((end = Util::findSubString(filterString, _T("MiB"))) != tstring::npos) {
		multiplier = 1024*1024;
	} else if((end = Util::findSubString(filterString, _T("KiB"))) != tstring::npos) {
		multiplier = 1024;
	} else if((end = Util::findSubString(filterString, _T("TB"))) != tstring::npos) {
		multiplier = 1000LL * 1000LL * 1000LL * 1000LL;
	} else if((end = Util::findSubString(filterString, _T("GB"))) != tstring::npos) {
		multiplier = 1000*1000*1000;
	} else if((end = Util::findSubString(filterString, _T("MB"))) != tstring::npos) {
		multiplier = 1000*1000;
	} else if((end = Util::findSubString(filterString, _T("kB"))) != tstring::npos) {
		multiplier = 1000;
	} else if((end = Util::findSubString(filterString, _T("B"))) != tstring::npos) {
		multiplier = 1;
	}

	if(end == tstring::npos) {
		end = filterString.length();
	}

	tstring tmpSize = filterString.substr(start, end-start);
	size = static_cast<int64_t>(Util::toDouble(Text::fromT(tmpSize)) * multiplier);

	return true;
}

void HubFrame::updateUserList(UserInfo* ui) {
	int64_t size = -1;
	FilterModes mode = NONE;

	int sel = filterType->getSelectedIndex();

	bool doSizeCompare = parseFilter(mode, size) && sel == COLUMN_SHARED;

	//single update?
	//avoid refreshing the whole list and just update the current item
	//instead
	if(ui != NULL) {
		if(ui->isHidden()) {
			return;
		}
		if(filterString.empty()) {
			if(users->findItem(ui) == -1) {
				users->insertItem(ui);
			}
		} else {
			if(matchFilter(*ui, sel, doSizeCompare, mode, size)) {
				if(users->findItem(ui) == -1) {
					users->insertItem(ui);
				}
			} else {
				//deleteItem checks to see that the item exists in the list
				//unnecessary to do it twice.
				users->deleteItem(ui);
			}
		}
	} else {
		HoldRedraw hold(users);
		users->removeAllRows();

		if(filterString.empty()) {
			for(UserMapIter i = userMap.begin(); i != userMap.end(); ++i){
				UserInfo* ui = i->second;
				if(!ui->isHidden())
					users->insertItem(i->second);
			}
		} else {
			for(UserMapIter i = userMap.begin(); i != userMap.end(); ++i) {
				UserInfo* ui = i->second;
				if(!ui->isHidden() && matchFilter(*ui, sel, doSizeCompare, mode, size)) {
					users->insertItem(ui);
				}
			}
		}
	}
}

bool HubFrame::matchFilter(const UserInfo& ui, int sel, bool doSizeCompare, FilterModes mode, int64_t size) {
	if(filterString.empty())
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
				if(Util::findSubString(ui.getText(i), filterString) != string::npos) {
					insert = true;
					break;
				}
			}
		} else {
			if(Util::findSubString(ui.getText(sel), filterString) != string::npos)
				insert = true;
		}
	}

	return insert;
}

HRESULT HubFrame::handleContextMenu(WPARAM wParam, LPARAM lParam) {
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

	bool doMenu = false;

	if(reinterpret_cast<HWND>(wParam) == chat->handle()) {
		if(pt.x == -1 || pt.y == -1) {
			pt = chat->getContextMenuPos();
		}
		
		chat->screenToClient(pt);
		tstring txt = chat->textUnderCursor(pt);
		chat->clientToScreen(pt);
		
		if(!txt.empty()) {
			// Possible nickname click, let's see if we can find one like it in the name list...
			int pos = users->findItem(txt);
			if(pos != -1) {
				users->clearSelection();
				users->setSelectedIndex(pos);
				users->ensureVisible(pos);
				doMenu = true;
			}
		}
	}

	if((doMenu || (reinterpret_cast<HWND>(wParam) == users->handle())) && users->hasSelection()) {
		if(pt.x == -1 || pt.y == -1) {
			pt = users->getContextMenuPos();
		}

		WidgetMenuPtr menu = createMenu(true);
		appendUserItems(getParent(), menu);
		
		menu->appendItem(IDC_COPY_NICK, TSTRING(COPY_NICK), std::tr1::bind(&HubFrame::handleCopyNick, this));
		menu->setDefaultItem(IDC_GETLIST);
		prepareMenu(menu, UserCommand::CONTEXT_CHAT, client->getHubUrl());
		
		inTabMenu = true;
		
		menu->trackPopupMenu(this, pt.x, pt.y, TPM_LEFTALIGN | TPM_RIGHTBUTTON);
		return TRUE;
	}
	return FALSE;
}

bool HubFrame::handleTabContextMenu(const SmartWin::Point& pt) {
	WidgetMenuPtr menu = createMenu(true);

	if(!FavoriteManager::getInstance()->isFavoriteHub(url)) {
		menu->appendItem(IDC_ADD_TO_FAVORITES, TSTRING(ADD_TO_FAVORITES), std::tr1::bind(&HubFrame::handleAddAsFavorite, this));
	}
	
	menu->appendItem(IDC_RECONNECT, TSTRING(MENU_RECONNECT), std::tr1::bind(&HubFrame::handleReconnect, this));
	menu->appendItem(IDC_COPY_HUB, TSTRING(COPY_HUB), std::tr1::bind(&HubFrame::handleCopyHub, this));

	prepareMenu(menu, UserCommand::CONTEXT_HUB, url);
	menu->appendSeparatorItem();
	menu->appendItem(IDC_CLOSE_WINDOW, TSTRING(CLOSE), std::tr1::bind(&HubFrame::close, this, true));

	inTabMenu = true;
	
	menu->trackPopupMenu(this, pt.x, pt.y, TPM_LEFTALIGN | TPM_RIGHTBUTTON);
	return true;
}

void HubFrame::handleShowUsersClicked() {
	bool checked = showUsers->getChecked();
	
	users->setVisible(checked);
	paned->setVisible(checked);
	
	if(checked) {
		updateUserList();
	}
	
	SettingsManager::getInstance()->set(SettingsManager::GET_USER_INFO, checked);

	layout();
}

#ifdef PORT_ME

LRESULT HubFrame::OnForwardMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	LPMSG pMsg = (LPMSG)lParam;
	if((pMsg->message >= WM_MOUSEFIRST) && (pMsg->message <= WM_MOUSELAST))
		ctrlLastLines.RelayEvent(pMsg);
	return 0;
}

#endif

void HubFrame::handleCopyNick() {
	int i=-1;
	string nicks;

	while( (i = users->getNextItem(i, LVNI_SELECTED)) != -1) {
		nicks += users->getItemData(i)->getNick();
		nicks += ' ';
	}
	if(!nicks.empty()) {
		// remove last space
		nicks.erase(nicks.length() - 1);
		WinUtil::setClipboard(Text::toT(nicks));
	}
}

void HubFrame::handleCopyHub() {
	WinUtil::setClipboard(Text::toT(url));
}

void HubFrame::handleDoubleClickUsers() {
	if(users->hasSelection()) {
		users->getSelectedItem()->getList();
	}
}

#ifdef PORT_ME
static const COLORREF RED = RGB(255, 0, 0);
static const COLORREF GREEN = RGB(0, 255, 0);

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
				} else if(showUsers->getChecked()) {
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

#endif

void HubFrame::runUserCommand(const UserCommand& uc) {
	if(!WinUtil::getUCParams(this, uc, ucLineParams))
		return;

	StringMap ucParams = ucLineParams;

	client->getMyIdentity().getParams(ucParams, "my", true);
	client->getHubIdentity().getParams(ucParams, "hub", false);

	if(inTabMenu) {
		client->escapeParams(ucParams);
		client->sendUserCmd(Util::formatParams(uc.getCommand(), ucParams, false));
	} else {
		int sel = -1;
		while((sel = users->getNextItem(sel, LVNI_SELECTED)) != -1) {
			UserInfo* u = users->getItemData(sel);
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

static bool compareCharsNoCase(string::value_type a, string::value_type b) {
	return Text::toLower(a) == Text::toLower(b);
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

bool HubFrame::tab() {
	if(message->length() == 0) {
		return false;
	}

	HWND focus = GetFocus();
	if( (focus == message->handle()) && !isShiftPressed() )
	{
		tstring text = message->getText();
		string::size_type textStart = text.find_last_of(_T(" \n\t"));

		if(complete.empty()) {
			if(textStart != string::npos) {
				complete = text.substr(textStart + 1);
			} else {
				complete = text;
			}
			if(complete.empty()) {
				// Still empty, no text entered...
				return false;
			}
			users->clearSelection();
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
			addChat(nicks);
			inTabComplete = false;
		} else {
			// First tab. Maximally extend proposed nick.
			tstring nick = scanNickPrefix(complete);
			if (tabCompleteNicks.empty()) return true;

			// Maybe it found a unique match. If userlist showing, highlight.
			if (showUsers->getChecked() && tabCompleteNicks.size() == 2) {
				int i = users->findItem(Text::toT(tabCompleteNicks[1]));
				users->setSelectedIndex(i);
				users->ensureVisible(i);
			}

			message->setSelection(textStart, -1);
			
			// no shift, use partial nick when appropriate
			if(isShiftPressed()) {
				message->replaceSelection(nick);
			} else {
				message->replaceSelection(Text::toT(stripNick(Text::fromT(nick))));
			}

			inTabComplete = true;
			return true;
		}
	}
	return false;
}

void HubFrame::handleReconnect() {
	client->reconnect();
}

#ifdef PORT_ME
LRESULT HubFrame::onFollow(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(!redirect.empty()) {
		if(ClientManager::getInstance()->isConnected(redirect)) {
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

#endif

void HubFrame::resortUsers() {
	for(FrameIter i = frames.begin(); i != frames.end(); ++i)
		(*i)->resortForFavsFirst(true);
}

void HubFrame::updateFilter(const tstring& newText) {

	if(newText != filterString) {
		filterString = newText;
		updateUserList();
	}
}

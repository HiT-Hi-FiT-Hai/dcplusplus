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

#include "HubFrame.h"
#include "PrivateFrame.h"
#include "LineDlg.h"
#include "HoldRedraw.h"

#include <dcpp/ClientManager.h>
#include <dcpp/Client.h>
#include <dcpp/LogManager.h>
#include <dcpp/User.h>
#include <dcpp/FavoriteManager.h>
#include <dcpp/ConnectionManager.h>
#include <dcpp/SearchManager.h>

int HubFrame::columnSizes[] = { 100, 75, 75, 100, 75, 100, 100, 125 };
int HubFrame::columnIndexes[] = { COLUMN_NICK, COLUMN_SHARED, COLUMN_DESCRIPTION, COLUMN_TAG, COLUMN_CONNECTION, COLUMN_IP, COLUMN_EMAIL, COLUMN_CID };
static const char* columnNames[] = {
	N_("Nick"),
	N_("Shared"),
	N_("Description"),
	N_("Tag"),
	N_("Connection"),
	N_("IP"),
	N_("E-Mail"),
	N_("CID")
};

HubFrame::FrameList HubFrame::frames;

void HubFrame::closeDisconnected() {
	for(FrameIter i=frames.begin(); i!= frames.end(); ++i) {
		HubFrame* frame = *i;
		if (!(frame->client->isConnected())) {
			::PostMessage(frame->handle(), WM_CLOSE, 0, 0);
		}
	}
}

void HubFrame::openWindow(dwt::TabView* mdiParent, const string& url) {
	for(FrameIter i = frames.begin(); i!= frames.end(); ++i) {
		HubFrame* frame = *i;
		if(frame->url == url) {
			frame->activate();
			return;
		}
	}

	new HubFrame(mdiParent, url);
}

HubFrame::HubFrame(dwt::TabView* mdiParent, const string& url_) : 
	BaseType(mdiParent, Text::toT(url_), IDH_HUB, dwt::IconPtr(new dwt::Icon(IDR_HUB))),
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
	paned = addChild(WidgetVPaned::Seed(0.7));

	{
		TextBox::Seed cs = WinUtil::Seeds::textBox;
		cs.style |= WS_VSCROLL | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE;
		message = addChild(cs);
		message->setHelpId(IDH_HUB_MESSAGE);
		addWidget(message, true, false);
		message->onRaw(std::tr1::bind(&HubFrame::handleMessageGetDlgCode, this), dwt::Message(WM_GETDLGCODE));
		message->onKeyDown(std::tr1::bind(&HubFrame::handleMessageKeyDown, this, _1));
		message->onSysKeyDown(std::tr1::bind(&HubFrame::handleMessageKeyDown, this, _1));
		message->onChar(std::tr1::bind(&HubFrame::handleMessageChar, this, _1));
	}

	{
		TextBox::Seed cs = WinUtil::Seeds::textBox;
		cs.style |= ES_AUTOHSCROLL;
		filter = addChild(cs);
		filter->setHelpId(IDH_HUB_FILTER);
		addWidget(filter);
		filter->onKeyUp(std::tr1::bind(&HubFrame::handleFilterKey, this, _1));
	}

	{
		filterType = addChild(WinUtil::Seeds::comboBoxStatic);
		filterType->setHelpId(IDH_HUB_FILTER);
		addWidget(filterType);
		
		for(int j=0; j<COLUMN_LAST; j++) {
			filterType->addValue(T_(columnNames[j]));
		}
		filterType->addValue(T_("Any"));
		filterType->setSelected(COLUMN_LAST);
		filterType->onSelectionChanged(std::tr1::bind(&HubFrame::updateUserList, this, (UserInfo*)0));
	}

	{
		users = addChild(WidgetUsers::Seed());
		addWidget(users);
		paned->setSecond(users);
		
		users->setSmallImageList(WinUtil::userImages);
		users->createColumns(WinUtil::getStrings(columnNames));
		users->setColumnOrder(WinUtil::splitTokens(SETTING(HUBFRAME_ORDER), columnIndexes));
		users->setColumnWidths(WinUtil::splitTokens(SETTING(HUBFRAME_WIDTHS), columnSizes));
		users->setSort(COLUMN_NICK);
		
		users->onSelectionChanged(std::tr1::bind(&HubFrame::updateStatus, this));
		users->onDblClicked(std::tr1::bind(&HubFrame::handleDoubleClickUsers, this));
		users->onKeyDown(std::tr1::bind(&HubFrame::handleUsersKeyDown, this, _1));
		users->onContextMenu(std::tr1::bind(&HubFrame::handleUsersContextMenu, this, _1));
	}

	{
		TextBox::Seed cs = WinUtil::Seeds::textBox;
		cs.style |= WS_VSCROLL | ES_MULTILINE | ES_NOHIDESEL | ES_READONLY;
		chat = addChild(cs);
		chat->setHelpId(IDH_HUB_CHAT);
		chat->setTextLimit(0);
		addWidget(chat);
		paned->setFirst(chat);
		chat->onContextMenu(std::tr1::bind(&HubFrame::handleChatContextMenu, this, _1));
	}
	
	{
		CheckBox::Seed cs(_T("+/-"));
		cs.style &= ~WS_TABSTOP;
		showUsers = addChild(cs);
		showUsers->setChecked(BOOLSETTING(GET_USER_INFO));
	}

	initStatus();
	///@todo get real resizer width
	statusSizes[STATUS_SHOW_USERS] = 16;

	layout();
	
	initSecond();
	
	onSpeaker(std::tr1::bind(&HubFrame::handleSpeaker, this, _1, _2));
	onTabContextMenu(std::tr1::bind(&HubFrame::handleTabContextMenu, this, _1));
	onCommand(std::tr1::bind(&HubFrame::handleReconnect, this), IDC_RECONNECT);
	onCommand(std::tr1::bind(&HubFrame::handleFollow, this), IDC_FOLLOW);
	
	client = ClientManager::getInstance()->getClient(url);
	client->addListener(this);
	client->connect();
	
	frames.push_back(this);
	
	showUsers->onClicked(std::tr1::bind(&HubFrame::handleShowUsersClicked, this));

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
}

void HubFrame::layout() {
	bool scroll = chat->scrollIsAtEnd();

	const int border = 2;
	
	dwt::Rectangle r(getClientAreaSize()); 

	layoutStatus(r);
	mapWidget(STATUS_SHOW_USERS, showUsers);
	
	int ymessage = message->getTextSize(_T("A")).y + 10;
	int xfilter = showUsers->getChecked() ? std::min(r.width() / 4, 200l) : 0;
	dwt::Rectangle rm(0, r.size.y - ymessage, r.width() - xfilter, ymessage);
	message->setBounds(rm);

	r.size.y -= rm.size.y + border;
	
	rm.pos.x += rm.width() + border;
	rm.size.x = showUsers->getChecked() ? xfilter * 2 / 3 - border : 0;
	filter->setBounds(rm);
	
	rm.pos.x += rm.width() + border;
	rm.size.x = showUsers->getChecked() ? xfilter / 3 - border : 0;
	rm.size.y += 140;
	filterType->setBounds(rm);

	bool checked = showUsers->getChecked();
	if(checked && !paned->getSecond()) {
		paned->setSecond(users);
	} else if(!checked && paned->getSecond()) {
		paned->setSecond(0);
	}
	paned->setRect(r);

	if(scroll)
		chat->sendMessage(WM_VSCROLL, SB_BOTTOM);
}

void HubFrame::updateStatus() {
	setStatus(STATUS_USERS, getStatusUsers());
	setStatus(STATUS_SHARED, getStatusShared());
	setStatus(STATUS_AVERAGE_SHARED, getStatusAverageShared());
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
		bool thirdPerson = false;
		if(WinUtil::checkCommand(cmd, param, msg, status, thirdPerson)) {
			if(!msg.empty()) {
				client->hubMessage(Text::fromT(msg), thirdPerson);
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
					handleFollow();
				}
			} else {
				addStatus(T_("Specify a server to connect to"));
			}
		} else if(Util::stricmp(cmd.c_str(), _T("clear")) == 0) {
			chat->setText(_T(""));
		} else if(Util::stricmp(cmd.c_str(), _T("ts")) == 0) {
			timeStamps = !timeStamps;
			if(timeStamps) {
				addStatus(T_("Timestamps enabled"));
			} else {
				addStatus(T_("Timestamps disabled"));
			}
		} else if( (Util::stricmp(cmd.c_str(), _T("password")) == 0) && waitingForPW ) {
			client->setPassword(Text::fromT(param));
			client->password(Text::fromT(param));
			waitingForPW = false;
		} else if( Util::stricmp(cmd.c_str(), _T("showjoins")) == 0 ) {
			showJoins = !showJoins;
			if(showJoins) {
				addStatus(T_("Join/part showing on"));
			} else {
				addStatus(T_("Join/part showing off"));
			}
		} else if( Util::stricmp(cmd.c_str(), _T("favshowjoins")) == 0 ) {
			favShowJoins = !favShowJoins;
			if(favShowJoins) {
				addStatus(T_("Join/part of favorite users showing on"));
			} else {
				addStatus(T_("Join/part of favorite users showing off"));
			}
		} else if(Util::stricmp(cmd.c_str(), _T("close")) == 0) {
			this->close(true);
		} else if(Util::stricmp(cmd.c_str(), _T("userlist")) == 0) {
			showUsers->setChecked(!showUsers->getChecked());
		} else if(Util::stricmp(cmd.c_str(), _T("connection")) == 0) {
			addStatus(str(TF_("IP: %1%, Port: %2%/%3%/%4%") % Text::toT(client->getLocalIp())
				% ConnectionManager::getInstance()->getPort()
				% SearchManager::getInstance()->getPort()
				% ConnectionManager::getInstance()->getSecurePort()
				));
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
				addStatus(str(TF_("Unknown command: %1%") % cmd));
			}
		}
		message->setText(_T(""));
	} else if(waitingForPW) {
		addStatus(T_("Don't remove /password before your password"));
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
	users->clear();
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
	line += Text::toDOS(aLine);

	bool scroll = chat->scrollIsAtEnd();
	HoldRedraw hold(chat, !scroll);

	size_t limit = chat->getTextLimit();
	if(chat->length() + line.size() > limit) {
		HoldRedraw hold2(chat, scroll);
		chat->setSelection(0, chat->lineIndex(chat->lineFromChar(limit / 10)));
		chat->replaceSelection(_T(""));
	}
	if(BOOLSETTING(LOG_MAIN_CHAT)) {
		StringMap params;
		params["message"] = Text::fromT(aLine);
		client->getHubIdentity().getParams(params, "hub", false);
		params["hubURL"] = client->getHubUrl();
		client->getMyIdentity().getParams(params, "my", true);
		LOG(LogManager::CHAT, params);
	}
	chat->addText(line);

	if(scroll)
		chat->sendMessage(WM_VSCROLL, SB_BOTTOM);

	WinUtil::playSound(SettingsManager::SOUND_MAIN_CHAT);
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
					addStatus(str(TF_("*** Joins: %1%") % Text::toT(u.identity.getNick())));
				}
			}
		} else if(i->first == REMOVE_USER) {
			UserTask& u = *static_cast<UserTask*>(i->second);
			removeUser(u.user);
			if (showJoins || (favShowJoins && FavoriteManager::getInstance()->isFavoriteUser(u.user))) {
				addStatus(str(TF_("*** Parts: %1%") % Text::toT(u.identity.getNick())));
			}
		} else if(i->first == CONNECTED) {
			addStatus(T_("Connected"));
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
				addStatus(T_("Stored password sent..."));
			} else {
				if(!BOOLSETTING(PROMPT_PASSWORD)) {
					message->setText(_T("/password "));
					message->setFocus();
					message->setSelection(10, 10);
					waitingForPW = true;
				} else {
					LineDlg linePwd(this, T_("Please enter a password"), T_("Please enter a password"), Util::emptyStringT, true);
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
					addStatus(str(TF_("Ignored message: %1%") % Text::toT(pm.str)), false);
				} else if(BOOLSETTING(POPUP_HUB_PMS) || PrivateFrame::isOpen(pm.replyTo)) {
					PrivateFrame::gotMessage(getParent(), pm.from, pm.to, pm.replyTo, Text::toT(pm.str));
				} else {
					addChat(str(TF_("Private message from %1%: %2%") % getNick(pm.from) % Text::toT(pm.str)));
				}
			} else if(pm.bot) {
				if(BOOLSETTING(IGNORE_BOT_PMS)) {
					addStatus(str(TF_("Ignored message: %1%") % Text::toT(pm.str)), false);
				} else if(BOOLSETTING(POPUP_BOT_PMS) || PrivateFrame::isOpen(pm.replyTo)) {
					PrivateFrame::gotMessage(getParent(), pm.from, pm.to, pm.replyTo, Text::toT(pm.str));
				} else {
					addChat(str(TF_("Private message from %1%: %2%") % getNick(pm.from) % Text::toT(pm.str)));
				}
			} else {
				if(BOOLSETTING(POPUP_PMS) || PrivateFrame::isOpen(pm.replyTo) || pm.from == client->getMyIdentity().getUser()) {
					PrivateFrame::gotMessage(getParent(), pm.from, pm.to, pm.replyTo, Text::toT(pm.str));
				} else {
					addChat(str(TF_("Private message from %1%: %2%") % getNick(pm.from) % Text::toT(pm.str)));
				}
			}
		} else if(i->first == FOLLOW) {
			handleFollow();
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

const tstring& HubFrame::getNick(const UserPtr& aUser) {
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
			users->insert(ui);

		if(!filterString.empty())
			updateUserList(ui);
		return true;
	} else {
		UserInfo* ui = i->second;
		if(!ui->isHidden() && u.identity.isHidden() && showUsers->getChecked()) {
			users->erase(ui);
		}

		resort = ui->update(u.identity, users->getSortColumn()) || resort;
		if(showUsers->getChecked()) {
			int pos = users->find(ui);
			if(pos != -1) {
				users->update(pos);
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

void HubFrame::removeUser(const UserPtr& aUser) {
	UserMapIter i = userMap.find(aUser);
	if(i == userMap.end()) {
		// Should never happen?
		dcassert(i != userMap.end());
		return;
	}

	UserInfo* ui = i->second;
	if(!ui->isHidden() && showUsers->getChecked())
		users->erase(ui);

	userMap.erase(i);
	delete ui;
}

bool HubFrame::historyActive() {
	return isAltPressed() || (BOOLSETTING(USE_CTRL_FOR_LINE_HISTORY) && isControlPressed());
}

bool HubFrame::handleUsersKeyDown(int c) {
	if(c == VK_RETURN && users->hasSelected()) {
		handleGetList();
		return true;
	}
	return false;
}

LRESULT HubFrame::handleMessageGetDlgCode() {
	// override the MDIChildFrame behavior, which tells the Dialog Manager to process Tab presses by itself
	return DLGC_WANTMESSAGE;
}

bool HubFrame::handleMessageChar(int c) {
	switch(c) {
	case VK_TAB: return true; break;
	case VK_RETURN: {
		if(!(isShiftPressed() || isControlPressed() || isAltPressed())) {
			return true;
		}
	} break;
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
	speak(ADD_STATUS_LINE, str(F_("Connecting to %1%...") % client->getHubUrl()));
	speak(SET_WINDOW_TITLE, client->getHubUrl());
}
void HubFrame::on(Connected, Client*) throw() {
	speak(CONNECTED);
}
void HubFrame::on(UserUpdated, Client*, const OnlineUser& user) throw() {
	speak(UPDATE_USER_JOIN, user);
}
void HubFrame::on(UsersUpdated, Client*, const OnlineUserList& aList) throw() {
	for(OnlineUserList::const_iterator i = aList.begin(); i != aList.end(); ++i) {
		tasks.add(UPDATE_USER, new UserTask(*(*i)));
	}
	updateUsers = true;
}

void HubFrame::on(ClientListener::UserRemoved, Client*, const OnlineUser& user) throw() {
	speak(REMOVE_USER, user);
}

void HubFrame::on(Redirect, Client*, const string& line) throw() {
	if(ClientManager::getInstance()->isConnected(line)) {
		speak(ADD_STATUS_LINE, _("Redirect request received to a hub that's already connected"));
		return;
	}
	redirect = line;
	if(BOOLSETTING(AUTO_FOLLOW)) {
		speak(FOLLOW);
	} else {
		speak(ADD_STATUS_LINE, str(F_("Press the follow redirect button to connect to %1%") % line));
	}
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

void HubFrame::on(Message, Client*, const OnlineUser& from, const string& msg, bool thirdPerson) throw() {
	speak(ADD_CHAT_LINE, Util::formatMessage(from.getIdentity().getNick(), msg, thirdPerson));
}

void HubFrame::on(StatusMessage, Client*, const string& line, int statusFlags) throw() {
	if(SETTING(FILTER_MESSAGES) && (statusFlags & ClientListener::FLAG_IS_SPAM)) {
		speak(ADD_SILENT_STATUS_LINE, line);
	} else {
		speak(ADD_STATUS_LINE, line);
	}
}

void HubFrame::on(PrivateMessage, Client*, const OnlineUser& from, const OnlineUser& to, const OnlineUser& replyTo, const string& line, bool thirdPerson) throw() {
	speak(from, to, replyTo, Util::formatMessage(from.getIdentity().getNick(), line, thirdPerson));
}

void HubFrame::on(NickTaken, Client*) throw() {
	speak(ADD_STATUS_LINE, _("Your nick was already taken, please change to something else!"));
}

void HubFrame::on(SearchFlood, Client*, const string& line) throw() {
	speak(ADD_STATUS_LINE, str(F_("Search spam detected from %1%") % line));
}

tstring HubFrame::getStatusShared() const {
	int64_t available;
	if (users->countSelected() > 1) {
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
	if (users->countSelected() > 1)
		textForUsers += Text::toT(Util::toString(users->countSelected()) + "/");
	if (showUsers->getChecked() && users->size() < userCount)
		textForUsers += Text::toT(Util::toString(users->size()) + "/");
	return textForUsers + str(TFN_("%1% user", "%1% users", userCount) % userCount);
}

tstring HubFrame::getStatusAverageShared() const {
	int64_t available;
	size_t userCount = 0;
	if (users->countSelected() > 1) {
		available = users->forEachSelectedT(CountAvailable()).available;
		userCount = users->countSelected();
	} else {
		available = std::for_each(userMap.begin(), userMap.end(), CountAvailable()).available;
		for(UserMap::const_iterator i = userMap.begin(); i != userMap.end(); ++i){
			UserInfo* ui = i->second;
			if(!ui->isHidden())
				userCount++;
		}
	}

	return str(TF_("Average: %1%") % Text::toT(Util::formatBytes(userCount > 0 ? available / userCount : 0)));
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
		aEntry.setName(client->getHubName());
		aEntry.setDescription(client->getHubDescription());
		aEntry.setConnect(false);
		aEntry.setNick(client->getMyNick());
		FavoriteManager::getInstance()->addFavorite(aEntry);
		addStatus(T_("Favorite hub added"));
	} else {
		addStatus(T_("Hub already exists as a favorite"));
	}
}

void HubFrame::removeFavoriteHub() {
	FavoriteHubEntry* removeHub = FavoriteManager::getInstance()->getFavoriteHubEntry(client->getHubUrl());
	if(removeHub) {
		FavoriteManager::getInstance()->removeFavorite(removeHub);
		addStatus(T_("Favorite hub removed"));
	} else {
		addStatus(T_("This hub is not a favorite hub"));
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

	int sel = filterType->getSelected();

	bool doSizeCompare = parseFilter(mode, size) && sel == COLUMN_SHARED;

	//single update?
	//avoid refreshing the whole list and just update the current item
	//instead
	if(ui != NULL) {
		if(ui->isHidden()) {
			return;
		}
		if(filterString.empty()) {
			if(users->find(ui) == -1) {
				users->insert(ui);
			}
		} else {
			if(matchFilter(*ui, sel, doSizeCompare, mode, size)) {
				if(users->find(ui) == -1) {
					users->insert(ui);
				}
			} else {
				//erase checks to see that the item exists in the list
				//unnecessary to do it twice.
				users->erase(ui);
			}
		}
	} else {
		HoldRedraw hold(users);
		users->clear();

		if(filterString.empty()) {
			for(UserMapIter i = userMap.begin(); i != userMap.end(); ++i){
				UserInfo* ui = i->second;
				if(!ui->isHidden())
					users->insert(i->second);
			}
		} else {
			for(UserMapIter i = userMap.begin(); i != userMap.end(); ++i) {
				UserInfo* ui = i->second;
				if(!ui->isHidden() && matchFilter(*ui, sel, doSizeCompare, mode, size)) {
					users->insert(ui);
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
			case NONE: ; break;
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

bool HubFrame::handleChatContextMenu(dwt::ScreenCoordinate pt) {
	bool doMenu = false;

	if(pt.x() == -1 || pt.y() == -1) {
		pt = chat->getContextMenuPos();
	}
		
	tstring txt = chat->textUnderCursor(pt);
	
	if(!txt.empty()) {
		// Possible nickname click, let's see if we can find one like it in the name list...
		int pos = users->find(txt);
		if(pos != -1) {
			users->clearSelection();
			users->setSelected(pos);
			users->ensureVisible(pos);
			doMenu = true;
		}
	}
	
	return doMenu ? handleUsersContextMenu(pt) : false;
}

bool HubFrame::handleUsersContextMenu(dwt::ScreenCoordinate pt) {
	if(users->hasSelected()) {
		if(pt.x() == -1 || pt.y() == -1) {
			pt = users->getContextMenuPos();
		}

		MenuPtr menu = createMenu(WinUtil::Seeds::menu);
		appendUserItems(getParent(), menu);
		
		menu->appendItem(IDC_COPY_NICK, T_("Copy &nick to clipboard"), std::tr1::bind(&HubFrame::handleCopyNick, this));
		menu->setDefaultItem(IDC_GETLIST);
		prepareMenu(menu, UserCommand::CONTEXT_CHAT, client->getHubUrl());
		
		inTabMenu = false;
		
		menu->trackPopupMenu(pt, TPM_LEFTALIGN | TPM_RIGHTBUTTON);
		return true;
	}
	return false;
}

bool HubFrame::handleTabContextMenu(const dwt::ScreenCoordinate& pt) {
	MenuPtr menu = createMenu(WinUtil::Seeds::menu);

	menu->setTitle(getParent()->getTabText(this));

	if(!FavoriteManager::getInstance()->isFavoriteHub(url)) {
		menu->appendItem(IDC_ADD_TO_FAVORITES, T_("Add To &Favorites"), std::tr1::bind(&HubFrame::addAsFavorite, this), dwt::BitmapPtr(new dwt::Bitmap(IDB_FAVORITE_HUBS)));
	}
	
	menu->appendItem(IDC_RECONNECT, T_("&Reconnect\tCtrl+R"), std::tr1::bind(&HubFrame::handleReconnect, this), dwt::BitmapPtr(new dwt::Bitmap(IDB_RECONNECT)));
	menu->appendItem(IDC_COPY_HUB, T_("Copy &address to clipboard"), std::tr1::bind(&HubFrame::handleCopyHub, this));

	prepareMenu(menu, UserCommand::CONTEXT_HUB, url);
	menu->appendSeparatorItem();
	menu->appendItem(IDC_CLOSE_WINDOW, T_("&Close"), std::tr1::bind(&HubFrame::close, this, true), dwt::BitmapPtr(new dwt::Bitmap(IDB_EXIT)));

	inTabMenu = true;
	
	menu->trackPopupMenu(pt, TPM_LEFTALIGN | TPM_RIGHTBUTTON);
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

void HubFrame::handleCopyNick() {
	int i=-1;
	string nicks;

	while( (i = users->getNext(i, LVNI_SELECTED)) != -1) {
		nicks += users->getData(i)->getNick();
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
	if(users->hasSelected()) {
		users->getSelectedData()->getList();
	}
}

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
		while((sel = users->getNext(sel, LVNI_SELECTED)) != -1) {
			UserInfo* u = users->getData(sel);
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
		::SetFocus(::GetNextDlgTabItem(handle(), message->handle(), isShiftPressed()));
		return true;
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
				int i = users->find(Text::toT(tabCompleteNicks[1]));
				users->setSelected(i);
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

void HubFrame::handleFollow() {
	if(!redirect.empty()) {
		if(ClientManager::getInstance()->isConnected(redirect)) {
			addStatus(T_("Redirect request received to a hub that's already connected"));
			return;
		}

		url = redirect;

		// the client is dead, long live the client!
		client->removeListener(this);
		ClientManager::getInstance()->putClient(client);
		clearUserList();
		clearTaskList();
		client = ClientManager::getInstance()->getClient(url);
		client->addListener(this);
		client->connect();
	}
}

void HubFrame::resortUsers() {
	for(FrameIter i = frames.begin(); i != frames.end(); ++i)
		(*i)->resortForFavsFirst(true);
}

bool HubFrame::handleFilterKey(int) {
	tstring newText = filter->getText();
	if(newText != filterString) {
		filterString = newText;
		updateUserList();
	}
	return true;
}

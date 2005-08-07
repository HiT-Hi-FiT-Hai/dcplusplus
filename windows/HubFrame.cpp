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

#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "Resource.h"

#include "HubFrame.h"
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

HubFrame::FrameMap HubFrame::frames;

int HubFrame::columnSizes[] = { 100, 75, 75, 100, 75, 100 };
int HubFrame::columnIndexes[] = { COLUMN_NICK, COLUMN_SHARED, COLUMN_DESCRIPTION, COLUMN_TAG, COLUMN_CONNECTION, COLUMN_EMAIL };
static ResourceManager::Strings columnNames[] = { ResourceManager::NICK, ResourceManager::SHARED,
ResourceManager::DESCRIPTION, ResourceManager::TAG, ResourceManager::CONNECTION, ResourceManager::EMAIL };


LRESULT HubFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	CreateSimpleStatusBar(ATL_IDS_IDLEMESSAGE, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SBARS_SIZEGRIP);
	ctrlStatus.Attach(m_hWndStatusBar);

	ctrlClient.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		WS_VSCROLL | ES_MULTILINE | ES_NOHIDESEL | ES_READONLY, WS_EX_CLIENTEDGE);

	ctrlClient.FmtLines(TRUE);
	ctrlClient.LimitText(0);
	ctrlClient.SetFont(WinUtil::font);
	clientContainer.SubclassWindow(ctrlClient.m_hWnd);
	
	ctrlMessage.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VSCROLL |
		ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE, WS_EX_CLIENTEDGE);
	
	ctrlMessageContainer.SubclassWindow(ctrlMessage.m_hWnd);
	ctrlMessage.SetFont(WinUtil::font);
	
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

	showJoins = BOOLSETTING(SHOW_JOINS);
	favShowJoins = BOOLSETTING(FAV_SHOW_JOINS);

	bHandled = FALSE;
	client->connect();
	
	FavoriteHubEntry *fhe = FavoriteManager::getInstance()->getFavoriteHubEntry(Text::fromT(server));
	if(fhe != NULL){
		//retrieve window position
		CRect rc(fhe->getLeft(), fhe->getTop(), fhe->getRight(), fhe->getBottom());
		
		//check that we have a window position stored
		if(! (rc.top == 0 && rc.bottom == 0 && rc.left == 0 && rc.right == 0) )
			MoveWindow(rc, TRUE);
	}

	TimerManager::getInstance()->addListener(this);

	return 1;
}

void HubFrame::openWindow(const tstring& aServer) {
	FrameIter i = frames.find(aServer);
	if(i == frames.end()) {
		HubFrame* frm = new HubFrame(aServer);
		frames[aServer] = frm;
		frm->CreateEx(WinUtil::mdiClient);
	} else {
		if(::IsIconic(i->second->m_hWnd))
			::ShowWindow(i->second->m_hWnd, SW_RESTORE);
		i->second->MDIActivate(i->second->m_hWnd);
	}
}

void HubFrame::onEnter() {
	if(ctrlMessage.GetWindowTextLength() > 0) {
		AutoArray<TCHAR> msg(ctrlMessage.GetWindowTextLength()+1);
		ctrlMessage.GetWindowText(msg, ctrlMessage.GetWindowTextLength()+1);
		tstring s(msg, ctrlMessage.GetWindowTextLength());

		// save command in history, reset current buffer pointer to the newest command
		curCommandPosition = prevCommands.size();		//this places it one position beyond a legal subscript
		if (!curCommandPosition || curCommandPosition > 0 && prevCommands[curCommandPosition - 1] != s) {
			++curCommandPosition;
			prevCommands.push_back(s);
		}
		currentCommand = _T("");

		// Special command
		if(s[0] == _T('/')) {
			tstring cmd = s;
			tstring param;
			tstring message;
			tstring status;
			if(WinUtil::checkCommand(cmd, param, message, status)) {
				if(!message.empty()) {
					client->hubMessage(Text::fromT(message));
				}
				if(!status.empty()) {
					addClientLine(status);
				}
			} else if(Util::stricmp(cmd.c_str(), _T("join"))==0) {
				if(!param.empty()) {
					redirect = param;
					if(BOOLSETTING(SETTINGS_OPEN_NEW_WINDOW)) {
						HubFrame::openWindow(param);
					} else {
						BOOL whatever = FALSE;
						onFollow(0, 0, 0, whatever);
					}
				} else {
					addClientLine(TSTRING(SPECIFY_SERVER));
				}
			} else if(Util::stricmp(cmd.c_str(), _T("clear")) == 0) {
				ctrlClient.SetWindowText(_T(""));
			} else if(Util::stricmp(cmd.c_str(), _T("ts")) == 0) {
				timeStamps = !timeStamps;
				if(timeStamps) {
					addClientLine(TSTRING(TIMESTAMPS_ENABLED));
				} else {
					addClientLine(TSTRING(TIMESTAMPS_DISABLED));
				}
			} else if( (Util::stricmp(cmd.c_str(), _T("password")) == 0) && waitingForPW ) {
				client->setPassword(Text::fromT(param));
				client->password(Text::fromT(param));
				waitingForPW = false;
			} else if( Util::stricmp(cmd.c_str(), _T("showjoins")) == 0 ) {
				showJoins = !showJoins;
				if(showJoins) {
					addClientLine(TSTRING(JOIN_SHOWING_ON));
				} else {
					addClientLine(TSTRING(JOIN_SHOWING_OFF));
				}
			} else if( Util::stricmp(cmd.c_str(), _T("favshowjoins")) == 0 ) {
				favShowJoins = !favShowJoins;
				if(favShowJoins) {
					addClientLine(TSTRING(FAV_JOIN_SHOWING_ON));
				} else {
					addClientLine(TSTRING(FAV_JOIN_SHOWING_OFF));
				}
			} else if(Util::stricmp(cmd.c_str(), _T("close")) == 0) {
				PostMessage(WM_CLOSE);
			} else if(Util::stricmp(cmd.c_str(), _T("userlist")) == 0) {
				ctrlShowUsers.SetCheck(showUsers ? BST_UNCHECKED : BST_CHECKED);
			} else if(Util::stricmp(cmd.c_str(), _T("connection")) == 0) {
				addClientLine(Text::toT((STRING(IP) + client->getLocalIp() + ", " + STRING(PORT) + Util::toString(ConnectionManager::getInstance()->getPort()) + "/" + Util::toString(SearchManager::getInstance()->getPort()))));
			} else if((Util::stricmp(cmd.c_str(), _T("favorite")) == 0) || (Util::stricmp(cmd.c_str(), _T("fav")) == 0)) {
				addAsFavorite();
			} else if(Util::stricmp(cmd.c_str(), _T("getlist")) == 0){
				if( !param.empty() ){
					int k = ctrlUsers.findItem(param);
					if(k != -1) {
						ctrlUsers.getItemData(k)->getList();
					}
				}
			} else if(Util::stricmp(cmd.c_str(), _T("log")) == 0) {
				StringMap params;
				params["hub"] = client->getHubName();
				params["hubaddr"] = client->getHubUrl();
				params["mynick"] = client->getMyNick(); 
				if(param.empty()) {
					WinUtil::openFile(Text::toT(Util::validateFileName(SETTING(LOG_DIRECTORY) + Util::formatParams(SETTING(LOG_FILE_MAIN_CHAT), params))));
				} else if(Util::stricmp(param.c_str(), _T("status")) == 0) {
					WinUtil::openFile(Text::toT(Util::validateFileName(SETTING(LOG_DIRECTORY) + Util::formatParams(SETTING(LOG_FILE_STATUS), params))));
				}
			} else if(Util::stricmp(cmd.c_str(), _T("help")) == 0) {
				addLine(_T("*** ") + WinUtil::commands + _T(", /join <hub-ip>, /clear, /ts, /showjoins, /favshowjoins, /close, /userlist, /connection, /favorite, /pm <user> [message], /getlist <user>, /log <status, system, downloads, uploads>"));
			} else if(Util::stricmp(cmd.c_str(), _T("pm")) == 0) {
				string::size_type j = param.find(_T(' '));
				if(j != string::npos) {
					tstring nick = param.substr(0, j);
					int k = ctrlUsers.findItem(nick);
					if(k != -1) {
						UserInfo* ui = ctrlUsers.getItemData(k);
						if(param.size() > j + 1)
							PrivateFrame::openWindow(ui->user, param.substr(j+1));
						else
							PrivateFrame::openWindow(ui->user);
					}
				} else if(!param.empty()) {
					int k = ctrlUsers.findItem(param);
					if(k != -1) {
						UserInfo* ui = ctrlUsers.getItemData(k);
						PrivateFrame::openWindow(ui->user);
					}
				}
			} else {
				if (BOOLSETTING(SEND_UNKNOWN_COMMANDS)) {
					client->hubMessage(Text::fromT(s));
				} else {
					addClientLine(TSTRING(UNKNOWN_COMMAND) + cmd);
				}
			}
			ctrlMessage.SetWindowText(_T(""));
		} else if(waitingForPW) {
			addClientLine(TSTRING(DONT_REMOVE_SLASH_PASSWORD));
			ctrlMessage.SetWindowText(_T("/password "));
			ctrlMessage.SetFocus();
			ctrlMessage.SetSel(10, 10);
		} else {
			client->hubMessage(Text::fromT(s));
			ctrlMessage.SetWindowText(_T(""));
		}
	} else {
		MessageBeep(MB_ICONEXCLAMATION);
	}
}

struct CompareItems {
	CompareItems(int aCol) : col(aCol) { }
	bool operator()(const HubFrame::UserInfo& a, const HubFrame::UserInfo& b) const {
		return HubFrame::UserInfo::compareItems(&a, &b, col) < 0;
	}
	const int col;
};

int HubFrame::findUser(const User::Ptr& aUser) {
	UserMapIter i = userMap.find(aUser);
	if(i == userMap.end())
		return -1;

	if(ctrlUsers.getSortColumn() == COLUMN_NICK) {
		// Sort order of the other columns changes too late when the user's updated
		UserInfo* ui = i->second;
		dcassert(ctrlUsers.getItemData(ctrlUsers.getSortPos(ui)) == ui);
		return ctrlUsers.getSortPos(ui);
	}
	return ctrlUsers.findItem(Text::toT(aUser->getFirstNick()));
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

LRESULT HubFrame::onCopyNick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int i=-1;
	if(client->isConnected()) {
		string nicks;

		while( (i = ctrlUsers.GetNextItem(i, LVNI_SELECTED)) != -1) {
			nicks += (ctrlUsers.getItemData(i))->user->getFirstNick();
			nicks += ' ';
		}
		if(!nicks.empty()) {
			// remove last space
			nicks.erase(nicks.length() - 1);
			WinUtil::setClipboard(Text::toT(nicks));
		}
	}
	return 0;
}

LRESULT HubFrame::onDoubleClickUsers(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NMITEMACTIVATE* item = (NMITEMACTIVATE*)pnmh;
	if(client->isConnected() && item->iItem != -1) {
		ctrlUsers.getItemData(item->iItem)->getList();
	}
	return 0;
}


bool HubFrame::updateUser(const UpdateInfo& u) {
	UserMapIter i = userMap.find(u.user);
	if(i == userMap.end()) {
		UserInfo* ui = new UserInfo(u);
		userMap.insert(make_pair(u.user, ui));
		if(!ui->getHidden() && showUsers)
			ctrlUsers.insertItem(ui, getImage(u.identity));
		return true;
	} else {
		UserInfo* ui = i->second;
		if(!ui->getHidden() && u.identity.isHidden() && showUsers) {
			ctrlUsers.deleteItem(ui);
		}
		
		resort = ui->update(u.identity, ctrlUsers.getSortColumn()) || resort;
		if(showUsers) {
			int pos = ctrlUsers.findItem(ui);
			dcassert(pos != -1);
			ctrlUsers.updateItem(pos);
			ctrlUsers.SetItem(pos, 0, LVIF_IMAGE, NULL, getImage(u.identity), 0, 0, NULL);
		}

		return false;
	}
}

void HubFrame::removeUser(const User::Ptr& aUser) {
	UserMapIter i = userMap.find(aUser);
	dcassert(i != userMap.end());

	UserInfo* ui = i->second;
	if(!ui->getHidden() && showUsers)
		ctrlUsers.deleteItem(ui);

	userMap.erase(i);
	delete ui;
}

bool HubFrame::UserInfo::update(const Identity& identity, int sortCol) {
	bool needsSort = (op != identity.isOp());
	tstring old;
	if(sortCol != -1)
		old = columns[sortCol];

	columns[COLUMN_NICK] = Text::toT(identity.getNick());
	columns[COLUMN_SHARED] = Text::toT(Util::formatBytes(identity.getBytesShared()));
	columns[COLUMN_DESCRIPTION] = Text::toT(identity.getDescription());
	columns[COLUMN_TAG] = Text::toT(identity.getTag());
	/// @todo columns[COLUMN_CONNECTION] = Text::toT(i->getConnection());
	columns[COLUMN_EMAIL] = Text::toT(identity.getEmail());

	op = identity.isOp();
	hidden = identity.isHidden();

	if(sortCol != -1) {
		needsSort = needsSort || (old != columns[sortCol]);
	}

	setIdentity(identity);
	return needsSort;
}

LRESULT HubFrame::onSpeaker(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	if(wParam == UPDATE_USERS) {
		ctrlUsers.SetRedraw(FALSE);
		{
			Lock l(updateCS);
			for(UpdateIter i = updateList.begin(); i != updateList.end(); ++i) {
				UpdateInfo& u = i->first;
				switch(i->second) {
				case UPDATE_USER:
					if(updateUser(u)) {
						if (showJoins || (favShowJoins && FavoriteManager::getInstance()->isFavoriteUser(u.user))) {
							addLine(_T("*** ") + TSTRING(JOINS) + Text::toT(u.identity.getNick()));
						} 
					}
					break;
				case UPDATE_USERS:
					updateUser(u);
					break;
				case REMOVE_USER:
					removeUser(u.user);
					if (showJoins || (favShowJoins && FavoriteManager::getInstance()->isFavoriteUser(u.user))) {
						addLine(Text::toT("*** " + STRING(PARTS) + u.identity.getNick()));
					}

					break;
				}
			}
			updateList.clear();
		}

		if(resort && showUsers) {
			ctrlUsers.resort();
			resort = false;
		}

		ctrlUsers.SetRedraw(TRUE);
	} else if(wParam == DISCONNECTED) {
		clearUserList();
		setTabColor(RGB(255, 0, 0));
	} else if(wParam == CONNECTED) {
		addClientLine(TSTRING(CONNECTED));
		setTabColor(RGB(0, 255, 0));
	} else if(wParam == ADD_CHAT_LINE) {
		tstring* x = (tstring*)lParam;
		addLine(*x);
		delete x;
	} else if(wParam == ADD_STATUS_LINE) {
		tstring* x = (tstring*)lParam;
		addClientLine(*x);
		delete x;
	} else if(wParam == ADD_SILENT_STATUS_LINE) {
		tstring* x = (tstring*)lParam;
		addClientLine(*x, false);
		delete x;
	} else if(wParam == SET_WINDOW_TITLE) {
		tstring* x = (tstring*)lParam;
		SetWindowText(x->c_str());
		delete x;
	} else if(wParam == STATS) {
		ctrlStatus.SetText(1, Text::toT(Util::toString(client->getUserCount()) + " " + STRING(HUB_USERS)).c_str());
		ctrlStatus.SetText(2, Text::toT(Util::formatBytes(client->getAvailable())).c_str());
	} else if(wParam == GET_PASSWORD) {
		if(client->getPassword().size() > 0) {
			client->password(client->getPassword());
			addClientLine(TSTRING(STORED_PASSWORD_SENT));
		} else {
			ctrlMessage.SetWindowText(_T("/password "));
			ctrlMessage.SetFocus();
			ctrlMessage.SetSel(10, 10);
			waitingForPW = true;
		}
	} else if(wParam == PRIVATE_MESSAGE) {
		PMInfo* i = (PMInfo*)lParam;
		if(i->user->isOnline()) {
			if(BOOLSETTING(POPUP_PMS) || PrivateFrame::isOpen(i->user)) {
				PrivateFrame::gotMessage(i->user, i->msg);
			} else {
				addLine(TSTRING(PRIVATE_MESSAGE_FROM) + Text::toT(i->user->getFirstNick()) + _T(": ") + i->msg);
			}
		} else {
			if(BOOLSETTING(IGNORE_OFFLINE)) {
				addClientLine(TSTRING(IGNORED_MESSAGE) + i->msg, false);
			} else if(BOOLSETTING(POPUP_OFFLINE)) {
				PrivateFrame::gotMessage(i->user, i->msg);
			} else {
				addLine(TSTRING(PRIVATE_MESSAGE_FROM) + Text::toT(i->user->getFirstNick()) + _T(": ") + i->msg);
			}
		}
		delete i;
	}

	return 0;
};

void HubFrame::UpdateLayout(BOOL bResizeBars /* = TRUE */) {
	RECT rect;
	GetClientRect(&rect);
	// position bars and offset their dimensions
	UpdateBarsPosition(rect, bResizeBars);
	
	if(ctrlStatus.IsWindow()) {
		CRect sr;
		int w[4];
		ctrlStatus.GetClientRect(sr);

		int tmp = (sr.Width()) > 332 ? 232 : ((sr.Width() > 132) ? sr.Width()-100 : 32);
		
		w[0] = sr.right - tmp;
		w[1] = w[0] + (tmp-32)/2;
		w[2] = w[0] + (tmp-32);
		w[3] = w[2] + 16;
		
		ctrlStatus.SetParts(4, w);

		ctrlLastLines.SetMaxTipWidth(w[0]);

		// Strange, can't get the correct width of the last field...
		ctrlStatus.GetRect(2, sr);
		sr.left = sr.right + 2;
		sr.right = sr.left + 16;
		ctrlShowUsers.MoveWindow(sr);
	}
	int h = WinUtil::fontHeight + 4;

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
	rc.right -=2;
	ctrlMessage.MoveWindow(rc);
}

LRESULT HubFrame::onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	if(!closed) {
		TimerManager::getInstance()->removeListener(this);
		client->removeListener(this);
		client->disconnect();

		closed = true;
		PostMessage(WM_CLOSE);
		return 0;
	} else {
		SettingsManager::getInstance()->set(SettingsManager::GET_USER_INFO, showUsers);
		FavoriteManager::getInstance()->removeUserCommand(Text::fromT(server));

		clearUserList();
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
			fhe->setBottom((u_int16_t)(rc.bottom > 0 ? rc.bottom : 0));
			fhe->setTop((u_int16_t)(rc.top > 0 ? rc.top : 0));
			fhe->setLeft((u_int16_t)(rc.left > 0 ? rc.left : 0));
			fhe->setRight((u_int16_t)(rc.right > 0 ? rc.right : 0));

			FavoriteManager::getInstance()->save();
		}

		bHandled = FALSE;
		return 0;
	}
}

void HubFrame::clearUserList() {
	{
		Lock l(updateCS);
		updateList.clear();
	}

	for(UserMapIter i = userMap.begin(); i != userMap.end(); ++i) {
		delete i->second;
	}
	ctrlUsers.DeleteAllItems();
	userMap.clear();
}


LRESULT HubFrame::onLButton(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
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
		if (!bHandled) {
			string::size_type end = x.find_first_of(_T(" >\t"), start+1);

			if(end == string::npos) // get EOL as well
				end = x.length();
			else if(end == start + 1)
				return 0;

			// Nickname click, let's see if we can find one like it in the name list...
			int pos = ctrlUsers.findItem(x.substr(start, end - start));
			if(pos != -1) {
				bHandled = true;
				if (wParam & MK_CONTROL) { // MK_CONTROL = 0x0008
					PrivateFrame::openWindow((ctrlUsers.getItemData(pos))->user);
				} else if (wParam & MK_SHIFT) {
					try {
						QueueManager::getInstance()->addList((ctrlUsers.getItemData(pos))->user, QueueItem::FLAG_CLIENT_VIEW);
					} catch(const Exception& e) {
						addClientLine(Text::toT(e.getError()));
					}
				} else {
					int items = ctrlUsers.GetItemCount();
					ctrlUsers.SetRedraw(FALSE);
					for(int i = 0; i < items; ++i) {
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

void HubFrame::addLine(const tstring& aLine) {
	if(ctrlClient.GetWindowTextLength() > 25000) {
		// We want to limit the buffer to 25000 characters...after that, w95 becomes sad...
		ctrlClient.SetRedraw(FALSE);
		ctrlClient.SetSel(0, ctrlClient.LineIndex(ctrlClient.LineFromChar(2000)), TRUE);
		ctrlClient.ReplaceSel(_T(""));
		ctrlClient.SetRedraw(TRUE);
	}
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
		params["hub"] = client->getHubName();
		params["hubaddr"] = client->getHubUrl();
		params["mynick"] = client->getMyNick(); 
		LOG(LogManager::CHAT, params);
	}
	if(timeStamps) {
		ctrlClient.AppendText((Text::toT("\r\n[" + Util::getShortTimeString() + "] ") + aLine).c_str());
	} else {
		ctrlClient.AppendText((_T("\r\n") + aLine).c_str());
	}
	if(noscroll) {
		ctrlClient.SetRedraw(TRUE);
	}
	if (BOOLSETTING(TAB_HUB_DIRTY)) {
		setDirty();
	}
}

LRESULT HubFrame::onTabContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };        // location of mouse click 
	tabMenuShown = true;
	prepareMenu(tabMenu, ::UserCommand::CONTEXT_HUB, Text::toT(client->getHubUrl()), client->getMyIdentity().isOp());
	tabMenu.AppendMenu(MF_SEPARATOR);
	tabMenu.AppendMenu(MF_STRING, IDC_CLOSE_WINDOW, CTSTRING(CLOSE));
	tabMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_BOTTOMALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
	tabMenu.DeleteMenu(tabMenu.GetMenuItemCount()-1, MF_BYPOSITION);
	tabMenu.DeleteMenu(tabMenu.GetMenuItemCount()-1, MF_BYPOSITION);
	cleanMenu(tabMenu);
	return TRUE;
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
		string::size_type start = (string::size_type)WinUtil::textUnderCursor(pt, ctrlClient, x);
		ctrlClient.ClientToScreen(&pt);

		string::size_type end = x.find_first_of(_T(" >\t"), start+1);
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
		prepareMenu(userMenu, ::UserCommand::CONTEXT_CHAT, Text::toT(client->getHubUrl()), client->getMyIdentity().isOp());
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
	if(!WinUtil::getUCParams(m_hWnd, uc, ucParams))
		return;

	client->getMyIdentity().getParams(ucParams, "my");

	if(tabMenuShown) {
		client->escapeParams(ucParams);
		client->sendUserCmd(Util::formatParams(uc.getCommand(), ucParams));
	} else {
		int sel = -1;
		while((sel = ctrlUsers.GetNextItem(sel, LVNI_SELECTED)) != -1) {
			UserInfo* u = (UserInfo*) ctrlUsers.GetItemData(sel);
			StringMap tmp = ucParams;

			u->getIdentity().getParams(tmp, "user");
			client->escapeParams(tmp);
			client->sendUserCmd(Util::formatParams(uc.getCommand(), tmp)); 
		}
	}
	return;
};

void HubFrame::onTab() {
	if(	BOOLSETTING(TAB_COMPLETION) && 
		(GetFocus() == ctrlMessage.m_hWnd) && 
		!(GetAsyncKeyState(VK_SHIFT) & 0x8000) ) 
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

		int start = ctrlUsers.GetNextItem(-1, LVNI_FOCUSED) + 1;
		int i = start;
		int j = ctrlUsers.GetItemCount();

		bool firstPass = i < j;
		if(!firstPass)
			i = 0;
		while(firstPass || (!firstPass && i < start)) {
			UserInfo* ui = ctrlUsers.getItemData(i);
			const tstring& nick = ui->columns[COLUMN_NICK];
			bool found = (Util::strnicmp(nick, complete, complete.length()) == 0);
			tstring::size_type x = 0;
			if(!found) {
				// Check if there's one or more [ISP] tags to ignore...
				tstring::size_type y = 0;
				while(nick[y] == _T('[')) {
					x = nick.find(_T(']'), y);
					if(x != string::npos) {
						if(Util::strnicmp(nick.c_str() + x + 1, complete.c_str(), complete.length()) == 0) {
							found = true;
							break;
						}
					} else {
						break;
					}
					y = x + 1; // assuming that nick[y] == '\0' is legal
				}
			}
			if(found) {
				if((start - 1) != -1) {
					ctrlUsers.SetItemState(start - 1, 0, LVNI_SELECTED | LVNI_FOCUSED);
				}
				ctrlUsers.SetItemState(i, LVNI_FOCUSED | LVNI_SELECTED, LVNI_FOCUSED | LVNI_SELECTED);
				ctrlUsers.EnsureVisible(i, FALSE);
				ctrlMessage.SetSel(textStart, ctrlMessage.GetWindowTextLength(), TRUE);
				ctrlMessage.ReplaceSel(nick.c_str());
				return;
			}
			i++;
			if(i == j) {
				firstPass = false;
				i = 0;
			}
		}
	} else {
		HWND focus = GetFocus();

		if(focus == ctrlClient.m_hWnd) {
			ctrlMessage.SetFocus();
		} else if(focus == ctrlMessage.m_hWnd) {
			ctrlUsers.SetFocus();
		} else if(focus == ctrlUsers.m_hWnd) {
			ctrlClient.SetFocus();
		} 
	}
}

LRESULT HubFrame::onChar(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled) {
	if(!complete.empty() && wParam != VK_TAB && uMsg == WM_KEYDOWN)
		complete.clear();

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
			if ( (GetKeyState(VK_MENU) & 0x8000) ||	( ((GetKeyState(VK_CONTROL) & 0x8000) == 0) ^ (BOOLSETTING( SETTINGS_USE_CTRL_FOR_LINE_HISTORY ) == true) ) ) {
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
			if ( (GetKeyState(VK_MENU) & 0x8000) ||	( ((GetKeyState(VK_CONTROL) & 0x8000) == 0) ^ (BOOLSETTING( SETTINGS_USE_CTRL_FOR_LINE_HISTORY ) == true) ) ) {
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
}

LRESULT HubFrame::onShowUsers(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled) {
	bHandled = FALSE;
	if((wParam == BST_CHECKED)) {
		showUsers = true;
		ctrlUsers.SetRedraw(FALSE);
		ctrlUsers.DeleteAllItems();
		
		for(UserMapIter i = userMap.begin(); i != userMap.end(); ++i) {
			UserInfo* ui = i->second;
			if(!ui->getHidden())
				ctrlUsers.insertItem(ui, getImage(ui->getIdentity()));
		}

		ctrlUsers.SetRedraw(TRUE);
		ctrlUsers.resort();
	} else {
		showUsers = false;
	}

	SettingsManager::getInstance()->set(SettingsManager::GET_USER_INFO, showUsers);

	UpdateLayout(FALSE);
	return 0;
}

LRESULT HubFrame::onFollow(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	
	if(!redirect.empty()) {
		string s, f;
		u_int16_t p = 411;
		Util::decodeUrl(Text::fromT(redirect), s, p, f);
		if(ClientManager::getInstance()->isConnected(s, p)) {
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
		client = ClientManager::getInstance()->getClient(Text::fromT(server));
		client->addListener(this);
		client->connect();
	}
	return 0;
}

LRESULT HubFrame::onEnterUsers(int /*idCtrl*/, LPNMHDR /* pnmh */, BOOL& /*bHandled*/) {
	int item = ctrlUsers.GetNextItem(-1, LVNI_FOCUSED);
	if(client->isConnected() && (item != -1)) {
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

void HubFrame::addClientLine(const tstring& aLine, bool inChat /* = true */) {
	tstring line = Text::toT("[" + Util::getShortTimeString() + "] ") + aLine;

	ctrlStatus.SetText(0, line.c_str());
	while(lastLinesList.size() + 1 > MAX_CLIENT_LINES)
		lastLinesList.erase(lastLinesList.begin());
	lastLinesList.push_back(line);

	if (BOOLSETTING(TAB_HUB_DIRTY)) {
		setDirty();
	}
	
	if(BOOLSETTING(STATUS_IN_CHAT) && inChat) {
		addLine(_T("*** ") + aLine);
	}
	if(BOOLSETTING(LOG_STATUS_MESSAGES)) {
		StringMap params;
		params["hub"] = client->getHubName();
		params["hubaddr"] = client->getHubUrl();
		params["message"] = Text::fromT(aLine);
		LOG(LogManager::STATUS, params);
	}
}

void HubFrame::closeDisconnected() {
	for(FrameIter i=frames.begin(); i!= frames.end(); ++i) {
		if (!(i->second->client->isConnected())) {
			i->second->PostMessage(WM_CLOSE);
		}
	}
};

void HubFrame::on(TimerManagerListener::Second, DWORD /*aTick*/) throw() {
	updateStatusBar();
	if(updateUsers) {
		updateUsers = false;
		PostMessage(WM_SPEAKER, UPDATE_USERS);
	}
}

void HubFrame::on(Connecting, Client*) throw() { 
	speak(ADD_STATUS_LINE, STRING(CONNECTING_TO) + client->getHubUrl() + "...");
	speak(SET_WINDOW_TITLE, client->getHubUrl());
}
void HubFrame::on(Connected, Client*) throw() { 
	speak(CONNECTED);
}
void HubFrame::on(BadPassword, Client*) throw() { 
	client->setPassword(Util::emptyString);
}
void HubFrame::on(UserUpdated, Client*, const OnlineUser& user) throw() { 
	speak(UPDATE_USER, user);
}
void HubFrame::on(UsersUpdated, Client*, const OnlineUser::List& aList) throw() {
	Lock l(updateCS);
	updateList.reserve(aList.size());
	for(OnlineUser::List::const_iterator i = aList.begin(); i != aList.end(); ++i) {
		if(!(*i)->getIdentity().isHidden())
			updateList.push_back(make_pair(UpdateInfo(*(*i)), UPDATE_USERS));
	}
	if(!updateList.empty()) {
		PostMessage(WM_SPEAKER, UPDATE_USERS);
	}
}

void HubFrame::on(UserRemoved, Client*, const OnlineUser& user) throw() {
	speak(REMOVE_USER, user);
}

void HubFrame::on(Redirect, Client*, const string& line) throw() { 
	string s, f;
	u_int16_t p = 411;
	Util::decodeUrl(line, s, p, f);
	if(ClientManager::getInstance()->isConnected(s, p)) {
		speak(ADD_STATUS_LINE, STRING(REDIRECT_ALREADY_CONNECTED));
		return;
	}

	redirect = Text::toT(line);
	if(BOOLSETTING(AUTO_FOLLOW)) {
		PostMessage(WM_COMMAND, IDC_FOLLOW, 0);
	} else {
		speak(ADD_STATUS_LINE, STRING(PRESS_FOLLOW) + line);
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
	speak(SET_WINDOW_TITLE, Util::validateMessage(client->getHubName(), true, false) + " (" + client->getHubUrl() + ")");
}
void HubFrame::on(Message, Client*, const string& line) throw() { 
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
void HubFrame::on(PrivateMessage, Client*, const OnlineUser& user, const string& line) throw() { 
	speak(PRIVATE_MESSAGE, user, Util::toDOS(line));
}
void HubFrame::on(NickTaken, Client*) throw() { 
	speak(ADD_STATUS_LINE, STRING(NICK_TAKEN));
	speak(DISCONNECTED);
}
void HubFrame::on(SearchFlood, Client*, const string& line) throw() {
	speak(ADD_STATUS_LINE, STRING(SEARCH_SPAM_FROM) + line);
}

/**
 * @file
 * $Id: HubFrame.cpp,v 1.112 2005/08/07 13:05:47 arnetheduck Exp $
 */

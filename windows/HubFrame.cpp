/* 
 * Copyright (C) 2001 Jacek Sieka, j_s@telia.com
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

#include "../client/QueueManager.h"
#include "../client/ShareManager.h"
#include "../client/Util.h"
#include "../client/UploadManager.h"
#include "../client/StringTokenizer.h"
#include "../client/HubManager.h"
#include "../client/LogManager.h"

CImageList* HubFrame::images = NULL;
HubFrame::FrameMap HubFrame::frames;

#define LINE2 "-- http://dcplusplus.sourceforge.net  <DC++ " VERSIONSTRING ">"
char *msgs[] = { "\r\n-- I'm a happy dc++ user. You could be happy too.\r\n" LINE2,
"\r\n-- Neo-...what? Nope...never heard of it...\r\n" LINE2,
"\r\n-- Evolution of species: Ape --> Man\r\n-- Evolution of science: \"The Earth is Flat\" --> \"The Earth is Round\"\r\n-- Evolution of sharing: NMDC --> DC++\r\n" LINE2,
"\r\n-- I share, therefore I am.\r\n" LINE2,
"\r\n-- I came, I searched, I found...\r\n" LINE2,
"\r\n-- I came, I shared, I sent...\r\n" LINE2,
"\r\n-- I can set away mode, can't you?\r\n" LINE2,
"\r\n-- I don't have to see any ads, do you?\r\n" LINE2,
"\r\n-- I don't have to see those annoying kick messages, do you?\r\n" LINE2,
"\r\n-- I can resume my files to a different filename, can you?\r\n" LINE2,
"\r\n-- I can share huge amounts of files, can you?\r\n" LINE2,
"\r\n-- My client doesn't spam the chat with useless debug messages, does yours?\r\n" LINE2,
"\r\n-- I can add multiple users to the same download and have the client connect to another automatically when one goes offline, can you?\r\n" LINE2,
"\r\n-- These addies are pretty annoying, aren't they? Get revenge by sending them yourself!\r\n" LINE2
};

#define MSGS 14

int HubFrame::columnSizes[] = { 100, 75, 100, 75, 100 };
int HubFrame::columnIndexes[] = { COLUMN_NICK, COLUMN_SHARED, COLUMN_DESCRIPTION, COLUMN_CONNECTION, COLUMN_EMAIL };
static ResourceManager::Strings columnNames[] = { ResourceManager::NICK, ResourceManager::SHARED,
	ResourceManager::DESCRIPTION, ResourceManager::CONNECTION, ResourceManager::EMAIL };


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
	
	ctrlMessage.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE, WS_EX_CLIENTEDGE);
	
	ctrlMessageContainer.SubclassWindow(ctrlMessage.m_hWnd);
	
	ctrlUsers.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS, WS_EX_CLIENTEDGE, IDC_USERS);
	
	if(BOOLSETTING(FULL_ROW_SELECT)) {
		ctrlUsers.SetExtendedListViewStyle(LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT);
	} else {
		ctrlUsers.SetExtendedListViewStyle(LVS_EX_HEADERDRAGDROP);
	}
	
	ctrlMessage.SetFont(ctrlUsers.GetFont());

	SetSplitterPanes(ctrlClient.m_hWnd, ctrlUsers.m_hWnd, false);
	SetSplitterExtendedStyle(SPLIT_PROPORTIONAL);
	m_nProportionalPos = 7500;

	ctrlShowUsers.Create(ctrlStatus.m_hWnd, rcDefault, "+/-", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	ctrlShowUsers.SetButtonStyle(BS_AUTOCHECKBOX, false);
	ctrlShowUsers.SetFont(ctrlStatus.GetFont());
	ctrlShowUsers.SetCheck(client->getUserInfo());
	showUsersContainer.SubclassWindow(ctrlShowUsers.m_hWnd);

	WinUtil::splitTokens(columnIndexes, SETTING(HUBFRAME_ORDER), COLUMN_LAST);
	WinUtil::splitTokens(columnSizes, SETTING(HUBFRAME_WIDTHS), COLUMN_LAST);
	
	for(int j=0; j<COLUMN_LAST; j++) {
		int fmt = (j == COLUMN_SHARED) ? LVCFMT_RIGHT : LVCFMT_LEFT;
		ctrlUsers.InsertColumn(j, CSTRING_I(columnNames[j]), fmt, columnSizes[j], j);
	}
	
	ctrlUsers.SetColumnOrderArray(COLUMN_LAST, columnIndexes);
	
	ctrlUsers.SetBkColor(WinUtil::bgColor);
	ctrlUsers.SetTextBkColor(WinUtil::bgColor);
	ctrlUsers.SetTextColor(WinUtil::textColor);
	
	ctrlUsers.setSort(COLUMN_NICK, ExListViewCtrl::SORT_FUNC, true, sortNick);
				
	if(!images) {
		images = new CImageList();
		images->CreateFromImage(IDB_USERS, 16, 8, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION | LR_SHARED);
	}
	ctrlUsers.SetImageList(*images, LVSIL_SMALL);

	userMenu.CreatePopupMenu();
	userMenu.AppendMenu(MF_STRING, IDC_GETLIST, CSTRING(GET_FILE_LIST));
	userMenu.AppendMenu(MF_STRING, IDC_PRIVATEMESSAGE, CSTRING(SEND_PRIVATE_MESSAGE));
	userMenu.AppendMenu(MF_STRING, IDC_GRANTSLOT, CSTRING(GRANT_EXTRA_SLOT));
	userMenu.AppendMenu(MF_STRING, IDC_ADD_TO_FAVORITES, CSTRING(ADD_TO_FAVORITES));
	userMenu.AppendMenu(MF_STRING, IDC_COPY_NICK, CSTRING(COPY_NICK));
	userMenu.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)NULL);
	userMenu.AppendMenu(MF_STRING, IDC_REFRESH, CSTRING(REFRESH_USER_LIST));

	opMenu.CreatePopupMenu();
	opMenu.AppendMenu(MF_STRING, IDC_GETLIST, CSTRING(GET_FILE_LIST));
	opMenu.AppendMenu(MF_STRING, IDC_PRIVATEMESSAGE, CSTRING(SEND_PRIVATE_MESSAGE));
	opMenu.AppendMenu(MF_STRING, IDC_GRANTSLOT, CSTRING(GRANT_EXTRA_SLOT));
	opMenu.AppendMenu(MF_STRING, IDC_COPY_NICK, CSTRING(COPY_NICK));
	opMenu.AppendMenu(MF_STRING, IDC_ADD_TO_FAVORITES, CSTRING(ADD_TO_FAVORITES));
	opMenu.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)NULL);
	opMenu.AppendMenu(MF_STRING, IDC_REFRESH, CSTRING(REFRESH_USER_LIST));
	opMenu.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)NULL);
	opMenu.AppendMenu(MF_STRING, IDC_KICK, CSTRING(KICK_USER));
	opMenu.AppendMenu(MF_STRING, IDC_REDIRECT, CSTRING(REDIRECT));
	
	showJoins = BOOLSETTING(SHOW_JOINS);

	bHandled = FALSE;
	client->connect(server);
	return 1;
}

void HubFrame::openWindow(HWND aParent, FlatTabCtrl* aTab, const string& aServer, const string& aNick /* = Util::emptyString */, const string& aPassword /* = Util::emptyString */) {
	FrameIter i = frames.find(aServer);
	if(i == frames.end()) {
		HubFrame* frm = new HubFrame(aServer, aNick, aPassword);
		frames[aServer] = frm;
		frm->setTab(aTab);
		frm->CreateEx(aParent);
	} else {
		i->second->MDIActivate(i->second->m_hWnd);
	}
}

void HubFrame::onEnter() {
	char* message;
	
	if(ctrlMessage.GetWindowTextLength() > 0) {
		message = new char[ctrlMessage.GetWindowTextLength()+1];
		ctrlMessage.GetWindowText(message, ctrlMessage.GetWindowTextLength()+1);
		string s(message, ctrlMessage.GetWindowTextLength());
		delete[] message;

		// Special command
		if(s[0] == '/') {
			string param;
			int i = s.find(' ');
			if(i != string::npos) {
				param = s.substr(i+1);
				s = s.substr(1, i - 1);
			} else {
				s = s.substr(1);
			}

			if(Util::stricmp(s.c_str(), "refresh")==0) {
				try {
					ShareManager::getInstance()->setDirty();
					ShareManager::getInstance()->refresh(true);
					addClientLine(STRING(FILE_LIST_REFRESHED));
				} catch(ShareException e) {
					addClientLine(e.getError());
				}
			} else if(Util::stricmp(s.c_str(), "slots")==0) {
				int j = Util::toInt(param);
				if(j > 0) {
					SettingsManager::getInstance()->set(SettingsManager::SLOTS, j);
					addClientLine(STRING(SLOTS_SET));
					ClientManager::getInstance()->infoUpdated();
				} else {
					addClientLine(STRING(INVALID_NUMBER_OF_SLOTS));
				}
			} else if(Util::stricmp(s.c_str(), "join")==0) {
				if(!param.empty()) {
					client->connect(param);
				} else {
					addClientLine(STRING(SPECIFY_SERVER));
				}
			} else if(Util::stricmp(s.c_str(), "search") == 0) {
				if(!param.empty()) {
					SearchFrame* pChild = new SearchFrame();
					pChild->setTab(getTab());
					pChild->setInitial(param, 0, SearchManager::SIZE_ATLEAST);
					pChild->CreateEx(m_hWndMDIClient);
				} else {
					addClientLine(STRING(SPECIFY_SEARCH_STRING));
				}
			} else if(Util::stricmp(s.c_str(), "dc++") == 0) {
				client->sendMessage(msgs[GET_TICK() % MSGS]);
			} else if(Util::stricmp(s.c_str(), "clear") == 0) {
				ctrlClient.SetWindowText("");
			} else if(Util::stricmp(s.c_str(), "away") == 0) {
				if(Util::getAway() && param.empty()) {
					Util::setAway(false);
					addClientLine(STRING(AWAY_MODE_OFF));
				} else {
					Util::setAway(true);
					Util::setAwayMessage(param);
					addClientLine(STRING(AWAY_MODE_ON) + Util::getAwayMessage());
				}
			} else if(Util::stricmp(s.c_str(), "back") == 0) {
				Util::setAway(false);
				addClientLine(STRING(AWAY_MODE_OFF));
			} else if(Util::stricmp(s.c_str(), "ts") == 0) {
				timeStamps = !timeStamps;
				if(timeStamps) {
					addClientLine(STRING(TIMESTAMPS_ENABLED));
				} else {
					addClientLine(STRING(TIMESTAMPS_DISABLED));
				}
			} else if( (Util::stricmp(s.c_str(), "password") == 0) && waitingForPW ) {
				client->setPassword(param);
				client->password(param);
				waitingForPW = false;
			} else if( Util::stricmp(s.c_str(), "showjoins") == 0 ) {
				showJoins = !showJoins;
				if(showJoins) {
					addClientLine(STRING(JOIN_SHOWING_ON));
				} else {
					addClientLine(STRING(JOIN_SHOWING_OFF));
				}
			} else if(Util::stricmp(s.c_str(), "close") == 0) {
				PostMessage(WM_CLOSE);
			} else if(Util::stricmp(s.c_str(), "userlist") == 0) {
				ctrlShowUsers.SetCheck(client->getUserInfo() ? BST_UNCHECKED : BST_CHECKED);
			} else if(Util::stricmp(s.c_str(), "help") == 0) {
				addLine("/clear, /refresh, /slots #, /join <hub-ip>, /search <string>, /dc++, /away <msg>, /back, /ts, /password, /showjoins, /close, /help, /userlist, /connection");
			} else if(Util::stricmp(s.c_str(), "connection") == 0) {
				addLine(STRING(IP) + SETTING(SERVER) + ", " + STRING(PORT) + Util::toString(SETTING(IN_PORT)));
			}
		} else {
			client->sendMessage(s);
		}
		ctrlMessage.SetWindowText("");
	} else {
		MessageBeep(MB_ICONEXCLAMATION);
	}
}

LRESULT HubFrame::onGetList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int i=-1;
	if(client->isConnected()) {
		while( (i = ctrlUsers.GetNextItem(i, LVNI_SELECTED)) != -1) {
			try {
				QueueManager::getInstance()->addList(((UserInfo*)ctrlUsers.GetItemData(i))->user);
			} catch(Exception e) {
				addClientLine(e.getError());
			}
		}
	}
	return 0;
}

LRESULT HubFrame::onAddToFavorites(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int i=-1;
	if(client->isConnected()) {
		while( (i = ctrlUsers.GetNextItem(i, LVNI_SELECTED)) != -1) {
			HubManager::getInstance()->addFavoriteUser(((UserInfo*)ctrlUsers.GetItemData(i))->user);
		}
	}
	return 0;
}

LRESULT HubFrame::onCopyNick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int i=-1;
	if(client->isConnected()) {
		string nicks;

		while( (i = ctrlUsers.GetNextItem(i, LVNI_SELECTED)) != -1) {
			nicks += ((UserInfo*)ctrlUsers.GetItemData(i))->user->getNick();
			nicks += ' ';
		}
		if(!nicks.empty()) {
			// remove last space
			nicks.erase(nicks.length() - 1);

			if(!OpenClipboard()) {
				return 0;
			}

			EmptyClipboard();
			
			// Allocate a global memory object for the text. 
			HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, (nicks.size() + 1)); 
			if (hglbCopy == NULL) { 
				CloseClipboard(); 
				return FALSE; 
			} 
			
			// Lock the handle and copy the text to the buffer. 
			char* lptstrCopy = (char*)GlobalLock(hglbCopy); 
			memcpy(lptstrCopy, nicks.c_str(), nicks.length() + 1);
			GlobalUnlock(hglbCopy); 
			
			// Place the handle on the clipboard. 
			SetClipboardData(CF_TEXT, hglbCopy); 
			CloseClipboard();
		}
	}
	return 0;
}

LRESULT HubFrame::onPrivateMessage(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int i=-1;
	if(client->isConnected()) {
		while( (i = ctrlUsers.GetNextItem(i, LVNI_SELECTED)) != -1) {
			PrivateFrame::openWindow(((UserInfo*)ctrlUsers.GetItemData(i))->user, m_hWndMDIClient, getTab());
		}
	}
	return 0;
}

LRESULT HubFrame::onDoubleClickUsers(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NMITEMACTIVATE* item = (NMITEMACTIVATE*)pnmh;

	if(client->isConnected() && item->iItem != -1) {
		try {
			QueueManager::getInstance()->addList(((UserInfo*)ctrlUsers.GetItemData(item->iItem))->user);
		} catch(Exception e) {
			addClientLine(e.getError());
		}
	}
	return 0;
}

LRESULT HubFrame::onKick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) { 
	LineDlg dlg;
	dlg.title = STRING(KICK_USER);
	dlg.description = STRING(ENTER_REASON);
	dlg.line = lastKick;
	if(dlg.DoModal() == IDOK) {
		lastKick = dlg.line;
		
		int i = -1;
		int k = 0;
		bool op = false;
		while( (k < 15) && (!op) && ((i = ctrlUsers.GetNextItem(i, LVNI_SELECTED)) != -1) ) {
			UserInfo* ui = (UserInfo*)ctrlUsers.GetItemData(i);
			if(ui->user->isSet(User::OP))
				op = true;
			ui->user->kick(dlg.line);
			k++;
		}
	}
	
	return 0; 
};

LRESULT HubFrame::onGrantSlot(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) { 
	int i = -1;
	while( (i = ctrlUsers.GetNextItem(i, LVNI_SELECTED)) != -1) {
		UploadManager::getInstance()->reserveSlot(((UserInfo*)ctrlUsers.GetItemData(i))->user);
	}
	return 0; 
};

LRESULT HubFrame::onRedirect(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) { 
	LineDlg dlg1, dlg2;
	dlg1.title = STRING(REDIRECT_USER);
	dlg1.description = STRING(ENTER_REASON);
	dlg1.line = lastRedir;

	if(dlg1.DoModal() == IDOK) {
		dlg2.title = STRING(REDIRECT_USER);
		dlg2.description = STRING(ENTER_SERVER);
		dlg2.line = lastServer;
		if(dlg2.DoModal() == IDOK) {
			lastRedir = dlg1.line;
			lastServer = dlg2.line;
			int i = -1;
			while( (i = ctrlUsers.GetNextItem(i, LVNI_SELECTED)) != -1) {
				client->opForceMove(((UserInfo*)ctrlUsers.GetItemData(i))->user, dlg2.line, STRING(YOU_ARE_BEING_REDIRECTED) + dlg2.line + ": " + dlg1.line);
			}
		}
	}
	
	return 0; 
};
bool HubFrame::updateUser(const User::Ptr& u, bool sorted /* = false */, UserInfo* ui /* = NULL */) {
	int i = ctrlUsers.find(u->getNick());
	bool newUser = false;
	if(i == -1) {
		newUser = true;
		if(ui == NULL)
			ui = new UserInfo(u);

		if(sorted) {
			StringList l;
			l.push_back(u->getNick());
			l.push_back(Util::formatBytes(u->getBytesShared()));
			l.push_back(u->getDescription());
			l.push_back(u->getConnection());
			l.push_back(u->getEmail());
			ctrlUsers.insert(l, getImage(u), (LPARAM)ui);
			return newUser;
		} else {
			i = ctrlUsers.insert(ctrlUsers.GetItemCount(), u->getNick(), getImage(u), (LPARAM)ui);
		}
	} else {
		ctrlUsers.SetItem(i, 0, LVIF_IMAGE, NULL, getImage(u), 0, 0, NULL);
	}

	ctrlUsers.SetItemText(i, COLUMN_SHARED, Util::formatBytes(u->getBytesShared()).c_str());
	ctrlUsers.SetItemText(i, COLUMN_DESCRIPTION, u->getDescription().c_str());
	ctrlUsers.SetItemText(i, COLUMN_CONNECTION, u->getConnection().c_str());
	ctrlUsers.SetItemText(i, COLUMN_EMAIL, u->getEmail().c_str());
	if(sorted && ctrlUsers.getSortColumn() != COLUMN_NICK) {
		needSort = true;
	}
	if(!newUser && ui)
		delete ui;
	
	return newUser;
}

LRESULT HubFrame::onSpeaker(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	if(wParam == UPDATE_USER) {
		UserInfo* ui = (UserInfo*)lParam;
		User::Ptr& u = ui->user;
		if(updateUser(u, true, ui) && showJoins) {
			addLine("*** " + STRING(JOINS) + '<' + ui->user->getNick() + '>');
		}
	} else if(wParam == UPDATE_USERS) {
		User::List* ul = (User::List*)lParam;

		ctrlUsers.SetRedraw(FALSE);
		for(User::Iter i = ul->begin(); i != ul->end(); ++i) {
			updateUser(*i);
		}
		ctrlUsers.SetRedraw(TRUE);
		ctrlUsers.resort();

		delete ul;
	} else if(wParam == REMOVE_USER) {
		UserInfo* ui = (UserInfo*)lParam;
		int j = ctrlUsers.find( ui->user->getNick() );
		if( j != -1 ) {
			delete (UserInfo*)ctrlUsers.GetItemData(j);
			ctrlUsers.DeleteItem(j);
		
			if(showJoins) {
				addLine("*** " + STRING(PARTS) + '<' + ui->user->getNick() +'>');
			}
		}
		delete ui;
	} else if(wParam == REMOVE_USERS) {
		clearUserList();
	} else if(wParam == ADD_CHAT_LINE) {
		string* x = (string*)lParam;
		addLine(*x);
		delete x;
	} else if(wParam == ADD_STATUS_LINE) {
		string* x = (string*)lParam;
		addClientLine(*x);
		delete x;
	} else if(wParam == ADD_SILENT_STATUS_LINE) {
		string* x = (string*)lParam;
		addClientLine(*x, false);
		delete x;
	} else if(wParam == SET_WINDOW_TITLE) {
		string* x = (string*)lParam;
		SetWindowText(x->c_str());
		delete x;
	} else if(wParam == STATS) {
		ctrlStatus.SetText(1, (Util::toString(client->getUserCount()) + " " + STRING(HUB_USERS)).c_str());
		if(client->getUserInfo())
			ctrlStatus.SetText(2, Util::formatBytes(client->getAvailable()).c_str());
		else
			ctrlStatus.SetText(2, "");

		if(needSort) {
			ctrlUsers.resort();
			needSort = false;
		}
	} else if(wParam == GET_PASSWORD) {
		if(client->getPassword().size() > 0) {
			client->password(client->getPassword());
		} else {
			ctrlMessage.SetWindowText("/password ");
			ctrlMessage.SetFocus();
			ctrlMessage.SetSel(10, 10);
			waitingForPW = true;
		}
	} else if(wParam == PRIVATE_MESSAGE) {
		PMInfo* i = (PMInfo*)lParam;
		if(i->user->isOnline()) {
			if(BOOLSETTING(POPUP_PMS) || PrivateFrame::isOpen(i->user)) {
				PrivateFrame::gotMessage(i->user, i->msg, m_hWndMDIClient, getTab());
			} else {
				addLine(STRING(PRIVATE_MESSAGE_FROM) + i->user->getNick() + ": " + i->msg);
			}
		} else {
			if(BOOLSETTING(IGNORE_OFFLINE)) {
				addClientLine(STRING(IGNORED_MESSAGE) + i->msg, false);
			} else if(BOOLSETTING(POPUP_OFFLINE)) {
				PrivateFrame::gotMessage(i->user, i->msg, m_hWndMDIClient, getTab());
			} else {
				addLine(STRING(PRIVATE_MESSAGE_FROM) + i->user->getNick() + ": " + i->msg);
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

		// Strange, can't get the correct width of the last field...
		ctrlStatus.GetRect(2, sr);
		sr.left = sr.right + 2;
		sr.right = sr.left + 16;
		ctrlShowUsers.MoveWindow(sr);
	}
	
	CRect rc = rect;
	rc.bottom -=28;
	if(!client->getUserInfo()) {
		if(GetSinglePaneMode() == SPLIT_PANE_NONE)
			SetSinglePaneMode(SPLIT_PANE_LEFT);
	} else {
		if(GetSinglePaneMode() != SPLIT_PANE_NONE)
			SetSinglePaneMode(SPLIT_PANE_NONE);
	}
	SetSplitterRect(rc);
	
	rc = rect;
	rc.bottom -= 2;
	rc.top = rc.bottom - 22;
	rc.left +=2;
	rc.right -=2;
	ctrlMessage.MoveWindow(rc);
	
}

LRESULT HubFrame::onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	int i = 0;
	
	SettingsManager::getInstance()->set(SettingsManager::GET_USER_INFO, client->getUserInfo());

	TimerManager::getInstance()->removeListener(this);
	client->removeListener(this);
	
	while(i < ctrlUsers.GetItemCount()) {
		delete (UserInfo*)ctrlUsers.GetItemData(i);
		i++;
	}
	
	string tmp1;
	string tmp2;
	
	ctrlUsers.GetColumnOrderArray(COLUMN_LAST, columnIndexes);
	for(int j = COLUMN_FIRST; j != COLUMN_LAST; j++) {
		columnSizes[j] = ctrlUsers.GetColumnWidth(j);
		tmp1 += Util::toString(columnIndexes[j]) + ",";
		tmp2 += Util::toString(columnSizes[j]) + ",";
	}
	tmp1.erase(tmp1.size()-1, 1);
	tmp2.erase(tmp2.size()-1, 1);
	
	SettingsManager::getInstance()->set(SettingsManager::HUBFRAME_ORDER, tmp1);
	SettingsManager::getInstance()->set(SettingsManager::HUBFRAME_WIDTHS, tmp2);
				
	bHandled = FALSE;
	return 0;
}

static int textUnderCursor(POINT p, CEdit& ctrl, string& x) {
	
	int i = ctrl.CharFromPos(p);
	int line = ctrl.LineFromChar(i);
	int c = LOWORD(i) - ctrl.LineIndex(line);
	int len = ctrl.LineLength(i) + 1;
	if(len < 3) {
		return 0;
	}

	char* buf = new char[len];
	ctrl.GetLine(line, buf, len);
	x = string(buf, len-1);
	delete buf;

	string::size_type start = x.rfind(' ', c);
	if(start == string::npos) {
		start = x.rfind('<', c);
		if(start == string::npos) {
			start = 0;
		}
	} else {
		start++;
	}
	return start;
}

LRESULT HubFrame::onLButton(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	HWND focus = GetFocus();
	if(focus == ctrlClient.m_hWnd) {
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		string x;
		string::size_type start = (string::size_type)textUnderCursor(pt, ctrlClient, x);
		
		if(x[start] == '<') {
			string::size_type end = x.find('>', 1);
			if(end == string::npos || end == start + 1) {
				return 0;
			}

			// Nickname click, let's see if we can find one like it in the name list...
			int pos = ctrlUsers.find(x.substr(start + 1, end - start - 1));
			if(pos != -1) {
				if (wParam & MK_CONTROL) { // MK_CONTROL = 0x0008
					PrivateFrame::openWindow(((UserInfo*)ctrlUsers.GetItemData(pos))->user, m_hWndMDIClient, getTab());
				} else if (wParam & MK_SHIFT) {
					try {
						QueueManager::getInstance()->addList(((UserInfo*)ctrlUsers.GetItemData(pos))->user);
					} catch(Exception e) {
						addClientLine(e.getError());
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
		} else if( (Util::strnicmp(x.c_str() + start, "http://", 7) == 0) || 
			(Util::strnicmp(x.c_str() + start, "www.", 4) == 0) ||
			(Util::strnicmp(x.c_str() + start, "ftp://", 6) == 0) )	{

			// Web links...
			string::size_type end = x.find(' ', start + 7);
			if(end == string::npos) {
				end = x.length();
			}
			if(end < start + 10) {
				return 0;
			}

			ShellExecute(NULL, NULL, x.substr(start, end-start).c_str(), NULL, NULL, SW_SHOWNORMAL);
		}
	}
	return 0;
}

void HubFrame::addLine(const string& aLine) {
	if(ctrlClient.GetWindowTextLength() > 25000) {
		// We want to limit the buffer to 25000 characters...after that, w95 becomes sad...
		ctrlClient.SetRedraw(FALSE);
		ctrlClient.SetSel(0, ctrlClient.LineIndex(ctrlClient.LineFromChar(2000)), TRUE);
		ctrlClient.ReplaceSel("");
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
		params["message"] = aLine;
		LOG(client->getServer(), Util::formatParams(SETTING(LOG_FORMAT_MAIN_CHAT), params));
	}
	if(timeStamps) {
		ctrlClient.AppendText(("\r\n[" + Util::getShortTimeString() + "] " + aLine).c_str());
	} else {
		ctrlClient.AppendText(("\r\n" + aLine).c_str());
	}
	if(noscroll) {
		ctrlClient.SetRedraw(TRUE);
	}
	setDirty();
}

LRESULT HubFrame::onContextMenu(UINT uMsg, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled) {
	RECT rc;                    // client area of window 
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };        // location of mouse click 

	ctrlClient.GetClientRect(&rc);
	if(uMsg == WM_CONTEXTMENU)
		ctrlClient.ScreenToClient(&pt);
	
	if (PtInRect(&rc, pt)) {
		string x;
		string::size_type start = (string::size_type)textUnderCursor(pt, ctrlClient, x);

		if(x[start] == '<') {
			string::size_type end = x.find('>', 1);
			if(end == string::npos || end == start + 1) {
				bHandled = FALSE;
				return FALSE;
			}
			
			// Nickname click, let's see if we can find one like it in the name list...
			int pos = ctrlUsers.find(x.substr(start + 1, end - start - 1));
			if(pos != -1) {
				int items = ctrlUsers.GetItemCount();
				ctrlUsers.SetRedraw(FALSE);
				for(int i = 0; i < items; ++i) {
					ctrlUsers.SetItemState(i, (i == pos) ? LVIS_SELECTED | LVIS_FOCUSED : 0, LVIS_SELECTED | LVIS_FOCUSED);
				}
				ctrlUsers.SetRedraw(TRUE);
				ctrlUsers.EnsureVisible(pos, FALSE);
				
				ctrlClient.ClientToScreen(&pt);
				if(client->getOp()) {
					opMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
				} else {
					userMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
				}
				return TRUE; 
			} else {
				bHandled = FALSE;
			}
		} else {
			bHandled = FALSE;
		}
	} else {
		ctrlClient.ClientToScreen(&pt);
		// Get the bounding rectangle of the client area. 
		ctrlUsers.GetClientRect(&rc);
		if(uMsg == WM_CONTEXTMENU)
			ctrlUsers.ScreenToClient(&pt); 
		
		if (PtInRect(&rc, pt)) { 
			ctrlUsers.ClientToScreen(&pt);
			if(client->getOp()) {
				opMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
			} else {
				userMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
			}
			return TRUE; 
		}
	}

	return FALSE;
}

void HubFrame::onTab() {
	HWND focus = GetFocus();
	
	if(focus == ctrlClient.m_hWnd) {
		ctrlMessage.SetFocus();
	} else if(focus == ctrlMessage.m_hWnd) {
		ctrlClient.SetFocus();
	}
}

LRESULT HubFrame::onChar(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled) {
	switch(wParam) {
		case VK_TAB:
			if(uMsg == WM_KEYDOWN) {
				onTab();
			}
			break;
		case VK_RETURN:
			if( (GetKeyState(VK_CONTROL) & 0x8000) || 
				(GetKeyState(VK_MENU) & 0x8000) ) {
					bHandled = FALSE;
				} else {
					if(uMsg == WM_KEYDOWN) {
						onEnter();
					}
				}
				break;
		default:
			bHandled = FALSE;
	}
	return 0;
}

LRESULT HubFrame::onShowUsers(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled) {
	bHandled = FALSE;
	if((wParam == BST_CHECKED) && !client->getUserInfo()) {
		User::NickMap& lst = client->lockUserList();
		ctrlUsers.SetRedraw(FALSE);
		for(User::NickIter i = lst.begin(); i != lst.end(); ++i) {
			updateUser(i->second);
		}
		client->unlockUserList();
		ctrlUsers.SetRedraw(TRUE);
		ctrlUsers.resort();

		client->setUserInfo(true);
		client->refreshUserList(true);		
	} else {
		client->setUserInfo(false);
		clearUserList();
	}

	UpdateLayout(FALSE);
	return 0;
}

LRESULT HubFrame::onFollow(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	
	if(!redirect.empty()) {
		string s, f;
		short p = 411;
		Util::decodeUrl(redirect, s, p, f);
		if(ClientManager::getInstance()->isConnected(s)) {
			addClientLine(STRING(REDIRECT_ALREADY_CONNECTED));
			return 0;
		}
		
		dcassert(frames.find(server) != frames.end());
		dcassert(frames[server] == this);
		frames.erase(server);
		server = redirect;
		frames[server] = this;
		client->addListener(this);
		client->connect(redirect);
	}
	return 0;
}

void HubFrame::addClientLine(const string& aLine, bool inChat /* = true */) {
	string line = "[" + Util::getShortTimeString() + "] " + aLine;

	ctrlStatus.SetText(0, (line + "\r\n").c_str());
	setDirty();
	
	if(BOOLSETTING(STATUS_IN_CHAT) && inChat) {
		addLine("*** " + aLine);
	}
}

void HubFrame::onAction(TimerManagerListener::Types type, DWORD /*aTick*/) throw() {
	switch(type) {
		case TimerManagerListener::SECOND:
			updateStatusBar(); break;
	}
}

// ClientListener
void HubFrame::onAction(ClientListener::Types type, Client* client) {
	switch(type) {
		case ClientListener::CONNECTING:
			speak(ADD_STATUS_LINE, STRING(CONNECTING_TO) + client->getServer() + "...");
			speak(SET_WINDOW_TITLE, client->getServer());
			break;
		case ClientListener::CONNECTED: speak(ADD_STATUS_LINE, STRING(CONNECTED)); break;
		case ClientListener::BAD_PASSWORD: client->setPassword(Util::emptyString); break;
		case ClientListener::GET_PASSWORD: speak(GET_PASSWORD); break;
		case ClientListener::HUB_NAME: speak(SET_WINDOW_TITLE, client->getName() + " (" + client->getServer() + ")"); break;
		case ClientListener::VALIDATE_DENIED:
			client->removeListener(this);
			client->disconnect();
			speak(ADD_STATUS_LINE, STRING(NICK_TAKEN));
			break;
	}
}

void HubFrame::onAction(ClientListener::Types type, Client* /*client*/, const string& line) {
	switch(type) {
		case ClientListener::SEARCH_FLOOD: speak(ADD_STATUS_LINE, STRING(SEARCH_SPAM_FROM) + line); break;
		case ClientListener::FAILED: speak(ADD_STATUS_LINE, line); speak(REMOVE_USERS); break;
		case ClientListener::MESSAGE: 
			if(SETTING(FILTER_KICKMSGS)) {
				if((line.find("Hub-Security") != string::npos) && (line.find("was kicked by") != string::npos)) {
					// Do nothing...
				} else if((line.find("is kicking") != string::npos) && (line.find("because:") != string::npos)) {
					speak(ADD_SILENT_STATUS_LINE, line);
				} else {
					speak(ADD_CHAT_LINE, line);
				}
			} else {
				speak(ADD_CHAT_LINE, line);
			}
			break;

		case ClientListener::FORCE_MOVE:
			{
				string s, f;
				short p = 411;
				Util::decodeUrl(line, s, p, f);
				if(ClientManager::getInstance()->isConnected(s)) {
					speak(ADD_STATUS_LINE, STRING(REDIRECT_ALREADY_CONNECTED));
					return;
				}
			}
			redirect = line;
			if(BOOLSETTING(AUTO_FOLLOW)) {
				PostMessage(WM_COMMAND, IDC_FOLLOW, 0);
			} else {
				speak(ADD_STATUS_LINE, STRING(PRESS_FOLLOW) + line);
			}
			break;
	}
}

void HubFrame::onAction(ClientListener::Types type, Client* /*client*/, const User::Ptr& user) {
	switch(type) {
		case ClientListener::MY_INFO: if(client->getUserInfo()) speak(UPDATE_USER, user); break;
		case ClientListener::QUIT: if(client->getUserInfo()) speak(REMOVE_USER, user); break;
		case ClientListener::HELLO: if(client->getUserInfo()) speak(UPDATE_USER, user); break;
	}
}

void HubFrame::onAction(ClientListener::Types type, Client* /*client*/, const User::List& aList) {
	switch(type) {
		case ClientListener::OP_LIST: // Fall through
		case ClientListener::NICK_LIST: 
			if(client->getUserInfo()) speak(UPDATE_USERS, aList); break;
	}
}

void HubFrame::onAction(ClientListener::Types type, Client* /*client*/, const User::Ptr& user, const string&  line) {
	switch(type) {
		case ClientListener::PRIVATE_MESSAGE: speak(PRIVATE_MESSAGE, user, line); break;
	}
}

/**
 * @file HubFrame.cpp
 * $Id: HubFrame.cpp,v 1.18 2002/12/28 01:31:50 arnetheduck Exp $
 */
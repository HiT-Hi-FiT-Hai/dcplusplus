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

char *msgs[] = { "\r\n-- I'm a happy dc++ user. You could be happy too.\r\n-- http://dcplusplus.sourceforge.net <DC++ " VERSIONSTRING ">",
"\r\n-- Neo-...what? Nope...never heard of it...\r\n-- http://dcplusplus.sourceforge.net <DC++ " VERSIONSTRING ">",
"\r\n-- Evolution of species: Ape --> Man\r\n-- Evolution of science: \"The Earth is Flat\" --> \"The Earth is Round\"\r\n-- Evolution of sharing: NMDC --> DC++\r\n-- http://dcplusplus.sourceforge.net  <DC++ " VERSIONSTRING ">",
"\r\n-- I share, therefore I am.\r\n-- http://dcplusplus.sourceforge.net  <DC++ " VERSIONSTRING ">",
"\r\n-- I came, I searched, I found...\r\n-- http://dcplusplus.sourceforge.net  <DC++ " VERSIONSTRING ">",
"\r\n-- I came, I shared, I sent...\r\n-- http://dcplusplus.sourceforge.net  <DC++ " VERSIONSTRING ">",
"\r\n-- I can set away mode, can't you?\r\n-- http://dcplusplus.sourceforge.net  <DC++ " VERSIONSTRING ">",
"\r\n-- I don't have to see any ads, do you?\r\n-- http://dcplusplus.sourceforge.net  <DC++ " VERSIONSTRING ">",
"\r\n-- I don't have to see those annoying kick messages, do you?\r\n-- http://dcplusplus.sourceforge.net  <DC++ " VERSIONSTRING ">",
"\r\n-- I can resume my files to a different filename, can you?\r\n-- http://dcplusplus.sourceforge.net  <DC++ " VERSIONSTRING ">"
};

int HubFrame::columnSizes[] = { 100, 75, 100, 75, 100 };
int HubFrame::columnIndexes[] = { COLUMN_NICK, COLUMN_SHARED, COLUMN_DESCRIPTION, COLUMN_CONNECTION, COLUMN_EMAIL };

#define MSGS 10

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

	StringList l = StringTokenizer(SETTING(HUBFRAME_ORDER), ',').getTokens();
	
	{
		int k = 0;
		for(StringIter i = l.begin(); i != l.end(); ++i) {
			if(k >= COLUMN_LAST)
				break;
			columnIndexes[k++] = Util::toInt(*i);
		}
	}
	
	l = StringTokenizer(SETTING(HUBFRAME_WIDTHS), ',').getTokens();
	{
		int k = 0;
		for(StringIter i = l.begin(); i != l.end(); ++i) {
			if(k >= COLUMN_LAST)
				break;
			columnSizes[k++] = Util::toInt(*i);
		}
	}

	LV_COLUMN lvc;
	ZeroMemory(&lvc, sizeof(lvc));
	lvc.mask = LVCF_FMT | LVCF_ORDER | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;

	lvc.pszText = const_cast<char*>(CSTRING(NICK));
	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = columnSizes[COLUMN_NICK];
	lvc.iOrder = columnIndexes[COLUMN_NICK];
	lvc.iSubItem = COLUMN_NICK;
	ctrlUsers.InsertColumn(0, &lvc);

	lvc.pszText = const_cast<char*>(CSTRING(SHARED));
	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = columnSizes[COLUMN_SHARED];
	lvc.iOrder = columnIndexes[COLUMN_SHARED];
	lvc.iSubItem = COLUMN_SHARED;
	ctrlUsers.InsertColumn(1, &lvc);
	
	lvc.pszText = const_cast<char*>(CSTRING(DESCRIPTION));
	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = columnSizes[COLUMN_DESCRIPTION];
	lvc.iOrder = columnIndexes[COLUMN_DESCRIPTION];
	lvc.iSubItem = COLUMN_DESCRIPTION;
	ctrlUsers.InsertColumn(2, &lvc);
	
	lvc.pszText = const_cast<char*>(CSTRING(CONNECTION));
	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = columnSizes[COLUMN_CONNECTION];
	lvc.iOrder = columnIndexes[COLUMN_CONNECTION];
	lvc.iSubItem = COLUMN_CONNECTION;
	ctrlUsers.InsertColumn(3, &lvc);
	
	lvc.pszText = const_cast<char*>(CSTRING(EMAIL));
	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = columnSizes[COLUMN_EMAIL];
	lvc.iOrder = columnIndexes[COLUMN_EMAIL];
	lvc.iSubItem = COLUMN_EMAIL;
	ctrlUsers.InsertColumn(4, &lvc);

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
	userMenu.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)NULL);
	userMenu.AppendMenu(MF_STRING, IDC_REFRESH, CSTRING(REFRESH_USER_LIST));

	opMenu.CreatePopupMenu();
	opMenu.AppendMenu(MF_STRING, IDC_GETLIST, CSTRING(GET_FILE_LIST));
	opMenu.AppendMenu(MF_STRING, IDC_PRIVATEMESSAGE, CSTRING(SEND_PRIVATE_MESSAGE));
	opMenu.AppendMenu(MF_STRING, IDC_GRANTSLOT, CSTRING(GRANT_EXTRA_SLOT));
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

			if(stricmp(s.c_str(), "refresh")==0) {
				try {
					ShareManager::getInstance()->setDirty();
					ShareManager::getInstance()->refresh(true);
					addClientLine(STRING(FILE_LIST_REFRESHED));
				} catch(ShareException e) {
					addClientLine(e.getError());
				}
			} else if(stricmp(s.c_str(), "slots")==0) {
				int j = Util::toInt(param);
				if(j > 0) {
					SettingsManager::getInstance()->set(SettingsManager::SLOTS, j);
					addClientLine(STRING(SLOTS_SET));
					ClientManager::getInstance()->infoUpdated();
				} else {
					addClientLine(STRING(INVALID_NUMBER_OF_SLOTS));
				}
			} else if(stricmp(s.c_str(), "join")==0) {
				if(!param.empty()) {
					client->connect(param);
				} else {
					addClientLine(STRING(SPECIFY_SERVER));
				}
			} else if(stricmp(s.c_str(), "search") == 0) {
				if(!param.empty()) {
					SearchFrame* pChild = new SearchFrame();
					pChild->setTab(getTab());
					pChild->setInitial(param, 0, SearchManager::SIZE_ATLEAST);
					pChild->CreateEx(m_hWndMDIClient);
				} else {
					addClientLine(STRING(SPECIFY_SEARCH_STRING));
				}
			} else if(stricmp(s.c_str(), "dc++") == 0) {
				client->sendMessage(msgs[GET_TICK() % MSGS]);
			} else if(stricmp(s.c_str(), "clear") == 0) {
				ctrlClient.SetWindowText("");
			} else if(stricmp(s.c_str(), "away") == 0) {
				if(Util::getAway()) {
					Util::setAway(false);
					addClientLine(STRING(AWAY_MODE_OFF));
				} else {
					Util::setAway(true);
					Util::setAwayMessage(param);
					addClientLine(STRING(AWAY_MODE_ON) + Util::getAwayMessage());
				}
			} else if(stricmp(s.c_str(), "back") == 0) {
				Util::setAway(false);
				addClientLine(STRING(AWAY_MODE_OFF));
			} else if(stricmp(s.c_str(), "ts") == 0) {
				timeStamps = !timeStamps;
				if(timeStamps) {
					addClientLine(STRING(TIMESTAMPS_ENABLED));
				} else {
					addClientLine(STRING(TIMESTAMPS_DISABLED));
				}
			} else if( (stricmp(s.c_str(), "password") == 0) && waitingForPW ) {
				client->setPassword(param);
				client->password(param);
				waitingForPW = false;
			} else if( stricmp(s.c_str(), "showjoins") == 0 ) {
				showJoins = !showJoins;
				if(showJoins) {
					addClientLine(STRING(JOIN_SHOWING_ON));
				} else {
					addClientLine(STRING(JOIN_SHOWING_OFF));
				}
			} else if(stricmp(s.c_str(), "close") == 0) {
				PostMessage(WM_CLOSE);
			} else if(stricmp(s.c_str(), "help") == 0) {
				addLine("/clear, /refresh, /slots #, /join <hub-ip>, /search <string>, /dc++, /away <msg>, /back, /ts, /password, /showjoins, /close, /help");
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
			} catch(...) {
				// ...
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
		} catch(...) {
			// ...
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

LRESULT HubFrame::onSpeaker(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	if(wParam == UPDATE_USER) {
		UserInfo* ui = (UserInfo*)lParam;
		User::Ptr& u = ui->user;
		int i;

		if( (i = ctrlUsers.find(u->getNick())) != -1 ) {

			ctrlUsers.SetRedraw(FALSE);
			ctrlUsers.SetItem(i, 0, LVIF_IMAGE, NULL, getImage(u), 0, 0, NULL);
			ctrlUsers.SetItemText(i, COLUMN_SHARED, Util::formatBytes(u->getBytesShared()).c_str());
			ctrlUsers.SetItemText(i, COLUMN_DESCRIPTION, u->getDescription().c_str());
			ctrlUsers.SetItemText(i, COLUMN_CONNECTION, u->getConnection().c_str());
			ctrlUsers.SetItemText(i, COLUMN_EMAIL, u->getEmail().c_str());
			ctrlUsers.SetRedraw(TRUE);

			RECT rc;
			ctrlUsers.GetItemRect(i, &rc, LVIR_BOUNDS);
			ctrlUsers.InvalidateRect(&rc);

			delete ui;
			needSort = true;
		} else {
			StringList l;
			l.push_back(u->getNick());
			l.push_back(Util::formatBytes(u->getBytesSharedString()));
			l.push_back(u->getDescription());
			l.push_back(u->getConnection());
			l.push_back(u->getEmail());
			ctrlUsers.insert(l, getImage(u), (LPARAM)ui);

			if(showJoins) {
				addLine("*** " + STRING(JOINS) + u->getNick());
			}
		}
	} else if(wParam == UPDATE_USERS) {
		User::List* ul = (User::List*)lParam;

		ctrlUsers.SetRedraw(FALSE);

		User::Iter i;
		
		for(i = ul->begin(); i != ul->end();) {
			User::Ptr& u = *i;
			int j = ctrlUsers.find(u->getNick());
			if(j != -1) {
				ctrlUsers.SetItem(j, 0, LVIF_IMAGE, NULL, getImage(u), 0, 0, NULL);
				ctrlUsers.SetItemText(j, COLUMN_SHARED, Util::formatBytes(u->getBytesShared()).c_str());
				ctrlUsers.SetItemText(j, COLUMN_DESCRIPTION, u->getDescription().c_str());
				ctrlUsers.SetItemText(j, COLUMN_CONNECTION, u->getConnection().c_str());
				ctrlUsers.SetItemText(j, COLUMN_EMAIL, u->getEmail().c_str());

				i = ul->erase(i);
			} else {
				++i;
			}
		}

		for(i = ul->begin(); i != ul->end(); ++i) {
			User::Ptr& u = *i;
            int j = ctrlUsers.insert(ctrlUsers.GetItemCount(), u->getNick(), getImage(u), (LPARAM)new UserInfo(u));
			ctrlUsers.SetItemText(j, COLUMN_SHARED, Util::formatBytes(u->getBytesShared()).c_str());
			ctrlUsers.SetItemText(j, COLUMN_DESCRIPTION, u->getDescription().c_str());
			ctrlUsers.SetItemText(j, COLUMN_CONNECTION, u->getConnection().c_str());
			ctrlUsers.SetItemText(j, COLUMN_EMAIL, u->getEmail().c_str());
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
				addLine("*** " + STRING(PARTS) + ui->user->getNick());
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
	} else if(wParam == SET_WINDOW_TITLE) {
		string* x = (string*)lParam;
		SetWindowText(x->c_str());
		delete x;
	} else if(wParam == STATS) {
		ctrlStatus.SetText(1, (Util::toString(client->getUserCount()) + " " + STRING(HUB_USERS)).c_str());
		ctrlStatus.SetText(2, Util::formatBytes(client->getAvailable()).c_str());

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
		int w[3];
		ctrlStatus.GetClientRect(sr);
		int tmp = (sr.Width()) > 316 ? 216 : ((sr.Width() > 116) ? sr.Width()-100 : 16);
		
		w[0] = sr.right - tmp;
		w[1] = w[0] + (tmp-16)/2;
		w[2] = w[0] + (tmp-16);
		
		ctrlStatus.SetParts(3, w);
	}
	
	CRect rc = rect;
	rc.bottom -=28;
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

LRESULT HubFrame::onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	RECT rc;                    // client area of window 
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };        // location of mouse click 
	
	// Get the bounding rectangle of the client area. 
	ctrlUsers.GetClientRect(&rc);
	ctrlUsers.ScreenToClient(&pt); 
	
	if (PtInRect(&rc, pt)) 
	{ 
		ctrlUsers.ClientToScreen(&pt);
		if(client->getOp()) {
			opMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
		} else {
			userMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
		}
		
		return TRUE; 
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
			if( (GetKeyState(VK_SHIFT) & 0x8000) || 
				(GetKeyState(VK_CONTROL) & 0x8000) || 
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

/**
 * @file HubFrame.cpp
 * $Id: HubFrame.cpp,v 1.9 2002/05/23 21:48:24 arnetheduck Exp $
 */


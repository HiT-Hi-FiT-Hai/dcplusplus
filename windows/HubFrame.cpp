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
					ctrlStatus.SetText(0, CSTRING(FILE_LIST_REFRESHED));
				} catch(ShareException e) {
					ctrlStatus.SetText(0, e.getError().c_str());
				}
			} else if(stricmp(s.c_str(), "slots")==0) {
				int j = Util::toInt(param);
				if(j > 0) {
					SettingsManager::getInstance()->set(SettingsManager::SLOTS, j);
					ctrlStatus.SetText(0, CSTRING(SLOTS_SET));
					ClientManager::getInstance()->infoUpdated();
				} else {
					ctrlStatus.SetText(0, CSTRING(INVALID_NUMBER_OF_SLOTS));
				}
			} else if(stricmp(s.c_str(), "join")==0) {
				if(!param.empty()) {
					if(client)
						client->connect(param);
				} else {
					ctrlStatus.SetText(0, CSTRING(SPECIFY_SERVER));
				}
			} else if(stricmp(s.c_str(), "search") == 0) {
				if(!param.empty()) {
					SearchFrame* pChild = new SearchFrame();
					pChild->setTab(getTab());
					pChild->setInitial(param, 0, SearchManager::SIZE_ATLEAST);
					pChild->CreateEx(m_hWndMDIClient);
				} else {
					ctrlStatus.SetText(0, CSTRING(SPECIFY_SEARCH_STRING));
				}
			} else if(stricmp(s.c_str(), "dc++") == 0) {
				if(client)
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
			}
		} else {
			if(client)
				client->sendMessage(s);
		}
		ctrlMessage.SetWindowText("");
	} else {
		MessageBeep(MB_ICONEXCLAMATION);
	}
}

/**
 * @todo fix the user stuff...
 */
LRESULT HubFrame::onGetList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int i=-1;
	char buf[256];

	if(client && client->isConnected()) {
		while( (i = ctrlUsers.GetNextItem(i, LVNI_SELECTED)) != -1) {
			ctrlUsers.GetItemText(i, COLUMN_NICK, buf, 256);
			try {
				QueueManager::getInstance()->addList(ClientManager::getInstance()->getUser(buf, client->getIp()));
			} catch(...) {
				// ...
			}
		}
	}
	return 0;
}

LRESULT HubFrame::onAddToFavorites(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int i=-1;
	char buf[256];
	
	if(client && client->isConnected()) {
		while( (i = ctrlUsers.GetNextItem(i, LVNI_SELECTED)) != -1) {
			ctrlUsers.GetItemText(i, COLUMN_NICK, buf, 256);
			HubManager::getInstance()->addFavoriteUser(ClientManager::getInstance()->getUser(buf, client->getIp()));
		}
	}
	return 0;
}

LRESULT HubFrame::onPrivateMessage(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int i=-1;
	char buf[256];
	if(client && client->isConnected()) {
		while( (i = ctrlUsers.GetNextItem(i, LVNI_SELECTED)) != -1) {
			ctrlUsers.GetItemText(i, COLUMN_NICK, buf, 256);
			
			PrivateFrame::openWindow(ClientManager::getInstance()->getUser(buf, client->getIp()), m_hWndMDIClient, getTab());
		}
	}
	return 0;
}

LRESULT HubFrame::onDoubleClickUsers(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NMITEMACTIVATE* item = (NMITEMACTIVATE*)pnmh;
	char buf[256];

	if(client && client->isConnected() && item->iItem != -1) {
		ctrlUsers.GetItemText(item->iItem, COLUMN_NICK, buf, 256);
		try {
			QueueManager::getInstance()->addList(ClientManager::getInstance()->getUser(buf, client->getIp()));
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
			char buf[256];
			ctrlUsers.GetItemText(i, COLUMN_NICK, buf, 256);
			if(client) {
				User::Ptr p = ClientManager::getInstance()->getUser(buf, client->getIp());
				if(p->isSet(User::OP))
					op = true;
				p->kick(dlg.line);
				k++;
			}
		}
	}
	
	return 0; 
};

LRESULT HubFrame::onGrantSlot(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) { 
	int i = -1;
	while( (i = ctrlUsers.GetNextItem(i, LVNI_SELECTED)) != -1) {
		char buf[256];
		ctrlUsers.GetItemText(i, COLUMN_NICK, buf, 256);
		if(client) {
			UploadManager::getInstance()->reserveSlot(ClientManager::getInstance()->getUser(buf, client->getIp()));
		}
	}
	return 0; 
};

LRESULT HubFrame::onRedirect(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) { 
	LineDlg dlg1, dlg2;
	dlg1.title = STRING(REDIRECT_USER);
	dlg1.description = STRING(ENTER_REASON);
	dlg1.line = lastRedir;

	if(dlg1.DoModal() == IDOK) {
		lastRedir = dlg1.line;
		dlg2.title = STRING(REDIRECT_USER);
		dlg2.description = STRING(ENTER_SERVER);
		dlg2.line = lastServer;
		if(dlg2.DoModal() == IDOK) {
			lastServer = dlg2.line;
			int i = -1;
			while( (i = ctrlUsers.GetNextItem(i, LVNI_SELECTED)) != -1) {
				char buf[256];
				ctrlUsers.GetItemText(i, COLUMN_NICK, buf, 256);
				if(client) {
					client->opForceMove(ClientManager::getInstance()->getUser(buf, client->getIp()), dlg2.line, STRING(YOU_ARE_BEING_REDIRECTED) + dlg2.line + ": " + dlg1.line);
				}
			}
		}
	}
	
	return 0; 
};

LRESULT HubFrame::onSpeaker(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	// First some specials to handle those messages that have to initialize variables...
	if(wParam == CLIENT_MESSAGE) {
		addLine(*(string*)lParam);
		delete (string*)lParam;
	} else if (wParam == CLIENT_STATUS) {
		addClientLine(*(string*)lParam);
		delete (string*)lParam;
	} else if(wParam == CLIENT_MYINFO || wParam == CLIENT_HELLO) {
		User::Ptr u = *(User::Ptr*)lParam;
		delete (User::Ptr*)lParam;
		
		LV_FINDINFO fi;
		fi.flags = LVFI_STRING;
		fi.psz = u->getNick().c_str();
		int j = ctrlUsers.FindItem(&fi, -1);
		if(j == -1) {
			UserInfo* ui = new UserInfo;
			ui->size = u->getBytesShared();
			StringList l;
			l.push_back(u->getNick());
			l.push_back(Util::formatBytes(u->getBytesSharedString()));
			l.push_back(u->getDescription());
			l.push_back(u->getConnection());
			l.push_back(u->getEmail());
			ctrlUsers.insert(l, getImage(u), (LPARAM)ui);
		} else {
			ctrlUsers.SetRedraw(FALSE);
			
			ctrlUsers.SetItem(j, 0, LVIF_IMAGE, NULL, getImage(u), 0, 0, NULL);
			ctrlUsers.SetItemText(j, COLUMN_SHARED, Util::formatBytes(u->getBytesShared()).c_str());
			ctrlUsers.SetItemText(j, COLUMN_DESCRIPTION, u->getDescription().c_str());
			ctrlUsers.SetItemText(j, COLUMN_CONNECTION, u->getConnection().c_str());
			ctrlUsers.SetItemText(j, COLUMN_EMAIL, u->getEmail().c_str());
			
			ctrlUsers.SetRedraw(TRUE);
			RECT rc;
			ctrlUsers.GetItemRect(j, &rc, LVIR_BOUNDS);
			ctrlUsers.InvalidateRect(&rc);

			((UserInfo*)ctrlUsers.GetItemData(j))->size = u->getBytesShared();
		}

		if(showJoins && (wParam == CLIENT_HELLO)) {
			addLine("*** " + STRING(JOINS) + u->getNick());
		}
		
	} else if(wParam==STATS) {
		if(client) {
			ctrlStatus.SetText(1, (Util::toString(client->getUserCount()) + " " + STRING(USERS)).c_str());
			ctrlStatus.SetText(2, Util::formatBytes(client->getAvailable()).c_str());
		}

	} else if(wParam == CLIENT_QUIT) {
		User::Ptr u = *(User::Ptr*)lParam;
		delete (User::Ptr*)lParam;
		
		int item = ctrlUsers.find(u->getNick());
		if(item != -1) {
			delete (UserInfo*)ctrlUsers.GetItemData(item);
			ctrlUsers.DeleteItem(item);
		}
		if(showJoins) {
			addLine("*** " + STRING(PARTS) + u->getNick());
		}
	} else if(wParam == CLIENT_GETPASSWORD) {
		if(client) {
			if(client->getPassword().size() > 0) {
				client->password(client->getPassword());
			} else {
				ctrlMessage.SetWindowText("/password ");
				ctrlMessage.SetFocus();
				waitingForPW = true;
			}
		}
	} else if(wParam == CLIENT_CONNECTING) {
		if(client) {
			addClientLine(STRING(CONNECTING_TO) + client->getServer() + "...");
			SetWindowText(client->getServer().c_str());
		}
	} else if(wParam == CLIENT_FAILED) {
		clearUserList();
		addClientLine(*(string*)lParam);
		delete (string*)lParam;
		//ctrlClient.Invalidate();
	} else if(wParam == CLIENT_HUBNAME) {
		if(client) {
			SetWindowText( (client->getName() + " (" + client->getServer() + ")").c_str());
			addClientLine(STRING(CONNECTED));
		}
	} else if(wParam == CLIENT_VALIDATEDENIED) {
		addClientLine(STRING(NICK_TAKEN));
		if(client)
			client->disconnect();
	} else if(wParam == CLIENT_PRIVATEMESSAGE) {
		PMInfo* i = (PMInfo*)lParam;
		if(i->user->isOnline()) {
			PrivateFrame::gotMessage(i->user, i->msg, m_hWndMDIClient, getTab());
		} else {
			if(BOOLSETTING(IGNORE_OFFLINE)) {
				addClientLine(STRING(IGNORED_MESSAGE) + i->msg, false);
			} else if(BOOLSETTING(POPUP_OFFLINE)) {
				PrivateFrame::gotMessage(i->user, i->msg, m_hWndMDIClient, getTab());
			} else {
				addLine(STRING(PRIVATE_MESSAGE_FROM) + i->user->getNick() + ": \r\n" + i->msg);
			}
		}
		delete i;
	} else if(wParam == CLIENT_CONNECTED) {
		//ctrlClient.Invalidate();
	} else if(wParam == CLIENT_SEARCH_FLOOD) {
		// We have a spammer!!!
		string* x = (string*)lParam;
		addClientLine("Search spam detected from " + (*x) + " (more than 5 searches within 7 seconds)");
		delete x;
	} else if(wParam == REDIRECT) {
		if(client)
			client->disconnect();

		if(!redirect.empty()) {
			ctrlStatus.SetText(0, ("Redirect to " + redirect + " received, press the follow redirect button if you want to go there").c_str());
		}
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
	ClientManager::getInstance()->putClient(client);
	client = NULL;
	
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
	if(ctrlClient.GetWindowTextLength() > 30000) {
		// We want to limit the buffer to 30000 characters...after that, w95 becomes sad...
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
	if(timeStamps) {
		ctrlClient.AppendText(("\r\n[" + Util::getShortTimeString() + "] " + aLine).c_str());
		if(BOOLSETTING(LOG_MAIN_CHAT)) {
			LOG(client->getServer(), "[" + Util::getShortTimeString() + "] " + aLine);
		}
	} else {
		ctrlClient.AppendText(("\r\n" + aLine).c_str());
		if(BOOLSETTING(LOG_MAIN_CHAT)) {
			LOG(client->getServer(), aLine);
		}
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
		if(op) {
			opMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
		} else {
			userMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
		}
		
		return TRUE; 
	}
	
	return FALSE; 
}

/**
 * @file HubFrame.cpp
 * $Id: HubFrame.cpp,v 1.3 2002/04/16 16:45:54 arnetheduck Exp $
 */


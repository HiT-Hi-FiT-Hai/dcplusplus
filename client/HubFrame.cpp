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
#include "DCPlusPlus.h"

#include "HubFrame.h"
#include "QueueManager.h"
#include "LineDlg.h"
#include "ShareManager.h"
#include "SearchFrm.h"
#include "Util.h"
#include "UploadManager.h"
#include "StringTokenizer.h"
#include "ResourceManager.h"

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
	ctrlClient.SetFont(Util::font);

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

	ctrlUsers.SetBkColor(Util::bgColor);
	ctrlUsers.SetTextBkColor(Util::bgColor);
	ctrlUsers.SetTextColor(Util::textColor);
	
	userMenu.CreatePopupMenu();
	opMenu.CreatePopupMenu();
	
	if(!images) {
		images = new CImageList();
		images->CreateFromImage(IDB_USERS, 16, 8, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION | LR_SHARED);
	}
	ctrlUsers.SetImageList(*images, LVSIL_SMALL);

	CMenuItemInfo mi;
	int n = 0;

	mi.fMask = MIIM_ID | MIIM_TYPE;
	mi.fType = MFT_STRING;
	mi.dwTypeData = const_cast<char*>(CSTRING(GET_FILE_LIST));
	mi.wID = IDC_GETLIST;
	userMenu.InsertMenuItem(n, TRUE, &mi);
	opMenu.InsertMenuItem(n++, TRUE, &mi);
	
	mi.fMask = MIIM_ID | MIIM_TYPE;
	mi.fType = MFT_STRING;
	mi.dwTypeData = const_cast<char*>(CSTRING(SEND_PRIVATE_MESSAGE));
	mi.wID = IDC_PRIVATEMESSAGE;
	userMenu.InsertMenuItem(n, TRUE, &mi);
	opMenu.InsertMenuItem(n++, TRUE, &mi);
	
	mi.fMask = MIIM_ID | MIIM_TYPE;
	mi.fType = MFT_STRING;
	mi.dwTypeData = const_cast<char*>(CSTRING(GRANT_EXTRA_SLOT));
	mi.wID = IDC_GRANTSLOT;
	userMenu.InsertMenuItem(n, TRUE, &mi);
	opMenu.InsertMenuItem(n++, TRUE, &mi);

	mi.fType = MFT_SEPARATOR;
	userMenu.InsertMenuItem(n, TRUE, &mi);
	opMenu.InsertMenuItem(n++, TRUE, &mi);

	mi.fMask = MIIM_ID | MIIM_TYPE;
	mi.fType = MFT_STRING;
	mi.dwTypeData = const_cast<char*>(CSTRING(REFRESH_USER_LIST));
	mi.wID = IDC_REFRESH;
	userMenu.InsertMenuItem(n, TRUE, &mi);
	opMenu.InsertMenuItem(n++, TRUE, &mi);

	mi.fMask = MIIM_TYPE;
	mi.fType = MFT_SEPARATOR;
	opMenu.InsertMenuItem(n++, TRUE, &mi);

	mi.fMask = MIIM_ID | MIIM_TYPE;
	mi.fType = MFT_STRING;
	mi.dwTypeData = const_cast<char*>(CSTRING(KICK_USER));
	mi.wID = IDC_KICK;
	opMenu.InsertMenuItem(n++, TRUE, &mi);

	mi.fMask = MIIM_ID | MIIM_TYPE;
	mi.fType = MFT_STRING;
	mi.dwTypeData = const_cast<char*>(CSTRING(REDIRECT));
	mi.wID = IDC_REDIRECT;
	opMenu.InsertMenuItem(n++, TRUE, &mi);
	
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
		delete message;

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
					client->sendMessage(msgs[TimerManager::getTick() % MSGS]);
			} else if(stricmp(s.c_str(), "clear") == 0) {
				ctrlClient.SetWindowText("");
			} else if(stricmp(s.c_str(), "away") == 0) {
				Util::setAway(true);
				Util::setAwayMessage(param);
				addClientLine(STRING(AWAY_MODE_ON) + Util::getAwayMessage());
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
	} else if(wParam == CLIENT_MYINFO) {
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
	} else if(wParam == CLIENT_GETPASSWORD) {
		if(client) {
			if(client->getPassword().size() > 0) {
				client->password(client->getPassword());
			} else {
				LineDlg dlg;
				dlg.title = STRING(HUB_PASSWORD) + " - " + client->getName();
				dlg.description = STRING(ENTER_PASSWORD);
				dlg.password = true;
				
				if(dlg.DoModal() == IDOK) {
					client->setPassword(dlg.line);
					client->password(dlg.line);
				} else {
					client->disconnect();
				}
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
				addClientLine(STRING(IGNORED_MESSAGE) + i->msg);
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
	if(ctrlClient.GetWindowTextLength() > 20000) {
		// We want to limit the buffer to the last 20000 characters...after that, w95 becomes sad...
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
 * $Id: HubFrame.cpp,v 1.47 2002/03/15 15:12:35 arnetheduck Exp $
 * @if LOG
 * $Log: HubFrame.cpp,v $
 * Revision 1.47  2002/03/15 15:12:35  arnetheduck
 * 0.16
 *
 * Revision 1.46  2002/03/15 11:59:35  arnetheduck
 * Final changes (I hope...) for 0.155
 *
 * Revision 1.45  2002/03/13 20:35:25  arnetheduck
 * Release canditate...internationalization done as far as 0.155 is concerned...
 * Also started using mirrors of the public hub lists
 *
 * Revision 1.44  2002/03/11 22:58:54  arnetheduck
 * A step towards internationalization
 *
 * Revision 1.43  2002/03/10 22:41:08  arnetheduck
 * Working on internationalization...
 *
 * Revision 1.42  2002/03/05 11:19:35  arnetheduck
 * Fixed a window closing bug
 *
 * Revision 1.41  2002/03/04 23:52:31  arnetheduck
 * Updates and bugfixes, new user handling almost finished...
 *
 * Revision 1.40  2002/02/28 00:10:47  arnetheduck
 * Some fixes to the new user model
 *
 * Revision 1.39  2002/02/27 12:02:09  arnetheduck
 * Completely new user handling, wonder how it turns out...
 *
 * Revision 1.38  2002/02/26 23:25:22  arnetheduck
 * Minor updates and fixes
 *
 * Revision 1.37  2002/02/25 15:39:28  arnetheduck
 * Release 0.154, lot of things fixed...
 *
 * Revision 1.36  2002/02/18 23:48:32  arnetheduck
 * New prerelease, bugs fixed and features added...
 *
 * Revision 1.35  2002/02/12 00:35:37  arnetheduck
 * 0.153
 *
 * Revision 1.34  2002/02/09 18:13:51  arnetheduck
 * Fixed level 4 warnings and started using new stl
 *
 * Revision 1.33  2002/02/07 17:25:28  arnetheduck
 * many bugs fixed, time for 0.152 I think
 *
 * Revision 1.32  2002/02/04 01:10:29  arnetheduck
 * Release 0.151...a lot of things fixed
 *
 * Revision 1.31  2002/02/03 01:06:56  arnetheduck
 * More bugfixes and some minor changes
 *
 * Revision 1.30  2002/02/01 02:00:29  arnetheduck
 * A lot of work done on the new queue manager, hopefully this should reduce
 * the number of crashes...
 *
 * Revision 1.29  2002/01/26 21:09:51  arnetheduck
 * Release 0.14
 *
 * Revision 1.28  2002/01/26 14:59:22  arnetheduck
 * Fixed disconnect crash
 *
 * Revision 1.27  2002/01/26 12:38:50  arnetheduck
 * Added some user options
 *
 * Revision 1.26  2002/01/26 12:06:39  arnetheduck
 * Småsaker
 *
 * Revision 1.25  2002/01/22 00:10:37  arnetheduck
 * Version 0.132, removed extra slots feature for nm dc users...and some bug
 * fixes...
 *
 * Revision 1.24  2002/01/20 22:54:46  arnetheduck
 * Bugfixes to 0.131 mainly...
 *
 * Revision 1.23  2002/01/17 23:35:59  arnetheduck
 * Reworked threading once more, now it actually seems stable. Also made
 * sure that noone tries to access client objects that have been deleted
 * as well as some other minor updates
 *
 * Revision 1.22  2002/01/15 21:57:53  arnetheduck
 * Hopefully fixed the two annoying bugs...
 *
 * Revision 1.21  2002/01/13 22:50:48  arnetheduck
 * Time for 0.12, added favorites, a bunch of new icons and lot's of other stuff
 *
 * Revision 1.20  2002/01/11 16:13:33  arnetheduck
 * Fixed some locks and bugs, added type field to the search frame
 *
 * Revision 1.19  2002/01/11 14:52:57  arnetheduck
 * Huge changes in the listener code, replaced most of it with templates,
 * also moved the getinstance stuff for the managers to a template
 *
 * Revision 1.18  2002/01/08 00:24:10  arnetheduck
 * Last bugs fixed before 0.11
 *
 * Revision 1.17  2002/01/07 20:17:59  arnetheduck
 * Finally fixed the reconnect bug that's been annoying me for a whole day...
 * Hopefully the app works better in w95 now too...
 *
 * Revision 1.16  2002/01/06 21:55:20  arnetheduck
 * Some minor bugs fixed, but there remains one strange thing, the reconnect
 * button doesn't work...
 *
 * Revision 1.15  2002/01/05 19:06:09  arnetheduck
 * Added user list images, fixed bugs and made things more effective
 *
 * Revision 1.13  2002/01/05 10:13:39  arnetheduck
 * Automatic version detection and some other updates
 *
 * Revision 1.12  2001/12/21 23:52:30  arnetheduck
 * Last commit for five days
 *
 * Revision 1.11  2001/12/16 19:47:48  arnetheduck
 * Reworked downloading and user handling some, and changed some small UI things
 *
 * Revision 1.10  2001/12/15 17:01:06  arnetheduck
 * Passive mode searching as well as some searching code added
 *
 * Revision 1.9  2001/12/13 19:21:57  arnetheduck
 * A lot of work done almost everywhere, mainly towards a friendlier UI
 * and less bugs...time to release 0.06...
 *
 * Revision 1.8  2001/12/08 20:59:26  arnetheduck
 * Fixing bugs...
 *
 * Revision 1.7  2001/12/08 14:25:49  arnetheduck
 * More bugs removed...did my first search as well...
 *
 * Revision 1.6  2001/12/07 20:03:07  arnetheduck
 * More work done towards application stability
 *
 * Revision 1.5  2001/12/05 19:40:13  arnetheduck
 * More bugfixes.
 *
 * Revision 1.4  2001/12/02 23:47:35  arnetheduck
 * Added the framework for uploading and file sharing...although there's something strange about
 * the file lists...my client takes them, but not the original...
 *
 * Revision 1.3  2001/11/26 23:40:36  arnetheduck
 * Downloads!! Now downloads are possible, although the implementation is
 * likely to change in the future...more UI work (splitters...) and some bug
 * fixes. Only user file listings are downloadable, but at least it's something...
 *
 * Revision 1.2  2001/11/24 10:37:09  arnetheduck
 * onQuit is now handled
 * User list sorting
 * File sizes correcly cut down to B, kB, MB, GB and TB
 *
 * Revision 1.1.1.1  2001/11/21 17:33:20  arnetheduck
 * Inital release
 *
 * @endif
 */


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
#include "DownloadManager.h"
#include "LineDlg.h"

CImageList* HubFrame::images = NULL;

LRESULT HubFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	
	CreateSimpleStatusBar(ATL_IDS_IDLEMESSAGE, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SBARS_SIZEGRIP);
	ctrlStatus.Attach(m_hWndStatusBar);
	
	ctrlClient.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		WS_VSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_NOHIDESEL | ES_READONLY, WS_EX_CLIENTEDGE);

	ctrlClient.FmtLines(TRUE);
	ctrlClient.LimitText(0);
	
	ctrlMessage.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		ES_AUTOHSCROLL, WS_EX_CLIENTEDGE);
	
	ctrlMessageContainer.SubclassWindow(ctrlMessage.m_hWnd);
	
	ctrlUsers.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS, WS_EX_CLIENTEDGE, IDC_USERS);

	ctrlClient.SetFont(ctrlUsers.GetFont());
	ctrlMessage.SetFont(ctrlUsers.GetFont());

	SetSplitterPanes(ctrlClient.m_hWnd, ctrlUsers.m_hWnd, false);
	SetSplitterExtendedStyle(SPLIT_PROPORTIONAL);
	m_nProportionalPos = 7500;

	ctrlUsers.InsertColumn(COLUMN_NICK, _T("Nick"), LVCFMT_LEFT, 100, COLUMN_NICK);
	ctrlUsers.InsertColumn(COLUMN_SHARED, _T("Shared"), LVCFMT_LEFT, 75, COLUMN_SHARED);
	ctrlUsers.InsertColumn(COLUMN_DESCRIPTION, _T("Description"), LVCFMT_LEFT, 100, COLUMN_DESCRIPTION);
	ctrlUsers.InsertColumn(COLUMN_CONNECTION, _T("Connection"), LVCFMT_LEFT, 75, COLUMN_CONNECTION);
	ctrlUsers.InsertColumn(COLUMN_EMAIL, _T("E-Mail"), LVCFMT_LEFT, 100, COLUMN_EMAIL);

	userMenu.CreatePopupMenu();
	opMenu.CreatePopupMenu();
	
	if(!images) {
		images = new CImageList();
		images->CreateFromImage(IDB_USERS, 16, 4, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION | LR_SHARED);
	}
	ctrlUsers.SetImageList(*images, LVSIL_SMALL);

	CMenuItemInfo mi;
	mi.fMask = MIIM_ID | MIIM_TYPE;
	mi.fType = MFT_STRING;
	mi.cch = 13;
	mi.dwTypeData = "Get File List";
	mi.wID = IDC_GETLIST;
	userMenu.InsertMenuItem(0, TRUE, &mi);
	opMenu.InsertMenuItem(0, TRUE, &mi);
	
	mi.fMask = MIIM_ID | MIIM_TYPE;
	mi.fType = MFT_STRING;
	mi.cch = 15;
	mi.dwTypeData = "Private Message";
	mi.wID = IDC_PRIVATEMESSAGE;
	userMenu.InsertMenuItem(1, TRUE, &mi);
	opMenu.InsertMenuItem(1, TRUE, &mi);
	
	mi.fType = MFT_SEPARATOR;
	userMenu.InsertMenuItem(2, TRUE, &mi);
	opMenu.InsertMenuItem(2, TRUE, &mi);

	mi.fMask = MIIM_ID | MIIM_TYPE;
	mi.fType = MFT_STRING;
	mi.cch = 17;
	mi.dwTypeData = "Refresh User List";
	mi.wID = IDC_REFRESH;
	userMenu.InsertMenuItem(3, TRUE, &mi);
	opMenu.InsertMenuItem(3, TRUE, &mi);

	mi.fMask = MIIM_TYPE;
	mi.fType = MFT_SEPARATOR;
	opMenu.InsertMenuItem(4, TRUE, &mi);

	mi.fMask = MIIM_ID | MIIM_TYPE;
	mi.fType = MFT_STRING;
	mi.cch = 9;
	mi.dwTypeData = "Kick User";
	mi.wID = IDC_KICK;
	opMenu.InsertMenuItem(5, TRUE, &mi);

	mi.fMask = MIIM_ID | MIIM_TYPE;
	mi.fType = MFT_STRING;
	mi.cch = 8;
	mi.dwTypeData = "Redirect";
	mi.wID = IDC_REDIRECT;
	opMenu.InsertMenuItem(6, TRUE, &mi);
	
	bHandled = FALSE;
	client->connect(server);
	return 1;
}

LRESULT HubFrame::onGetList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int i=-1;
	char buf[256];
	if(client->isConnected()) {
		while( (i = ctrlUsers.GetNextItem(i, LVNI_SELECTED)) != -1) {
			ctrlUsers.GetItemText(i, COLUMN_NICK, buf, 256);
			string user = buf;
			User::Ptr& u = client->getUser(user);
			try {
				if(u)
					DownloadManager::getInstance()->downloadList(u);
				else 
					DownloadManager::getInstance()->downloadList(user);
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
	if(client->isConnected()) {
		while( (i = ctrlUsers.GetNextItem(i, LVNI_SELECTED)) != -1) {
			ctrlUsers.GetItemText(i, COLUMN_NICK, buf, 256);
			string user = buf;
			User::Ptr& u = client->getUser(user);
			if(u) {
				PrivateFrame* frm = PrivateFrame::getFrame(u, m_hWndMDIClient);
				if(frm->m_hWnd == NULL) {
					frm->setTab(getTab());
					frm->CreateEx(m_hWndMDIClient);
				} else {
					frm->MDIActivate(frm->m_hWnd);
				}
			}
		}
	}
	return 0;
}

LRESULT HubFrame::onDoubleClickUsers(int idCtrl, LPNMHDR pnmh, BOOL& bHandled) {
	NMITEMACTIVATE* item = (NMITEMACTIVATE*)pnmh;
	string user;
	char buf[256];
	
	if(client->isConnected() && item->iItem != -1) {
		ctrlUsers.GetItemText(item->iItem, COLUMN_NICK, buf, 256);
		user = buf;
		User::Ptr& u = client->getUser(user);
		try {
			if(u)
				DownloadManager::getInstance()->downloadList(u);
			else 
				DownloadManager::getInstance()->downloadList(user);
		} catch(...) {
			// ...
		}
	}
	return 0;
}

LRESULT HubFrame::onKick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) { 
	LineDlg dlg;
	dlg.title = "Kick user(s)";
	dlg.description = "Please enter a reason";
	if(dlg.DoModal() == IDOK) {
		int i = -1;
		while( (i = ctrlUsers.GetNextItem(i, LVNI_SELECTED)) != -1) {
			char buf[256];
			ctrlUsers.GetItemText(i, COLUMN_NICK, buf, 256);
			string user = buf;
			User::Ptr& u = client->getUser(user);
			if(u) {
				client->sendMessage(client->getNick() + " is kicking " + u->getNick() + " because: " + dlg.line);
				client->privateMessage(u, "You are being kicked because: " + dlg.line);
				client->kick(u);
			}
		}
	}
	
	return 0; 
};

LRESULT HubFrame::onRedirect(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) { 
	LineDlg dlg1, dlg2;
	dlg1.title = "Redirect user(s)";
	dlg1.description = "Please enter a reason";
	if(dlg1.DoModal() == IDOK) {
		dlg2.title = "Redirect user(s)";
		dlg2.description = "Please enter destination server";
		if(dlg2.DoModal() == IDOK) {
			int i = -1;
			while( (i = ctrlUsers.GetNextItem(i, LVNI_SELECTED)) != -1) {
				char buf[256];
				ctrlUsers.GetItemText(i, COLUMN_NICK, buf, 256);
				string user = buf;
				User::Ptr& u = client->getUser(user);
				if(u) {
					client->opForceMove(u, dlg2.line, "You are being redirected to " + dlg2.line + ": " + dlg1.line);
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
	} else if(wParam == CLIENT_MYINFO) {
		User::Ptr& u = *(User::Ptr*)lParam;
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
			int image = u->isSet(User::OP) ? IMAGE_OP : IMAGE_USER;
			
			if(u->isSet(User::DCPLUSPLUS))
				image+=2;

			ctrlUsers.insert(l, image, (LPARAM)ui);
		} else {
			int image = u->isSet(User::OP) ? IMAGE_OP : IMAGE_USER;
			if(u->isSet(User::DCPLUSPLUS))
				image+=2;
			
			ctrlUsers.SetRedraw(FALSE);
			
			ctrlUsers.SetItem(j, 0, LVIF_IMAGE, NULL, image, 0, 0, NULL);
			ctrlUsers.SetItemText(j, 1, Util::formatBytes(u->getBytesShared()).c_str());
			ctrlUsers.SetItemText(j, 2, u->getDescription().c_str());
			ctrlUsers.SetItemText(j, 3, u->getConnection().c_str());
			ctrlUsers.SetItemText(j, 4, u->getEmail().c_str());
			
			ctrlUsers.SetRedraw(TRUE);
			RECT rc;
			ctrlUsers.GetItemRect(j, &rc, LVIR_BOUNDS);
			ctrlUsers.InvalidateRect(&rc);

			((UserInfo*)ctrlUsers.GetItemData(j))->size = u->getBytesShared();
		}
		
		delete (User::Ptr*)lParam;
	} else if(wParam==STATS) {
		ctrlStatus.SetText(1, (Util::toString(client->getUserCount()) + " users").c_str());
		ctrlStatus.SetText(2, Util::formatBytes(client->getAvailable()).c_str());

	} else if(wParam == CLIENT_QUIT) {
		User::Ptr& u = *(User::Ptr*)lParam;
		
		int item = ctrlUsers.find(u->getNick());
		if(item != -1) {
			delete (UserInfo*)ctrlUsers.GetItemData(item);
			ctrlUsers.DeleteItem(item);
		}
		delete (User::Ptr*)lParam;
	} else if(wParam == CLIENT_GETPASSWORD) {

		if(client->getPassword().size() > 0) {
			client->password(client->getPassword());
		} else {
			LineDlg dlg;
			dlg.title = "Hub Password";
			dlg.description = "Please enter your password";
			dlg.password = true;
			
			if(dlg.DoModal() == IDOK) {
				client->setPassword(dlg.line);
				client->password(dlg.line);
			} else {
				client->disconnect();
			}
		}
	} else if(wParam == CLIENT_CONNECTING) {
		addClientLine("Connecting to " + client->getServer() + "...");
		SetWindowText(client->getServer().c_str());
	} else if(wParam == CLIENT_FAILED) {
		addClientLine(*(string*)lParam);
		delete (string*)lParam;
		//ctrlClient.Invalidate();
	} else if(wParam == CLIENT_HUBNAME) {
		SetWindowText( (client->getName() + " (" + client->getServer() + ")").c_str());
		addClientLine("Connected");
	} else if(wParam == CLIENT_VALIDATEDENIED) {
		addClientLine("Your nick was already taken, please change to something else!");
		client->disconnect();
	} else if(wParam == CLIENT_PRIVATEMESSAGE) {
		PMInfo* i = (PMInfo*)lParam;
		if(i->frm->m_hWnd == NULL) {
			i->frm->setTab(getTab());
			i->frm->CreateEx(m_hWndMDIClient);
			MessageBeep(MB_OK);
		} 
		i->frm->addLine(i->msg);
		delete i;
	} else if(wParam == CLIENT_CONNECTED) {
		//ctrlClient.Invalidate();
	}
	return 0;
};

/**
 * @file HubFrame.cpp
 * $Id: HubFrame.cpp,v 1.25 2002/01/22 00:10:37 arnetheduck Exp $
 * @if LOG
 * $Log: HubFrame.cpp,v $
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


/* 
 * Copyright (C) 2001 Jacek Sieka, jacek@creatio.se
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

#include "MainFrm.h"
#include "AboutDlg.h"
#include "HubFrame.h"
#include "SearchFrm.h"
#include "PublicHubsFrm.h"
#include "SettingsDlg.h"

#include "ConnectionManager.h"
#include "DownloadManager.h"
#include "UploadManager.h"
#include "CryptoManager.h"
#include "DirectoryListing.h"
#include "DirectoryListingFrm.h"
#include "ShareManager.h"
#include "SearchManager.h"

MainFrame::~MainFrame() {
	arrows.Destroy();
}

DWORD WINAPI MainFrame::stopper(void* p) {
	MainFrame* mf = (MainFrame*)p;
	HWND wnd, wnd2 = NULL;
	while( (wnd=::GetWindow(mf->m_hWndClient, GW_CHILD)) != NULL) {
		if(wnd == wnd2) 
			Sleep(1);
		else { 
			::SendMessage(wnd, WM_CLOSE, 0, 0);
			wnd2 = wnd;
		}
	}
	TimerManager::getInstance()->removeListeners();
	
	ShareManager::deleteInstance();
	CryptoManager::deleteInstance();
	DownloadManager::deleteInstance();
	UploadManager::deleteInstance();
	SearchManager::deleteInstance();
	ConnectionManager::deleteInstance();
	ClientManager::deleteInstance();
	HubManager::deleteInstance();
	TimerManager::deleteInstance();
	mf->PostMessage(WM_CLOSE);	
	return 0;
}

LRESULT MainFrame::onSpeaker(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	cs.enter();
	if(wParam == UPLOAD_COMPLETE || wParam == UPLOAD_FAILED) {
		ctrlTransfers.DeleteItem(ctrlTransfers.find(lParam));
	} else if(wParam == UPLOAD_STARTING) {
		dcassert(uploadStarting.find(lParam) != uploadStarting.end());
		ctrlTransfers.insert(uploadStarting[lParam], 1, lParam);
		uploadStarting.erase(lParam);

	} else if(wParam == UPLOAD_TICK) {
		dcassert(uploadTick.find(lParam) != uploadTick.end());
		ctrlTransfers.SetItemText(ctrlTransfers.find(lParam), 1, uploadTick[lParam].c_str());
		uploadTick.erase(lParam);
	} else if(wParam == DOWNLOAD_ADDED) {
		dcassert(downloadAdded.find(lParam) != downloadAdded.end());
		ctrlTransfers.insert(downloadAdded[lParam], 0, lParam);
		downloadAdded.erase(lParam);
	} else if(wParam == DOWNLOAD_CONNECTING) {
		ctrlTransfers.SetItemText(ctrlTransfers.find(lParam), 1, "Connecting...");
	} else if(wParam == DOWNLOAD_FAILED) {
		dcassert(downloadFailed.find(lParam) != downloadFailed.end());
		ctrlTransfers.SetItemText(ctrlTransfers.find(lParam), 1, downloadFailed[lParam].c_str());
		downloadFailed.erase(lParam);
	} else if(wParam == DOWNLOAD_STARTING) {
		dcassert(downloadStarting.find(lParam) != downloadStarting.end());
		ctrlTransfers.SetItemText(ctrlTransfers.find(lParam), 2, downloadStarting[lParam].c_str());
		downloadStarting.erase(lParam);
	} else if(wParam == DOWNLOAD_TICK) {
		dcassert(downloadTick.find(lParam) != downloadTick.end());
		ctrlTransfers.SetItemText(ctrlTransfers.find(lParam), 1, downloadTick[lParam].c_str());
		downloadTick.erase(lParam);
	}
	cs.leave();

	return 0;
}

void MainFrame::onUploadStarting(Upload* aUpload) {
	StringList l;
	l.push_back(aUpload->getFileName());
	l.push_back("Connecting...");
	l.push_back(Util::shortenBytes(aUpload->getSize()));
	l.push_back(aUpload->getUser()->getNick() + " (" + aUpload->getUser()->getClient()->getName() + ")");
	cs.enter();
	uploadStarting[(LPARAM)aUpload] = l;
	cs.leave();
	PostMessage(WM_SPEAKER, UPLOAD_STARTING, (LPARAM)aUpload);
}

void MainFrame::onUploadTick(Upload* aUpload) {
	char buf[256];
	sprintf(buf, "Uploaded %s (%.01f%%)", Util::shortenBytes(aUpload->getPos()).c_str(), (double)aUpload->getPos()*100.0/(double)aUpload->getSize());
	cs.enter();
	uploadTick[(LPARAM)aUpload] = buf;
	cs.leave();
	PostMessage(WM_SPEAKER, UPLOAD_TICK, (LPARAM)aUpload);
}

void MainFrame::onDownloadAdded(Download* p) {
	StringList l;
	l.push_back(p->isSet(Download::USER_LIST) ? p->getLastNick() + ".DcLst" : p->getFileName());
	l.push_back("Waiting to connect...");
	l.push_back((p->getSize() != -1) ? Util::shortenBytes(p->getSize()).c_str() : "Unknown");

	if(p->getUser() && p->getUser()->isOnline()) {
		l.push_back(p->getUser()->getNick() + " (" + p->getUser()->getClient()->getName() + ")");
	} else {
		l.push_back(p->getLastNick() + " (Offline)");
	}
	
	cs.enter();
	downloadAdded[(LPARAM)p] = l;
	cs.leave();
	PostMessage(WM_SPEAKER, DOWNLOAD_ADDED, (LPARAM)p);
}

/**
 * This is an exception to the WM_SPEAKER thing....
 * -> DownloadManager::fireComplete must never be called when it's holding DownloadManager::cs
 * @todo Work this out...
 */
void MainFrame::onDownloadComplete(Download* p) {
	if(p->isSet(Download::USER_LIST)) {
		// We have a new DC listing, show it...
		ctrlTransfers.SetItemText(ctrlTransfers.find((LPARAM)p), 1, "Preparing file list...");
		HANDLE h = CreateFile(p->getTarget().c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
		if(h==INVALID_HANDLE_VALUE) {
			return;
		}
		DirectoryListing* dl = new DirectoryListing();
		DWORD size = GetFileSize(h, NULL);
		BYTE* buf = new BYTE[size];
		ReadFile(h, buf, size, &size, NULL);
		CloseHandle(h);
		string tmp;
		CryptoManager::getInstance()->decodeHuffman(buf, tmp);
		delete buf;
		dl->load(tmp);

		DirectoryListingFrame* pChild = new DirectoryListingFrame(dl, p->getLastNick());
		pChild->setTab(&ctrlTab);
		SendMessage(WM_CREATEDIRECTORYLISTING, (WPARAM)pChild);
		
	}

	ctrlTransfers.DeleteItem(ctrlTransfers.find((LPARAM)p));
//	ctrlTransfers.SetItemText(ctrlTransfers.find((LPARAM)p), 1, "Download finished");
	
}

void MainFrame::onDownloadFailed(Download::Ptr aDownload, const string& aReason) {
	cs.enter();
	downloadFailed[(LPARAM) aDownload] = aReason;
	cs.leave();
	PostMessage(WM_SPEAKER, DOWNLOAD_FAILED, (LPARAM)aDownload);
}

void MainFrame::onDownloadStarting(Download* aDownload) {
	cs.enter();
	downloadStarting[(LPARAM)aDownload] = Util::shortenBytes(aDownload->getSize());
	cs.leave();
	PostMessage(WM_SPEAKER, DOWNLOAD_STARTING, (LPARAM)aDownload);
}

void MainFrame::onDownloadTick(Download* aDownload) {
	char buf[256];
	sprintf(buf, "Downloaded %s (%.01f%%)", Util::shortenBytes(aDownload->getPos()).c_str(), (double)aDownload->getPos()*100.0/(double)aDownload->getSize());
	cs.enter();
	downloadTick[(LPARAM)aDownload] = buf;
	cs.leave();
	PostMessage(WM_SPEAKER, DOWNLOAD_TICK, (LPARAM)aDownload);

}

LRESULT MainFrame::OnCreateDirectory(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	DirectoryListingFrame* dlg = (DirectoryListingFrame*)wParam;
	dlg->CreateEx(m_hWndClient);
	dlg->setWindowTitle();
	return 0;
}
LRESULT MainFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
		
	// Set window name
	SetWindowText(APPNAME " " VERSIONSTRING);
	// create command bar window
	HWND hWndCmdBar = m_CmdBar.Create(m_hWnd, rcDefault, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE);
	// attach menu
	m_CmdBar.AttachMenu(GetMenu());
	// load command bar images
	m_CmdBar.LoadImages(IDR_MDICHILD);
	// remove old menu
	SetMenu(NULL);
	
	HWND hWndToolBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_MDICHILD, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);
	
	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
	AddSimpleReBarBand(hWndCmdBar);
	AddSimpleReBarBand(hWndToolBar, NULL, TRUE);
	CreateSimpleStatusBar(ATL_IDS_IDLEMESSAGE, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SBARS_SIZEGRIP);
	
	ctrlStatus.Attach(m_hWndStatusBar);
	ctrlStatus.SetSimple(FALSE);
	
	CreateMDIClient();
	m_CmdBar.SetMDIClient(m_hWndMDIClient);
	
	arrows.CreateFromImage(IDB_ARROWS, 16, 2, CLR_DEFAULT, IMAGE_BITMAP, LR_SHARED);
	ctrlTransfers.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS, WS_EX_CLIENTEDGE, IDC_TRANSFERS);
	
	ctrlTransfers.InsertColumn(0, "File", LVCFMT_LEFT, 400, 0);
	ctrlTransfers.InsertColumn(1, "Status", LVCFMT_LEFT, 300, 1);
	ctrlTransfers.InsertColumn(2, "Size", LVCFMT_RIGHT, 100, 2);
	ctrlTransfers.InsertColumn(3, "User", LVCFMT_LEFT, 200, 3);
	
	ctrlTransfers.SetImageList(arrows, LVSIL_SMALL);
	
	ctrlTab.Create(m_hWnd, rcDefault);
	
	SetSplitterPanes(m_hWndClient, ctrlTransfers.m_hWnd);
	SetSplitterExtendedStyle(SPLIT_PROPORTIONAL);
	m_nProportionalPos = 8000;
	
	UIAddToolBar(hWndToolBar);
	UISetCheck(ID_VIEW_TOOLBAR, 1);
	UISetCheck(ID_VIEW_STATUS_BAR, 1);
	
	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	ShareManager::newInstance();
	Settings::load();	
	
	TimerManager::newInstance();
	TimerManager::getInstance()->addListener(this);
	CryptoManager::newInstance();
	SearchManager::newInstance();
	ClientManager::newInstance();
	ConnectionManager::newInstance();
	DownloadManager::newInstance();
	UploadManager::newInstance();
	HubManager::newInstance();
	DownloadManager::getInstance()->addListener(this);
	UploadManager::getInstance()->addListener(this);

	ShareManager::getInstance()->refresh();

	if(Settings::getConnectionType() == Settings::CONNECTION_ACTIVE) {
		try {
			int port;
			if(Settings::getPort() == -1) {
				port = 412;
			} else {
				port = Settings::getPort();
			}
			ConnectionManager::getInstance()->setPort(port);
			SearchManager::getInstance()->setPort(port);
		} catch(Exception e) {
			dcdebug("MainFrame::OnCreate caught %s\n", e.getError().c_str());
			MessageBox("Port 412 is busy, please choose another one in the settings dialog, or disable any other application that might be using it and restart DC++");
		}
	}
	// We want to pass this one on to the splitter...hope it get's there...
	bHandled = FALSE;
	return 0;
}

LRESULT MainFrame::OnFileSearch(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	
	SearchFrame* pChild = new SearchFrame();
	pChild->setTab(&ctrlTab);
	pChild->CreateEx(m_hWndClient);
	return 0;
}	
	
LRESULT MainFrame::OnFileConnect(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(PublicHubsFrame::frame == NULL) {
		PublicHubsFrame* pChild = new PublicHubsFrame();
		pChild->setTab(&ctrlTab);
		pChild->CreateEx(m_hWndClient);
	} else {
		PublicHubsFrame::frame->SetFocus();
	}

/*	HANDLE h = CreateFile("c:\\temp\\test.dcl", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	BYTE* buf = new BYTE[GetFileSize(h, NULL)];
	DWORD x;

	ReadFile(h, buf, GetFileSize(h, NULL), &x, NULL);
	string tmp1;
	CloseHandle(h);

	CryptoManager::getInstance()->decodeHuffman(buf, tmp1);
	delete buf;
*/	
	return 0;
}

LRESULT MainFrame::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	CAboutDlg dlg;
	dlg.DoModal(m_hWnd);
	return 0;
}

LRESULT MainFrame::OnFileSettings(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	SettingsDlg dlg;
	dlg.nick = Settings::getNick();
	dlg.email = Settings::getEmail();
	dlg.description = Settings::getDescription();
	dlg.connection = Settings::getConnection();
	dlg.server = Settings::getServer();
	dlg.directory = Settings::getDownloadDirectory();
	dlg.port = Settings::getPortString();
	dlg.connectionType = Settings::getConnectionType();
	dlg.slots = Settings::getSlots();

	if(dlg.DoModal(m_hWnd) == IDOK) {
		Settings::setNick(dlg.nick);
		Settings::setDescription(dlg.description);
		Settings::setEmail(dlg.email);
		Settings::setConnection(dlg.connection);
		Settings::setServer(dlg.server);
		Settings::setPort(dlg.port);
		Settings::setConnectionType(dlg.connectionType);
		Settings::setDownloadDirectory(dlg.directory);
		Settings::setSlots(dlg.slots);
		Settings::save();

		ShareManager::getInstance()->refresh();

		if(Settings::getConnectionType() == Settings::CONNECTION_ACTIVE) {
			try {
				int port;
				if(Settings::getPort() == -1) {
					port = 412;
				} else {
					port = Settings::getPort();
				}
				ConnectionManager::getInstance()->setPort(port);
				SearchManager::getInstance()->setPort(port);
			} catch(Exception e) {
				dcdebug("MainFrame::OnCreate caught %s\n", e.getError().c_str());
				MessageBox("Port 412 is busy, please choose another one in the settings dialog, or disable any other application that might be using it and restart DC++");
			}
		}
	}
	return 0;
}

/**
 * @file MainFrm.cpp
 * $Id: MainFrm.cpp,v 1.24 2001/12/27 18:14:36 arnetheduck Exp $
 * @if LOG
 * $Log: MainFrm.cpp,v $
 * Revision 1.24  2001/12/27 18:14:36  arnetheduck
 * Version 0.08, here we go...
 *
 * Revision 1.23  2001/12/27 12:05:00  arnetheduck
 * Added flat tabs, fixed sorting and a StringTokenizer bug
 *
 * Revision 1.22  2001/12/21 23:52:30  arnetheduck
 * Last commit for five days
 *
 * Revision 1.21  2001/12/21 20:21:17  arnetheduck
 * Private messaging added, and a lot of other updates as well...
 *
 * Revision 1.20  2001/12/19 23:07:59  arnetheduck
 * Added directory downloading from the directory tree (although it hasn't been
 * tested at all) and password support.
 *
 * Revision 1.19  2001/12/16 19:47:48  arnetheduck
 * Reworked downloading and user handling some, and changed some small UI things
 *
 * Revision 1.18  2001/12/13 19:21:57  arnetheduck
 * A lot of work done almost everywhere, mainly towards a friendlier UI
 * and less bugs...time to release 0.06...
 *
 * Revision 1.17  2001/12/12 00:06:04  arnetheduck
 * Updated the public hub listings, fixed some minor transfer bugs, reworked the
 * sockets to use only one thread (instead of an extra thread for sending files),
 * and fixed a major bug in the client command decoding (still have to fix this
 * one for the userconnections...)
 *
 * Revision 1.16  2001/12/11 01:10:29  arnetheduck
 * More bugfixes...I really have to change the bufferedsocket so that it only
 * uses one thread...or maybe even multiple sockets/thread...
 *
 * Revision 1.15  2001/12/08 20:59:26  arnetheduck
 * Fixing bugs...
 *
 * Revision 1.14  2001/12/08 14:25:49  arnetheduck
 * More bugs removed...did my first search as well...
 *
 * Revision 1.13  2001/12/07 20:03:13  arnetheduck
 * More work done towards application stability
 *
 * Revision 1.12  2001/12/04 21:50:34  arnetheduck
 * Work done towards application stability...still a lot to do though...
 * a bit more and it's time for a new release.
 *
 * Revision 1.11  2001/12/03 20:52:19  arnetheduck
 * Blah! Finally, the listings are working...one line of code missing (of course),
 * but more than 2 hours of search...hate that kind of bugs...=(...some other
 * things spiffed up as well...
 *
 * Revision 1.10  2001/12/02 23:47:35  arnetheduck
 * Added the framework for uploading and file sharing...although there's something strange about
 * the file lists...my client takes them, but not the original...
 *
 * Revision 1.9  2001/12/02 14:05:36  arnetheduck
 * More sorting work, the hub list is now fully usable...
 *
 * Revision 1.8  2001/12/02 11:16:46  arnetheduck
 * Optimised hub listing, removed a few bugs and leaks, and added a few small
 * things...downloads are now working, time to start writing the sharing
 * code...
 *
 * Revision 1.7  2001/12/01 17:15:03  arnetheduck
 * Added a crappy version of huffman encoding, and some other minor changes...
 *
 * Revision 1.6  2001/11/29 19:10:55  arnetheduck
 * Refactored down/uploading and some other things completely.
 * Also added download indicators and download resuming, along
 * with some other stuff.
 *
 * Revision 1.5  2001/11/26 23:40:36  arnetheduck
 * Downloads!! Now downloads are possible, although the implementation is
 * likely to change in the future...more UI work (splitters...) and some bug
 * fixes. Only user file listings are downloadable, but at least it's something...
 *
 * Revision 1.4  2001/11/25 22:06:25  arnetheduck
 * Finally downloading is working! There are now a few quirks and bugs to be fixed
 * but what the heck....!
 *
 * Revision 1.3  2001/11/22 20:42:18  arnetheduck
 * Fixed Settings dialog (Speed setting actually works now!)
 *
 * Revision 1.2  2001/11/22 19:47:42  arnetheduck
 * A simple XML parser. Doesn't have all the features, but works good enough for
 * the configuration file.
 *
 * Revision 1.1.1.1  2001/11/21 17:33:20  arnetheduck
 * Inital release
 *
 * @endif
 */


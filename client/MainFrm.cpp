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
#include "PublicHubsDlg.h"
#include "SettingsDlg.h"

#include "ConnectionManager.h"
#include "DownloadManager.h"
#include "UploadManager.h"
#include "CryptoManager.h"

#include "DirectoryListing.h"
#include "DirectoryListingFrm.h"

MainFrame::~MainFrame() {
	ProtocolHandler::deleteInstance();
	DownloadManager::getInstance()->removeListener(this);
	CryptoManager::deleteInstance();
	ConnectionManager::deleteInstance();
	DownloadManager::deleteInstance();
	UploadManager::deleteInstance();
	HubManager::deleteInstance();
}

void MainFrame::onDownloadAdded(Download::Ptr p) {
	int i = ctrlDownloads.insert(ctrlDownloads.GetItemCount(), p->fileName.c_str(), (LPARAM)p);
	char buf[24];
	ctrlDownloads.SetItemText(i, 2, _i64toa(p->size, buf, 10));
}

void MainFrame::onDownloadConnecting(Download* aDownload) {
	ctrlDownloads.SetItemText(ctrlDownloads.find((LPARAM)aDownload), 1, ("Connecting to " + aDownload->user->getNick()).c_str());
}
void MainFrame::onDownloadComplete(Download::Ptr p) {
	if(p->fileName.find(".DcLst")!=string::npos) {
		// We have a new DC listing, show it...
		DirectoryListing* dl = new DirectoryListing();
		HANDLE h = CreateFile(p->targetFileName.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
		if(h==NULL) {
			return;
		}
		DWORD size = GetFileSize(h, NULL);
		char* buf = new char[size];
		ReadFile(h, buf, size, &size, NULL);
		string code(buf, size);
		delete buf;
		CloseHandle(h);
		string tmp;
		CryptoManager::getInstance()->decodeHuffman(code, tmp);
		dl->load(tmp);
		
		DirectoryListingFrame* pChild = new DirectoryListingFrame(dl, p->user);
		SendMessage(WM_CREATEDIRECTORYLISTING, (WPARAM)pChild);
		
	}

	ctrlDownloads.DeleteItem(ctrlDownloads.find((LPARAM)p));
//	ctrlDownloads.SetItemText(ctrlDownloads.find((LPARAM)p), 1, "Download finished");
	
}

void MainFrame::onDownloadFailed(Download::Ptr aDownload, const string& aReason) {
	ctrlDownloads.SetItemText(ctrlDownloads.find((LPARAM)aDownload), 1, aReason.c_str());
}
void MainFrame::onDownloadStarting(Download* aDownload) {
	char buf[24];
	ctrlDownloads.SetItemText(ctrlDownloads.find((LPARAM)aDownload), 2, _i64toa(aDownload->size, buf, 10));
	ctrlDownloads.SetItemText(ctrlDownloads.find((LPARAM)aDownload), 3, aDownload->user->getNick().c_str());
}
void MainFrame::onDownloadTick(Download* aDownload) {
	char buf[1024];
	sprintf(buf, "Downloaded %I64d bytes(%.01f%%)", aDownload->pos, (double)aDownload->pos*100.0/(double)aDownload->size);
	ctrlDownloads.SetItemText(ctrlDownloads.find((LPARAM)aDownload), 1, buf);
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
	CreateSimpleStatusBar();
	
	CreateMDIClient();
	m_CmdBar.SetMDIClient(m_hWndMDIClient);
	
	ctrlDownloads.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS, WS_EX_CLIENTEDGE, IDC_USERS);
	
	ctrlDownloads.InsertColumn(0, "File", LVCFMT_LEFT, 300, 0);
	ctrlDownloads.InsertColumn(1, "Status", LVCFMT_LEFT, 400, 1);
	ctrlDownloads.InsertColumn(2, "Size", LVCFMT_LEFT, 300, 2);
	ctrlDownloads.InsertColumn(3, "Downloading from", LVCFMT_LEFT, 100, 3);

	SetSplitterPanes(m_hWndClient, ctrlDownloads.m_hWnd);
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

	Settings::load();	
	
	CryptoManager::newInstance();
	ConnectionManager::newInstance();
	DownloadManager::newInstance();
	UploadManager::newInstance();
	HubManager::newInstance();
	ProtocolHandler::newInstance();
	DownloadManager::getInstance()->addListener(this);

	// We want to pass this one on to the splitter...hope it get's there...
	bHandled = FALSE;
	
	return 0;
}
	
LRESULT MainFrame::OnFileConnect(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	PublicHubsDlg dlg;
	if(dlg.DoModal(m_hWnd) == IDCANCEL) {
		return 0;
	}
	if(dlg.server.length() == 0) {
		return 0;
	}
	
	HubFrame* pChild = new HubFrame(dlg.server);
	pChild->CreateEx(m_hWndClient);

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
	dlg.port = Settings::getPort();
	dlg.connectionType = Settings::getConnectionType();
	if(dlg.DoModal(m_hWnd) == IDOK) {
		Settings::setNick(dlg.nick);
		Settings::setDescription(dlg.description);
		Settings::setEmail(dlg.email);
		Settings::setConnection(dlg.connection);
		Settings::setServer(dlg.server);
		Settings::setPort(dlg.port);
		Settings::setConnectionType(dlg.connectionType);
		Settings::save();

		ConnectionManager::getInstance()->setPort(atoi(Settings::getPort().c_str()));
	}
	return 0;
}

/**
 * @file MainFrm.cpp
 * $Id: MainFrm.cpp,v 1.6 2001/11/29 19:10:55 arnetheduck Exp $
 * @if LOG
 * $Log: MainFrm.cpp,v $
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


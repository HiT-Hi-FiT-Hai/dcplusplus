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

#include "IncomingManager.h"
#include "DownloadManager.h"
#include "UploadManager.h"
#include "CryptoManager.h"

#include "DirectoryListing.h"
#include "DirectoryListingFrm.h"

#include <fstream>
#include <strstream>

MainFrame::~MainFrame() {
	DownloadManager::getInstance()->removeListener(this);
	CryptoManager::deleteInstance();
	IncomingManager::deleteInstance();
	DownloadManager::deleteInstance();
	UploadManager::deleteInstance();
	HubManager::deleteInstance();
}

void MainFrame::onDownloadFailed(Download::Ptr p, const string& aReason) {
	MessageBox(aReason.c_str(), "Download failed", MB_OK | MB_ICONERROR);
}

void MainFrame::onDownloadComplete(Download::Ptr p) {
	if(p->fileName.find(".DcLst")) {
		// We have a new DC listing, show it...
		DirectoryListing* dl = new DirectoryListing();
		HANDLE h = CreateFile(p->destination.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
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
		
		DirectoryListingFrame* pChild = new DirectoryListingFrame(dl, p->lastUser);
		SendMessage(WM_CREATEDIRECTORYLISTING, (WPARAM)pChild);
		
	} else {
		MessageBox(("Download of " + p->fileName + " complete!").c_str());
	}
}
LRESULT MainFrame::OnCreateDirectory(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	DirectoryListingFrame* dlg = (DirectoryListingFrame*)wParam;
	dlg->CreateEx(m_hWndClient);
	dlg->setWindowTitle();
	return 0;
}
LRESULT MainFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		
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
	IncomingManager::newInstance();
	DownloadManager::newInstance();
	UploadManager::newInstance();
	HubManager::newInstance();
	
	DownloadManager::getInstance()->addListener(this);
	
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

		IncomingManager::getInstance()->setPort(atoi(Settings::getPort().c_str()));
	}
	return 0;
}

/**
 * @file MainFrm.cpp
 * $Id: MainFrm.cpp,v 1.5 2001/11/26 23:40:36 arnetheduck Exp $
 * @if LOG
 * $Log: MainFrm.cpp,v $
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


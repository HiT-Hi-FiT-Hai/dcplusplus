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

#include "MainFrm.h"
#include "AboutDlg.h"
#include "HubFrame.h"
#include "SearchFrm.h"
#include "PublicHubsFrm.h"
#include "SimpleXML.h"
#include "PropertiesDlg.h"

#include "ConnectionManager.h"
#include "DownloadManager.h"
#include "UploadManager.h"
#include "CryptoManager.h"
#include "DirectoryListing.h"
#include "DirectoryListingFrm.h"
#include "ShareManager.h"
#include "SearchManager.h"
#include "FavoritesFrm.h"
#include "NotepadFrame.h"
#include "QueueManager.h"
#include "QueueFrame.h"

MainFrame::~MainFrame() {
	arrows.Destroy();
	images.Destroy();
	largeImages.Destroy();
	DeleteObject(Util::bgBrush);
	DeleteObject(Util::font);
}

DWORD WINAPI MainFrame::stopper(void* p) {
	MainFrame* mf = (MainFrame*)p;
	HWND wnd, wnd2 = NULL;

	ConnectionManager::getInstance()->disconnectAll();
	
	while( (wnd=::GetWindow(mf->m_hWndClient, GW_CHILD)) != NULL) {
		if(wnd == wnd2) 
			Sleep(100);
		else { 
			::SendMessage(wnd, WM_CLOSE, 0, 0);
			wnd2 = wnd;
		}
	}
	TimerManager::getInstance()->removeListeners();
	
	SettingsManager::getInstance()->save();
	SettingsManager::deleteInstance();
	
	ShareManager::deleteInstance();
	CryptoManager::deleteInstance();
	DownloadManager::deleteInstance();
	UploadManager::deleteInstance();
	QueueManager::deleteInstance();
	ConnectionManager::deleteInstance();
	SearchManager::deleteInstance();
	ClientManager::deleteInstance();
	HubManager::deleteInstance();
	TimerManager::deleteInstance();
	mf->PostMessage(WM_CLOSE);	
	return 0;
}

LRESULT MainFrame::onSpeaker(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
		
	if(wParam == ADD_DOWNLOAD_ITEM) {
		StringListInfo* i = (StringListInfo*)lParam;
		StringList l;
		for(int j = 0; j < COLUMN_LAST; j++) {
			l.push_back(i->columns[j]);
		}
		dcassert(ctrlTransfers.find(i->lParam) == -1);
		ctrlTransfers.insert(l, IMAGE_DOWNLOAD, i->lParam);
		delete i;
	} else if(wParam == ADD_UPLOAD_ITEM) {
		StringListInfo* i = (StringListInfo*)lParam;
		StringList l;
		for(int j = 0; j < COLUMN_LAST; j++) {
			l.push_back(i->columns[j]);
		}
		dcassert(ctrlTransfers.find(i->lParam) == -1);
		ctrlTransfers.insert(l, IMAGE_UPLOAD, i->lParam);
		delete i;
	} else if(wParam == REMOVE_ITEM) {
		dcassert(ctrlTransfers.find(lParam) != -1);
		ctrlTransfers.DeleteItem(ctrlTransfers.find(lParam));
	} else if(wParam == SET_TEXT) {
		StringListInfo* l = (StringListInfo*)lParam;
		int n = ctrlTransfers.find(l->lParam);
		if(n != -1) {
			ctrlTransfers.SetRedraw(FALSE);
			for(int i = 0; i < COLUMN_LAST; i++) {
				if(!l->columns[i].empty()) {
					ctrlTransfers.SetItemText(n, i, l->columns[i].c_str());
				}
			}
			ctrlTransfers.SetRedraw(TRUE);
		}
		delete l;
	} else if(wParam == DOWNLOAD_LISTING) {
		DirectoryListInfo* i = (DirectoryListInfo*)lParam;
		int n = ctrlTransfers.find(i->lParam);
		ctrlTransfers.SetItemText(n, COLUMN_STATUS, "Preparing file list...");
		try {
			File f(i->file, File::READ, File::OPEN);

			DirectoryListing* dl = new DirectoryListing();
			DWORD size = (DWORD)f.getSize();

			string tmp;
			if(size > 16) {
				BYTE* buf = new BYTE[size];
				f.read(buf, size);
				CryptoManager::getInstance()->decodeHuffman(buf, tmp);
				delete buf;
			} else {
				tmp = "";
			}
			dl->load(tmp);
			
			DirectoryListingFrame* pChild = new DirectoryListingFrame(dl, i->user);
			pChild->setTab(&ctrlTab);
			pChild->CreateEx(m_hWndClient);
			pChild->setWindowTitle();
			delete i;
		} catch(FileException e) {
			// ...
		}
		ctrlTransfers.SetItemText(n, COLUMN_STATUS, "Download finished, idle...");
	} else if(wParam == STATS) {
		StringList* str = (StringList*)lParam;
		if(ctrlStatus.IsWindow()) {

			ctrlStatus.SetText(1, (*str)[0].c_str());
			ctrlStatus.SetText(2, (*str)[1].c_str());
			ctrlStatus.SetText(3, (*str)[2].c_str());
			ctrlStatus.SetText(4, (*str)[3].c_str());
			ctrlStatus.SetText(5, (*str)[4].c_str());
		}
		delete str;
	} else if(wParam == AUTO_CONNECT) {
		HubManager::getInstance()->addListener(this);
		HubManager::getInstance()->getFavoriteHubs();
	}

	return 0;
}

void MainFrame::onConnectionAdded(ConnectionQueueItem* aCqi) {
	StringListInfo* i = new StringListInfo((LPARAM)aCqi);
	i->columns[COLUMN_USER] = aCqi->getUser()->getNick() + " (" + aCqi->getUser()->getClientName() + ")";
	i->columns[COLUMN_STATUS] = "Connecting...";

	if(aCqi->getConnection() && aCqi->getConnection()->isSet(UserConnection::FLAG_UPLOAD)) {
		PostMessage(WM_SPEAKER, ADD_UPLOAD_ITEM, (LPARAM)i);
	} else {
		PostMessage(WM_SPEAKER, ADD_DOWNLOAD_ITEM, (LPARAM)i);
	}
}

void MainFrame::onConnectionStatus(ConnectionQueueItem* aCqi) {
	StringListInfo* i = new StringListInfo((LPARAM)aCqi);
	i->columns[COLUMN_STATUS] = aCqi->getStatus() == ConnectionQueueItem::CONNECTING ? "Connecting..." : "Waiting to retry...";
	PostMessage(WM_SPEAKER, SET_TEXT, (LPARAM)i);
}


void MainFrame::onConnectionRemoved(ConnectionQueueItem* aCqi) {
	PostMessage(WM_SPEAKER, REMOVE_ITEM, (LPARAM)aCqi);
}

void MainFrame::onConnectionFailed(ConnectionQueueItem* aCqi, const string& aReason) {
	StringListInfo* i = new StringListInfo((LPARAM)aCqi);
	i->columns[COLUMN_STATUS] = aReason;
	PostMessage(WM_SPEAKER, SET_TEXT, (LPARAM)i);
}
void MainFrame::onUploadStarting(Upload* aUpload) {
	StringListInfo* i = new StringListInfo((LPARAM)aUpload->getCQI());
	i->columns[COLUMN_FILE] = aUpload->getFileName();
	i->columns[COLUMN_STATUS] = "Upload starting...";
	i->columns[COLUMN_SIZE] = Util::formatBytes(aUpload->getSize());

	PostMessage(WM_SPEAKER, SET_TEXT, (LPARAM)i);
}

void MainFrame::onUploadTick(Upload* aUpload) {
	char buf[128];
	LONGLONG dif = (LONGLONG)(TimerManager::getTick() - aUpload->getStart());
	LONGLONG seconds = 0;
	LONGLONG avg = 0;
	if(dif > 0) {
		avg = aUpload->getTotal() * (LONGLONG)1000 / dif;
		if(avg > 0) {
			seconds = (aUpload->getSize() - aUpload->getPos()) / avg;
		}
	}

	sprintf(buf, "Uploaded %s (%.01f%%), %s/s, %s left", Util::formatBytes(aUpload->getPos()).c_str(), 
		(double)aUpload->getPos()*100.0/(double)aUpload->getSize(), Util::formatBytes(avg).c_str(), Util::formatSeconds(seconds).c_str());

	StringListInfo* i = new StringListInfo((LPARAM)aUpload->getCQI());
	i->columns[COLUMN_STATUS] = buf;
	PostMessage(WM_SPEAKER, SET_TEXT, (LPARAM)i);
}

void MainFrame::onUploadComplete(Upload* aUpload) {
	StringListInfo* i = new StringListInfo((LPARAM)aUpload->getCQI());
	i->columns[COLUMN_STATUS] = "Upload finished, idle...";
	PostMessage(WM_SPEAKER, SET_TEXT, (LPARAM)i);	
	
}

void MainFrame::onDownloadComplete(Download* p) {
	if(p->isSet(Download::USER_LIST)) {
		// We have a new DC listing, show it...
		DirectoryListInfo* i = new DirectoryListInfo((LPARAM)p->getCQI());
		i->file = p->getTarget();
		i->user = p->getCQI()->getUser();
		
		PostMessage(WM_SPEAKER, DOWNLOAD_LISTING, (LPARAM)i);
	} else {
		StringListInfo* i = new StringListInfo((LPARAM)p->getCQI());
		i->columns[COLUMN_STATUS] = "Download finished, idle...";
		PostMessage(WM_SPEAKER, SET_TEXT, (LPARAM)i);	
	}
}

void MainFrame::onDownloadFailed(Download* aDownload, const string& aReason) {
	StringListInfo* i = new StringListInfo((LPARAM)aDownload->getCQI());
	i->columns[COLUMN_STATUS] = aReason;
	PostMessage(WM_SPEAKER, SET_TEXT, (LPARAM)i);
}

void MainFrame::onDownloadStarting(Download* aDownload) {
	StringListInfo* i = new StringListInfo((LPARAM)aDownload->getCQI());
	i->columns[COLUMN_FILE] = aDownload->getTargetFileName();
	i->columns[COLUMN_STATUS] = "Download starting...";
	i->columns[COLUMN_SIZE] = Util::formatBytes(aDownload->getSize());
	
	PostMessage(WM_SPEAKER, SET_TEXT, (LPARAM)i);
}

void MainFrame::onDownloadTick(Download* aDownload) {
	char buf[256];
	LONGLONG dif = (LONGLONG)(TimerManager::getTick() - aDownload->getStart());
	LONGLONG seconds = 0;
	LONGLONG avg = 0;
	if(dif > 0) {
		avg = aDownload->getTotal() * (LONGLONG)1000 / dif;
		if(avg > 0) {
			seconds = (aDownload->getSize() - aDownload->getPos()) / avg;
		}
	}
	
	sprintf(buf, "Downloaded %s (%.01f%%), %s/s, %s left", Util::formatBytes(aDownload->getPos()).c_str(), 
		(double)aDownload->getPos()*100.0/(double)aDownload->getSize(), Util::formatBytes(avg).c_str(), Util::formatSeconds(seconds).c_str());
	
	StringListInfo* i = new StringListInfo((LPARAM)aDownload->getCQI());
	i->columns[COLUMN_STATUS] = buf;
	PostMessage(WM_SPEAKER, SET_TEXT, (LPARAM)i);
}

HWND MainFrame::createToolbar() {
	
	CToolBarCtrl ctrl;
	ctrl.Create(m_hWnd, NULL, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS, 0, ATL_IDW_TOOLBAR);
	ctrl.SetImageList(largeImages);
	
	TBBUTTON tb[7];
	memset(tb, 0, sizeof(tb));
	int n = 0;
	tb[n].iBitmap = n;
	tb[n].idCommand = ID_FILE_CONNECT;
	tb[n].fsState = TBSTATE_ENABLED;
	tb[n].fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE;

	n++;
	tb[n].iBitmap = n;
	tb[n].idCommand = ID_FILE_RECONNECT;
	tb[n].fsState = TBSTATE_ENABLED;
	tb[n].fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE;
	
	n++;
	tb[n].iBitmap = n;
	tb[n].idCommand = IDC_FOLLOW;
	tb[n].fsState = TBSTATE_ENABLED;
	tb[n].fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE;

	n++;
	tb[n].iBitmap = n;
	tb[n].idCommand = IDC_FAVORITES;
	tb[n].fsState = TBSTATE_ENABLED;
	tb[n].fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE;

	n++;
	tb[n].iBitmap = n;
	tb[n].idCommand = ID_FILE_SEARCH;
	tb[n].fsState = TBSTATE_ENABLED;
	tb[n].fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE;
	
	n++;
	tb[n].iBitmap = n;
	tb[n].idCommand = ID_FILE_SETTINGS;
	tb[n].fsState = TBSTATE_ENABLED;
	tb[n].fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE;

	n++;
	tb[n].iBitmap = n;
	tb[n].idCommand = IDC_NOTEPAD;
	tb[n].fsState = TBSTATE_ENABLED;
	tb[n].fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE;
	
	
	ctrl.SetButtonStructSize();
	ctrl.AddButtons(7, tb);
	
	ctrl.AutoSize();

	return ctrl.m_hWnd;
	
}

LRESULT MainFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {

	SettingsManager::newInstance();
	ShareManager::newInstance();
	TimerManager::newInstance();
	CryptoManager::newInstance();
	SearchManager::newInstance();
	ClientManager::newInstance();
	ConnectionManager::newInstance();
	DownloadManager::newInstance();
	UploadManager::newInstance();
	HubManager::newInstance();
	QueueManager::newInstance();

	TimerManager::getInstance()->addListener(this);
	DownloadManager::getInstance()->addListener(this);
	UploadManager::getInstance()->addListener(this);
	ConnectionManager::getInstance()->addListener(this);
	
	SettingsManager::getInstance()->load();	
	
	ShareManager::getInstance()->refresh(false, false);
	HubManager::getInstance()->refresh();

	TimerManager::getInstance()->start();
	
	Util::bgBrush = CreateSolidBrush(SETTING(BACKGROUND_COLOR));
	Util::textColor = SETTING(TEXT_COLOR);
	Util::bgColor = SETTING(BACKGROUND_COLOR);
	
	LOGFONT lf;
	Util::decodeFont(SETTING(TEXT_FONT), lf);
	Util::font = ::CreateFontIndirect(&lf);
	
	// Set window name
	SetWindowText(APPNAME " " VERSIONSTRING);

	// Load images
	// create command bar window
	HWND hWndCmdBar = m_CmdBar.Create(m_hWnd, rcDefault, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE);
	// attach menu
	m_CmdBar.AttachMenu(GetMenu());
	// load command bar images
	images.CreateFromImage(IDB_TOOLBAR, 16, 5, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION | LR_SHARED);
	largeImages.CreateFromImage(IDB_TOOLBAR20, 20, 5, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION | LR_SHARED);
	m_CmdBar.m_hImageList = images;
	
	m_CmdBar.m_arrCommand.Add(ID_FILE_CONNECT);
	m_CmdBar.m_arrCommand.Add(ID_FILE_RECONNECT);
	m_CmdBar.m_arrCommand.Add(IDC_FOLLOW);
	m_CmdBar.m_arrCommand.Add(IDC_FAVORITES);
	m_CmdBar.m_arrCommand.Add(ID_FILE_SEARCH);
	m_CmdBar.m_arrCommand.Add(ID_FILE_SETTINGS);
	m_CmdBar.m_arrCommand.Add(IDC_NOTEPAD);
	
	// remove old menu
	SetMenu(NULL);
	
	HWND hWndToolBar = createToolbar();
	
	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
	AddSimpleReBarBand(hWndCmdBar);
	AddSimpleReBarBand(hWndToolBar, NULL, TRUE);
	CreateSimpleStatusBar(ATL_IDS_IDLEMESSAGE, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SBARS_SIZEGRIP);
	
	ctrlStatus.Attach(m_hWndStatusBar);
	ctrlStatus.SetSimple(FALSE);
	
	CreateMDIClient();
	m_CmdBar.SetMDIClient(m_hWndMDIClient);
	
	arrows.CreateFromImage(IDB_ARROWS, 16, 2, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION | LR_SHARED);
	ctrlTransfers.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS | LVS_NOSORTHEADER, WS_EX_CLIENTEDGE, IDC_TRANSFERS);

	
	ctrlTransfers.InsertColumn(COLUMN_USER, "User", LVCFMT_LEFT, 200, COLUMN_USER);
	ctrlTransfers.InsertColumn(COLUMN_STATUS, "Status", LVCFMT_LEFT, 300, COLUMN_STATUS);
	ctrlTransfers.InsertColumn(COLUMN_FILE, "File", LVCFMT_LEFT, 400, COLUMN_FILE);
	ctrlTransfers.InsertColumn(COLUMN_SIZE, "Size", LVCFMT_RIGHT, 100, COLUMN_SIZE);

	ctrlTransfers.SetBkColor(Util::bgColor);
	ctrlTransfers.SetTextBkColor(Util::bgColor);
	ctrlTransfers.SetTextColor(Util::textColor);
	
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


	if(BOOLSETTING(FULL_ROW_SELECT)) {
		ctrlTransfers.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);
	}
	
	if(SETTING(CONNECTION_TYPE) == SettingsManager::CONNECTION_ACTIVE) {
		try {
			ConnectionManager::getInstance()->setPort((short)SETTING(PORT));
			SearchManager::getInstance()->setPort((short)SETTING(PORT));
		} catch(Exception e) {
			dcdebug("MainFrame::OnCreate caught %s\n", e.getError().c_str());
			MessageBox(("Port " + Util::toString(SETTING(PORT)) + " is busy, please choose another one in the settings dialog, or disable any other application that might be using it and restart DC++").c_str());
		}
	}

	transferMenu.CreatePopupMenu();

	CMenuItemInfo mi;
	int n = 0;
	
	mi.fMask = MIIM_ID | MIIM_TYPE;
	mi.fType = MFT_STRING;
	mi.dwTypeData = "Close connection";
	mi.wID = IDC_REMOVE;
	transferMenu.InsertMenuItem(n++, TRUE, &mi);
	
	mi.fMask = MIIM_ID | MIIM_TYPE;
	mi.fType = MFT_STRING;
	mi.dwTypeData = "Force attempt";
	mi.wID = IDC_FORCE;
	transferMenu.InsertMenuItem(n++, TRUE, &mi);
	
	mi.fMask = MIIM_ID | MIIM_TYPE;
	mi.fType = MFT_STRING;
	mi.dwTypeData = "Get File List";
	mi.wID = IDC_GETLIST;
	transferMenu.InsertMenuItem(n++, TRUE, &mi);

	mi.fMask = MIIM_ID | MIIM_TYPE;
	mi.fType = MFT_STRING;
	mi.dwTypeData = "Send Private Message";
	mi.wID = IDC_PRIVATEMESSAGE;
	transferMenu.InsertMenuItem(n++, TRUE, &mi);

	c.addListener(this);
	c.downloadFile("http://dcplusplus.sourceforge.net/version.xml");

	if(BOOLSETTING(OPEN_PUBLIC))
		PostMessage(WM_COMMAND, ID_FILE_CONNECT);
	if(BOOLSETTING(OPEN_QUEUE))
		PostMessage(WM_COMMAND, IDC_QUEUE);
	
	PostMessage(WM_SPEAKER, AUTO_CONNECT);
	
	// We want to pass this one on to the splitter...hope it get's there...
	bHandled = FALSE;
	return 0;
}

LRESULT MainFrame::onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	RECT rc;                    // client area of window 
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };        // location of mouse click 
	
	// Get the bounding rectangle of the client area. 
	ctrlTransfers.GetClientRect(&rc);
	ctrlTransfers.ScreenToClient(&pt); 
	if (PtInRect(&rc, pt) && ctrlTransfers.GetSelectedCount() > 0) 
	{ 
		ctrlTransfers.ClientToScreen(&pt);
		transferMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);

		return TRUE; 
	}
	return FALSE; 
}

LRESULT MainFrame::OnFileSearch(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	
	SearchFrame* pChild = new SearchFrame();
	pChild->setTab(&ctrlTab);
	pChild->CreateEx(m_hWndClient);
	return 0;
}	

LRESULT MainFrame::onFavorites(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(FavoriteHubsFrame::frame == NULL) {
		FavoriteHubsFrame* pChild = new FavoriteHubsFrame();
		pChild->setTab(&ctrlTab);
		pChild->CreateEx(m_hWndClient);
	} else {
		MDIActivate(FavoriteHubsFrame::frame->m_hWnd);
	}
	return 0;
}

LRESULT MainFrame::onNotepad(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(NotepadFrame::frame == NULL) {
		NotepadFrame* pChild = new NotepadFrame();
		pChild->setTab(&ctrlTab);
		pChild->CreateEx(m_hWndClient);
	} else {
		MDIActivate(NotepadFrame::frame->m_hWnd);
	}
	return 0;
}

LRESULT MainFrame::onQueue(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(QueueFrame::frame == NULL) {
		QueueFrame* pChild = new QueueFrame();
		pChild->setTab(&ctrlTab);
		pChild->CreateEx(m_hWndClient);
	} else {
		MDIActivate(QueueFrame::frame->m_hWnd);
	}
	return 0;
}

LRESULT MainFrame::OnFileConnect(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(PublicHubsFrame::frame == NULL) {
		PublicHubsFrame* pChild = new PublicHubsFrame();
		pChild->setTab(&ctrlTab);
		pChild->CreateEx(m_hWndClient);
	} else {
		MDIActivate(PublicHubsFrame::frame->m_hWnd);
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

LRESULT MainFrame::OnFileSettings(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	PropertiesDlg dlg(SettingsManager::getInstance());
	
	if(dlg.DoModal(m_hWnd) == IDOK)
	{		
		SettingsManager::getInstance()->save();
		
		if(SETTING(CONNECTION_TYPE) == SettingsManager::CONNECTION_ACTIVE) {
			try {
				ConnectionManager::getInstance()->setPort((short)SETTING(PORT));
				SearchManager::getInstance()->setPort((short)SETTING(PORT));
			} catch(Exception e) {
				dcdebug("MainFrame::OnCreate caught %s\n", e.getError().c_str());
				MessageBox(("Port " + Util::toString(SETTING(PORT)) + " is busy, please choose another one in the settings dialog, or disable any other application that might be using it and restart DC++").c_str());
			}
		}
	}
	return 0;
}

void MainFrame::onHttpData(HttpConnection* /*aConn*/, const BYTE* aBuf, int aLen) {
	versionInfo += string((const char*)aBuf, aLen);
}

void MainFrame::onHttpComplete(HttpConnection* /*aConn*/)  {
	try {
		SimpleXML xml;
		xml.fromXML(versionInfo);
		xml.stepIn();
		if(xml.findChild("Version")) {
			if(atof(xml.getChildData().c_str()) > VERSIONFLOAT) {
				xml.resetCurrentChild();
				if(xml.findChild("Message")) {
					const string& msg = xml.getChildData();
					xml.resetCurrentChild();
					if(xml.findChild("Title")) {
						MessageBox(msg.c_str(), xml.getChildData().c_str());
					}
				}
				xml.resetCurrentChild();
				if(xml.findChild("VeryOldVersion")) {
					if(atof(xml.getChildData().c_str()) >= VERSIONFLOAT) {
						xml.resetCurrentChild();
						if(xml.findChild("URL")) {
							MessageBox(("Your version of DC++ is very old and will be shut down. Please get a new one at \r\n" + xml.getChildData()).c_str());
							PostMessage(WM_CLOSE);
						}
					}
				}
			}
		}
	} catch (Exception e) {
		// ...
	}
}

void MainFrame::onAction(HubManagerListener::Types type, const FavoriteHubEntry::List& fl) {
	switch(type) {
	case HubManagerListener::GET_FAVORITE_HUBS: 
		for(FavoriteHubEntry::List::const_iterator i = fl.begin(); i != fl.end(); ++i) {
			FavoriteHubEntry* entry = *i;
			if(entry->getConnect()) {
				HubFrame* frm = new HubFrame(entry->getServer(), entry->getNick(), entry->getPassword());
				frm->setTab(&ctrlTab);
				frm->CreateEx(m_hWndMDIClient);
			}
		}
		HubManager::getInstance()->removeListener(this);
		break;
	}
}

LRESULT MainFrame::onPrivateMessage(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int i = -1;
	while( (i = ctrlTransfers.GetNextItem(i, LVNI_SELECTED)) != -1) {
		PrivateFrame::openWindow(((ConnectionQueueItem*)ctrlTransfers.GetItemData(i))->getUser(), m_hWndMDIClient, &ctrlTab);
	}
	return 0;
}

LRESULT MainFrame::onForce(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int i = -1;
	while( (i = ctrlTransfers.GetNextItem(i, LVNI_SELECTED)) != -1) {
		ctrlTransfers.SetItemText(i, COLUMN_STATUS, "Connecting (forced)...");
		((ConnectionQueueItem*)ctrlTransfers.GetItemData(i))->getUser()->connect();
	}
	return 0;
}

LRESULT MainFrame::onSysCommand(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
	if (wParam== SC_MINIMIZE && BOOLSETTING(MINIMIZE_TRAY)) {
		NOTIFYICONDATA nid;
		nid.cbSize = sizeof(NOTIFYICONDATA);
		nid.hWnd = m_hWnd;
		nid.uID = 0;
		nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
		nid.uCallbackMessage = WM_APP + 242;
		nid.hIcon = (HICON)::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_MAINFRAME), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
		strncpy(nid.szTip, "DC++",64);
		nid.szTip[63] = '\0';
		
		::Shell_NotifyIcon(NIM_ADD, &nid);
		ShowWindow(SW_HIDE);
		return 0;
	}
	
	bHandled = FALSE;
	return 0;
}

LRESULT MainFrame::onGetList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) { 
	int i = -1;
	while( (i = ctrlTransfers.GetNextItem(i, LVNI_SELECTED)) != -1) {
		try {
			QueueManager::getInstance()->addList(((ConnectionQueueItem*)ctrlTransfers.GetItemData(i))->getUser());
		} catch(QueueException e) {
			ctrlStatus.SetText(0, e.getError().c_str());
		} catch(FileException e) {
			dcdebug("MainFrm::onGetList caught %s\n", e.getError().c_str());
		}
	}
	return 0;
}

/**
 * @file MainFrm.cpp
 * $Id: MainFrm.cpp,v 1.65 2002/03/05 11:19:35 arnetheduck Exp $
 * @if LOG
 * $Log: MainFrm.cpp,v $
 * Revision 1.65  2002/03/05 11:19:35  arnetheduck
 * Fixed a window closing bug
 *
 * Revision 1.64  2002/03/04 23:52:31  arnetheduck
 * Updates and bugfixes, new user handling almost finished...
 *
 * Revision 1.63  2002/02/25 15:39:29  arnetheduck
 * Release 0.154, lot of things fixed...
 *
 * Revision 1.62  2002/02/18 23:48:32  arnetheduck
 * New prerelease, bugs fixed and features added...
 *
 * Revision 1.61  2002/02/12 00:35:37  arnetheduck
 * 0.153
 *
 * Revision 1.60  2002/02/10 12:25:24  arnetheduck
 * New properties for favorites, and some minor performance tuning...
 *
 * Revision 1.59  2002/02/09 18:13:51  arnetheduck
 * Fixed level 4 warnings and started using new stl
 *
 * Revision 1.58  2002/02/07 22:12:22  arnetheduck
 * Last fixes before 0.152
 *
 * Revision 1.57  2002/02/07 17:25:28  arnetheduck
 * many bugs fixed, time for 0.152 I think
 *
 * Revision 1.56  2002/02/04 01:10:30  arnetheduck
 * Release 0.151...a lot of things fixed
 *
 * Revision 1.55  2002/02/03 01:06:56  arnetheduck
 * More bugfixes and some minor changes
 *
 * Revision 1.54  2002/02/02 17:21:27  arnetheduck
 * Fixed search bugs and some other things...
 *
 * Revision 1.53  2002/02/01 02:00:31  arnetheduck
 * A lot of work done on the new queue manager, hopefully this should reduce
 * the number of crashes...
 *
 * Revision 1.52  2002/01/26 21:09:51  arnetheduck
 * Release 0.14
 *
 * Revision 1.51  2002/01/26 14:59:22  arnetheduck
 * Fixed disconnect crash
 *
 * Revision 1.50  2002/01/26 12:52:51  arnetheduck
 * More minor fixes
 *
 * Revision 1.49  2002/01/26 12:38:50  arnetheduck
 * Added some user options
 *
 * Revision 1.48  2002/01/26 12:06:39  arnetheduck
 * Småsaker
 *
 * Revision 1.47  2002/01/25 00:15:41  arnetheduck
 * New Settings dialogs
 *
 * Revision 1.46  2002/01/25 00:11:26  arnetheduck
 * New settings dialog and various fixes
 *
 * Revision 1.45  2002/01/22 00:10:37  arnetheduck
 * Version 0.132, removed extra slots feature for nm dc users...and some bug
 * fixes...
 *
 * Revision 1.44  2002/01/20 22:54:46  arnetheduck
 * Bugfixes to 0.131 mainly...
 *
 * Revision 1.43  2002/01/19 19:07:39  arnetheduck
 * Last fixes before 0.13
 *
 * Revision 1.42  2002/01/19 13:09:10  arnetheduck
 * Added a file class to hide ugly file code...and fixed a small resume bug (I think...)
 *
 * Revision 1.41  2002/01/18 17:41:43  arnetheduck
 * Reworked many right button menus, adding op commands and making more easy to use
 *
 * Revision 1.40  2002/01/17 23:35:59  arnetheduck
 * Reworked threading once more, now it actually seems stable. Also made
 * sure that noone tries to access client objects that have been deleted
 * as well as some other minor updates
 *
 * Revision 1.39  2002/01/16 20:56:27  arnetheduck
 * Bug fixes, file listing sort and some other small changes
 *
 * Revision 1.38  2002/01/14 22:19:43  arnetheduck
 * Commiting minor bugfixes
 *
 * Revision 1.37  2002/01/13 22:50:48  arnetheduck
 * Time for 0.12, added favorites, a bunch of new icons and lot's of other stuff
 *
 * Revision 1.36  2002/01/11 14:52:57  arnetheduck
 * Huge changes in the listener code, replaced most of it with templates,
 * also moved the getinstance stuff for the managers to a template
 *
 * Revision 1.35  2002/01/10 12:33:14  arnetheduck
 * Various fixes
 *
 * Revision 1.34  2002/01/08 00:24:10  arnetheduck
 * Last bugs fixed before 0.11
 *
 * Revision 1.33  2002/01/07 20:17:59  arnetheduck
 * Finally fixed the reconnect bug that's been annoying me for a whole day...
 * Hopefully the app works better in w95 now too...
 *
 * Revision 1.32  2002/01/05 19:06:09  arnetheduck
 * Added user list images, fixed bugs and made things more effective
 *
 * Revision 1.30  2002/01/05 10:13:39  arnetheduck
 * Automatic version detection and some other updates
 *
 * Revision 1.29  2002/01/02 16:55:56  arnetheduck
 * Time for 0.09
 *
 * Revision 1.28  2002/01/02 16:12:32  arnetheduck
 * Added code for multiple download sources
 *
 * Revision 1.27  2001/12/30 17:41:16  arnetheduck
 * Fixed some XML parsing bugs
 *
 * Revision 1.26  2001/12/30 15:03:45  arnetheduck
 * Added framework to handle incoming searches
 *
 * Revision 1.25  2001/12/29 13:47:14  arnetheduck
 * Fixing bugs and UI work
 *
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


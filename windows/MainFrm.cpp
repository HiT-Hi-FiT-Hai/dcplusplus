/* 
 * Copyright (C) 2001-2003 Jacek Sieka, j_s@telia.com
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

#include "MainFrm.h"
#include "AboutDlg.h"
#include "HubFrame.h"
#include "SearchFrm.h"
#include "PublicHubsFrm.h"
#include "PropertiesDlg.h"
#include "UsersFrame.h"
#include "DirectoryListingFrm.h"
#include "FavoritesFrm.h"
#include "NotepadFrame.h"
#include "QueueFrame.h"
#include "SpyFrame.h"
#include "FinishedFrame.h"
#include "ADLSearchFrame.h"
#include "FinishedULFrame.h"
#include "TextFrame.h"

#include "../client/ConnectionManager.h"
#include "../client/DownloadManager.h"
#include "../client/UploadManager.h"
#include "../client/StringTokenizer.h"
#include "../client/SimpleXML.h"

MainFrame::~MainFrame() {
	m_CmdBar.m_hImageList = NULL;

	images.Destroy();
	largeImages.Destroy();

	WinUtil::uninit();
}

DWORD WINAPI MainFrame::stopper(void* p) {
	MainFrame* mf = (MainFrame*)p;
	HWND wnd, wnd2 = NULL;

	while( (wnd=::GetWindow(mf->m_hWndClient, GW_CHILD)) != NULL) {
		if(wnd == wnd2) 
			Sleep(100);
		else { 
			::PostMessage(wnd, WM_CLOSE, 0, 0);
			wnd2 = wnd;
		}
	}

	shutdown();
	
	mf->PostMessage(WM_CLOSE);	
	return 0;
}

LRESULT MainFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {

	TimerManager::getInstance()->addListener(this);
	QueueManager::getInstance()->addListener(this);

	WinUtil::init(m_hWnd);

	// Register server socket message
	WSAAsyncSelect(ConnectionManager::getInstance()->getServerSocket().getSocket(),
		m_hWnd, SERVER_SOCKET_MESSAGE, FD_ACCEPT);

	trayMessage = RegisterWindowMessage("TaskbarCreated");

	TimerManager::getInstance()->start();

	// Set window name
	SetWindowText(APPNAME " " VERSIONSTRING);

	// Load images
	// create command bar window
	HWND hWndCmdBar = m_CmdBar.Create(m_hWnd, rcDefault, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE);

	m_hMenu = WinUtil::mainMenu;

	// attach menu
	m_CmdBar.AttachMenu(m_hMenu);
	// load command bar images
	images.CreateFromImage(IDB_TOOLBAR, 16, 5, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION | LR_SHARED);
	largeImages.CreateFromImage(IDB_TOOLBAR20, 20, 5, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION | LR_SHARED);
	m_CmdBar.m_hImageList = images;

	m_CmdBar.m_arrCommand.Add(ID_FILE_CONNECT);
	m_CmdBar.m_arrCommand.Add(ID_FILE_RECONNECT);
	m_CmdBar.m_arrCommand.Add(IDC_FOLLOW);
	m_CmdBar.m_arrCommand.Add(IDC_FAVORITES);
	m_CmdBar.m_arrCommand.Add(IDC_QUEUE);
	m_CmdBar.m_arrCommand.Add(IDC_FINISHED); // adds icon to File menu
	m_CmdBar.m_arrCommand.Add(IDC_FINISHED_UL); // Finished Upload 16 x 16 menu icon
	m_CmdBar.m_arrCommand.Add(ID_FILE_SEARCH);
	m_CmdBar.m_arrCommand.Add(IDC_FILE_ADL_SEARCH);
	m_CmdBar.m_arrCommand.Add(ID_FILE_SETTINGS);
	m_CmdBar.m_arrCommand.Add(IDC_NOTEPAD);
	
	// remove old menu
	SetMenu(NULL);

	HWND hWndToolBar = createToolbar();

	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
	AddSimpleReBarBand(hWndCmdBar);
	AddSimpleReBarBand(hWndToolBar, NULL, TRUE);
	CreateSimpleStatusBar();

	ctrlStatus.Attach(m_hWndStatusBar);
	ctrlStatus.SetSimple(FALSE);
	int w[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	ctrlStatus.SetParts(8, w);
	statusSizes[0] = WinUtil::getTextWidth(STRING(AWAY), ::GetDC(ctrlStatus.m_hWnd)); // for "AWAY" segment

	CreateMDIClient();
	m_CmdBar.SetMDIClient(m_hWndMDIClient);
	WinUtil::mdiClient = m_hWndMDIClient;

	ctrlTab.Create(m_hWnd, rcDefault);
	WinUtil::tabCtrl = &ctrlTab;

	transferView.Create(m_hWnd);

	SetSplitterPanes(m_hWndClient, transferView.m_hWnd);
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

	c->addListener(this);
	c->downloadFile("http://dcplusplus.sourceforge.net/version.xml");

	if(BOOLSETTING(OPEN_PUBLIC))
		PostMessage(WM_COMMAND, ID_FILE_CONNECT);
	if(BOOLSETTING(OPEN_QUEUE))
		PostMessage(WM_COMMAND, IDC_QUEUE);
	if(BOOLSETTING(OPEN_FAVORITE_HUBS))
		PostMessage(WM_COMMAND, IDC_FAVORITES);
	if(BOOLSETTING(OPEN_FINISHED_DOWNLOADS))
		PostMessage(WM_COMMAND, IDC_FINISHED);

	if(!(GetAsyncKeyState(VK_SHIFT) & 0x8000))
		PostMessage(WM_SPEAKER, AUTO_CONNECT);

	PostMessage(WM_SPEAKER, PARSE_COMMAND_LINE);

	Util::ensureDirectory(SETTING(LOG_DIRECTORY));

	startSocket();

	if(SETTING(NICK).empty()) {
		PostMessage(WM_COMMAND, ID_FILE_SETTINGS);
	}

	// We want to pass this one on to the splitter...hope it get's there...
	bHandled = FALSE;
	return 0;
}

void MainFrame::startSocket() {
	SearchManager::getInstance()->disconnect();
	ConnectionManager::getInstance()->disconnect();

	if(SETTING(CONNECTION_TYPE) == SettingsManager::CONNECTION_ACTIVE) {

		short lastPort = (short)SETTING(IN_PORT);
		short firstPort = lastPort;

		while(true) {
			try {
				ConnectionManager::getInstance()->setPort(lastPort);
				WSAAsyncSelect(ConnectionManager::getInstance()->getServerSocket().getSocket(), m_hWnd, SERVER_SOCKET_MESSAGE, FD_ACCEPT);
				SearchManager::getInstance()->setPort(lastPort);
				break;
			} catch(const Exception& e) {
				dcdebug("MainFrame::OnCreate caught %s\n", e.getError().c_str());
				short newPort = (short)((lastPort == 32000) ? 1025 : lastPort + 1);
				SettingsManager::getInstance()->setDefault(SettingsManager::IN_PORT, newPort);
				if(SETTING(IN_PORT) == lastPort || (firstPort == newPort)) {
					// Changing default didn't change port, a fixed port must be in use...(or we
					// tried all ports
					char* buf = new char[STRING(PORT_IS_BUSY).size() + 8];
					sprintf(buf, CSTRING(PORT_IS_BUSY), SETTING(IN_PORT));
					MessageBox(buf);
					delete[] buf;
					break;
				}
				lastPort = newPort;
			}
		}
	}
}

HWND MainFrame::createToolbar() {

	CToolBarCtrl ctrl;
	ctrl.Create(m_hWnd, NULL, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS, 0, ATL_IDW_TOOLBAR);
	ctrl.SetImageList(largeImages);

	TBBUTTON tb[11];
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
	tb[n].idCommand = IDC_QUEUE;
	tb[n].fsState = TBSTATE_ENABLED;
	tb[n].fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE;

	n++; // toolbar icon
	tb[n].iBitmap = n;
	tb[n].idCommand = IDC_FINISHED;
	tb[n].fsState = TBSTATE_ENABLED;
	tb[n].fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE;

	n++; // Finished Upload 20x20 toolbar icon
	tb[n].iBitmap = n;
	tb[n].idCommand = IDC_FINISHED_UL;
	tb[n].fsState = TBSTATE_ENABLED;
	tb[n].fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE;

	n++;
	tb[n].iBitmap = n;
	tb[n].idCommand = ID_FILE_SEARCH;
	tb[n].fsState = TBSTATE_ENABLED;
	tb[n].fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE;

	n++;
	tb[n].iBitmap = n;
	tb[n].idCommand = IDC_FILE_ADL_SEARCH;
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
	ctrl.AddButtons(11, tb);
	ctrl.AutoSize();

	return ctrl.m_hWnd;
}



LRESULT MainFrame::onSpeaker(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
		
	if(wParam == DOWNLOAD_LISTING) {
		DirectoryListInfo* i = (DirectoryListInfo*)lParam;
		try {
			DirectoryListingFrame* pChild = new DirectoryListingFrame(i->file, i->user);
			pChild->setTab(&ctrlTab);
			pChild->CreateEx(m_hWndClient);
			pChild->setWindowTitle();
			delete i;
		} catch(const FileException&) {
			// ...
		}
	} else if(wParam == VIEW_TEXT) {
		string* file = (string*)lParam;
		TextFrame* pChild = new TextFrame(*file);
		pChild->setTab(&ctrlTab);
		pChild->CreateEx(m_hWndClient);
		delete file;
	} else if(wParam == STATS) {
		StringList& str = *(StringList*)lParam;
		if(ctrlStatus.IsWindow()) {
			HDC dc = ::GetDC(ctrlStatus.m_hWnd);
			bool u = false;
			ctrlStatus.SetText(1, str[0].c_str());
			for(int i = 1; i < 7; i++) {
				int w = WinUtil::getTextWidth(str[i], dc);
				
				if(statusSizes[i] < w) {
					statusSizes[i] = w;
					u = true;
				}
				ctrlStatus.SetText(i+1, str[i].c_str());
			}
			::ReleaseDC(ctrlStatus.m_hWnd, dc);
			if(u)
				UpdateLayout(TRUE);
		}
		delete &str;
	} else if(wParam == AUTO_CONNECT) {
		autoConnect(HubManager::getInstance()->getFavoriteHubs());
	} else if(wParam == PARSE_COMMAND_LINE) {
		parseCommandLine(GetCommandLine());
	}

	return 0;
}

void MainFrame::parseCommandLine(const string& cmdLine)
{
	string::size_type i = 0;
	string::size_type j;

	if( (j = cmdLine.find("dchub://", i)) != string::npos) {
		i = j + 8;
		string server;
		string user;
		if( (j = cmdLine.find('/', i)) == string::npos) {
			server = cmdLine.substr(i);
		} else {
			server = cmdLine.substr(i, j-i);
			i = j + 1;
			if( (j = cmdLine.find_first_of("\\/ ", i)) == string::npos) {
				user = cmdLine.substr(i);
			} else {
				user = cmdLine.substr(i, j-i);
			}
		}

		if(!server.empty()) {
			HubFrame::openWindow(m_hWndMDIClient, &ctrlTab, server);
		}
		if(!user.empty()) {
			try {
				QueueManager::getInstance()->addList(ClientManager::getInstance()->getUser(user));
			} catch(const Exception&) {
				// ...
			}
		}
	}
}

LRESULT MainFrame::onCopyData(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	string cmdLine = (LPCSTR) (((COPYDATASTRUCT *)lParam)->lpData);
	parseCommandLine(Util::getAppName() + " " + cmdLine);
	return true;
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

LRESULT MainFrame::onFavoriteUsers(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(UsersFrame::frame == NULL) {
		UsersFrame* pChild = new UsersFrame();
		pChild->setTab(&ctrlTab);
		pChild->CreateEx(m_hWndClient);
	} else {
		MDIActivate(UsersFrame::frame->m_hWnd);
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

	return 0;
}

LRESULT MainFrame::onSearchSpy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(SpyFrame::frame == NULL) {
		SpyFrame* pChild = new SpyFrame();
		pChild->setTab(&ctrlTab);
		pChild->CreateEx(m_hWndClient);
	} else {
		MDIActivate(SpyFrame::frame->m_hWnd);
	}
	
	return 0;
}

LRESULT MainFrame::onFileADLSearch(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) 
{
	if(ADLSearchFrame::frame == NULL) 
	{
		ADLSearchFrame* pChild = new ADLSearchFrame();
		pChild->setTab(&ctrlTab);
		pChild->CreateEx(m_hWndClient);
	} else 
	{
		MDIActivate(ADLSearchFrame::frame->m_hWnd);
	}

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

	short lastPort = (short)SETTING(IN_PORT);
	int lastConn = SETTING(CONNECTION_TYPE);

	if(dlg.DoModal(m_hWnd) == IDOK)
	{
		SettingsManager::getInstance()->save();
		if(SETTING(CONNECTION_TYPE) != lastConn || SETTING(IN_PORT) != lastPort) {
			startSocket();
		}
		ClientManager::getInstance()->infoUpdated();
	}
	return 0;
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
							oldshutdown = true;
							PostMessage(WM_CLOSE);
						}
					}
				}
			}
		}
	} catch (const Exception&) {
		// ...
	}
}

LRESULT MainFrame::onServerSocket(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	ConnectionManager::getInstance()->getServerSocket().incoming();
	return 0;
}

LRESULT MainFrame::onGetToolTip(int idCtrl, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	LPNMTTDISPINFO pDispInfo = (LPNMTTDISPINFO)pnmh;
	pDispInfo->szText[0] = 0;

	if((idCtrl != 0) && !(pDispInfo->uFlags & TTF_IDISHWND))
	{
		int stringId = -1;
		switch(idCtrl) {
			case ID_FILE_CONNECT: stringId = ResourceManager::MENU_PUBLIC_HUBS; break;
			case ID_FILE_RECONNECT: stringId = ResourceManager::MENU_RECONNECT; break;
			case IDC_FOLLOW: stringId = ResourceManager::MENU_FOLLOW_REDIRECT; break;
			case IDC_FAVORITES: stringId = ResourceManager::MENU_FAVORITE_HUBS; break;
			case IDC_QUEUE: stringId = ResourceManager::MENU_DOWNLOAD_QUEUE; break;
			case ID_FILE_SEARCH: stringId = ResourceManager::MENU_SEARCH; break;
			case ID_FILE_SETTINGS: stringId = ResourceManager::MENU_SETTINGS; break;
			case IDC_NOTEPAD: stringId = ResourceManager::MENU_NOTEPAD; break;
			case IDC_FILE_ADL_SEARCH: stringId = ResourceManager::MENU_ADL_SEARCH; break;
			case IDC_FINISHED: stringId = ResourceManager::FINISHED_DOWNLOADS; break; // tooltip
			case IDC_FINISHED_UL: stringId = ResourceManager::FINISHED_UPLOADS; break; // Finished Uploads tooltip
		}
		if(stringId != -1) {
			strncpy(pDispInfo->lpszText, ResourceManager::getInstance()->getString((ResourceManager::Strings)stringId).c_str(), 79);
			pDispInfo->uFlags |= TTF_DI_SETITEM;
		}
	}
	return 0;
}

void MainFrame::autoConnect(const FavoriteHubEntry::List& fl) {
	for(FavoriteHubEntry::List::const_iterator i = fl.begin(); i != fl.end(); ++i) {
		FavoriteHubEntry* entry = *i;
		if(entry->getConnect())
			HubFrame::openWindow(m_hWndMDIClient, &ctrlTab, entry->getServer(), entry->getNick(), entry->getPassword(), entry->getUserDescription());
	}
}

void MainFrame::updateTray(bool add /* = true */) {
	if(add) {
		NOTIFYICONDATA nid;
		nid.cbSize = sizeof(NOTIFYICONDATA);
		nid.hWnd = m_hWnd;
		nid.uID = 0;
		nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
		nid.uCallbackMessage = WM_APP + 242;
		nid.hIcon = (HICON)::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_MAINFRAME), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
		strncpy(nid.szTip, "DC++",64);
		nid.szTip[63] = '\0';
		lastMove = GET_TICK() - 1000;
		::Shell_NotifyIcon(NIM_ADD, &nid);
		trayIcon = true;
	} else {
		NOTIFYICONDATA nid;
		nid.cbSize = sizeof(NOTIFYICONDATA);
		nid.hWnd = m_hWnd;
		nid.uID = 0;
		nid.uFlags = 0;
		::Shell_NotifyIcon(NIM_DELETE, &nid);
		ShowWindow(SW_SHOW);
		trayIcon = false;		
	}
}

/**
 * @todo Fix so that the away mode is not reset if it was set manually...
 */
LRESULT MainFrame::onSize(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
	if(wParam == SIZE_MINIMIZED) {
		if(BOOLSETTING(AUTO_AWAY)) {
			Util::setAway(true);
		}
		if(BOOLSETTING(MINIMIZE_TRAY)) {
			updateTray(true);
			ShowWindow(SW_HIDE);
		}
		maximized = IsZoomed() > 0;

	} else if( (wParam == SIZE_RESTORED || wParam == SIZE_MAXIMIZED) ) {
		if(BOOLSETTING(AUTO_AWAY)) {
			Util::setAway(false);
		}
		if(trayIcon) {
			updateTray(false);
		}
	}
	
	bHandled = FALSE;
	return 0;
}

LRESULT MainFrame::onEndSession(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	if(c != NULL) {
		c->removeListener(this);
		delete c;
		c = NULL;
	}

	WINDOWPLACEMENT wp;
	wp.length = sizeof(wp);
	GetWindowPlacement(&wp);
	
	CRect rc;
	GetWindowRect(rc);
	
	if(wp.showCmd == SW_SHOW || wp.showCmd == SW_SHOWNORMAL) {
		SettingsManager::getInstance()->set(SettingsManager::MAIN_WINDOW_POS_X, rc.left);
		SettingsManager::getInstance()->set(SettingsManager::MAIN_WINDOW_POS_Y, rc.top);
		SettingsManager::getInstance()->set(SettingsManager::MAIN_WINDOW_SIZE_X, rc.Width());
		SettingsManager::getInstance()->set(SettingsManager::MAIN_WINDOW_SIZE_Y, rc.Height());
	}
	if(wp.showCmd == SW_SHOWNORMAL || wp.showCmd == SW_SHOW || wp.showCmd == SW_SHOWMAXIMIZED || wp.showCmd == SW_MAXIMIZE)
		SettingsManager::getInstance()->set(SettingsManager::MAIN_WINDOW_STATE, (int)wp.showCmd);
	
	QueueManager::getInstance()->saveQueue();
	SettingsManager::getInstance()->save();
	
	return 0;
}

LRESULT MainFrame::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	if(c != NULL) {
		c->removeListener(this);
		delete c;
		c = NULL;
	}
	
	if(!closing) {
		if( oldshutdown ||(!BOOLSETTING(CONFIRM_EXIT)) || (MessageBox(CSTRING(REALLY_EXIT), APPNAME " " VERSIONSTRING, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDYES) ) {
			string tmp1;
			string tmp2;

			WINDOWPLACEMENT wp;
			wp.length = sizeof(wp);
			GetWindowPlacement(&wp);

			CRect rc;
			GetWindowRect(rc);
			if(wp.showCmd == SW_SHOW || wp.showCmd == SW_SHOWNORMAL) {
				SettingsManager::getInstance()->set(SettingsManager::MAIN_WINDOW_POS_X, rc.left);
				SettingsManager::getInstance()->set(SettingsManager::MAIN_WINDOW_POS_Y, rc.top);
				SettingsManager::getInstance()->set(SettingsManager::MAIN_WINDOW_SIZE_X, rc.Width());
				SettingsManager::getInstance()->set(SettingsManager::MAIN_WINDOW_SIZE_Y, rc.Height());
			}
			if(wp.showCmd == SW_SHOWNORMAL || wp.showCmd == SW_SHOW || wp.showCmd == SW_SHOWMAXIMIZED || wp.showCmd == SW_MAXIMIZE)
				SettingsManager::getInstance()->set(SettingsManager::MAIN_WINDOW_STATE, (int)wp.showCmd);

			ShowWindow(SW_HIDE);
			transferView.prepareClose();

			SearchManager::getInstance()->disconnect();
			ConnectionManager::getInstance()->disconnect();

			DWORD id;
			stopperThread = CreateThread(NULL, 0, stopper, this, 0, &id);
			closing = true;
		}
		bHandled = TRUE;
	} else {
		// This should end immideately, as it only should be the stopper that sends another WM_CLOSE
		WaitForSingleObject(stopperThread, 60*1000);
		CloseHandle(stopperThread);
		stopperThread = NULL;
		MDIDestroy(m_hWnd);
	}

	return 0;
}

LRESULT MainFrame::onLink(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {

	string site;

	switch(wID) {
	case IDC_HELP_README: site = Util::getAppPath() + "README.txt"; break;
	case IDC_HELP_CHANGELOG: site = Util::getAppPath() + "changelog.txt"; break;
	case IDC_HELP_HOMEPAGE: site = "http://dcplusplus.sourceforge.net"; break;
	case IDC_HELP_DOWNLOADS: site = "http://dcplusplus.sourceforge.net/index.php?page=download"; break;
	case IDC_HELP_FAQ: site = "http://dcplusplus.sourceforge.net/faq/faq.php?list=all&prog=1&lang=en"; break;
	case IDC_HELP_HELP_FORUM: site = "http://dcplusplus.sf.net/forum"; break;
	case IDC_HELP_DISCUSS: site = "http://dcplusplus.sf.net/forum"; break;
	case IDC_HELP_REQUEST_FEATURE: site = "http://sourceforge.net/tracker/?atid=427635&group_id=40287&func=browse"; break;
	case IDC_HELP_REPORT_BUG: site = "http://sourceforge.net/tracker/?atid=427632&group_id=40287&func=browse"; break;
	case IDC_HELP_DONATE: site = "https://www.paypal.com/xclick/business=j_s%40telia.com&item_name=DCPlusPlus&no_shipping=1&return=http%3A//dcplusplus.sf.net&cn=Greeting+%28and+forum+nick%3F%29&currency_code=EUR"; break;
	default: dcassert(0);
	}

	ShellExecute(NULL, NULL, site.c_str(), NULL, NULL, SW_SHOWNORMAL);

	return 0;
}

LRESULT MainFrame::onImport(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	string file = Util::getAppPath() + "queue.config";
 	if(WinUtil::browseFile(file, m_hWnd, false) == IDOK) {
		try {
			QueueManager::getInstance()->importNMQueue(file);
 		} catch(const FileException&) {
			ctrlStatus.SetText(0, CSTRING(ERROR_OPENING_FILE));
 		}
 	} 
 
 	return 0;
}
 
void MainFrame::UpdateLayout(BOOL bResizeBars /* = TRUE */)
{
	RECT rect;
	GetClientRect(&rect);
	// position bars and offset their dimensions
	UpdateBarsPosition(rect, bResizeBars);
	
	if(ctrlStatus.IsWindow()) {
		CRect sr;
		int w[8];
		ctrlStatus.GetClientRect(sr);
		w[7] = sr.right - 16;
#define setw(x) w[x] = max(w[x+1] - statusSizes[x], 0)
		setw(6); setw(5); setw(4); setw(3); setw(2); setw(1); setw(0);

		ctrlStatus.SetParts(8, w);
	}
	CRect rc = rect;
	rc.top = rc.bottom - ctrlTab.getHeight();
	if(ctrlTab.IsWindow())
		ctrlTab.MoveWindow(rc);
	
	CRect rc2 = rect;
	rc2.bottom = rc.top;
	SetSplitterRect(rc2);
}

static const char types[] = "File Lists\0*.DcLst;*.bz2\0All Files\0*.*\0";

LRESULT MainFrame::onOpenFileList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	string file;
	if(WinUtil::browseFile(file, m_hWnd, false, Util::getAppPath() + "FileLists\\", types)) {
		string username;
		if(file.rfind('\\') != string::npos) {
			username = file.substr(file.rfind('\\') + 1);
			if(username.rfind('.') != string::npos) {
				username.erase(username.rfind('.'));
			}
			DirectoryListingFrame* pChild = new DirectoryListingFrame(file, ClientManager::getInstance()->getUser(username));
			pChild->setTab(&ctrlTab);
			pChild->CreateEx(m_hWndClient);
			pChild->setWindowTitle();
		}
	}
	return 0;
}

LRESULT MainFrame::onTrayIcon(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	if (lParam == WM_LBUTTONUP) {
		ShowWindow(SW_SHOW);
		ShowWindow(maximized ? SW_MAXIMIZE : SW_RESTORE);
	} else if(lParam == WM_MOUSEMOVE && ((lastMove + 1000) < GET_TICK()) ) {
		NOTIFYICONDATA nid;
		nid.cbSize = sizeof(NOTIFYICONDATA);
		nid.hWnd = m_hWnd;
		nid.uID = 0;
		nid.uFlags = NIF_TIP;
		strncpy(nid.szTip, ("D: " + Util::formatBytes(DownloadManager::getInstance()->getAverageSpeed()) + "/s (" + 
			Util::toString(DownloadManager::getInstance()->getDownloads()) + ")\r\nU: " +
			Util::formatBytes(UploadManager::getInstance()->getAverageSpeed()) + "/s (" + 
			Util::toString(UploadManager::getInstance()->getUploads()) + ")").c_str(), 64);
		
		::Shell_NotifyIcon(NIM_MODIFY, &nid);
		lastMove = GET_TICK();
	}
	return 0;
}

LRESULT MainFrame::OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	static BOOL bVisible = TRUE;	// initially visible
	bVisible = !bVisible;
	CReBarCtrl rebar = m_hWndToolBar;
	int nBandIndex = rebar.IdToIndex(ATL_IDW_BAND_FIRST + 1);	// toolbar is 2nd added band
	rebar.ShowBand(nBandIndex, bVisible);
	UISetCheck(ID_VIEW_TOOLBAR, bVisible);
	UpdateLayout();
	return 0;
}

LRESULT MainFrame::onFinished(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
 	if(FinishedFrame::frame == NULL) {
 		FinishedFrame* pChild = new FinishedFrame();
 		pChild->setTab(&ctrlTab);
 		pChild->CreateEx(m_hWndClient);
 	} else {
 		MDIActivate(FinishedFrame::frame->m_hWnd);
	}
	return 0;
}

LRESULT MainFrame::onFinishedUploads(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
 	if(FinishedULFrame::frame == NULL) {
		FinishedULFrame* pChild = new FinishedULFrame();
 		pChild->setTab(&ctrlTab);
 		pChild->CreateEx(m_hWndClient);
 	} else {
 		MDIActivate(FinishedULFrame::frame->m_hWnd);
	}
	return 0;
}

LRESULT MainFrame::onCloseDisconnected(WORD , WORD , HWND , BOOL& ) {
	HubFrame::closeDisconnected();
	return 0;
}

void MainFrame::onAction(TimerManagerListener::Types type, u_int32_t aTick) throw() {
	if(type == TimerManagerListener::SECOND) {
		int64_t diff = (int64_t)((lastUpdate == 0) ? aTick - 1000 : aTick - lastUpdate);

		StringList* str = new StringList();
		str->push_back(Util::getAway() ? STRING(AWAY) : "");
		str->push_back("H: " + Client::getCounts());
		str->push_back(STRING(SLOTS) + ": " + Util::toString(SETTING(SLOTS) - UploadManager::getInstance()->getRunning()) + '/' + Util::toString(SETTING(SLOTS)));
		str->push_back("D: " + Util::formatBytes(Socket::getTotalDown()));
		str->push_back("U: " + Util::formatBytes(Socket::getTotalUp()));
		str->push_back("D: " + Util::formatBytes(Socket::getDown()*1000I64/diff) + "/s (" + Util::toString(DownloadManager::getInstance()->getDownloads()) + ")");
		str->push_back("U: " + Util::formatBytes(Socket::getUp()*1000I64/diff) + "/s (" + Util::toString(UploadManager::getInstance()->getUploads()) + ")");
		PostMessage(WM_SPEAKER, STATS, (LPARAM)str);
		SettingsManager::getInstance()->set(SettingsManager::TOTAL_UPLOAD, SETTING(TOTAL_UPLOAD) + Socket::getUp());
		SettingsManager::getInstance()->set(SettingsManager::TOTAL_DOWNLOAD, SETTING(TOTAL_DOWNLOAD) + Socket::getDown());
		lastUpdate = aTick;
		Socket::resetStats();
	}
}

// HttpConnectionListener
void MainFrame::onAction(HttpConnectionListener::Types type, HttpConnection* conn, string const& /*aLine*/) throw() {
	switch(type) {
		case HttpConnectionListener::COMPLETE: onHttpComplete(conn); break;
	}
}
void MainFrame::onAction(HttpConnectionListener::Types type, HttpConnection* /*conn*/, const BYTE* buf, int len) throw() {
	switch(type) {
		case HttpConnectionListener::DATA: versionInfo += string((const char*)buf, len); break;
	}
}

void MainFrame::onAction(QueueManagerListener::Types type, QueueItem* qi) throw() {
	if(type == QueueManagerListener::FINISHED) {
		if(qi->isSet(QueueItem::FLAG_CLIENT_VIEW)) {
			if(qi->isSet(QueueItem::FLAG_USER_LIST)) {
				// This is a file listing, show it...

				DirectoryListInfo* i = new DirectoryListInfo();
				if(qi->isSet(QueueItem::FLAG_BZLIST) ){
					dcassert(qi->getTarget().rfind('.') != string::npos);
					i->file = qi->getTarget().substr(0, qi->getTarget().rfind('.')+1) + "bz2";
				} else {
					i->file = qi->getTarget();
				}
				i->user = qi->getCurrent()->getUser();
				PostMessage(WM_SPEAKER, DOWNLOAD_LISTING, (LPARAM)i);
			} else if(qi->isSet(QueueItem::FLAG_TEXT)) {
				PostMessage(WM_SPEAKER, VIEW_TEXT, (LPARAM) new string(qi->getTarget()));
			}
		}
	}
}

/**
 * @file
 * $Id: MainFrm.cpp,v 1.30 2003/10/07 00:35:08 arnetheduck Exp $
 */


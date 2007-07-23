/*
 * Copyright (C) 2001-2007 Jacek Sieka, arnetheduck on gmail point com
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
#include <dcpp/DCPlusPlus.h>

#include "MainWindow.h"
#include "resource.h"

#include "SystemFrame.h"
#include "NotepadFrame.h"
#include "HubFrame.h"
#include "PublicHubsFrame.h"
#include "FavHubsFrame.h"
#include "QueueFrame.h"
#include "SearchFrame.h"
#include "ADLSearchFrame.h"
#include "SpyFrame.h"
#include "FinishedDLFrame.h"
#include "FinishedULFrame.h"
#include "LineDlg.h"
#include "HashProgressDlg.h"
#include "SettingsDialog.h"
#include "TextFrame.h"
#include "DirectoryListingFrame.h"
#include "PrivateFrame.h"
#include "StatsFrame.h"
#include "UsersFrame.h"
#include "WaitingUsersFrame.h"
#include "AboutDlg.h"

#include <dcpp/SettingsManager.h>
#include <dcpp/ResourceManager.h>
#include <dcpp/version.h>
#include <dcpp/DownloadManager.h>
#include <dcpp/UploadManager.h>
#include <dcpp/FavoriteManager.h>
#include <dcpp/Client.h>
#include <dcpp/TimerManager.h>
#include <dcpp/SearchManager.h>
#include <dcpp/ConnectionManager.h>
#include <dcpp/ShareManager.h>
#include <dcpp/QueueManager.h>

MainWindow* MainWindow::instance = 0;

MainWindow::MainWindow() :
	WidgetFactory<SmartWin::WidgetMDIFrame>(0),
	paned(0),
	transfers(0),
	tabs(0),
	trayIcon(false),
	maximized(false),
	lastMove(0),
	c(0),
	stopperThread(NULL),
	lastUp(0),
	lastDown(0),
	lastTick(GET_TICK())
{
	instance = this;
	
	links.homepage = _T("http://dcpp.net/");
	links.downloads = links.homepage + _T("download/");
	links.geoipfile = _T("http://www.maxmind.com/download/geoip/database/GeoIPCountryCSV.zip");
	links.translations = _T("http://sourceforge.net/tracker/?atid=460289&group_id=40287");
	links.faq = links.homepage + _T("faq/");
	links.help = links.homepage + _T("forum/");
	links.discuss = links.homepage + _T("forum/");
	links.features = links.homepage + _T("bugzilla/");
	links.bugs = links.homepage + _T("bugzilla/");

	initWindow();
	initMenu();
	initStatusBar();
	initTabs();
	initTransfers();
	initSecond();

	updateStatus();
	layout();
	
	onSized(std::tr1::bind(&MainWindow::sized, this, _1));
	onSpeaker(std::tr1::bind(&MainWindow::handleSpeaker, this, _1, _2));
	
	if(!WinUtil::isShift())
		speak(AUTO_CONNECT);
	
	QueueManager::getInstance()->addListener(this);
	LogManager::getInstance()->addListener(this);

	onClosing(std::tr1::bind(&MainWindow::closing, this));
	
	onRaw(std::tr1::bind(&MainWindow::trayMessage, this, _1, _2), SmartWin::Message(RegisterWindowMessage(_T("TaskbarCreated"))));
	onRaw(std::tr1::bind(&MainWindow::handleEndSession, this, _1, _2), SmartWin::Message(WM_ENDSESSION));
	
	TimerManager::getInstance()->start();

	c = new HttpConnection;
	c->addListener(this);
	c->downloadFile("http://dcplusplus.sourceforge.net/version.xml");

	File::ensureDirectory(SETTING(LOG_DIRECTORY));
	startSocket();
	
	SmartWin::Application::instance().setMDIClient(getMDIClient()->handle());

#ifdef PORT_ME
	// Load images
	// create command bar window
	HWND hWndCmdBar = m_CmdBar.Create(m_hWnd, rcDefault, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE);

	m_hMenu = WinUtil::mainMenu;

	// attach menu
	m_CmdBar.AttachMenu(m_hMenu);
	// load command bar images
	images.CreateFromImage(IDB_TOOLBAR, 16, 16, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION | LR_SHARED);
	m_CmdBar.m_hImageList = images;

	m_CmdBar.m_arrCommand.Add(ID_VIEW_CONNECT);
	m_CmdBar.m_arrCommand.Add(ID_FILE_RECONNECT);
	m_CmdBar.m_arrCommand.Add(IDC_FOLLOW);
	m_CmdBar.m_arrCommand.Add(IDC_FAVORITES);
	m_CmdBar.m_arrCommand.Add(IDC_FAVUSERS);
	m_CmdBar.m_arrCommand.Add(IDC_QUEUE);
	m_CmdBar.m_arrCommand.Add(IDC_FINISHED_DL);
	m_CmdBar.m_arrCommand.Add(IDC_VIEW_WAITING_USERS);
	m_CmdBar.m_arrCommand.Add(IDC_FINISHED_UL);
	m_CmdBar.m_arrCommand.Add(ID_FILE_SEARCH);
	m_CmdBar.m_arrCommand.Add(IDC_FILE_ADL_SEARCH);
	m_CmdBar.m_arrCommand.Add(IDC_SEARCH_SPY);
	m_CmdBar.m_arrCommand.Add(IDC_OPEN_FILE_LIST);
	m_CmdBar.m_arrCommand.Add(ID_FILE_SETTINGS);
	m_CmdBar.m_arrCommand.Add(IDC_NOTEPAD);
	m_CmdBar.m_arrCommand.Add(IDC_NET_STATS);
	m_CmdBar.m_arrCommand.Add(ID_WINDOW_CASCADE);
	m_CmdBar.m_arrCommand.Add(ID_WINDOW_TILE_HORZ);
	m_CmdBar.m_arrCommand.Add(ID_WINDOW_TILE_VERT);
	m_CmdBar.m_arrCommand.Add(ID_WINDOW_MINIMIZE_ALL);
	m_CmdBar.m_arrCommand.Add(ID_WINDOW_RESTORE_ALL);

	// remove old menu
	SetMenu(NULL);

	HWND hWndToolBar = createToolbar();

	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
	AddSimpleReBarBand(hWndCmdBar);
	AddSimpleReBarBand(hWndToolBar, NULL, TRUE);
	CreateSimpleStatusBar();

	CToolInfo ti(TTF_SUBCLASS, ctrlStatus.m_hWnd);

	ctrlLastLines.Create(ctrlStatus.m_hWnd, rcDefault, NULL, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP, WS_EX_TOPMOST);
	ctrlLastLines.SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	ctrlLastLines.AddTool(&ti);

	WinUtil::mdiClient = m_hWndMDIClient;

	ctrlTab.Create(m_hWnd, rcDefault);
	WinUtil::tabCtrl = &ctrlTab;

	transferView.Create(m_hWnd);

	UIAddToolBar(hWndToolBar);
	UISetCheck(ID_VIEW_TOOLBAR, 1);
	UISetCheck(ID_VIEW_STATUS_BAR, 1);
	UISetCheck(ID_VIEW_TRANSFER_VIEW, 1);

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);


	if(BOOLSETTING(OPEN_SYSTEM_LOG)) PostMessage(WM_COMMAND, IDC_SYSTEM_LOG);
	if(BOOLSETTING(OPEN_PUBLIC)) PostMessage(WM_COMMAND, ID_VIEW_CONNECT);
	if(BOOLSETTING(OPEN_FAVORITE_HUBS)) PostMessage(WM_COMMAND, IDC_FAVORITES);
	if(BOOLSETTING(OPEN_FAVORITE_USERS)) PostMessage(WM_COMMAND, IDC_FAVUSERS);
	if(BOOLSETTING(OPEN_QUEUE)) PostMessage(WM_COMMAND, IDC_QUEUE);
	if(BOOLSETTING(OPEN_FINISHED_DOWNLOADS)) PostMessage(WM_COMMAND, IDC_FINISHED_DL);
	if(BOOLSETTING(OPEN_WAITING_USERS)) PostMessage(WM_COMMAND, IDC_VIEW_WAITING_USERS);
	if(BOOLSETTING(OPEN_FINISHED_UPLOADS)) PostMessage(WM_COMMAND, IDC_FINISHED_UL);
	if(BOOLSETTING(OPEN_SEARCH_SPY)) PostMessage(WM_COMMAND, IDC_SEARCH_SPY);
	if(BOOLSETTING(OPEN_NETWORK_STATISTICS)) PostMessage(WM_COMMAND, IDC_NET_STATS);
	if(BOOLSETTING(OPEN_NOTEPAD)) PostMessage(WM_COMMAND, IDC_NOTEPAD);

	if(!BOOLSETTING(SHOW_STATUSBAR)) PostMessage(WM_COMMAND, ID_VIEW_STATUS_BAR);
	if(!BOOLSETTING(SHOW_TOOLBAR)) PostMessage(WM_COMMAND, ID_VIEW_TOOLBAR);
	if(!BOOLSETTING(SHOW_TRANSFERVIEW)) PostMessage(WM_COMMAND, ID_VIEW_TRANSFER_VIEW);

	PostMessage(WM_SPEAKER, PARSE_COMMAND_LINE);

	if(SETTING(NICK).empty()) {
		HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_GENERALPAGE);
		PostMessage(WM_COMMAND, ID_FILE_SETTINGS);
	}

#endif
}

void MainWindow::initWindow() {
	// Create main window
	dcdebug("initWindow\n");
	Seed cs;

	int pos_x = SETTING(MAIN_WINDOW_POS_X);
	int pos_y = SETTING(MAIN_WINDOW_POS_Y);
	int size_x = SETTING(MAIN_WINDOW_SIZE_X);
	int size_y = SETTING(MAIN_WINDOW_SIZE_Y);
	
	if( (pos_x != static_cast<int>(CW_USEDEFAULT)) &&
		(pos_y != static_cast<int>(CW_USEDEFAULT)) &&
		(size_x != static_cast<int>(CW_USEDEFAULT)) &&
		(size_y != static_cast<int>(CW_USEDEFAULT)) &&
		(pos_x > 0 && pos_y > 0) &&
		(size_x > 10 && size_y > 10)
		)
	{
		cs.location = SmartWin::Rectangle(pos_x, pos_y, size_x, size_y);
	}
	
	cs.style |= WS_CLIPCHILDREN;
	cs.exStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	if(ResourceManager::getInstance()->isRTL())
		cs.exStyle |= WS_EX_RTLREADING; 

#ifdef PORT_ME
	wndMain.ShowWindow(((nCmdShow == SW_SHOWDEFAULT) || (nCmdShow == SW_SHOWNORMAL)) ? SETTING(MAIN_WINDOW_STATE) : nCmdShow);
#endif
	// Set window name
	cs.caption = _T(APPNAME) _T(" ") _T(VERSIONSTRING);

	createWindow(cs);
	
	paned = createHPaned();
	paned->setRelativePos(0.7);
}

void MainWindow::initMenu() {
	dcdebug("initMenu\n");
	mainMenu = createMenu(false);
	
	WidgetMenuPtr file = mainMenu->appendPopup(CTSTRING(MENU_FILE));
	
	file->appendItem(IDC_QUICK_CONNECT, TSTRING(MENU_QUICK_CONNECT), std::tr1::bind(&MainWindow::handleQuickConnect, this));
#ifdef PORT_ME
	file->appendItem(IDC_FOLLOW, TSTRING(MENU_FOLLOW_REDIRECT));
	file->appendItem(IDC_RECONNECT, TSTRING(MENU_RECONNECT));
#endif
	file->appendSeparatorItem();
	
	file->appendItem(IDC_OPEN_FILE_LIST, TSTRING(MENU_OPEN_FILE_LIST), std::tr1::bind(&MainWindow::handleOpenFileList, this));
	file->appendItem(IDC_OPEN_OWN_LIST, TSTRING(MENU_OPEN_OWN_LIST), std::tr1::bind(&MainWindow::handleOpenOwnList, this));
	file->appendItem(IDC_MATCH_ALL, TSTRING(MENU_OPEN_MATCH_ALL), std::tr1::bind(&MainWindow::handleMatchAll, this));
	file->appendItem(IDC_REFRESH_FILE_LIST, TSTRING(MENU_REFRESH_FILE_LIST), std::tr1::bind(&MainWindow::handleRefreshFileList, this));
	file->appendItem(IDC_OPEN_DOWNLOADS, TSTRING(MENU_OPEN_DOWNLOADS_DIR), std::tr1::bind(&MainWindow::handleOpenDownloadsDir, this));
	file->appendSeparatorItem();

	file->appendItem(IDC_SETTINGS, TSTRING(MENU_SETTINGS), std::tr1::bind(&MainWindow::handleSettings, this));
	file->appendSeparatorItem();
	file->appendItem(IDC_EXIT, TSTRING(MENU_EXIT), std::tr1::bind(&MainWindow::handleExit, this));

	WidgetMenuPtr view = mainMenu->appendPopup(CTSTRING(MENU_VIEW));

	view->appendItem(IDC_PUBLIC_HUBS, TSTRING(MENU_PUBLIC_HUBS), std::tr1::bind(&MainWindow::handleOpenWindow, this, _1));
	view->appendItem(IDC_FAVORITE_HUBS, TSTRING(MENU_FAVORITE_HUBS), std::tr1::bind(&MainWindow::handleOpenWindow, this, _1));
	view->appendItem(IDC_FAVUSERS, TSTRING(MENU_FAVORITE_USERS), std::tr1::bind(&MainWindow::handleOpenWindow, this, _1));
	view->appendSeparatorItem();
	view->appendItem(IDC_QUEUE, TSTRING(MENU_DOWNLOAD_QUEUE), std::tr1::bind(&MainWindow::handleOpenWindow, this, _1));
	view->appendItem(IDC_FINISHED_DL, TSTRING(FINISHED_DOWNLOADS), std::tr1::bind(&MainWindow::handleOpenWindow, this, _1));
	view->appendItem(IDC_WAITING_USERS, TSTRING(WAITING_USERS), std::tr1::bind(&MainWindow::handleOpenWindow, this, _1));
	view->appendItem(IDC_FINISHED_UL, TSTRING(FINISHED_UPLOADS), std::tr1::bind(&MainWindow::handleOpenWindow, this, _1));
	view->appendSeparatorItem();
	view->appendItem(IDC_SEARCH, TSTRING(MENU_SEARCH), std::tr1::bind(&MainWindow::handleOpenWindow, this, _1));
	view->appendItem(IDC_ADL_SEARCH, TSTRING(MENU_ADL_SEARCH), std::tr1::bind(&MainWindow::handleOpenWindow, this, _1));
	view->appendItem(IDC_SEARCH_SPY, TSTRING(MENU_SEARCH_SPY), std::tr1::bind(&MainWindow::handleOpenWindow, this, _1));
	view->appendSeparatorItem();
	view->appendItem(IDC_NOTEPAD, TSTRING(MENU_NOTEPAD), std::tr1::bind(&MainWindow::handleOpenWindow, this, _1));
	view->appendItem(IDC_SYSTEM_LOG, TSTRING(MENU_SYSTEM_LOG), std::tr1::bind(&MainWindow::handleOpenWindow, this, _1));
	view->appendItem(IDC_NET_STATS, TSTRING(MENU_NETWORK_STATISTICS), std::tr1::bind(&MainWindow::handleOpenWindow, this, _1));
	view->appendItem(IDC_HASH_PROGRESS, TSTRING(MENU_HASH_PROGRESS), std::tr1::bind(&MainWindow::handleHashProgress, this));
#ifdef PORT_ME
	view.AppendMenu(MF_SEPARATOR);
	view.AppendMenu(MF_STRING, ID_VIEW_TOOLBAR, CTSTRING(MENU_TOOLBAR));
	view.AppendMenu(MF_STRING, ID_VIEW_STATUS_BAR, CTSTRING(MENU_STATUS_BAR));
	view.AppendMenu(MF_STRING, ID_VIEW_TRANSFER_VIEW, CTSTRING(MENU_TRANSFER_VIEW));

#endif

	WidgetMenuPtr window = mainMenu->appendPopup(CTSTRING(MENU_WINDOW));
	
	window->appendItem(IDC_MDI_CASCADE, TSTRING(MENU_CASCADE), std::tr1::bind(&MainWindow::handleMDIReorder, this, _1));
	window->appendItem(IDC_MDI_TILE_HORZ, TSTRING(MENU_HORIZONTAL_TILE), std::tr1::bind(&MainWindow::handleMDIReorder, this, _1));
	window->appendItem(IDC_MDI_TILE_VERT, TSTRING(MENU_VERTICAL_TILE), std::tr1::bind(&MainWindow::handleMDIReorder, this, _1));
	window->appendItem(IDC_MDI_ARRANGE, TSTRING(MENU_ARRANGE), std::tr1::bind(&MainWindow::handleMDIReorder, this, _1));
	window->appendItem(IDC_MDI_MINIMIZE_ALL, TSTRING(MENU_MINIMIZE_ALL), std::tr1::bind(&MainWindow::handleMinimizeAll, this));
	window->appendItem(IDC_MDI_RESTORE_ALL, TSTRING(MENU_RESTORE_ALL), std::tr1::bind(&MainWindow::handleRestoreAll, this));
	window->appendSeparatorItem();
	window->appendItem(IDC_CLOSE_ALL_DISCONNECTED, TSTRING(MENU_CLOSE_DISCONNECTED), std::tr1::bind(&MainWindow::handleCloseWindows, this, _1));
	window->appendItem(IDC_CLOSE_ALL_PM, TSTRING(MENU_CLOSE_ALL_PM), std::tr1::bind(&MainWindow::handleCloseWindows, this, _1));
	window->appendItem(IDC_CLOSE_ALL_OFFLINE_PM, TSTRING(MENU_CLOSE_ALL_OFFLINE_PM), std::tr1::bind(&MainWindow::handleCloseWindows, this, _1));
	window->appendItem(IDC_CLOSE_ALL_DIR_LIST, TSTRING(MENU_CLOSE_ALL_DIR_LIST), std::tr1::bind(&MainWindow::handleCloseWindows, this, _1));
	window->appendItem(IDC_CLOSE_ALL_SEARCH_FRAME, TSTRING(MENU_CLOSE_ALL_SEARCHFRAME), std::tr1::bind(&MainWindow::handleCloseWindows, this, _1));

	WidgetMenuPtr help = mainMenu->appendPopup(TSTRING(MENU_HELP));

	help->appendItem(IDC_HELP_CONTENTS, TSTRING(MENU_CONTENTS), std::tr1::bind(&MainWindow::handleHelp, this, _1));
	help->appendSeparatorItem();
	help->appendItem(IDC_HELP_CHANGELOG, TSTRING(MENU_CHANGELOG), std::tr1::bind(&MainWindow::handleHelp, this, _1));
	help->appendItem(IDC_ABOUT, TSTRING(MENU_ABOUT), std::tr1::bind(&MainWindow::handleAbout, this));
	help->appendSeparatorItem();
	help->appendItem(IDC_HELP_HOMEPAGE, TSTRING(MENU_HOMEPAGE), std::tr1::bind(&MainWindow::handleLink, this, _1));
	help->appendItem(IDC_HELP_DOWNLOADS, TSTRING(MENU_HELP_DOWNLOADS), std::tr1::bind(&MainWindow::handleLink, this, _1));
	help->appendItem(IDC_HELP_GEOIPFILE, TSTRING(MENU_HELP_GEOIPFILE), std::tr1::bind(&MainWindow::handleLink, this, _1));
	help->appendItem(IDC_HELP_TRANSLATIONS, TSTRING(MENU_HELP_TRANSLATIONS), std::tr1::bind(&MainWindow::handleLink, this, _1));
	help->appendItem(IDC_HELP_FAQ, TSTRING(MENU_FAQ), std::tr1::bind(&MainWindow::handleLink, this, _1));
	help->appendItem(IDC_HELP_FORUM, TSTRING(MENU_HELP_FORUM), std::tr1::bind(&MainWindow::handleLink, this, _1));
	help->appendItem(IDC_HELP_DISCUSS, TSTRING(MENU_DISCUSS), std::tr1::bind(&MainWindow::handleLink, this, _1));
	help->appendItem(IDC_HELP_REQUEST_FEATURE, TSTRING(MENU_REQUEST_FEATURE), std::tr1::bind(&MainWindow::handleLink, this, _1));
	help->appendItem(IDC_HELP_REPORT_BUG, TSTRING(MENU_REPORT_BUG), std::tr1::bind(&MainWindow::handleLink, this, _1));
	help->appendItem(IDC_HELP_DONATE, TSTRING(MENU_DONATE), std::tr1::bind(&MainWindow::handleLink, this, _1));

	mainMenu->attach(this);	
}

void MainWindow::initStatusBar() {
	dcdebug("initStatusBar\n");
	initStatus();
	statusSizes[STATUS_AWAY] = status->getTextSize(TSTRING(AWAY)).x + 12;
	///@todo set to checkbox width + resizedrag width really
	statusSizes[STATUS_DUMMY] = 32;
}

void MainWindow::initTabs() {
	MDITab::Seed cs;
	cs.style = WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE | TCS_FOCUSNEVER | TCS_MULTILINE | TCS_BUTTONS | TCS_FLATBUTTONS | TCS_HOTTRACK;
	cs.font = WinUtil::font;
	tabs = SmartWin::WidgetCreator<MDITab>::create(this, cs);
	tabs->setFlatSeparators(false);
	tabs->onResized(std::tr1::bind(&MainWindow::speak, this, LAYOUT, 0));
	paned->setFirst(tabs);
}

void MainWindow::initTransfers() {
	dcdebug("initTransfers\n");
	transfers = new TransferView(this);
	paned->setSecond(transfers);
}

void MainWindow::handleExit() {
	close(true);
}

void MainWindow::handleQuickConnect() {
	///@todo send user to settings
	if(SETTING(NICK).empty())
		return;

	LineDlg dlg(this, TSTRING(QUICK_CONNECT), TSTRING(HUB_ADDRESS));
	
	if(dlg.run() == IDOK) {

		tstring tmp = dlg.getLine();
		// Strip out all the spaces
		string::size_type i;
		while((i = tmp.find(' ')) != string::npos)
			tmp.erase(i, 1);

		HubFrame::openWindow(getMDIClient(), tmp);
	}
}
 
void MainWindow::sized(const SmartWin::WidgetSizedEventResult& sz) {
	layout();
}

HRESULT MainWindow::handleSpeaker(WPARAM wParam, LPARAM lParam) {
	Speaker s = static_cast<Speaker>(wParam);

	switch(s) {
	case DOWNLOAD_LISTING: {
		boost::scoped_ptr<DirectoryListInfo> i(reinterpret_cast<DirectoryListInfo*>(lParam));
		DirectoryListingFrame::openWindow(getMDIClient(), i->file, i->dir, i->user, i->speed);
	} break;
	case BROWSE_LISTING: {
		boost::scoped_ptr<DirectoryBrowseInfo> i(reinterpret_cast<DirectoryBrowseInfo*>(lParam));
		DirectoryListingFrame::openWindow(getMDIClient(), i->user, i->text, 0);
	} break;
	case AUTO_CONNECT: {
		autoConnect(FavoriteManager::getInstance()->getFavoriteHubs());			
	} break;
	case PARSE_COMMAND_LINE: {
#ifdef PORT_ME
		parseCommandLine(GetCommandLine());
#endif
	} break;
	case VIEW_FILE_AND_DELETE: {
		boost::scoped_ptr<tstring> file(reinterpret_cast<tstring*>(lParam));
		new TextFrame(this->getMDIClient(), *file);
		File::deleteFile(Text::fromT(*file));
	} break;
	case STATUS_MESSAGE: {
		boost::scoped_ptr<pair<time_t, tstring> > msg(reinterpret_cast<std::pair<time_t, tstring>*>(lParam));
		tstring line = Text::toT("[" + Util::getShortTimeString(msg->first) + "] ") + msg->second;

		setStatus(STATUS_STATUS, line);
		while(lastLinesList.size() + 1 > MAX_CLIENT_LINES)
			lastLinesList.erase(lastLinesList.begin());
		if (line.find(_T('\r')) == tstring::npos) {
			lastLinesList.push_back(line);
		} else {
			lastLinesList.push_back(line.substr(0, line.find(_T('\r'))));
		}
	} break;
	case LAYOUT: {
		layout();
	} break;
	}	
	return 0;
}

void MainWindow::autoConnect(const FavoriteHubEntryList& fl) {
	for(FavoriteHubEntryList::const_iterator i = fl.begin(); i != fl.end(); ++i) {
		FavoriteHubEntry* entry = *i;
		if(entry->getConnect()) {
			if(!entry->getNick().empty() || !SETTING(NICK).empty()) {
				HubFrame::openWindow(getMDIClient(), entry->getServer());
			}
		}
	}
}

void MainWindow::saveWindowSettings() {
	WINDOWPLACEMENT wp = { sizeof(wp) };
	
	::GetWindowPlacement(this->handle(), &wp);

	if(wp.showCmd == SW_SHOW || wp.showCmd == SW_SHOWNORMAL) {
		SettingsManager::getInstance()->set(SettingsManager::MAIN_WINDOW_POS_X, static_cast<int>(wp.rcNormalPosition.left));
		SettingsManager::getInstance()->set(SettingsManager::MAIN_WINDOW_POS_Y, static_cast<int>(wp.rcNormalPosition.top));
		SettingsManager::getInstance()->set(SettingsManager::MAIN_WINDOW_SIZE_X, static_cast<int>(wp.rcNormalPosition.right - wp.rcNormalPosition.left));
		SettingsManager::getInstance()->set(SettingsManager::MAIN_WINDOW_SIZE_Y, static_cast<int>(wp.rcNormalPosition.bottom - wp.rcNormalPosition.top));
	}
	if(wp.showCmd == SW_SHOWNORMAL || wp.showCmd == SW_SHOW || wp.showCmd == SW_SHOWMAXIMIZED || wp.showCmd == SW_MAXIMIZE)
		SettingsManager::getInstance()->set(SettingsManager::MAIN_WINDOW_STATE, (int)wp.showCmd);
}

bool MainWindow::closing() {
	if(stopperThread == NULL) {
		if( !BOOLSETTING(CONFIRM_EXIT) || (createMessageBox().show(TSTRING(REALLY_EXIT), _T(APPNAME) _T(" ") _T(VERSIONSTRING), WidgetMessageBox::BOX_YESNO, WidgetMessageBox::BOX_ICONQUESTION) == IDYES) ) {
			if(c != NULL) {
				c->removeListener(this);
				delete c;
				c = NULL;
			}
			saveWindowSettings();

			::ShowWindow(this->handle(), SW_HIDE);
			transfers->prepareClose();

			LogManager::getInstance()->removeListener(this);
			QueueManager::getInstance()->removeListener(this);

			if(trayIcon) {
				updateTray(false);
			}
			SearchManager::getInstance()->disconnect();
			ConnectionManager::getInstance()->disconnect();

			stopUPnP();

			DWORD id;
			stopperThread = CreateThread(NULL, 0, stopper, this, 0, &id);
		}
		return false;
	} else {
		// This should end immediately, as it only should be the stopper that sends another WM_CLOSE
		WaitForSingleObject(stopperThread, 60*1000);
		CloseHandle(stopperThread);
		stopperThread = NULL;
		::PostQuitMessage(0);
	}
	return true;
}

HRESULT MainWindow::trayMessage(WPARAM wParam, LPARAM lParam) {
	updateTray(true);
	return 0;
}

void MainWindow::handleMDIReorder(unsigned id) {
	switch(id) {
	case IDC_MDI_CASCADE: getMDIClient()->cascade(); break;
	case IDC_MDI_TILE_VERT: getMDIClient()->tile(false); break;
	case IDC_MDI_TILE_HORZ: getMDIClient()->tile(true); break;
	case IDC_MDI_ARRANGE: getMDIClient()->arrange(); break;
	}
}

void MainWindow::initSecond() {
	createTimer(std::tr1::bind(&MainWindow::eachSecond, this), 1000);
}

bool MainWindow::eachSecond() {
	updateStatus();
	return true;
}

void MainWindow::layout() {
	const int border = 2;
	SmartWin::Rectangle r(getClientAreaSize()); 

	SmartWin::Rectangle rs = layoutStatus();
	
	r.size.y -= rs.size.y + border;

	paned->setRect(r);
	
	getMDIClient()->setBounds(tabs->getUsableArea());
}

void MainWindow::updateStatus() {
	uint64_t now = GET_TICK();
	uint64_t tdiff = lastTick - now;
	if(tdiff < 100) {
		tdiff = 1;
	}
	
	uint64_t up = Socket::getTotalUp();
	uint64_t down = Socket::getTotalDown();
	uint64_t updiff = up - lastUp;
	uint64_t downdiff = down - lastDown;
	
	lastTick = now;
	lastUp = up;
	lastDown = down;

	/** @todo move this to client/ */
	SettingsManager::getInstance()->set(SettingsManager::TOTAL_UPLOAD, SETTING(TOTAL_UPLOAD) + static_cast<int64_t>(updiff));
	SettingsManager::getInstance()->set(SettingsManager::TOTAL_DOWNLOAD, SETTING(TOTAL_DOWNLOAD) + static_cast<int64_t>(downdiff));

	setStatus(STATUS_AWAY, Util::getAway() ? TSTRING(AWAY) : _T(""));
	setStatus(STATUS_COUNTS, Client::getCounts());
	setStatus(STATUS_SLOTS, Text::toT(STRING(SLOTS) + ": " + Util::toString(UploadManager::getInstance()->getFreeSlots()) + '/' + Util::toString(SETTING(SLOTS))));
	setStatus(STATUS_DOWN_TOTAL, Text::toT("D: " + Util::formatBytes(Socket::getTotalDown())));
	setStatus(STATUS_UP_TOTAL, Text::toT("U: " + Util::formatBytes(Socket::getTotalUp())));
	setStatus(STATUS_DOWN_DIFF, Text::toT("D: " + Util::formatBytes(downdiff*1000/tdiff) + "/s (" + Util::toString(DownloadManager::getInstance()->getDownloadCount()) + ")"));
	setStatus(STATUS_UP_DIFF, Text::toT("U: " + Util::formatBytes(updiff*1000/tdiff) + "/s (" + Util::toString(UploadManager::getInstance()->getUploadCount()) + ")"));
}

MainWindow::~MainWindow() {
	instance = 0;
	
#ifdef PORT_ME
	m_CmdBar.m_hImageList = NULL;

	images.Destroy();
	largeImages.Destroy();
	largeImagesHot.Destroy();
#endif
}

void MainWindow::handleSettings() {
	
	SettingsDialog dlg(this);

	unsigned short lastTCP = static_cast<unsigned short>(SETTING(TCP_PORT));
	unsigned short lastUDP = static_cast<unsigned short>(SETTING(UDP_PORT));
	unsigned short lastTLS = static_cast<unsigned short>(SETTING(TLS_PORT));

	int lastConn = SETTING(INCOMING_CONNECTIONS);

	bool lastSortFavUsersFirst = BOOLSETTING(SORT_FAVUSERS_FIRST);

	if(dlg.run() == IDOK) {
		SettingsManager::getInstance()->save();
#ifdef PORT_ME
		if(missedAutoConnect && !SETTING(NICK).empty()) {
			PostMessage(WM_SPEAKER, AUTO_CONNECT);
		}
#endif
		if(SETTING(INCOMING_CONNECTIONS) != lastConn || SETTING(TCP_PORT) != lastTCP || SETTING(UDP_PORT) != lastUDP || SETTING(TLS_PORT) != lastTLS) {
			startSocket();
		}
		ClientManager::getInstance()->infoUpdated();

#ifdef PORT_ME
		if(BOOLSETTING(SORT_FAVUSERS_FIRST) != lastSortFavUsersFirst)
			HubFrame::resortUsers();

		if(BOOLSETTING(URL_HANDLER)) {
			WinUtil::registerDchubHandler();
			WinUtil::registerADChubHandler();
			WinUtil::urlDcADCRegistered = true;
		} else if(WinUtil::urlDcADCRegistered) {
			WinUtil::unRegisterDchubHandler();
			WinUtil::unRegisterADChubHandler();
			WinUtil::urlDcADCRegistered = false;
		}
		if(BOOLSETTING(MAGNET_REGISTER)) {
			WinUtil::registerMagnetHandler();
			WinUtil::urlMagnetRegistered = true;
		} else if(WinUtil::urlMagnetRegistered) {
			WinUtil::unRegisterMagnetHandler();
			WinUtil::urlMagnetRegistered = false;
		}
#endif
	}
}

void MainWindow::startSocket() {
	SearchManager::getInstance()->disconnect();
	ConnectionManager::getInstance()->disconnect();

	if(ClientManager::getInstance()->isActive()) {
		try {
			ConnectionManager::getInstance()->listen();
		} catch(const Exception&) {
			WidgetMessageBox().show(TSTRING(TCP_PORT_BUSY), _T(APPNAME) _T(" ") _T(VERSIONSTRING), WidgetMessageBox::BOX_OK, WidgetMessageBox::BOX_ICONSTOP);
		}
		try {
			SearchManager::getInstance()->listen();
		} catch(const Exception&) {
			WidgetMessageBox().show(CTSTRING(TCP_PORT_BUSY), _T(APPNAME) _T(" ") _T(VERSIONSTRING), WidgetMessageBox::BOX_OK, WidgetMessageBox::BOX_ICONSTOP);
		}
	}

	startUPnP();
}

void MainWindow::startUPnP() {
#ifdef PORT_ME
	stopUPnP();

	if( SETTING(INCOMING_CONNECTIONS) == SettingsManager::INCOMING_FIREWALL_UPNP ) {
		UPnP_TCPConnection = new UPnP( Util::getLocalIp(), "TCP", APPNAME " Download Port (" + Util::toString(ConnectionManager::getInstance()->getPort()) + " TCP)", ConnectionManager::getInstance()->getPort() );
		UPnP_UDPConnection = new UPnP( Util::getLocalIp(), "UDP", APPNAME " Search Port (" + Util::toString(SearchManager::getInstance()->getPort()) + " UDP)", SearchManager::getInstance()->getPort() );

		if ( FAILED(UPnP_UDPConnection->OpenPorts()) || FAILED(UPnP_TCPConnection->OpenPorts()) )
		{
			LogManager::getInstance()->message(STRING(UPNP_FAILED_TO_CREATE_MAPPINGS));
			MessageBox(CTSTRING(UPNP_FAILED_TO_CREATE_MAPPINGS), _T(APPNAME) _T(" ") _T(VERSIONSTRING), MB_OK | MB_ICONWARNING);

			// We failed! thus reset the objects
			delete UPnP_TCPConnection;
			delete UPnP_UDPConnection;
			UPnP_TCPConnection = UPnP_UDPConnection = NULL;
		}
		else
		{
			if(!BOOLSETTING(NO_IP_OVERRIDE)) {
				// now lets configure the external IP (connect to me) address
				string ExternalIP = UPnP_TCPConnection->GetExternalIP();
				if ( !ExternalIP.empty() ) {
					// woohoo, we got the external IP from the UPnP framework
					SettingsManager::getInstance()->set(SettingsManager::EXTERNAL_IP, ExternalIP );
				} else {
					//:-( Looks like we have to rely on the user setting the external IP manually
					// no need to do cleanup here because the mappings work
					LogManager::getInstance()->message(STRING(UPNP_FAILED_TO_GET_EXTERNAL_IP));
					MessageBox(CTSTRING(UPNP_FAILED_TO_GET_EXTERNAL_IP), _T(APPNAME) _T(" ") _T(VERSIONSTRING), MB_OK | MB_ICONWARNING);
				}
			}
		}
	}
#endif
}

void MainWindow::stopUPnP() {
#ifdef PORT_ME
	// Just check if the port mapping objects are initialized (NOT NULL)
	if ( UPnP_TCPConnection != NULL )
	{
		if (FAILED(UPnP_TCPConnection->ClosePorts()) )
		{
			LogManager::getInstance()->message(STRING(UPNP_FAILED_TO_REMOVE_MAPPINGS));
		}
		delete UPnP_TCPConnection;
	}
	if ( UPnP_UDPConnection != NULL )
	{
		if (FAILED(UPnP_UDPConnection->ClosePorts()) )
		{
			LogManager::getInstance()->message(STRING(UPNP_FAILED_TO_REMOVE_MAPPINGS));
		}
		delete UPnP_UDPConnection;
	}
	// Not sure this is required (i.e. Objects are checked later in execution)
	// But its better being on the save side :P
	UPnP_TCPConnection = UPnP_UDPConnection = NULL;
#endif
}

static const TCHAR types[] = _T("File Lists\0*.DcLst;*.xml.bz2\0All Files\0*.*\0");

void MainWindow::handleOpenFileList() {
	tstring file;
	if(WinUtil::browseFile(file, handle(), false, Text::toT(Util::getListPath()), types)) {
		User::Ptr u = DirectoryListing::getUserFromFilename(Text::fromT(file));
		if(u) {
			DirectoryListingFrame::openWindow(getMDIClient(), file, Text::toT(Util::emptyString), u, 0);
		} else {
#ifdef PORT_ME
			MessageBox(CTSTRING(INVALID_LISTNAME), _T(APPNAME) _T(" ") _T(VERSIONSTRING));
#endif
		}
	}
}

void MainWindow::handleOpenOwnList() {
	if(!ShareManager::getInstance()->getOwnListFile().empty()){
		DirectoryListingFrame::openWindow(getMDIClient(), Text::toT(ShareManager::getInstance()->getOwnListFile()), Text::toT(Util::emptyString), ClientManager::getInstance()->getMe(), 0);
	}
}

void MainWindow::updateTray(bool add /* = true */) {
	if(add) {
		NOTIFYICONDATA nid;
		nid.cbSize = sizeof(NOTIFYICONDATA);
		nid.hWnd = handle();
		nid.uID = 0;
		nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
		nid.uCallbackMessage = WM_APP + 242;
		nid.hIcon = (HICON)::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_MAINFRAME), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
		_tcscpy(nid.szTip, _T("DC++"));
		nid.szTip[63] = '\0';
		lastMove = GET_TICK() - 1000;
		::Shell_NotifyIcon(NIM_ADD, &nid);
		trayIcon = true;
	} else {
		NOTIFYICONDATA nid;
		nid.cbSize = sizeof(NOTIFYICONDATA);
		nid.hWnd = handle();
		nid.uID = 0;
		nid.uFlags = 0;
		::Shell_NotifyIcon(NIM_DELETE, &nid);
		::ShowWindow(handle(), SW_SHOW);
		trayIcon = false;
	}
}

DWORD WINAPI MainWindow::stopper(void* p) {
	MainWindow* mf = reinterpret_cast<MainWindow*>(p);
	HWND wnd, wnd2 = NULL;

	while( (wnd=::GetWindow(mf->getMDIClient()->handle(), GW_CHILD)) != NULL) {
		if(wnd == wnd2)
			Sleep(100);
		else {
			::PostMessage(wnd, WM_CLOSE, 0, 0);
			wnd2 = wnd;
		}
	}

	::PostMessage(mf->handle(), WM_CLOSE, 0, 0);
	return 0;
}

class ListMatcher : public Thread {
public:
	ListMatcher(StringList files_) : files(files_) {

	}
	virtual int run() {
		for(StringIter i = files.begin(); i != files.end(); ++i) {
			User::Ptr u = DirectoryListing::getUserFromFilename(*i);
			if(!u)
				continue;
			DirectoryListing dl(u);
			try {
				dl.loadFile(*i);
				const size_t BUF_SIZE = STRING(MATCHED_FILES).size() + 16;
				AutoArray<char> tmp(BUF_SIZE);
				snprintf(tmp, BUF_SIZE, CSTRING(MATCHED_FILES), QueueManager::getInstance()->matchListing(dl));
				LogManager::getInstance()->message(Util::toString(ClientManager::getInstance()->getNicks(u->getCID())) + ": " + string(tmp));
			} catch(const Exception&) {

			}
		}
		delete this;
		return 0;
	}
	StringList files;
};

void MainWindow::handleMatchAll() {
	ListMatcher* matcher = new ListMatcher(File::findFiles(Util::getListPath(), "*.xml*"));
	try {
		matcher->start();
	} catch(const ThreadException&) {
		///@todo add error message
		delete matcher;
	}
}

#ifdef PORT_ME
HWND MainFrame::createToolbar() {
	CToolBarCtrl ctrlToolbar;
	largeImages.CreateFromImage(IDB_TOOLBAR20, 20, 15, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION | LR_SHARED);
	largeImagesHot.CreateFromImage(IDB_TOOLBAR20_HOT, 20, 15, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION | LR_SHARED);

	ctrlToolbar.Create(m_hWnd, NULL, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS, 0, ATL_IDW_TOOLBAR);
	ctrlToolbar.SetImageList(largeImages);
	ctrlToolbar.SetHotImageList(largeImagesHot);

	const int numButtons = 22;


	TBBUTTON tb[numButtons];
	memset(tb, 0, sizeof(tb));
	int n = 0, bitmap = 0;

	tb[n].iBitmap = bitmap++;
	tb[n].idCommand = ID_VIEW_CONNECT;
	tb[n].fsState = TBSTATE_ENABLED;
	tb[n].fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE;

	n++;
	tb[n].fsStyle = TBSTYLE_SEP;

	n++;
	tb[n].iBitmap = bitmap++;
	tb[n].idCommand = ID_FILE_RECONNECT;
	tb[n].fsState = TBSTATE_ENABLED;
	tb[n].fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE;

	n++;
	tb[n].iBitmap = bitmap++;
	tb[n].idCommand = IDC_FOLLOW;
	tb[n].fsState = TBSTATE_ENABLED;
	tb[n].fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE;

	n++;
	tb[n].fsStyle = TBSTYLE_SEP;

	n++;
	tb[n].iBitmap = bitmap++;
	tb[n].idCommand = IDC_FAVORITES;
	tb[n].fsState = TBSTATE_ENABLED;
	tb[n].fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE;

	n++;
	tb[n].iBitmap = bitmap++;
	tb[n].idCommand = IDC_FAVUSERS;
	tb[n].fsState = TBSTATE_ENABLED;
	tb[n].fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE;

	n++;
	tb[n].fsStyle = TBSTYLE_SEP;

	n++;
	tb[n].iBitmap = bitmap++;
	tb[n].idCommand = IDC_QUEUE;
	tb[n].fsState = TBSTATE_ENABLED;
	tb[n].fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE;

	n++;
	tb[n].iBitmap = bitmap++;
	tb[n].idCommand = IDC_FINISHED_DL;
	tb[n].fsState = TBSTATE_ENABLED;
	tb[n].fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE;

	n++;
	tb[n].iBitmap = bitmap++;
	tb[n].idCommand = IDC_VIEW_WAITING_USERS;
	tb[n].fsState = TBSTATE_ENABLED;
	tb[n].fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE;

	n++;
	tb[n].iBitmap = bitmap++;
	tb[n].idCommand = IDC_FINISHED_UL;
	tb[n].fsState = TBSTATE_ENABLED;
	tb[n].fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE;

	n++;
	tb[n].fsStyle = TBSTYLE_SEP;

	n++;
	tb[n].iBitmap = bitmap++;
	tb[n].idCommand = ID_FILE_SEARCH;
	tb[n].fsState = TBSTATE_ENABLED;
	tb[n].fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE;

	n++;
	tb[n].iBitmap = bitmap++;
	tb[n].idCommand = IDC_FILE_ADL_SEARCH;
	tb[n].fsState = TBSTATE_ENABLED;
	tb[n].fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE;

	n++;
	tb[n].iBitmap = bitmap++;
	tb[n].idCommand = IDC_SEARCH_SPY;
	tb[n].fsState = TBSTATE_ENABLED;
	tb[n].fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE;

	n++;
	tb[n].fsStyle = TBSTYLE_SEP;

	n++;
	tb[n].iBitmap = bitmap++;
	tb[n].idCommand = IDC_OPEN_FILE_LIST;
	tb[n].fsState = TBSTATE_ENABLED;
	tb[n].fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE;

	n++;
	tb[n].fsStyle = TBSTYLE_SEP;

	n++;
	tb[n].iBitmap = bitmap++;
	tb[n].idCommand = ID_FILE_SETTINGS;
	tb[n].fsState = TBSTATE_ENABLED;
	tb[n].fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE;

	n++;
	tb[n].fsStyle = TBSTYLE_SEP;

	n++;
	tb[n].iBitmap = bitmap++;
	tb[n].idCommand = IDC_NOTEPAD;
	tb[n].fsState = TBSTATE_ENABLED;
	tb[n].fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE;

	ctrlToolbar.SetButtonStructSize();
	ctrlToolbar.AddButtons(numButtons, tb);
	ctrlToolbar.AutoSize();

	return ctrlToolbar.m_hWnd;
}

void MainFrame::parseCommandLine(const tstring& cmdLine)
{
	string::size_type i = 0;
	string::size_type j;

	if( (j = cmdLine.find(_T("dchub://"), i)) != string::npos) {
		WinUtil::parseDchubUrl(cmdLine.substr(j));
	}
	if( (j = cmdLine.find(_T("adc://"), i)) != string::npos) {
		WinUtil::parseADChubUrl(cmdLine.substr(j));
	}
	if( (j = cmdLine.find(_T("magnet:?"), i)) != string::npos) {
		WinUtil::parseMagnetUri(cmdLine.substr(j));
	}
}

LRESULT MainFrame::onCopyData(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	tstring cmdLine = (LPCTSTR) (((COPYDATASTRUCT *)lParam)->lpData);
	parseCommandLine(Text::toT(WinUtil::getAppName() + " ") + cmdLine);
	return true;
}
#endif

void MainWindow::handleHashProgress() {
	HashProgressDlg(this, false).run();
}

void MainWindow::handleAbout() {
	AboutDlg(this).run();
}

void MainWindow::handleOpenDownloadsDir() {
	WinUtil::openFile(Text::toT(SETTING(DOWNLOAD_DIRECTORY)));
}

void MainWindow::handleMinimizeAll() {
	HWND tmpWnd = ::GetWindow(getMDIClient()->handle(), GW_CHILD); //getting first child window
	while (tmpWnd!=NULL) {
		::CloseWindow(tmpWnd);
		tmpWnd = ::GetWindow(tmpWnd, GW_HWNDNEXT);
	}
}

void MainWindow::handleRestoreAll() {
	HWND tmpWnd = ::GetWindow(getMDIClient()->handle(), GW_CHILD); //getting first child window
	while (tmpWnd!=NULL) {
		::SendMessage(getMDIClient()->handle(), WM_MDIRESTORE, (WPARAM)tmpWnd, 0);
		tmpWnd = ::GetWindow(tmpWnd, GW_HWNDNEXT);
	}
}

void MainWindow::handleOpenWindow(unsigned id) {
	switch(id) {
	case IDC_PUBLIC_HUBS: PublicHubsFrame::openWindow(getMDIClient()); break;
	case IDC_FAVORITE_HUBS: FavHubsFrame::openWindow(getMDIClient()); break;
	case IDC_FAVUSERS: UsersFrame::openWindow(getMDIClient()); break;
	case IDC_QUEUE: QueueFrame::openWindow(getMDIClient()); break;
	case IDC_FINISHED_DL: FinishedDLFrame::openWindow(getMDIClient()); break;
	case IDC_WAITING_USERS: WaitingUsersFrame::openWindow(getMDIClient()); break;
	case IDC_FINISHED_UL: FinishedULFrame::openWindow(getMDIClient()); break;
	case IDC_SEARCH: SearchFrame::openWindow(getMDIClient()); break;
	case IDC_ADL_SEARCH: ADLSearchFrame::openWindow(getMDIClient()); break;
	case IDC_SEARCH_SPY: SpyFrame::openWindow(getMDIClient()); break;
	case IDC_NOTEPAD: NotepadFrame::openWindow(getMDIClient()); break;
	case IDC_SYSTEM_LOG: SystemFrame::openWindow(getMDIClient()); break;
	case IDC_NET_STATS: StatsFrame::openWindow(getMDIClient()); break;
	default: dcassert(0); break;
	}
}

void MainWindow::on(HttpConnectionListener::Complete, HttpConnection* /*aConn*/, const string&) throw() {
	try {
		SimpleXML xml;
		xml.fromXML(versionInfo);
		xml.stepIn();

		string url = Text::fromT(links.homepage);

		if(xml.findChild("URL")) {
			url = xml.getChildData();
		}

		xml.resetCurrentChild();
		if(xml.findChild("Version")) {
			if(Util::toDouble(xml.getChildData()) > VERSIONFLOAT) {
				xml.resetCurrentChild();
				xml.resetCurrentChild();
				if(xml.findChild("Title")) {
					const string& title = xml.getChildData();
					xml.resetCurrentChild();
					if(xml.findChild("Message")) {
#ifdef PORT_ME
						if(url.empty()) {
							const string& msg = xml.getChildData();
							MessageBox(Text::toT(msg).c_str(), Text::toT(title).c_str(), MB_OK);
						} else {
							string msg = xml.getChildData() + "\r\n" + STRING(OPEN_DOWNLOAD_PAGE);
							if(MessageBox(Text::toT(msg).c_str(), Text::toT(title).c_str(), MB_YESNO) == IDYES) {
								WinUtil::openLink(Text::toT(url));
							}
						}
#endif
					}
				}
			}

			xml.resetCurrentChild();
			if(xml.findChild("Links")) {
				xml.stepIn();
				if(xml.findChild("Homepage")) {
					links.homepage = Text::toT(xml.getChildData());
				}
				xml.resetCurrentChild();
				if(xml.findChild("Downloads")) {
					links.downloads = Text::toT(xml.getChildData());
				}
				xml.resetCurrentChild();
				if(xml.findChild("GeoIP database update")) {
					links.geoipfile = Text::toT(xml.getChildData());
				}
				xml.resetCurrentChild();
				if(xml.findChild("Translations")) {
					links.translations = Text::toT(xml.getChildData());
				}
				xml.resetCurrentChild();
				if(xml.findChild("Faq")) {
					links.faq = Text::toT(xml.getChildData());
				}
				xml.resetCurrentChild();
				if(xml.findChild("Bugs")) {
					links.bugs = Text::toT(xml.getChildData());
				}
				xml.resetCurrentChild();
				if(xml.findChild("Features")) {
					links.features = Text::toT(xml.getChildData());
				}
				xml.resetCurrentChild();
				if(xml.findChild("Help")) {
					links.help = Text::toT(xml.getChildData());
				}
				xml.resetCurrentChild();
				if(xml.findChild("Forum")) {
					links.discuss = Text::toT(xml.getChildData());
				}
				xml.stepOut();
			}
		}
		xml.stepOut();
	} catch (const Exception&) {
		// ...
	}
}
#ifdef PORT_ME

LRESULT MainFrame::onHelp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_DISPLAY_TOC, NULL);
	bHandled = TRUE;
	return 0;
}
#endif

void MainWindow::handleHelp(unsigned id) {
#ifdef PORT_ME
	UINT action = (id == IDC_HELP_CONTENTS) ? HH_DISPLAY_TOC : HH_HELP_CONTEXT;
	::HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), action, id);
#endif
}

#ifdef PORT_ME
LRESULT MainFrame::onGetToolTip(int idCtrl, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	LPNMTTDISPINFO pDispInfo = (LPNMTTDISPINFO)pnmh;
	pDispInfo->szText[0] = 0;

	if((idCtrl != 0) && !(pDispInfo->uFlags & TTF_IDISHWND))
	{
		int stringId = -1;
		switch(idCtrl) {
			case ID_VIEW_CONNECT: stringId = ResourceManager::MENU_PUBLIC_HUBS; break;
			case ID_FILE_RECONNECT: stringId = ResourceManager::MENU_RECONNECT; break;
			case IDC_FOLLOW: stringId = ResourceManager::MENU_FOLLOW_REDIRECT; break;
			case IDC_FAVORITES: stringId = ResourceManager::MENU_FAVORITE_HUBS; break;
			case IDC_FAVUSERS: stringId = ResourceManager::MENU_FAVORITE_USERS; break;
			case IDC_QUEUE: stringId = ResourceManager::MENU_DOWNLOAD_QUEUE; break;
			case IDC_FINISHED_DL: stringId = ResourceManager::FINISHED_DOWNLOADS; break;
			case IDC_FINISHED_UL: stringId = ResourceManager::FINISHED_UPLOADS; break;
			case ID_FILE_SEARCH: stringId = ResourceManager::MENU_SEARCH; break;
			case IDC_FILE_ADL_SEARCH: stringId = ResourceManager::MENU_ADL_SEARCH; break;
			case IDC_VIEW_WAITING_USERS: stringId = ResourceManager::WAITING_USERS; break;
			case IDC_SEARCH_SPY: stringId = ResourceManager::MENU_SEARCH_SPY; break;
			case IDC_OPEN_FILE_LIST: stringId = ResourceManager::MENU_OPEN_FILE_LIST; break;
			case ID_FILE_SETTINGS: stringId = ResourceManager::MENU_SETTINGS; break;
			case IDC_NET_STATS: stringId = ResourceManager::MENU_NETWORK_STATISTICS; break;
			case IDC_NOTEPAD: stringId = ResourceManager::MENU_NOTEPAD; break;
		}
		if(stringId != -1) {
			_tcsncpy(pDispInfo->lpszText, CTSTRING_I((ResourceManager::Strings)stringId), 79);
			pDispInfo->uFlags |= TTF_DI_SETITEM;
		}
	} else { // if we're really in the status bar, this should be detected intelligently
		lastLines.clear();
		for(TStringIter i = lastLinesList.begin(); i != lastLinesList.end(); ++i) {
			lastLines += *i;
			lastLines += _T("\r\n");
		}
		if(lastLines.size() > 2) {
			lastLines.erase(lastLines.size() - 2);
		}
		pDispInfo->lpszText = const_cast<TCHAR*>(lastLines.c_str());
	}
	return 0;
}


LRESULT MainFrame::onSize(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
	if(wParam == SIZE_MINIMIZED) {
		if(BOOLSETTING(AUTO_AWAY) && !Util::getManualAway()) {
			Util::setAway(true);
		}
		if(BOOLSETTING(MINIMIZE_TRAY) != WinUtil::isShift()) {
			updateTray(true);
			ShowWindow(SW_HIDE);
		}
		maximized = IsZoomed() > 0;

	} else if( (wParam == SIZE_RESTORED || wParam == SIZE_MAXIMIZED) ) {
		if(BOOLSETTING(AUTO_AWAY) && !Util::getManualAway()) {
			Util::setAway(false);
		}
		if(trayIcon) {
			updateTray(false);
		}
	}

	bHandled = FALSE;
	return 0;
}
#endif

HRESULT MainWindow::handleEndSession(WPARAM wParam, LPARAM lParam) {
	if(c != NULL) {
		c->removeListener(this);
		delete c;
		c = NULL;
	}

	saveWindowSettings();
	QueueManager::getInstance()->saveQueue();
	SettingsManager::getInstance()->save();

	return 0;
}

void MainWindow::handleLink(unsigned id) {

	tstring site;
	switch(id) {
	case IDC_HELP_HOMEPAGE: site = links.homepage; break;
	case IDC_HELP_DOWNLOADS: site = links.downloads; break;
	case IDC_HELP_GEOIPFILE: site = links.geoipfile; break;
	case IDC_HELP_TRANSLATIONS: site = links.translations; break;
	case IDC_HELP_FAQ: site = links.faq; break;
	case IDC_HELP_FORUM: site = links.help; break;
	case IDC_HELP_DISCUSS: site = links.discuss; break;
	case IDC_HELP_REQUEST_FEATURE: site = links.features; break;
	case IDC_HELP_REPORT_BUG: site = links.bugs; break;
	case IDC_HELP_DONATE: site = Text::toT("https://www.paypal.com/cgi-bin/webscr?cmd=_xclick&business=arnetheduck%40gmail%2ecom&item_name=DCPlusPlus&no_shipping=1&return=http%3a%2f%2fdcplusplus%2esf%2enet%2f&cancel_return=http%3a%2f%2fdcplusplus%2esf%2enet%2f&cn=Greeting&tax=0&currency_code=EUR&bn=PP%2dDonationsBF&charset=UTF%2d8"); break;
	default: dcassert(0);
	}

	WinUtil::openLink(site);
}

void MainWindow::handleRefreshFileList() {
	ShareManager::getInstance()->setDirty();
	ShareManager::getInstance()->refresh(true);
}

#ifdef PORT_ME
LRESULT MainFrame::onTrayIcon(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	if (lParam == WM_LBUTTONUP) {
		ShowWindow(SW_SHOW);
		ShowWindow(maximized ? SW_MAXIMIZE : SW_RESTORE);
	} else if(lParam == WM_RBUTTONDOWN || lParam == WM_CONTEXTMENU){ 
		CPoint pt;
		CMenu mnuTrayMenu;
		mnuTrayMenu.CreatePopupMenu();
		mnuTrayMenu.AppendMenu(MF_STRING, IDC_TRAY_SHOW, CTSTRING(MENU_SHOW));
		mnuTrayMenu.AppendMenu(MF_STRING, IDC_TRAY_QUIT, CTSTRING(MENU_EXIT));
		mnuTrayMenu.AppendMenu(MF_STRING, IDC_OPEN_DOWNLOADS, CTSTRING(MENU_OPEN_DOWNLOADS_DIR));
		mnuTrayMenu.AppendMenu(MF_STRING, ID_FILE_SETTINGS, CTSTRING(MENU_SETTINGS));
		GetCursorPos(&pt);
		SetForegroundWindow(m_hWnd);
		mnuTrayMenu.TrackPopupMenu(TPM_BOTTOMALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON,pt.x,pt.y,m_hWnd);
		PostMessage(WM_NULL, 0, 0);
		mnuTrayMenu.SetMenuDefaultItem(0,TRUE);
	} else if(lParam == WM_MOUSEMOVE && ((lastMove + 1000) < GET_TICK()) ) {
		NOTIFYICONDATA nid;
		nid.cbSize = sizeof(NOTIFYICONDATA);
		nid.hWnd = m_hWnd;
		nid.uID = 0;
		nid.uFlags = NIF_TIP;
		_tcsncpy(nid.szTip, Text::toT("D: " + Util::formatBytes(DownloadManager::getInstance()->getRunningAverage()) + "/s (" +
			Util::toString(DownloadManager::getInstance()->getDownloadCount()) + ")\r\nU: " +
			Util::formatBytes(UploadManager::getInstance()->getRunningAverage()) + "/s (" +
			Util::toString(UploadManager::getInstance()->getUploadCount()) + ")").c_str(), 64);

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
	SettingsManager::getInstance()->set(SettingsManager::SHOW_TOOLBAR, bVisible);
	return 0;
}

LRESULT MainFrame::OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	BOOL bVisible = !::IsWindowVisible(m_hWndStatusBar);
	::ShowWindow(m_hWndStatusBar, bVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
	UISetCheck(ID_VIEW_STATUS_BAR, bVisible);
	UpdateLayout();
	SettingsManager::getInstance()->set(SettingsManager::SHOW_STATUSBAR, bVisible);
	return 0;
}

LRESULT MainFrame::OnViewTransferView(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	BOOL bVisible = !transferView.IsWindowVisible();
	if(!bVisible) {
		if(GetSinglePaneMode() == SPLIT_PANE_NONE)
			SetSinglePaneMode(SPLIT_PANE_TOP);
	} else {
		if(GetSinglePaneMode() != SPLIT_PANE_NONE)
			SetSinglePaneMode(SPLIT_PANE_NONE);
	}
	UISetCheck(ID_VIEW_TRANSFER_VIEW, bVisible);
	UpdateLayout();
	SettingsManager::getInstance()->set(SettingsManager::SHOW_TRANSFERVIEW, bVisible);
	return 0;
}
#endif

void MainWindow::handleCloseWindows(unsigned id) {
	switch(id) {
	case IDC_CLOSE_ALL_DISCONNECTED:	HubFrame::closeDisconnected();		break;
	case IDC_CLOSE_ALL_PM:				PrivateFrame::closeAll();			break;
	case IDC_CLOSE_ALL_OFFLINE_PM:		PrivateFrame::closeAllOffline();	break;
	case IDC_CLOSE_ALL_DIR_LIST:		DirectoryListingFrame::closeAll();	break;
	case IDC_CLOSE_ALL_SEARCH_FRAME:	SearchFrame::closeAll();			break;
	}
}

void MainWindow::on(HttpConnectionListener::Data, HttpConnection* /*conn*/, const uint8_t* buf, size_t len) throw() {
	versionInfo += string((const char*)buf, len);
}

void MainWindow::on(PartialList, const User::Ptr& aUser, const string& text) throw() {
	speak(BROWSE_LISTING, (LPARAM)new DirectoryBrowseInfo(aUser, text));
}

void MainWindow::on(QueueManagerListener::Finished, QueueItem* qi, const string& dir, int64_t speed) throw() {
	if(qi->isSet(QueueItem::FLAG_CLIENT_VIEW)) {
		if(qi->isSet(QueueItem::FLAG_USER_LIST)) {
			// This is a file listing, show it...
			DirectoryListInfo* i = new DirectoryListInfo(qi->getCurrent(), Text::toT(qi->getListName()), Text::toT(dir), speed);

			speak(DOWNLOAD_LISTING, (LPARAM)i);
		} else if(qi->isSet(QueueItem::FLAG_TEXT)) {
			speak(VIEW_FILE_AND_DELETE, (LPARAM) new tstring(Text::toT(qi->getTarget())));
		}
	}
}

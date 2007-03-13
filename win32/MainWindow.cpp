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
#include <client/DCPlusPlus.h>

#include "MainWindow.h"
#include "resource.h"

#include "SystemFrame.h"
#include "NotepadFrame.h"
#include "HubFrame.h"
#include "PublicHubsFrm.h"
#include "FavHubsFrame.h"

#include "LineDlg.h"

#include <client/SettingsManager.h>
#include <client/ResourceManager.h>
#include <client/version.h>
#include <client/DownloadManager.h>
#include <client/UploadManager.h>
#include <client/Client.h>
#include <client/TimerManager.h>

MainWindow::MainWindow() :
	status(0),
	mdi(0),
	lastUp(0),
	lastDown(0),
	lastTick(GET_TICK())
{
	#ifdef PORT_ME

	links.homepage = _T("http://dcpp.net/");
	links.downloads = links.homepage + _T("download/");
	links.geoipfile = _T("http://www.maxmind.com/download/geoip/database/GeoIPCountryCSV.zip");
	links.translations = _T("http://sourceforge.net/tracker/?atid=460289&group_id=40287");
	links.faq = links.homepage + _T("faq/");
	links.help = links.homepage + _T("forum/");
	links.discuss = links.homepage + _T("forum/");
	links.features = links.homepage + _T("bugzilla/");
	links.bugs = links.homepage + _T("bugzilla/");
#endif

	initWindow();
	initMenu();
	initStatusBar();
	initMDI();
	initSecond();

	updateStatus();
	layout();
	
	onSized(&MainWindow::sized);
	
#ifdef PORT_ME
	TimerManager::getInstance()->addListener(this);
	QueueManager::getInstance()->addListener(this);
	LogManager::getInstance()->addListener(this);


	trayMessage = RegisterWindowMessage(_T("TaskbarCreated"));

	TimerManager::getInstance()->start();

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
	m_CmdBar.m_arrCommand.Add(IDC_FINISHED);
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

	SetSplitterPanes(m_hWndMDIClient, transferView.m_hWnd);
	SetSplitterExtendedStyle(SPLIT_PROPORTIONAL);
	m_nProportionalPos = 8000;

	UIAddToolBar(hWndToolBar);
	UISetCheck(ID_VIEW_TOOLBAR, 1);
	UISetCheck(ID_VIEW_STATUS_BAR, 1);
	UISetCheck(ID_VIEW_TRANSFER_VIEW, 1);

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	c->addListener(this);
	c->downloadFile("http://dcplusplus.sourceforge.net/version.xml");

	if(BOOLSETTING(OPEN_SYSTEM_LOG)) PostMessage(WM_COMMAND, IDC_SYSTEM_LOG);
	if(BOOLSETTING(OPEN_PUBLIC)) PostMessage(WM_COMMAND, ID_VIEW_CONNECT);
	if(BOOLSETTING(OPEN_FAVORITE_HUBS)) PostMessage(WM_COMMAND, IDC_FAVORITES);
	if(BOOLSETTING(OPEN_FAVORITE_USERS)) PostMessage(WM_COMMAND, IDC_FAVUSERS);
	if(BOOLSETTING(OPEN_QUEUE)) PostMessage(WM_COMMAND, IDC_QUEUE);
	if(BOOLSETTING(OPEN_FINISHED_DOWNLOADS)) PostMessage(WM_COMMAND, IDC_FINISHED);
	if(BOOLSETTING(OPEN_WAITING_USERS)) PostMessage(WM_COMMAND, IDC_VIEW_WAITING_USERS);
	if(BOOLSETTING(OPEN_FINISHED_UPLOADS)) PostMessage(WM_COMMAND, IDC_FINISHED_UL);
	if(BOOLSETTING(OPEN_SEARCH_SPY)) PostMessage(WM_COMMAND, IDC_SEARCH_SPY);
	if(BOOLSETTING(OPEN_NETWORK_STATISTICS)) PostMessage(WM_COMMAND, IDC_NET_STATS);
	if(BOOLSETTING(OPEN_NOTEPAD)) PostMessage(WM_COMMAND, IDC_NOTEPAD);

	if(!BOOLSETTING(SHOW_STATUSBAR)) PostMessage(WM_COMMAND, ID_VIEW_STATUS_BAR);
	if(!BOOLSETTING(SHOW_TOOLBAR)) PostMessage(WM_COMMAND, ID_VIEW_TOOLBAR);
	if(!BOOLSETTING(SHOW_TRANSFERVIEW)) PostMessage(WM_COMMAND, ID_VIEW_TRANSFER_VIEW);

	if(!WinUtil::isShift())
		PostMessage(WM_SPEAKER, AUTO_CONNECT);

	PostMessage(WM_SPEAKER, PARSE_COMMAND_LINE);

	File::ensureDirectory(SETTING(LOG_DIRECTORY));

	startSocket();

	if(SETTING(NICK).empty()) {
		HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_GENERALPAGE);
		PostMessage(WM_COMMAND, ID_FILE_SETTINGS);
	}

	// We want to pass this one on to the splitter...hope it get's there...
	bHandled = FALSE;
	return 0;
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
	
	cs.exStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	if(ResourceManager::getInstance()->isRTL())
		cs.exStyle |= WS_EX_RTLREADING; 

#ifdef PORT_ME
	wndMain.ShowWindow(((nCmdShow == SW_SHOWDEFAULT) || (nCmdShow == SW_SHOWNORMAL)) ? SETTING(MAIN_WINDOW_STATE) : nCmdShow);
#endif
	// Set window name
	cs.caption = _T(APPNAME) _T(" ") _T(VERSIONSTRING);

	createWindow(cs);
}

void MainWindow::initMenu() {
	dcdebug("initMenu\n");
	mainMenu = createMenu();
	
	WidgetMenuPtr file = mainMenu->appendPopup(CTSTRING(MENU_FILE));
	
	file->appendItem(IDC_QUICK_CONNECT, TSTRING(MENU_QUICK_CONNECT), &MainWindow::handleQuickConnect);

#ifdef PORT_ME
	file.AppendMenu(MF_STRING, IDC_OPEN_FILE_LIST, CTSTRING(MENU_OPEN_FILE_LIST));
	file.AppendMenu(MF_STRING, IDC_OPEN_OWN_LIST, CTSTRING(MENU_OPEN_OWN_LIST));
	file.AppendMenu(MF_STRING, IDC_MATCH_ALL, CTSTRING(MENU_OPEN_MATCH_ALL));
	file.AppendMenu(MF_STRING, IDC_REFRESH_FILE_LIST, CTSTRING(MENU_REFRESH_FILE_LIST));
	file.AppendMenu(MF_STRING, IDC_OPEN_DOWNLOADS, CTSTRING(MENU_OPEN_DOWNLOADS_DIR));
	file.AppendMenu(MF_SEPARATOR);
	file.AppendMenu(MF_STRING, IDC_FOLLOW, CTSTRING(MENU_FOLLOW_REDIRECT));
	file.AppendMenu(MF_STRING, ID_FILE_RECONNECT, CTSTRING(MENU_RECONNECT));
	file.AppendMenu(MF_SEPARATOR);
	file.AppendMenu(MF_STRING, ID_FILE_SETTINGS, CTSTRING(MENU_SETTINGS));

#endif
	file->appendSeparatorItem();
	file->appendItem(IDC_EXIT, TSTRING(MENU_EXIT), &MainWindow::handleExit);

	WidgetMenuPtr view = mainMenu->appendPopup(CTSTRING(MENU_VIEW));

	view->appendItem(IDC_PUBLIC_HUBS, TSTRING(MENU_PUBLIC_HUBS), &MainWindow::handlePublicHubs);
	view->appendItem(IDC_FAVORITE_HUBS, TSTRING(MENU_FAVORITE_HUBS), &MainWindow::handleFavHubs);
	view->appendItem(IDC_SYSTEM_LOG, TSTRING(MENU_SYSTEM_LOG), &MainWindow::handleSystemLog);
	view->appendItem(IDC_NOTEPAD, TSTRING(MENU_NOTEPAD), &MainWindow::handleNotepad);
	
#ifdef PORT_ME
	view.AppendMenu(MF_STRING, IDC_QUEUE, CTSTRING(MENU_DOWNLOAD_QUEUE));
	view.AppendMenu(MF_STRING, IDC_VIEW_WAITING_USERS, CTSTRING(WAITING_USERS));
	view.AppendMenu(MF_STRING, IDC_FINISHED, CTSTRING(FINISHED_DOWNLOADS));
	view.AppendMenu(MF_STRING, IDC_FINISHED_UL, CTSTRING(FINISHED_UPLOADS));
	view.AppendMenu(MF_STRING, IDC_FAVUSERS, CTSTRING(MENU_FAVORITE_USERS));
	view.AppendMenu(MF_STRING, ID_FILE_SEARCH, CTSTRING(MENU_SEARCH));
	view.AppendMenu(MF_STRING, IDC_FILE_ADL_SEARCH, CTSTRING(MENU_ADL_SEARCH));
	view.AppendMenu(MF_STRING, IDC_SEARCH_SPY, CTSTRING(MENU_SEARCH_SPY));
	view.AppendMenu(MF_STRING, IDC_NET_STATS, CTSTRING(MENU_NETWORK_STATISTICS));
	view.AppendMenu(MF_STRING, IDC_HASH_PROGRESS, CTSTRING(MENU_HASH_PROGRESS));
	view.AppendMenu(MF_STRING, IDC_SYSTEM_LOG, CTSTRING(MENU_SYSTEM_LOG));
	view.AppendMenu(MF_SEPARATOR);
	view.AppendMenu(MF_STRING, ID_VIEW_TOOLBAR, CTSTRING(MENU_TOOLBAR));
	view.AppendMenu(MF_STRING, ID_VIEW_STATUS_BAR, CTSTRING(MENU_STATUS_BAR));
	view.AppendMenu(MF_STRING, ID_VIEW_TRANSFER_VIEW, CTSTRING(MENU_TRANSFER_VIEW));

#endif

	WidgetMenuPtr window = mainMenu->appendPopup(CTSTRING(MENU_WINDOW));
	
#ifdef PORT_ME
	window.AppendMenu(MF_STRING, ID_WINDOW_CASCADE, CTSTRING(MENU_CASCADE));
	window.AppendMenu(MF_STRING, ID_WINDOW_TILE_HORZ, CTSTRING(MENU_HORIZONTAL_TILE));
	window.AppendMenu(MF_STRING, ID_WINDOW_TILE_VERT, CTSTRING(MENU_VERTICAL_TILE));
	window.AppendMenu(MF_STRING, ID_WINDOW_ARRANGE, CTSTRING(MENU_ARRANGE));
	window.AppendMenu(MF_STRING, ID_WINDOW_MINIMIZE_ALL, CTSTRING(MENU_MINIMIZE_ALL));
	window.AppendMenu(MF_STRING, ID_WINDOW_RESTORE_ALL, CTSTRING(MENU_RESTORE_ALL));
	window.AppendMenu(MF_SEPARATOR);
	window.AppendMenu(MF_STRING, IDC_CLOSE_DISCONNECTED, CTSTRING(MENU_CLOSE_DISCONNECTED));
	window.AppendMenu(MF_STRING, IDC_CLOSE_ALL_PM, CTSTRING(MENU_CLOSE_ALL_PM));
	window.AppendMenu(MF_STRING, IDC_CLOSE_ALL_OFFLINE_PM, CTSTRING(MENU_CLOSE_ALL_OFFLINE_PM));
	window.AppendMenu(MF_STRING, IDC_CLOSE_ALL_DIR_LIST, CTSTRING(MENU_CLOSE_ALL_DIR_LIST));
	window.AppendMenu(MF_STRING, IDC_CLOSE_ALL_SEARCH_FRAME, CTSTRING(MENU_CLOSE_ALL_SEARCHFRAME));

#endif

	WidgetMenuPtr help = mainMenu->appendPopup(CTSTRING(MENU_HELP));

#ifdef PORT_ME
	help.AppendMenu(MF_STRING, IDC_HELP_CONTENTS, CTSTRING(MENU_CONTENTS));
	help.AppendMenu(MF_SEPARATOR);
	help.AppendMenu(MF_STRING, IDC_HELP_CHANGELOG, CTSTRING(MENU_CHANGELOG));
	help.AppendMenu(MF_STRING, ID_APP_ABOUT, CTSTRING(MENU_ABOUT));
	help.AppendMenu(MF_SEPARATOR);
	help.AppendMenu(MF_STRING, IDC_HELP_HOMEPAGE, CTSTRING(MENU_HOMEPAGE));
	help.AppendMenu(MF_STRING, IDC_HELP_DOWNLOADS, CTSTRING(MENU_HELP_DOWNLOADS));
	help.AppendMenu(MF_STRING, IDC_HELP_GEOIPFILE, CTSTRING(MENU_HELP_GEOIPFILE));
	help.AppendMenu(MF_STRING, IDC_HELP_TRANSLATIONS, CTSTRING(MENU_HELP_TRANSLATIONS));
	help.AppendMenu(MF_STRING, IDC_HELP_FAQ, CTSTRING(MENU_FAQ));
	help.AppendMenu(MF_STRING, IDC_HELP_HELP_FORUM, CTSTRING(MENU_HELP_FORUM));
	help.AppendMenu(MF_STRING, IDC_HELP_DISCUSS, CTSTRING(MENU_DISCUSS));
	help.AppendMenu(MF_STRING, IDC_HELP_REQUEST_FEATURE, CTSTRING(MENU_REQUEST_FEATURE));
	help.AppendMenu(MF_STRING, IDC_HELP_REPORT_BUG, CTSTRING(MENU_REPORT_BUG));
	help.AppendMenu(MF_STRING, IDC_HELP_DONATE, CTSTRING(MENU_DONATE));

#endif

	mainMenu->attach(this);	
}

void MainWindow::initStatusBar() {
	dcdebug("initStatusBar\n");
	status = createStatusBarSections();

	memset(statusSizes, 0, sizeof(statusSizes));
	statusSizes[STATUS_AWAY] = status->getTextSize(TSTRING(AWAY)).x + 12;
	///@todo set to checkbox width + resizedrag width really
	statusSizes[STATUS_DUMMY] = 32;
}

void MainWindow::initMDI() {
	dcdebug("initMDI\n");
	mdi = createMDIParent();
}

void MainWindow::handleExit(WidgetMenuPtr /* menu */, unsigned /* id*/) {
	close(true);
}

void MainWindow::handlePublicHubs(WidgetMenuPtr, unsigned) {
	PublicHubsFrame::openWindow(mdi);
}

void MainWindow::handleFavHubs(WidgetMenuPtr, unsigned) {
	FavHubsFrame::openWindow(mdi);
}

void MainWindow::handleSystemLog(WidgetMenuPtr, unsigned) {
	SystemFrame::openWindow(mdi);
}

void MainWindow::handleNotepad(WidgetMenuPtr, unsigned) {
	NotepadFrame::openWindow(mdi);
}

void MainWindow::handleQuickConnect(WidgetMenuPtr, unsigned) {
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

		HubFrame::openWindow(mdi, tmp);
	}
}
 
void MainWindow::sized(const SmartWin::WidgetSizedEventResult& sz) {
	layout();
}

void MainWindow::initSecond() {
	SmartWin::Command cmd(_T("1 second"));
	createTimer(&MainWindow::eachSecond, 1000, cmd);	
}

void MainWindow::eachSecond(const SmartWin::CommandPtr&) {
	updateStatus();
	///@todo change smartwin to support recurring timers 
	initSecond();
}

void MainWindow::layout() {
	SmartWin::Rectangle r(getClientAreaSize()); 
	status->refresh();

	SmartWin::Rectangle rs(status->getClientAreaSize());
	
	{
		std::vector<unsigned> w(STATUS_LAST);
		w[0] = rs.size.x - rs.pos.x - std::accumulate(statusSizes+1, statusSizes+STATUS_LAST, 0); 
		std::copy(statusSizes+1, statusSizes + STATUS_LAST, w.begin()+1);

		status->setSections(w);
#ifdef PORT_ME
		ctrlLastLines.SetMaxTipWidth(w[0]);
#endif
	}

	r.size.y -= status->getSize().y;

#ifdef PORT_ME
	CRect rc = rect;
	rc.top = rc.bottom - ctrlTab.getHeight();
	if(ctrlTab.IsWindow())
		ctrlTab.MoveWindow(rc);

	CRect rc2 = rect;
	rc2.bottom = rc.top;
	SetSplitterRect(rc2);
#endif
	mdi->setBounds(r);
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

void MainWindow::setStatus(Status s, const tstring& text) {
	int w = status->getTextSize(text).x + 12;
	if(w > static_cast<int>(statusSizes[s])) {
		dcdebug("Setting status size %d to %d\n", s, w);
		statusSizes[s] = w;
		layout();
	}
	status->setText(text, s);
}

#ifdef PORT_ME

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
#include "StatsFrame.h"
#include "WaitingUsersFrame.h"
#include "LineDlg.h"
#include "HashProgressDlg.h"
#include "UPnP.h"
#include "SystemFrame.h"
#include "PrivateFrame.h"

#include "../client/ConnectionManager.h"
#include "../client/DownloadManager.h"
#include "../client/UploadManager.h"
#include "../client/StringTokenizer.h"
#include "../client/SimpleXML.h"
#include "../client/ShareManager.h"
#include "../client/version.h"

MainFrame::MainFrame() : trayMessage(0), trayIcon(false), maximized(false), lastUpload(-1), lastUpdate(0),
lastUp(0), lastDown(0), oldshutdown(false), stopperThread(NULL), c(new HttpConnection()),
closing(false), missedAutoConnect(false), UPnP_TCPConnection(NULL), UPnP_UDPConnection(NULL)
#endif

MainWindow::~MainWindow() {
#ifdef PORT_ME
	m_CmdBar.m_hImageList = NULL;

	images.Destroy();
	largeImages.Destroy();
	largeImagesHot.Destroy();

	WinUtil::uninit();
#endif
}
#ifdef PORT_ME
DWORD WINAPI MainFrame::stopper(void* p) {
	MainFrame* mf = (MainFrame*)p;
	HWND wnd, wnd2 = NULL;

	while( (wnd=::GetWindow(mf->m_hWndMDIClient, GW_CHILD)) != NULL) {
		if(wnd == wnd2)
			Sleep(100);
		else {
			::PostMessage(wnd, WM_CLOSE, 0, 0);
			wnd2 = wnd;
		}
	}

	mf->PostMessage(WM_CLOSE);
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

LRESULT MainFrame::onMatchAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	ListMatcher* matcher = new ListMatcher(File::findFiles(Util::getListPath(), "*.xml*"));
	try {
		matcher->start();
	} catch(const ThreadException&) {
		///@todo add error message
		delete matcher;
	}
	
	return 0;
}

void MainFrame::startSocket() {
	SearchManager::getInstance()->disconnect();
	ConnectionManager::getInstance()->disconnect();

	if(ClientManager::getInstance()->isActive()) {
		try {
			ConnectionManager::getInstance()->listen();
		} catch(const Exception&) {
			MessageBox(CTSTRING(TCP_PORT_BUSY), _T(APPNAME) _T(" ") _T(VERSIONSTRING), MB_ICONSTOP | MB_OK);
		}
		try {
			SearchManager::getInstance()->listen();
		} catch(const Exception&) {
			MessageBox(CTSTRING(TCP_PORT_BUSY), _T(APPNAME) _T(" ") _T(VERSIONSTRING), MB_ICONSTOP | MB_OK);
		}
	}

	startUPnP();
}

void MainFrame::startUPnP() {
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
}

void MainFrame::stopUPnP() {
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
}

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
	tb[n].idCommand = IDC_FINISHED;
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

LRESULT MainFrame::onSpeaker(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {

	if(wParam == DOWNLOAD_LISTING) {
		auto_ptr<DirectoryListInfo> i(reinterpret_cast<DirectoryListInfo*>(lParam));
		DirectoryListingFrame::openWindow(i->file, i->dir, i->user, i->speed);
	} else if(wParam == BROWSE_LISTING) {
		auto_ptr<DirectoryBrowseInfo> i(reinterpret_cast<DirectoryBrowseInfo*>(lParam));
		DirectoryListingFrame::openWindow(i->user, i->text, 0);
	} else if(wParam == VIEW_FILE_AND_DELETE) {
		auto_ptr<tstring> file(reinterpret_cast<tstring*>(lParam));
		TextFrame::openWindow(*file);
		File::deleteFile(Text::fromT(*file));
	} else if(wParam == AUTO_CONNECT) {
		autoConnect(FavoriteManager::getInstance()->getFavoriteHubs());
	} else if(wParam == PARSE_COMMAND_LINE) {
		parseCommandLine(GetCommandLine());
	} else if(wParam == STATUS_MESSAGE) {
		auto_ptr<pair<time_t, tstring> > msg((pair<time_t, tstring>*)lParam);
		if(ctrlStatus.IsWindow()) {
			tstring line = Text::toT("[" + Util::getShortTimeString(msg->first) + "] ") + msg->second;

			ctrlStatus.SetText(0, line.c_str());
			while(lastLinesList.size() + 1 > MAX_CLIENT_LINES)
				lastLinesList.erase(lastLinesList.begin());
			if (line.find(_T('\r')) == tstring::npos) {
				lastLinesList.push_back(line);
			} else {
				lastLinesList.push_back(line.substr(0, line.find(_T('\r'))));
			}
		}
	}

	return 0;
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

LRESULT MainFrame::onHashProgress(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	HashProgressDlg(false).DoModal(m_hWnd);
	return 0;
}

LRESULT MainFrame::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	AboutDlg dlg;
	dlg.DoModal(m_hWnd);
	return 0;
}

LRESULT MainFrame::onOpenWindows(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	switch(wID) {
		case ID_FILE_SEARCH: SearchFrame::openWindow(); break;
		case ID_VIEW_CONNECT: PublicHubsFrame::openWindow(); break;
		case IDC_FAVORITES: FavoriteHubsFrame::openWindow(); break;
		case IDC_FAVUSERS: UsersFrame::openWindow(); break;
		case IDC_NOTEPAD: NotepadFrame::openWindow(); break;
		case IDC_QUEUE: QueueFrame::openWindow(); break;
		case IDC_SEARCH_SPY: SpyFrame::openWindow(); break;
		case IDC_FILE_ADL_SEARCH: ADLSearchFrame::openWindow(); break;
		case IDC_NET_STATS: StatsFrame::openWindow(); break;
		case IDC_FINISHED: FinishedFrame::openWindow(); break;
		case IDC_FINISHED_UL: FinishedULFrame::openWindow(); break;
		case IDC_VIEW_WAITING_USERS: WaitingUsersFrame::openWindow(); break;
		case IDC_SYSTEM_LOG: SystemFrame::openWindow(); break;
		default: dcassert(0); break;
	}
	return 0;
}

LRESULT MainFrame::OnFileSettings(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	PropertiesDlg dlg(m_hWnd, SettingsManager::getInstance());

	unsigned short lastTCP = static_cast<unsigned short>(SETTING(TCP_PORT));
	unsigned short lastUDP = static_cast<unsigned short>(SETTING(UDP_PORT));
	unsigned short lastTLS = static_cast<unsigned short>(SETTING(TLS_PORT));

	int lastConn = SETTING(INCOMING_CONNECTIONS);

	bool lastSortFavUsersFirst = BOOLSETTING(SORT_FAVUSERS_FIRST);

	if(dlg.DoModal(m_hWnd) == IDOK)
	{
		SettingsManager::getInstance()->save();
		if(missedAutoConnect && !SETTING(NICK).empty()) {
			PostMessage(WM_SPEAKER, AUTO_CONNECT);
		}
		if(SETTING(INCOMING_CONNECTIONS) != lastConn || SETTING(TCP_PORT) != lastTCP || SETTING(UDP_PORT) != lastUDP || SETTING(TLS_PORT) != lastTLS) {
			startSocket();
		}
		ClientManager::getInstance()->infoUpdated();

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
	}
	return 0;
}

void MainFrame::on(HttpConnectionListener::Complete, HttpConnection* /*aConn*/, const string&) throw() {
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
						if(url.empty()) {
							const string& msg = xml.getChildData();
							MessageBox(Text::toT(msg).c_str(), Text::toT(title).c_str(), MB_OK);
						} else {
							string msg = xml.getChildData() + "\r\n" + STRING(OPEN_DOWNLOAD_PAGE);
							if(MessageBox(Text::toT(msg).c_str(), Text::toT(title).c_str(), MB_YESNO) == IDYES) {
								WinUtil::openLink(Text::toT(url));
							}
						}
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

LRESULT MainFrame::onHelp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_DISPLAY_TOC, NULL);
	bHandled = TRUE;
	return 0;
}

LRESULT MainFrame::onMenuHelp(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled) {
	UINT action = (wID == IDC_HELP_CONTENTS) ? HH_DISPLAY_TOC : HH_HELP_CONTEXT;
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), action, wID);
	bHandled = TRUE;
	return 0;
}

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
			case IDC_FINISHED: stringId = ResourceManager::FINISHED_DOWNLOADS; break;
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

void MainFrame::autoConnect(const FavoriteHubEntry::List& fl) {
	missedAutoConnect = false;
	for(FavoriteHubEntry::List::const_iterator i = fl.begin(); i != fl.end(); ++i) {
		FavoriteHubEntry* entry = *i;
		if(entry->getConnect()) {
			if(!entry->getNick().empty() || !SETTING(NICK).empty())
				HubFrame::openWindow(Text::toT(entry->getServer()));
			else
				missedAutoConnect = true;
		}
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
		_tcscpy(nid.szTip, _T("DC++"));
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
		if( oldshutdown ||(!BOOLSETTING(CONFIRM_EXIT)) || (MessageBox(CTSTRING(REALLY_EXIT), _T(APPNAME) _T(" ") _T(VERSIONSTRING), MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDYES) ) {
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

			stopUPnP();

			DWORD id;
			stopperThread = CreateThread(NULL, 0, stopper, this, 0, &id);
			closing = true;
		}
		bHandled = TRUE;
	} else {
		// This should end immediately, as it only should be the stopper that sends another WM_CLOSE
		WaitForSingleObject(stopperThread, 60*1000);
		CloseHandle(stopperThread);
		stopperThread = NULL;
		bHandled = FALSE;
	}

	return 0;
}

LRESULT MainFrame::onLink(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {

	tstring site;
	switch(wID) {
	case IDC_HELP_HOMEPAGE: site = links.homepage; break;
	case IDC_HELP_DOWNLOADS: site = links.downloads; break;
	case IDC_HELP_GEOIPFILE: site = links.geoipfile; break;
	case IDC_HELP_TRANSLATIONS: site = links.translations; break;
	case IDC_HELP_FAQ: site = links.faq; break;
	case IDC_HELP_HELP_FORUM: site = links.help; break;
	case IDC_HELP_DISCUSS: site = links.discuss; break;
	case IDC_HELP_REQUEST_FEATURE: site = links.features; break;
	case IDC_HELP_REPORT_BUG: site = links.bugs; break;
	case IDC_HELP_DONATE: site = Text::toT("https://www.paypal.com/cgi-bin/webscr?cmd=_xclick&business=arnetheduck%40gmail%2ecom&item_name=DCPlusPlus&no_shipping=1&return=http%3a%2f%2fdcplusplus%2esf%2enet%2f&cancel_return=http%3a%2f%2fdcplusplus%2esf%2enet%2f&cn=Greeting&tax=0&currency_code=EUR&bn=PP%2dDonationsBF&charset=UTF%2d8"); break;
	default: dcassert(0);
	}

	WinUtil::openLink(site);

	return 0;
}

static const TCHAR types[] = _T("File Lists\0*.DcLst;*.xml.bz2\0All Files\0*.*\0");

LRESULT MainFrame::onOpenFileList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	tstring file;
	if(WinUtil::browseFile(file, m_hWnd, false, Text::toT(Util::getListPath()), types)) {
		User::Ptr u = DirectoryListing::getUserFromFilename(Text::fromT(file));
		if(u) {
			DirectoryListingFrame::openWindow(file, Text::toT(Util::emptyString), u, 0);
		} else {
			MessageBox(CTSTRING(INVALID_LISTNAME), _T(APPNAME) _T(" ") _T(VERSIONSTRING));
		}
	}
	return 0;
}

LRESULT MainFrame::onOpenOwnList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(!ShareManager::getInstance()->getOwnListFile().empty()){
		DirectoryListingFrame::openWindow(Text::toT(ShareManager::getInstance()->getOwnListFile()), Text::toT(Util::emptyString), ClientManager::getInstance()->getMe(), 0);
	}
	return 0;
}

LRESULT MainFrame::onRefreshFileList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	ShareManager::getInstance()->setDirty();
	ShareManager::getInstance()->refresh(true);
	return 0;
}

LRESULT MainFrame::onTrayIcon(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	if (lParam == WM_LBUTTONUP) {
		ShowWindow(SW_SHOW);
		ShowWindow(maximized ? SW_MAXIMIZE : SW_RESTORE);
	}else if(lParam == WM_RBUTTONDOWN || lParam == WM_CONTEXTMENU){ 
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

LRESULT MainFrame::onCloseWindows(WORD , WORD wID, HWND , BOOL& ) {
	switch(wID) {
	case IDC_CLOSE_DISCONNECTED:		HubFrame::closeDisconnected();		break;
	case IDC_CLOSE_ALL_PM:				PrivateFrame::closeAll();			break;
	case IDC_CLOSE_ALL_OFFLINE_PM:		PrivateFrame::closeAllOffline();	break;
	case IDC_CLOSE_ALL_DIR_LIST:		DirectoryListingFrame::closeAll();	break;
	case IDC_CLOSE_ALL_SEARCH_FRAME:	SearchFrame::closeAll();			break;
	}
	return 0;
}

void MainFrame::on(HttpConnectionListener::Data, HttpConnection* /*conn*/, const uint8_t* buf, size_t len) throw() {
	versionInfo += string((const char*)buf, len);
}

void MainFrame::on(PartialList, const User::Ptr& aUser, const string& text) throw() {
	PostMessage(WM_SPEAKER, BROWSE_LISTING, (LPARAM)new DirectoryBrowseInfo(aUser, text));
}

void MainFrame::on(QueueManagerListener::Finished, QueueItem* qi, const string& dir, int64_t speed) throw() {
	if(qi->isSet(QueueItem::FLAG_CLIENT_VIEW)) {
		if(qi->isSet(QueueItem::FLAG_USER_LIST)) {
			// This is a file listing, show it...
			DirectoryListInfo* i = new DirectoryListInfo(qi->getCurrent(), Text::toT(qi->getListName()), Text::toT(dir), speed);

			PostMessage(WM_SPEAKER, DOWNLOAD_LISTING, (LPARAM)i);
		} else if(qi->isSet(QueueItem::FLAG_TEXT)) {
			PostMessage(WM_SPEAKER, VIEW_FILE_AND_DELETE, (LPARAM) new tstring(Text::toT(qi->getTarget())));
		}
	}
}


LRESULT MainFrame::onDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	LogManager::getInstance()->removeListener(this);
	QueueManager::getInstance()->removeListener(this);
	TimerManager::getInstance()->removeListener(this);

	if(trayIcon) {
		updateTray(false);
	}
	bHandled = FALSE;
	return 0;
}
#endif

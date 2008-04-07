/*
 * Copyright (C) 2001-2008 Jacek Sieka, arnetheduck on gmail point com
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

#include "MainWindow.h"
#include "resource.h"

#include "LineDlg.h"
#include "HashProgressDlg.h"
#include "SettingsDialog.h"
#include "TextFrame.h"
#include "SingleInstance.h"
#include "AboutDlg.h"
#include "UPnP.h"
#include "TransferView.h"
#include "HubFrame.h"
#include "PrivateFrame.h"
#include "DirectoryListingFrame.h"
#include "SearchFrame.h"

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
#include <dcpp/ClientManager.h>
#include <dcpp/Download.h>

#include <smartwin/widgets/ToolBar.h>
#include <smartwin/LibraryLoader.h>

MainWindow::MainWindow() :
	WidgetFactory<SmartWin::Window>(0), 
	paned(0), 
	transfers(0), 
	toolbar(0),
	tabs(0), 
	trayIcon(false), 
	maximized(false),
	lastMove(0), 
	c(0), 
	stopperThread(NULL), 
	lastUp(0), 
	lastDown(0), 
	lastTick(GET_TICK()),
	UPnP_TCPConnection(0),
	UPnP_UDPConnection(0)
{
	links.homepage = _T("http://dcplusplus.sourceforge.net/");
	links.downloads = links.homepage + _T("download/");
	links.geoipfile = _T("http://www.maxmind.com/download/geoip/database/GeoIPCountryCSV.zip");
	links.faq = links.homepage + _T("faq/");
	links.help = links.homepage + _T("help/");
	links.discuss = links.homepage + _T("discussion/");
	links.features = links.homepage + _T("featurereq/");
	links.bugs = links.homepage + _T("bugs/");
	links.donate = _T("https://www.paypal.com/cgi-bin/webscr?cmd=_xclick&business=arnetheduck%40gmail%2ecom&item_name=DCPlusPlus&no_shipping=1&return=http%3a%2f%2fdcplusplus%2esf%2enet%2f&cancel_return=http%3a%2f%2fdcplusplus%2esf%2enet%2f&cn=Greeting&tax=0&currency_code=EUR&bn=PP%2dDonationsBF&charset=UTF%2d8");
	
	initWindow();
	initMenu();
	initToolbar();
	initStatusBar();
	initTabs();
	initTransfers();
	initSecond();

	onActivate(std::tr1::bind(&MainWindow::handleActivate, this, _1));
	onSized(std::tr1::bind(&MainWindow::handleSized, this, _1));
	onSpeaker(std::tr1::bind(&MainWindow::handleSpeaker, this, _1, _2));
	onHelp(std::tr1::bind(&WinUtil::help, _1, _2));
	onRaw(std::tr1::bind(&MainWindow::handleTrayIcon, this, _2), SmartWin::Message(WM_APP + 242));
	
	updateStatus();
	layout();

	QueueManager::getInstance()->addListener(this);
	LogManager::getInstance()->addListener(this);

	onClosing(std::tr1::bind(&MainWindow::closing, this));

	onRaw(std::tr1::bind(&MainWindow::handleTrayMessage, this), SmartWin::Message(RegisterWindowMessage(_T("TaskbarCreated"))));
	onRaw(std::tr1::bind(&MainWindow::handleEndSession, this), SmartWin::Message(WM_ENDSESSION));
	onRaw(std::tr1::bind(&MainWindow::handleCopyData, this, _2), SmartWin::Message(WM_COPYDATA));
	onRaw(std::tr1::bind(&MainWindow::handleWhereAreYou, this), SmartWin::Message(SingleInstance::WMU_WHERE_ARE_YOU));
	
	TimerManager::getInstance()->start();

	c = new HttpConnection;
	c->addListener(this);
	c->downloadFile("http://dcplusplus.sourceforge.net/version.xml");

	File::ensureDirectory(SETTING(LOG_DIRECTORY));
	startSocket();

	if(BOOLSETTING(OPEN_SYSTEM_LOG)) postMessage(WM_COMMAND, IDC_SYSTEM_LOG);
	if(BOOLSETTING(OPEN_FAVORITE_USERS)) postMessage(WM_COMMAND, IDC_FAVUSERS);
	if(BOOLSETTING(OPEN_QUEUE)) postMessage(WM_COMMAND, IDC_QUEUE);
	if(BOOLSETTING(OPEN_FINISHED_DOWNLOADS)) postMessage(WM_COMMAND, IDC_FINISHED_DL);
	if(BOOLSETTING(OPEN_WAITING_USERS)) postMessage(WM_COMMAND, IDC_WAITING_USERS);
	if(BOOLSETTING(OPEN_FINISHED_UPLOADS)) postMessage(WM_COMMAND, IDC_FINISHED_UL);
	if(BOOLSETTING(OPEN_SEARCH_SPY)) postMessage(WM_COMMAND, IDC_SEARCH_SPY);
	if(BOOLSETTING(OPEN_NETWORK_STATISTICS)) postMessage(WM_COMMAND, IDC_NET_STATS);
	if(BOOLSETTING(OPEN_NOTEPAD)) postMessage(WM_COMMAND, IDC_NOTEPAD);
	if(BOOLSETTING(OPEN_PUBLIC)) postMessage(WM_COMMAND, IDC_PUBLIC_HUBS);
	if(BOOLSETTING(OPEN_FAVORITE_HUBS)) postMessage(WM_COMMAND, IDC_FAVORITE_HUBS);

	if (!WinUtil::isShift())
		speak(AUTO_CONNECT);
	
	speak(PARSE_COMMAND_LINE);

	if(SETTING(NICK).empty()) {
		WinUtil::help(handle(), IDH_GENERALPAGE);
		postMessage(WM_COMMAND, IDC_SETTINGS);
	}
	
	filterIter = SmartWin::Application::instance().addFilter(std::tr1::bind(&MainWindow::filter, this, _1));	
	accel = SmartWin::AcceleratorPtr(new SmartWin::Accelerator(this, IDR_MAINFRAME));
	
	int cmdShow = SmartWin::Application::instance().getCmdShow();
	::ShowWindow(handle(), ((cmdShow == SW_SHOWDEFAULT) || (cmdShow == SW_SHOWNORMAL)) ? SETTING(MAIN_WINDOW_STATE) : cmdShow);

	if(SmartWin::LibraryLoader::getCommonControlsVersion() < PACK_COMCTL_VERSION(5,80))
		createMessageBox().show(T_("Your version of windows common controls is too old for DC++ to run correctly, and you will most probably experience problems with the user interface. You should download version 5.80 or higher from the DC++ homepage or from Microsoft directly."), _T(APPNAME) _T(" ") _T(VERSIONSTRING), MessageBox::BOX_OK, MessageBox::BOX_ICONEXCLAMATION);
}

void MainWindow::initWindow() {
	// Create main window
	dcdebug("initWindow\n");

	Seed cs(_T(APPNAME) _T(" ") _T(VERSIONSTRING));

	int pos_x = SETTING(MAIN_WINDOW_POS_X);
	int pos_y = SETTING(MAIN_WINDOW_POS_Y);
	int size_x = SETTING(MAIN_WINDOW_SIZE_X);
	int size_y = SETTING(MAIN_WINDOW_SIZE_Y);
	if ( (pos_x != static_cast<int>(CW_USEDEFAULT)) &&(pos_y != static_cast<int>(CW_USEDEFAULT))&&(size_x
	    != static_cast<int>(CW_USEDEFAULT))&&(size_y != static_cast<int>(CW_USEDEFAULT))&&(pos_x > 0&& pos_y > 0)
	    &&(size_x > 10&& size_y > 10)) {
		cs.location = SmartWin::Rectangle(pos_x, pos_y, size_x, size_y);
	}

	cs.exStyle |= WS_EX_APPWINDOW;
	if (ResourceManager::getInstance()->isRTL())
		cs.exStyle |= WS_EX_RTLREADING;

	cs.icon = SmartWin::IconPtr(new SmartWin::Icon(IDR_MAINFRAME));
	cs.background = (HBRUSH)(COLOR_3DFACE + 1);
	create(cs);
	
	setHelpId(IDH_STARTPAGE);

	paned = addChild(WidgetHPaned::Seed(0.7));
}

void MainWindow::initMenu() {
	dcdebug("initMenu\n");

	{
		WidgetMenu::Seed cs = WinUtil::Seeds::menu;
		cs.popup = false;
		mainMenu = createMenu(cs);
	}

	{
		WidgetMenuPtr file = mainMenu->appendPopup(T_("&File"));

		file->appendItem(IDC_QUICK_CONNECT, T_("&Quick Connect ...\tCtrl+Q"), std::tr1::bind(&MainWindow::handleQuickConnect, this), SmartWin::BitmapPtr(new SmartWin::Bitmap(IDB_HUB)));
		file->appendItem(IDC_RECONNECT, T_("&Reconnect\tCtrl+R"), std::tr1::bind(&MainWindow::handleForward, this, _1), SmartWin::BitmapPtr(new SmartWin::Bitmap(IDB_RECONNECT)));
		file->appendItem(IDC_FOLLOW, T_("Follow last redirec&t\tCtrl+T"), std::tr1::bind(&MainWindow::handleForward, this, _1), SmartWin::BitmapPtr(new SmartWin::Bitmap(IDB_FOLLOW)));
		file->appendSeparatorItem();

		file->appendItem(IDC_OPEN_FILE_LIST, T_("Open file list...\tCtrl+L"), std::tr1::bind(&MainWindow::handleOpenFileList, this), SmartWin::BitmapPtr(new SmartWin::Bitmap(IDB_OPEN_FILE_LIST)));
		file->appendItem(IDC_OPEN_OWN_LIST, T_("Open own list"), std::tr1::bind(&MainWindow::handleOpenOwnList, this));
		file->appendItem(IDC_MATCH_ALL, T_("Match downloaded lists"), std::tr1::bind(&MainWindow::handleMatchAll, this));
		file->appendItem(IDC_REFRESH_FILE_LIST, T_("Refresh file list\tCtrl+E"), std::tr1::bind(&MainWindow::handleRefreshFileList, this));
		file->appendItem(IDC_OPEN_DOWNLOADS, T_("Open downloads directory"), std::tr1::bind(&MainWindow::handleOpenDownloadsDir, this));
		file->appendSeparatorItem();

		file->appendItem(IDC_SETTINGS, T_("Settings..."), std::tr1::bind(&MainWindow::handleSettings, this), SmartWin::BitmapPtr(new SmartWin::Bitmap(IDB_SETTINGS)));
		file->appendSeparatorItem();
		file->appendItem(IDC_EXIT, T_("E&xit"), std::tr1::bind(&MainWindow::handleExit, this), SmartWin::BitmapPtr(new SmartWin::Bitmap(IDB_EXIT)));
	}

	{
		WidgetMenuPtr view = mainMenu->appendPopup(T_("&View"));

		view->appendItem(IDC_PUBLIC_HUBS, T_("&Public Hubs\tCtrl+P"), std::tr1::bind(&MainWindow::handleOpenWindow, this, _1), SmartWin::BitmapPtr(new SmartWin::Bitmap(IDB_PUBLIC_HUBS)));
		view->appendItem(IDC_FAVORITE_HUBS, T_("&Favorite Hubs\tCtrl+F"), std::tr1::bind(&MainWindow::handleOpenWindow, this, _1), SmartWin::BitmapPtr(new SmartWin::Bitmap(IDB_FAVORITE_HUBS)));
		view->appendItem(IDC_FAVUSERS, T_("Favorite &Users\tCtrl+U"), std::tr1::bind(&MainWindow::handleOpenWindow, this, _1), SmartWin::BitmapPtr(new SmartWin::Bitmap(IDB_FAVORITE_USERS)));
		view->appendSeparatorItem();
		view->appendItem(IDC_QUEUE, T_("&Download Queue\tCtrl+D"), std::tr1::bind(&MainWindow::handleOpenWindow, this, _1), SmartWin::BitmapPtr(new SmartWin::Bitmap(IDB_DL_QUEUE)));
		view->appendItem(IDC_FINISHED_DL, T_("Finished Downloads"), std::tr1::bind(&MainWindow::handleOpenWindow, this, _1), SmartWin::BitmapPtr(new SmartWin::Bitmap(IDB_FINISHED_DL)));
		view->appendItem(IDC_WAITING_USERS, T_("Waiting Users"), std::tr1::bind(&MainWindow::handleOpenWindow, this, _1), SmartWin::BitmapPtr(new SmartWin::Bitmap(IDB_WAITING_USERS)));
		view->appendItem(IDC_FINISHED_UL, T_("Finished Uploads"), std::tr1::bind(&MainWindow::handleOpenWindow, this, _1), SmartWin::BitmapPtr(new SmartWin::Bitmap(IDB_FINISHED_UL)));
		view->appendSeparatorItem();
		view->appendItem(IDC_SEARCH, T_("&Search\tCtrl+S"), std::tr1::bind(&MainWindow::handleOpenWindow, this, _1), SmartWin::BitmapPtr(new SmartWin::Bitmap(IDB_SEARCH)));
		view->appendItem(IDC_ADL_SEARCH, T_("ADL Search"), std::tr1::bind(&MainWindow::handleOpenWindow, this, _1), SmartWin::BitmapPtr(new SmartWin::Bitmap(IDB_ADL_SEARCH)));
		view->appendItem(IDC_SEARCH_SPY, T_("Search Spy"), std::tr1::bind(&MainWindow::handleOpenWindow, this, _1), SmartWin::BitmapPtr(new SmartWin::Bitmap(IDB_SEARCH_SPY)));
		view->appendSeparatorItem();
		view->appendItem(IDC_NOTEPAD, T_("&Notepad\tCtrl+N"), std::tr1::bind(&MainWindow::handleOpenWindow, this, _1), SmartWin::BitmapPtr(new SmartWin::Bitmap(IDB_NOTEPAD)));
		view->appendItem(IDC_SYSTEM_LOG, T_("System Log"), std::tr1::bind(&MainWindow::handleOpenWindow, this, _1));
		view->appendItem(IDC_NET_STATS, T_("Network Statistics"), std::tr1::bind(&MainWindow::handleOpenWindow, this, _1), SmartWin::BitmapPtr(new SmartWin::Bitmap(IDB_NETWORK_STATS)));
		view->appendItem(IDC_HASH_PROGRESS, T_("Indexing progress"), std::tr1::bind(&MainWindow::handleHashProgress, this));
	}

	{
		WidgetMenuPtr window = mainMenu->appendPopup(T_("&Window"));

		window->appendItem(IDC_CLOSE_ALL_DISCONNECTED, T_("Close disconnected"), std::tr1::bind(&MainWindow::handleCloseWindows, this, _1));
		window->appendItem(IDC_CLOSE_ALL_PM, T_("Close all PM windows"), std::tr1::bind(&MainWindow::handleCloseWindows, this, _1));
		window->appendItem(IDC_CLOSE_ALL_OFFLINE_PM, T_("Close all offline PM windows"), std::tr1::bind(&MainWindow::handleCloseWindows, this, _1));
		window->appendItem(IDC_CLOSE_ALL_DIR_LIST, T_("Close all file list windows"), std::tr1::bind(&MainWindow::handleCloseWindows, this, _1));
		window->appendItem(IDC_CLOSE_ALL_SEARCH_FRAME, T_("Close all search windows"), std::tr1::bind(&MainWindow::handleCloseWindows, this, _1));
	}

	{
		WidgetMenuPtr help = mainMenu->appendPopup(T_("&Help"));

		help->appendItem(IDH_STARTPAGE, T_("Help &Contents\tF1"), std::tr1::bind(&WinUtil::help, handle(), _1), SmartWin::BitmapPtr(new SmartWin::Bitmap(IDB_HELP)));
		help->appendSeparatorItem();
		help->appendItem(IDH_CHANGELOG, T_("Change Log"), std::tr1::bind(&WinUtil::help, handle(), _1));
		help->appendItem(IDC_ABOUT, T_("About DC++..."), std::tr1::bind(&MainWindow::handleAbout, this), SmartWin::BitmapPtr(new SmartWin::Bitmap(IDB_DCPP)));
		help->appendSeparatorItem();
		help->appendItem(IDC_HELP_HOMEPAGE, T_("DC++ Homepage"), std::tr1::bind(&MainWindow::handleLink, this, _1));
		help->appendItem(IDC_HELP_DOWNLOADS, T_("Downloads"), std::tr1::bind(&MainWindow::handleLink, this, _1));
		help->appendItem(IDC_HELP_GEOIPFILE, T_("GeoIP database update"), std::tr1::bind(&MainWindow::handleLink, this, _1));
		help->appendItem(IDC_HELP_FAQ, T_("Frequently asked questions"), std::tr1::bind(&MainWindow::handleLink, this, _1));
		help->appendItem(IDC_HELP_FORUM, T_("Help forum"), std::tr1::bind(&MainWindow::handleLink, this, _1));
		help->appendItem(IDC_HELP_DISCUSS, T_("DC++ discussion forum"), std::tr1::bind(&MainWindow::handleLink, this, _1));
		help->appendItem(IDC_HELP_REQUEST_FEATURE, T_("Request a feature"), std::tr1::bind(&MainWindow::handleLink, this, _1));
		help->appendItem(IDC_HELP_REPORT_BUG, T_("Report a bug"), std::tr1::bind(&MainWindow::handleLink, this, _1));
		help->appendItem(IDC_HELP_DONATE, T_("Donate (paypal)"), std::tr1::bind(&MainWindow::handleLink, this, _1));
	}

	mainMenu->setMenu();
}

void MainWindow::initToolbar() {
	dcdebug("initToolbar\n");

	ToolBar::Seed cs;
	cs.style |= TBSTYLE_FLAT;
	toolbar = addChild(cs);
	{
		SmartWin::ImageListPtr list(new SmartWin::ImageList(20, 20, ILC_COLOR32 | ILC_MASK));
		SmartWin::Bitmap bmp(IDB_TOOLBAR20);
		list->add(bmp, RGB(255, 0, 255));
		
		toolbar->setNormalImageList(list);
	}
	{
		SmartWin::ImageListPtr list(new SmartWin::ImageList(20, 20, ILC_COLOR32 | ILC_MASK));
		SmartWin::Bitmap bmp(IDB_TOOLBAR20_HOT);
		list->add(bmp, RGB(255, 0, 255));
		
		toolbar->setHotImageList(list);
	}
	
	int image = 0;
	toolbar->appendItem(image++, T_("Public Hubs"), std::tr1::bind(&MainWindow::handleOpenWindow, this, IDC_PUBLIC_HUBS));
	toolbar->appendSeparator();
	toolbar->appendItem(image++, T_("Reconnect"), std::tr1::bind(&MainWindow::handleForward, this, IDC_RECONNECT));
	toolbar->appendItem(image++, T_("Follow last redirect"), std::tr1::bind(&MainWindow::handleForward, this, IDC_FOLLOW));
	toolbar->appendSeparator();
	toolbar->appendItem(image++, T_("Favorite Hubs"), std::tr1::bind(&MainWindow::handleOpenWindow, this, IDC_FAVORITE_HUBS));
	toolbar->appendItem(image++, T_("Favorite Users"), std::tr1::bind(&MainWindow::handleOpenWindow, this, IDC_FAVUSERS));
	toolbar->appendSeparator();
	toolbar->appendItem(image++, T_("Download Queue"), std::tr1::bind(&MainWindow::handleOpenWindow, this, IDC_QUEUE));
	toolbar->appendItem(image++, T_("Finished Downloads"), std::tr1::bind(&MainWindow::handleOpenWindow, this, IDC_FINISHED_DL));
	toolbar->appendItem(image++, T_("Waiting Users"), std::tr1::bind(&MainWindow::handleOpenWindow, this, IDC_WAITING_USERS));
	toolbar->appendItem(image++, T_("Finished Uploads"), std::tr1::bind(&MainWindow::handleOpenWindow, this, IDC_FINISHED_UL));
	toolbar->appendSeparator();
	toolbar->appendItem(image++, T_("Search"), std::tr1::bind(&MainWindow::handleOpenWindow, this, IDC_SEARCH));
	toolbar->appendItem(image++, T_("ADL Search"), std::tr1::bind(&MainWindow::handleOpenWindow, this, IDC_ADL_SEARCH));
	toolbar->appendItem(image++, T_("Search Spy"), std::tr1::bind(&MainWindow::handleOpenWindow, this, IDC_SEARCH_SPY));
	toolbar->appendSeparator();
	toolbar->appendItem(image++, T_("Open file list..."), std::tr1::bind(&MainWindow::handleOpenFileList, this));
	toolbar->appendSeparator();
	toolbar->appendItem(image++, T_("Settings"), std::tr1::bind(&MainWindow::handleSettings, this));
	toolbar->appendItem(image++, T_("Notepad"), std::tr1::bind(&MainWindow::handleOpenWindow, this, IDC_NOTEPAD));
	toolbar->appendSeparator();
	toolbar->appendItem(image++, T_("\"What's this?\""), std::tr1::bind(&MainWindow::handleWhatsThis, this));
}

void MainWindow::initStatusBar() {
	dcdebug("initStatusBar\n");
	initStatus(true);
	statusSizes[STATUS_AWAY] = status->getTextSize(T_("AWAY")).x + 12;
	///@todo set to checkbox width + resizedrag width really
	statusSizes[STATUS_DUMMY] = 32;
}

void MainWindow::initTabs() {
	dcdebug("initTabs\n");
	tabs = addChild(SmartWin::TabView::Seed(BOOLSETTING(TOGGLE_ACTIVE_WINDOW)));
	tabs->onTitleChanged(std::tr1::bind(&MainWindow::handleTabsTitleChanged, this, _1));
	tabs->onHelp(std::tr1::bind(&WinUtil::help, _1, _2));
	paned->setFirst(tabs);
}

void MainWindow::initTransfers() {
	dcdebug("initTransfers\n");
	transfers = new TransferView(this, getTabView());
	paned->setSecond(transfers);
}

bool MainWindow::filter(MSG& msg) {
	if(tabs && tabs->filter(msg)) {
		return true;
	}
	
	if(accel && accel->translate(msg)) {
		return true;
	}

	Container* active = getTabView()->getActive();
	if(active) {
		if(::IsDialogMessage( active->handle(), & msg )) {
			return true;
		}
	}

	return false;
}

void MainWindow::handleTabsTitleChanged(const SmartUtil::tstring& title) {
	setText(title.empty() ? _T(APPNAME) _T(" ") _T(VERSIONSTRING) : _T(APPNAME) _T(" ") _T(VERSIONSTRING) _T(" - [") + title + _T("]"));
}

void MainWindow::handleExit() {
	close(true);
}

void MainWindow::handleForward(WPARAM wParam) {
	Container* active = getTabView()->getActive();
	if(active) {
		active->sendMessage(WM_COMMAND, wParam);
	}
}

void MainWindow::handleQuickConnect() {
	if (SETTING(NICK).empty()) {
        postMessage(WM_COMMAND, IDC_SETTINGS);
		return;
	}

	LineDlg dlg(this, T_("Quick Connect"), T_("Address"));

	if (dlg.run() == IDOK) {

		tstring tmp = dlg.getLine();
		// Strip out all the spaces
		string::size_type i;
		while ((i = tmp.find(' ')) != string::npos)
			tmp.erase(i, 1);

		HubFrame::openWindow(getTabView(), Text::fromT(tmp));
	}
}

void MainWindow::handleSized(const SmartWin::SizedEvent& sz) {
	if(sz.isMinimized) {
		if(BOOLSETTING(AUTO_AWAY) && !Util::getManualAway()) {
			Util::setAway(true);
		}
		if(BOOLSETTING(MINIMIZE_TRAY) != WinUtil::isShift()) {
			updateTray(true);
			setVisible(false);
		}
		maximized = isZoomed();
	} else if( sz.isMaximized || sz.isRestored ) {
		if(BOOLSETTING(AUTO_AWAY) && !Util::getManualAway()) {
			Util::setAway(false);
		}
		if(trayIcon) {
			updateTray(false);
		}
		layout();
	}
}

LRESULT MainWindow::handleSpeaker(WPARAM wParam, LPARAM lParam) {
	Speaker s = static_cast<Speaker>(wParam);

	switch (s) {
	case DOWNLOAD_LISTING: {
		boost::scoped_ptr<DirectoryListInfo> i(reinterpret_cast<DirectoryListInfo*>(lParam));
		DirectoryListingFrame::openWindow(getTabView(), i->file, i->dir, i->user, i->speed);
	}
		break;
	case BROWSE_LISTING: {
		boost::scoped_ptr<DirectoryBrowseInfo> i(reinterpret_cast<DirectoryBrowseInfo*>(lParam));
		DirectoryListingFrame::openWindow(getTabView(), i->user, i->text, 0);
	}
		break;
	case AUTO_CONNECT: {
		autoConnect(FavoriteManager::getInstance()->getFavoriteHubs());
	}
		break;
	case PARSE_COMMAND_LINE: {
		parseCommandLine(GetCommandLine());
	}
		break;
	case VIEW_FILE_AND_DELETE: {
		boost::scoped_ptr<std::string> file(reinterpret_cast<std::string*>(lParam));
		new TextFrame(this->getTabView(), *file);
		File::deleteFile(*file);
	}
		break;
	case STATUS_MESSAGE: {
		boost::scoped_ptr<pair<time_t, tstring> > msg(reinterpret_cast<std::pair<time_t, tstring>*>(lParam));
		tstring line = Text::toT("[" + Util::getShortTimeString(msg->first) + "] ") + msg->second;

		setStatus(STATUS_STATUS, line);
		while (lastLinesList.size() + 1> MAX_CLIENT_LINES)
			lastLinesList.erase(lastLinesList.begin());
		if (line.find(_T('\r')) == tstring::npos) {
			lastLinesList.push_back(line);
		} else {
			lastLinesList.push_back(line.substr(0, line.find(_T('\r'))));
		}
	}
		break;
	case LAYOUT: {
		layout();
	}
		break;
	}
	return 0;
}

void MainWindow::autoConnect(const FavoriteHubEntryList& fl) {
	for (FavoriteHubEntryList::const_iterator i = fl.begin(); i != fl.end(); ++i) {
		FavoriteHubEntry* entry = *i;
		if (entry->getConnect()) {
			if (!entry->getNick().empty() || !SETTING(NICK).empty()) {
				HubFrame::openWindow(getTabView(), entry->getServer());
			}
		}
	}
}

void MainWindow::saveWindowSettings() {
	WINDOWPLACEMENT wp = { sizeof(wp)};

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
	if (stopperThread == NULL) {
		if ( !BOOLSETTING(CONFIRM_EXIT) || (createMessageBox().show(T_("Really exit?"), _T(APPNAME) _T(" ") _T(VERSIONSTRING), MessageBox::BOX_YESNO, MessageBox::BOX_ICONQUESTION) == IDYES)) {
			if (c != NULL) {
				c->removeListener(this);
				delete c;
				c = NULL;
			}
			saveWindowSettings();

			setVisible(false);
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
			dcdebug("Starting stopper\n");
			stopperThread = CreateThread(NULL, 0, &stopper, this, 0, &id);
		}
		return false;
	} 

	dcdebug("Waiting for stopper\n");
	// This should end immediately, as it only should be the stopper that sends another WM_CLOSE
	WaitForSingleObject(stopperThread, 60*1000);
	CloseHandle(stopperThread);
	stopperThread = NULL;
	::PostQuitMessage(0);
	dcdebug("Quit message posted\n");
	return true;
}

LRESULT MainWindow::handleTrayMessage() {
	if(BOOLSETTING(MINIMIZE_TRAY) && isIconic())
		updateTray(true);
	return 0;
}

void MainWindow::initSecond() {
	createTimer(std::tr1::bind(&MainWindow::eachSecond, this), 1000);
}

bool MainWindow::eachSecond() {
	updateStatus();
	return true;
}

void MainWindow::layout() {
	SmartWin::Rectangle r(getClientAreaSize());
	
	toolbar->refresh();
	SmartWin::Point pt = toolbar->getSize();
	r.pos.y += pt.y;
	r.size.y -= pt.y;
	
	layoutStatus(r);

	paned->setRect(r);
}

LRESULT MainWindow::handleWhereAreYou() {
	return SingleInstance::WMU_WHERE_ARE_YOU;
}

void MainWindow::updateStatus() {
	uint64_t now= GET_TICK();
	uint64_t tdiff = now - lastTick;
	if (tdiff < 100) {
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

	setStatus(STATUS_AWAY, Util::getAway() ? T_("AWAY") : _T(""));
	setStatus(STATUS_COUNTS, Text::toT(Client::getCounts()));
	setStatus(STATUS_SLOTS, str(TF_("Slots: %1%/%2%") % UploadManager::getInstance()->getFreeSlots() % (SETTING(SLOTS))));
	setStatus(STATUS_DOWN_TOTAL, str(TF_("D: %1%") % Text::toT(Util::formatBytes(down))));
	setStatus(STATUS_UP_TOTAL, str(TF_("U: %1%") % Text::toT(Util::formatBytes(up))));
	setStatus(STATUS_DOWN_DIFF, str(TF_("D: %1%/s (%2%)") % Text::toT(Util::formatBytes((downdiff*1000)/tdiff)) % DownloadManager::getInstance()->getDownloadCount()));
	setStatus(STATUS_UP_DIFF, str(TF_("U: %1%/s (%2%)") % Text::toT(Util::formatBytes((updiff*1000)/tdiff)) % UploadManager::getInstance()->getUploadCount()));
}

MainWindow::~MainWindow() {
	SmartWin::Application::instance().removeFilter(filterIter);
}

void MainWindow::handleSettings() {
	SettingsDialog dlg(this);

	unsigned short lastTCP = static_cast<unsigned short>(SETTING(TCP_PORT));
	unsigned short lastUDP = static_cast<unsigned short>(SETTING(UDP_PORT));
	unsigned short lastTLS = static_cast<unsigned short>(SETTING(TLS_PORT));

	int lastConn= SETTING(INCOMING_CONNECTIONS);

	bool lastSortFavUsersFirst= BOOLSETTING(SORT_FAVUSERS_FIRST);

	if (dlg.run() == IDOK) {
		SettingsManager::getInstance()->save();
		if (SETTING(INCOMING_CONNECTIONS) != lastConn || SETTING(TCP_PORT) != lastTCP || SETTING(UDP_PORT) != lastUDP || SETTING(TLS_PORT) != lastTLS) {
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
}

void MainWindow::startSocket() {
	SearchManager::getInstance()->disconnect();
	ConnectionManager::getInstance()->disconnect();

	if (ClientManager::getInstance()->isActive()) {
		try {
			ConnectionManager::getInstance()->listen();
		} catch(const Exception&) {
			MessageBox().show(T_("Unable to open TCP/TLS port. File transfers will not work correctly until you change settings or turn off any application that might be using the TCP/TLS port"), _T(APPNAME) _T(" ") _T(VERSIONSTRING), MessageBox::BOX_OK, MessageBox::BOX_ICONSTOP);
		}
		try {
			SearchManager::getInstance()->listen();
		} catch(const Exception&) {
			MessageBox().show(T_("Unable to open UDP port. Searching will not work correctly until you change settings or turn off any application that might be using the UDP port"), _T(APPNAME) _T(" ") _T(VERSIONSTRING), MessageBox::BOX_OK, MessageBox::BOX_ICONSTOP);
		}
	}

	startUPnP();
}

void MainWindow::startUPnP() {
	stopUPnP();

	if( SETTING(INCOMING_CONNECTIONS) == SettingsManager::INCOMING_FIREWALL_UPNP ) {
		UPnP_TCPConnection = new UPnP( Util::getLocalIp(), "TCP", APPNAME " Download Port (" + Util::toString(ConnectionManager::getInstance()->getPort()) + " TCP)", ConnectionManager::getInstance()->getPort() );
		UPnP_UDPConnection = new UPnP( Util::getLocalIp(), "UDP", APPNAME " Search Port (" + Util::toString(SearchManager::getInstance()->getPort()) + " UDP)", SearchManager::getInstance()->getPort() );

		if ( FAILED(UPnP_UDPConnection->OpenPorts()) || FAILED(UPnP_TCPConnection->OpenPorts()) )
		{
			LogManager::getInstance()->message(_("Failed to create port mappings. Please set up your NAT yourself."));
			createMessageBox().show(T_("Failed to create port mappings. Please set up your NAT yourself."), _T(APPNAME) _T(" ") _T(VERSIONSTRING));

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
					LogManager::getInstance()->message(_("Failed to get external IP via  UPnP. Please set it yourself."));
					createMessageBox().show(T_("Failed to get external IP via  UPnP. Please set it yourself."), _T(APPNAME) _T(" ") _T(VERSIONSTRING));
				}
			}
		}
	}
}

void MainWindow::stopUPnP() {
	// Just check if the port mapping objects are initialized (NOT NULL)
	if ( UPnP_TCPConnection != NULL )
	{
		if (FAILED(UPnP_TCPConnection->ClosePorts()) )
		{
			LogManager::getInstance()->message(_("Failed to remove port mappings"));
		}
		delete UPnP_TCPConnection;
	}
	if ( UPnP_UDPConnection != NULL )
	{
		if (FAILED(UPnP_UDPConnection->ClosePorts()) )
		{
			LogManager::getInstance()->message(_("Failed to remove port mappings"));
		}
		delete UPnP_UDPConnection;
	}
	// Not sure this is required (i.e. Objects are checked later in execution)
	// But its better being on the save side :P
	UPnP_TCPConnection = UPnP_UDPConnection = NULL;
}

void MainWindow::handleOpenFileList() {
	tstring file;
	if(WinUtil::browseFileList(createLoadDialog(), file)) {
		UserPtr u = DirectoryListing::getUserFromFilename(Text::fromT(file));
		if (u) {
			DirectoryListingFrame::openWindow(getTabView(), file, Text::toT(Util::emptyString), u, 0);
		} else {
			createMessageBox().show(T_("Invalid file list name"), _T(APPNAME) _T(" ") _T(VERSIONSTRING));
		}
	}
}

void MainWindow::handleOpenOwnList() {
	if (!ShareManager::getInstance()->getOwnListFile().empty()) {
		DirectoryListingFrame::openWindow(getTabView(), Text::toT(ShareManager::getInstance()->getOwnListFile()), Text::toT(Util::emptyString), ClientManager::getInstance()->getMe(), 0);
	}
}

void MainWindow::updateTray(bool add /* = true */) {
	if (add) {
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

	while( (wnd=::GetWindow(mf->getTabView()->getTab()->handle(), GW_CHILD)) != NULL) {
		if(wnd == wnd2) {
			::Sleep(100);
		} else {
			::PostMessage(wnd, WM_CLOSE, 0, 0);
			wnd2 = wnd;
		}
	}

	::PostMessage(mf->handle(), WM_CLOSE, 0, 0);
	return 0;
}

class ListMatcher : public Thread {
public:
	ListMatcher(StringList files_) :
		files(files_) {

	}
	virtual int run() {
		for (StringIter i = files.begin(); i != files.end(); ++i) {
			UserPtr u = DirectoryListing::getUserFromFilename(*i);
			if (!u)
				continue;
			DirectoryListing dl(u);
			try {
				dl.loadFile(*i);
				LogManager::getInstance()->message(str(FN_("%1%: matched %2% file", "%1%: matched %2% files", QueueManager::getInstance()->matchListing(dl)) 
				% Util::toString(ClientManager::getInstance()->getNicks(u->getCID()))
				% QueueManager::getInstance()->matchListing(dl)));
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

void MainWindow::handleActivate(bool active) {
	// Forward to active tab window
	Container* w = getTabView()->getActive();
	if(w) {
		w->sendMessage(WM_ACTIVATE, active ? WA_ACTIVE : WA_INACTIVE);
	}
}

void MainWindow::parseCommandLine(const tstring& cmdLine)
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

LRESULT MainWindow::handleCopyData(LPARAM lParam) {
	parseCommandLine(Text::toT(WinUtil::getAppName() + " ") + reinterpret_cast<LPCTSTR>(reinterpret_cast<COPYDATASTRUCT*>(lParam)->lpData));
	return TRUE;
}

void MainWindow::handleHashProgress() {
	HashProgressDlg(this, false).run();
}

void MainWindow::handleAbout() {
	AboutDlg(this).run();
}

void MainWindow::handleOpenDownloadsDir() {
	WinUtil::openFile(Text::toT(SETTING(DOWNLOAD_DIRECTORY)));
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
				if(xml.findChild("Title")) {
					const string& title = xml.getChildData();
					xml.resetCurrentChild();
					if(xml.findChild("Message")) {
						if(url.empty()) {
							const string& msg = xml.getChildData();
							createMessageBox().show(Text::toT(msg), Text::toT(title));
						} else {
							if(createMessageBox().show(str(TF_("%1%\nOpen download page?") % Text::toT(xml.getChildData())), Text::toT(title), MessageBox::BOX_YESNO, MessageBox::BOX_ICONQUESTION) == IDYES) {
								WinUtil::openLink(Text::toT(url));
							}
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
		xml.stepOut();
	} catch (const Exception&) {
		// ...
	}
}

LRESULT MainWindow::handleEndSession() {
	if (c != NULL) {
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
	switch (id) {
	case IDC_HELP_HOMEPAGE:
		site = links.homepage;
		break;
	case IDC_HELP_DOWNLOADS:
		site = links.downloads;
		break;
	case IDC_HELP_GEOIPFILE:
		site = links.geoipfile;
		break;
	case IDC_HELP_FAQ:
		site = links.faq;
		break;
	case IDC_HELP_FORUM:
		site = links.help;
		break;
	case IDC_HELP_DISCUSS:
		site = links.discuss;
		break;
	case IDC_HELP_REQUEST_FEATURE:
		site = links.features;
		break;
	case IDC_HELP_REPORT_BUG:
		site = links.bugs;
		break;
	case IDC_HELP_DONATE:
		site = links.donate;
		break;
	default:
		dcassert(0);
	}

	WinUtil::openLink(site);
}

void MainWindow::handleRefreshFileList() {
	ShareManager::getInstance()->setDirty();
	ShareManager::getInstance()->refresh(true);
}

void MainWindow::handleRestore() {
	setVisible(true);
	if(maximized) {
		maximize();
	} else {
		restore();
	}
}

bool MainWindow::tryFire(const MSG& msg, LRESULT& retVal) {
	bool handled = SmartWin::Window::tryFire(msg, retVal);
	if(!handled && msg.message == WM_COMMAND && tabs) {
		handled = tabs->tryFire(msg, retVal);
	}
	return handled;
}

LRESULT MainWindow::handleTrayIcon(LPARAM lParam)
{
	if (lParam == WM_LBUTTONUP) {
		handleRestore();
	} else if(lParam == WM_RBUTTONDOWN || lParam == WM_CONTEXTMENU) {
		SmartWin::ScreenCoordinate pt;
		WidgetMenuPtr trayMenu = createMenu(WinUtil::Seeds::menu);
		trayMenu->appendItem(IDC_TRAY_SHOW, T_("Show"), std::tr1::bind(&MainWindow::handleRestore, this));
		trayMenu->appendItem(IDC_TRAY_QUIT, T_("Exit"), std::tr1::bind(&MainWindow::close, this, true));
		trayMenu->appendItem(IDC_OPEN_DOWNLOADS, T_("Open downloads directory"));
		trayMenu->appendItem(IDC_SETTINGS, T_("Settings..."));
		trayMenu->setDefaultItem(0,TRUE);
		::GetCursorPos(&pt.getPoint());
		::SetForegroundWindow(handle());
		trayMenu->trackPopupMenu(pt, TPM_BOTTOMALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON);
		postMessage(WM_NULL);
	} else if(lParam == WM_MOUSEMOVE && ((lastMove + 1000) < GET_TICK()) ) {
		NOTIFYICONDATA nid;
		nid.cbSize = sizeof(NOTIFYICONDATA);
		nid.hWnd = handle();
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

#ifdef PORT_ME
LRESULT MainFrame::OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	static BOOL bVisible = TRUE; // initially visible
	bVisible = !bVisible;
	CReBarCtrl rebar = m_hWndToolBar;
	int nBandIndex = rebar.IdToIndex(ATL_IDW_BAND_FIRST + 1); // toolbar is 2nd added band
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
	switch (id) {
	case IDC_CLOSE_ALL_DISCONNECTED:
		HubFrame::closeDisconnected();
		break;
	case IDC_CLOSE_ALL_PM:
		PrivateFrame::closeAll();
		break;
	case IDC_CLOSE_ALL_OFFLINE_PM:
		PrivateFrame::closeAllOffline();
		break;
	case IDC_CLOSE_ALL_DIR_LIST:
		DirectoryListingFrame::closeAll();
		break;
	case IDC_CLOSE_ALL_SEARCH_FRAME:
		SearchFrame::closeAll();
		break;
	}
}

void MainWindow::handleWhatsThis() {
	sendMessage(WM_SYSCOMMAND, SC_CONTEXTHELP);
}

void MainWindow::on(HttpConnectionListener::Data, HttpConnection* /*conn*/, const uint8_t* buf, size_t len) throw() {
	versionInfo += string((const char*)buf, len);
}

void MainWindow::on(PartialList, const UserPtr& aUser, const string& text) throw() {
	speak(BROWSE_LISTING, (LPARAM)new DirectoryBrowseInfo(aUser, text));
}

void MainWindow::on(QueueManagerListener::Finished, QueueItem* qi, const string& dir, int64_t speed) throw() {
	if (qi->isSet(QueueItem::FLAG_CLIENT_VIEW)) {
		if (qi->isSet(QueueItem::FLAG_USER_LIST)) {
			// This is a file listing, show it...
			DirectoryListInfo* i = new DirectoryListInfo(qi->getDownloads()[0]->getUser(), Text::toT(qi->getListName()), Text::toT(dir), speed);

			speak(DOWNLOAD_LISTING, (LPARAM)i);
		} else if (qi->isSet(QueueItem::FLAG_TEXT)) {
			speak(VIEW_FILE_AND_DELETE, reinterpret_cast<LPARAM>(new std::string(qi->getTarget())));
		}
	}
}

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

#ifndef DCPLUSPLUS_WIN32_MAIN_WINDOW_H
#define DCPLUSPLUS_WIN32_MAIN_WINDOW_H

#include <dcpp/forward.h>

#include <dcpp/QueueManagerListener.h>
#include <dcpp/LogManager.h>
#include <dcpp/HttpConnection.h>
#include <dcpp/User.h>

#include "WidgetFactory.h"
#include "AspectStatus.h"
#include "AspectSpeaker.h"

class UPnP;
class TransferView;

class MainWindow : 
	public WidgetFactory<SmartWin::WidgetWindow>, 
	public AspectSpeaker<MainWindow>,
	private HttpConnectionListener, 
	private QueueManagerListener, 
	private LogManagerListener,
	public AspectStatus<MainWindow>
{
public:
	enum Status {
		STATUS_STATUS,
		STATUS_AWAY,
		STATUS_COUNTS,
		STATUS_SLOTS,
		STATUS_DOWN_TOTAL,
		STATUS_UP_TOTAL,
		STATUS_DOWN_DIFF,
		STATUS_UP_DIFF,
		STATUS_DUMMY,
		STATUS_LAST
	};

	/// @deprecated
	WidgetTabView* getMDIParent() { return tabs; }

	virtual bool tryFire( const MSG & msg, LRESULT & retVal );

	MainWindow();
	
	virtual ~MainWindow();
private:
	
	class DirectoryListInfo {
	public:
		DirectoryListInfo(const UserPtr& aUser, const tstring& aFile, const tstring& aDir, int64_t aSpeed) : user(aUser), file(aFile), dir(aDir), speed(aSpeed) { }
		UserPtr user;
		tstring file;
		tstring dir;
		int64_t speed;
	};
	class DirectoryBrowseInfo {
	public:
		DirectoryBrowseInfo(const UserPtr& ptr, string aText) : user(ptr), text(aText) { }
		UserPtr user;
		string text;
	};

	enum Speaker {
		DOWNLOAD_LISTING,
		BROWSE_LISTING,
		AUTO_CONNECT,
		PARSE_COMMAND_LINE,
		VIEW_FILE_AND_DELETE,
		STATUS_MESSAGE,
		LAYOUT
	};

	struct {
		tstring homepage;
		tstring downloads;
		tstring geoipfile;
		tstring faq;
		tstring help;
		tstring discuss;
		tstring features;
		tstring bugs;
		tstring donate;
	} links;

	WidgetHPanedPtr paned;
	WidgetMenuPtr mainMenu;
	TransferView* transfers;
	WidgetToolbarPtr toolbar;
	WidgetTabViewPtr tabs;
	
	/** Is the tray icon visible? */
	bool trayIcon;
	/** Was the window maximized when minimizing it? */
	bool maximized;
	uint32_t lastMove;

	HttpConnection* c;
	string versionInfo;

	HANDLE stopperThread;

	int64_t lastUp;
	int64_t lastDown;
	uint64_t lastTick;

	SmartWin::Application::FilterIter filterIter;
	SmartWin::AcceleratorPtr accel;

	enum { MAX_CLIENT_LINES = 10 };
	TStringList lastLinesList;
	tstring lastLines;

	// UPnP connectors
	UPnP* UPnP_TCPConnection;
	UPnP* UPnP_UDPConnection;

	void initWindow();
	void initMenu();
	void initToolbar();
	void initStatusBar();
	void initTabs();
	void initTransfers();
	void initSecond();
	
	// User actions
	void handleExit();
	void handleOpenWindow(unsigned id);
	void handleQuickConnect();
	void handleSettings();
	void handleOpenFileList();
	void handleOpenOwnList();
	void handleRefreshFileList();
	void handleMatchAll();
	void handleOpenDownloadsDir();
	void handleLink(unsigned id);
	void handleAbout();
	void handleMenuHelp(unsigned id);
	void handleHashProgress();
	void handleCloseWindows(unsigned id);
	void handleSize();
	void handleActivate(bool active);
	LRESULT handleHelp(WPARAM wParam, LPARAM lParam);
	LRESULT handleEndSession(WPARAM wParam, LPARAM lParam);
	LRESULT handleTrayIcon(WPARAM wParam, LPARAM lParam);
	
	// Other events
	bool handleSized(const SmartWin::WidgetSizedEventResult& sz);
	
	LRESULT handleSpeaker(WPARAM wParam, LPARAM lParam);
	LRESULT handleTrayMessage();
	LRESULT handleCopyData(WPARAM wParam, LPARAM lParam);
	LRESULT handleWhereAreYou(WPARAM wParam, LPARAM lParam);

	void handleTabsTitleChanged(const SmartUtil::tstring& title);
	
	void layout();
	bool eachSecond();
	void updateStatus();
	void updateTray(bool add = true);
	void autoConnect(const FavoriteHubEntryList& fl);
	void startSocket();
	void startUPnP();
	void stopUPnP();
	void saveWindowSettings();
	void parseCommandLine(const tstring& cmdLine);
	bool filter(MSG& msg);
	
	bool closing();
	void handleRestore();
	
	static DWORD WINAPI stopper(void* p);

	// LogManagerListener
	virtual void on(LogManagerListener::Message, time_t t, const string& m) throw() { speak(STATUS_MESSAGE, (LPARAM)new pair<time_t, tstring>(t, tstring(Text::toT(m)))); }

	// HttpConnectionListener
	virtual void on(HttpConnectionListener::Complete, HttpConnection* conn, string const& /*aLine*/) throw();
	virtual void on(HttpConnectionListener::Data, HttpConnection* /*conn*/, const uint8_t* buf, size_t len) throw();

	// QueueManagerListener
	virtual void on(QueueManagerListener::Finished, QueueItem* qi, const string& dir, int64_t speed) throw();
	virtual void on(PartialList, const UserPtr&, const string& text) throw();

};

#endif // !defined(MAIN_FRM_H)

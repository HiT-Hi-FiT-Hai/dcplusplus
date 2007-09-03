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

#ifndef DCPLUSPLUS_WIN32_MAIN_WINDOW_H
#define DCPLUSPLUS_WIN32_MAIN_WINDOW_H

#include <dcpp/forward.h>

#include <dcpp/QueueManagerListener.h>
#include <dcpp/LogManager.h>
#include <dcpp/HttpConnection.h>

#include "TransferView.h"
#include "WidgetFactory.h"
#include "AspectStatus.h"
#include "AspectSpeaker.h"
#include "MDITab.h"

class UPnP;

class MainWindow : 
	public WidgetFactory<SmartWin::WidgetMDIFrame>, 
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
		tstring translations;
		tstring faq;
		tstring help;
		tstring discuss;
		tstring features;
		tstring bugs;
	} links;

	WidgetHPanedPtr paned;
	WidgetMenuPtr mainMenu;
	TransferView* transfers;
	WidgetToolbarPtr toolbar;
	MDITab* tabs;
	
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

	static MainWindow* instance;
	
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
	void handleMDIReorder(unsigned id);
	void handleMenuHelp(unsigned id);
	void handleHashProgress();
	void handleCloseWindows(unsigned id);
	void handleMinimizeAll();
	void handleRestoreAll();
	void handleSize();
	LRESULT handleHelp(WPARAM wParam, LPARAM lParam);
	LRESULT handleEndSession(WPARAM wParam, LPARAM lParam);
	bool handleTabResize(const SmartWin::WidgetSizedEventResult& sz);
	LRESULT handleTrayIcon(WPARAM wParam, LPARAM lParam);
	
	// Other events
	bool handleSized(const SmartWin::WidgetSizedEventResult& sz);
	
	LRESULT handleSpeaker(WPARAM wParam, LPARAM lParam);
	LRESULT trayMessage(WPARAM wParam, LPARAM lParam);
	LRESULT handleCopyData(WPARAM wParam, LPARAM lParam);
	
	void layout();
	bool eachSecond();
	void updateStatus();
	void updateTray(bool add = true);
	void autoConnect(const FavoriteHubEntryList& fl);
	void startSocket();
	void startUPnP();
	void stopUPnP();
	void saveWindowSettings();
	void resizeMDIClient();
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

#ifdef PORT_ME

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		if((pMsg->message >= WM_MOUSEFIRST) && (pMsg->message <= WM_MOUSELAST))
			ctrlLastLines.RelayEvent(pMsg);

		if(CMDIFrameWindowImpl<MainFrame>::PreTranslateMessage(pMsg))
			return TRUE;

		HWND hWnd = MDIGetActive();
		if(hWnd != NULL)
			return (BOOL)::SendMessage(hWnd, WM_FORWARDMSG, 0, (LPARAM)pMsg);

		return FALSE;
	}

	BEGIN_MSG_MAP(MainFrame)
		MESSAGE_HANDLER(WMU_WHERE_ARE_YOU, onWhereAreYou)
		NOTIFY_CODE_HANDLER(TTN_GETDISPINFO, onGetToolTip)
		CHAIN_MDI_CHILD_COMMANDS()
	END_MSG_MAP()

	BEGIN_UPDATE_UI_MAP(MainFrame)
		UPDATE_ELEMENT(ID_VIEW_TOOLBAR, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_VIEW_STATUS_BAR, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_VIEW_TRANSFER_VIEW, UPDUI_MENUPOPUP)
	END_UPDATE_UI_MAP()


	LRESULT OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnViewTransferView(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onCloseWindows(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onServerSocket(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	LRESULT onWhereAreYou(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		return WMU_WHERE_ARE_YOU;
	}

	LRESULT onTrayQuit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		PostMessage(WM_CLOSE);
		return 0;
	}

	LRESULT onTrayShow(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		ShowWindow(SW_SHOW);
		ShowWindow(maximized ? SW_MAXIMIZE : SW_RESTORE);
		return 0;
	}

#endif

};

#endif // !defined(MAIN_FRM_H)

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

#if !defined(AFX_MAINFRM_H__E73C3806_489F_4918_B986_23DCFBD603D5__INCLUDED_)
#define AFX_MAINFRM_H__E73C3806_489F_4918_B986_23DCFBD603D5__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "../client/ConnectionManager.h"
#include "../client/DownloadManager.h"
#include "../client/UploadManager.h"
#include "../client/TimerManager.h"
#include "../client/CriticalSection.h"
#include "../client/HttpConnection.h"
#include "../client/HubManager.h"

#include "FlatTabCtrl.h"
#include "ExListViewCtrl.h"

class MainFrame : public CMDIFrameWindowImpl<MainFrame>, public CUpdateUI<MainFrame>,
		public CMessageFilter, public CIdleHandler, public DownloadManagerListener, public CSplitterImpl<MainFrame, false>,
		private TimerManagerListener, private UploadManagerListener, private HttpConnectionListener, private HubManagerListener,
		private ConnectionManagerListener
{
public:
	MainFrame() : trayIcon(false), lastUpload(-1), stopperThread(NULL) { 
	};
	virtual ~MainFrame();
	DECLARE_FRAME_WND_CLASS("DC++", IDR_MAINFRAME)

	CMDICommandBarCtrl m_CmdBar;

	enum {
		ADD_UPLOAD_ITEM,
		ADD_DOWNLOAD_ITEM,
		REMOVE_ITEM,
		SET_TEXT,
		SET_TEXTS,
		DOWNLOAD_LISTING,
		STATS,
		AUTO_CONNECT
	};

	enum {
		IMAGE_DOWNLOAD = 0,
		IMAGE_UPLOAD
	};
	
	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		if(CMDIFrameWindowImpl<MainFrame>::PreTranslateMessage(pMsg))
			return TRUE;
		
		HWND hWnd = MDIGetActive();
		if(hWnd != NULL)
			return (BOOL)::SendMessage(hWnd, WM_FORWARDMSG, 0, (LPARAM)pMsg);
		
		return FALSE;
	}
	
	virtual BOOL OnIdle()
	{
		UIUpdateToolBar();
		return FALSE;
	}
	typedef CSplitterImpl<MainFrame, false> splitterBase;
	BEGIN_MSG_MAP(MainFrame)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_SPEAKER, onSpeaker)
		MESSAGE_HANDLER(FTN_SELECTED, onSelected)
		MESSAGE_HANDLER(WM_CONTEXTMENU, onContextMenu)
		MESSAGE_HANDLER(WM_APP+242, onTrayIcon)
		MESSAGE_HANDLER(WM_DESTROY, onDestroy)
		MESSAGE_HANDLER(WM_SIZE, onSize)
		COMMAND_ID_HANDLER(ID_APP_EXIT, OnFileExit)
		COMMAND_ID_HANDLER(ID_FILE_CONNECT, OnFileConnect)
		COMMAND_ID_HANDLER(ID_FILE_SETTINGS, OnFileSettings)
		COMMAND_ID_HANDLER(ID_FILE_SEARCH, OnFileSearch)
		COMMAND_ID_HANDLER(ID_VIEW_TOOLBAR, OnViewToolBar)
		COMMAND_ID_HANDLER(ID_VIEW_STATUS_BAR, OnViewStatusBar)
		COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
		COMMAND_ID_HANDLER(ID_WINDOW_CASCADE, OnWindowCascade)
		COMMAND_ID_HANDLER(ID_WINDOW_TILE_HORZ, OnWindowTile)
		COMMAND_ID_HANDLER(ID_WINDOW_ARRANGE, OnWindowArrangeIcons)
		COMMAND_ID_HANDLER(IDC_FAVORITES, onFavorites)
		COMMAND_ID_HANDLER(IDC_FAVUSERS, onFavoriteUsers)
		COMMAND_ID_HANDLER(IDC_REMOVE, onRemove)
		COMMAND_ID_HANDLER(IDC_NOTEPAD, onNotepad)
		COMMAND_ID_HANDLER(IDC_QUEUE, onQueue)
		COMMAND_ID_HANDLER(IDC_PRIVATEMESSAGE, onPrivateMessage)
		COMMAND_ID_HANDLER(IDC_GETLIST, onGetList)
		COMMAND_ID_HANDLER(IDC_FORCE, onForce)
		COMMAND_ID_HANDLER(IDC_SEARCH_SPY, onSearchSpy)
		COMMAND_ID_HANDLER(IDC_HELP_HOMEPAGE, onLink)
		COMMAND_ID_HANDLER(IDC_HELP_DOWNLOADS, onLink)
		COMMAND_ID_HANDLER(IDC_HELP_FAQ, onLink)
		COMMAND_ID_HANDLER(IDC_HELP_HELP_FORUM, onLink)
		COMMAND_ID_HANDLER(IDC_HELP_DISCUSS, onLink)
		COMMAND_ID_HANDLER(IDC_HELP_REQUEST_FEATURE, onLink)
		COMMAND_ID_HANDLER(IDC_HELP_REPORT_BUG, onLink)
		COMMAND_ID_HANDLER(IDC_IMPORT_QUEUE, onImport)
		CHAIN_MDI_CHILD_COMMANDS()
		NOTIFY_HANDLER(IDC_TRANSFERS, LVN_KEYDOWN, onKeyDownTransfers)
		NOTIFY_HANDLER(IDC_TRANSFERS, LVN_COLUMNCLICK, onColumnClick)
		CHAIN_MSG_MAP(CUpdateUI<MainFrame>)
		CHAIN_MSG_MAP(CMDIFrameWindowImpl<MainFrame>)
		CHAIN_MSG_MAP(splitterBase);
	END_MSG_MAP()

	BEGIN_UPDATE_UI_MAP(MainFrame)
		UPDATE_ELEMENT(ID_VIEW_TOOLBAR, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_VIEW_STATUS_BAR, UPDUI_MENUPOPUP)
	END_UPDATE_UI_MAP()

	LRESULT onSize(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onSpeaker(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onNotepad(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onQueue(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onFavorites(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onFavoriteUsers(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onGetList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);	
	LRESULT onPrivateMessage(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);			
	LRESULT onForce(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);			
	LRESULT onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnFileConnect(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onSearchSpy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnFileSettings(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnFileSearch(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onLink(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onImport(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		
	static DWORD WINAPI stopper(void* p);

	LRESULT onColumnClick(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
		NMLISTVIEW* l = (NMLISTVIEW*)pnmh;
		if(l->iSubItem == ctrlTransfers.getSortColumn()) {
			ctrlTransfers.setSortDirection(!ctrlTransfers.getSortDirection());
		} else {
			if(l->iSubItem == COLUMN_SIZE) {
				ctrlTransfers.setSort(l->iSubItem, ExListViewCtrl::SORT_FLOAT);
			} else {
				ctrlTransfers.setSort(l->iSubItem, ExListViewCtrl::SORT_STRING_NOCASE);
			}
		}
		return 0;
	}

	LRESULT onSelected(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		::ShowWindow((HWND)wParam, SW_RESTORE);
		MDIActivate((HWND)wParam);
		SendMessage(m_hWndMDIClient, WM_MDIACTIVATE, wParam, 0);
		return 0;
	}
	
	LRESULT onTrayIcon(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
	{
		if (lParam == WM_LBUTTONUP)
		{
			ShowWindow(SW_SHOW);
			ShowWindow(SW_RESTORE);
		}
		return 0;
	}
	
	LRESULT onDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if(trayIcon) {
			NOTIFYICONDATA nid;
			nid.cbSize = sizeof(NOTIFYICONDATA);
			nid.hWnd = m_hWnd;
			nid.uID = 0;
			nid.uFlags = 0;
			::Shell_NotifyIcon(NIM_DELETE, &nid);
		}
		bHandled = FALSE;
		return 0;
	}
	
	LRESULT onKeyDownTransfers(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
		NMLVKEYDOWN* kd = (NMLVKEYDOWN*) pnmh;

		if(kd->wVKey == VK_DELETE) {
			removeSelected();
		}
		return 0;
	}

	LRESULT onRemove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		removeSelected();
		return 0;
	}
	
	void removeSelected() {
		int i = -1;
		while( (i = ctrlTransfers.GetNextItem(i, LVNI_SELECTED)) != -1) {
			ConnectionManager::getInstance()->removeConnection((ConnectionQueueItem*)ctrlTransfers.GetItemData(i));
		}
	}
	
	
	LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		return 0;
	}

	LRESULT OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		PostMessage(WM_CLOSE);
		return 0;
	}
	
	/**
	 * This is called from CMDIFrameWindowImpl, and is a copy of what I found in CFrameWindowImplBase,
	 * plus a few additions of my own...
	 */
	void UpdateLayout(BOOL bResizeBars = TRUE)
	{
		RECT rect;
		GetClientRect(&rect);
		// position bars and offset their dimensions
		UpdateBarsPosition(rect, bResizeBars);

		if(ctrlStatus.IsWindow()) {
			CRect sr;
			int w[6];
			ctrlStatus.GetClientRect(sr);
			int tmp = (sr.Width()) > 516 ? 416 : ((sr.Width() > 116) ? sr.Width()-100 : 16);
			
			w[0] = sr.right - tmp;
			w[1] = w[0] + (tmp-16)*1/5;
			w[2] = w[0] + (tmp-16)*2/5;
			w[3] = w[0] + (tmp-16)*3/5;
			w[4] = w[0] + (tmp-16)*4/5;
			w[5] = w[0] + (tmp-16)*5/5;
			
			ctrlStatus.SetParts(6, w);
		}
		CRect rc = rect;
		rc.top = rc.bottom - ctrlTab.getHeight();
		if(ctrlTab.IsWindow())
			ctrlTab.MoveWindow(rc);
		
		CRect rc2 = rect;
		rc2.bottom = rc.top;
		SetSplitterRect(rc2);
	}
	
	LRESULT OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
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

	LRESULT OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		BOOL bVisible = !::IsWindowVisible(m_hWndStatusBar);
		::ShowWindow(m_hWndStatusBar, bVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
		UISetCheck(ID_VIEW_STATUS_BAR, bVisible);
		UpdateLayout();
		return 0;
	}


	LRESULT OnWindowCascade(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		MDICascade();
		return 0;
	}

	LRESULT OnWindowTile(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		MDITile();
		return 0;
	}

	LRESULT OnWindowArrangeIcons(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		MDIIconArrange();
		return 0;
	}

private:
	enum {
		COLUMN_FIRST,
		COLUMN_USER = COLUMN_FIRST,
		COLUMN_STATUS,
		COLUMN_FILE,
		COLUMN_SIZE,
		COLUMN_LAST
	};

	class StringInfo {
	public:
		StringInfo(LPARAM lp = NULL, const string& s = Util::emptyString) : lParam(lp), str(s) { };
		string str;
		LPARAM lParam;
	};

	class StringListInfo;
	friend class StringListInfo;

	class StringListInfo {
	public:
		StringListInfo(LPARAM lp = NULL) : lParam(lp) { };
		LPARAM lParam;
		string columns[COLUMN_LAST];
	};

	class DirectoryListInfo {
	public:
		DirectoryListInfo(LPARAM lp = NULL) : lParam(lp) { };
		User::Ptr user;
		string file;
		
		LPARAM lParam;
		
	};
	CriticalSection cs;
	ExListViewCtrl ctrlTransfers;
	CStatusBarCtrl ctrlStatus;
	FlatTabCtrl ctrlTab;
	HttpConnection c;
	string versionInfo;
	CImageList images;
	CImageList largeImages;
	
	CMenu transferMenu;

	bool trayIcon;

	int lastUpload;
	static int columnIndexes[COLUMN_LAST];
	static int columnSizes[COLUMN_LAST];
	
	CImageList arrows;
	HANDLE stopperThread;

	HWND createToolbar();
	void buildMenu();

	MainFrame(const MainFrame&) { dcassert(0); };
	// UploadManagerListener
	virtual void onAction(UploadManagerListener::Types type, Upload* aUpload) {
		switch(type) {
		case UploadManagerListener::COMPLETE:
			onUploadComplete(aUpload); break;
		case UploadManagerListener::STARTING:
			onUploadStarting(aUpload); break;
		default:
			dcassert(0);
		}
	}
	virtual void onAction(UploadManagerListener::Types type, const Upload::List ul) {
		switch(type) {	
		case UploadManagerListener::TICK: onUploadTick(ul); break;
		}
	}
	
	void onUploadStarting(Upload* aUpload);
	void onUploadTick(const Upload::List aUpload);
	void onUploadComplete(Upload* aUpload);
	
	// DownloadManagerListener
	virtual void onAction(DownloadManagerListener::Types type, Download* aDownload) {
		switch(type) {
		case DownloadManagerListener::COMPLETE: onDownloadComplete(aDownload); break;
		case DownloadManagerListener::STARTING: onDownloadStarting(aDownload); break;
		default: dcassert(0); break;
		}
	}
	virtual void onAction(DownloadManagerListener::Types type, const Download::List& dl) {
		switch(type) {	
		case DownloadManagerListener::TICK: onDownloadTick(dl); break;
		}
	}
	virtual void onAction(DownloadManagerListener::Types type, Download* aDownload, const string& aReason) {
		switch(type) {
		case DownloadManagerListener::FAILED: onDownloadFailed(aDownload, aReason); break;
		default: dcassert(0); break;
		}
	}

	void onDownloadComplete(Download* aDownload);
	void onDownloadFailed(Download* aDownload, const string& aReason);
	void onDownloadStarting(Download* aDownload);
	void onDownloadTick(const Download::List aDownload);

	// ConnectionManagerListener
	virtual void onAction(ConnectionManagerListener::Types type, ConnectionQueueItem* aCqi) { 
		switch(type) {
		case ConnectionManagerListener::ADDED: onConnectionAdded(aCqi); break;
		case ConnectionManagerListener::CONNECTED: onConnectionConnected(aCqi); break;
		case ConnectionManagerListener::REMOVED: onConnectionRemoved(aCqi); break;
		case ConnectionManagerListener::STATUS_CHANGED: onConnectionStatus(aCqi); break;
		default: dcassert(0); break;
		}
	};

	virtual void onAction(ConnectionManagerListener::Types type, ConnectionQueueItem* aCqi, const string& aLine) { 
		switch(type) {
		case ConnectionManagerListener::FAILED: onConnectionFailed(aCqi, aLine); break;
		default: dcassert(0); break;
		}
	}
	
	void onConnectionAdded(ConnectionQueueItem* aCqi);
	void onConnectionConnected(ConnectionQueueItem* /*aCqi*/) { };
	void onConnectionFailed(ConnectionQueueItem* aCqi, const string& aReason);
	void onConnectionRemoved(ConnectionQueueItem* aCqi);
	void onConnectionStatus(ConnectionQueueItem* aCqi);

	// TimerManagerListener
	virtual void onAction(TimerManagerListener::Types type, DWORD /*aTick*/) {
		if(type == TimerManagerListener::SECOND) {
			StringList* str = new StringList();
			str->push_back("Slots: " + Util::toString(UploadManager::getInstance()->getRunning()) + '/' + Util::toString(SETTING(SLOTS)));
			str->push_back("D: " + Util::formatBytes(Socket::getTotalDown()));
			str->push_back("U: " + Util::formatBytes(Socket::getTotalUp()));
			str->push_back("D: " + Util::formatBytes(Socket::getDown()) + "/s");
			str->push_back("U: " + Util::formatBytes(Socket::getUp()) + "/s");
			PostMessage(WM_SPEAKER, STATS, (LPARAM)str);
			Socket::resetStats();
		}
	}
	
	// HttpConnectionListener
	virtual void onAction(HttpConnectionListener::Types type, HttpConnection* conn) {
		switch(type) {
		case HttpConnectionListener::COMPLETE:
			onHttpComplete(conn); break;
		}
	}
	virtual void onAction(HttpConnectionListener::Types type, HttpConnection* conn, const BYTE* buf, int len) {
		switch(type) {
		case HttpConnectionListener::DATA:
			onHttpData(conn, buf, len); break;
		}
	}
	
	void onHttpComplete(HttpConnection* aConn);
	void onHttpData(HttpConnection* aConn, const BYTE* aBuf, int aLen);

	// HubManagerListener
	virtual void onAction(HubManagerListener::Types type, const FavoriteHubEntry::List& fl);
	
};

#endif // !defined(AFX_MAINFRM_H__E73C3806_489F_4918_B986_23DCFBD603D5__INCLUDED_)

/**
 * @file MainFrm.h
 * $Id: MainFrm.h,v 1.2 2002/04/13 12:57:23 arnetheduck Exp $
 */

 
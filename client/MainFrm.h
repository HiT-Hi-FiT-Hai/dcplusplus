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

#if !defined(AFX_MAINFRM_H__E73C3806_489F_4918_B986_23DCFBD603D5__INCLUDED_)
#define AFX_MAINFRM_H__E73C3806_489F_4918_B986_23DCFBD603D5__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "DownloadManager.h"
#include "UploadManager.h"
#include "ExListViewCtrl.h"
#include "TimerManager.h"
#include "CriticalSection.h"
#include "FlatTabCtrl.h"
#include "HttpConnection.h"

class MainFrame : public CMDIFrameWindowImpl<MainFrame>, public CUpdateUI<MainFrame>,
		public CMessageFilter, public CIdleHandler, public DownloadManagerListener, public CSplitterImpl<MainFrame, false>,
		private TimerManagerListener, private UploadManagerListener, private HttpConnectionListener
{
public:
	MainFrame() : stopperThread(NULL) { };
	virtual ~MainFrame();
	DECLARE_FRAME_WND_CLASS("DC++", IDR_MAINFRAME)

	CCommandBarCtrl2 m_CmdBar;

	enum {
		UPLOAD_COMPLETE,
		UPLOAD_FAILED,
		UPLOAD_STARTING,
		UPLOAD_TICK,
		DOWNLOAD_ADDED,
		DOWNLOAD_COMPLETE,
		DOWNLOAD_CONNECTING,
		DOWNLOAD_FAILED,
		DOWNLOAD_REMOVED,
		DOWNLOAD_STARTING,
		DOWNLOAD_SOURCEADDED,
		DOWNLOAD_TICK,
		DOWNLOAD_LISTING
	};

	enum {
		IMAGE_DOWNLOAD = 0,
		IMAGE_UPLOAD
	};
	
	enum {
		IMAGE_USER = 0,
		IMAGE_OP
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
		COMMAND_ID_HANDLER(IDC_REMOVE, onRemove)
		COMMAND_RANGE_HANDLER(IDC_TRANSFERITEM, (IDC_TRANSFERITEM + menuItems), onTransferItem)
		NOTIFY_HANDLER(IDC_TRANSFERS, LVN_KEYDOWN, onKeyDownTransfers)
		CHAIN_MDI_CHILD_COMMANDS()
		CHAIN_MSG_MAP(CUpdateUI<MainFrame>)
		CHAIN_MSG_MAP(CMDIFrameWindowImpl<MainFrame>)
		CHAIN_MSG_MAP(splitterBase);
	END_MSG_MAP()

	BEGIN_UPDATE_UI_MAP(MainFrame)
		UPDATE_ELEMENT(ID_VIEW_TOOLBAR, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_VIEW_STATUS_BAR, UPDUI_MENUPOPUP)
	END_UPDATE_UI_MAP()

	LRESULT onSelected(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		SendMessage(m_hWndMDIClient, WM_MDIACTIVATE, wParam, 0);
		return 0;
	}
	
	static DWORD WINAPI stopper(void* p);

	LRESULT onKeyDownTransfers(int idCtrl, LPNMHDR pnmh, BOOL& bHandled) {
		NMLVKEYDOWN* kd = (NMLVKEYDOWN*) pnmh;

		if(kd->wVKey == VK_DELETE) {
			removeSelected();
		}
		return 0;
	}

	LRESULT onTransferItem(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onRemove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		removeSelected();
		return 0;
	}
	
	void removeSelected() {
		LVITEM item;
		item.iItem = -1;
		item.iSubItem = 0;
		item.mask = LVIF_PARAM | LVIF_IMAGE;
		cs.enter();
		while( (item.iItem = ctrlTransfers.GetNextItem(item.iItem, LVNI_SELECTED)) != -1) {
			ctrlTransfers.GetItem(&item);
			
			if(item.iImage == IMAGE_DOWNLOAD)
				DownloadManager::getInstance()->removeDownload((Download*)item.lParam);
			else
				UploadManager::getInstance()->removeUpload((Upload*)item.lParam);
		}
		cs.leave();
	}

	LRESULT onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled) {
		RECT rc;                    // client area of window 
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };        // location of mouse click 
		
		// Get the bounding rectangle of the client area. 
		ctrlTransfers.GetClientRect(&rc);
		ctrlTransfers.ScreenToClient(&pt); 
		cs.enter();
		if (PtInRect(&rc, pt)) 
		{ 
			// Remove all old items
			while(transferMenu.GetMenuItemCount() > 0) {
				transferMenu.DeleteMenu(0, MF_BYPOSITION);
			}

			int n = 0;
			CMenuItemInfo mi;
			
			mi.fMask = MIIM_ID | MIIM_STRING;
			mi.dwTypeData = "Remove Transfer";
			mi.wID = IDC_REMOVE;
			transferMenu.InsertMenuItem(n++, TRUE, &mi);

			if(ctrlTransfers.GetSelectedCount() == 1) {

				mi.fMask = MIIM_TYPE;
				mi.fType = MFT_SEPARATOR;
				transferMenu.InsertMenuItem(n++, TRUE, &mi);
				
				
				LVITEM lvi;
				lvi.iItem = ctrlTransfers.GetNextItem(-1, LVNI_SELECTED);
				lvi.iSubItem = 0;
				lvi.mask = LVIF_IMAGE | LVIF_PARAM;

				ctrlTransfers.GetItem(&lvi);

				if(lvi.iImage == IMAGE_DOWNLOAD) {
					Download* d = (Download*)lvi.lParam;
					menuItems = 0;
					for(Download::Source::Iter i = d->getSources().begin(); i != d->getSources().end(); ++i) {
						string str = "Browse " + (*i)->getNick() + "'s files";
						mi.fMask = MIIM_ID | MIIM_STRING | MIIM_DATA;
						mi.dwTypeData = (LPSTR)str.c_str();
						mi.dwItemData = (DWORD)*i;
						mi.wID = IDC_TRANSFERITEM + menuItems++;
						transferMenu.InsertMenuItem(n++, TRUE, &mi);

						str = "Remove " + (*i)->getNick() + " from this transfer";
						mi.fMask = MIIM_ID | MIIM_STRING | MIIM_DATA;
						mi.dwTypeData = (LPSTR)str.c_str();
						mi.dwItemData = (DWORD)*i;
						mi.wID = IDC_TRANSFERITEM + menuItems++;
						transferMenu.InsertMenuItem(n++, TRUE, &mi);
						
						str = "Send Message To " + (*i)->getNick();
						mi.fMask = MIIM_ID | MIIM_STRING | MIIM_DATA;
						mi.dwTypeData = (LPSTR)str.c_str();
						mi.dwItemData = (DWORD)*i;
						mi.wID = IDC_TRANSFERITEM + menuItems++;
						transferMenu.InsertMenuItem(n++, TRUE, &mi);
					}
				}
			}
			
			
			
			ctrlTransfers.ClientToScreen(&pt);

			transferMenu.TrackPopupMenuEx(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
			
			return TRUE; 
		}
		cs.leave();
		return FALSE; 
	}
	
	LRESULT OnClose(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled) {
		DWORD id;
		if(stopperThread) {
			if(WaitForSingleObject(stopperThread, 0) == WAIT_TIMEOUT) {
				// Hm, the thread's not finished stopping the client yet...post a close message and continue processing...
				PostMessage(WM_CLOSE);
				return 0;
			}
			CloseHandle(stopperThread);
			stopperThread = NULL;
			bHandled = FALSE;
		} else {
			stopperThread = CreateThread(NULL, 0, stopper, this, 0, &id);
		}
		return 0;
	}
	
	LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
		return 0;
	}

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
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
			int w[5];
			ctrlStatus.GetClientRect(sr);
			int tmp = (sr.Width()) > 416 ? 316 : ((sr.Width() > 116) ? sr.Width()-100 : 16);
			
			w[0] = sr.right - tmp;
			w[1] = w[0] + (tmp-16)/4;
			w[2] = w[0] + (tmp-16)*2/4;
			w[3] = w[0] + (tmp-16)*3/4;
			w[4] = w[0] + (tmp-16)*4/4;
			
			ctrlStatus.SetParts(5, w);
		}
		CRect rc = rect;
		rc.top = rc.bottom - ctrlTab.getHeight();
		if(ctrlTab.IsWindow())
			ctrlTab.MoveWindow(rc);
		
		CRect rc2 = rect;
		rc2.bottom = rc.top;
		SetSplitterRect(rc2);
	}
	
	LRESULT OnFileConnect(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnFileSettings(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnFileSearch(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	
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

	LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	LRESULT OnWindowCascade(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		MDICascade();
		return 0;
	}

	LRESULT OnWindowTile(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		MDITile();
		return 0;
	}

	LRESULT OnWindowArrangeIcons(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		MDIIconArrange();
		return 0;
	}
private:

	enum {
		IDC_TRANSFERITEM = 3000
	};
	int menuItems;
	
	class StringInfo {
	public:
		StringInfo(LPARAM lp = NULL, const string& s = "") : lParam(lp), str(s) { };
		string str;
		LPARAM lParam;
	};

	class StringListInfo {
	public:
		StringListInfo(LPARAM lp = NULL) : lParam(lp) { };
		StringList l;
		LPARAM lParam;
	};

	CriticalSection cs;
	ExListViewCtrl ctrlTransfers;
	CStatusBarCtrl ctrlStatus;
	FlatTabCtrl ctrlTab;
	HttpConnection c;
	string versionInfo;
	
	CMenu transferMenu;
	
	CImageList arrows;
	HANDLE stopperThread;

	LRESULT onSpeaker(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	// UploadManagerListener
	virtual void onUploadComplete(Upload* aUpload) { PostMessage(WM_SPEAKER, UPLOAD_COMPLETE, (LPARAM)aUpload); };
	virtual void onUploadFailed(Upload* aUpload, const string& aReason) { PostMessage(WM_SPEAKER, UPLOAD_FAILED, (LPARAM)aUpload); };
	virtual void onUploadStarting(Upload* aUpload);
	virtual void onUploadTick(Upload* aUpload);
	
	// DownloadManagerListener
	virtual void onDownloadAdded(Download* aDownload);
	virtual void onDownloadComplete(Download* aDownload);
	virtual void onDownloadConnecting(Download* aDownload) { PostMessage(WM_SPEAKER, DOWNLOAD_CONNECTING, (LPARAM) aDownload); };
	virtual void onDownloadFailed(Download* aDownload, const string& aReason);
	virtual void onDownloadRemoved(Download* aDownload);
	virtual void onDownloadSourceAdded(Download* aDownload, Download::Source* aSource);
	virtual void onDownloadSourceRemoved(Download* aDownload, Download::Source* aSource);
	virtual void onDownloadStarting(Download* aDownload);
	virtual void onDownloadTick(Download* aDownload);
	
	// TimerManagerListener
	virtual void onTimerSecond(DWORD aTick) {
		if(ctrlStatus.IsWindow()) {
			char buf[128];
			sprintf(buf, "D: %s", Util::formatBytes(Socket::getTotalDown()).c_str());
			ctrlStatus.SetText(1, buf);
			sprintf(buf, "U: %s", Util::formatBytes(Socket::getTotalUp()).c_str());
			ctrlStatus.SetText(2, buf);
			sprintf(buf, "D: %s/s", Util::formatBytes(Socket::getDown()).c_str());
			ctrlStatus.SetText(3, buf);
			sprintf(buf, "U: %s/s", Util::formatBytes(Socket::getUp()).c_str());
			ctrlStatus.SetText(4, buf);
		}
		Socket::resetStats();
	}
	
	// HttpConnectionListener
	virtual void onHttpComplete(HttpConnection* aConn);
	virtual void onHttpData(HttpConnection* aConn, const BYTE* aBuf, int aLen);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__E73C3806_489F_4918_B986_23DCFBD603D5__INCLUDED_)

/**
 * @file MainFrm.h
 * $Id: MainFrm.h,v 1.23 2002/01/05 18:32:42 arnetheduck Exp $
 * @if LOG
 * $Log: MainFrm.h,v $
 * Revision 1.23  2002/01/05 18:32:42  arnetheduck
 * Added two new icons, fixed some bugs, and updated some other things
 *
 * Revision 1.22  2002/01/05 10:13:39  arnetheduck
 * Automatic version detection and some other updates
 *
 * Revision 1.21  2002/01/02 16:12:32  arnetheduck
 * Added code for multiple download sources
 *
 * Revision 1.20  2001/12/30 17:41:16  arnetheduck
 * Fixed some XML parsing bugs
 *
 * Revision 1.19  2001/12/29 13:47:14  arnetheduck
 * Fixing bugs and UI work
 *
 * Revision 1.18  2001/12/27 18:14:36  arnetheduck
 * Version 0.08, here we go...
 *
 * Revision 1.17  2001/12/27 12:05:00  arnetheduck
 * Added flat tabs, fixed sorting and a StringTokenizer bug
 *
 * Revision 1.16  2001/12/21 20:21:17  arnetheduck
 * Private messaging added, and a lot of other updates as well...
 *
 * Revision 1.15  2001/12/18 12:32:18  arnetheduck
 * Stability fixes
 *
 * Revision 1.14  2001/12/16 19:47:48  arnetheduck
 * Reworked downloading and user handling some, and changed some small UI things
 *
 * Revision 1.13  2001/12/15 17:01:06  arnetheduck
 * Passive mode searching as well as some searching code added
 *
 * Revision 1.12  2001/12/13 19:21:57  arnetheduck
 * A lot of work done almost everywhere, mainly towards a friendlier UI
 * and less bugs...time to release 0.06...
 *
 * Revision 1.11  2001/12/11 01:10:29  arnetheduck
 * More bugfixes...I really have to change the bufferedsocket so that it only
 * uses one thread...or maybe even multiple sockets/thread...
 *
 * Revision 1.10  2001/12/08 20:59:26  arnetheduck
 * Fixing bugs...
 *
 * Revision 1.9  2001/12/07 20:03:15  arnetheduck
 * More work done towards application stability
 *
 * Revision 1.8  2001/12/04 21:50:34  arnetheduck
 * Work done towards application stability...still a lot to do though...
 * a bit more and it's time for a new release.
 *
 * Revision 1.7  2001/12/02 23:47:35  arnetheduck
 * Added the framework for uploading and file sharing...although there's something strange about
 * the file lists...my client takes them, but not the original...
 *
 * Revision 1.6  2001/12/02 11:16:46  arnetheduck
 * Optimised hub listing, removed a few bugs and leaks, and added a few small
 * things...downloads are now working, time to start writing the sharing
 * code...
 *
 * Revision 1.5  2001/11/29 19:10:55  arnetheduck
 * Refactored down/uploading and some other things completely.
 * Also added download indicators and download resuming, along
 * with some other stuff.
 *
 * Revision 1.4  2001/11/26 23:40:36  arnetheduck
 * Downloads!! Now downloads are possible, although the implementation is
 * likely to change in the future...more UI work (splitters...) and some bug
 * fixes. Only user file listings are downloadable, but at least it's something...
 *
 * Revision 1.3  2001/11/25 22:06:25  arnetheduck
 * Finally downloading is working! There are now a few quirks and bugs to be fixed
 * but what the heck....!
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


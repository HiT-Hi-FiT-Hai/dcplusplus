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

#if !defined(AFX_SEARCHFRM_H__A7078724_FD85_4F39_8463_5A08A5F45E33__INCLUDED_)
#define AFX_SEARCHFRM_H__A7078724_FD85_4F39_8463_5A08A5F45E33__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "FlatTabCtrl.h"
#include "ExListViewCtrl.h"
#include "WinUtil.h"

#include "../client/Client.h"
#include "../client/SearchManager.h"
#include "../client/CriticalSection.h"
#include "../client/ClientManagerListener.h"

#include "UCHandler.h"

#define SEARCH_MESSAGE_MAP 6		// This could be any number, really...
#define SHOWUI_MESSAGE_MAP 7

class SearchFrame : public MDITabChildWindowImpl<SearchFrame, RGB(127, 127, 255)>, 
	private SearchManagerListener, private ClientListener, private ClientManagerListener, public UCHandler<SearchFrame>
{
public:
	static void openWindow(const string& str = Util::emptyString, LONGLONG size = 0, SearchManager::SizeModes mode = SearchManager::SIZE_ATLEAST, SearchManager::TypeModes type = SearchManager::TYPE_ANY);

	DECLARE_FRAME_WND_CLASS_EX("SearchFrame", IDR_SEARCH, 0, COLOR_3DFACE)

	typedef MDITabChildWindowImpl<SearchFrame, RGB(127, 127, 255)> baseClass;
	typedef UCHandler<SearchFrame> ucBase;

	BEGIN_MSG_MAP(SearchFrame)
		MESSAGE_HANDLER(WM_CREATE, onCreate)
		MESSAGE_HANDLER(WM_SETFOCUS, onFocus)
		MESSAGE_HANDLER(WM_CONTEXTMENU, onContextMenu)
		MESSAGE_HANDLER(WM_SPEAKER, onSpeaker)
		MESSAGE_HANDLER(WM_CTLCOLOREDIT, onCtlColor)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, onCtlColor)
		MESSAGE_HANDLER(WM_CTLCOLORLISTBOX, onCtlColor)
		MESSAGE_HANDLER(WM_CLOSE, onClose)
		NOTIFY_HANDLER(IDC_RESULTS, NM_DBLCLK, onDoubleClickResults)
		NOTIFY_HANDLER(IDC_RESULTS, LVN_COLUMNCLICK, onColumnClickResults)
		NOTIFY_HANDLER(IDC_RESULTS, LVN_KEYDOWN, onKeyDown)
		NOTIFY_HANDLER(IDC_HUB, LVN_ITEMCHANGED, onItemChangedHub)
		COMMAND_ID_HANDLER(IDC_DOWNLOAD, onDownload)
		COMMAND_ID_HANDLER(IDC_DOWNLOADTO, onDownloadTo)
		COMMAND_ID_HANDLER(IDC_DOWNLOADDIR, onDownloadWhole)
		COMMAND_ID_HANDLER(IDC_DOWNLOADDIRTO, onDownloadWholeTo)
		COMMAND_ID_HANDLER(IDC_VIEW_AS_TEXT, onViewAsText)
		COMMAND_ID_HANDLER(IDC_GETLIST, onGetList)
		COMMAND_ID_HANDLER(IDC_MATCH_QUEUE, onMatchQueue)
		COMMAND_ID_HANDLER(IDC_PRIVATEMESSAGE, onPrivateMessage)
		COMMAND_ID_HANDLER(IDC_ADD_TO_FAVORITES, onAddToFavorites)
		COMMAND_ID_HANDLER(IDC_REMOVE, onRemove)
		COMMAND_ID_HANDLER(IDC_SEARCH, onSearch)
		COMMAND_ID_HANDLER(IDC_FREESLOTS, onFreeSlots)
		COMMAND_RANGE_HANDLER(IDC_DOWNLOAD_TARGET, IDC_DOWNLOAD_TARGET + targets.size() + WinUtil::lastDirs.size(), onDownloadTarget)
		COMMAND_RANGE_HANDLER(IDC_DOWNLOAD_WHOLE_TARGET, IDC_DOWNLOAD_WHOLE_TARGET + WinUtil::lastDirs.size(), onDownloadWholeTarget)
		CHAIN_COMMANDS(ucBase)
		CHAIN_MSG_MAP(baseClass)
	ALT_MSG_MAP(SEARCH_MESSAGE_MAP)
		MESSAGE_HANDLER(WM_CHAR, onChar)
		MESSAGE_HANDLER(WM_KEYDOWN, onChar)
		MESSAGE_HANDLER(WM_KEYUP, onChar)
	ALT_MSG_MAP(SHOWUI_MESSAGE_MAP)
		MESSAGE_HANDLER(BM_SETCHECK, onShowUI)
	END_MSG_MAP()

	SearchFrame() : 
	searchBoxContainer("COMBOBOX", this, SEARCH_MESSAGE_MAP),
		searchContainer("edit", this, SEARCH_MESSAGE_MAP), 
		sizeContainer("edit", this, SEARCH_MESSAGE_MAP), 
		modeContainer("COMBOBOX", this, SEARCH_MESSAGE_MAP),
		sizeModeContainer("COMBOBOX", this, SEARCH_MESSAGE_MAP),
		fileTypeContainer("COMBOBOX", this, SEARCH_MESSAGE_MAP),
		showUIContainer("BUTTON", this, SHOWUI_MESSAGE_MAP),
		slotsContainer("BUTTON", this, SEARCH_MESSAGE_MAP),
		doSearchContainer("BUTTON", this, SEARCH_MESSAGE_MAP),
		resultsContainer(WC_LISTVIEW, this, SEARCH_MESSAGE_MAP),
		hubsContainer(WC_LISTVIEW, this, SEARCH_MESSAGE_MAP),
		lastSearch(0), initialSize(0), initialMode(SearchManager::SIZE_ATLEAST), initialType(SearchManager::TYPE_ANY),
		showUI(true), onlyFree(false), closed(false)
	{	
		SearchManager::getInstance()->addListener(this);
	}

	virtual ~SearchFrame() {
	}
	virtual void OnFinalMessage(HWND /*hWnd*/) { delete this; }

	LRESULT onChar(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onClose(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT onColumnClickResults(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);
	LRESULT onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled);
	LRESULT onCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onCtlColor(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onDoubleClickResults(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT onDownloadTarget(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onDownloadTo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onDownloadWholeTarget(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onDownloadWholeTo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onGetList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onMatchQueue(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onPrivateMessage(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onAddToFavorites(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onSpeaker(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	
	void UpdateLayout(BOOL bResizeBars = TRUE);
	void runUserCommand(UserCommand& uc);

	static int sortSize(LPARAM a, LPARAM b) {
		SearchResult* c = (SearchResult*)a;
		SearchResult* d = (SearchResult*)b;
		return compare(c->getSize(), d->getSize());
	}
	static int sortSlots(LPARAM a, LPARAM b) {
		SearchResult* c = (SearchResult*)a;
		SearchResult* d = (SearchResult*)b;
		if(c->getFreeSlots() == d->getFreeSlots())
			return compare(c->getSlots(), d->getSlots());
		else
			return compare(c->getFreeSlots(), d->getFreeSlots());
	}

	void removeSelected() {
		int i = -1;
		while( (i = ctrlResults.GetNextItem(-1, LVNI_SELECTED)) != -1) {
			((SearchResult*)ctrlResults.GetItemData(i))->decRef();
			ctrlResults.DeleteItem(i);
		}
	}
	
	LRESULT onDownload(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		downloadSelected(SETTING(DOWNLOAD_DIRECTORY));
		return 0;
	}

	LRESULT onViewAsText(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		downloadSelected(Util::getTempPath(), true);
		return 0;
	}

	LRESULT onDownloadWhole(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		downloadWholeSelected(SETTING(DOWNLOAD_DIRECTORY));
		return 0;
	}
	
	LRESULT onRemove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		removeSelected();
		return 0;
	}

	LRESULT onFreeSlots(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		onlyFree = (ctrlSlots.GetCheck() == 1);
		return 0;
	}

	LRESULT onSearch(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		onEnter();
		return 0;
	}

	LRESULT onKeyDown(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
		NMLVKEYDOWN* kd = (NMLVKEYDOWN*) pnmh;
		
		if(kd->wVKey == VK_DELETE) {
			removeSelected();
		} 
		return 0;
	}

	LRESULT onFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		if(::IsWindow(ctrlSearch))
			ctrlSearch.SetFocus();
		return 0;
	}

	LRESULT onShowUI(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled) {
		bHandled = FALSE;
		showUI = (wParam == BST_CHECKED);
		UpdateLayout(FALSE);
		return 0;
	}

	void SearchFrame::setInitial(const string& str, LONGLONG size, SearchManager::SizeModes mode, SearchManager::TypeModes type) {
		initialString = str; initialSize = size; initialMode = mode; initialType = type;
	}
	
private:
	enum {
		COLUMN_FIRST,
		COLUMN_NICK = COLUMN_FIRST,
		COLUMN_FILENAME,
		COLUMN_TYPE,
		COLUMN_SIZE,
		COLUMN_PATH,
		COLUMN_SLOTS,
		COLUMN_CONNECTION,
		COLUMN_HUB,
		COLUMN_EXACT_SIZE,
		COLUMN_LAST
	};

	enum Images {
		IMAGE_UNKOWN,
		IMAGE_SLOW,
		IMAGE_NORMAL,
		IMAGE_FAST
	};

	enum {
		IDC_DOWNLOAD_TARGET = 5000,
		IDC_DOWNLOAD_WHOLE_TARGET = 5500,
		IDC_USER_COMMAND = 6000
	};

	struct HubInfo {
		Client* client;
		string name;
		bool op;
	};

	// WM_SPEAKER
	enum Speakers {
		HUB_ADDED,
		HUB_CHANGED,
		HUB_REMOVED,

		SPEAKER_MESSAGE_COUNT
	};

	string initialString;
	int64_t initialSize;
	SearchManager::SizeModes initialMode;
	SearchManager::TypeModes initialType;

	CStatusBarCtrl ctrlStatus;
	CEdit ctrlSearch;
	CComboBox ctrlSearchBox;
	CEdit ctrlSize;
	CComboBox ctrlMode;
	CComboBox ctrlSizeMode;
	CComboBox ctrlFiletype;
	CButton ctrlDoSearch;
	
	CContainedWindow searchContainer;
	CContainedWindow searchBoxContainer;
	CContainedWindow sizeContainer;
	CContainedWindow modeContainer;
	CContainedWindow sizeModeContainer;
	CContainedWindow fileTypeContainer;
	CContainedWindow slotsContainer;
	CContainedWindow showUIContainer;
	CContainedWindow doSearchContainer;
	CContainedWindow resultsContainer;
	CContainedWindow hubsContainer;
	
	CStatic searchLabel, sizeLabel, optionLabel, typeLabel, hubsLabel;
	CButton ctrlSlots, ctrlShowUI;
	bool showUI;

	ExListViewCtrl ctrlResults, ctrlHubs;

	CMenu resultsMenu;
	CMenu targetMenu;
	CMenu targetDirMenu;
	
	StringList search;
	StringList targets;
	StringList wholeTargets;

	/** Parameter map for user commands */
	StringMap ucParams;

	bool onlyFree;

	static StringList lastSearches;

	DWORD lastSearch;
	bool closed;

	static int columnIndexes[];
	static int columnSizes[];

	CriticalSection cs;
	CriticalSection csHub;

	void downloadSelected(const string& aDir, bool view = false); 
	void downloadWholeSelected(const string& aDir);
	void onEnter();
	void onTab(bool shift);

	void download(SearchResult* aSR, const string& aDir, bool view);
	
	// SearchManagerListener
	virtual void onAction(SearchManagerListener::Types type, SearchResult* sr) throw() {
		switch(type) {
		case SearchManagerListener::SEARCH_RESULT:
			onSearchResult(sr); break;
		}
	}
	
	void onSearchResult(SearchResult* aResult);

	// ClientListener
	virtual void onAction(ClientListener::Types type, Client* client) throw() {
		switch(type) {
		case ClientListener::CONNECTED:
			speak(HUB_ADDED, client); break;
		case ClientListener::HUB_NAME:
			speak(HUB_CHANGED, client); break;
		default:
			break;
		}
	}

	virtual void onAction(ClientListener::Types type, Client* client, const string& /*line*/) throw() {
		switch(type) {
			case ClientListener::FAILED:
				speak(HUB_REMOVED, client); break;
			default:
				break;
		}
	}

	virtual void onAction(ClientListener::Types type, Client* client, const User::List& /* aList */) throw() {
		switch(type) {
		case ClientListener::OP_LIST:
			speak(HUB_CHANGED, client); break;
		default:
			break;
		}
	}

	//virtual void onAction(ClientListener::Types type, Client* /*client*/, const User::Ptr& user) throw() {}
	//virtual void onAction(ClientListener::Types type, Client* /*client*/, const User::Ptr& user, const string&  line) throw() {}

	// ClientManagerListener
	virtual void onAction(ClientManagerListener::Types type, Client* client) throw() {
		switch(type) {
			case ClientManagerListener::CLIENT_ADDED:
				client->addListener(this); break;
			case ClientManagerListener::CLIENT_REMOVED:
				speak(HUB_REMOVED, client); break;
			default:
				break;
		}
	}

	//virtual void onAction(ClientManagerListener::Types, const User::Ptr&) throw(;
	//virtual void onAction(ClientManagerListener::Types, const string&) throw();

	void initHubs();
	void onHubAdded(HubInfo* info);
	void onHubChanged(HubInfo* info);
	void onHubRemoved(HubInfo* info);

	LRESULT onItemChangedHub(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	void speak(Speakers s, Client* aClient) {
		HubInfo* hubInfo = new HubInfo;
		hubInfo->client = aClient;
		hubInfo->name = aClient->getName();
		hubInfo->op = aClient->getOp();
		PostMessage(WM_SPEAKER, WPARAM(s), LPARAM(hubInfo)); 
	};
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHILDFRM_H__A7078724_FD85_4F39_8463_5A08A5F45E33__INCLUDED_)

/**
 * @file
 * $Id: SearchFrm.h,v 1.26 2003/11/12 01:17:12 arnetheduck Exp $
 */


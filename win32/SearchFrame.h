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

#ifndef DCPLUSPLUS_WIN32_SEARCH_FRAME_H
#define DCPLUSPLUS_WIN32_SEARCH_FRAME_H

#include "MDIChildFrame.h"
#include "TypedListViewCtrl.h"

#include <client/SearchManager.h>
#include <client/ClientManagerListener.h>

#include "UserInfoBase.h"

class SearchFrame : 
	public MDIChildFrame<SearchFrame>, 
	private SearchManagerListener, 
	private ClientManagerListener,
	public AspectUserInfo<SearchFrame>
{
public:
	enum Status {
		STATUS_SHOW_UI,
		STATUS_STATUS,
		STATUS_COUNT,
		STATUS_FILTERED,
		STATUS_DUMMY,
		STATUS_LAST
	};

	static void openWindow(SmartWin::Widget* mdiParent, const tstring& str = Util::emptyStringT, LONGLONG size = 0, SearchManager::SizeModes mode = SearchManager::SIZE_ATLEAST, SearchManager::TypeModes type = SearchManager::TYPE_ANY);
	static void closeAll();

protected:
	friend class MDIChildFrame<SearchFrame>;
	friend class AspectUserInfo<SearchFrame>;
	void layout();

	bool preClosing();
	void postClosing();

private:
	enum Speakers {
		SPEAK_ADD_RESULT,
		SPEAK_FILTER_RESULT,
		SPEAK_HUB_ADDED,
		SPEAK_HUB_CHANGED,
		SPEAK_HUB_REMOVED
	};

	enum {
		COLUMN_FIRST,
		COLUMN_FILENAME = COLUMN_FIRST,
		COLUMN_NICK,
		COLUMN_TYPE,
		COLUMN_SIZE,
		COLUMN_PATH,
		COLUMN_SLOTS,
		COLUMN_CONNECTION,
		COLUMN_HUB,
		COLUMN_EXACT_SIZE,
		COLUMN_IP,
		COLUMN_TTH,
		COLUMN_CID,
		COLUMN_LAST
	};

	static int columnIndexes[COLUMN_LAST];
	static int columnSizes[COLUMN_LAST];

	class SearchInfo : public UserInfoBase {
	public:
		SearchInfo(SearchResult* aSR) : UserInfoBase(aSR->getUser()), sr(aSR) {
			sr->incRef(); update();
		}
		~SearchInfo() {
			sr->decRef();
		}

		void getList();
		void browseList();

		void view();
		struct Download {
			Download(const tstring& aTarget) : tgt(aTarget) { }
			void operator()(SearchInfo* si);
			const tstring& tgt;
		};
		struct DownloadWhole {
			DownloadWhole(const tstring& aTarget) : tgt(aTarget) { }
			void operator()(SearchInfo* si);
			const tstring& tgt;
		};
		struct DownloadTarget {
			DownloadTarget(const tstring& aTarget) : tgt(aTarget) { }
			void operator()(SearchInfo* si);
			const tstring& tgt;
		};
		struct CheckTTH {
			CheckTTH() : firstHubs(true), op(true), hasTTH(false), firstTTH(true) { }
			void operator()(SearchInfo* si);
			bool firstHubs;
			StringList hubs;
			bool op;
			bool hasTTH;
			bool firstTTH;
			tstring tth;
		};

		const tstring& getText(int col) const { return columns[col]; }
		int getImage();

		static int compareItems(SearchInfo* a, SearchInfo* b, int col);

		void update();

		SearchResult* sr;

		tstring columns[COLUMN_LAST];
	};

	struct HubInfo : public FastAlloc<HubInfo> {
		HubInfo(const tstring& aUrl, const tstring& aName, bool aOp) : url(aUrl),
			name(aName), op(aOp) { }

		const tstring& getText(int col) const {
			return (col == 0) ? name : Util::emptyStringT;
		}
		int getImage() const {
			return 0;
		}
		static int compareItems(HubInfo* a, HubInfo* b, int col) {
			return (col == 0) ? lstrcmpi(a->name.c_str(), b->name.c_str()) : 0;
		}
		tstring url;
		tstring name;
		bool op;
	};

	typedef set<SearchFrame*> FrameSet;
	typedef FrameSet::iterator FrameIter;
	static FrameSet frames;

	WidgetStaticPtr searchLabel;
	WidgetTextBoxPtr searchBox;
	WidgetButtonPtr purge;

	WidgetStaticPtr sizeLabel;
	WidgetComboBoxPtr mode;
	WidgetTextBoxPtr size;
	WidgetComboBoxPtr sizeMode;

	WidgetStaticPtr typeLabel;
	WidgetComboBoxPtr fileType;

	WidgetStaticPtr optionLabel;
	WidgetCheckBoxPtr slots;
	bool onlyFree;

	WidgetStaticPtr hubsLabel;
	typedef TypedListViewCtrl<SearchFrame, HubInfo> WidgetHubs;
	typedef WidgetHubs* WidgetHubsPtr;
	WidgetHubsPtr hubs;

	WidgetButtonPtr doSearch;

	typedef TypedListViewCtrl<SearchFrame, SearchInfo> WidgetResults;
	typedef WidgetResults* WidgetResultsPtr;
	WidgetResultsPtr results;

	WidgetCheckBoxPtr showUI;
	bool bShowUI;
	bool isHash;

	tstring initialString;
	int64_t initialSize;
	SearchManager::SizeModes initialMode;
	SearchManager::TypeModes initialType;

	static TStringList lastSearches;

	size_t droppedResults;

	TStringList currentSearch;
	StringList targets;

	CriticalSection cs;

#ifdef PORT_ME
	StringMap ucLineParams;

	// Timer ID, needed to turn off timer
	UINT timerID;
#endif

	SearchFrame(SmartWin::Widget* mdiParent, const tstring& initialString_, LONGLONG initialSize_, SearchManager::SizeModes initialMode_, SearchManager::TypeModes initialType_);
	virtual ~SearchFrame();

	HRESULT spoken(LPARAM lParam, WPARAM wParam);

	void handlePurgeClicked(WidgetButtonPtr);
	void handleSlotsClicked(WidgetCheckBoxPtr);
	void handleDoSearchClicked(WidgetButtonPtr);
	void handleShowUIClicked(WidgetCheckBoxPtr);

	typedef SmartWin::WidgetDataGrid<SearchFrame, SmartWin::MessageMapPolicyMDIChildWidget>* DataGridMessageType;

	HRESULT handleHubItemChanged(DataGridMessageType, LPARAM lParam, WPARAM /*wParam*/);

	HRESULT handleDoubleClick(DataGridMessageType, LPARAM lParam, WPARAM /*wParam*/);
	HRESULT handleKeyDown(DataGridMessageType, LPARAM lParam, WPARAM /*wParam*/);
	HRESULT handleContextMenu(DataGridMessageType, LPARAM lParam, WPARAM /*wParam*/);

	void handleDownload(WidgetMenuPtr /*menu*/, unsigned /*id*/);
	void handleDownloadFavoriteDirs(WidgetMenuPtr /*menu*/, unsigned id);
	void handleDownloadTo(WidgetMenuPtr /*menu*/, unsigned /*id*/);
	void handleDownloadTarget(WidgetMenuPtr /*menu*/, unsigned id);
	void handleDownloadDir(WidgetMenuPtr /*menu*/, unsigned /*id*/);
	void handleDownloadWholeFavoriteDirs(WidgetMenuPtr /*menu*/, unsigned id);
	void handleDownloadWholeTarget(WidgetMenuPtr /*menu*/, unsigned id);
	void handleDownloadDirTo(WidgetMenuPtr /*menu*/, unsigned /*id*/);
	void handleViewAsText(WidgetMenuPtr /*menu*/, unsigned /*id*/);
	void handleSearchAlternates(WidgetMenuPtr /*menu*/, unsigned /*id*/);
	void handleBitziLookup(WidgetMenuPtr /*menu*/, unsigned /*id*/);
	void handleCopyMagnet(WidgetMenuPtr /*menu*/, unsigned /*id*/);
	void handleRemove(WidgetMenuPtr /*menu*/, unsigned /*id*/);

	WidgetPopupMenuPtr makeMenu();
	void addTargetMenu(const WidgetPopupMenuPtr& parent, const StringPairList& favoriteDirs, const SearchInfo::CheckTTH& checkTTH);
	void addTargetDirMenu(const WidgetPopupMenuPtr& parent, const StringPairList& favoriteDirs);

	WidgetResultsPtr getUserList() { return results; }
	
	// SearchManagerListener
	virtual void on(SearchManagerListener::SR, SearchResult* aResult) throw();

	// ClientManagerListener
	virtual void on(ClientConnected, Client* c) throw() { speak(SPEAK_HUB_ADDED, c); }
	virtual void on(ClientUpdated, Client* c) throw() { speak(SPEAK_HUB_CHANGED, c); }
	virtual void on(ClientDisconnected, Client* c) throw() { speak(SPEAK_HUB_REMOVED, c); }

	void onHubAdded(HubInfo* info);
	void onHubChanged(HubInfo* info);
	void onHubRemoved(HubInfo* info);

	void speak(Speakers s, Client* aClient);
};

#ifdef PORT_ME

class SearchFrame :
	public UCHandler<SearchFrame>, public UserInfoBaseHandler<SearchFrame>
{
public:
	DECLARE_FRAME_WND_CLASS_EX(_T("SearchFrame"), IDR_SEARCH, 0, COLOR_3DFACE)

	typedef MDITabChildWindowImpl<SearchFrame, RGB(127, 127, 255)> baseClass;
	typedef UCHandler<SearchFrame> ucBase;
	typedef UserInfoBaseHandler<SearchFrame> uicBase;

	BEGIN_MSG_MAP(SearchFrame)
		MESSAGE_HANDLER(WM_CTLCOLOREDIT, onCtlColor)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, onCtlColor)
		MESSAGE_HANDLER(WM_CTLCOLORLISTBOX, onCtlColor)
		MESSAGE_HANDLER(WM_TIMER, onTimer)
		COMMAND_ID_HANDLER(IDC_GETLIST, onGetList)
		COMMAND_ID_HANDLER(IDC_BROWSELIST, onBrowseList)
		CHAIN_COMMANDS(ucBase)
		CHAIN_COMMANDS(uicBase)
		CHAIN_MSG_MAP(baseClass)
	ALT_MSG_MAP(SEARCH_MESSAGE_MAP)
		MESSAGE_HANDLER(WM_CHAR, onChar)
		MESSAGE_HANDLER(WM_KEYDOWN, onChar)
		MESSAGE_HANDLER(WM_KEYUP, onChar)
	END_MSG_MAP()

	LRESULT onChar(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onCtlColor(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onGetList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onBrowseList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	void runUserCommand(UserCommand& uc);

private:
	class SearchInfo;
public:
	TypedListViewCtrl<SearchInfo, IDC_RESULTS>& getUserList() { return ctrlResults; }

private:
	void onEnter();
	void onTab(bool shift);
};

#endif

#endif // !defined(DCPLUSPLUS_WIN32_SEARCH_FRAME_H)

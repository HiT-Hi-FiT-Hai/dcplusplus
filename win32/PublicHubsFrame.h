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

#ifndef DCPLUSPLUS_WIN32_PUBLIC_HUBS_FRAME_H
#define DCPLUSPLUS_WIN32_PUBLIC_HUBS_FRAME_H

#include "StaticFrame.h"

#include "TypedListViewCtrl.h"

#include <dcpp/FavoriteManager.h>

class PublicHubsFrame : 
	public StaticFrame<PublicHubsFrame>,
	public FavoriteManagerListener
{
public:
	enum Status {
		STATUS_STATUS,
		STATUS_HUBS,
		STATUS_USERS,
		STATUS_DUMMY,
		STATUS_LAST
	};
	static const ResourceManager::Strings TITLE_RESOURCE = ResourceManager::PUBLIC_HUBS;

private:
	typedef StaticFrame<PublicHubsFrame> BaseType;
	friend class StaticFrame<PublicHubsFrame>;
	friend class MDIChildFrame<PublicHubsFrame>;

	PublicHubsFrame(SmartWin::Widget* mdiParent);
	virtual ~PublicHubsFrame();

	enum {
		COLUMN_FIRST,
		COLUMN_NAME = COLUMN_FIRST,
		COLUMN_DESCRIPTION,
		COLUMN_USERS,
		COLUMN_SERVER,
		COLUMN_COUNTRY,
		COLUMN_SHARED,
		COLUMN_MINSHARE,
		COLUMN_MINSLOTS,
		COLUMN_MAXHUBS,
		COLUMN_MAXUSERS,
		COLUMN_RELIABILITY,
		COLUMN_RATING,
		COLUMN_LAST
	};

	enum {
		FINISHED,
		LOADED_FROM_CACHE,
		STARTING,
		FAILED
	};

	enum FilterModes{
		NONE,
		EQUAL,
		GREATER_EQUAL,
		LESS_EQUAL,
		GREATER,
		LESS,
		NOT_EQUAL
	};
	
	class HubInfo {
	public:
		HubInfo(const HubEntry* entry_);
		
		static int compareItems(const HubInfo* a, const HubInfo* b, int col);
		const tstring& getText(int column) const { return columns[column]; }
		int getImage() const { return 0; }
		const HubEntry* entry;
		tstring columns[COLUMN_LAST];
	};

	typedef TypedListViewCtrl<PublicHubsFrame, HubInfo> WidgetHubs;
	typedef WidgetHubs* WidgetHubsPtr;
	WidgetHubsPtr hubs;

	WidgetButtonPtr configure;
	WidgetButtonPtr refresh;
	WidgetButtonPtr lists;
	WidgetButtonPtr filterDesc;
	WidgetTextBoxPtr filter;
	WidgetComboBoxPtr pubLists;
	WidgetComboBoxPtr filterSel;

	int visibleHubs;
	int users;
	
	string filterString;

	HubEntry::List entries;
	
	static int columnIndexes[];
	static int columnSizes[];

	using AspectSpeaker<PublicHubsFrame>::speak;
	
	void layout();
	bool preClosing();
	void postClosing();
	HRESULT handleSpeaker(WPARAM wParam, LPARAM lParam);
	void handleConfigure();
	void handleRefresh();
	void handleConnect();
	void handleAdd();
	void handleCopyHub();
	HRESULT handleContextMenu(WPARAM wParam, LPARAM lParam);

	bool checkNick();
	void updateStatus();
	void updateList();
	void updateDropDown();

	bool parseFilter(FilterModes& mode, double& size);
	bool matchFilter(const HubEntry& entry, const int& sel, bool doSizeCompare, const FilterModes& mode, const double& size);

	virtual void on(DownloadStarting, const string& l) throw() { speak(STARTING, l); }
	virtual void on(DownloadFailed, const string& l) throw() { speak(FAILED, l); }
	virtual void on(DownloadFinished, const string& l) throw() { speak(FINISHED, l); }
	virtual void on(LoadedFromCache, const string& l) throw() { speak(LOADED_FROM_CACHE, l); }

	void speak(int x, const string& l) {
		speak(static_cast<WPARAM>(x), reinterpret_cast<LPARAM>(new tstring(Text::toT(l))));
	}


};

#ifdef PORT_ME

#include "ExListViewCtrl.h"

#include "../client/FavoriteManager.h"
#include "../client/StringSearch.h"

#include "WinUtil.h"

#define FILTER_MESSAGE_MAP 8
class PublicHubsFrame : public MDITabChildWindowImpl<PublicHubsFrame>, public StaticFrame<PublicHubsFrame, >,
	private FavoriteManagerListener
{
public:
	PublicHubsFrame() : users(0), hubs(0), closed(false),
		filterContainer(WC_EDIT, this, FILTER_MESSAGE_MAP) {
	}

	virtual ~PublicHubsFrame() { }

	DECLARE_FRAME_WND_CLASS_EX(_T("PublicHubsFrame"), IDR_PUBLICHUBS, 0, COLOR_3DFACE);

	typedef MDITabChildWindowImpl<PublicHubsFrame> baseClass;
	BEGIN_MSG_MAP(PublicHubsFrame)
		MESSAGE_HANDLER(WM_CREATE, onCreate)
		COMMAND_ID_HANDLER(IDC_FILTER_FOCUS, onFilterFocus)
		COMMAND_ID_HANDLER(IDC_ADD, onAdd)
		COMMAND_ID_HANDLER(IDC_REFRESH, onClickedRefresh)
		COMMAND_ID_HANDLER(IDC_PUB_LIST_CONFIG, onClickedConfigure)
		COMMAND_ID_HANDLER(IDC_CONNECT, onClickedConnect)
		COMMAND_ID_HANDLER(IDC_COPY_HUB, onCopyHub);
		NOTIFY_HANDLER(IDC_HUBLIST, LVN_COLUMNCLICK, onColumnClickHublist)
		NOTIFY_HANDLER(IDC_HUBLIST, NM_RETURN, onEnter)
		NOTIFY_HANDLER(IDC_HUBLIST, NM_DBLCLK, onDoubleClickHublist)
		COMMAND_HANDLER(IDC_PUB_LIST_DROPDOWN, CBN_SELCHANGE, onListSelChanged)
		CHAIN_MSG_MAP(baseClass)
	ALT_MSG_MAP(FILTER_MESSAGE_MAP)
		MESSAGE_HANDLER(WM_CHAR, onFilterChar)
	END_MSG_MAP()

	LRESULT onFilterChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT onCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onDoubleClickHublist(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT onEnter(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT onFilterFocus(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onAdd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT onClickedRefresh(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT onClickedConfigure(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT onClickedConnect(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT onCopyHub(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onListSelChanged(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onColumnClickHublist(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);
private:
	CContainedWindow filterContainer;

};

#endif /* PORT_ME */

#endif // !defined(PUBLIC_HUBS_FRM_H)

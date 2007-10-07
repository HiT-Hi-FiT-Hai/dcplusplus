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

#include "TypedListView.h"

#include <dcpp/HubEntry.h>
#include <dcpp/FavoriteManagerListener.h>
#include "resource.h"

class PublicHubsFrame : 
	public StaticFrame<PublicHubsFrame>,
	public FavoriteManagerListener
{
public:
	enum Status {
		STATUS_STATUS,
		STATUS_HUBS,
		STATUS_USERS,
		STATUS_LAST
	};
	static const ResourceManager::Strings TITLE_RESOURCE = ResourceManager::PUBLIC_HUBS;
	static const unsigned ICON_RESOURCE = IDR_PUBLICHUBS;
	
private:
	typedef StaticFrame<PublicHubsFrame> BaseType;
	friend class StaticFrame<PublicHubsFrame>;
	friend class MDIChildFrame<PublicHubsFrame>;

	PublicHubsFrame(SmartWin::WidgetTabView* mdiParent);
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

	typedef TypedListView<PublicHubsFrame, HubInfo> WidgetHubs;
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

	HubEntryList entries;
	
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
	bool handleKeyDown(int c);
	void handleListSelChanged();
	bool handleFilterChar(int c);
	
	bool checkNick();
	void updateStatus();
	void updateList();
	void updateDropDown();
	void openSelected();

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

#endif // !defined(PUBLIC_HUBS_FRM_H)

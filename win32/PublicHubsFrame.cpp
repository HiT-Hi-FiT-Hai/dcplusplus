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

#include "PublicHubsFrame.h"

#include "resource.h"
#include "HubFrame.h"
#include "HoldRedraw.h"
#include "HubListsDlg.h"

#include <dcpp/FavoriteManager.h>
#include <dcpp/version.h>

int PublicHubsFrame::columnIndexes[] = {
	COLUMN_NAME,
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
	COLUMN_RATING
 };

int PublicHubsFrame::columnSizes[] = { 200, 290, 50, 100, 100, 100, 100, 100, 100, 100, 100, 100 };

static const char*  columnNames[] = {
	N_("Name"),
	N_("Description"),
	N_("Users"),
	N_("Address"),
	N_("Country"),
	N_("Shared"),
	N_("Min Share"),
	N_("Min Slots"),
	N_("Max Hubs"),
	N_("Max Users"),
	N_("Reliability"),
	N_("Rating")
};

PublicHubsFrame::HubInfo::HubInfo(const HubEntry* entry_) : entry(entry_) {
	columns[COLUMN_NAME] = Text::toT(entry->getName());
	columns[COLUMN_DESCRIPTION] = Text::toT(entry->getDescription());
	columns[COLUMN_USERS] = Text::toT(Util::toString(entry->getUsers()));
	columns[COLUMN_SERVER] = Text::toT(entry->getServer());
	columns[COLUMN_COUNTRY] = Text::toT(entry->getCountry());
	columns[COLUMN_SHARED] = Text::toT(Util::formatBytes(entry->getShared()));
	columns[COLUMN_MINSHARE] = Text::toT(Util::formatBytes(entry->getMinShare()));
	columns[COLUMN_MINSLOTS] = Text::toT(Util::toString(entry->getMinSlots()));
	columns[COLUMN_MAXHUBS] = Text::toT(Util::toString(entry->getMaxHubs()));
	columns[COLUMN_MAXUSERS] = Text::toT(Util::toString(entry->getMaxUsers()));
	columns[COLUMN_RELIABILITY] = Text::toT(Util::toString(entry->getReliability()));
	columns[COLUMN_RATING] = Text::toT(entry->getRating());
}

int PublicHubsFrame::HubInfo::compareItems(const HubInfo* a, const HubInfo* b, int col) {
	switch(col) {
	case COLUMN_USERS: return compare(a->entry->getUsers(), b->entry->getUsers()); 
	case COLUMN_MINSLOTS: return compare(a->entry->getMinSlots(), b->entry->getMinSlots());
	case COLUMN_MAXHUBS: return compare(a->entry->getMaxHubs(), b->entry->getMaxHubs());
	case COLUMN_MAXUSERS: return compare(a->entry->getMaxUsers(), b->entry->getMaxUsers());
	case COLUMN_RELIABILITY: return compare(a->entry->getReliability(), b->entry->getReliability());
	case COLUMN_SHARED: return compare(a->entry->getShared(), b->entry->getShared());
	case COLUMN_MINSHARE: return compare(a->entry->getMinShare(), b->entry->getMinShare());
	case COLUMN_RATING: return compare(a->entry->getRating(), b->entry->getRating());
	default: return lstrcmpi(a->columns[col].c_str(), b->columns[col].c_str());
	}
}

PublicHubsFrame::PublicHubsFrame(SmartWin::WidgetTabView* mdiParent) :
	BaseType(mdiParent, T_("Public Hubs"), IDH_PUBLIC_HUBS, IDR_PUBLICHUBS),
	hubs(0),
	configure(0),
	refresh(0),
	lists(0),
	filterDesc(0),
	filter(0),
	pubLists(0),
	filterSel(0),
	visibleHubs(0),
	users(0)
{
	{
		WidgetHubs::Seed cs;
		cs.style |= LVS_SINGLESEL;
		hubs = addChild(cs);
		addWidget(hubs);
		
		hubs->createColumns(WinUtil::getStrings(columnNames));
		hubs->setColumnOrder(WinUtil::splitTokens(SETTING(FAVHUBSFRAME_ORDER), columnIndexes));
		hubs->setColumnWidths(WinUtil::splitTokens(SETTING(FAVHUBSFRAME_WIDTHS), columnSizes));
		hubs->setSort(COLUMN_USERS, false);
		
		hubs->onDblClicked(std::tr1::bind(&PublicHubsFrame::openSelected, this));
		hubs->onKeyDown(std::tr1::bind(&PublicHubsFrame::handleKeyDown, this, _1));
		hubs->onContextMenu(std::tr1::bind(&PublicHubsFrame::handleContextMenu, this, _1));
	}

	{
		TextBox::Seed cs = WinUtil::Seeds::textBox;
		cs.style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL;
		filter = addChild(cs);
		filter->setHelpId(IDH_PUBLIC_HUBS_FILTER);
		addWidget(filter);
		filter->onKeyDown(std::tr1::bind(&PublicHubsFrame::handleFilterKeyDown, this, _1));
	}

	{
		filterSel = addChild(WinUtil::Seeds::comboBoxStatic);
		filterSel->setHelpId(IDH_PUBLIC_HUBS_FILTER);
		addWidget(filterSel);

		//populate the filter list with the column names
		for(int j=0; j<COLUMN_LAST; j++) {
			filterSel->addValue(T_(columnNames[j]));
		}
		filterSel->addValue(T_("Any"));
		filterSel->setSelected(COLUMN_LAST);
		filterSel->onSelectionChanged(std::tr1::bind(&PublicHubsFrame::updateList, this));

		pubLists = addChild(WinUtil::Seeds::comboBoxStatic);
		pubLists->setHelpId(IDH_PUBLIC_HUBS_LISTS);
		addWidget(pubLists);
		pubLists->onSelectionChanged(std::tr1::bind(&PublicHubsFrame::handleListSelChanged, this));
	}

	{
		Button::Seed cs = WinUtil::Seeds::button;
		
		cs.caption = T_("&Configure");
		configure = addChild(cs);
		configure->setHelpId(IDH_PUBLIC_HUBS_LISTS);
		configure->setFont(WinUtil::font);
		addWidget(configure);
		configure->onClicked(std::tr1::bind(&PublicHubsFrame::handleConfigure, this));
		
		cs.caption = T_("&Refresh");
		refresh = addChild(cs);
		refresh->setHelpId(IDH_PUBLIC_HUBS_REFRESH);
		refresh->setFont(WinUtil::font);
		addWidget(refresh);
		refresh->onClicked(std::tr1::bind(&PublicHubsFrame::handleRefresh, this));

		cs.style = WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_GROUPBOX;
		cs.exStyle = WS_EX_TRANSPARENT;

		cs.caption = T_("F&ilter");
		filterDesc = addChild(cs);
		filterDesc->setHelpId(IDH_PUBLIC_HUBS_FILTER);
		filterDesc->setFont(WinUtil::font);

		cs.caption = T_("Configured Public Hub Lists");
		lists = addChild(cs);
		lists->setHelpId(IDH_PUBLIC_HUBS_LISTS);
		lists->setFont(WinUtil::font);
	}

	initStatus();

	FavoriteManager::getInstance()->addListener(this);

	entries	 = FavoriteManager::getInstance()->getPublicHubs();

	// populate with values from the settings
	updateDropDown();
	updateList();

	layout();
	
	onSpeaker(std::tr1::bind(&PublicHubsFrame::handleSpeaker, this, _1, _2));
	
	if(FavoriteManager::getInstance()->isDownloading()) {
		setStatus(STATUS_STATUS, T_("Downloading public hub list..."));
	} else if(entries.empty()) {
		FavoriteManager::getInstance()->refresh();
	}
}

PublicHubsFrame::~PublicHubsFrame() {
}

bool PublicHubsFrame::preClosing() {
	FavoriteManager::getInstance()->removeListener(this);

	return true;
}

void PublicHubsFrame::postClosing() {
	SettingsManager::getInstance()->set(SettingsManager::PUBLICHUBSFRAME_ORDER, WinUtil::toString(hubs->getColumnOrder()));
	SettingsManager::getInstance()->set(SettingsManager::PUBLICHUBSFRAME_WIDTHS, WinUtil::toString(hubs->getColumnWidths()));
}

void PublicHubsFrame::layout() {
	const int border = 2;

	SmartWin::Rectangle r(getClientAreaSize()); 
	
	layoutStatus(r);

	int const comboH = 140;

	// Table
	int ymessage = filter->getTextSize(_T("A")).y + 10;

	r.size.y -= ymessage*2 + 8 + border * 2;
	hubs->setBounds(r);

	r.pos.y += r.size.y + border;
	r.size.y = ymessage * 2 + 8;
	
	// filter box
	r.size.x = (r.width() - 100 - border * 2) / 2 ;
	filterDesc->setBounds(r);

	SmartWin::Rectangle rc = r;
	// filter edit
	rc.pos.y += ymessage - 4;
	rc.size.y = ymessage;
	rc.pos.x += 16;
	rc.size.x = rc.width() * 2 / 3 - 24 - border;
	filter->setBounds(rc);
	
	//filter sel
	rc.size.y += comboH;
	
	rc.pos.x += rc.width() + border;
	rc.size.x = (rc.width() + 24 + border) / 2 - 8;
	filterSel->setBounds(rc);

	// lists box
	r.pos.x = r.width() + border;
	lists->setBounds(r);

	rc = r;
	// lists dropdown
	rc.pos.y += ymessage - 4;
	rc.size.y = ymessage;
	
	rc.size.y += comboH;
	rc.pos.x += 16;
	rc.size.x -= 100 + 24 + border;
	pubLists->setBounds(rc);

	// configure button
	rc.size.y -= comboH;
	rc.pos.x += rc.width() + border;
	rc.size.x = 100;
	configure->setBounds(rc);

	// refresh button
	rc.pos.x += rc.width() + 8;
	refresh->setBounds(rc);
}

void PublicHubsFrame::updateStatus() {
	setStatus(STATUS_HUBS, str(TF_("Hubs: %1%") % visibleHubs));
	setStatus(STATUS_USERS, str(TF_("Users: %1%") % users));
}

void PublicHubsFrame::updateDropDown() {
	pubLists->clear();
	StringList lists(FavoriteManager::getInstance()->getHubLists());
	for(StringList::iterator idx = lists.begin(); idx != lists.end(); ++idx) {
		pubLists->addValue(Text::toT(*idx).c_str());
	}
	pubLists->setSelected(FavoriteManager::getInstance()->getSelectedHubList());
}

void PublicHubsFrame::updateList() {
	hubs->clear();
	users = 0;
	visibleHubs = 0;

	HoldRedraw hold(hubs);
	
	double size = -1;
	FilterModes mode = NONE;

	int sel = filterSel->getSelected();

	bool doSizeCompare = parseFilter(mode, size);

	for(HubEntryList::const_iterator i = entries.begin(); i != entries.end(); ++i) {
		if(matchFilter(*i, sel, doSizeCompare, mode, size)) {
			hubs->insert(hubs->size(), new HubInfo(&(*i)));
			visibleHubs++;
			users += i->getUsers();
		}
	}

	hubs->resort();

	updateStatus();
}

LRESULT PublicHubsFrame::handleSpeaker(WPARAM wParam, LPARAM lParam) {
	if((wParam == FINISHED) || (wParam == LOADED_FROM_CACHE)) {
		std::auto_ptr<tstring> x(reinterpret_cast<tstring*>(lParam));
		entries = FavoriteManager::getInstance()->getPublicHubs();
		updateList();
		setStatus(STATUS_STATUS, ((wParam == LOADED_FROM_CACHE) ? T_("Hub list loaded from cache...") : str(TF_("Hub list downloaded... (%1%)") % (*x))));
	} else if(wParam == STARTING) {
		std::auto_ptr<tstring> x(reinterpret_cast<tstring*>(lParam));
		setStatus(STATUS_STATUS, str(TF_("Downloading public hub list... (%1%)") % (*x)));
	} else if(wParam == FAILED) {
		std::auto_ptr<tstring> x(reinterpret_cast<tstring*>(lParam));
		setStatus(STATUS_STATUS, str(TF_("Download failed: %1%") % (*x)));
	}
	return 0;
}

bool PublicHubsFrame::parseFilter(FilterModes& mode, double& size) {
	string::size_type start = (string::size_type)string::npos;
	string::size_type end = (string::size_type)string::npos;
	int64_t multiplier = 1;

	if(filterString.compare(0, 2, ">=") == 0) {
		mode = GREATER_EQUAL;
		start = 2;
	} else if(filterString.compare(0, 2, "<=") == 0) {
		mode = LESS_EQUAL;
		start = 2;
	} else if(filterString.compare(0, 2, "==") == 0) {
		mode = EQUAL;
		start = 2;
	} else if(filterString.compare(0, 2, "!=") == 0) {
		mode = NOT_EQUAL;
		start = 2;
	} else if(filterString[0] == _T('<')) {
		mode = LESS;
		start = 1;
	} else if(filterString[0] == _T('>')) {
		mode = GREATER;
		start = 1;
	} else if(filterString[0] == _T('=')) {
		mode = EQUAL;
		start = 1;
	}

	if(start == string::npos)
		return false;
	if(filterString.length() <= start)
		return false;

	if((end = Util::findSubString(filterString, "TiB")) != tstring::npos) {
		multiplier = 1024LL * 1024LL * 1024LL * 1024LL;
	} else if((end = Util::findSubString(filterString, "GiB")) != tstring::npos) {
		multiplier = 1024*1024*1024;
	} else if((end = Util::findSubString(filterString, "MiB")) != tstring::npos) {
		multiplier = 1024*1024;
	} else if((end = Util::findSubString(filterString, "KiB")) != tstring::npos) {
		multiplier = 1024;
	} else if((end = Util::findSubString(filterString, "TB")) != tstring::npos) {
		multiplier = 1000LL * 1000LL * 1000LL * 1000LL;
	} else if((end = Util::findSubString(filterString, "GB")) != tstring::npos) {
		multiplier = 1000*1000*1000;
	} else if((end = Util::findSubString(filterString, "MB")) != tstring::npos) {
		multiplier = 1000*1000;
	} else if((end = Util::findSubString(filterString, "kB")) != tstring::npos) {
		multiplier = 1000;
	} else if((end = Util::findSubString(filterString, "B")) != tstring::npos) {
		multiplier = 1;
	}

	if(end == string::npos) {
		end = filterString.length();
	}

	string tmpSize = filterString.substr(start, end-start);
	size = Util::toDouble(tmpSize) * multiplier;

	return true;
}

bool PublicHubsFrame::matchFilter(const HubEntry& entry, const int& sel, bool doSizeCompare, const FilterModes& mode, const double& size) {
	if(filterString.empty())
		return true;

	double entrySize = 0;
	string entryString = "";

	switch(sel) {
		case COLUMN_NAME: entryString = entry.getName(); doSizeCompare = false; break;
		case COLUMN_DESCRIPTION: entryString = entry.getDescription(); doSizeCompare = false; break;
		case COLUMN_USERS: entrySize = entry.getUsers(); break;
		case COLUMN_SERVER: entryString = entry.getServer(); doSizeCompare = false; break;
		case COLUMN_COUNTRY: entryString = entry.getCountry(); doSizeCompare = false; break;
		case COLUMN_SHARED: entrySize = (double)entry.getShared(); break;
		case COLUMN_MINSHARE: entrySize = (double)entry.getMinShare(); break;
		case COLUMN_MINSLOTS: entrySize = entry.getMinSlots(); break;
		case COLUMN_MAXHUBS: entrySize = entry.getMaxHubs(); break;
		case COLUMN_MAXUSERS: entrySize = entry.getMaxUsers(); break;
		case COLUMN_RELIABILITY: entrySize = entry.getReliability(); break;
		case COLUMN_RATING: entryString = entry.getRating(); doSizeCompare = false; break;
		default: break;
	}

	bool insert = false;
	if(doSizeCompare) {
		switch(mode) {
			case EQUAL: insert = (size == entrySize); break;
			case GREATER_EQUAL: insert = (size <= entrySize); break;
			case LESS_EQUAL: insert = (size >= entrySize); break;
			case GREATER: insert = (size < entrySize); break;
			case LESS: insert = (size > entrySize); break;
			case NOT_EQUAL: insert = (size != entrySize); break;
			case NONE: ; break;
		}
	} else {
		if(sel >= COLUMN_LAST) {
			if( Util::findSubString(entry.getName(), filterString) != string::npos ||
				Util::findSubString(entry.getDescription(), filterString) != string::npos ||
				Util::findSubString(entry.getServer(), filterString) != string::npos ||
				Util::findSubString(entry.getCountry(), filterString) != string::npos ||
				Util::findSubString(entry.getRating(), filterString) != string::npos ) {
					insert = true;
				}
		}
		if(Util::findSubString(entryString, filterString) != string::npos)
			insert = true;
	}

	return insert;
}

bool PublicHubsFrame::handleContextMenu(SmartWin::ScreenCoordinate pt) {
	if(hubs->hasSelected()) {
		if(pt.x() == -1 && pt.y() == -1) {
			pt = hubs->getContextMenuPos();
		}

		WidgetMenuPtr menu = createMenu(WinUtil::Seeds::menu);
		menu->appendItem(IDC_CONNECT, T_("&Connect"), std::tr1::bind(&PublicHubsFrame::handleConnect, this));
		menu->appendItem(IDC_ADD, T_("Add To &Favorites"), std::tr1::bind(&PublicHubsFrame::handleAdd, this), SmartWin::BitmapPtr(new SmartWin::Bitmap(IDB_FAVORITE_HUBS)));
		menu->appendItem(IDC_COPY_HUB, T_("Copy &address to clipboard"), std::tr1::bind(&PublicHubsFrame::handleCopyHub, this));
		menu->setDefaultItem(IDC_CONNECT);
		menu->trackPopupMenu(pt, TPM_LEFTALIGN | TPM_RIGHTBUTTON);
		return true;
	}
	return false;
}

void PublicHubsFrame::handleRefresh() {
	setStatus(STATUS_STATUS, T_("Downloading public hub list..."));
	FavoriteManager::getInstance()->refresh(true);
	updateDropDown();
}

void PublicHubsFrame::handleConfigure() {
	HubListsDlg dlg(this);
	if(dlg.run() == IDOK) {
		updateDropDown();
	}
}

void PublicHubsFrame::handleConnect() {
	if(!checkNick())
		return;

	if(hubs->hasSelected() == 1) {
		HubFrame::openWindow(getParent(), hubs->getSelectedData()->entry->getServer());
	}
}

void PublicHubsFrame::handleAdd() {
	if(!checkNick())
		return;

	if(hubs->hasSelected()) {
		FavoriteManager::getInstance()->addFavorite(*hubs->getSelectedData()->entry);
	}	
}

void PublicHubsFrame::handleCopyHub() {
	if(hubs->hasSelected()) {
		WinUtil::setClipboard(Text::toT(hubs->getSelectedData()->entry->getServer()));
	}
}

bool PublicHubsFrame::checkNick() {
	if(SETTING(NICK).empty()) {
		createMessageBox().show(T_("Please enter a nickname in the settings dialog!"), _T(APPNAME) _T(" ") _T(VERSIONSTRING));
		return false;
	}
	return true;
}


void PublicHubsFrame::openSelected() {
	if(!checkNick())
		return;
	
	if(hubs->hasSelected()) {
		HubFrame::openWindow(getParent(), hubs->getSelectedData()->entry->getServer());
	}
}

bool PublicHubsFrame::handleKeyDown(int c) {
	if(c == VK_RETURN) {
		openSelected();
	}
	
	return false;
}

void PublicHubsFrame::handleListSelChanged() {
	FavoriteManager::getInstance()->setHubList(pubLists->getSelected());
	entries = FavoriteManager::getInstance()->getPublicHubs();
	updateList();
}

bool PublicHubsFrame::handleFilterKeyDown(int c) {
	if(c == VK_RETURN) {
		filterString = Text::fromT(filter->getText());
		updateList();
		return true;
	} 
	return false;
}

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

#include "stdafx.h"
#include "resource.h"

#include "SpyFrame.h"

#include <dcpp/ShareManager.h>
#include <dcpp/ClientManager.h>

#include "SearchFrame.h"

int SpyFrame::columnSizes[] = { 305, 70, 85 };
int SpyFrame::columnIndexes[] = { COLUMN_STRING, COLUMN_COUNT, COLUMN_TIME };
static ResourceManager::Strings columnNames[] = { ResourceManager::SEARCH_STRING, ResourceManager::COUNT, ResourceManager::TIME };

SpyFrame::SpyFrame(SmartWin::WidgetTabView* mdiParent) :
	BaseType(mdiParent),
	searches(0),
	ignoreTTH(0),
	bIgnoreTTH(BOOLSETTING(SPY_FRAME_IGNORE_TTH_SEARCHES)),
	total(0),
	cur(0)
{
	memset(perSecond, 0, sizeof(perSecond));

	{
		WidgetListView::Seed cs = WinUtil::Seeds::listView;
		cs.style |= LVS_SINGLESEL;
		searches = createListView(cs);
		addWidget(searches);

		searches->createColumns(ResourceManager::getInstance()->getStrings(columnNames));
		searches->setColumnOrder(WinUtil::splitTokens(SETTING(SPYFRAME_ORDER), columnIndexes));
		searches->setColumnWidths(WinUtil::splitTokens(SETTING(SPYFRAME_WIDTHS), columnSizes));
		searches->setSort(COLUMN_COUNT, SmartWin::WidgetListView::SORT_INT, false);
		searches->setColor(WinUtil::textColor, WinUtil::bgColor);
		searches->onColumnClick(std::tr1::bind(&SpyFrame::handleColumnClick, this, _1));
		searches->onContextMenu(std::tr1::bind(&SpyFrame::handleContextMenu, this, _1));

	}

	{
		WidgetCheckBox::Seed cs(TSTRING(IGNORE_TTH_SEARCHES));
		ignoreTTH = createCheckBox(cs);
		ignoreTTH->setChecked(bIgnoreTTH);
		ignoreTTH->onClicked(std::tr1::bind(&SpyFrame::handleIgnoreTTHClicked, this));
	}

	initStatus();
	statusSizes[STATUS_IGNORE_TTH] = 150; ///@todo get real checkbox + text width

	layout();

	onSpeaker(std::tr1::bind(&SpyFrame::handleSpeaker, this, _1, _2)) ;

	ShareManager::getInstance()->setHits(0);

	ClientManager::getInstance()->addListener(this);

	initSecond();
#if 0
	// for testing purposes; adds 2 dummy lines into the list
	postMessage(WM_SPEAKER, SPEAK_SEARCH, (LPARAM)new tstring(_T("search 1")));
	postMessage(WM_SPEAKER, SPEAK_SEARCH, (LPARAM)new tstring(_T("search 2")));
#endif
}

SpyFrame::~SpyFrame() {
}

void SpyFrame::layout() {
	SmartWin::Rectangle r(this->getClientAreaSize());

	layoutStatus(r);
	mapWidget(STATUS_IGNORE_TTH, ignoreTTH);

	searches->setBounds(r);
}

void SpyFrame::initSecond() {
	createTimer(std::tr1::bind(&SpyFrame::eachSecond, this), 1000);
}

bool SpyFrame::eachSecond() {
	size_t tot = std::accumulate(perSecond, perSecond + AVG_TIME, 0u);
	size_t t = std::max(1u, std::min(cur, (size_t)AVG_TIME));
	
	float x = static_cast<float>(tot)/t;

	cur++;
	perSecond[cur] = 0;
	setStatus(STATUS_AVG_PER_SECOND, Text::toT(STRING(AVERAGE) + Util::toString(x)));
	return true;
}

bool SpyFrame::preClosing() {
	ClientManager::getInstance()->removeListener(this);
	return true;
}

void SpyFrame::postClosing() {
	searches->clear();

	SettingsManager::getInstance()->set(SettingsManager::SPYFRAME_ORDER, WinUtil::toString(searches->getColumnOrder()));
	SettingsManager::getInstance()->set(SettingsManager::SPYFRAME_WIDTHS, WinUtil::toString(searches->getColumnWidths()));

	SettingsManager::getInstance()->set(SettingsManager::SPY_FRAME_IGNORE_TTH_SEARCHES, bIgnoreTTH);
}

LRESULT SpyFrame::handleSpeaker(WPARAM wParam, LPARAM lParam) {
	if(wParam == SPEAK_SEARCH) {
		boost::scoped_ptr<tstring> x((tstring*)lParam);

		total++;

		// Not thread safe, but who cares really...
		perSecond[cur % AVG_TIME]++;

		int j = searches->find(*x);
		if(j == -1) {
			TStringList a;
			a.push_back(*x);
			a.push_back(Text::toT(Util::toString(1)));
			a.push_back(Text::toT(Util::getTimeString()));
			searches->insert(a);
			if(searches->size() > 500) {
				searches->erase(searches->size() - 1);
			}
		} else {
			searches->setText(j, COLUMN_COUNT, Text::toT(Util::toString(Util::toInt(Text::fromT(searches->getText(j, COLUMN_COUNT))) + 1)));
			searches->setText(j, COLUMN_TIME, Text::toT(Util::getTimeString()));
			if(searches->getSortColumn() == COLUMN_COUNT || searches->getSortColumn() == COLUMN_TIME )
				searches->resort();
		}

		setStatus(STATUS_TOTAL, Text::toT(STRING(TOTAL) + Util::toString(total)));
		setStatus(STATUS_HITS, Text::toT(STRING(HITS) + Util::toString(ShareManager::getInstance()->getHits())));
		double ratio = total > 0 ? ((double)ShareManager::getInstance()->getHits()) / (double)total : 0.0;
		setStatus(STATUS_HIT_RATIO, Text::toT(STRING(HIT_RATIO) + Util::toString(ratio)));
	}
	return 0;
}

void SpyFrame::handleColumnClick(int column) {
	if(column == searches->getSortColumn()) {
		if (!searches->isAscending())
			searches->setSort(-1, searches->getSortType());
		else
			searches->setSort(searches->getSortColumn(), searches->getSortType(), false);
	} else {
		if(column == COLUMN_COUNT) {
			searches->setSort(column, SmartWin::WidgetListView::SORT_INT);
		} else {
			searches->setSort(column, SmartWin::WidgetListView::SORT_STRING_NOCASE);
		}
	}
}

bool SpyFrame::handleContextMenu(SmartWin::ScreenCoordinate pt) {
	if(searches->getSelectedCount() == 1) {
		if(pt.x() == -1 && pt.y() == -1) {
			pt = searches->getContextMenuPos();
		}

		searchString = searches->getText(searches->getSelectedIndex(), COLUMN_STRING);

		WidgetMenuPtr contextMenu = createMenu(true);
		contextMenu->appendItem(IDC_SEARCH, TSTRING(SEARCH), std::tr1::bind(&SpyFrame::handleSearch, this));

		contextMenu->trackPopupMenu(this, pt, TPM_LEFTALIGN | TPM_RIGHTBUTTON);
		return true;
	}
	return false;
}

void SpyFrame::handleSearch() {
	if(Util::strnicmp(searchString.c_str(), _T("TTH:"), 4) == 0)
		SearchFrame::openWindow(getParent(), searchString.substr(4), 0, SearchManager::SIZE_DONTCARE, SearchManager::TYPE_TTH);
	else
		SearchFrame::openWindow(getParent(), searchString);
}

void SpyFrame::handleIgnoreTTHClicked() {
	bIgnoreTTH = ignoreTTH->getChecked();
}

void SpyFrame::on(ClientManagerListener::IncomingSearch, const string& s) throw() {
	if(bIgnoreTTH && s.compare(0, 4, "TTH:") == 0)
		return;
	tstring* x = new tstring(Text::toT(s));
	tstring::size_type i = 0;
	while( (i=x->find(_T('$'))) != string::npos) {
		(*x)[i] = _T(' ');
	}
	speak(SPEAK_SEARCH, static_cast<LPARAM>(SPEAK_SEARCH));
}

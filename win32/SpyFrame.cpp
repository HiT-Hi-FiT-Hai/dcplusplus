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
#include <client/DCPlusPlus.h>
#include "resource.h"

#include "SpyFrame.h"

#include <client/ShareManager.h>

int SpyFrame::columnSizes[] = { 305, 70, 85 };
int SpyFrame::columnIndexes[] = { COLUMN_STRING, COLUMN_COUNT, COLUMN_TIME };
static ResourceManager::Strings columnNames[] = { ResourceManager::SEARCH_STRING, ResourceManager::COUNT, ResourceManager::TIME };

SpyFrame::SpyFrame(SmartWin::Widget* mdiParent) :
	SmartWin::Widget(mdiParent),
	total(0),
	cur(0),
	bIgnoreTTH(BOOLSETTING(SPY_FRAME_IGNORE_TTH_SEARCHES))
{
	ZeroMemory(perSecond, sizeof(perSecond));

	{
		WidgetDataGrid::Seed cs;
		cs.style = WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL;
		cs.exStyle = WS_EX_CLIENTEDGE;
		searches = createDataGrid(cs);
		add_widget(searches);
		searches->setListViewStyle(LVS_EX_LABELTIP | LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT);

#ifdef PORT_ME
		for(int j=0; j<COLUMN_LAST; j++) {
			int fmt = (j == COLUMN_COUNT) ? LVCFMT_RIGHT : LVCFMT_LEFT;
			searches->InsertColumn(j, CTSTRING_I(columnNames[j]), fmt, columnSizes[j], j);
		}
#endif
		searches->createColumns(ResourceManager::getInstance()->getStrings(columnNames));
		searches->setColumnOrder(WinUtil::splitTokens(SETTING(SPYFRAME_ORDER), columnIndexes));
		searches->setColumnWidths(WinUtil::splitTokens(SETTING(SPYFRAME_WIDTHS), columnSizes));
#ifdef PORT_ME
		searches->setSort(COLUMN_COUNT, ExListViewCtrl::SORT_INT, false);
#endif

		searches->setColor(WinUtil::textColor, WinUtil::bgColor);
	}

	{
		WidgetCheckBox::Seed cs;
		cs.caption = TSTRING(IGNORE_TTH_SEARCHES);
		ignoreTTH = createCheckBox(cs);
		ignoreTTH->setChecked(bIgnoreTTH);
#ifdef PORT_ME
		ignoreTTH->setFont(WinUtil::systemFont);
#endif

		ignoreTTH->onClicked(&SpyFrame::handleIgnoreTTHClicked);
	}

	memset(statusSizes, 0, sizeof(statusSizes));
	statusSizes[STATUS_IGNORE_TTH] = 150; ///@todo get real checkbox + text width
	statusSizes[STATUS_DUMMY] = 16; ///@todo get real resizer width
	status = this->createStatusBarSections();

	layout();

	onSpeaker(&SpyFrame::spoken);

	ShareManager::getInstance()->setHits(0);

	ClientManager::getInstance()->addListener(this);
	TimerManager::getInstance()->addListener(this);

	searches->onRaw(&SpyFrame::handleColumnClick, SmartWin::Message(WM_NOTIFY, LVN_COLUMNCLICK));

	contextMenu = createPopupMenu();
	contextMenu->appendItem(IDC_SEARCH, TSTRING(SEARCH), &SpyFrame::handleSearch);
	searches->onRaw(&SpyFrame::handleContextMenu, SmartWin::Message(WM_CONTEXTMENU));

#if 1
	// for testing purposes; adds 2 dummy lines into the list
	StupidWin::postMessage(this, WM_SPEAKER, SPEAK_SEARCH, (LPARAM)new tstring(_T("search 1")));
	StupidWin::postMessage(this, WM_SPEAKER, SPEAK_SEARCH, (LPARAM)new tstring(_T("search 2")));
#endif
}

SpyFrame::~SpyFrame() {
}

void SpyFrame::layout() {
	const int border = 2;

	SmartWin::Rectangle r(this->getClientAreaSize());
	status->refresh();

	{
		std::vector<unsigned> w(STATUS_LAST);

		w[STATUS_STATUS] = status->getSize().x - std::accumulate(statusSizes, statusSizes+STATUS_LAST, 0) - w[STATUS_STATUS];
		std::copy(statusSizes, statusSizes + STATUS_LAST, w.begin());

		status->setSections(w);

		RECT sr;
		::SendMessage(status->handle(), SB_GETRECT, STATUS_IGNORE_TTH, reinterpret_cast<LPARAM>(&sr));
		::MapWindowPoints(status->handle(), this->handle(), (POINT*)&sr, 2);
		ignoreTTH->setBounds(SmartWin::Rectangle::FromRECT(sr));
	}

	r.size.y -= status->getSize().y - border;
	searches->setBounds(r);
}

bool SpyFrame::preClosing() {
	ClientManager::getInstance()->removeListener(this);
	TimerManager::getInstance()->removeListener(this);
	return true;
}

void SpyFrame::postClosing() {
	searches->removeAllRows();

	SettingsManager::getInstance()->set(SettingsManager::SPYFRAME_ORDER, WinUtil::toString(searches->getColumnOrder()));
	SettingsManager::getInstance()->set(SettingsManager::SPYFRAME_WIDTHS, WinUtil::toString(searches->getColumnWidths()));

	SettingsManager::getInstance()->set(SettingsManager::SPY_FRAME_IGNORE_TTH_SEARCHES, bIgnoreTTH);
}

HRESULT SpyFrame::spoken(LPARAM lParam, WPARAM wParam) {
	if(wParam == SPEAK_SEARCH) {
		tstring* x = (tstring*)lParam;

		total++;

		// Not thread safe, but who cares really...
		perSecond[cur]++;

		int j = searches->findItem(*x);
		if(j == -1) {
			TStringList a;
			a.push_back(*x);
			a.push_back(Text::toT(Util::toString(1)));
			a.push_back(Text::toT(Util::getTimeString()));
			searches->insertRow(a);
			if(searches->getRowCount() > 500) {
				searches->removeRow(searches->getRowCount() - 1);
			}
		} else {
			searches->setCellText(COLUMN_COUNT, j, Text::toT(Util::toString(Util::toInt(Text::fromT(searches->getCellText(COLUMN_COUNT, j))) + 1)));
			searches->setCellText(COLUMN_TIME, j, Text::toT(Util::getTimeString()));
#ifdef PORT_ME
			if(searches->getSortColumn() == COLUMN_COUNT )
				searches->resort();
			if(searches->getSortColumn() == COLUMN_TIME )
				searches->resort();
#endif
		}
		delete x;

		setStatus(STATUS_TOTAL, Text::toT(STRING(TOTAL) + Util::toString(total)));
		setStatus(STATUS_HITS, Text::toT(STRING(HITS) + Util::toString(ShareManager::getInstance()->getHits())));
		double ratio = total > 0 ? ((double)ShareManager::getInstance()->getHits()) / (double)total : 0.0;
		setStatus(STATUS_HIT_RATIO, Text::toT(STRING(HIT_RATIO) + Util::toString(ratio)));
	} else if(wParam == SPEAK_TICK_AVG) {
		float* x = (float*)lParam;
		setStatus(STATUS_AVG_PER_SECOND, Text::toT(STRING(AVERAGE) + Util::toString(*x)));
		delete x;
	}
	return 0;
}

HRESULT SpyFrame::handleColumnClick(DataGridMessageType, LPARAM lParam, WPARAM /*wParam*/) {
	LPNMLISTVIEW l = (LPNMLISTVIEW)lParam;
#ifdef PORT_ME
	if(l->iSubItem == searches->getSortColumn()) {
		if (!searches->isAscending())
			searches->setSort(-1, searches->getSortType());
		else
			searches->setSortDirection(false);
	} else {
		if(l->iSubItem == COLUMN_COUNT) {
			searches->setSort(l->iSubItem, ExListViewCtrl::SORT_INT);
		} else {
			searches->setSort(l->iSubItem, ExListViewCtrl::SORT_STRING_NOCASE);
		}
	}
#endif
	return 0;
}

HRESULT SpyFrame::handleContextMenu(DataGridMessageType, LPARAM lParam, WPARAM /*wParam*/) {
	if(searches->getSelectedCount() == 1) {
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

		if(pt.x == -1 && pt.y == -1) {
			pt = searches->getContextMenuPos();
		}

		searchString = searches->getCellText(COLUMN_STRING, searches->getSelectedIndex());

		contextMenu->trackPopupMenu(this, pt.x, pt.y, TPM_LEFTALIGN | TPM_RIGHTBUTTON);
		return TRUE;
	}
	return FALSE;
}

void SpyFrame::handleSearch(WidgetMenuPtr /*menu*/, unsigned /*id*/) {
#ifdef PORT_ME
	if(Util::strnicmp(searchString.c_str(), _T("TTH:"), 4) == 0)
		SearchFrame::openWindow(searchString.substr(4), 0, SearchManager::SIZE_DONTCARE, SearchManager::TYPE_TTH);
	else
		SearchFrame::openWindow(searchString);
#endif
}

void SpyFrame::handleIgnoreTTHClicked(WidgetCheckBoxPtr) {
	bIgnoreTTH = ignoreTTH->getChecked();
}

void SpyFrame::setStatus(Status s, const tstring& text) {
	int w = status->getTextSize(text).x + 12;
	if(w > static_cast<int>(statusSizes[s])) {
		dcdebug("Setting status size %d to %d\n", s, w);
		statusSizes[s] = w;
		layout();
	}
	status->setText(text, s);
}

void SpyFrame::on(ClientManagerListener::IncomingSearch, const string& s) throw() {
	if(bIgnoreTTH && s.compare(0, 4, "TTH:") == 0)
		return;
	tstring* x = new tstring(Text::toT(s));
	tstring::size_type i = 0;
	while( (i=x->find(_T('$'))) != string::npos) {
		(*x)[i] = _T(' ');
	}
	StupidWin::postMessage(this, WM_SPEAKER, SPEAK_SEARCH, (LPARAM)x);
}

void SpyFrame::on(TimerManagerListener::Second, uint32_t) throw() {
	float* f = new float(0.0);
	for(int i = 0; i < AVG_TIME; ++i) {
		(*f) += (float)perSecond[i];
	}
	(*f) /= AVG_TIME;

	cur = (cur + 1) % AVG_TIME;
	perSecond[cur] = 0;
	StupidWin::postMessage(this, WM_SPEAKER, SPEAK_TICK_AVG, (LPARAM)f);
}

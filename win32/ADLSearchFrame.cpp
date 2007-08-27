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

#include "ADLSearchFrame.h"

#include <dcpp/ResourceManager.h>
#include <dcpp/Client.h>
#include "HoldRedraw.h"
#include "ADLSProperties.h"

int ADLSearchFrame::columnIndexes[] = { COLUMN_ACTIVE_SEARCH_STRING, COLUMN_SOURCE_TYPE, COLUMN_DEST_DIR, COLUMN_MIN_FILE_SIZE, COLUMN_MAX_FILE_SIZE };
int ADLSearchFrame::columnSizes[] = { 120, 90, 90, 90, 90 };
static ResourceManager::Strings columnNames[] = { ResourceManager::ACTIVE_SEARCH_STRING,
	ResourceManager::SOURCE_TYPE, ResourceManager::DESTINATION, ResourceManager::MIN_SIZE, ResourceManager::MAX_SIZE,
};

ADLSearchFrame::ADLSearchFrame(SmartWin::WidgetMDIParent* mdiParent) :
	BaseType(mdiParent),
	add(0),
	properties(0),
	up(0),
	down(0),
	remove(0),
	help(0)
{
	{
		WidgetDataGrid::Seed cs;
		cs.style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS;
		cs.exStyle = WS_EX_CLIENTEDGE;
		items = createDataGrid(cs);
		items->setListViewStyle(LVS_EX_LABELTIP | LVS_EX_HEADERDRAGDROP | LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);
		items->setFont(WinUtil::font);
		addWidget(items);

		items->createColumns(ResourceManager::getInstance()->getStrings(columnNames));
		items->setColumnOrder(WinUtil::splitTokens(SETTING(ADLSEARCHFRAME_ORDER), columnIndexes));
		items->setColumnWidths(WinUtil::splitTokens(SETTING(ADLSEARCHFRAME_WIDTHS), columnSizes));
		items->setColor(WinUtil::textColor, WinUtil::bgColor);

		items->onDblClicked(std::tr1::bind(&ADLSearchFrame::handleDoubleClick, this));
		items->onKeyDown(std::tr1::bind(&ADLSearchFrame::handleKeyDown, this, _1));
		items->onRaw(std::tr1::bind(&ADLSearchFrame::handleItemChanged, this, _1, _2), SmartWin::Message(WM_NOTIFY, LVN_ITEMCHANGED));
	}

	{
		WidgetButton::Seed cs;
		cs.style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON;

		cs.caption = TSTRING(NEW);
		add = createButton(cs);
		add->onClicked(std::tr1::bind(&ADLSearchFrame::handleAdd, this));
		addWidget(add);

		cs.caption = TSTRING(PROPERTIES);
		properties = createButton(cs);
		properties->onClicked(std::tr1::bind(&ADLSearchFrame::handleProperties, this));
		addWidget(properties);

		cs.caption = TSTRING(MOVE_UP);
		up = createButton(cs);
		up->onClicked(std::tr1::bind(&ADLSearchFrame::handleUp, this));
		addWidget(up);

		cs.caption = TSTRING(MOVE_DOWN);
		down = createButton(cs);
		down->onClicked(std::tr1::bind(&ADLSearchFrame::handleDown, this));
		addWidget(down);

		cs.caption = TSTRING(REMOVE);
		remove = createButton(cs);
		remove->onClicked(std::tr1::bind(&ADLSearchFrame::handleRemove, this));
		addWidget(remove);

		cs.caption = TSTRING(MENU_HELP);
		help = createButton(cs);
		help->onClicked(std::tr1::bind(&ADLSearchFrame::handleHelp, this));
		addWidget(help);
	}

	initStatus();

	layout();

	// Load all searches
	ADLSearchManager::SearchCollection& collection = ADLSearchManager::getInstance()->collection;
	for(ADLSearchManager::SearchCollection::iterator i = collection.begin(); i != collection.end(); ++i)
		addEntry(*i);

	contextMenu = createMenu(true);
	contextMenu->appendItem(IDC_ADD, TSTRING(NEW), std::tr1::bind(&ADLSearchFrame::handleAdd, this));
	contextMenu->appendItem(IDC_EDIT, TSTRING(PROPERTIES), std::tr1::bind(&ADLSearchFrame::handleProperties, this));
	contextMenu->appendItem(IDC_REMOVE, TSTRING(REMOVE), std::tr1::bind(&ADLSearchFrame::handleRemove, this));
	items->onRaw(std::tr1::bind(&ADLSearchFrame::handleContextMenu, this, _1, _2), SmartWin::Message(WM_CONTEXTMENU));
}

ADLSearchFrame::~ADLSearchFrame() {
}

void ADLSearchFrame::layout() {
	const int border = 2;

	SmartWin::Rectangle r(SmartWin::Point(0, 0), getClientAreaSize());

	SmartWin::Rectangle rs = layoutStatus();
	r.size.y -= rs.size.y + border;

	/// @todo dynamic width
	const int ybutton = add->getTextSize(_T("A")).y + 10;
	const int xbutton = 90;
	const int xborder = 10;

	SmartWin::Rectangle rb(r.getBottom(ybutton));
	r.size.y -= ybutton;
	items->setBounds(r);

	rb.size.x = xbutton;
	add->setBounds(rb);

	rb.pos.x += xbutton + xborder;
	properties->setBounds(rb);

	rb.pos.x += xbutton + xborder;
	up->setBounds(rb);

	rb.pos.x += xbutton + xborder;
	down->setBounds(rb);

	rb.pos.x += xbutton + xborder;
	remove->setBounds(rb);

	rb.pos.x += xbutton + xborder;
	help->setBounds(rb);
}

bool ADLSearchFrame::preClosing() {
	ADLSearchManager::getInstance()->Save();
	return true;
}

void ADLSearchFrame::postClosing() {
	SettingsManager::getInstance()->set(SettingsManager::ADLSEARCHFRAME_ORDER, WinUtil::toString(items->getColumnOrder()));
	SettingsManager::getInstance()->set(SettingsManager::ADLSEARCHFRAME_WIDTHS, WinUtil::toString(items->getColumnWidths()));
}

void ADLSearchFrame::handleAdd() {
	ADLSearch search;
	ADLSProperties dlg(this, &search);
	if(dlg.run() == IDOK) {
		ADLSearchManager::SearchCollection& collection = ADLSearchManager::getInstance()->collection;

		int index;

		// Add new search to the end or if selected, just before
		if(items->getSelectedCount() == 1) {
			index = items->getSelectedIndex();
			collection.insert(collection.begin() + index, search);
		} else {
			index = -1;
			collection.push_back(search);
		}

		addEntry(search, index);
	}
}

void ADLSearchFrame::handleProperties() {
	// Get selection info
	std::vector<unsigned> selected = items->getSelectedRows();
	for(std::vector<unsigned>::const_iterator i = selected.begin(); i != selected.end(); ++i) {
		// Edit existing
		ADLSearchManager::SearchCollection& collection = ADLSearchManager::getInstance()->collection;
		ADLSearch search = collection[*i];

		// Invoke dialog with selected search
		ADLSProperties dlg(this, &search);
		if(dlg.run() == IDOK) {
			// Update search collection
			collection[*i] = search;

			// Update list control
			HoldRedraw hold(items);
			items->removeRow(*i);
			addEntry(search, *i);
			items->selectRow(*i);
		}
	}
}

void ADLSearchFrame::handleUp() {
	ADLSearchManager::SearchCollection& collection = ADLSearchManager::getInstance()->collection;
	HoldRedraw hold(items);
	std::vector<unsigned> selected = items->getSelectedRows();
	for(std::vector<unsigned>::const_iterator i = selected.begin(); i != selected.end(); ++i) {
		if(*i > 0) {
			ADLSearch search = collection[*i];
			swap(collection[*i], collection[*i - 1]);
			items->removeRow(*i);
			addEntry(search, *i - 1);
			items->selectRow(*i - 1);
		}
	}
}

void ADLSearchFrame::handleDown() {
	ADLSearchManager::SearchCollection& collection = ADLSearchManager::getInstance()->collection;
	HoldRedraw hold(items);
	std::vector<unsigned> selected = items->getSelectedRows();
	for(std::vector<unsigned>::reverse_iterator i = selected.rbegin(); i != selected.rend(); ++i) {
		if(*i < items->getRowCount() - 1) {
			ADLSearch search = collection[*i];
			swap(collection[*i], collection[*i + 1]);
			items->removeRow(*i);
			addEntry(search, *i + 1);
			items->selectRow(*i + 1);
		}
	}
}

void ADLSearchFrame::handleRemove() {
	ADLSearchManager::SearchCollection& collection = ADLSearchManager::getInstance()->collection;
	int i;
	while((i = items->getNextItem(-1, LVNI_SELECTED)) != -1) {
		collection.erase(collection.begin() + i);
		items->removeRow(i);
	}
}

void ADLSearchFrame::handleHelp() {
#ifdef PORT_ME
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDR_ADLSEARCH);
#endif
}

void ADLSearchFrame::handleDoubleClick() {
	if(items->hasSelection()) {
		handleProperties();
	} else {
		handleAdd();
	}
}

bool ADLSearchFrame::handleKeyDown(int c) {
	switch(c) {
	case VK_INSERT:
		handleAdd();
		return true;
	case VK_DELETE:
		handleRemove();
		return true;
#ifdef PORT_ME // pressing enter doesn't do anything
	case VK_RETURN:
		handleProperties();
		return true;
#endif
	}
	return false;
}

LRESULT ADLSearchFrame::handleItemChanged(WPARAM /*wParam*/, LPARAM lParam) {
	LPNMITEMACTIVATE item = reinterpret_cast<LPNMITEMACTIVATE>(lParam);

	if((item->uChanged & LVIF_STATE) == 0)
		return 0;
	if((item->uOldState & INDEXTOSTATEIMAGEMASK(0xf)) == 0)
		return 0;
	if((item->uNewState & INDEXTOSTATEIMAGEMASK(0xf)) == 0)
		return 0;

	if(item->iItem >= 0)
	{
		// Set new active status check box
		ADLSearchManager::SearchCollection& collection = ADLSearchManager::getInstance()->collection;
		ADLSearch& search = collection[item->iItem];
		search.isActive = items->getIsRowChecked(item->iItem);
	}
	return 0;
}

LRESULT ADLSearchFrame::handleContextMenu(WPARAM /*wParam*/, LPARAM lParam) {
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

	if(pt.x == -1 && pt.y == -1) {
		pt = items->getContextMenuPos();
	}

	bool status = items->hasSelection();
	contextMenu->setItemEnabled(IDC_EDIT, status);
	contextMenu->setItemEnabled(IDC_REMOVE, status);

	contextMenu->trackPopupMenu(this, pt.x, pt.y, TPM_LEFTALIGN | TPM_RIGHTBUTTON);
	return TRUE;
}

void ADLSearchFrame::addEntry(ADLSearch& search, int index) {
	TStringList l;
	l.push_back(Text::toT(search.searchString));
	l.push_back(Text::toT(search.SourceTypeToDisplayString(search.sourceType)));
	l.push_back(Text::toT(search.destDir));
	l.push_back((search.minFileSize >= 0) ? Text::toT(Util::toString(search.minFileSize)) + _T(" ") + search.SizeTypeToDisplayString(search.typeFileSize) : Util::emptyStringT);
	l.push_back((search.maxFileSize >= 0) ? Text::toT(Util::toString(search.maxFileSize)) + _T(" ") + search.SizeTypeToDisplayString(search.typeFileSize) : Util::emptyStringT);
	int itemCount = items->insertRow(l, 0, index);
	items->setRowChecked((index == -1) ? itemCount : index, search.isActive);
}

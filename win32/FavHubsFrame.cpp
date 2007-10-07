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

#include "FavHubsFrame.h"

#include <dcpp/FavoriteManager.h>
#include <dcpp/version.h>
#include "HoldRedraw.h"
#include "HubFrame.h"
#include "FavHubProperties.h"

int FavHubsFrame::columnIndexes[] = { COLUMN_NAME, COLUMN_DESCRIPTION, COLUMN_NICK, COLUMN_PASSWORD, COLUMN_SERVER, COLUMN_USERDESCRIPTION };
int FavHubsFrame::columnSizes[] = { 200, 290, 125, 100, 100, 125 };
static ResourceManager::Strings columnNames[] = { ResourceManager::AUTO_CONNECT, ResourceManager::DESCRIPTION,
	ResourceManager::NICK, ResourceManager::PASSWORD, ResourceManager::SERVER, ResourceManager::USER_DESCRIPTION
};

FavHubsFrame::FavHubsFrame(SmartWin::WidgetTabView* mdiParent) : 
	BaseType(mdiParent),
	hubs(0),
	connect(0),
	add(0),
	properties(0),
	up(0),
	down(0),
	remove(0),
	nosave(false)
{
	{
		WidgetDataGrid::Seed cs;
		cs.style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_NOSORTHEADER;
		cs.exStyle = WS_EX_CLIENTEDGE;
		hubs = createDataGrid(cs);
		hubs->setListViewStyle(LVS_EX_CHECKBOXES | LVS_EX_LABELTIP | LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT);
		hubs->setFont(WinUtil::font);
		addWidget(hubs);

		hubs->createColumns(ResourceManager::getInstance()->getStrings(columnNames));
		hubs->setColumnOrder(WinUtil::splitTokens(SETTING(FAVHUBSFRAME_ORDER), columnIndexes));
		hubs->setColumnWidths(WinUtil::splitTokens(SETTING(FAVHUBSFRAME_WIDTHS), columnSizes));
		hubs->setColor(WinUtil::textColor, WinUtil::bgColor);

		hubs->onDblClicked(std::tr1::bind(&FavHubsFrame::handleDoubleClick, this));
		hubs->onKeyDown(std::tr1::bind(&FavHubsFrame::handleKeyDown, this, _1));
		hubs->onRaw(std::tr1::bind(&FavHubsFrame::handleItemChanged, this, _1, _2), SmartWin::Message(WM_NOTIFY, LVN_ITEMCHANGED));
	}

	{
		WidgetButton::Seed cs;
		cs.style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON;

		cs.caption = TSTRING(CONNECT);
		connect = createButton(cs);
		connect->onClicked(std::tr1::bind(&FavHubsFrame::openSelected, this));
		addWidget(connect);

		cs.caption = TSTRING(NEW);
		add = createButton(cs);
		add->onClicked(std::tr1::bind(&FavHubsFrame::handleAdd, this));
		addWidget(add);

		cs.caption = TSTRING(PROPERTIES);
		properties = createButton(cs);
		properties->onClicked(std::tr1::bind(&FavHubsFrame::handleProperties, this));
		addWidget(properties);

		cs.caption = TSTRING(MOVE_UP);
		up = createButton(cs);
		up->onClicked(std::tr1::bind(&FavHubsFrame::handleUp, this));
		addWidget(up);

		cs.caption = TSTRING(MOVE_DOWN);
		down = createButton(cs);
		down->onClicked(std::tr1::bind(&FavHubsFrame::handleDown, this));
		addWidget(down);

		cs.caption = TSTRING(REMOVE);
		remove = createButton(cs);
		remove->onClicked(std::tr1::bind(&FavHubsFrame::handleRemove, this));
		addWidget(remove);
	}

	initStatus();

	layout();

	const FavoriteHubEntryList& fl = FavoriteManager::getInstance()->getFavoriteHubs();
	for(FavoriteHubEntryList::const_iterator i = fl.begin(); i != fl.end(); ++i)
		addEntry(*i);

	FavoriteManager::getInstance()->addListener(this);

	hubsMenu = createMenu(true);
	hubsMenu->appendItem(IDC_CONNECT, TSTRING(CONNECT), std::tr1::bind(&FavHubsFrame::openSelected, this));
	hubsMenu->appendSeparatorItem();
	hubsMenu->appendItem(IDC_NEWFAV, TSTRING(NEW), std::tr1::bind(&FavHubsFrame::handleAdd, this));
	hubsMenu->appendItem(IDC_EDIT, TSTRING(PROPERTIES), std::tr1::bind(&FavHubsFrame::handleProperties, this));
	hubsMenu->appendItem(IDC_MOVE_UP, TSTRING(MOVE_UP), std::tr1::bind(&FavHubsFrame::handleUp, this));
	hubsMenu->appendItem(IDC_MOVE_DOWN, TSTRING(MOVE_DOWN), std::tr1::bind(&FavHubsFrame::handleDown, this));
	hubsMenu->appendSeparatorItem();
	hubsMenu->appendItem(IDC_REMOVE, CTSTRING(REMOVE), std::tr1::bind(&FavHubsFrame::handleRemove, this));
	hubsMenu->setDefaultItem(IDC_CONNECT);
	hubs->onRaw(std::tr1::bind(&FavHubsFrame::handleContextMenu, this, _1, _2), SmartWin::Message(WM_CONTEXTMENU));
}

FavHubsFrame::~FavHubsFrame() {
	
}

void FavHubsFrame::layout() {
	const int border = 2;
	
	SmartWin::Rectangle r(getClientAreaSize());

	layoutStatus(r);

	/// @todo dynamic width
	const int ybutton = add->getTextSize(_T("A")).y + 10;
	const int xbutton = 90;
	const int xborder = 10;

	SmartWin::Rectangle rb(r.getBottom(ybutton));
	r.size.y -= ybutton;
	hubs->setBounds(r);

	rb.size.x = xbutton;
	connect->setBounds(rb);

	rb.pos.x += xbutton + xborder;
	add->setBounds(rb);

	rb.pos.x += xbutton + xborder;
	properties->setBounds(rb);

	rb.pos.x += xbutton + xborder;
	up->setBounds(rb);

	rb.pos.x += xbutton + xborder;
	down->setBounds(rb);

	rb.pos.x += xbutton + xborder;
	remove->setBounds(rb);
}

bool FavHubsFrame::preClosing() {
	FavoriteManager::getInstance()->removeListener(this);
	return true;
}

void FavHubsFrame::postClosing() {
	SettingsManager::getInstance()->set(SettingsManager::FAVHUBSFRAME_ORDER, WinUtil::toString(hubs->getColumnOrder()));
	SettingsManager::getInstance()->set(SettingsManager::FAVHUBSFRAME_WIDTHS, WinUtil::toString(hubs->getColumnWidths()));
}

void FavHubsFrame::handleAdd() {
	FavoriteHubEntry e;
	FavHubProperties dlg(this, &e);

	while(true) {
		if(dlg.run() == IDOK) {
			if(FavoriteManager::getInstance()->isFavoriteHub(e.getServer())) {
				createMessageBox().show(TSTRING(FAVORITE_HUB_ALREADY_EXISTS), _T(APPNAME) _T(" ") _T(VERSIONSTRING), WidgetMessageBox::BOX_OK, WidgetMessageBox::BOX_ICONEXCLAMATION);
			} else {
				FavoriteManager::getInstance()->addFavorite(e);
				break;
			}
		} else
			break;
	}
}

void FavHubsFrame::handleProperties() {
	if(hubs->getSelectedCount() == 1) {
		int i = hubs->getSelectedIndex();
		FavoriteHubEntryPtr e = reinterpret_cast<FavoriteHubEntryPtr>(hubs->getData(i));
		dcassert(e != NULL);
		FavHubProperties dlg(this, e);
		if(dlg.run() == IDOK) {
			hubs->setText(COLUMN_NAME, i, Text::toT(e->getName()));
			hubs->setText(COLUMN_DESCRIPTION, i, Text::toT(e->getDescription()));
			hubs->setText(COLUMN_SERVER, i, Text::toT(e->getServer()));
			hubs->setText(COLUMN_NICK, i, Text::toT(e->getNick(false)));
			hubs->setText(COLUMN_PASSWORD, i, tstring(e->getPassword().size(), '*'));
			hubs->setText(COLUMN_USERDESCRIPTION, i, Text::toT(e->getUserDescription()));
		}
	}
}

void FavHubsFrame::handleUp() {
	nosave = true;
	FavoriteHubEntryList& fh = FavoriteManager::getInstance()->getFavoriteHubs();
	HoldRedraw hold(hubs);
	std::vector<unsigned> selected = hubs->getSelected();
	for(std::vector<unsigned>::const_iterator i = selected.begin(); i != selected.end(); ++i) {
		if(*i > 0) {
			FavoriteHubEntryPtr e = fh[*i];
			swap(fh[*i], fh[*i - 1]);
			hubs->erase(*i);
			addEntry(e, *i - 1);
			hubs->select(*i - 1);
		}
	}
	FavoriteManager::getInstance()->save();
	nosave = false;
}

void FavHubsFrame::handleDown() {
	nosave = true;
	FavoriteHubEntryList& fh = FavoriteManager::getInstance()->getFavoriteHubs();
	HoldRedraw hold(hubs);
	std::vector<unsigned> selected = hubs->getSelected();
	for(std::vector<unsigned>::reverse_iterator i = selected.rbegin(); i != selected.rend(); ++i) {
		if(*i < hubs->size() - 1) {
			FavoriteHubEntryPtr e = fh[*i];
			swap(fh[*i], fh[*i + 1]);
			hubs->erase(*i);
			addEntry(e, *i + 1);
			hubs->select(*i + 1);
		}
	}
	FavoriteManager::getInstance()->save();
	nosave = false;
}

void FavHubsFrame::handleRemove() {
	if(hubs->hasSelection() && (!BOOLSETTING(CONFIRM_HUB_REMOVAL) || createMessageBox().show(TSTRING(REALLY_REMOVE), _T(APPNAME) _T(" ") _T(VERSIONSTRING), WidgetMessageBox::BOX_YESNO, WidgetMessageBox::BOX_ICONQUESTION) == WidgetMessageBox::RETBOX_YES)) {
		int i;
		while((i = hubs->getNext(-1, LVNI_SELECTED)) != -1)
			FavoriteManager::getInstance()->removeFavorite(reinterpret_cast<FavoriteHubEntryPtr>(hubs->getData(i)));
	}
}

void FavHubsFrame::handleDoubleClick() {
	if(hubs->hasSelection()) {
		openSelected();
	} else {
		handleAdd();
	}
}

bool FavHubsFrame::handleKeyDown(int c) {
	switch(c) {
	case VK_INSERT:
		handleAdd();
		return true;
	case VK_DELETE:
		handleRemove();
		return true;
	case VK_RETURN:
		openSelected();
		return true;
	}
	return false;
}

LRESULT FavHubsFrame::handleItemChanged(WPARAM /*wParam*/, LPARAM lParam) {
	LPNMITEMACTIVATE l = reinterpret_cast<LPNMITEMACTIVATE>(lParam);
	if(!nosave && l->iItem != -1 && ((l->uNewState & LVIS_STATEIMAGEMASK) != (l->uOldState & LVIS_STATEIMAGEMASK))) {
		FavoriteHubEntryPtr f = reinterpret_cast<FavoriteHubEntryPtr>(hubs->getData(l->iItem));
		f->setConnect(hubs->isChecked(l->iItem));
		FavoriteManager::getInstance()->save();
	}
	return 0;
}

LRESULT FavHubsFrame::handleContextMenu(WPARAM /*wParam*/, LPARAM lParam) {
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

	if(pt.x == -1 && pt.y == -1) {
		pt = hubs->getContextMenuPos();
	}

	bool status = hubs->hasSelection();
	hubsMenu->setItemEnabled(IDC_CONNECT, status);
	hubsMenu->setItemEnabled(IDC_EDIT, status);
	hubsMenu->setItemEnabled(IDC_MOVE_UP, status);
	hubsMenu->setItemEnabled(IDC_MOVE_DOWN, status);
	hubsMenu->setItemEnabled(IDC_REMOVE, status);

	hubsMenu->trackPopupMenu(this, pt.x, pt.y, TPM_LEFTALIGN | TPM_RIGHTBUTTON);
	return TRUE;
}

void FavHubsFrame::addEntry(const FavoriteHubEntryPtr entry, int index) {
	TStringList l;
	l.push_back(Text::toT(entry->getName()));
	l.push_back(Text::toT(entry->getDescription()));
	l.push_back(Text::toT(entry->getNick(false)));
	l.push_back(tstring(entry->getPassword().size(), '*'));
	l.push_back(Text::toT(entry->getServer()));
	l.push_back(Text::toT(entry->getUserDescription()));
	bool b = entry->getConnect();
	int i = hubs->insert(l, reinterpret_cast<LPARAM>(entry), index);
	hubs->setChecked(i, b);
}

void FavHubsFrame::openSelected() {
	if(!hubs->hasSelection())
		return;

	if(SETTING(NICK).empty()) {
		createMessageBox().show(TSTRING(ENTER_NICK), _T(APPNAME) _T(" ") _T(VERSIONSTRING), WidgetMessageBox::BOX_OK, WidgetMessageBox::BOX_ICONSTOP);
		return;
	}

	std::vector<unsigned> items = hubs->getSelected();
	for(std::vector<unsigned>::iterator i = items.begin(); i != items.end(); ++i) {
		FavoriteHubEntryPtr entry = reinterpret_cast<FavoriteHubEntryPtr>(hubs->getData(*i));
		HubFrame::openWindow(getParent(), entry->getServer());
	}
}

void FavHubsFrame::on(FavoriteAdded, const FavoriteHubEntryPtr e) throw() {
	addEntry(e);
}

void FavHubsFrame::on(FavoriteRemoved, const FavoriteHubEntryPtr e) throw() {
	hubs->erase(hubs->findData(reinterpret_cast<LPARAM>(e)));
}

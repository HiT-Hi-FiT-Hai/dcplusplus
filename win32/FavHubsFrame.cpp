/*
 * Copyright (C) 2001-2006 Jacek Sieka, arnetheduck on gmail point com
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
#include "HubFrame.h"

#include <dcpp/ResourceManager.h>
#include <dcpp/FavoriteManager.h>
#include <dcpp/version.h>

int FavHubsFrame::columnIndexes[] = { COLUMN_NAME, COLUMN_DESCRIPTION, COLUMN_NICK, COLUMN_PASSWORD, COLUMN_SERVER, COLUMN_USERDESCRIPTION };
int FavHubsFrame::columnSizes[] = { 200, 290, 125, 100, 100, 125 };
static ResourceManager::Strings columnNames[] = { ResourceManager::AUTO_CONNECT, ResourceManager::DESCRIPTION,
	ResourceManager::NICK, ResourceManager::PASSWORD, ResourceManager::SERVER, ResourceManager::USER_DESCRIPTION
};

FavHubsFrame::FavHubsFrame(SmartWin::WidgetMDIParent* mdiParent) : 
	BaseType(mdiParent),
	connect(0),
	add(0),
	remove(0),
	properties(0),
	up(0),
	down(0),
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
	}
	
	{
		WidgetButton::Seed cs;
		cs.style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON;

		cs.caption = TSTRING(CONNECT);
		connect = createButton(cs);
		connect->onClicked(std::tr1::bind(&FavHubsFrame::handleConnect, this));
		addWidget(connect);
		
		cs.caption = TSTRING(NEW);
		add = createButton(cs);
		add->onClicked(std::tr1::bind(&FavHubsFrame::handleAdd, this));
		addWidget(add);
		
		cs.caption = TSTRING(REMOVE);
		remove = createButton(cs);
		remove->onClicked(std::tr1::bind(&FavHubsFrame::handleRemove, this));
		addWidget(remove);
		
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
	}

	initStatus();
	statusSizes[STATUS_DUMMY] = 16;

	layout();
	
	const FavoriteHubEntry::List& fl = FavoriteManager::getInstance()->getFavoriteHubs();
	for(FavoriteHubEntry::List::const_iterator i = fl.begin(); i != fl.end(); ++i) {
		addEntry(*i, hubs->getRowCount());
	}

	FavoriteManager::getInstance()->addListener(this);
}

FavHubsFrame::~FavHubsFrame() {
	
}

void FavHubsFrame::layout() {
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
	hubs->setBounds(r);
	
	rb.size.x = xbutton;
	connect->setBounds(rb);
	
	rb.pos.x += xbutton + xborder;
	add->setBounds(rb);

	rb.pos.x += xbutton + xborder;
	remove->setBounds(rb);
	
	rb.pos.x += xbutton + xborder;
	properties->setBounds(rb);
	
	rb.pos.x += xbutton + xborder;
	up->setBounds(rb);
	
	rb.pos.x += xbutton + xborder;
	down->setBounds(rb);
}

void FavHubsFrame::addEntry(const FavoriteHubEntryPtr entry, int pos) {
	TStringList l;
	l.push_back(Text::toT(entry->getName()));
	l.push_back(Text::toT(entry->getDescription()));
	l.push_back(Text::toT(entry->getNick(false)));
	l.push_back(tstring(entry->getPassword().size(), '*'));
	l.push_back(Text::toT(entry->getServer()));
	l.push_back(Text::toT(entry->getUserDescription()));
	bool b = entry->getConnect();
	int i = hubs->getRowNumberFromLParam(hubs->insertRow(l, reinterpret_cast<LPARAM>(entry), pos));
	hubs->setRowChecked(i, b);
}

void FavHubsFrame::openSelected() {
	if(!checkNick())
		return;
	std::vector<unsigned> items = hubs->getSelectedRows();
	for(std::vector<unsigned>::iterator i = items.begin(); i != items.end(); ++i) {
		FavoriteHubEntry* entry = (FavoriteHubEntry*)hubs->getItemData(*i);
		HubFrame::openWindow(getParent(), entry->getServer());
	}
}

void FavHubsFrame::handleConnect() {
	openSelected();
}

void FavHubsFrame::handleAdd() {
	FavoriteHubEntry e;
#ifdef PORT_ME
	FavHubProperties dlg(&e);

	while (true) {
		if(dlg.DoModal((HWND)*this) == IDOK) {
			if (FavoriteManager::getInstance()->isFavoriteHub(e.getServer())){
				MessageBox(
					CTSTRING(FAVORITE_HUB_ALREADY_EXISTS), _T(" "), MB_ICONWARNING | MB_OK);
			} else {
				FavoriteManager::getInstance()->addFavorite(e);
				break;
			}
		} else {
			break;
		}
	}
#endif	
}

void FavHubsFrame::handleRemove() {
	int i = -1;
	if(!BOOLSETTING(CONFIRM_HUB_REMOVAL) || createMessageBox().show(TSTRING(REALLY_REMOVE), _T(APPNAME) _T(" ") _T(VERSIONSTRING), WidgetMessageBox::BOX_YESNO, WidgetMessageBox::BOX_ICONQUESTION) == IDYES) {
		while( (i = hubs->getNextItem(-1, LVNI_SELECTED)) != -1) {
			FavoriteManager::getInstance()->removeFavorite((FavoriteHubEntry*)hubs->getItemData(i));
		}
	}
}

void FavHubsFrame::handleProperties() {
	int i = -1;
	if((i = hubs->getNextItem(i, LVNI_SELECTED)) != -1)
	{
		FavoriteHubEntry* e = (FavoriteHubEntry*)hubs->getItemData(i);
		dcassert(e != NULL);
#ifdef PORT_ME
		FavHubProperties dlg(e);
		if(dlg.DoModal(m_hWnd) == IDOK)
		{
			ctrlHubs.SetItemText(i, COLUMN_NAME, Text::toT(e->getName()).c_str());
			ctrlHubs.SetItemText(i, COLUMN_DESCRIPTION, Text::toT(e->getDescription()).c_str());
			ctrlHubs.SetItemText(i, COLUMN_SERVER, Text::toT(e->getServer()).c_str());
			ctrlHubs.SetItemText(i, COLUMN_NICK, Text::toT(e->getNick(false)).c_str());
			ctrlHubs.SetItemText(i, COLUMN_PASSWORD, tstring(e->getPassword().size(), '*').c_str());
			ctrlHubs.SetItemText(i, COLUMN_USERDESCRIPTION, Text::toT(e->getUserDescription()).c_str());
		}
#endif
	}
}

void FavHubsFrame::handleUp() {
#ifdef PORT_ME
	nosave = true;
	int j = ctrlHubs.GetItemCount();
	FavoriteHubEntry::List& fh = FavoriteManager::getInstance()->getFavoriteHubs();
	ctrlHubs.SetRedraw(FALSE);
	for(int i = 1; i < j; ++i) {
		if(ctrlHubs.GetItemState(i, LVIS_SELECTED)) {
			FavoriteHubEntry* e = fh[i];
			swap(fh[i], fh[i-1]);
			ctrlHubs.DeleteItem(i);
			addEntry(e, i-1);
			ctrlHubs.SetItemState(i-1, LVIS_SELECTED, LVIS_SELECTED);
			ctrlHubs.EnsureVisible(i-1, FALSE);
		}
	}
	ctrlHubs.SetRedraw(TRUE);
	nosave = false;
	FavoriteManager::getInstance()->save();
#endif
}

void FavHubsFrame::handleDown() {
#ifdef PORT_ME
	int j = ctrlHubs.GetItemCount() - 2;
	FavoriteHubEntry::List& fh = FavoriteManager::getInstance()->getFavoriteHubs();

	nosave = true;
	ctrlHubs.SetRedraw(FALSE);
	for(int i = j; i >= 0; --i) {
		if(ctrlHubs.GetItemState(i, LVIS_SELECTED)) {
			FavoriteHubEntry* e = fh[i];
			swap(fh[i], fh[i+1]);
			addEntry(e, i+2);
			ctrlHubs.SetItemState(i+2, LVIS_SELECTED, LVIS_SELECTED);
			ctrlHubs.DeleteItem(i);
			ctrlHubs.EnsureVisible(i, FALSE);
		}
	}
	ctrlHubs.SetRedraw(TRUE);
	nosave = false;
	FavoriteManager::getInstance()->save();
#endif
}

bool FavHubsFrame::checkNick() {
	if(SETTING(NICK).empty()) {
		createMessageBox().show(TSTRING(ENTER_NICK), _T(APPNAME) _T(" ") _T(VERSIONSTRING));
		return false;
	}
	return true;
}

void FavHubsFrame::on(FavoriteAdded, const FavoriteHubEntryPtr e) throw() {
	
}

void FavHubsFrame::on(FavoriteRemoved, const FavoriteHubEntryPtr e) throw() {
	
}

LRESULT FavHubsFrame::handleContextMenu(WPARAM wParam, LPARAM lParam) {
	if(reinterpret_cast<HWND>(wParam) == hubs->handle()) {
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

		if(pt.x == -1 && pt.y == -1) {
			pt = hubs->getContextMenuPos();
		}

		bool selected = hubs->hasSelection();
		WidgetMenuPtr menu = createMenu(true);
		if(selected) {
			menu->appendItem(IDC_CONNECT, TSTRING(CONNECT), std::tr1::bind(&FavHubsFrame::handleConnect, this));
			menu->appendSeparatorItem();
		}	
		menu->appendItem(IDC_NEWFAV, TSTRING(NEW), std::tr1::bind(&FavHubsFrame::handleAdd, this));

		if(selected) {
			menu->appendItem(IDC_EDIT, TSTRING(PROPERTIES), std::tr1::bind(&FavHubsFrame::handleProperties, this));
			menu->appendItem(IDC_MOVE_UP, TSTRING(MOVE_UP), std::tr1::bind(&FavHubsFrame::handleUp, this));
			menu->appendItem(IDC_MOVE_DOWN, TSTRING(MOVE_DOWN), std::tr1::bind(&FavHubsFrame::handleDown, this));
			menu->appendSeparatorItem();
			menu->appendItem(IDC_REMOVE, TSTRING(REMOVE), std::tr1::bind(&FavHubsFrame::handleRemove, this));
		}
		menu->setDefaultItem(IDC_CONNECT);

		menu->trackPopupMenu(this, pt.x, pt.y, TPM_LEFTALIGN | TPM_RIGHTBUTTON);

		return TRUE;
	}

	return FALSE;
}

void FavHubsFrame::handleDoubleClick() {
	if(hubs->hasSelection()) {
		openSelected();
	} else {
		handleAdd();
	}
}

#ifdef PORT_ME
LRESULT FavoriteHubsFrame::onKeyDown(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled) {
	NMLVKEYDOWN* kd = (NMLVKEYDOWN*) pnmh;
	switch(kd->wVKey) {
	case VK_INSERT:
		PostMessage(WM_COMMAND, IDC_NEWFAV, 0);
		break;
	case VK_DELETE:
		PostMessage(WM_COMMAND, IDC_REMOVE, 0);
		break;
	case VK_RETURN:
		PostMessage(WM_COMMAND, IDC_CONNECT, 0);
		break;
	default:
		bHandled = FALSE;
	}
	return 0;
}

LRESULT FavoriteHubsFrame::onItemChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NMITEMACTIVATE* l = (NMITEMACTIVATE*)pnmh;
	if(!nosave && l->iItem != -1 && ((l->uNewState & LVIS_STATEIMAGEMASK) != (l->uOldState & LVIS_STATEIMAGEMASK))) {
		FavoriteHubEntry* f = (FavoriteHubEntry*)ctrlHubs.GetItemData(l->iItem);
		f->setConnect(ctrlHubs.GetCheckState(l->iItem) != FALSE);
		FavoriteManager::getInstance()->save();
	}
	return 0;
}

#endif

bool FavHubsFrame::preClosing() {
	FavoriteManager::getInstance()->removeListener(this);

	SettingsManager::getInstance()->set(SettingsManager::FAVHUBSFRAME_ORDER, WinUtil::toString(hubs->getColumnOrder()));
	SettingsManager::getInstance()->set(SettingsManager::FAVHUBSFRAME_WIDTHS, WinUtil::toString(hubs->getColumnWidths()));
	return true;
}

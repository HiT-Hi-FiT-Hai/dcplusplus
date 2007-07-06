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
#include <dcpp/DCPlusPlus.h>

#include "FavHubsFrame.h"
#include "HubFrame.h"

#include <dcpp/ResourceManager.h>
#include <dcpp/FavoriteManager.h>

int FavHubsFrame::columnIndexes[] = { COLUMN_NAME, COLUMN_DESCRIPTION, COLUMN_NICK, COLUMN_PASSWORD, COLUMN_SERVER, COLUMN_USERDESCRIPTION };
int FavHubsFrame::columnSizes[] = { 200, 290, 125, 100, 100, 125 };
static ResourceManager::Strings columnNames[] = { ResourceManager::AUTO_CONNECT, ResourceManager::DESCRIPTION,
	ResourceManager::NICK, ResourceManager::PASSWORD, ResourceManager::SERVER, ResourceManager::USER_DESCRIPTION
};

FavHubsFrame::FavHubsFrame(SmartWin::Widget* mdiParent) : 
	SmartWin::Widget(mdiParent),
	BaseType(mdiParent),
	connect(0),
	add(0),
	remove(0),
	properties(0),
	up(0),
	down(0) 
{
	{
		WidgetDataGrid::Seed cs;
		cs.style = WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_NOSORTHEADER;
		cs.exStyle = WS_EX_CLIENTEDGE;
		hubs = createDataGrid(cs);
		hubs->setListViewStyle(LVS_EX_LABELTIP | LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT);
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
		cs.style = WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON;

		cs.caption = TSTRING(CONNECT);
		connect = createButton(cs);
		connect->onClicked(&FavHubsFrame::handleConnect);
		addWidget(connect);
		
		cs.caption = TSTRING(NEW);
		add = createButton(cs);
		add->onClicked(&FavHubsFrame::handleAdd);
		addWidget(add);
		
		cs.caption = TSTRING(REMOVE);
		remove = createButton(cs);
		remove->onClicked(&FavHubsFrame::handleRemove);
		addWidget(remove);
		
		cs.caption = TSTRING(PROPERTIES);
		properties = createButton(cs);
		properties->onClicked(&FavHubsFrame::handleProperties);
		addWidget(properties);
		
		cs.caption = TSTRING(MOVE_UP);
		up = createButton(cs);
		up->onClicked(&FavHubsFrame::handleUp);
		addWidget(up);
		
		cs.caption = TSTRING(MOVE_DOWN);
		down = createButton(cs);
		down->onClicked(&FavHubsFrame::handleDown);
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
	const int ybutton = add->getTextSize("A").y + 10;
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
#ifdef PORT_ME
	if(!checkNick())
		return;
#endif
	std::vector<unsigned> items = hubs->getSelectedRows();
	for(std::vector<unsigned>::iterator i = items.begin(); i != items.end(); ++i) {
		FavoriteHubEntry* entry = (FavoriteHubEntry*)hubs->getItemData(*i);
		HubFrame::openWindow(getParent(), Text::toT(entry->getServer()));
	}
}

void FavHubsFrame::handleConnect(WidgetButtonPtr) {
	openSelected();
}

void FavHubsFrame::handleAdd(WidgetButtonPtr) {
	
}

void FavHubsFrame::handleRemove(WidgetButtonPtr) {
	
}

void FavHubsFrame::handleProperties(WidgetButtonPtr) {
	
}

void FavHubsFrame::handleUp(WidgetButtonPtr) {
	
}

void FavHubsFrame::handleDown(WidgetButtonPtr) {
	
}

void FavHubsFrame::on(FavoriteAdded, const FavoriteHubEntryPtr e) throw() {
	
}

void FavHubsFrame::on(FavoriteRemoved, const FavoriteHubEntryPtr e) throw() {
	
}
#ifdef PORT_ME

#include "Resource.h"

#include "FavoritesFrm.h"
#include "HubFrame.h"
#include "FavHubProperties.h"

#include "../client/ClientManager.h"
#include "../client/StringTokenizer.h"
#include "../client/version.h"

LRESULT FavoriteHubsFrame::onCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	ctrlHubs.SetExtendedListViewStyle(LVS_EX_LABELTIP | LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES | LVS_EX_HEADERDRAGDROP);
	ctrlHubs.SetBkColor(WinUtil::bgColor);
	ctrlHubs.SetTextBkColor(WinUtil::bgColor);
	ctrlHubs.SetTextColor(WinUtil::textColor);

	hubsMenu.CreatePopupMenu();
	hubsMenu.AppendMenu(MF_STRING, IDC_CONNECT, CTSTRING(CONNECT));
	hubsMenu.AppendMenu(MF_STRING, IDC_NEWFAV, CTSTRING(NEW));
	hubsMenu.AppendMenu(MF_STRING, IDC_EDIT, CTSTRING(PROPERTIES));
	hubsMenu.AppendMenu(MF_STRING, IDC_MOVE_UP, CTSTRING(MOVE_UP));
	hubsMenu.AppendMenu(MF_STRING, IDC_MOVE_DOWN, CTSTRING(MOVE_DOWN));
	hubsMenu.AppendMenu(MF_SEPARATOR);
	hubsMenu.AppendMenu(MF_STRING, IDC_REMOVE, CTSTRING(REMOVE));
	hubsMenu.SetMenuDefaultItem(IDC_CONNECT);

	nosave = false;

	bHandled = FALSE;
	return TRUE;
}

LRESULT FavoriteHubsFrame::onContextMenu(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	if(reinterpret_cast<HWND>(wParam) == ctrlHubs) {
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

		if(pt.x == -1 && pt.y == -1) {
			WinUtil::getContextMenuPos(ctrlHubs, pt);
		}

		int status = ctrlHubs.GetSelectedCount() > 0 ? MFS_ENABLED : MFS_DISABLED;
		hubsMenu.EnableMenuItem(IDC_CONNECT, status);
		hubsMenu.EnableMenuItem(IDC_EDIT, status);
		hubsMenu.EnableMenuItem(IDC_MOVE_UP, status);
		hubsMenu.EnableMenuItem(IDC_MOVE_DOWN, status);
		hubsMenu.EnableMenuItem(IDC_REMOVE, status);

		hubsMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);

		return TRUE;
	}

	bHandled = FALSE;
	return FALSE;
}

#endif

void FavHubsFrame::handleDoubleClick() {
	if(hubs->hasSelection()) {
		openSelected();
	} else {
#ifdef PORT_ME
		PostMessage(WM_COMMAND, IDC_NEW, 0);
#endif
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

LRESULT FavoriteHubsFrame::onRemove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int i = -1;
	if(!BOOLSETTING(CONFIRM_HUB_REMOVAL) || MessageBox(CTSTRING(REALLY_REMOVE), _T(APPNAME) _T(" ") _T(VERSIONSTRING), MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDYES) {
		while( (i = ctrlHubs.GetNextItem(-1, LVNI_SELECTED)) != -1) {
			FavoriteManager::getInstance()->removeFavorite((FavoriteHubEntry*)ctrlHubs.GetItemData(i));
		}
	}
	return 0;
}

LRESULT FavoriteHubsFrame::onEdit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int i = -1;
	if((i = ctrlHubs.GetNextItem(i, LVNI_SELECTED)) != -1)
	{
		FavoriteHubEntry* e = (FavoriteHubEntry*)ctrlHubs.GetItemData(i);
		dcassert(e != NULL);
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
	}
	return 0;
}

LRESULT FavoriteHubsFrame::onNew(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	FavoriteHubEntry e;
	FavHubProperties dlg(&e);

	while (true) {
		if(dlg.DoModal((HWND)*this) == IDOK) {
			if (FavoriteManager::getInstance()->checkFavHubExists(e)){
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
	return 0;
}

bool FavoriteHubsFrame::checkNick() {
	if(SETTING(NICK).empty()) {
		MessageBox(CTSTRING(ENTER_NICK), _T(APPNAME) _T(" ") _T(VERSIONSTRING), MB_ICONSTOP | MB_OK);
		return false;
	}
	return true;
}

LRESULT FavoriteHubsFrame::onMoveUp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
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
	return 0;
}

LRESULT FavoriteHubsFrame::onMoveDown(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
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

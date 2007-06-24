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

#include "UsersFrame.h"

#include <client/FavoriteManager.h>
#include <client/ResourceManager.h>

int UsersFrame::columnIndexes[] = { COLUMN_NICK, COLUMN_HUB, COLUMN_SEEN, COLUMN_DESCRIPTION, COLUMN_CID };
int UsersFrame::columnSizes[] = { 200, 300, 150, 200, 125 };
static ResourceManager::Strings columnNames[] = { ResourceManager::AUTO_GRANT, ResourceManager::LAST_HUB, ResourceManager::LAST_SEEN, ResourceManager::DESCRIPTION, ResourceManager::CID };

UsersFrame::UserInfo::UserInfo(const FavoriteUser& u) : UserInfoBase(u.getUser()) {
	update(u);
}

void UsersFrame::UserInfo::update(const FavoriteUser& u) {
	columns[COLUMN_NICK] = Text::toT(u.getNick());
	columns[COLUMN_HUB] = user->isOnline() ? WinUtil::getHubNames(u.getUser()).first : Text::toT(u.getUrl());
	columns[COLUMN_SEEN] = user->isOnline() ? TSTRING(ONLINE) : Text::toT(Util::formatTime("%Y-%m-%d %H:%M", u.getLastSeen()));
	columns[COLUMN_DESCRIPTION] = Text::toT(u.getDescription());
	columns[COLUMN_CID] = Text::toT(u.getUser()->getCID().toBase32());
}

void UsersFrame::UserInfo::remove() { 
	FavoriteManager::getInstance()->removeFavoriteUser(user); 
}

UsersFrame::UsersFrame(SmartWin::Widget* mdiParent) : 
	SmartWin::Widget(mdiParent),
	users(0),
	startup(true)
{
	{
		WidgetUsers::Seed cs;
		
		cs.style = WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS;
		cs.exStyle =  WS_EX_CLIENTEDGE;
		users = SmartWin::WidgetCreator<WidgetUsers>::create(this, cs);
		users->setListViewStyle(LVS_EX_CHECKBOXES | LVS_EX_LABELTIP | LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT);
		users->setFont(WinUtil::font);
		add_widget(users);
		
		users->createColumns(ResourceManager::getInstance()->getStrings(columnNames));
		users->setColumnOrder(WinUtil::splitTokens(SETTING(HUBFRAME_ORDER), columnIndexes));
		users->setColumnWidths(WinUtil::splitTokens(SETTING(HUBFRAME_WIDTHS), columnSizes));
		users->setSortColumn(COLUMN_NICK);
	}
	
	FavoriteManager::FavoriteMap ul = FavoriteManager::getInstance()->getFavoriteUsers();
#ifdef PORT_ME
	ctrlUsers.SetRedraw(FALSE);
#endif
	for(FavoriteManager::FavoriteMap::iterator i = ul.begin(); i != ul.end(); ++i) {
		addUser(i->second);
	}
#ifdef PORT_ME
	ctrlUsers.SetRedraw(TRUE);
#endif
	FavoriteManager::getInstance()->addListener(this);

	startup = false;
	onSpeaker(&UsersFrame::spoken);

}

void UsersFrame::layout() {
	const int border = 2;
	
	SmartWin::Rectangle r(SmartWin::Point(0, 0), getClientAreaSize());

	SmartWin::Rectangle rs = layoutStatus();
	r.size.y -= rs.size.y + border;
}

void UsersFrame::addUser(const FavoriteUser& aUser) {
	int i = users->insertItem(new UserInfo(aUser));
	bool b = aUser.isSet(FavoriteUser::FLAG_GRANTSLOT);
#ifdef PORT_ME
	ctrlUsers.SetCheckState(i, b);
#endif
}

void UsersFrame::updateUser(const User::Ptr& aUser) {
	for(int i = 0; i < users->getRowCount(); ++i) {
		UserInfo *ui = users->getItemData(i);
		if(ui->user == aUser) {
			ui->columns[COLUMN_SEEN] = aUser->isOnline() ? TSTRING(ONLINE) : Text::toT(Util::formatTime("%Y-%m-%d %H:%M", FavoriteManager::getInstance()->getLastSeen(aUser)));
			users->updateItem(i);
		}
	}
}

void UsersFrame::removeUser(const FavoriteUser& aUser) {
	for(int i = 0; i < users->getRowCount(); ++i) {
		UserInfo *ui = users->getItemData(i);
		if(ui->user == aUser.getUser()) {
			users->removeRow(i);
			delete ui;
			return;
		}
	}
}

bool UsersFrame::preClosing() {
	FavoriteManager::getInstance()->removeListener(this);
	return true;
}

void UsersFrame::postClosing() {
	SettingsManager::getInstance()->set(SettingsManager::USERSFRAME_ORDER, WinUtil::toString(users->getColumnOrder()));
	SettingsManager::getInstance()->set(SettingsManager::USERSFRAME_WIDTHS, WinUtil::toString(users->getColumnWidths()));

	for(int i = 0; i < users->getRowCount(); ++i) {
		delete users->getItemData(i);
	}
}

HRESULT UsersFrame::spoken(LPARAM lp, WPARAM wp) {
	if(wp == USER_UPDATED) {
		UserInfoBase* uib = (UserInfoBase*)lp;
		updateUser(uib->user);
		delete uib;
	}
}

#ifdef PORT_ME

#include "../client/StringTokenizer.h"
#include "../client/ClientManager.h"

#include "LineDlg.h"

#include "HubFrame.h"

LRESULT UsersFrame::onCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{


	usersMenu.CreatePopupMenu();
	appendUserItems(usersMenu);
	usersMenu.AppendMenu(MF_SEPARATOR);
	usersMenu.AppendMenu(MF_STRING, IDC_EDIT, CTSTRING(PROPERTIES));
	usersMenu.AppendMenu(MF_STRING, IDC_REMOVE, CTSTRING(REMOVE));



	bHandled = FALSE;
	return TRUE;

}

LRESULT UsersFrame::onContextMenu(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	if (reinterpret_cast<HWND>(wParam) == ctrlUsers && ctrlUsers.GetSelectedCount() > 0 ) {
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

		if(pt.x == -1 && pt.y == -1) {
			WinUtil::getContextMenuPos(ctrlUsers, pt);
		}

		checkAdcItems(usersMenu);
		usersMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);

		return TRUE;
	}
	bHandled = FALSE;
	return FALSE;
}

LRESULT UsersFrame::onRemove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int i = -1;
	while( (i = ctrlUsers.GetNextItem(-1, LVNI_SELECTED)) != -1) {
		ctrlUsers.getItemData(i)->remove();
	}
	return 0;
}

LRESULT UsersFrame::onEdit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(ctrlUsers.GetSelectedCount() == 1) {
		int i = ctrlUsers.GetNextItem(-1, LVNI_SELECTED);
		UserInfo* ui = ctrlUsers.getItemData(i);
		dcassert(i != -1);
		LineDlg dlg;
		dlg.description = TSTRING(DESCRIPTION);
		dlg.title = ui->columns[COLUMN_NICK];
		dlg.line = ui->columns[COLUMN_DESCRIPTION];
		if(dlg.DoModal(m_hWnd)) {
			FavoriteManager::getInstance()->setUserDescription(ui->user, Text::fromT(dlg.line));
			ui->columns[COLUMN_DESCRIPTION] = dlg.line;
			ctrlUsers.updateItem(i);
		}
	}
	return 0;
}

LRESULT UsersFrame::onItemChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NMITEMACTIVATE* l = (NMITEMACTIVATE*)pnmh;
	if(!startup && l->iItem != -1 && ((l->uNewState & LVIS_STATEIMAGEMASK) != (l->uOldState & LVIS_STATEIMAGEMASK))) {
		FavoriteManager::getInstance()->setAutoGrant(ctrlUsers.getItemData(l->iItem)->user, ctrlUsers.GetCheckState(l->iItem) != FALSE);
	}
	return 0;
}

LRESULT UsersFrame::onDoubleClick(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled) {
	NMITEMACTIVATE* item = (NMITEMACTIVATE*) pnmh;

	if(item->iItem != -1) {
		PostMessage(WM_COMMAND, IDC_GETLIST, 0);
	} else {
		bHandled = FALSE;
	}

	return 0;
}

LRESULT UsersFrame::onKeyDown(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled) {
	NMLVKEYDOWN* kd = (NMLVKEYDOWN*) pnmh;
	switch(kd->wVKey) {
	case VK_DELETE:
		PostMessage(WM_COMMAND, IDC_REMOVE, 0);
		break;
	case VK_RETURN:
		PostMessage(WM_COMMAND, IDC_EDIT, 0);
		break;
	default:
		bHandled = FALSE;
	}
	return 0;
}

LRESULT UsersFrame::onConnect(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	for(int i = 0; i < ctrlUsers.GetItemCount(); ++i) {
		UserInfo *ui = ctrlUsers.getItemData(i);
		FavoriteManager::FavoriteMap favUsers = FavoriteManager::getInstance()->getFavoriteUsers();
		const FavoriteUser u = favUsers.find(ui->user->getCID())->second;
		if(u.getUrl().length() > 0)
		{
			HubFrame::openWindow(Text::toT(u.getUrl()));
		}
	}
	return 0;
}

#endif

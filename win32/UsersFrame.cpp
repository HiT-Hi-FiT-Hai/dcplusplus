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

#include "UsersFrame.h"
#include "LineDlg.h"
#include "HoldRedraw.h"

#include <dcpp/FavoriteManager.h>
#include <dcpp/ResourceManager.h>

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

UsersFrame::UsersFrame(SmartWin::WidgetTabView* mdiParent) : 
	BaseType(mdiParent),
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
		addWidget(users);
		
		users->createColumns(ResourceManager::getInstance()->getStrings(columnNames));
		users->setColumnOrder(WinUtil::splitTokens(SETTING(HUBFRAME_ORDER), columnIndexes));
		users->setColumnWidths(WinUtil::splitTokens(SETTING(HUBFRAME_WIDTHS), columnSizes));
		users->setSortColumn(COLUMN_NICK);
	}
	
	FavoriteManager::FavoriteMap ul = FavoriteManager::getInstance()->getFavoriteUsers();
	HoldRedraw hold(users);
	for(FavoriteManager::FavoriteMap::iterator i = ul.begin(); i != ul.end(); ++i) {
		addUser(i->second);
	}
	FavoriteManager::getInstance()->addListener(this);
	
	initStatus();
	layout();
	
	startup = false;
	onSpeaker(std::tr1::bind(&UsersFrame::handleSpeaker, this, _1, _2));

}

void UsersFrame::layout() {
	const int border = 2;
	
	SmartWin::Rectangle r(SmartWin::Point(0, 0), getClientAreaSize());

	layoutStatus(r);

	users->setBounds(r);
}

void UsersFrame::addUser(const FavoriteUser& aUser) {
	int i = users->insert(new UserInfo(aUser));
	bool b = aUser.isSet(FavoriteUser::FLAG_GRANTSLOT);
	users->setChecked(i, b);
}

void UsersFrame::updateUser(const UserPtr& aUser) {
	for(int i = 0; i < users->size(); ++i) {
		UserInfo *ui = users->getData(i);
		if(ui->user == aUser) {
			ui->columns[COLUMN_SEEN] = aUser->isOnline() ? TSTRING(ONLINE) : Text::toT(Util::formatTime("%Y-%m-%d %H:%M", FavoriteManager::getInstance()->getLastSeen(aUser)));
			users->update(i);
		}
	}
}

void UsersFrame::removeUser(const FavoriteUser& aUser) {
	for(int i = 0; i < users->size(); ++i) {
		UserInfo *ui = users->getData(i);
		if(ui->user == aUser.getUser()) {
			users->erase(i);
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

	for(int i = 0; i < users->size(); ++i) {
		delete users->getData(i);
	}
}

HRESULT UsersFrame::handleSpeaker(WPARAM wParam, LPARAM lParam) {
	if(wParam == USER_UPDATED) {
		boost::scoped_ptr<UserInfoBase> uib(reinterpret_cast<UserInfoBase*>(lParam));
		updateUser(uib->user);
	}
	return 0;
}

HRESULT UsersFrame::handleContextMenu(WPARAM wParam, LPARAM lParam) {
	if (reinterpret_cast<HWND>(wParam) == users->handle() && users->hasSelection()) {
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

		if(pt.x == -1 && pt.y == -1) {
			pt = users->getContextMenuPos();
		}

		WidgetMenuPtr menu = createMenu(true);
		appendUserItems(getParent(), menu);
		menu->appendSeparatorItem();
		menu->appendItem(IDC_EDIT, TSTRING(PROPERTIES), std::tr1::bind(&UsersFrame::handleProperties, this));
		menu->appendItem(IDC_REMOVE, TSTRING(REMOVE), std::tr1::bind(&UsersFrame::handleRemove, this));
		
		menu->trackPopupMenu(this, pt.x, pt.y, TPM_LEFTALIGN | TPM_RIGHTBUTTON);

		return TRUE;
	}
	return FALSE;
}

void UsersFrame::handleRemove() {
	users->forEachSelected(&UsersFrame::UserInfo::remove);
}

void UsersFrame::handleProperties() {
	if(users->getSelectedCount() == 1) {
		int i = users->getSelectedIndex();
		UserInfo* ui = users->getData(i);
		LineDlg dlg(this, ui->columns[COLUMN_NICK], TSTRING(DESCRIPTION), ui->columns[COLUMN_DESCRIPTION]);

		if(dlg.run() == IDOK) {
			FavoriteManager::getInstance()->setUserDescription(ui->user, Text::fromT(dlg.getLine()));
			ui->columns[COLUMN_DESCRIPTION] = dlg.getLine();
			users->update(i);
		}
	}
}

#ifdef PORT_ME

LRESULT UsersFrame::onItemChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NMITEMACTIVATE* l = (NMITEMACTIVATE*)pnmh;
	if(!startup && l->iItem != -1 && ((l->uNewState & LVIS_STATEIMAGEMASK) != (l->uOldState & LVIS_STATEIMAGEMASK))) {
		FavoriteManager::getInstance()->setAutoGrant(ctrlUsers.getData(l->iItem)->user, ctrlUsers.GetCheckState(l->iItem) != FALSE);
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
		UserInfo *ui = ctrlUsers.getData(i);
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

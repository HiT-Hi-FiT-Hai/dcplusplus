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

int UsersFrame::columnIndexes[] = { COLUMN_NICK, COLUMN_HUB, COLUMN_SEEN, COLUMN_DESCRIPTION, COLUMN_CID };
int UsersFrame::columnSizes[] = { 200, 300, 150, 200, 125 };
static const char* columnNames[] = {
	N_("Auto grant slot / Nick"),
	N_("Hub (last seen in, if offline)"),
	N_("Time last seen"),
	N_("Description"),
	N_("CID")
};

UsersFrame::UsersFrame(SmartWin::WidgetTabView* mdiParent) : 
	BaseType(mdiParent, T_("Favorite Users"), IDR_USERS),
	users(0),
	startup(true)
{
	{
		WidgetUsers::Seed cs = WinUtil::Seeds::listView;
		cs.lvStyle |= LVS_EX_CHECKBOXES;
		users = SmartWin::WidgetCreator<WidgetUsers>::create(this, cs);
		addWidget(users);

		users->createColumns(WinUtil::getStrings(columnNames));
		users->setColumnOrder(WinUtil::splitTokens(SETTING(HUBFRAME_ORDER), columnIndexes));
		users->setColumnWidths(WinUtil::splitTokens(SETTING(HUBFRAME_WIDTHS), columnSizes));
		users->setSort(COLUMN_NICK);
		users->setColor(WinUtil::textColor, WinUtil::bgColor);

		users->onDblClicked(std::tr1::bind(&UsersFrame::handleGetList, this));
		users->onKeyDown(std::tr1::bind(&UsersFrame::handleKeyDown, this, _1));
		users->onRaw(std::tr1::bind(&UsersFrame::handleItemChanged, this, _1, _2), SmartWin::Message(WM_NOTIFY, LVN_ITEMCHANGED));
		users->onContextMenu(std::tr1::bind(&UsersFrame::handleContextMenu, this, _1));
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

UsersFrame::~UsersFrame() {
}

void UsersFrame::layout() {
	SmartWin::Rectangle r(SmartWin::Point(0, 0), getClientAreaSize());

	layoutStatus(r);

	users->setBounds(r);
}

bool UsersFrame::preClosing() {
	FavoriteManager::getInstance()->removeListener(this);
	return true;
}

void UsersFrame::postClosing() {
	SettingsManager::getInstance()->set(SettingsManager::USERSFRAME_ORDER, WinUtil::toString(users->getColumnOrder()));
	SettingsManager::getInstance()->set(SettingsManager::USERSFRAME_WIDTHS, WinUtil::toString(users->getColumnWidths()));
}

UsersFrame::UserInfo::UserInfo(const FavoriteUser& u) : UserInfoBase(u.getUser()) {
	update(u);
}

void UsersFrame::UserInfo::remove() { 
	FavoriteManager::getInstance()->removeFavoriteUser(user); 
}

void UsersFrame::UserInfo::update(const FavoriteUser& u) {
	columns[COLUMN_NICK] = Text::toT(u.getNick());
	columns[COLUMN_HUB] = user->isOnline() ? WinUtil::getHubNames(u.getUser()).first : Text::toT(u.getUrl());
	columns[COLUMN_SEEN] = user->isOnline() ? T_("Online") : Text::toT(Util::formatTime("%Y-%m-%d %H:%M", u.getLastSeen()));
	columns[COLUMN_DESCRIPTION] = Text::toT(u.getDescription());
	columns[COLUMN_CID] = Text::toT(u.getUser()->getCID().toBase32());
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
			ui->columns[COLUMN_SEEN] = aUser->isOnline() ? T_("Online") : Text::toT(Util::formatTime("%Y-%m-%d %H:%M", FavoriteManager::getInstance()->getLastSeen(aUser)));
			users->update(i);
		}
	}
}

void UsersFrame::removeUser(const FavoriteUser& aUser) {
	for(int i = 0; i < users->size(); ++i) {
		UserInfo *ui = users->getData(i);
		if(ui->user == aUser.getUser()) {
			users->erase(i);
			return;
		}
	}
}

void UsersFrame::handleProperties() {
	if(users->getSelectedCount() == 1) {
		int i = users->getSelectedIndex();
		UserInfo* ui = users->getData(i);
		LineDlg dlg(this, ui->columns[COLUMN_NICK], T_("Description"), ui->columns[COLUMN_DESCRIPTION]);

		if(dlg.run() == IDOK) {
			FavoriteManager::getInstance()->setUserDescription(ui->user, Text::fromT(dlg.getLine()));
			ui->columns[COLUMN_DESCRIPTION] = dlg.getLine();
			users->update(i);
		}
	}
}

void UsersFrame::handleRemove() {
	users->forEachSelected(&UsersFrame::UserInfo::remove);
}

bool UsersFrame::handleKeyDown(int c) {
	switch(c) {
	case VK_DELETE:
		handleRemove();
		return true;
	case VK_RETURN:
		handleProperties();
		return true;
	}
	return false;
}

LRESULT UsersFrame::handleItemChanged(WPARAM /*wParam*/, LPARAM lParam) {
	LPNMITEMACTIVATE l = reinterpret_cast<LPNMITEMACTIVATE>(lParam);
	if(!startup && l->iItem != -1 && ((l->uNewState & LVIS_STATEIMAGEMASK) != (l->uOldState & LVIS_STATEIMAGEMASK))) {
		FavoriteManager::getInstance()->setAutoGrant(users->getData(l->iItem)->user, users->isChecked(l->iItem));
	}
	return 0;
}

bool UsersFrame::handleContextMenu(SmartWin::ScreenCoordinate pt) {
	if (users->hasSelection()) {
		if(pt.x() == -1 && pt.y() == -1) {
			pt = users->getContextMenuPos();
		}

		WidgetMenuPtr menu = createMenu(WinUtil::Seeds::menu);
		appendUserItems(getParent(), menu);
		menu->appendSeparatorItem();
		menu->appendItem(IDC_EDIT, T_("&Properties"), std::tr1::bind(&UsersFrame::handleProperties, this));
		menu->appendItem(IDC_REMOVE, T_("&Remove"), std::tr1::bind(&UsersFrame::handleRemove, this));
		
		menu->trackPopupMenu(this, pt, TPM_LEFTALIGN | TPM_RIGHTBUTTON);

		return true;
	}
	return false;
}

LRESULT UsersFrame::handleSpeaker(WPARAM wParam, LPARAM lParam) {
	if(wParam == USER_UPDATED) {
		boost::scoped_ptr<UserInfoBase> uib(reinterpret_cast<UserInfoBase*>(lParam));
		updateUser(uib->user);
	}
	return 0;
}

void UsersFrame::on(UserAdded, const FavoriteUser& aUser) throw() {
	addUser(aUser);
}

void UsersFrame::on(UserRemoved, const FavoriteUser& aUser) throw() {
	removeUser(aUser);
}

void UsersFrame::on(StatusChanged, const UserPtr& aUser) throw() {
	speak(USER_UPDATED, reinterpret_cast<LPARAM>(new UserInfoBase(aUser)));
}

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
#include "../client/DCPlusPlus.h"
#include "Resource.h"

#include "UsersFrame.h"

#include "../client/StringTokenizer.h"
#include "../client/ClientManager.h"

#include "LineDlg.h"

#include "HubFrame.h"

int UsersFrame::columnIndexes[] = { COLUMN_NICK, COLUMN_HUB, COLUMN_SEEN, COLUMN_DESCRIPTION, COLUMN_CID };
int UsersFrame::columnSizes[] = { 200, 300, 150, 200, 125 };
static ResourceManager::Strings columnNames[] = { ResourceManager::AUTO_GRANT, ResourceManager::LAST_HUB, ResourceManager::LAST_SEEN, ResourceManager::DESCRIPTION, ResourceManager::CID };

LRESULT UsersFrame::onCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	CreateSimpleStatusBar(ATL_IDS_IDLEMESSAGE, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SBARS_SIZEGRIP);
	ctrlStatus.Attach(m_hWndStatusBar);

	ctrlUsers.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS , WS_EX_CLIENTEDGE, IDC_USERS);
	ctrlUsers.SetExtendedListViewStyle(LVS_EX_LABELTIP | LVS_EX_HEADERDRAGDROP | LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);
	ctrlUsers.SetBkColor(WinUtil::bgColor);
	ctrlUsers.SetTextBkColor(WinUtil::bgColor);
	ctrlUsers.SetTextColor(WinUtil::textColor);
	
	// Create listview columns
	WinUtil::splitTokens(columnIndexes, SETTING(USERSFRAME_ORDER), COLUMN_LAST);
	WinUtil::splitTokens(columnSizes, SETTING(USERSFRAME_WIDTHS), COLUMN_LAST);
	
	for(int j=0; j<COLUMN_LAST; j++) {
		ctrlUsers.InsertColumn(j, CTSTRING_I(columnNames[j]), LVCFMT_LEFT, columnSizes[j], j);
	}
	
	ctrlUsers.SetColumnOrderArray(COLUMN_LAST, columnIndexes);
	ctrlUsers.setSortColumn(COLUMN_NICK);
	usersMenu.CreatePopupMenu();
	appendUserItems(usersMenu);
	usersMenu.AppendMenu(MF_SEPARATOR);
	usersMenu.AppendMenu(MF_STRING, IDC_EDIT, CTSTRING(PROPERTIES));
	usersMenu.AppendMenu(MF_STRING, IDC_REMOVE, CTSTRING(REMOVE));

	FavoriteManager::getInstance()->addListener(this);

	FavoriteManager::FavoriteMap ul = FavoriteManager::getInstance()->getFavoriteUsers();
	ctrlUsers.SetRedraw(FALSE);
	for(FavoriteManager::FavoriteMap::iterator i = ul.begin(); i != ul.end(); ++i) {
		addUser(i->second);
	}
	ctrlUsers.SetRedraw(TRUE);

	startup = false;

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

void UsersFrame::UpdateLayout(BOOL bResizeBars /* = TRUE */) {
	RECT rect;
	GetClientRect(&rect);
	// position bars and offset their dimensions
	UpdateBarsPosition(rect, bResizeBars);

	if(ctrlStatus.IsWindow()) {
		CRect sr;
		int w[3];
		ctrlStatus.GetClientRect(sr);
		int tmp = (sr.Width()) > 316 ? 216 : ((sr.Width() > 116) ? sr.Width()-100 : 16);

		w[0] = sr.right - tmp;
		w[1] = w[0] + (tmp-16)/2;
		w[2] = w[0] + (tmp-16);

		ctrlStatus.SetParts(3, w);
	}

	CRect rc = rect;
	ctrlUsers.MoveWindow(rc);
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

void UsersFrame::addUser(const FavoriteUser& aUser) {
	int i = ctrlUsers.insertItem(new UserInfo(aUser), 0);
	bool b = aUser.isSet(FavoriteUser::FLAG_GRANTSLOT);
	ctrlUsers.SetCheckState(i, b);
}

void UsersFrame::updateUser(const User::Ptr& aUser) {
	for(int i = 0; i < ctrlUsers.GetItemCount(); ++i) {
		UserInfo *ui = ctrlUsers.getItemData(i);
		if(ui->user == aUser) {
			ui->columns[COLUMN_SEEN] = aUser->isOnline() ? TSTRING(ONLINE) : Text::toT(Util::formatTime("%Y-%m-%d %H:%M", FavoriteManager::getInstance()->getLastSeen(aUser)));
			ctrlUsers.updateItem(i);
		}
	}
}

void UsersFrame::removeUser(const FavoriteUser& aUser) {
	for(int i = 0; i < ctrlUsers.GetItemCount(); ++i) {
		UserInfo *ui = ctrlUsers.getItemData(i);
		if(ui->user == aUser.getUser()) {
			ctrlUsers.DeleteItem(i);
			delete ui;
			return;
		}
	}
}

LRESULT UsersFrame::onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	if(!closed) {
		FavoriteManager::getInstance()->removeListener(this);

		closed = true;
		PostMessage(WM_CLOSE);
		return 0;
	} else {
		WinUtil::saveHeaderOrder(ctrlUsers, SettingsManager::USERSFRAME_ORDER, 
			SettingsManager::USERSFRAME_WIDTHS, COLUMN_LAST, columnIndexes, columnSizes);

		for(int i = 0; i < ctrlUsers.GetItemCount(); ++i) {
			delete ctrlUsers.getItemData(i);
		}

		bHandled = FALSE;
		return 0;
	}
}

void UsersFrame::UserInfo::update(const FavoriteUser& u) {
	columns[COLUMN_NICK] = Text::toT(u.getNick());
	columns[COLUMN_HUB] = user->isOnline() ? WinUtil::getHubNames(u.getUser()).first : Text::toT(u.getUrl());
	columns[COLUMN_SEEN] = user->isOnline() ? TSTRING(ONLINE) : Text::toT(Util::formatTime("%Y-%m-%d %H:%M", u.getLastSeen()));
	columns[COLUMN_DESCRIPTION] = Text::toT(u.getDescription());
	columns[COLUMN_CID] = Text::toT(u.getUser()->getCID().toBase32());
}

LRESULT UsersFrame::onSpeaker(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	if(wParam == USER_UPDATED) {
		UserInfoBase* uib = (UserInfoBase*)lParam;
		updateUser(uib->user);
		delete uib;
	}
	return 0;
}

/**
 * @file
 * $Id: UsersFrame.cpp,v 1.43 2006/01/01 17:49:59 arnetheduck Exp $
 */

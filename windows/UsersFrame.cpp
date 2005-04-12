/* 
 * Copyright (C) 2001-2005 Jacek Sieka, arnetheduck on gmail point com
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
#include "LineDlg.h"

int UsersFrame::columnIndexes[] = { COLUMN_NICK, COLUMN_STATUS, COLUMN_HUB, COLUMN_SEEN, COLUMN_DESCRIPTION };
int UsersFrame::columnSizes[] = { 200, 150, 300, 125, 200 };
static ResourceManager::Strings columnNames[] = { ResourceManager::AUTO_GRANT, ResourceManager::STATUS, ResourceManager::LAST_HUB, ResourceManager::LAST_SEEN, ResourceManager::DESCRIPTION };

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
	ClientManager::getInstance()->addListener(this);

	FavoriteUser::List ul = FavoriteManager::getInstance()->getFavoriteUsers();
	ctrlUsers.SetRedraw(FALSE);
	for(FavoriteUser::Iter i = ul.begin(); i != ul.end(); ++i) {
		/// @todo addUser(*i);
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
	ctrlUsers.forEachSelected(&UserInfo::remove);
	return 0;
}

LRESULT UsersFrame::onEdit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(ctrlUsers.GetSelectedCount() == 1) {
		int i = ctrlUsers.GetNextItem(-1, LVNI_SELECTED);
		UserInfo* ui = ctrlUsers.getItemData(i);
		dcassert(i != -1);
		LineDlg dlg;
		dlg.description = TSTRING(DESCRIPTION);
		/// @todo dlg.title = Text::toT(ui->user->getNick());
		/// @todo dlg.line = Text::toT(ui->user->getUserDescription());
		if(dlg.DoModal(m_hWnd)) {
			/// @todo ui->user->setUserDescription(Text::fromT(dlg.line));
			/// @todo ui->update();
			ctrlUsers.updateItem(i);
			FavoriteManager::getInstance()->save();
		}
	}
	return 0;
}

LRESULT UsersFrame::onItemChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NMITEMACTIVATE* l = (NMITEMACTIVATE*)pnmh;
	if(!startup && l->iItem != -1 && ((l->uNewState & LVIS_STATEIMAGEMASK) != (l->uOldState & LVIS_STATEIMAGEMASK))) {
		/// @todo ctrlUsers.getItemData(l->iItem)->user->setFavoriteGrantSlot(ctrlUsers.GetCheckState(l->iItem) != FALSE);
		FavoriteManager::getInstance()->save();
	}
	return 0;
} 

void UsersFrame::addUser(const User::Ptr& aUser) {
	int i = ctrlUsers.insertItem(new UserInfo(aUser), 0);
	/// @todo bool b = aUser->getFavoriteGrantSlot();
	//ctrlUsers.SetCheckState(i, b);
}

void UsersFrame::updateUser(const User::Ptr& aUser) {
/** @todo	int i = -1;
	while((i = ctrlUsers.findItem(Text::toT(aUser->getNick()), i)) != -1) {
		UserInfo *ui = ctrlUsers.getItemData(i);
		if(ui->user == aUser) {
			ui->update();
			ctrlUsers.updateItem(i);
		}
	} */
}

void UsersFrame::removeUser(const User::Ptr& aUser) {
/**	@todo int i = -1;
	while((i = ctrlUsers.findItem(Text::toT(aUser->getNick()), i)) != -1) {
		UserInfo *ui = ctrlUsers.getItemData(i);
		if(ui->user == aUser) {
			ctrlUsers.DeleteItem(i);
			delete ui;
			return;
		}
	} */
}

LRESULT UsersFrame::onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	if(!closed) {
		FavoriteManager::getInstance()->removeListener(this);
		ClientManager::getInstance()->removeListener(this);

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

/**
 * @file
 * $Id: UsersFrame.cpp,v 1.34 2005/04/12 23:24:03 arnetheduck Exp $
 */


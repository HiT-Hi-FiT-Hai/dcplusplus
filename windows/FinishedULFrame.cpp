/* 
 * Copyright (C) 2001-2003 Jacek Sieka, j_s@telia.com
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

#include "FinishedULFrame.h"
#include "WinUtil.h"
#include "TextFrame.h"

#include "../client/ClientManager.h"
#include "../client/StringTokenizer.h"

int FinishedULFrame::columnIndexes[] = { COLUMN_DONE, COLUMN_FILE, COLUMN_PATH, COLUMN_NICK, COLUMN_SIZE, COLUMN_SPEED };
int FinishedULFrame::columnSizes[] = { 110, 100, 290, 125, 80, 80 };
static ResourceManager::Strings columnNames[] = { ResourceManager::TIME, ResourceManager::FILENAME, ResourceManager::PATH, 
ResourceManager::NICK, ResourceManager::SIZE, ResourceManager::SPEED
};

LRESULT FinishedULFrame::onCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	CreateSimpleStatusBar(ATL_IDS_IDLEMESSAGE, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SBARS_SIZEGRIP);
	ctrlStatus.Attach(m_hWndStatusBar);
	
	ctrlList.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS , WS_EX_CLIENTEDGE, IDC_FINISHED_UL);

	if(BOOLSETTING(FULL_ROW_SELECT)) {
		ctrlList.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP);
	} else {
		ctrlList.SetExtendedListViewStyle(LVS_EX_HEADERDRAGDROP);
	}
	
	ctrlList.SetBkColor(WinUtil::bgColor);
	ctrlList.SetTextBkColor(WinUtil::bgColor);
	ctrlList.SetTextColor(WinUtil::textColor);
	
	// Create listview columns
	WinUtil::splitTokens(columnIndexes, SETTING(FINISHED_UL_ORDER), COLUMN_LAST);
	WinUtil::splitTokens(columnSizes, SETTING(FINISHED_UL_WIDTHS), COLUMN_LAST);
	
	for(int j=0; j<COLUMN_LAST; j++) {
		int fmt = (j == COLUMN_SIZE || j == COLUMN_SPEED) ? LVCFMT_RIGHT : LVCFMT_LEFT;
		ctrlList.InsertColumn(j, CSTRING_I(columnNames[j]), fmt, columnSizes[j], j);
	}
	
	ctrlList.SetColumnOrderArray(COLUMN_LAST, columnIndexes);
	
	UpdateLayout();
	
	FinishedManager::getInstance()->addListener(this);
	updateList(FinishedManager::getInstance()->lockList(true));
	FinishedManager::getInstance()->unlockList();
	
	ctxMenu.CreatePopupMenu();
	ctxMenu.AppendMenu(MF_STRING, IDC_VIEW_AS_TEXT, CSTRING(VIEW_AS_TEXT));
	ctxMenu.AppendMenu(MF_STRING, IDC_OPEN_FILE, CSTRING(OPEN));
	ctxMenu.AppendMenu(MF_STRING, IDC_OPEN_FOLDER, CSTRING(OPEN_FOLDER));
	ctxMenu.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)NULL);
	ctxMenu.AppendMenu(MF_STRING, IDC_REMOVE, CSTRING(REMOVE));
	ctxMenu.AppendMenu(MF_STRING, IDC_TOTAL, CSTRING(REMOVE_ALL));

	m_hMenu = WinUtil::mainMenu;

	bHandled = FALSE;
	return TRUE;
}

LRESULT FinishedULFrame::onDoubleClick(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	
	NMITEMACTIVATE * const item = (NMITEMACTIVATE*) pnmh;

	if(item->iItem != -1) {
		FinishedItem* entry = (FinishedItem*)ctrlList.GetItemData(item->iItem);
		WinUtil::openFile(entry->getTarget());
	}
	return 0;
}

LRESULT FinishedULFrame::onViewAsText(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int i;
	if((i = ctrlList.GetNextItem(-1, LVNI_SELECTED)) != -1) {
		FinishedItem * const entry = (FinishedItem*)ctrlList.GetItemData(i);
		TextFrame::openWindow(entry->getTarget());
	}
	return 0;
}

LRESULT FinishedULFrame::onOpenFile(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int i;
	if((i = ctrlList.GetNextItem(-1, LVNI_SELECTED)) != -1) {
		FinishedItem * const entry = (FinishedItem*)ctrlList.GetItemData(i);
		WinUtil::openFile(entry->getTarget());
	}
	return 0;
}

LRESULT FinishedULFrame::onOpenFolder(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int i;
	if((i = ctrlList.GetNextItem(-1, LVNI_SELECTED)) != -1) {
		FinishedItem * const entry = (FinishedItem*)ctrlList.GetItemData(i);
		::ShellExecute(NULL, "explore", Util::getFilePath(entry->getTarget()).c_str(), NULL, NULL, NULL);
	}
	return 0;
}

LRESULT FinishedULFrame::onRemove(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	switch(wID)
	{
	case IDC_REMOVE:
		{
			int i = -1;
			while((i = ctrlList.GetNextItem(-1, LVNI_SELECTED)) != -1) {
				FinishedManager::getInstance()->remove((FinishedItem*)ctrlList.GetItemData(i), true);
				ctrlList.DeleteItem(i);
			}
			break;
		}
	case IDC_TOTAL:
		FinishedManager::getInstance()->removeAll(true);
		break;
	}
	return 0;
}

LRESULT FinishedULFrame::onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	if(!closed){
		FinishedManager::getInstance()->removeListener(this);

		closed = true;
		PostMessage(WM_CLOSE);
		return 0;
	} else {
		WinUtil::saveHeaderOrder(ctrlList, SettingsManager::FINISHED_UL_ORDER, 
			SettingsManager::FINISHED_UL_WIDTHS, COLUMN_LAST, columnIndexes, columnSizes);

		MDIDestroy(m_hWnd);
		return 0;
	}
}

LRESULT FinishedULFrame::onSpeaker(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	if(wParam == SPEAK_ADD_LINE) {
		FinishedItem* entry = (FinishedItem*)lParam;
		addEntry(entry);
		if(BOOLSETTING(FINISHED_DIRTY))
			setDirty();
		updateStatus();
	} else if(wParam == SPEAK_REMOVE) {
		updateStatus();
	} else if(wParam == SPEAK_REMOVE_ALL) {
		ctrlList.DeleteAllItems();
		updateStatus();
	}
	return 0;
}

void FinishedULFrame::onAction(FinishedManagerListener::Types type, FinishedItem* entry)  throw() {
	switch(type) {
		case FinishedManagerListener::ADDED_UL: PostMessage(WM_SPEAKER, SPEAK_ADD_LINE, (LPARAM)entry); break;
		case FinishedManagerListener::REMOVED_ALL_UL: 
			PostMessage(WM_SPEAKER, SPEAK_REMOVE_ALL);
			totalBytes = 0;
			totalTime = 0;
			break;
		case FinishedManagerListener::REMOVED_UL:
			totalBytes -= entry->getChunkSize();
			totalTime -= entry->getMilliSeconds();
			PostMessage(WM_SPEAKER, SPEAK_REMOVE);
			break;
	}
};

void FinishedULFrame::addEntry(FinishedItem* entry) {
	StringList l;
	l.push_back(entry->getTime());
	l.push_back(Util::getFileName(entry->getTarget()));
	l.push_back(Util::getFilePath(entry->getTarget()));
	l.push_back(entry->getUser() + " (" + entry->getHub() + ")");
	l.push_back(Util::formatBytes(entry->getSize()));
	l.push_back(Util::formatBytes(entry->getAvgSpeed()) + "/s");
	totalBytes += entry->getChunkSize();
	totalTime += entry->getMilliSeconds();

	int loc = ctrlList.insert(l, 0, (LPARAM)entry);
	ctrlList.EnsureVisible(loc, FALSE);
}


/**
 * @file
 * $Id: FinishedULFrame.cpp,v 1.11 2003/11/14 15:37:36 arnetheduck Exp $
 */

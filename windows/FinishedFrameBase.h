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

#if !defined(FINISHED_FRAME_BASE_H)
#define FINISHED_FRAME_BASE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "Resource.h"

#include "FlatTabCtrl.h"
#include "ExListViewCtrl.h"
#include "WinUtil.h"
#include "TextFrame.h"

#include "../client/ClientManager.h"
#include "../client/StringTokenizer.h"
#include "../client/FinishedManager.h"

template<class T, int title, int id>
class FinishedFrameBase : public MDITabChildWindowImpl<T>, public StaticFrame<T, title>,
	protected FinishedManagerListener
{
public:
	FinishedFrameBase() : totalBytes(0), totalTime(0), closed(false) { }
	virtual ~FinishedFrameBase() { }

	BEGIN_MSG_MAP(T)
		MESSAGE_HANDLER(WM_CREATE, onCreate)
		MESSAGE_HANDLER(WM_CLOSE, onClose)
		MESSAGE_HANDLER(WM_CONTEXTMENU, onContextMenu)
		MESSAGE_HANDLER(WM_SETFOCUS, onSetFocus)
		MESSAGE_HANDLER(WM_SPEAKER, onSpeaker)
		COMMAND_ID_HANDLER(IDC_REMOVE, onRemove)
		COMMAND_ID_HANDLER(IDC_TOTAL, onRemove)
		COMMAND_ID_HANDLER(IDC_VIEW_AS_TEXT, onViewAsText)
		COMMAND_ID_HANDLER(IDC_OPEN_FILE, onOpenFile)
		COMMAND_ID_HANDLER(IDC_OPEN_FOLDER, onOpenFolder)
		NOTIFY_HANDLER(id, LVN_COLUMNCLICK, onColumnClickFinished)
		NOTIFY_HANDLER(id, LVN_KEYDOWN, onKeyDown)
		NOTIFY_HANDLER(id, NM_DBLCLK, onDoubleClick)
		CHAIN_MSG_MAP(MDITabChildWindowImpl<T>)
	END_MSG_MAP()

	LRESULT onCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
		CreateSimpleStatusBar(ATL_IDS_IDLEMESSAGE, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SBARS_SIZEGRIP);
		ctrlStatus.Attach(m_hWndStatusBar);

		ctrlList.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
			WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS, WS_EX_CLIENTEDGE, id);
		ctrlList.SetExtendedListViewStyle(LVS_EX_LABELTIP | LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT);

		ctrlList.SetImageList(WinUtil::fileImages, LVSIL_SMALL);
		ctrlList.SetBkColor(WinUtil::bgColor);
		ctrlList.SetTextBkColor(WinUtil::bgColor);
		ctrlList.SetTextColor(WinUtil::textColor);

		// Create listview columns
		WinUtil::splitTokens(columnIndexes, SettingsManager::getInstance()->get(columnOrder), COLUMN_LAST);
		WinUtil::splitTokens(columnSizes, SettingsManager::getInstance()->get(columnWidth), COLUMN_LAST);

		for(int j=0; j<COLUMN_LAST; j++) {
			int fmt = (j == COLUMN_SIZE || j == COLUMN_SPEED) ? LVCFMT_RIGHT : LVCFMT_LEFT;
			ctrlList.InsertColumn(j, CTSTRING_I(columnNames[j]), fmt, columnSizes[j], j);
		}

		ctrlList.SetColumnOrderArray(COLUMN_LAST, columnIndexes);
		ctrlList.setSort(COLUMN_DONE, ExListViewCtrl::SORT_STRING_NOCASE);

		UpdateLayout();

		FinishedManager::getInstance()->addListener(this);
		updateList(FinishedManager::getInstance()->lockList(upload));
		FinishedManager::getInstance()->unlockList();

		ctxMenu.CreatePopupMenu();
		ctxMenu.AppendMenu(MF_STRING, IDC_VIEW_AS_TEXT, CTSTRING(VIEW_AS_TEXT));
		ctxMenu.AppendMenu(MF_STRING, IDC_OPEN_FILE, CTSTRING(OPEN));
		ctxMenu.AppendMenu(MF_STRING, IDC_OPEN_FOLDER, CTSTRING(OPEN_FOLDER));
		ctxMenu.AppendMenu(MF_SEPARATOR);
		ctxMenu.AppendMenu(MF_STRING, IDC_REMOVE, CTSTRING(REMOVE));
		ctxMenu.AppendMenu(MF_STRING, IDC_TOTAL, CTSTRING(REMOVE_ALL));
		ctxMenu.SetMenuDefaultItem(IDC_OPEN_FILE);

		bHandled = FALSE;
		return TRUE;
	}

	LRESULT onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
		if(!closed) {
			FinishedManager::getInstance()->removeListener(this);

			closed = true;
			PostMessage(WM_CLOSE);
			return 0;
		} else {
			WinUtil::saveHeaderOrder(ctrlList, columnOrder, columnWidth,
				COLUMN_LAST, columnIndexes, columnSizes);

			bHandled = FALSE;
			return 0;
		}
	}

	LRESULT onDoubleClick(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
		NMITEMACTIVATE * const item = (NMITEMACTIVATE*) pnmh;

		if(item->iItem != -1) {
			FinishedItem* entry = (FinishedItem*)ctrlList.GetItemData(item->iItem);
			WinUtil::openFile(Text::toT(entry->getTarget()));
		}
		return 0;
	}

	LRESULT onSpeaker(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
		if(wParam == SPEAK_ADD_LINE) {
			FinishedItem* entry = (FinishedItem*)lParam;
			addEntry(entry);
			if(SettingsManager::getInstance()->get(boldFinished))
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

	LRESULT onRemove(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		switch(wID)
		{
		case IDC_REMOVE:
			{
				int i = -1;
				while((i = ctrlList.GetNextItem(-1, LVNI_SELECTED)) != -1) {
					FinishedManager::getInstance()->remove((FinishedItem*)ctrlList.GetItemData(i), upload);
					ctrlList.DeleteItem(i);
				}
				break;
			}
		case IDC_TOTAL:
			FinishedManager::getInstance()->removeAll(upload);
			break;
		}
		return 0;
	}

	LRESULT onViewAsText(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		int i;
		if((i = ctrlList.GetNextItem(-1, LVNI_SELECTED)) != -1) {
			FinishedItem * const entry = (FinishedItem*)ctrlList.GetItemData(i);
			TextFrame::openWindow(Text::toT(entry->getTarget()));
		}
		return 0;
	}

	LRESULT onOpenFile(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		int i;
		if((i = ctrlList.GetNextItem(-1, LVNI_SELECTED)) != -1) {
			FinishedItem * const entry = (FinishedItem*)ctrlList.GetItemData(i);
			WinUtil::openFile(Text::toT(entry->getTarget()));
		}
		return 0;
	}

	LRESULT onOpenFolder(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		int i;
		if((i = ctrlList.GetNextItem(-1, LVNI_SELECTED)) != -1) {
			FinishedItem * const entry = (FinishedItem*)ctrlList.GetItemData(i);
			WinUtil::openFolder(Text::toT(entry->getTarget()));
		}
		return 0;
	}

	LRESULT onContextMenu(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		if (reinterpret_cast<HWND>(wParam) == ctrlList && ctrlList.GetSelectedCount() > 0) { 
			POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

			if(pt.x == -1 && pt.y == -1) {
				WinUtil::getContextMenuPos(ctrlList, pt);
			}

			ctxMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);			
			return TRUE; 
		}
		bHandled = FALSE;
		return FALSE; 
	}

	LRESULT onColumnClickFinished(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
		NMLISTVIEW* const l = (NMLISTVIEW*)pnmh;
		if(l->iSubItem == ctrlList.getSortColumn()) {
			if (!ctrlList.isAscending())
				ctrlList.setSort(-1, ctrlList.getSortType());
			else
				ctrlList.setSortDirection(false);
		} else {
			switch(l->iSubItem) {
			case COLUMN_SIZE:
				ctrlList.setSort(l->iSubItem, ExListViewCtrl::SORT_FUNC, true, sortSize);
				break;
			case COLUMN_SPEED:
				ctrlList.setSort(l->iSubItem, ExListViewCtrl::SORT_FUNC, true, sortSpeed);
				break;
			default:
				ctrlList.setSort(l->iSubItem, ExListViewCtrl::SORT_STRING_NOCASE);
				break;
			}
		}
		return 0;
	}

	void UpdateLayout(BOOL bResizeBars = TRUE) {
		RECT rect;
		GetClientRect(&rect);

		// position bars and offset their dimensions
		UpdateBarsPosition(rect, bResizeBars);

		if(ctrlStatus.IsWindow()) {
			CRect sr;
			int w[4];
			ctrlStatus.GetClientRect(sr);
			w[3] = sr.right - 16;
			w[2] = max(w[3] - 100, 0);
			w[1] = max(w[2] - 100, 0);
			w[0] = max(w[1] - 100, 0);

			ctrlStatus.SetParts(4, w);
		}

		CRect rc(rect);
		ctrlList.MoveWindow(rc);
	}

	LRESULT onSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /* bHandled */) {
		ctrlList.SetFocus();
		return 0;
	}

	static int sortSize(LPARAM a, LPARAM b) {
		FinishedItem* c = (FinishedItem*)a;
		FinishedItem* d = (FinishedItem*)b;
		return compare(c->getSize(), d->getSize());
	}

	static int sortSpeed(LPARAM a, LPARAM b) {
		FinishedItem* c = (FinishedItem*)a;
		FinishedItem* d = (FinishedItem*)b;
		return compare(c->getAvgSpeed(), d->getAvgSpeed());
	}

	LRESULT onKeyDown(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
		NMLVKEYDOWN* kd = (NMLVKEYDOWN*) pnmh;

		if(kd->wVKey == VK_DELETE) {
			PostMessage(WM_COMMAND, IDC_REMOVE, 0);
		} 
		return 0;
	}

protected:
	enum {
		SPEAK_ADD_LINE,
		SPEAK_REMOVE,
		SPEAK_REMOVE_ALL
	};

	enum {
		COLUMN_FIRST,
		COLUMN_FILE = COLUMN_FIRST,
		COLUMN_DONE,
		COLUMN_PATH,
		COLUMN_NICK,
		COLUMN_HUB,
		COLUMN_SIZE,
		COLUMN_SPEED,
		COLUMN_CRC32,
		COLUMN_LAST
	};

	CStatusBarCtrl ctrlStatus;
	CMenu ctxMenu;

	ExListViewCtrl ctrlList;

	int64_t totalBytes;
	int64_t totalTime;

	bool closed;

	bool upload;
	SettingsManager::IntSetting boldFinished;
	SettingsManager::StrSetting columnWidth;
	SettingsManager::StrSetting columnOrder;

	static int columnSizes[COLUMN_LAST];
	static int columnIndexes[COLUMN_LAST];

	void updateStatus() {
		ctrlStatus.SetText(1, Text::toT(Util::toString(ctrlList.GetItemCount()) + ' ' + STRING(ITEMS)).c_str());
		ctrlStatus.SetText(2, Text::toT(Util::formatBytes(totalBytes)).c_str());
		ctrlStatus.SetText(3, Text::toT(Util::formatBytes((totalTime > 0) ? totalBytes * ((int64_t)1000) / totalTime : 0) + "/s").c_str());
	}

	void updateList(const FinishedItem::List& fl) {
		ctrlList.SetRedraw(FALSE);
		for(FinishedItem::List::const_iterator i = fl.begin(); i != fl.end(); ++i) {
			addEntry(*i);
		}
		ctrlList.SetRedraw(TRUE);
		ctrlList.Invalidate();
		updateStatus();
	}

	void addEntry(FinishedItem* entry) {
		TStringList l;
		l.push_back(Text::toT(Util::getFileName(entry->getTarget())));
		l.push_back(Text::toT(Util::formatTime("%Y-%m-%d %H:%M:%S", entry->getTime())));
		l.push_back(Text::toT(Util::getFilePath(entry->getTarget())));
		l.push_back(Text::toT(entry->getUser()));
		l.push_back(Text::toT(entry->getHub()));
		l.push_back(Text::toT(Util::formatBytes(entry->getSize())));
		l.push_back(Text::toT(Util::formatBytes(entry->getAvgSpeed()) + "/s"));
		l.push_back(entry->getCrc32Checked() ? TSTRING(YES_STR) : TSTRING(NO_STR));
		totalBytes += entry->getChunkSize();
		totalTime += entry->getMilliSeconds();

		int image = WinUtil::getIconIndex(Text::toT(entry->getTarget()));
		int loc = ctrlList.insert(l, image, (LPARAM)entry);
		ctrlList.EnsureVisible(loc, FALSE);
	}
};

template <class T, int title, int id>
int FinishedFrameBase<T, title, id>::columnIndexes[] = { COLUMN_DONE, COLUMN_FILE, COLUMN_PATH, COLUMN_NICK, COLUMN_HUB, COLUMN_SIZE, COLUMN_SPEED, COLUMN_CRC32 };

template <class T, int title, int id>
int FinishedFrameBase<T, title, id>::columnSizes[] = { 100, 110, 290, 125, 80, 80, 80, 90 };
static ResourceManager::Strings columnNames[] = { ResourceManager::FILENAME, ResourceManager::TIME, ResourceManager::PATH, 
ResourceManager::NICK, ResourceManager::HUB, ResourceManager::SIZE, ResourceManager::SPEED, ResourceManager::CRC_CHECKED
};

#endif // !defined(FINISHED_FRAME_BASE_H)

/**
* @file
* $Id: FinishedFrameBase.h,v 1.2 2006/02/19 16:19:06 arnetheduck Exp $
*/

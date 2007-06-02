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

#ifndef DCPLUSPLUS_WIN32_FINISHED_FRAME_BASE_H
#define DCPLUSPLUS_WIN32_FINISHED_FRAME_BASE_H

#include "StaticFrame.h"

#include <client/FinishedManager.h>

template<class T, ResourceManager::Strings title, int id>
class FinishedFrameBase : public StaticFrame<T>, private FinishedManagerListener {
public:
	static const ResourceManager::Strings TITLE_RESOURCE = title;

protected:
	friend class StaticFrame<T>;
	friend class MDIChildFrame<T>;

	FinishedFrameBase() :
		in_UL(id == IDC_FINISHED_UL),
		items(0),
		status(0),
		totalBytes(0),
		totalTime(0),
		closed(false)
	{
		{
			WidgetDataGrid::Seed cs;
			cs.style = WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_NOSORTHEADER;
			cs.exStyle = WS_EX_CLIENTEDGE;
			items = createDataGrid(cs);
			items->setListViewStyle(LVS_EX_LABELTIP | LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT);
			add_widget(items);

			items->createColumns(ResourceManager::getInstance()->getStrings(columnNames));
			items->setColumnOrder(WinUtil::splitTokens(SettingsManager::getInstance()->get(in_UL ? SettingsManager::FINISHED_UL_ORDER : SettingsManager::FINISHED_ORDER), columnIndexes));
			items->setColumnWidths(WinUtil::splitTokens(SettingsManager::getInstance()->get(in_UL ? SettingsManager::FINISHED_UL_WIDTHS : SettingsManager::FINISHED_WIDTHS), columnSizes));

			items->setBackgroundColor(WinUtil::bgColor);

#ifdef PORT_ME
		ctrlList.SetImageList(WinUtil::fileImages, LVSIL_SMALL);
		ctrlList.SetTextBkColor(WinUtil::bgColor);
		ctrlList.SetTextColor(WinUtil::textColor);

		for(int j=0; j<COLUMN_LAST; j++) {
			int fmt = (j == COLUMN_SIZE || j == COLUMN_SPEED) ? LVCFMT_RIGHT : LVCFMT_LEFT;
			ctrlList.InsertColumn(j, CTSTRING_I(columnNames[j]), fmt, columnSizes[j], j);
		}

		ctrlList.SetColumnOrderArray(COLUMN_LAST, columnIndexes);
		ctrlList.setSort(COLUMN_DONE, ExListViewCtrl::SORT_STRING_NOCASE);
#endif
		}

		statusSizes[STATUS_COUNT] = statusSizes[STATUS_BYTES] = statusSizes[STATUS_SPEED] = 100;
		statusSizes[STATUS_DUMMY] = 16; ///@todo get real resizer width
		status = createStatusBarSections();

		layout();

		onSpeaker(&T::spoken);

		FinishedManager::getInstance()->addListener(this);
		updateList(FinishedManager::getInstance()->lockList(in_UL));
		FinishedManager::getInstance()->unlockList();

#ifdef PORT_ME
		ctxMenu.CreatePopupMenu();
		ctxMenu.AppendMenu(MF_STRING, IDC_VIEW_AS_TEXT, CTSTRING(VIEW_AS_TEXT));
		ctxMenu.AppendMenu(MF_STRING, IDC_OPEN_FILE, CTSTRING(OPEN));
		ctxMenu.AppendMenu(MF_STRING, IDC_OPEN_FOLDER, CTSTRING(OPEN_FOLDER));
		ctxMenu.AppendMenu(MF_SEPARATOR);
		ctxMenu.AppendMenu(MF_STRING, IDC_REMOVE, CTSTRING(REMOVE));
		ctxMenu.AppendMenu(MF_STRING, IDC_TOTAL, CTSTRING(REMOVE_ALL));
		ctxMenu.SetMenuDefaultItem(IDC_OPEN_FILE);
#endif
	}

	virtual ~FinishedFrameBase() { }

	void layout() {
		const int border = 2;

		SmartWin::Rectangle r(getClientAreaSize());
		status->refresh();

		{
			std::vector<unsigned> w(STATUS_LAST);

			w[0] = status->getSize().x - std::accumulate(statusSizes+1, statusSizes+STATUS_LAST, 0);
			std::copy(statusSizes+1, statusSizes + STATUS_LAST, w.begin()+1);

			status->setSections(w);
		}

		r.size.y -= status->getSize().y - border;
		items->setBounds(r);
	}

	HRESULT spoken(LPARAM lParam, WPARAM wParam) {
		if(wParam == SPEAK_ADD_LINE) {
			FinishedItemPtr entry = (FinishedItemPtr)lParam;
			addEntry(entry);
#ifdef PORT_ME
			if(SettingsManager::getInstance()->get(in_UL ? SettingsManager::BOLD_FINISHED_UPLOADS : SettingsManager::BOLD_FINISHED_DOWNLOADS))
				setDirty();
#endif
			updateStatus();
		} else if(wParam == SPEAK_REMOVE) {
			updateStatus();
		} else if(wParam == SPEAK_REMOVE_ALL) {
			items->removeAllRows();
			updateStatus();
		}
		return 0;
	}

private:
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

	enum Status {
		STATUS_STATUS,
		STATUS_COUNT,
		STATUS_BYTES,
		STATUS_SPEED,
		STATUS_DUMMY,
		STATUS_LAST
	};
	unsigned statusSizes[STATUS_LAST];

	static int columnSizes[COLUMN_LAST];
	static int columnIndexes[COLUMN_LAST];

	WidgetDataGridPtr items;
	WidgetStatusBarSectionsPtr status;

	bool in_UL, closed;

	int64_t totalBytes, totalTime;

	void updateStatus() {
		status->setText(Text::toT(Util::toString(items->getRowCount()) + ' ' + STRING(ITEMS)), STATUS_COUNT);
		status->setText(Text::toT(Util::formatBytes(totalBytes)), STATUS_BYTES);
		status->setText(Text::toT(Util::formatBytes((totalTime > 0) ? totalBytes * ((int64_t)1000) / totalTime : 0) + "/s"), STATUS_SPEED);
	}

	void updateList(const FinishedItemList& fl) {
#ifdef PORT_ME
		ctrlList.SetRedraw(FALSE);
#endif
		for(FinishedItemList::const_iterator i = fl.begin(); i != fl.end(); ++i)
			addEntry(*i);
#ifdef PORT_ME
		ctrlList.SetRedraw(TRUE);
		ctrlList.Invalidate();
#endif
		updateStatus();
	}

	void addEntry(FinishedItemPtr entry) {
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

#ifdef PORT_ME
		int image = WinUtil::getIconIndex(Text::toT(entry->getTarget()));
		int loc = ctrlList.insert(l, image, (LPARAM)entry);
#endif
		items->insertRow(l);
#ifdef PORT_ME
		ctrlList.EnsureVisible(loc, FALSE);
#endif
	}

	virtual void on(Added, bool upload, FinishedItemPtr entry) throw() {
		if(upload == in_UL)
			StupidWin::postMessage(this, WM_SPEAKER, SPEAK_ADD_LINE, (LPARAM)entry);
	}

	virtual void on(Removed, bool upload, FinishedItemPtr entry) throw() {
		if(upload == in_UL) {
			totalBytes -= entry->getChunkSize();
			totalTime -= entry->getMilliSeconds();
			StupidWin::postMessage(this, WM_SPEAKER, SPEAK_REMOVE);
		}
	}

	virtual void on(RemovedAll, bool upload) throw() {
		if(upload == in_UL) {
			StupidWin::postMessage(this, WM_SPEAKER, SPEAK_REMOVE_ALL);
			totalBytes = 0;
			totalTime = 0;
		}
	}
};

template <class T, ResourceManager::Strings title, int id>
int FinishedFrameBase<T, title, id>::columnIndexes[] = { COLUMN_DONE, COLUMN_FILE, COLUMN_PATH, COLUMN_NICK, COLUMN_HUB, COLUMN_SIZE, COLUMN_SPEED, COLUMN_CRC32 };

template <class T, ResourceManager::Strings title, int id>
int FinishedFrameBase<T, title, id>::columnSizes[] = { 100, 110, 290, 125, 80, 80, 80, 90 };
static ResourceManager::Strings columnNames[] = { ResourceManager::FILENAME, ResourceManager::TIME, ResourceManager::PATH,
ResourceManager::NICK, ResourceManager::HUB, ResourceManager::SIZE, ResourceManager::SPEED, ResourceManager::CRC_CHECKED
};

#ifdef PORT_ME

template<class T, int title, int id>
class FinishedFrameBase : public MDITabChildWindowImpl<T>, public StaticFrame<T, title>,
	protected FinishedManagerListener
{
public:
	BEGIN_MSG_MAP(T)
		MESSAGE_HANDLER(WM_CLOSE, onClose)
		MESSAGE_HANDLER(WM_CONTEXTMENU, onContextMenu)
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

	LRESULT onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
		if(!closed) {
			FinishedManager::getInstance()->removeListener(this);

			closed = true;
			PostMessage(WM_CLOSE);
			return 0;
		} else {
			WinUtil::saveHeaderOrder(ctrlList,
				in_UL ? SettingsManager::FINISHED_UL_ORDER : SettingsManager::FINISHED_ORDER,
				in_UL ? SettingsManager::FINISHED_UL_WIDTHS : SettingsManager::FINISHED_WIDTHS,
				COLUMN_LAST, columnIndexes, columnSizes);

			bHandled = FALSE;
			return 0;
		}
	}

	LRESULT onDoubleClick(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
		NMITEMACTIVATE * const item = (NMITEMACTIVATE*) pnmh;

		if(item->iItem != -1) {
			FinishedItemPtr entry = (FinishedItemPtr)ctrlList.GetItemData(item->iItem);
			WinUtil::openFile(Text::toT(entry->getTarget()));
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
					FinishedManager::getInstance()->remove((FinishedItemPtr)ctrlList.GetItemData(i), upload);
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
			FinishedItem * const entry = (FinishedItemPtr)ctrlList.GetItemData(i);
			TextFrame::openWindow(Text::toT(entry->getTarget()));
		}
		return 0;
	}

	LRESULT onOpenFile(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		int i;
		if((i = ctrlList.GetNextItem(-1, LVNI_SELECTED)) != -1) {
			FinishedItem * const entry = (FinishedItemPtr)ctrlList.GetItemData(i);
			WinUtil::openFile(Text::toT(entry->getTarget()));
		}
		return 0;
	}

	LRESULT onOpenFolder(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		int i;
		if((i = ctrlList.GetNextItem(-1, LVNI_SELECTED)) != -1) {
			FinishedItem * const entry = (FinishedItemPtr)ctrlList.GetItemData(i);
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

			bool bShellMenuShown = false;
			if(BOOLSETTING(SHOW_SHELL_MENU) && (ctrlList.GetSelectedCount() == 1)) {
				string path = ((FinishedItemPtr)ctrlList.GetItemData(ctrlList.GetSelectedIndex()))->getTarget();
				if(File::getSize(path) != 1) {
					CShellContextMenu shellMenu;
					shellMenu.SetPath(Text::toT(path));

					CMenu* pShellMenu = shellMenu.GetMenu();
					pShellMenu->AppendMenu(MF_STRING, IDC_VIEW_AS_TEXT, CTSTRING(VIEW_AS_TEXT));
					pShellMenu->AppendMenu(MF_STRING, IDC_OPEN_FOLDER, CTSTRING(OPEN_FOLDER));
					pShellMenu->AppendMenu(MF_SEPARATOR);
					pShellMenu->AppendMenu(MF_STRING, IDC_REMOVE, CTSTRING(REMOVE));
					pShellMenu->AppendMenu(MF_STRING, IDC_TOTAL, CTSTRING(REMOVE_ALL));
					pShellMenu->AppendMenu(MF_SEPARATOR);

					UINT idCommand = shellMenu.ShowContextMenu(m_hWnd, pt);
					if(idCommand != 0)
						PostMessage(WM_COMMAND, idCommand);

					bShellMenuShown = true;
				}
			}

			if(!bShellMenuShown)
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

	static int sortSize(LPARAM a, LPARAM b) {
		FinishedItemPtr c = (FinishedItemPtr)a;
		FinishedItemPtr d = (FinishedItemPtr)b;
		return compare(c->getSize(), d->getSize());
	}

	static int sortSpeed(LPARAM a, LPARAM b) {
		FinishedItemPtr c = (FinishedItemPtr)a;
		FinishedItemPtr d = (FinishedItemPtr)b;
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
	CMenu ctxMenu;
};

#endif

#endif // !defined(DCPLUSPLUS_WIN32_FINISHED_FRAME_BASE_H)

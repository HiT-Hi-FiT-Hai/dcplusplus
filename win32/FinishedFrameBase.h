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
#include "ShellContextMenu.h"

#include <client/FinishedManager.h>

template<class T, bool in_UL>
class FinishedFrameBase : public StaticFrame<T>, private FinishedManagerListener {
public:
	static const ResourceManager::Strings TITLE_RESOURCE = in_UL ? ResourceManager::FINISHED_UPLOADS : ResourceManager::FINISHED_DOWNLOADS;

protected:
	friend class StaticFrame<T>;
	friend class MDIChildFrame<T>;

	FinishedFrameBase() :
		totalBytes(0),
		totalTime(0),
		closed(false)
	{
		{
			typename MDIChildFrame<T>::WidgetDataGrid::Seed cs;
			cs.style = WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_NOSORTHEADER;
			cs.exStyle = WS_EX_CLIENTEDGE;
			items = createDataGrid(cs);
			items->setListViewStyle(LVS_EX_LABELTIP | LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT);
			add_widget(items);

			items->createColumns(ResourceManager::getInstance()->getStrings(columnNames));
			items->setColumnOrder(WinUtil::splitTokens(SettingsManager::getInstance()->get(in_UL ? SettingsManager::FINISHED_UL_ORDER : SettingsManager::FINISHED_ORDER), columnIndexes));
			items->setColumnWidths(WinUtil::splitTokens(SettingsManager::getInstance()->get(in_UL ? SettingsManager::FINISHED_UL_WIDTHS : SettingsManager::FINISHED_WIDTHS), columnSizes));

			items->setColor(WinUtil::textColor, WinUtil::bgColor);
			items->setSmallImageList(WinUtil::fileImages);

#ifdef PORT_ME
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
		status = this->createStatusBarSections();

		layout();

		onSpeaker(&T::spoken);

		FinishedManager::getInstance()->addListener(this);
		updateList(FinishedManager::getInstance()->lockList(in_UL));
		FinishedManager::getInstance()->unlockList();

		onRaw(&T::handleDoubleClick, SmartWin::Message(WM_NOTIFY, NM_DBLCLK));
		onRaw(&T::handleColumnClick, SmartWin::Message(WM_NOTIFY, LVN_COLUMNCLICK));
		onRaw(&T::handleKeyDown, SmartWin::Message(WM_NOTIFY, LVN_KEYDOWN));

		mainMenu = createMenu();
		contextMenu = mainMenu->appendPopup(Util::emptyStringT);
		contextMenu->appendItem(IDC_VIEW_AS_TEXT, TSTRING(VIEW_AS_TEXT), &T::handleViewAsText);
		contextMenu->appendItem(IDC_OPEN_FILE, TSTRING(OPEN), &T::handleOpenFile);
		contextMenu->appendItem(IDC_OPEN_FOLDER, TSTRING(OPEN_FOLDER), &T::handleOpenFolder);
		contextMenu->appendSeparatorItem();
		contextMenu->appendItem(IDC_REMOVE, TSTRING(REMOVE), &T::handleRemove);
		contextMenu->appendItem(IDC_REMOVE_ALL, TSTRING(REMOVE_ALL), &T::handleRemoveAll);
#ifdef PORT_ME
		contextMenu->setMenuDefaultItem(IDC_OPEN_FILE);
#endif
		onRaw(&T::handleContextMenu, SmartWin::Message(WM_CONTEXTMENU));

#if 1
		// for testing purposes; adds 2 dummy lines into the list
		addEntry(new FinishedItem("C:\\folder\\file.txt", "nicks", "hubs", 1024, 1024, 1000, GET_TIME(), false));
		addEntry(new FinishedItem("C:\\folder\\file2.txt", "nicks2", "hubs2", 2048, 2048, 1000, GET_TIME(), false));
#endif
	}

	virtual ~FinishedFrameBase() { }

	void layout() {
		const int border = 2;

		SmartWin::Rectangle r(this->getClientAreaSize());
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

	bool preClosing() {
		FinishedManager::getInstance()->removeListener(this);
		return true;
	}

	void postClosing() {
		items->removeAllRows();

		SettingsManager::getInstance()->set(in_UL ? SettingsManager::FINISHED_UL_ORDER : SettingsManager::FINISHED_ORDER, WinUtil::toString(items->getColumnOrder()));
		SettingsManager::getInstance()->set(in_UL ? SettingsManager::FINISHED_UL_WIDTHS : SettingsManager::FINISHED_WIDTHS, WinUtil::toString(items->getColumnWidths()));
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
	static ResourceManager::Strings columnNames[COLUMN_LAST];

	typename MDIChildFrame<T>::WidgetDataGridPtr items;
	typename MDIChildFrame<T>::WidgetStatusBarSectionsPtr status;

	typename MDIChildFrame<T>::WidgetMenuPtr mainMenu, contextMenu;

	bool closed;

	int64_t totalBytes, totalTime;

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

	HRESULT handleDoubleClick(LPARAM lParam, WPARAM /*wParam*/) {
		if(((LPNMHDR)lParam)->hwndFrom == items->handle()) {
			LPNMITEMACTIVATE item = (LPNMITEMACTIVATE)lParam;
			if(item->iItem != -1)
				WinUtil::openFile(Text::toT(((FinishedItemPtr)items->getItemData(item->iItem))->getTarget()));
		}
		return 0;
	}

	HRESULT handleColumnClick(LPARAM lParam, WPARAM /*wParam*/) {
		if(((LPNMHDR)lParam)->hwndFrom == items->handle()) {
			//PORT_ME
		}
		return 0;
	}

	HRESULT handleKeyDown(LPARAM lParam, WPARAM /*wParam*/) {
		if(((LPNMHDR)lParam)->hwndFrom == items->handle() && ((LPNMLVKEYDOWN)lParam)->wVKey == VK_DELETE)
			StupidWin::postMessage(this, WM_COMMAND, IDC_REMOVE);
		return 0;
	}

	HRESULT handleContextMenu(LPARAM lParam, WPARAM wParam) {
		if (reinterpret_cast<HWND>(wParam) == items->handle() && items->getSelectedCount() > 0) {
			POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

			if(pt.x == -1 && pt.y == -1) {
				pt = items->getContextMenuPos();
			}

			if(BOOLSETTING(SHOW_SHELL_MENU) && items->getSelectedCount() == 1) {
				string path = ((FinishedItemPtr)items->getItemData(items->getSelectedIndex()))->getTarget();
				if(File::getSize(path) != -1) {
					CShellContextMenu<T> shellMenu;
					shellMenu.SetPath(Text::utf8ToWide(path));

					WidgetMenuPtr pShellMenu = mainMenu->appendPopup(Util::emptyStringT);
					pShellMenu->appendItem(IDC_VIEW_AS_TEXT, CTSTRING(VIEW_AS_TEXT), &T::handleViewAsText);
					pShellMenu->appendItem(IDC_OPEN_FOLDER, CTSTRING(OPEN_FOLDER), &T::handleOpenFolder);
					pShellMenu->appendSeparatorItem();
					pShellMenu->appendItem(IDC_REMOVE, CTSTRING(REMOVE), &T::handleRemove);
					pShellMenu->appendItem(IDC_REMOVE_ALL, CTSTRING(REMOVE_ALL), &T::handleRemoveAll);
					pShellMenu->appendSeparatorItem();

					UINT idCommand = shellMenu.ShowContextMenu(pShellMenu, static_cast<T*>(this), pt);
					if(idCommand != 0)
						StupidWin::postMessage(this, WM_COMMAND, idCommand);
					return TRUE;
				}
			}

			contextMenu->trackPopupMenu(static_cast<T*>(this), pt.x, pt.y, TPM_LEFTALIGN | TPM_RIGHTBUTTON);
			return TRUE;
		}
		return FALSE;
	}

	void handleViewAsText(WidgetMenuPtr /*menu*/, unsigned /*id*/) {
#ifdef PORT_ME
		int i = -1;
		while((i = items->getNextItem(i, LVNI_SELECTED)) != -1)
			TextFrame::openWindow(Text::toT(((FinishedItemPtr)items->getItemData(i))->getTarget()));
#endif
	}

	void handleOpenFile(WidgetMenuPtr /*menu*/, unsigned /*id*/) {
		int i = -1;
		while((i = items->getNextItem(i, LVNI_SELECTED)) != -1)
			WinUtil::openFile(Text::toT(((FinishedItemPtr)items->getItemData(i))->getTarget()));
	}

	void handleOpenFolder(WidgetMenuPtr /*menu*/, unsigned /*id*/) {
		int i = -1;
		while((i = items->getNextItem(i, LVNI_SELECTED)) != -1)
			WinUtil::openFolder(Text::toT(((FinishedItemPtr)items->getItemData(i))->getTarget()));
	}

	void handleRemove(WidgetMenuPtr /*menu*/, unsigned /*id*/) {
		int i;
		while((i = items->getNextItem(-1, LVNI_SELECTED)) != -1) {
			FinishedManager::getInstance()->remove((FinishedItemPtr)items->getItemData(i), in_UL);
			items->removeRow(i);
		}
	}

	void handleRemoveAll(WidgetMenuPtr /*menu*/, unsigned /*id*/) {
		FinishedManager::getInstance()->removeAll(in_UL);
	}

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

		items->insertRow(l, (LPARAM)entry, -1, WinUtil::getIconIndex(Text::toT(entry->getTarget())));
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

template <class T, bool in_UL>
int FinishedFrameBase<T, in_UL>::columnIndexes[] = { COLUMN_DONE, COLUMN_FILE, COLUMN_PATH, COLUMN_NICK, COLUMN_HUB, COLUMN_SIZE, COLUMN_SPEED, COLUMN_CRC32 };

template <class T, bool in_UL>
int FinishedFrameBase<T, in_UL>::columnSizes[] = { 100, 110, 290, 125, 80, 80, 80, 90 };

template <class T, bool in_UL>
ResourceManager::Strings FinishedFrameBase<T, in_UL>::columnNames[] = { ResourceManager::FILENAME, ResourceManager::TIME, ResourceManager::PATH,
ResourceManager::NICK, ResourceManager::HUB, ResourceManager::SIZE, ResourceManager::SPEED, ResourceManager::CRC_CHECKED
};

#ifdef PORT_ME

template<class T, int title, int id>
class FinishedFrameBase : public MDITabChildWindowImpl<T>, public StaticFrame<T, title>,
	protected FinishedManagerListener
{
public:
	BEGIN_MSG_MAP(T)
		NOTIFY_HANDLER(id, LVN_COLUMNCLICK, onColumnClickFinished)
		CHAIN_MSG_MAP(MDITabChildWindowImpl<T>)
	END_MSG_MAP()

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
};

#endif

#endif // !defined(DCPLUSPLUS_WIN32_FINISHED_FRAME_BASE_H)

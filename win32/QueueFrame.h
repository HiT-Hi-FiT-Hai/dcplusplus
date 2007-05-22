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

#ifndef DCPLUSPLUS_WIN32_QUEUE_FRAME_H
#define DCPLUSPLUS_WIN32_QUEUE_FRAME_H

#include "StaticFrame.h"
#include "TypedListViewCtrl.h"

#include <client/TaskQueue.h>
#include <client/FastAlloc.h>
#include <client/QueueManagerListener.h>
#include <client/QueueItem.h>
#include <client/ClientListener.h>

class QueueFrame : public StaticFrame<QueueFrame>, private ClientListener, private QueueManagerListener
{
public:
	static const ResourceManager::Strings TITLE_RESOURCE = ResourceManager::DOWNLOAD_QUEUE;

protected:
	typedef StaticFrame<QueueFrame> Base;
	friend class StaticFrame<QueueFrame>;
	friend class MDIChildFrame<QueueFrame>;
	
	void layout();
	HRESULT spoken(LPARAM lp, WPARAM wp);
	bool preClosing();
	void postClosing();
	
	void splitterMoved(WidgetSplitterCool*, const SmartWin::Point& pt);
	
private:
	enum {
		COLUMN_FIRST,
		COLUMN_TARGET = COLUMN_FIRST,
		COLUMN_STATUS,
		COLUMN_SIZE,
		COLUMN_DOWNLOADED,
		COLUMN_PRIORITY,
		COLUMN_USERS,
		COLUMN_PATH,
		COLUMN_EXACT_SIZE,
		COLUMN_ERRORS,
		COLUMN_ADDED,
		COLUMN_TTH,
		COLUMN_TYPE,
		COLUMN_LAST
	};
	enum Status {
		STATUS_SHOW_TREE,
		STATUS_STATUS,
		STATUS_PARTIAL_COUNT,
		STATUS_PARTIAL_BYTES,
		STATUS_TOTAL_COUNT,
		STATUS_TOTAL_BYTES,
		STATUS_DUMMY,
		STATUS_LAST
	};
	unsigned statusSizes[STATUS_LAST];
	
	enum Tasks {
		ADD_ITEM,
		REMOVE_ITEM,
		UPDATE_ITEM
	};

	class QueueItemInfo;
	friend class QueueItemInfo;

	class QueueItemInfo : public Flags, public FastAlloc<QueueItemInfo> {
	public:

		struct Display : public FastAlloc<Display> {
			tstring columns[COLUMN_LAST];
		};

		enum {
			MASK_TARGET = 1 << COLUMN_TARGET,
			MASK_STATUS = 1 << COLUMN_STATUS,
			MASK_SIZE = 1 << COLUMN_SIZE,
			MASK_DOWNLOADED = 1 << COLUMN_DOWNLOADED,
			MASK_PRIORITY = 1 << COLUMN_PRIORITY,
			MASK_USERS = 1 << COLUMN_USERS,
			MASK_PATH = 1 << COLUMN_PATH,
			MASK_ERRORS = 1 << COLUMN_ERRORS,
			MASK_ADDED = 1 << COLUMN_ADDED,
			MASK_TTH = 1 << COLUMN_TTH,
			MASK_TYPE = 1 << COLUMN_TYPE
		};

		QueueItemInfo(const QueueItem& aQI) : Flags(aQI), target(aQI.getTarget()),
			path(Util::getFilePath(aQI.getTarget())),
			size(aQI.getSize()), downloadedBytes(aQI.getDownloadedBytes()),
			added(aQI.getAdded()), priority(aQI.getPriority()), status(aQI.getStatus()), tth(aQI.getTTH()), 
			sources(aQI.getSources()), badSources(aQI.getBadSources()), updateMask((uint32_t)-1), display(0)
		{
		}

		~QueueItemInfo() { delete display; }

		void update();

		void remove();

		// TypedListViewCtrl functions
		const tstring& getText(int col) {
			return getDisplay()->columns[col];
		}
		static int compareItems(QueueItemInfo* a, QueueItemInfo* b, int col) {
			switch(col) {
				case COLUMN_SIZE: case COLUMN_EXACT_SIZE: return compare(a->getSize(), b->getSize());
				case COLUMN_PRIORITY: return compare((int)a->getPriority(), (int)b->getPriority());
				case COLUMN_DOWNLOADED: return compare(a->getDownloadedBytes(), b->getDownloadedBytes());
				case COLUMN_ADDED: return compare(a->getAdded(), b->getAdded());
				default: return lstrcmpi(a->getDisplay()->columns[col].c_str(), b->getDisplay()->columns[col].c_str());
			}
		}

		QueueItem::SourceList& getSources() { return sources; }
		QueueItem::SourceList& getBadSources() { return badSources; }

		Display* getDisplay() {
			if(!display) {
				display = new Display;
				update();
			}
			return display;
		}

		bool isSource(const User::Ptr& u) {
			return find(sources.begin(), sources.end(), u) != sources.end();
		}
		bool isBadSource(const User::Ptr& u) {
			return find(badSources.begin(), badSources.end(), u) != badSources.end();
		}

		GETSET(string, target, Target);
		GETSET(string, path, Path);
		GETSET(int64_t, size, Size);
		GETSET(int64_t, downloadedBytes, DownloadedBytes);
		GETSET(time_t, added, Added);
		GETSET(QueueItem::Priority, priority, Priority);
		GETSET(QueueItem::Status, status, Status);
		GETSET(TTHValue, tth, TTH);
		GETSET(QueueItem::SourceList, sources, Sources);
		GETSET(QueueItem::SourceList, badSources, BadSources);
		uint32_t updateMask;

	private:

		Display* display;

		QueueItemInfo(const QueueItemInfo&);
		QueueItemInfo& operator=(const QueueItemInfo&);
	};

	struct QueueItemInfoTask : public FastAlloc<QueueItemInfoTask>, public Task {
		QueueItemInfoTask(QueueItemInfo* ii_) : ii(ii_) { }
		QueueItemInfo* ii;
	};

	struct UpdateTask : public FastAlloc<UpdateTask>, public Task {
		UpdateTask(const QueueItem& source) : target(source.getTarget()), priority(source.getPriority()),
			status(source.getStatus()), downloadedBytes(source.getDownloadedBytes()), sources(source.getSources()), badSources(source.getBadSources()) 
		{
		}

		string target;
		QueueItem::Priority priority;
		QueueItem::Status status;
		int64_t downloadedBytes;

		QueueItem::SourceList sources;
		QueueItem::SourceList badSources;
	};

	TaskQueue tasks;

	WidgetStatusBarSectionsPtr status;
	WidgetTreeViewPtr dirs;
	typedef TypedListViewCtrl<QueueFrame, QueueItemInfo> WidgetFiles;
	typedef WidgetFiles* WidgetFilesPtr;
	
	WidgetFilesPtr files;
	WidgetSplitterCoolPtr splitter;
	WidgetCheckBoxPtr showTree;
	
	typedef HASH_MULTIMAP_X(string, QueueItemInfo*, noCaseStringHash, noCaseStringEq, noCaseStringLess) DirectoryMap;
	typedef DirectoryMap::iterator DirectoryIter;
	typedef pair<DirectoryIter, DirectoryIter> DirectoryPair;
	DirectoryMap directories;

	std::string curDir;

	bool dirty;
	
	int64_t queueSize;
	int queueItems;

	static int columnIndexes[COLUMN_LAST];
	static int columnSizes[COLUMN_LAST];

	QueueFrame(Widget* mdiParent);
	virtual ~QueueFrame();
	
	void setStatus(Status s, const tstring& text);
	void updateStatus();
	void updateQueue();
	const std::string& getSelectedDir();

	void addQueueItem(QueueItemInfo* qi, bool noSort);
	void addQueueList(const QueueItem::StringMap& l);

	HTREEITEM addDirectory(const string& dir, bool isFileList = false, HTREEITEM startAt = NULL);
	void removeDirectories(HTREEITEM ht);
	void removeDirectory(const string& dir, bool isFileList = false);

	bool isCurDir(const string& aDir) const;

	QueueItemInfo* getItemInfo(const string& target);

	using MDIChildFrame<QueueFrame>::speak;
	void speak(Tasks s, Task* t) { tasks.add(s, t); speak(); }
	void speak(Tasks s, const string& msg) { tasks.add(s, new StringTask(msg)); speak(); }

	virtual void on(QueueManagerListener::Added, QueueItem* aQI) throw();
	virtual void on(QueueManagerListener::Moved, QueueItem* aQI, const string& oldTarget) throw();
	virtual void on(QueueManagerListener::Removed, QueueItem* aQI) throw();
	virtual void on(QueueManagerListener::SourcesUpdated, QueueItem* aQI) throw();
	virtual void on(QueueManagerListener::StatusUpdated, QueueItem* aQI) throw() { on(QueueManagerListener::SourcesUpdated(), aQI); }
};

#ifdef PORT_ME
#include "FlatTabCtrl.h"

#include "../client/QueueManager.h"
#include "../client/TaskQueue.h"

#define SHOWTREE_MESSAGE_MAP 12

class QueueFrame : public MDITabChildWindowImpl<QueueFrame>, public StaticFrame<QueueFrame, ResourceManager::DOWNLOAD_QUEUE>,
	private QueueManagerListener, public CSplitterImpl<QueueFrame>
{
public:
	DECLARE_FRAME_WND_CLASS_EX(_T("QueueFrame"), IDR_QUEUE, 0, COLOR_3DFACE);

	QueueFrame() : menuItems(0), queueSize(0), queueItems(0), spoken(false), dirty(false),
		usingDirMenu(false), readdItems(0), fileLists(NULL), showTree(true), closed(false),
		showTreeContainer(WC_BUTTON, this, SHOWTREE_MESSAGE_MAP)
	{
	}

	virtual ~QueueFrame() { }

	typedef MDITabChildWindowImpl<QueueFrame> baseClass;
	typedef CSplitterImpl<QueueFrame> splitBase;

	BEGIN_MSG_MAP(QueueFrame)
		NOTIFY_HANDLER(IDC_QUEUE, LVN_GETDISPINFO, ctrlQueue.onGetDispInfo)
		NOTIFY_HANDLER(IDC_QUEUE, LVN_COLUMNCLICK, ctrlQueue.onColumnClick)
		NOTIFY_HANDLER(IDC_QUEUE, LVN_KEYDOWN, onKeyDown)
		NOTIFY_HANDLER(IDC_QUEUE, LVN_ITEMCHANGED, onItemChangedQueue)
		NOTIFY_HANDLER(IDC_DIRECTORIES, TVN_SELCHANGED, onItemChanged)
		NOTIFY_HANDLER(IDC_DIRECTORIES, TVN_KEYDOWN, onKeyDownDirs)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_CLOSE, onClose)
		MESSAGE_HANDLER(WM_SPEAKER, onSpeaker)
		MESSAGE_HANDLER(WM_CONTEXTMENU, onContextMenu)
		MESSAGE_HANDLER(WM_SETFOCUS, onSetFocus)
		COMMAND_ID_HANDLER(IDC_SEARCH_ALTERNATES, onSearchAlternates)
		COMMAND_ID_HANDLER(IDC_BITZI_LOOKUP, onBitziLookup)
		COMMAND_ID_HANDLER(IDC_COPY_MAGNET, onCopyMagnet)
		COMMAND_ID_HANDLER(IDC_REMOVE, onRemove)
		COMMAND_ID_HANDLER(IDC_MOVE, onMove)
		COMMAND_RANGE_HANDLER(IDC_PRIORITY_PAUSED, IDC_PRIORITY_HIGHEST, onPriority)
		COMMAND_RANGE_HANDLER(IDC_BROWSELIST, IDC_BROWSELIST + menuItems, onBrowseList)
		COMMAND_RANGE_HANDLER(IDC_REMOVE_SOURCE, IDC_REMOVE_SOURCE + menuItems, onRemoveSource)
		COMMAND_RANGE_HANDLER(IDC_REMOVE_SOURCES, IDC_REMOVE_SOURCES + 1 + menuItems, onRemoveSources)
		COMMAND_RANGE_HANDLER(IDC_PM, IDC_PM + menuItems, onPM)
		COMMAND_RANGE_HANDLER(IDC_READD, IDC_READD + 1 + readdItems, onReadd)
		CHAIN_MSG_MAP(splitBase)
		CHAIN_MSG_MAP(baseClass)
	ALT_MSG_MAP(SHOWTREE_MESSAGE_MAP)
		MESSAGE_HANDLER(BM_SETCHECK, onShowTree)
	END_MSG_MAP()

	LRESULT onPriority(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onBrowseList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onRemoveSource(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onRemoveSources(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onPM(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onReadd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onSearchAlternates(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onBitziLookup(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onCopyMagnet(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onItemChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled);
	LRESULT onSpeaker(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled);
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onKeyDown(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);

	void UpdateLayout(BOOL bResizeBars = TRUE);
	void removeDir(HTREEITEM ht);
	void setPriority(HTREEITEM ht, const QueueItem::Priority& p);
	void changePriority(bool inc);

	LRESULT onItemChangedQueue(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
		NMLISTVIEW* lv = (NMLISTVIEW*)pnmh;
		if((lv->uNewState & LVIS_SELECTED) != (lv->uOldState & LVIS_SELECTED))
			updateStatus();
		return 0;
	}

	LRESULT onSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /* bHandled */) {
		ctrlQueue.SetFocus();
		return 0;
	}

	LRESULT onRemove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		usingDirMenu ? removeSelectedDir() : removeSelected();
		return 0;
	}

	LRESULT onMove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		usingDirMenu ? moveSelectedDir() : moveSelected();
		return 0;
	}

	LRESULT onKeyDownDirs(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
		NMTVKEYDOWN* kd = (NMTVKEYDOWN*) pnmh;
		if(kd->wVKey == VK_DELETE) {
			removeSelectedDir();
		} else if(kd->wVKey == VK_TAB) {
			onTab();
		}
		return 0;
	}

	void onTab();

	LRESULT onShowTree(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled) {
		bHandled = FALSE;
		showTree = (wParam == BST_CHECKED);
		UpdateLayout(FALSE);
		return 0;
	}

private:
	bool spoken;

	/** Single selection in the queue part */
	CMenu singleMenu;
	/** Multiple selection in the queue part */
	CMenu multiMenu;
	/** Tree part menu */
	CMenu browseMenu;

	CMenu removeMenu;
	CMenu removeAllMenu;
	CMenu pmMenu;
	CMenu priorityMenu;
	CMenu readdMenu;
	CMenu dirMenu;

	CButton ctrlShowTree;
	CContainedWindow showTreeContainer;

	bool usingDirMenu;

	int menuItems;
	int readdItems;

	HTREEITEM fileLists;

	CTreeViewCtrl ctrlDirs;

	void moveSelected();
	void moveSelectedDir();
	void moveDir(HTREEITEM ht, const string& target);

	void moveNode(HTREEITEM item, HTREEITEM parent);

	void clearTree(HTREEITEM item);

	void removeSelected();
	void removeSelectedDir();

	const string& getSelectedDir() {
		HTREEITEM ht = ctrlDirs.GetSelectedItem();
		return ht == NULL ? Util::emptyString : getDir(ctrlDirs.GetSelectedItem());
	}

	const string& getDir(HTREEITEM ht) { dcassert(ht != NULL); return *reinterpret_cast<string*>(ctrlDirs.GetItemData(ht)); }

};
#endif
#endif // !defined(QUEUE_FRAME_H)

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

#if !defined(AFX_QUEUEFRAME_H__8F6D05EC_ADCF_4987_8881_6DF3C0E355FA__INCLUDED_)
#define AFX_QUEUEFRAME_H__8F6D05EC_ADCF_4987_8881_6DF3C0E355FA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FlatTabCtrl.h"
#include "TypedListViewCtrl.h"

#include "../client/QueueManager.h"
#include "../client/CriticalSection.h"
#include "../client/FastAlloc.h"

#define SHOWTREE_MESSAGE_MAP 12

class QueueFrame : public MDITabChildWindowImpl<QueueFrame>, public StaticFrame<QueueFrame, ResourceManager::DOWNLOAD_QUEUE>,
	private QueueManagerListener, public CSplitterImpl<QueueFrame>
{
public:
	DECLARE_FRAME_WND_CLASS_EX(_T("QueueFrame"), IDR_QUEUE, 0, COLOR_3DFACE);

	QueueFrame() : menuItems(0), queueSize(0), queueItems(0), spoken(false), dirty(false), 
		usingDirMenu(false),  readdItems(0), fileLists(NULL), showTree(true), closed(false),
		showTreeContainer(WC_BUTTON, this, SHOWTREE_MESSAGE_MAP)
	{ 
	}

	virtual ~QueueFrame() { }
	
	virtual void OnFinalMessage(HWND /*hWnd*/) {
		delete this;
	}

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

	void onTab() {
		if(showTree) {
			HWND focus = ::GetFocus();
			if(focus == ctrlDirs.m_hWnd) {
				ctrlQueue.SetFocus();
			} else if(focus == ctrlQueue.m_hWnd) {
				ctrlDirs.SetFocus();
			}
		}
	}

	LRESULT onShowTree(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled) {
		bHandled = FALSE;
		showTree = (wParam == BST_CHECKED);
		UpdateLayout(FALSE);
		return 0;
	}
	
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
		COLUMN_LAST
	};
	enum Tasks {
		ADD_ITEM,
		REMOVE_ITEM,
		UPDATE_ITEM
	};
	
	class QueueItemInfo;
	friend class QueueItemInfo;
	
	class QueueItemInfo : public Flags, public FastAlloc<QueueItemInfo> {
	public:

		struct SourceInfo : public Flags {
			explicit SourceInfo(const QueueItem::Source& s) : Flags(s), user(s.getUser()) { };

			SourceInfo& operator=(const QueueItem::Source& s) {
				*((Flags*)this) = s;
				user = s.getUser();
				return *this;
			}
			User::Ptr& getUser() { return user; };

			User::Ptr user;
		};
		struct Display : public FastAlloc<Display> {
			tstring columns[COLUMN_LAST];
		};

		typedef vector<SourceInfo> SourceList;
		typedef SourceList::iterator SourceIter;

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
			MASK_TTH = 1 << COLUMN_TTH
		};

		QueueItemInfo(QueueItem* aQI) : Flags(*aQI), target(Text::toT(aQI->getTarget())),
			path(Text::toT(Util::getFilePath(aQI->getTarget()))),
			size(aQI->getSize()), downloadedBytes(aQI->getDownloadedBytes()), 
			added(aQI->getAdded()), tth(aQI->getTTH()), priority(aQI->getPriority()), status(aQI->getStatus()),
			updateMask((u_int32_t)-1), display(NULL)
		{ 
			for(QueueItem::Source::Iter i = aQI->getSources().begin(); i != aQI->getSources().end(); ++i) {
				sources.push_back(SourceInfo(*(*i)));
			}
			for(QueueItem::Source::Iter i = aQI->getBadSources().begin(); i != aQI->getBadSources().end(); ++i) {
				badSources.push_back(SourceInfo(*(*i)));
			}
		};

		~QueueItemInfo() { delete display; };

		void update();

		void remove() { QueueManager::getInstance()->remove(Text::fromT(getTarget())); }

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
				default: return Util::stricmp(a->getDisplay()->columns[col], b->getDisplay()->columns[col]);
			}
		}

		const tstring& getTargetFileName() { return getDisplay()->columns[COLUMN_TARGET]; }

		SourceList& getSources() { return sources; };
		SourceList& getBadSources() { return badSources; };

		Display* getDisplay() {
			if(display == NULL) {
				display = new Display;
				update();
			}
			return display;
		}

		bool isSource(const User::Ptr& u) {
			for(SourceIter i = sources.begin(); i != sources.end(); ++i) {
				if(i->getUser() == u)
					return true;
			}
			return false;
		}
		bool isBadSource(const User::Ptr& u) {
			for(SourceIter i = badSources.begin(); i != badSources.end(); ++i) {
				if(i->getUser() == u)
					return true;
			}
			return false;
		}
		
		GETSET(tstring, target, Target);
		GETSET(tstring, path, Path);
		GETSET(int64_t, size, Size);
		GETSET(int64_t, downloadedBytes, DownloadedBytes);
		GETSET(u_int32_t, added, Added);
		GETSET(QueueItem::Priority, priority, Priority);
		GETSET(QueueItem::Status, status, Status);
		GETSET(TTHValue*, tth, TTH);
		u_int32_t updateMask;
	
	private:

		Display* display;

		QueueItemInfo(const QueueItemInfo&);
		QueueItemInfo& operator=(const QueueItemInfo&);
		
		SourceList sources;
		SourceList badSources;

	};

	typedef pair<Tasks, void*> Task;
	typedef list<Task> TaskList;
	typedef TaskList::iterator TaskIter;
	
	TaskList tasks;
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
	bool showTree;

	bool usingDirMenu;

	bool dirty;

	int menuItems;
	int readdItems;

	HTREEITEM fileLists;

	typedef hash_map<QueueItem*, QueueItemInfo*, PointerHash<QueueItem> > QueueMap;
	typedef QueueMap::iterator QueueIter;
	QueueMap queue;
	
	typedef HASH_MULTIMAP_X(tstring, QueueItemInfo*, noCaseStringHash, noCaseStringEq, noCaseStringLess) DirectoryMap;
	typedef DirectoryMap::iterator DirectoryIter;
	typedef pair<DirectoryIter, DirectoryIter> DirectoryPair;
	DirectoryMap directories;
	tstring curDir;
	
	CriticalSection cs;
	TypedListViewCtrl<QueueItemInfo, IDC_QUEUE> ctrlQueue;
	CTreeViewCtrl ctrlDirs;
	
	CStatusBarCtrl ctrlStatus;
	int statusSizes[6];
	
	int64_t queueSize;
	int queueItems;

	bool closed;
	
	static int columnIndexes[COLUMN_LAST];
	static int columnSizes[COLUMN_LAST];

	void addQueueList(const QueueItem::StringMap& l);
	void addQueueItem(QueueItemInfo* qi, bool noSort);
	HTREEITEM addDirectory(const tstring& dir, bool isFileList = false, HTREEITEM startAt = NULL);
	void removeDirectory(const tstring& dir, bool isFileList = false);
	void removeDirectories(HTREEITEM ht);

	void updateQueue();
	void updateStatus();
	
	/**
	 * This one is different from the others because when a lot of files are removed
	 * at the same time, the WM_SPEAKER messages seem to get lost in the handling or
	 * something, they're not correctly processed anyway...thanks windows.
	 */
	void speak(Tasks t, void* p) {
		Lock l(cs);
		tasks.push_back(make_pair(t, p));
		if(!spoken) {
			PostMessage(WM_SPEAKER);
			spoken = true;
		}
	}

	bool isCurDir(const tstring& aDir) const { return Util::stricmp(curDir, aDir) == 0; };

	void moveSelected();	
	void moveSelectedDir();
	void moveDir(HTREEITEM ht, const tstring& target);

	void moveNode(HTREEITEM item, HTREEITEM parent);

	void clearTree(HTREEITEM item) {
		HTREEITEM next = ctrlDirs.GetChildItem(item);
		while(next != NULL) {
			clearTree(next);
			next = ctrlDirs.GetNextSiblingItem(next);
		}
		delete (string*)ctrlDirs.GetItemData(item);
	}

	void removeSelected() {
		if(!BOOLSETTING(CONFIRM_ITEM_REMOVAL) || MessageBox(CTSTRING(REALLY_REMOVE), _T("Really remove?"), MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDYES)
			ctrlQueue.forEachSelected(&QueueItemInfo::remove);
	}
	
	void removeSelectedDir() { removeDir(ctrlDirs.GetSelectedItem()); };
	
	const tstring& getSelectedDir() { 
		HTREEITEM ht = ctrlDirs.GetSelectedItem();
		return ht == NULL ? Util::emptyStringT : getDir(ctrlDirs.GetSelectedItem());
	};
	
	const tstring& getDir(HTREEITEM ht) { dcassert(ht != NULL); return *((tstring*)ctrlDirs.GetItemData(ht)); };

	virtual void on(QueueManagerListener::Added, QueueItem* aQI) throw();
	virtual void on(QueueManagerListener::Moved, QueueItem* aQI) throw();
	virtual void on(QueueManagerListener::Removed, QueueItem* aQI) throw();
	virtual void on(QueueManagerListener::SourcesUpdated, QueueItem* aQI) throw();
	virtual void on(QueueManagerListener::StatusUpdated, QueueItem* aQI) throw() { on(QueueManagerListener::SourcesUpdated(), aQI); }
};

#endif // !defined(AFX_QUEUEFRAME_H__8F6D05EC_ADCF_4987_8881_6DF3C0E355FA__INCLUDED_)

/**
 * @file
 * $Id: QueueFrame.h,v 1.50 2005/01/20 15:42:14 arnetheduck Exp $
 */

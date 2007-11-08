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
#include "TypedListView.h"
#include "TypedTreeView.h"

#include <dcpp/TaskQueue.h>
#include <dcpp/FastAlloc.h>
#include <dcpp/QueueManagerListener.h>
#include <dcpp/QueueItem.h>
#include <dcpp/ClientListener.h>
#include "resource.h"

class QueueFrame : 
	public StaticFrame<QueueFrame>, 
	private ClientListener, 
	private QueueManagerListener
{
public:
	enum Status {
		STATUS_SHOW_TREE,
		STATUS_STATUS,
		STATUS_PARTIAL_COUNT,
		STATUS_PARTIAL_BYTES,
		STATUS_TOTAL_COUNT,
		STATUS_TOTAL_BYTES,
		STATUS_LAST
	};

	static const ResourceManager::Strings TITLE_RESOURCE = ResourceManager::DOWNLOAD_QUEUE;
	static const unsigned ICON_RESOURCE = IDR_QUEUE;
	
private:
	typedef StaticFrame<QueueFrame> BaseType;
	friend class StaticFrame<QueueFrame>;
	friend class MDIChildFrame<QueueFrame>;
	
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
	enum Tasks {
		ADD_ITEM,
		REMOVE_ITEM,
		UPDATE_ITEM
	};

	class DirItemInfo : public FastAlloc<DirItemInfo> {
	public:
		DirItemInfo(const string& dir);
		DirItemInfo(const string& dir_, const tstring& text_) : dir(dir_), text(text_) { }
		const tstring& getText() const { return text; }
		int getImage();
		int getSelectedImage();
		const string& getDir() const { return dir; }
	private:
		string dir;
		tstring text;
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
			added(aQI.getAdded()), priority(aQI.getPriority()), running(aQI.isRunning()), tth(aQI.getTTH()), 
			sources(aQI.getSources()), badSources(aQI.getBadSources()), updateMask((uint32_t)-1), display(0)
		{
		}

		~QueueItemInfo() { delete display; }

		void update();

		void remove();

		// TypedListView functions
		const tstring& getText(int col) {
			return getDisplay()->columns[col];
		}
		int getImage() const {
			return WinUtil::getIconIndex(Text::toT(getTarget()));
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

		bool isSource(const UserPtr& u) {
			return find(sources.begin(), sources.end(), u) != sources.end();
		}
		bool isBadSource(const UserPtr& u) {
			return find(badSources.begin(), badSources.end(), u) != badSources.end();
		}

		GETSET(string, target, Target);
		GETSET(string, path, Path);
		GETSET(int64_t, size, Size);
		GETSET(int64_t, downloadedBytes, DownloadedBytes);
		GETSET(time_t, added, Added);
		GETSET(QueueItem::Priority, priority, Priority);
		GETSET(bool, running, Running);
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
			running(source.isRunning()), downloadedBytes(source.getDownloadedBytes()), sources(source.getSources()), badSources(source.getBadSources()) 
		{
		}

		string target;
		QueueItem::Priority priority;
		bool running;
		int64_t downloadedBytes;

		QueueItem::SourceList sources;
		QueueItem::SourceList badSources;
	};

	TaskQueue tasks;

	typedef TypedTreeView<QueueFrame, DirItemInfo> WidgetDirs;
	typedef WidgetDirs* WidgetDirsPtr;
	WidgetDirsPtr dirs;
	
	typedef TypedListView<QueueFrame, QueueItemInfo> WidgetFiles;
	typedef WidgetFiles* WidgetFilesPtr;
	WidgetFilesPtr files;
	WidgetVPanedPtr paned;
	WidgetCheckBoxPtr showTree;

	typedef unordered_multimap<string, QueueItemInfo*, noCaseStringHash, noCaseStringEq> DirectoryMap;
	typedef DirectoryMap::iterator DirectoryIter;
	typedef pair<DirectoryIter, DirectoryIter> DirectoryPair;
	DirectoryMap directories;

	std::string curDir;

	bool dirty;
	bool usingDirMenu;
	
	int64_t queueSize;
	int queueItems;

	HTREEITEM fileLists;

	static int columnIndexes[COLUMN_LAST];
	static int columnSizes[COLUMN_LAST];

	QueueFrame(SmartWin::WidgetTabView* mdiParent);
	virtual ~QueueFrame();
	
	void updateStatus();
	void updateFiles();

	void addQueueItem(QueueItemInfo* qi, bool noSort);
	void addQueueList(const QueueItem::StringMap& l);

	HTREEITEM addDirectory(const string& dir, bool isFileList = false, HTREEITEM startAt = NULL);
	void removeDirectories(HTREEITEM ht);
	void removeDirectory(const string& dir, bool isFileList = false);

	bool isCurDir(const string& aDir) const;

	QueueItemInfo* getItemInfo(const string& target);

	void clearTree(HTREEITEM item);

	void moveSelected();
	void moveSelectedDir();
	void moveDir(HTREEITEM ht, const string& target);

	void moveNode(HTREEITEM item, HTREEITEM parent);

	void removeSelected();
	void removeSelectedDir();

	const string& getSelectedDir();
	const string& getDir(HTREEITEM ht);

	void removeDir(HTREEITEM ht);
	void setPriority(HTREEITEM ht, const QueueItem::Priority& p);
	void changePriority(bool inc);

	WidgetMenuPtr makeSingleMenu(QueueItemInfo* qii);
	WidgetMenuPtr makeMultiMenu();
	WidgetMenuPtr makeDirMenu();
	
	void addBrowseMenu(const WidgetMenuPtr& parent, QueueItemInfo* qii);
	void addRemoveMenu(const WidgetMenuPtr& parent, QueueItemInfo* qii);
	void addRemoveAllMenu(const WidgetMenuPtr& parent, QueueItemInfo* qii);
	void addPMMenu(const WidgetMenuPtr& parent, QueueItemInfo* qii);
	void addPriorityMenu(const WidgetMenuPtr& parent);
	void addReaddMenu(const WidgetMenuPtr& parent, QueueItemInfo* qii);
	unsigned int addUsers(const WidgetMenuPtr& menu, unsigned int startId, void (QueueFrame::*handler)(const UserPtr&), QueueItemInfo* qii, bool offline);

	void layout();
	HRESULT handleSpeaker(WPARAM wParam, LPARAM lParam);
	bool preClosing();
	void postClosing();
	
	void handleShowTreeClicked();

	void handleSearchAlternates();
	void handleBitziLookup();
	void handleCopyMagnet();
	void handleMove();
	void handleRemove();
	void handlePriority(unsigned id);
	void handlePM(const UserPtr& user);
	void handleRemoveSource(const UserPtr& user);
	void handleRemoveSources(const UserPtr& user);
	void handleBrowseList(const UserPtr& user);
	void handleReadd(const UserPtr& user);
	bool handleKeyDownFiles(int c);
	bool handleKeyDownDirs(int c);
	bool handleFilesContextMenu(SmartWin::ScreenCoordinate pt);
	bool handleDirsContextMenu(SmartWin::ScreenCoordinate pt);
	
	using MDIChildFrame<QueueFrame>::speak;
	void speak(Tasks s, Task* t) { tasks.add(s, t); speak(); }
	void speak(Tasks s, const string& msg) { tasks.add(s, new StringTask(msg)); speak(); }

	virtual void on(QueueManagerListener::Added, QueueItem* aQI) throw();
	virtual void on(QueueManagerListener::Moved, QueueItem* aQI, const string& oldTarget) throw();
	virtual void on(QueueManagerListener::Removed, QueueItem* aQI) throw();
	virtual void on(QueueManagerListener::SourcesUpdated, QueueItem* aQI) throw();
	virtual void on(QueueManagerListener::StatusUpdated, QueueItem* aQI) throw() { on(QueueManagerListener::SourcesUpdated(), aQI); }
};

#endif // !defined(QUEUE_FRAME_H)

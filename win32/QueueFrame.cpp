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
#include <dcpp/DCPlusPlus.h>

#include "QueueFrame.h"
#include "WinUtil.h"
#include "resource.h"

#include <dcpp/QueueManager.h>
#include <dcpp/ResourceManager.h>
#include <dcpp/version.h>

int QueueFrame::columnIndexes[] = { COLUMN_TARGET, COLUMN_STATUS, COLUMN_SIZE, COLUMN_DOWNLOADED, COLUMN_PRIORITY,
COLUMN_USERS, COLUMN_PATH, COLUMN_EXACT_SIZE, COLUMN_ERRORS, COLUMN_ADDED, COLUMN_TTH, COLUMN_TYPE };

int QueueFrame::columnSizes[] = { 200, 300, 75, 110, 75, 200, 200, 75, 200, 100, 125, 75 };

static ResourceManager::Strings columnNames[] = { ResourceManager::FILENAME, ResourceManager::STATUS, ResourceManager::SIZE, ResourceManager::DOWNLOADED,
ResourceManager::PRIORITY, ResourceManager::USERS, ResourceManager::PATH, ResourceManager::EXACT_SIZE, ResourceManager::ERRORS,
ResourceManager::ADDED, ResourceManager::TTH_ROOT, ResourceManager::TYPE };

#define FILE_LIST_NAME _T("File Lists")

void QueueFrame::QueueItemInfo::remove() { 
	QueueManager::getInstance()->remove(getTarget()); 
}

QueueFrame::QueueFrame(SmartWin::Widget* mdiParent) :
	SmartWin::Widget(mdiParent),
	BaseType(mdiParent),
	dirs(0),
	files(0),
	paned(0),
	showTree(0),
	dirty(true),
	usingDirMenu(false),
	queueSize(0),
	queueItems(0),
	fileLists(0)
{		
	paned = createVPaned();
	paned->setRelativePos(0.3);
	{
		WidgetTreeView::Seed cs;
		cs.style = WS_CHILD | WS_VISIBLE | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_HASLINES | TVS_SHOWSELALWAYS | TVS_DISABLEDRAGDROP;
		cs.exStyle = WS_EX_CLIENTEDGE;
		dirs = SmartWin::WidgetCreator<WidgetDirs>::create(this, cs);
		addWidget(dirs);
		dirs->setColor(WinUtil::textColor, WinUtil::bgColor);
		dirs->setNormalImageList(WinUtil::fileImages);
		dirs->onSelectionChanged(&QueueFrame::handleSelectionChanged);
		paned->setFirst(dirs);
	}
	
	{
		WidgetFiles::Seed cs;
		cs.style = WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_NOSORTHEADER | LVS_SHAREIMAGELISTS;
		cs.exStyle = WS_EX_CLIENTEDGE;
		files = SmartWin::WidgetCreator<WidgetFiles>::create(this, cs);
		files->setListViewStyle(LVS_EX_LABELTIP | LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT);
		files->setFont(WinUtil::font);
		addWidget(files);

		files->setSmallImageList(WinUtil::fileImages);
		files->createColumns(ResourceManager::getInstance()->getStrings(columnNames));
		files->setColumnOrder(WinUtil::splitTokens(SETTING(QUEUEFRAME_ORDER), columnIndexes));
		files->setColumnWidths(WinUtil::splitTokens(SETTING(QUEUEFRAME_WIDTHS), columnSizes));
		files->setColor(WinUtil::textColor, WinUtil::bgColor);
		files->setSortColumn(COLUMN_TARGET);

		paned->setSecond(files);
	}
	
	{
		WidgetCheckBox::Seed cs;
		cs.caption = _T("+/-");
		showTree = createCheckBox(cs);
		showTree->setChecked(BOOLSETTING(QUEUEFRAME_SHOW_TREE));
	}
	
	initStatus();
	statusSizes[STATUS_SHOW_TREE] = 16;
	///@todo get real resizer width
	statusSizes[STATUS_DUMMY] = 16;
	
	showTree->onClicked(&QueueFrame::handleShowTreeClicked);

	addQueueList(QueueManager::getInstance()->lockQueue());
	QueueManager::getInstance()->unlockQueue();
	QueueManager::getInstance()->addListener(this);

	onSpeaker(std::tr1::bind(&QueueFrame::handleSpeaker, this, _1, _2));
	onRaw(std::tr1::bind(&QueueFrame::handleContextMenu, this, _1, _2), SmartWin::Message(WM_CONTEXTMENU));
	
	updateStatus();	
	layout();
}

QueueFrame::~QueueFrame() {
	
}

HRESULT QueueFrame::handleSpeaker(WPARAM, LPARAM) {
	TaskQueue::List t;

	tasks.get(t);

	for(TaskQueue::Iter ti = t.begin(); ti != t.end(); ++ti) {
		if(ti->first == ADD_ITEM) {
			boost::scoped_ptr<QueueItemInfoTask> iit(static_cast<QueueItemInfoTask*>(ti->second));
			
			dcassert(files->findItem(iit->ii) == -1);
			addQueueItem(iit->ii, false);
			updateStatus();
		} else if(ti->first == REMOVE_ITEM) {
			boost::scoped_ptr<StringTask> target(static_cast<StringTask*>(ti->second));
			QueueItemInfo* ii = getItemInfo(target->str);
			if(!ii) {
				dcassert(ii);
				continue;
			}

			if(!showTree->getChecked() || isCurDir(ii->getPath()) ) {
				dcassert(files->findItem(ii) != -1);
				files->deleteItem(ii);
			}

			if(!ii->isSet(QueueItem::FLAG_USER_LIST)) {
				queueSize-=ii->getSize();
				dcassert(queueSize >= 0);
			}
			queueItems--;
			dcassert(queueItems >= 0);

			pair<DirectoryIter, DirectoryIter> i = directories.equal_range(ii->getPath());
			DirectoryIter j;
			for(j = i.first; j != i.second; ++j) {
				if(j->second == ii)
					break;
			}
			dcassert(j != i.second);
			directories.erase(j);
			if(directories.count(ii->getPath()) == 0) {
				removeDirectory(ii->getPath(), ii->isSet(QueueItem::FLAG_USER_LIST));
				if(isCurDir(ii->getPath()))
					curDir.clear();
			}

			delete ii;
			updateStatus();
			if (BOOLSETTING(BOLD_QUEUE)) {
#ifdef PORT_ME
				setDirty();
#endif
			}
			dirty = true;
		} else if(ti->first == UPDATE_ITEM) {
			boost::scoped_ptr<UpdateTask> ui(reinterpret_cast<UpdateTask*>(ti->second));
            QueueItemInfo* ii = getItemInfo(ui->target);

			ii->setPriority(ui->priority);
			ii->setStatus(ui->status);
			ii->setDownloadedBytes(ui->downloadedBytes);
			ii->setSources(ui->sources);
			ii->setBadSources(ui->badSources);

			ii->updateMask |= QueueItemInfo::MASK_PRIORITY | QueueItemInfo::MASK_USERS | QueueItemInfo::MASK_ERRORS | QueueItemInfo::MASK_STATUS | QueueItemInfo::MASK_DOWNLOADED;

			if(!showTree->getChecked() || isCurDir(ii->getPath())) {
				dcassert(files->findItem(ii) != -1);
				ii->update();
				files->updateItem(ii);
			}
		}
	}

	return 0;
}

void QueueFrame::layout() {
	SmartWin::Rectangle r(getClientAreaSize()); 

	SmartWin::Rectangle rs = layoutStatus();

	mapWidget(STATUS_SHOW_TREE, showTree);
	{
#ifdef PORT_ME
		ctrlLastLines.SetMaxTipWidth(w[0]);
#endif
	}
	
	r.size.y -= rs.size.y;
	
	bool checked = showTree->getChecked();
	if(checked && !paned->getFirst()) {
		paned->setFirst(dirs);
	} else if(!checked && paned->getFirst()) {
		paned->setFirst(0);
	}
	paned->setRect(r);
	
}

void QueueFrame::addQueueList(const QueueItem::StringMap& li) {
#ifdef PORT_ME
	ctrlQueue.SetRedraw(FALSE);
	ctrlDirs.SetRedraw(FALSE);
#endif
	for(QueueItem::StringMap::const_iterator j = li.begin(); j != li.end(); ++j) {
		QueueItem* aQI = j->second;
		QueueItemInfo* ii = new QueueItemInfo(*aQI);
		addQueueItem(ii, true);
	}

	files->resort();
#ifdef PORT_ME
	ctrlQueue.SetRedraw(TRUE);
	ctrlDirs.SetRedraw(TRUE);
	ctrlDirs.Invalidate();
#endif
}

QueueFrame::QueueItemInfo* QueueFrame::getItemInfo(const string& target) {
	string path = Util::getFilePath(target);
	DirectoryPair items = directories.equal_range(path);
	for(DirectoryIter i = items.first; i != items.second; ++i) {
		if(i->second->getTarget() == target) {
			return i->second;
		}
	}
	return 0;
}

bool QueueFrame::isCurDir(const std::string& aDir) const {
	 return Util::stricmp(curDir, aDir) == 0;
}

void QueueFrame::updateStatus() {
	int64_t total = 0;
	int cnt = files->getSelectedCount();
	if(cnt == 0) {
		cnt = files->getRowCount();
		if(showTree->getChecked()) {
			for(int i = 0; i < cnt; ++i) {
				QueueItemInfo* ii = files->getItemData(i);
				total += (ii->getSize() > 0) ? ii->getSize() : 0;
			}
		} else {
			total = queueSize;
		}
	} else {
		int i = -1;
		while( (i = files->getNextItem(i, LVNI_SELECTED)) != -1) {
			QueueItemInfo* ii = files->getItemData(i);
			total += (ii->getSize() > 0) ? ii->getSize() : 0;
		}

	}

	setStatus(STATUS_PARTIAL_COUNT, Text::toT(STRING(ITEMS) + ": " + Util::toString(cnt)));
	setStatus(STATUS_PARTIAL_BYTES, Text::toT(STRING(SIZE) + ": " + Util::formatBytes(total)));
	
	if(dirty) {
		setStatus(STATUS_TOTAL_COUNT, Text::toT(STRING(FILES) + ": " + Util::toString(queueItems)));
		setStatus(STATUS_TOTAL_BYTES, Text::toT(STRING(SIZE) + ": " + Util::formatBytes(queueSize)));
		dirty = false;
	}
}

bool QueueFrame::preClosing() {
	QueueManager::getInstance()->removeListener(this);
	return true;
}

void QueueFrame::postClosing() {
	HTREEITEM ht = dirs->getRoot();
	while(ht != NULL) {
		clearTree(ht);
		ht = dirs->getNextSibling(ht);
	}
	
	SettingsManager::getInstance()->set(SettingsManager::QUEUEFRAME_SHOW_TREE, showTree->getChecked());

	for(DirectoryIter i = directories.begin(); i != directories.end(); ++i) {
		delete i->second;
	}
	directories.clear();
	files->removeAllRows();
	
	SettingsManager::getInstance()->set(SettingsManager::QUEUEFRAME_ORDER, WinUtil::toString(files->getColumnOrder()));
	SettingsManager::getInstance()->set(SettingsManager::QUEUEFRAME_WIDTHS, WinUtil::toString(files->getColumnWidths()));
}

void QueueFrame::addQueueItem(QueueItemInfo* ii, bool noSort) {
	if(!ii->isSet(QueueItem::FLAG_USER_LIST)) {
		queueSize+=ii->getSize();
	}
	queueItems++;
	dirty = true;

	const string& dir = ii->getPath();

	bool updateDir = (directories.find(dir) == directories.end());
	directories.insert(make_pair(dir, ii));

	if(updateDir) {
		addDirectory(dir, ii->isSet(QueueItem::FLAG_USER_LIST));
	}
	if(!showTree->getChecked() || isCurDir(dir)) {
		ii->update();
		if(noSort) {
			files->insertItem(files->getRowCount(), ii);
		} else {
			files->insertItem(ii);
		}
	}
}

void QueueFrame::handleShowTreeClicked(WidgetCheckBoxPtr) {
	bool checked = showTree->getChecked();
	
	dirs->setVisible(checked);
	paned->setVisible(checked);
	
	layout();
}

void QueueFrame::handleSelectionChanged(WidgetTreeViewPtr) {
	updateQueue();
}

void QueueFrame::updateQueue() {
	files->removeAllRows();
	pair<DirectoryIter, DirectoryIter> i;
	if(showTree->getChecked()) {
		i = directories.equal_range(getSelectedDir());
	} else {
		i.first = directories.begin();
		i.second = directories.end();
	}

#ifdef PORT_ME
	ctrlQueue.SetRedraw(FALSE);
#endif
	
	for(DirectoryIter j = i.first; j != i.second; ++j) {
		QueueItemInfo* ii = j->second;
		ii->update();
		files->insertItem(files->getRowCount(), ii);
	}
	
	files->resort();

#ifdef PORT_ME
	ctrlQueue.SetRedraw(TRUE);
#endif
	
	curDir = getSelectedDir();
	updateStatus();
}

void QueueFrame::on(QueueManagerListener::Added, QueueItem* aQI) throw() {
	QueueItemInfo* ii = new QueueItemInfo(*aQI);

	speak(ADD_ITEM,	new QueueItemInfoTask(ii));
}

void QueueFrame::on(QueueManagerListener::Removed, QueueItem* aQI) throw() {
	speak(REMOVE_ITEM, aQI->getTarget());
}

void QueueFrame::on(QueueManagerListener::Moved, QueueItem* aQI, const string& oldTarget) throw() {
	speak(REMOVE_ITEM, oldTarget);
	speak(ADD_ITEM,	new QueueItemInfoTask(new QueueItemInfo(*aQI)));
}

void QueueFrame::on(QueueManagerListener::SourcesUpdated, QueueItem* aQI) throw() {
	speak(UPDATE_ITEM, new UpdateTask(*aQI));
}

void QueueFrame::QueueItemInfo::update() {
	if(display != NULL) {
		int colMask = updateMask;
		updateMask = 0;

		if(colMask & MASK_TARGET) {
			display->columns[COLUMN_TARGET] = Text::toT(Util::getFileName(getTarget()));
		}
		int online = 0;
		if(colMask & MASK_USERS || colMask & MASK_STATUS) {
			tstring tmp;

			for(QueueItem::SourceIter j = getSources().begin(); j != getSources().end(); ++j) {
				if(tmp.size() > 0)
					tmp += _T(", ");

				if(j->getUser()->isOnline())
					online++;

				tmp += WinUtil::getNicks(j->getUser());
			}
			display->columns[COLUMN_USERS] = tmp.empty() ? TSTRING(NO_USERS) : tmp;
		}
		if(colMask & MASK_STATUS) {
			if(getStatus() == QueueItem::STATUS_WAITING) {

				TCHAR buf[64];
				if(online > 0) {
					if(getSources().size() == 1) {
						display->columns[COLUMN_STATUS] = TSTRING(WAITING_USER_ONLINE);
					} else {
						_stprintf(buf, CTSTRING(WAITING_USERS_ONLINE), online, getSources().size());
						display->columns[COLUMN_STATUS] = buf;
					}
				} else {
					if(getSources().size() == 0) {
						display->columns[COLUMN_STATUS] = TSTRING(NO_USERS_TO_DOWNLOAD_FROM);
					} else if(getSources().size() == 1) {
						display->columns[COLUMN_STATUS] = TSTRING(USER_OFFLINE);
					} else if(getSources().size() == 2) {
						display->columns[COLUMN_STATUS] = TSTRING(BOTH_USERS_OFFLINE);
					} else if(getSources().size() == 3) {
						display->columns[COLUMN_STATUS] = TSTRING(ALL_3_USERS_OFFLINE);
					} else if(getSources().size() == 4) {
						display->columns[COLUMN_STATUS] = TSTRING(ALL_4_USERS_OFFLINE);
					} else {
						_stprintf(buf, CTSTRING(ALL_USERS_OFFLINE), getSources().size());
						display->columns[COLUMN_STATUS] = buf;
					}
				}
			} else if(getStatus() == QueueItem::STATUS_RUNNING) {
				display->columns[COLUMN_STATUS] = TSTRING(RUNNING);
			}
		}
		if(colMask & MASK_SIZE) {
			display->columns[COLUMN_SIZE] = (getSize() == -1) ? TSTRING(UNKNOWN) : Text::toT(Util::formatBytes(getSize()));
			display->columns[COLUMN_EXACT_SIZE] = (getSize() == -1) ? TSTRING(UNKNOWN) : Text::toT(Util::formatExactSize(getSize()));
		}
		if(colMask & MASK_DOWNLOADED) {
			if(getSize() > 0)
				display->columns[COLUMN_DOWNLOADED] = Text::toT(Util::formatBytes(getDownloadedBytes()) + " (" + Util::toString((double)getDownloadedBytes()*100.0/(double)getSize()) + "%)");
			else
				display->columns[COLUMN_DOWNLOADED].clear();
		}
		if(colMask & MASK_PRIORITY) {
			switch(getPriority()) {
		case QueueItem::PAUSED: display->columns[COLUMN_PRIORITY] = TSTRING(PAUSED); break;
		case QueueItem::LOWEST: display->columns[COLUMN_PRIORITY] = TSTRING(LOWEST); break;
		case QueueItem::LOW: display->columns[COLUMN_PRIORITY] = TSTRING(LOW); break;
		case QueueItem::NORMAL: display->columns[COLUMN_PRIORITY] = TSTRING(NORMAL); break;
		case QueueItem::HIGH: display->columns[COLUMN_PRIORITY] = TSTRING(HIGH); break;
		case QueueItem::HIGHEST: display->columns[COLUMN_PRIORITY] = TSTRING(HIGHEST); break;
		default: dcasserta(0); break;
			}
		}

		if(colMask & MASK_PATH) {
			display->columns[COLUMN_PATH] = Text::toT(getPath());
		}

		if(colMask & MASK_ERRORS) {
			tstring tmp;
			for(QueueItem::SourceIter j = getBadSources().begin(); j != getBadSources().end(); ++j) {
				if(!j->isSet(QueueItem::Source::FLAG_REMOVED)) {
					if(tmp.size() > 0)
						tmp += _T(", ");
					tmp += WinUtil::getNicks(j->getUser());
					tmp += _T(" (");
					if(j->isSet(QueueItem::Source::FLAG_FILE_NOT_AVAILABLE)) {
						tmp += TSTRING(FILE_NOT_AVAILABLE);
					} else if(j->isSet(QueueItem::Source::FLAG_PASSIVE)) {
						tmp += TSTRING(PASSIVE_USER);
					} else if(j->isSet(QueueItem::Source::FLAG_CRC_FAILED)) {
						tmp += TSTRING(SFV_INCONSISTENCY);
					} else if(j->isSet(QueueItem::Source::FLAG_BAD_TREE)) {
						tmp += TSTRING(INVALID_TREE);
					} else if(j->isSet(QueueItem::Source::FLAG_SLOW_SOURCE)) {
						tmp += TSTRING(SOURCE_TOO_SLOW);
					} else if(j->isSet(QueueItem::Source::FLAG_NO_TTHF)) {
						tmp += TSTRING(SOURCE_TOO_OLD);
					}
					tmp += ')';
				}
			}
			display->columns[COLUMN_ERRORS] = tmp.empty() ? TSTRING(NO_ERRORS) : tmp;
		}

		if(colMask & MASK_ADDED) {
			display->columns[COLUMN_ADDED] = Text::toT(Util::formatTime("%Y-%m-%d %H:%M", getAdded()));
		}
		if(colMask & MASK_TTH) {
			display->columns[COLUMN_TTH] = Text::toT(getTTH().toBase32());
		}
		if(colMask & MASK_TYPE) {
			display->columns[COLUMN_TYPE] = Text::toT(Util::getFileExt(getTarget()));
			if(display->columns[COLUMN_TYPE].size() > 0 && display->columns[COLUMN_TYPE][0] == '.')
				display->columns[COLUMN_TYPE].erase(0, 1);
		}
	}
}

QueueFrame::DirItemInfo::DirItemInfo(const string& dir_) : dir(dir_), text(dir_.empty() ? Util::emptyStringT : Text::toT(dir_.substr(0, dir_.length()-1))) {
	
}

int QueueFrame::DirItemInfo::getImage() {
	return WinUtil::getDirIconIndex();
}

int QueueFrame::DirItemInfo::getSelectedImage() {
	return WinUtil::getDirIconIndex();
}

HTREEITEM QueueFrame::addDirectory(const string& dir, bool isFileList /* = false */, HTREEITEM startAt /* = NULL */) {
	if(isFileList) {
		// We assume we haven't added it yet, and that all filelists go to the same
		// directory...
		dcassert(fileLists == NULL);
		fileLists = dirs->insert(NULL, new DirItemInfo(dir, FILE_LIST_NAME));
		return fileLists;
	}

	// More complicated, we have to find the last available tree item and then see...
	string::size_type i = 0;
	string::size_type j;

	HTREEITEM next = NULL;
	HTREEITEM parent = NULL;

	if(startAt == NULL) {
		// First find the correct drive letter
		dcassert(dir[1] == ':');
		dcassert(dir[2] == '\\');

		next = dirs->getRoot();

		while(next != NULL) {
			if(next != fileLists) {
				if(Util::strnicmp(getDir(next), dir, 3) == 0)
					break;
			}
			next = dirs->getNextSibling(next);
		}

		if(next == NULL) {
			// First addition, set commonStart to the dir minus the last part...
			i = dir.rfind('\\', dir.length()-2);
			if(i != string::npos) {
				next = dirs->insert(NULL, new DirItemInfo(dir.substr(0, i+1)));
			} else {
				dcassert(dir.length() == 3);
				next = dirs->insert(NULL, new DirItemInfo(dir, Text::toT(dir)));
			}
		}

		// Ok, next now points to x:\... find how much is common

		DirItemInfo* rootInfo = dirs->getData(next);
		const string& rootStr = rootInfo->getDir();

		i = 0;

		for(;;) {
			j = dir.find('\\', i);
			if(j == string::npos)
				break;
			if(Util::strnicmp(dir.c_str() + i, rootStr.c_str() + i, j - i + 1) != 0)
				break;
			i = j + 1;
		}

		if(i < rootStr.length()) {
			HTREEITEM oldRoot = next;

			// Create a new root
			HTREEITEM newRoot = dirs->insert(NULL, new DirItemInfo(rootStr.substr(0, i)));

			parent = addDirectory(rootStr, false, newRoot);

			next = dirs->getChild(oldRoot);
			while(next != NULL) {
				moveNode(next, parent);
				next = dirs->getChild(oldRoot);
			}
			delete rootInfo;
			dirs->deleteItem(oldRoot);
			parent = newRoot;
		} else {
			// Use this root as parent
			parent = next;
			next = dirs->getChild(parent);
		}
	} else {
		parent = startAt;
		next = dirs->getChild(parent);
		i = getDir(parent).length();
		dcassert(Util::strnicmp(getDir(parent), dir, i) == 0);
	}

	HTREEITEM firstParent = parent;

	while( i < dir.length() ) {
		while(next != NULL) {
			if(next != fileLists) {
				const string& n = getDir(next);
				if(Util::strnicmp(n.c_str()+i, dir.c_str()+i, n.length()-i) == 0) {
					// Found a part, we assume it's the best one we can find...
					i = n.length();

					parent = next;
					next = dirs->getChild(next);
					break;
				}
			}
			next = dirs->getNextSibling(next);
		}

		if(next == NULL) {
			// We didn't find it, add...
			j = dir.find('\\', i);
			dcassert(j != string::npos);
			parent = dirs->insert(parent, new DirItemInfo(dir.substr(0, j+1), Text::toT(dir.substr(i, j-i))));
			i = j + 1;
		}
	}

#ifdef PORT_ME
	if(firstParent != NULL)
		ctrlDirs.Expand(firstParent);
#endif
	return parent;
}

void QueueFrame::removeDirectory(const string& dir, bool isFileList /* = false */) {
	// First, find the last name available
	string::size_type i = 0;

	HTREEITEM next = dirs->getRoot();
	HTREEITEM parent = NULL;

	if(isFileList) {
		dcassert(fileLists != NULL);
		delete dirs->getData(fileLists);
		dirs->deleteItem(fileLists);
		fileLists = NULL;
		return;
	} else {
		while(i < dir.length()) {
			while(next != NULL) {
				if(next != fileLists) {
					const string& n = getDir(next);
					if(Util::strnicmp(n.c_str()+i, dir.c_str()+i, n.length()-i) == 0) {
						// Match!
						parent = next;
						next = dirs->getChild(next);
						i = n.length();
						break;
					}
				}
				next = dirs->getNextSibling(next);
			}
			if(next == NULL)
				break;
		}
	}

	next = parent;

	while((dirs->getChild(next) == NULL) && (directories.find(getDir(next)) == directories.end())) {
		delete dirs->getData(next);
		parent = dirs->getParent(next);

		dirs->deleteItem(next);
		if(parent == NULL)
			break;
		next = parent;
	}
}

void QueueFrame::removeDirectories(HTREEITEM ht) {
	HTREEITEM next = dirs->getChild(ht);
	while(next != NULL) {
		removeDirectories(next);
		next = dirs->getNextSibling(ht);
	}
	delete dirs->getData(ht);
	dirs->deleteItem(ht);
}

void QueueFrame::removeSelected() {
#ifdef PORT_ME
	if(!BOOLSETTING(CONFIRM_ITEM_REMOVAL) || MessageBox(CTSTRING(REALLY_REMOVE), _T(APPNAME) _T(" ") _T(VERSIONSTRING), MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDYES)
		ctrlQueue.forEachSelected(&QueueItemInfo::remove);
#endif
}

void QueueFrame::removeSelectedDir() {
#ifdef PORT_ME
	if(!BOOLSETTING(CONFIRM_ITEM_REMOVAL) || MessageBox(CTSTRING(REALLY_REMOVE), _T(APPNAME) _T(" ") _T(VERSIONSTRING), MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDYES)
		removeDir(ctrlDirs.GetSelectedItem());
#endif
}

void QueueFrame::moveSelected() {

	int n = files->getSelectedCount();
	if(n == 1) {
		// Single file, get the full filename and move...
		QueueItemInfo* ii = files->getSelectedItem();
		tstring target = Text::toT(ii->getTarget());
		tstring ext = Util::getFileExt(target);
		tstring ext2;
		if (!ext.empty())
		{
			ext = ext.substr(1); // remove leading dot so default extension works when browsing for file
			ext2 = _T("*.") + ext;
			ext2 += (TCHAR)0;
			ext2 += _T("*.") + ext;
		}
		ext2 += _T("*.*");
		ext2 += (TCHAR)0;
		ext2 += _T("*.*");
		ext2 += (TCHAR)0;

		tstring path = Text::toT(ii->getPath());
#ifdef PORT_ME
		if(WinUtil::browseFile(target, handle(), true, path, ext2.c_str(), ext.empty() ? NULL : ext.c_str())) {
			QueueManager::getInstance()->move(ii->getTarget(), Text::fromT(target));
		}
#endif
	} else if(n > 1) {
		tstring name;
		if(showTree->getChecked()) {
			name = Text::toT(curDir);
		}
#ifdef PORT_ME
		if(WinUtil::browseDirectory(name, handle())) {
			int i = -1;
			while( (i = ctrlQueue.GetNextItem(i, LVNI_SELECTED)) != -1) {
				QueueItemInfo* ii = ctrlQueue.getItemData(i);
				QueueManager::getInstance()->move(ii->getTarget(), Text::fromT(name) + Util::getFileName(ii->getTarget()));
			}
		}
#endif
	}
}

void QueueFrame::moveSelectedDir() {
	if(dirs->getSelected() == NULL)
		return;

	dcassert(!curDir.empty());
	tstring name = Text::toT(curDir);

#ifdef PORT_ME
	if(WinUtil::browseDirectory(name, m_hWnd)) {
		moveDir(ctrlDirs.GetSelectedItem(), Text::fromT(name));
	}
#endif
}

void QueueFrame::moveDir(HTREEITEM ht, const string& target) {
	HTREEITEM next = dirs->getChild(ht);
	while(next != NULL) {
		// must add path separator since getLastDir only give us the name
		moveDir(next, target + Util::getLastDir(getDir(next)) + PATH_SEPARATOR);
		next = dirs->getNextSibling(next);
	}
	const string& dir = getDir(ht);

	DirectoryPair p = directories.equal_range(dir);

	for(DirectoryIter i = p.first; i != p.second; ++i) {
		QueueItemInfo* ii = i->second;
		QueueManager::getInstance()->move(ii->getTarget(), target + Util::getFileName(ii->getTarget()));
	}
}

void QueueFrame::handleSearchAlternates(WidgetMenuPtr menu, unsigned id) {
	if(files->getSelectedCount() == 1) {
#ifdef PORT_ME
		WinUtil::searchHash(files->getSelectedItem()->getTTH());
#endif
	}
}

void QueueFrame::handleBitziLookup(WidgetMenuPtr menu, unsigned id) {
	if(files->getSelectedCount() == 1) {
#ifdef PORT_ME
		WinUtil::bitziLink(files->getSelectedItem()->getTTH());
#endif
	}
}

void QueueFrame::handleCopyMagnet(WidgetMenuPtr menu, unsigned id) {
	if(files->getSelectedCount() == 1) {
		QueueItemInfo* ii = files->getSelectedItem();
#ifdef PORT_ME
		WinUtil::copyMagnet(ii->getTTH(), Text::toT(Util::getFileName(ii->getTarget())));
#endif
	}
}

void QueueFrame::handleBrowseList(WidgetMenuPtr menu, unsigned id) {

	if(files->getSelectedCount() == 1) {
#ifdef PORT_ME
		CMenuItemInfo mi;
		mi.fMask = MIIM_DATA;

		browseMenu.GetMenuItemInfo(wID, FALSE, &mi);
		QueueItem::Source* s = (QueueItem::Source*)mi.dwItemData;
		try {
			QueueManager::getInstance()->addList(s->getUser(), QueueItem::FLAG_CLIENT_VIEW);
		} catch(const Exception&) {
		}
#endif
	}
}

void QueueFrame::handleReadd(WidgetMenuPtr menu, unsigned id) {

	if(files->getSelectedCount() == 1) {
		QueueItemInfo* ii = files->getSelectedItem();

#ifdef PORT_ME
		CMenuItemInfo mi;
		mi.fMask = MIIM_DATA;

		readdMenu.GetMenuItemInfo(wID, FALSE, &mi);
		if(wID == IDC_READD) {
			// re-add all sources
			for(QueueItem::SourceIter s = ii->getBadSources().begin(); s != ii->getBadSources().end(); ++s) {
				QueueManager::getInstance()->readd(ii->getTarget(), s->getUser());
			}
		} else {
			QueueItem::Source* s = (QueueItem::Source*)mi.dwItemData;
			try {
				QueueManager::getInstance()->readd(ii->getTarget(), s->getUser());
			} catch(const Exception& e) {
				ctrlStatus.SetText(0, Text::toT(e.getError()).c_str());
			}
		}
#endif
	}
}

void QueueFrame::handleRemove(WidgetMenuPtr menu, unsigned id) {
	usingDirMenu ? removeSelectedDir() : removeSelected();
}

void QueueFrame::handleMove(WidgetMenuPtr menu, unsigned id) {
	usingDirMenu ? moveSelectedDir() : moveSelected();
}

void QueueFrame::handleRemoveSource(WidgetMenuPtr menu, unsigned id) {

	if(files->getSelectedCount() == 1) {
		QueueItemInfo* ii = files->getSelectedItem();

		if(id == IDC_REMOVE_SOURCE) {
			for(QueueItem::SourceIter si = ii->getSources().begin(); si != ii->getSources().end(); ) {
				QueueManager::getInstance()->removeSource(ii->getTarget(), si->getUser(), QueueItem::Source::FLAG_REMOVED);
			}
		} else {
#ifdef PORT_ME
			CMenuItemInfo mi;
			mi.fMask = MIIM_DATA;

			removeMenu.GetMenuItemInfo(wID, FALSE, &mi);
			QueueItem::Source* s = (QueueItem::Source*)mi.dwItemData;
			QueueManager::getInstance()->removeSource(ii->getTarget(), s->getUser(), QueueItem::Source::FLAG_REMOVED);
#endif
		}
	}
}

void QueueFrame::handleRemoveSources(WidgetMenuPtr menu, unsigned id) {
#ifdef PORT_ME
	CMenuItemInfo mi;
	mi.fMask = MIIM_DATA;
	removeAllMenu.GetMenuItemInfo(wID, FALSE, &mi);
	QueueItem::Source* s = (QueueItem::Source*)mi.dwItemData;
	QueueManager::getInstance()->removeSource(s->getUser(), QueueItem::Source::FLAG_REMOVED);
	return 0;
#endif
}

void QueueFrame::handlePM(WidgetMenuPtr menu, unsigned id) {
	if(files->getSelectedCount() == 1) {
#ifdef PORT_ME
		CMenuItemInfo mi;
		mi.fMask = MIIM_DATA;

		pmMenu.GetMenuItemInfo(wID, FALSE, &mi);
		QueueItem::Source* s = (QueueItem::Source*)mi.dwItemData;
		PrivateFrame::openWindow(s->getUser());
#endif
	}
}

void QueueFrame::handlePriority(WidgetMenuPtr menu, unsigned id) {
	QueueItem::Priority p;

	switch(id) {
		case IDC_PRIORITY_PAUSED: p = QueueItem::PAUSED; break;
		case IDC_PRIORITY_LOWEST: p = QueueItem::LOWEST; break;
		case IDC_PRIORITY_LOW: p = QueueItem::LOW; break;
		case IDC_PRIORITY_NORMAL: p = QueueItem::NORMAL; break;
		case IDC_PRIORITY_HIGH: p = QueueItem::HIGH; break;
		case IDC_PRIORITY_HIGHEST: p = QueueItem::HIGHEST; break;
		default: p = QueueItem::DEFAULT; break;
	}

	if(usingDirMenu) {
		setPriority(dirs->getSelected(), p);
	} else {
		std::vector<size_t> selected = files->getSelectedRows();
		for(std::vector<size_t>::iterator i = selected.begin(); i != selected.end(); ++i) {
			QueueManager::getInstance()->setPriority(files->getItemData(*i)->getTarget(), p);
		}
	}
}

void QueueFrame::removeDir(HTREEITEM ht) {
	if(ht == NULL)
		return;
	HTREEITEM child = dirs->getChild(ht);
	while(child) {
		removeDir(child);
		child = dirs->getNextSibling(child);
	}
	const string& name = getDir(ht);
	DirectoryPair dp = directories.equal_range(name);
	for(DirectoryIter i = dp.first; i != dp.second; ++i) {
		QueueManager::getInstance()->remove(i->second->getTarget());
	}
}

/*
 * @param inc True = increase, False = decrease
 */
void QueueFrame::changePriority(bool inc){
	std::vector<size_t> selected = files->getSelectedRows();
	for(std::vector<size_t>::iterator i = selected.begin(); i != selected.end(); ++i) {
		QueueItemInfo* ii = files->getItemData(*i);
		QueueItem::Priority p = ii->getPriority();

		if ((inc && p == QueueItem::HIGHEST) || (!inc && p == QueueItem::PAUSED)){
			// Trying to go higher than HIGHEST or lower than PAUSED
			// so do nothing
			continue;
		}

		switch(p){
			case QueueItem::HIGHEST: p = inc ? QueueItem::HIGHEST : QueueItem::HIGH; break;
			case QueueItem::HIGH:    p = inc ? QueueItem::HIGHEST : QueueItem::NORMAL; break;
			case QueueItem::NORMAL:  p = inc ? QueueItem::HIGH    : QueueItem::LOW; break;
			case QueueItem::LOW:     p = inc ? QueueItem::NORMAL  : QueueItem::LOWEST; break;
			case QueueItem::LOWEST:  p = inc ? QueueItem::LOW     : QueueItem::PAUSED; break;
			case QueueItem::PAUSED:  p = inc ? QueueItem::LOWEST : QueueItem::PAUSED; break;
		}

		QueueManager::getInstance()->setPriority(ii->getTarget(), p);
	}
}

void QueueFrame::setPriority(HTREEITEM ht, const QueueItem::Priority& p) {
	if(ht == NULL)
		return;
	HTREEITEM child = dirs->getChild(ht);
	while(child) {
		setPriority(child, p);
		child = dirs->getNextSibling(child);
	}
	const string& name = getDir(ht);
	DirectoryPair dp = directories.equal_range(name);
	for(DirectoryIter i = dp.first; i != dp.second; ++i) {
		QueueManager::getInstance()->setPriority(i->second->getTarget(), p);
	}
}


void QueueFrame::clearTree(HTREEITEM item) {
	HTREEITEM next = dirs->getChild(item);
	while(next != NULL) {
		clearTree(next);
		next = dirs->getNextSibling(next);
	}
	delete dirs->getData(item);
}

// Put it here to avoid a copy for each recursion...
static TCHAR tmpBuf[1024];
void QueueFrame::moveNode(HTREEITEM item, HTREEITEM parent) {
	TVINSERTSTRUCT tvis;
	memset(&tvis, 0, sizeof(tvis));
	tvis.itemex.hItem = item;
	tvis.itemex.mask = TVIF_CHILDREN | TVIF_HANDLE | TVIF_IMAGE | TVIF_INTEGRAL | TVIF_PARAM |
		TVIF_SELECTEDIMAGE | TVIF_STATE | TVIF_TEXT;
	tvis.itemex.pszText = tmpBuf;
	tvis.itemex.cchTextMax = 1024;
	dirs->getItem(&tvis.itemex);
	tvis.hInsertAfter =	TVI_SORT;
	tvis.hParent = parent;
	HTREEITEM ht = dirs->insert(&tvis);
	HTREEITEM next = dirs->getChild(item);
	while(next != NULL) {
		moveNode(next, ht);
		next = dirs->getChild(item);
	}
	dirs->deleteItem(item);
}

const string& QueueFrame::getSelectedDir() {
	DirItemInfo* info = dirs->getSelectedData();
	return info == NULL ? Util::emptyString : info->getDir();
}

const string& QueueFrame::getDir(HTREEITEM item) {
	DirItemInfo* info = dirs->getData(item);
	return info == NULL ? Util::emptyString : info->getDir();
}

QueueFrame::WidgetMenuPtr QueueFrame::makeSingleMenu(QueueItemInfo* qii) {
	WidgetMenuPtr menu = createMenu(true);

	menu->appendItem(IDC_SEARCH_ALTERNATES, TSTRING(SEARCH_FOR_ALTERNATES), &QueueFrame::handleSearchAlternates);
	menu->appendItem(IDC_BITZI_LOOKUP, TSTRING(LOOKUP_AT_BITZI), &QueueFrame::handleBitziLookup);
	menu->appendItem(IDC_COPY_MAGNET, TSTRING(COPY_MAGNET), &QueueFrame::handleCopyMagnet);
	menu->appendItem(IDC_MOVE, TSTRING(MOVE), &QueueFrame::handleMove);
	addPriorityMenu(menu);
	addBrowseMenu(menu, qii);
	addPMMenu(menu, qii);
	menu->appendSeparatorItem();
	addReaddMenu(menu, qii);
	addRemoveMenu(menu, qii);
	addRemoveAllMenu(menu, qii);
	menu->appendItem(IDC_REMOVE, TSTRING(REMOVE), &QueueFrame::handleRemove);
	
	return menu;
}

QueueFrame::WidgetMenuPtr QueueFrame::makeMultiMenu() {
	WidgetMenuPtr menu = createMenu(true);

	addPriorityMenu(menu);
	
	menu->appendItem(IDC_MOVE, TSTRING(MOVE), &QueueFrame::handleMove);
	menu->appendSeparatorItem();
	menu->appendItem(IDC_REMOVE, TSTRING(REMOVE), &QueueFrame::handleRemove);
	return menu;
}

QueueFrame::WidgetMenuPtr QueueFrame::makeDirMenu() {
	WidgetMenuPtr menu = createMenu(true);

	addPriorityMenu(menu);
	menu->appendItem(IDC_MOVE, TSTRING(MOVE), &QueueFrame::handleMove);
	menu->appendSeparatorItem();
	menu->appendItem(IDC_REMOVE, TSTRING(REMOVE), &QueueFrame::handleRemove);
	return menu;
}

void QueueFrame::addPriorityMenu(const WidgetMenuPtr& parent) {
	WidgetMenuPtr menu = parent->appendPopup(TSTRING(SET_PRIORITY));
	menu->appendItem(IDC_PRIORITY_PAUSED, TSTRING(PAUSED), &QueueFrame::handlePriority);
	menu->appendItem(IDC_PRIORITY_LOWEST, TSTRING(LOWEST), &QueueFrame::handlePriority);
	menu->appendItem(IDC_PRIORITY_LOW, TSTRING(LOW), &QueueFrame::handlePriority);
	menu->appendItem(IDC_PRIORITY_NORMAL, TSTRING(NORMAL), &QueueFrame::handlePriority);
	menu->appendItem(IDC_PRIORITY_HIGH, TSTRING(HIGH), &QueueFrame::handlePriority);
	menu->appendItem(IDC_PRIORITY_HIGHEST, TSTRING(HIGHEST), &QueueFrame::handlePriority);
}

void QueueFrame::addBrowseMenu(const WidgetMenuPtr& parent, QueueItemInfo* qii) {
	unsigned int pos = parent->getCount();
	WidgetMenuPtr menu = parent->appendPopup(TSTRING(GET_FILE_LIST));
	if(addUsers(menu, IDC_BROWSELIST, &QueueFrame::handleBrowseList, qii, false) == 0) {
		::EnableMenuItem(reinterpret_cast<HMENU>(menu->handle()), pos, MF_BYPOSITION | MF_GRAYED);
	}
}

void QueueFrame::addPMMenu(const WidgetMenuPtr& parent, QueueItemInfo* qii) {
	unsigned int pos = parent->getCount();
	WidgetMenuPtr menu = parent->appendPopup(TSTRING(SEND_PRIVATE_MESSAGE));
	if(addUsers(menu, IDC_PM, &QueueFrame::handlePM, qii, false) == 0) {
		::EnableMenuItem(reinterpret_cast<HMENU>(menu->handle()), pos, MF_BYPOSITION | MF_GRAYED);
	}
}
void QueueFrame::addReaddMenu(const WidgetMenuPtr& parent, QueueItemInfo* qii) {
	unsigned int pos = parent->getCount();
	WidgetMenuPtr menu = parent->appendPopup(TSTRING(READD_SOURCE));
	
	menu->appendItem(IDC_READD, TSTRING(ALL), &QueueFrame::handleReadd);
	menu->appendSeparatorItem();
	if(addUsers(menu, IDC_READD + 1, &QueueFrame::handleReadd, qii, true) == 0) {
		::EnableMenuItem(reinterpret_cast<HMENU>(menu->handle()), pos, MF_BYPOSITION | MF_GRAYED);
	}
}

void QueueFrame::addRemoveMenu(const WidgetMenuPtr& parent, QueueItemInfo* qii) {
	unsigned int pos = parent->getCount();
	WidgetMenuPtr menu = parent->appendPopup(TSTRING(REMOVE_SOURCE));
	menu->appendItem(IDC_REMOVE_SOURCE, TSTRING(ALL), &QueueFrame::handleRemoveSource);
	menu->appendSeparatorItem();
	if(addUsers(menu, IDC_REMOVE_SOURCE + 1, &QueueFrame::handleRemoveSource, qii, true) == 0) {
		::EnableMenuItem(reinterpret_cast<HMENU>(menu->handle()), pos, MF_BYPOSITION | MF_GRAYED);
	}
}

void QueueFrame::addRemoveAllMenu(const WidgetMenuPtr& parent, QueueItemInfo* qii) {
	unsigned int pos = parent->getCount();
	WidgetMenuPtr menu = parent->appendPopup(TSTRING(REMOVE_FROM_ALL));
	if(addUsers(menu, IDC_REMOVE_SOURCES, &QueueFrame::handleRemoveSources, qii, true) == 0) {
		::EnableMenuItem(reinterpret_cast<HMENU>(menu->handle()), pos, MF_BYPOSITION | MF_GRAYED);
	}
}

unsigned int QueueFrame::addUsers(const WidgetMenuPtr& menu, unsigned int startId, WidgetMenu::itsVoidMenuFunctionTakingUInt handler, QueueItemInfo* qii, bool offline) {
	unsigned int id = startId;
	for(QueueItem::SourceIter i = qii->getSources().begin(); i != qii->getSources().end(); ++i) {
		QueueItem::Source& source = *i;
		if(offline || source.getUser()->isOnline()) {
			tstring nick = WinUtil::escapeMenu(WinUtil::getNicks(source.getUser()));
			menu->appendItem(id++, nick, reinterpret_cast<ULONG_PTR>(&source), handler);
		}
	}
	return id - startId;
}

HRESULT QueueFrame::handleContextMenu(WPARAM wParam, LPARAM lParam) {
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
	if (reinterpret_cast<HWND>(wParam) == files->handle() && files->getSelectedCount() > 0) {
		if(pt.x == -1 || pt.y == -1) {
			pt = files->getContextMenuPos();
		}

		usingDirMenu = false;
		WidgetMenuPtr contextMenu;
		
		if(files->getSelectedCount() == 1) {
			QueueItemInfo* ii = files->getSelectedItem();
			contextMenu = makeSingleMenu(ii);
		} else {
			contextMenu = makeMultiMenu();
		}
		contextMenu->trackPopupMenu(this, pt.x, pt.y, TPM_LEFTALIGN | TPM_RIGHTBUTTON);

		return TRUE;
	} else if (reinterpret_cast<HWND>(wParam) == dirs->handle()) {
		if(pt.x == -1 && pt.y == -1) {
			pt = dirs->getContextMenuPos();
		} else {
			dirs->select(pt);
		}
		
		if(dirs->getSelected() == NULL) {
			return FALSE;
		}
		usingDirMenu = true;
		WidgetMenuPtr contextMenu = makeDirMenu();
		contextMenu->trackPopupMenu(this, pt.x, pt.y, TPM_LEFTALIGN | TPM_RIGHTBUTTON);

		return TRUE;
	}

	return FALSE;
}

#ifdef PORT_ME

LRESULT QueueFrame::onKeyDown(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NMLVKEYDOWN* kd = (NMLVKEYDOWN*) pnmh;
	if(kd->wVKey == VK_DELETE) {
		removeSelected();
	} else if(kd->wVKey == VK_ADD){
		// Increase Item priority
		changePriority(true);
	} else if(kd->wVKey == VK_SUBTRACT){
		// Decrease item priority
		changePriority(false);
	} else if(kd->wVKey == VK_TAB) {
		onTab();
	}
	return 0;
}

#endif


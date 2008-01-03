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

#include <dcpp/SettingsManager.h>
#include <dcpp/QueueItem.h>
#include <dcpp/QueueManager.h>
#include <dcpp/Download.h>

#include "DownloadsFrame.h"
#include "HoldRedraw.h"

int DownloadsFrame::columnIndexes[] = { COLUMN_FILE, COLUMN_PATH, COLUMN_STATUS, COLUMN_TIMELEFT, COLUMN_SPEED, COLUMN_SIZE };
int DownloadsFrame::columnSizes[] = { 200, 300, 150, 200, 125, 100};

static ResourceManager::Strings columnNames[] = { ResourceManager::FILENAME, ResourceManager::PATH, 
	ResourceManager::STATUS, ResourceManager::TIME_LEFT, ResourceManager::SPEED, ResourceManager::SIZE };

DownloadsFrame::DownloadsFrame(SmartWin::WidgetTabView* mdiParent) : 
	BaseType(mdiParent, T_("Downloads")),
	downloads(0),
	startup(true)
{
	{
		downloads = SmartWin::WidgetCreator<WidgetDownloads>::create(this, WinUtil::Seeds::listView);
		addWidget(downloads);

		downloads->createColumns(ResourceManager::getInstance()->getStrings(columnNames));
		downloads->setColumnOrder(WinUtil::splitTokens(SETTING(HUBFRAME_ORDER), columnIndexes));
		downloads->setColumnWidths(WinUtil::splitTokens(SETTING(HUBFRAME_WIDTHS), columnSizes));
		downloads->setSort(COLUMN_STATUS);
		downloads->setColor(WinUtil::textColor, WinUtil::bgColor);

		downloads->onKeyDown(std::tr1::bind(&DownloadsFrame::handleKeyDown, this, _1));
		downloads->onContextMenu(std::tr1::bind(&DownloadsFrame::handleContextMenu, this, _1));
	}

	initStatus();

	layout();
		
	startup = false;
	
	onSpeaker(std::tr1::bind(&DownloadsFrame::handleSpeaker, this, _1, _2));
}

DownloadsFrame::~DownloadsFrame() {
	
}

void DownloadsFrame::layout() {
	SmartWin::Rectangle r(SmartWin::Point(0, 0), getClientAreaSize());

	layoutStatus(r);

	downloads->setBounds(r);
}

bool DownloadsFrame::preClosing() {
	return true;
}

void DownloadsFrame::postClosing() {
	SettingsManager::getInstance()->set(SettingsManager::DOWNLOADSFRAME_ORDER, WinUtil::toString(downloads->getColumnOrder()));
	SettingsManager::getInstance()->set(SettingsManager::DOWNLOADSFRAME_WIDTHS, WinUtil::toString(downloads->getColumnWidths()));
}

DownloadsFrame::DownloadInfo::DownloadInfo(const string& filename, int64_t size_) : path(filename), done(0), size(size), users(0) {
	columns[COLUMN_FILE] = Text::toT(Util::getFileName(filename));
	columns[COLUMN_PATH] = Text::toT(Util::getFilePath(filename));
	columns[COLUMN_SIZE] = Text::toT(Util::toString(size));
}

void DownloadsFrame::DownloadInfo::update(const DownloadsFrame::TickInfo& ti) {
	users = ti.users;
	done = ti.done; // TODO Add done from queuemanager...
	bps = ti.bps;
}

void DownloadsFrame::DownloadInfo::update() {
	if(users == 0) {
		columns[COLUMN_STATUS] = T_("Waiting for slot");
		columns[COLUMN_TIMELEFT].clear();
		columns[COLUMN_SPEED].clear();
	} else {
		double timeleft = bps > 0 ? (size - done) / bps : 0;
		columns[COLUMN_STATUS] = str(TFN_("Downloading from %1% user", "Downloading from %1% users", users) % users);
		columns[COLUMN_TIMELEFT] = Text::toT(Util::formatSeconds(static_cast<int64_t>(timeleft)));
		columns[COLUMN_SPEED] = str(TF_("%1%/s") % Text::toT(Util::formatBytes(static_cast<int64_t>(bps))));
	}
}

bool DownloadsFrame::handleKeyDown(int c) {
	switch(c) {
	}
	return false;
}

bool DownloadsFrame::handleContextMenu(SmartWin::ScreenCoordinate pt) {
	if (downloads->hasSelection()) {
		if(pt.x() == -1 && pt.y() == -1) {
			pt = downloads->getContextMenuPos();
		}

		WidgetMenuPtr menu = createMenu(true);
		
		menu->trackPopupMenu(this, pt, TPM_LEFTALIGN | TPM_RIGHTBUTTON);

		return true;
	}
	return false;
}

int DownloadsFrame::find(const string& path) {
	for(size_t i = 0; i < downloads->size(); ++i) {
		DownloadInfo* di = downloads->getData(i);
		if(Util::stricmp(di->path, path) == 0) {
			return i;
		}
	}
	return -1;
}

LRESULT DownloadsFrame::handleSpeaker(WPARAM wParam, LPARAM lParam) {
	if(wParam == SPEAKER_TICK) {
		boost::scoped_ptr<TickInfo> ti(reinterpret_cast<TickInfo*>(lParam));
		int i = find(ti->path);
		if(i == -1) {
			// TODO get size
			i = downloads->insert(new DownloadInfo(ti->path, 0));
		}
		DownloadInfo* di = downloads->getData(i);
		di->update(*ti);
	} else if(wParam == SPEAKER_DISCONNECTED) {
		boost::scoped_ptr<string> path(reinterpret_cast<string*>(lParam));
		
		int i = find(*path);
		if(i != -1) {
			DownloadInfo* di = downloads->getData(i);
			di->users--;
			di->update();
		}
	} else if(wParam == SPEAKER_REMOVED) {
		boost::scoped_ptr<string> path(reinterpret_cast<string*>(lParam));
		int i = find(*path);
		if(i != -1) {
			downloads->erase(i);
		}
	}
	return 0;
}

void DownloadsFrame::on(DownloadManagerListener::Tick, const DownloadList& l) throw() {
	std::vector<TickInfo*> dis;
	for(DownloadList::const_iterator i = l.begin(); i != l.end(); ++i) {
		Download* d = *i;
		TickInfo* ti = 0;
		for(std::vector<TickInfo*>::iterator j = dis.begin(); j != dis.end(); ++j) {
			TickInfo* ti2 = *j;
			if(Util::stricmp(ti2->path, d->getPath()) == 0) {
				ti = ti2;
				break;
			}
		}
		if(!ti) {
			ti = new TickInfo(d->getPath());
		}
		ti->users++;
		ti->bps += d->getAverageSpeed();
		ti->done += d->getPos();
	}
	
	for(std::vector<TickInfo*>::iterator i = dis.begin(); i != dis.end(); ++i) {
		speak(SPEAKER_TICK, reinterpret_cast<LPARAM>(*i));
	}
}

void DownloadsFrame::on(DownloadManagerListener::Complete, Download* d) throw() {
	speak(SPEAKER_DISCONNECTED, reinterpret_cast<LPARAM>(new string(d->getPath())));
}

void DownloadsFrame::on(DownloadManagerListener::Failed, Download* d, const string&) throw() {
	speak(SPEAKER_DISCONNECTED, reinterpret_cast<LPARAM>(new string(d->getPath())));
}

void DownloadsFrame::on(QueueManagerListener::Removed, QueueItem* qi) throw() {
	speak(SPEAKER_REMOVED, reinterpret_cast<LPARAM>(new string(qi->getTarget())));
}

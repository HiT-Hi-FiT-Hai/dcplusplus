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

#ifndef DCPLUSPLUS_WIN32_DOWNLOADS_FRAME_H
#define DCPLUSPLUS_WIN32_DOWNLOADS_FRAME_H

#include <dcpp/DownloadManagerListener.h>
#include <dcpp/QueueManagerListener.h>
#include <dcpp/forward.h>

#include "TypedListView.h"
#include "StaticFrame.h"
#include "AspectSpeaker.h"

class DownloadsFrame : 
	public StaticFrame<DownloadsFrame>,
	public DownloadManagerListener, 
	public QueueManagerListener
{
public:
	enum Status {
		STATUS_STATUS,
		STATUS_LAST
	};
protected:
	typedef StaticFrame<DownloadsFrame> BaseType;
	friend class StaticFrame<DownloadsFrame>;
	friend class MDIChildFrame<DownloadsFrame>;

	DownloadsFrame(SmartWin::WidgetTabView* mdiParent);
	virtual ~DownloadsFrame();

	void layout();

	bool preClosing();
	void postClosing();

private:
	enum {
		COLUMN_FIRST,
		COLUMN_FILE = COLUMN_FIRST,
		COLUMN_PATH,
		COLUMN_STATUS,
		COLUMN_TIMELEFT,
		COLUMN_SPEED,
		COLUMN_DONE,
		COLUMN_SIZE,
		COLUMN_LAST
	};
	
	enum {
		SPEAKER_DISCONNECTED,
		SPEAKER_REMOVED,
		SPEAKER_TICK
	};
	
	struct TickInfo {
		TickInfo(const string& path_) : path(path_), done(0), bps(0), users(0) { }
		
		string path;
		int64_t done;
		double bps;
		int users;
	};

	class DownloadInfo {
	public:
		DownloadInfo(const string& filename, int64_t size, const TTHValue& tth);
		
		const tstring& getText(int col) const {
			return columns[col];
		}

		int getImage() const;

		static int compareItems(DownloadInfo* a, DownloadInfo* b, int col) {
			switch(col) {
			case COLUMN_STATUS: return compare(a->users, b->users);
			case COLUMN_TIMELEFT: return compare(a->timeleft(), b->timeleft());
			case COLUMN_SPEED: return compare(a->bps, b->bps);
			case COLUMN_SIZE: return compare(a->size, b->size);
			case COLUMN_DONE: return compare(a->done, b->done);
			default: return lstrcmpi(a->columns[col].c_str(), b->columns[col].c_str());
			}
		}
		
		void update();
		void update(const TickInfo& ti);
		
		int64_t timeleft() { return bps == 0 ? 0 : (size - done) / bps; }
		string path;
		int64_t done;
		int64_t size;
		double bps;
		int users;
		TTHValue tth;
		
		tstring columns[COLUMN_LAST];
	};
	
	typedef TypedListView<DownloadInfo> WidgetDownloads;
	typedef WidgetDownloads* WidgetDownloadsPtr;
	WidgetDownloadsPtr downloads;

	static int columnSizes[COLUMN_LAST];
	static int columnIndexes[COLUMN_LAST];
	
	bool startup;
	
	int find(const string& path);

	bool handleContextMenu(SmartWin::ScreenCoordinate pt);
	LRESULT handleSpeaker(WPARAM wParam, LPARAM lParam);
	
	virtual void on(DownloadManagerListener::Tick, const DownloadList&) throw();
	virtual void on(DownloadManagerListener::Complete, Download*) throw();
	virtual void on(DownloadManagerListener::Failed, Download*, const string&) throw();

	virtual void on(QueueManagerListener::Removed, QueueItem*) throw();
};

#endif 

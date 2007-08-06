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

#ifndef DCPLUSPLUS_WIN32_STATS_FRAME_H
#define DCPLUSPLUS_WIN32_STATS_FRAME_H

#include "StaticFrame.h"
#include "resource.h"

class StatsFrame : public StaticFrame<StatsFrame>
{
public:
	enum Stats {
		STATUS_STATUS,
		STATUS_LAST
	};
	static const ResourceManager::Strings TITLE_RESOURCE = ResourceManager::NETWORK_STATISTICS;
	static const unsigned ICON_RESOURCE = IDR_NET_STATS;
	
protected:
	void layout() {
		
	}
	
private:
	typedef StaticFrame<StatsFrame> BaseType;
	friend class StaticFrame<StatsFrame>;
	friend class MDIChildFrame<StatsFrame>;

	StatsFrame(SmartWin::WidgetMDIParent* mdiParent);

	virtual ~StatsFrame() { }

#ifdef PORT_ME
	typedef MDITabChildWindowImpl<StatsFrame> baseClass;
	BEGIN_MSG_MAP(StatsFrame)
		MESSAGE_HANDLER(WM_CREATE, onCreate)
		MESSAGE_HANDLER(WM_CLOSE, onClose)
		MESSAGE_HANDLER(WM_PAINT, onPaint)
		MESSAGE_HANDLER(WM_TIMER, onTimer)
		MESSAGE_HANDLER(WM_SIZE, onSize)
		CHAIN_MSG_MAP(baseClass)
	END_MSG_MAP()

	LRESULT onCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);

	void UpdateLayout(BOOL bResizeBars = TRUE);

private:
	// Pixels per second
	enum { PIX_PER_SEC = 2 };
	enum { LINE_HEIGHT = 10 };
	enum { AVG_SIZE = 5 };

	CBrush backgr;
	CPen upload;
	CPen download;
	CPen foregr;

	struct Stat {
		Stat() : scroll(0), speed(0) { }
		Stat(uint32_t aScroll, int64_t aSpeed) : scroll(aScroll), speed(aSpeed) { }
		uint32_t scroll;
		int64_t speed;
	};
	typedef deque<Stat> StatList;
	typedef StatList::iterator StatIter;
	typedef deque<int64_t> AvgList;
	typedef AvgList::iterator AvgIter;

	StatList up;
	AvgList upAvg;
	StatList down;
	AvgList downAvg;

	int width;
	int height;
	UINT_PTR timerId;
	int twidth;

	uint32_t lastTick;
	uint32_t scrollTick;
	int64_t lastUp;
	int64_t lastDown;

	int64_t max;

	void drawLine(CDC& dc, StatIter begin, StatIter end, CRect& rc, CRect& crc);
	void addTick(int64_t bdiff, int64_t tdiff, StatList& lst, AvgList& avg, int scroll);
#endif
	
};

#endif

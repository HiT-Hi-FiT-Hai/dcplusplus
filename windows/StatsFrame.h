/* 
 * Copyright (C) 2001-2003 Jacek Sieka, j_s@telia.com
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

#if !defined(AFX_StatsFRAME_H__8F6D05EC_ADCF_4987_8881_6DF3C0E355FA__INCLUDED_)
#define AFX_StatsFRAME_H__8F6D05EC_ADCF_4987_8881_6DF3C0E355FA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../client/TimerManager.h"

#include "FlatTabCtrl.h"
#include "WinUtil.h"

class StatsFrame : public MDITabChildWindowImpl<StatsFrame>, public StaticFrame<StatsFrame, ResourceManager::NETWORK_STATISTICS>
{
public:
	StatsFrame() : width(0), height(0), timerId(0), twidth(0), lastTick(GET_TICK()), scrollTick(0),
		lastUp(Socket::getTotalUp()), lastDown(Socket::getTotalDown()), max(0) 
	{ 
		black.CreateSolidBrush(RGB(0, 0, 0));
		upload.CreatePen(PS_SOLID, 0, SETTING(UPLOAD_BAR_COLOR));
		download.CreatePen(PS_SOLID, 0, SETTING(DOWNLOAD_BAR_COLOR));
		grey.CreatePen(PS_SOLID, 0, RGB(127, 127, 127));
	}

	~StatsFrame() { 
	}

	static CFrameWndClassInfo& GetWndClassInfo() { 
		static CFrameWndClassInfo wc = { 
			{	
				sizeof(WNDCLASSEX), 0, StartWindowProc, 
				0, 0, NULL, NULL, NULL, NULL, NULL, "StatsFrame", NULL 
			},
			NULL, NULL, IDC_ARROW, TRUE, 0, _T(""), IDR_NET_STATS 
		};
		
		return wc; 
	}

	virtual void OnFinalMessage(HWND /*hWnd*/) {
		delete this;
	}

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

	CBrush black;
	CPen upload;
	CPen download;
	CPen grey;

	struct Stat {
		Stat() : scroll(0), speed(0) { };
		Stat(u_int32_t aScroll, int64_t aSpeed) : scroll(aScroll), speed(aSpeed) { };
		u_int32_t scroll;
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
	int timerId;
	int twidth;

	u_int32_t lastTick;
	u_int32_t scrollTick;
	int64_t lastUp;
	int64_t lastDown;

	int64_t max;

	void drawLine(CDC& dc, StatIter begin, StatIter end, CRect& rc, CRect& crc);
	void addTick(int64_t bdiff, int64_t tdiff, StatList& lst, AvgList& avg, int scroll);
};

#endif // !defined(AFX_StatsFRAME_H__8F6D05EC_ADCF_4987_8881_6DF3C0E355FA__INCLUDED_)

/**
 * @file
 * $Id: StatsFrame.h,v 1.5 2004/08/02 15:29:19 arnetheduck Exp $
 */

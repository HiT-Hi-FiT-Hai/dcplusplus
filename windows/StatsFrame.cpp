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

#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "Resource.h"

#include "../client/Socket.h"

#include "StatsFrame.h"
#include "WinUtil.h"

LRESULT StatsFrame::onCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	timerId = SetTimer(1, 1000);
	
	m_hMenu = WinUtil::mainMenu;

	bHandled = FALSE;
	return 1;
}

LRESULT StatsFrame::onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	if(timerId != 0)
		KillTimer(timerId);
	MDIDestroy(m_hWnd);
	return 0;
}

void StatsFrame::drawLine(CDC& dc, StatIter begin, StatIter end, CRect& rc, CRect& crc) {
	int x = crc.right;
	
	StatIter i;
	for(i = begin; i != end; ++i) {
		if((x - i->scroll) < rc.right)
			break;
		x -= i->scroll;
	}
	if(i != end) {
		int y = (max == 0) ? height : (int)((i->speed * height) / max);
		dc.MoveTo(x, height - y);
		x -= i->scroll;
		++i;

		for(; i != end; ++i) {
			y = (max == 0) ? height : (int)((i->speed * height) / max);
			dc.LineTo(x, height - y);
			if(x < rc.left)
				break;
			x -= i->scroll;
		}
	}
}

LRESULT StatsFrame::onPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	if(GetUpdateRect(NULL)) {
		CPaintDC dc(m_hWnd);
		CRect rc(dc.m_ps.rcPaint);
		dcdebug("Update: %d, %d, %d, %d\n", rc.left, rc.top, rc.right, rc.bottom);

		dc.SelectBrush(black);
		dc.BitBlt(rc.left, rc.top, rc.Width(), rc.Height(), NULL, 0, 0, PATCOPY);

		CRect clientRC;
		GetClientRect(clientRC);

/*		dc.SelectPen(grey);

		int x = gridX;

		while(x < rc.left) {
			x += GRID_SPAN;
		}
		while(x < rc.right) {
			dc.MoveTo(x, rc.top);
			dc.LineTo(x, rc.bottom);
			x += GRID_SPAN;
		}

		x = clientRC.bottom - GRID_SPAN;
		while(x > rc.bottom) {
			x -= GRID_SPAN;
		}

		while(x > rc.top) {
			dc.MoveTo(rc.left, x);
			dc.LineTo(rc.right, x);
			x -= GRID_SPAN;
		}
*/
		dc.SelectPen(red);
		drawLine(dc, up.begin(), up.end(), rc, clientRC);

		dc.SelectPen(green);
		drawLine(dc, down.begin(), down.end(), rc, clientRC);

	}
	return 0;
}

LRESULT StatsFrame::onTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	u_int32_t tick = GET_TICK();
	u_int32_t tdiff = tick - lastTick;
	if(tdiff == 0)
		return 0;

	int64_t d = Socket::getTotalDown();
	int64_t ddiff = d - lastDown;
	int64_t u = Socket::getTotalUp();
	int64_t udiff = u - lastUp;

	while((int)down.size() > ((width / PIX_PER_SEC) + 1) ) {
		down.pop_back();
	}
	while((int)up.size() > ((width / PIX_PER_SEC) + 1) ) {
		up.pop_back();
	}

	u_int32_t scrollms = (tdiff + scrollTick)*PIX_PER_SEC;
	u_int32_t scroll = scrollms / 1000;

	scrollTick = scrollms - (scroll * 1000);

	ScrollWindow(-((int)scroll), 0);
	gridX -= scroll;
	gridX %= GRID_SPAN;

	int64_t dspeed = ddiff * (int64_t)1000 / (int64_t)tdiff;
	int64_t uspeed = udiff * (int64_t)1000 / (int64_t)tdiff;
	// Weigh with previous value to get nicer lines...
	if(down.size() > 1) {
		dspeed = (dspeed + down.front().speed) / 2;
	}
	if(up.size() > 1) {
		uspeed = (uspeed + up.front().speed) / 2;
	}

	int64_t mspeed = dspeed > uspeed ? dspeed : uspeed; 

	down.push_front(Stat(scroll, dspeed));
	up.push_front(Stat(scroll, uspeed));

	if(mspeed > max) {
		max = mspeed;
		Invalidate();
	}
	lastTick = tick;
	lastUp = u;
	lastDown = d;
	return 0;
}

LRESULT StatsFrame::onSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	CRect rc;
	GetClientRect(rc);
	width = rc.Width();
	height = rc.Height() - 1;
	bHandled = FALSE;
	return 0;
}

void StatsFrame::UpdateLayout(BOOL /*bResizeBars*/ /* = TRUE */) {
	
}

/**
 * @file
 * $Id: StatsFrame.cpp,v 1.1 2003/10/08 21:59:29 arnetheduck Exp $
 */



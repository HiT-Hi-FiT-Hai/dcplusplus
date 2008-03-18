/*
 * Copyright (C) 2001-2008 Jacek Sieka, arnetheduck on gmail point com
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

#include "StatsFrame.h"

#include <dcpp/Socket.h>
#include <dcpp/TimerManager.h>

StatsFrame::StatsFrame(SmartWin::WidgetTabView* mdiParent) :
	BaseType(mdiParent, T_("Network Statistics"), IDR_NET_STATS),
	pen(new SmartWin::Pen(WinUtil::textColor)),
	upPen(new SmartWin::Pen(SETTING(UPLOAD_BAR_COLOR))),
	downPen(new SmartWin::Pen(SETTING(DOWNLOAD_BAR_COLOR))),
	width(0),
	height(0),
	twidth(0),
	lastTick(GET_TICK()),
	scrollTick(0),
	lastUp(Socket::getTotalUp()),
	lastDown(Socket::getTotalDown()),
	max(0)
{
	setFont(WinUtil::font);

	onPainting(std::tr1::bind(&StatsFrame::handlePaint, this, _1));

	initStatus();
	layout();

	createTimer(std::tr1::bind(&StatsFrame::eachSecond, this), 1000);
}

StatsFrame::~StatsFrame() {
}

void StatsFrame::handlePaint(SmartWin::PaintCanvas& canvas) {
	SmartWin::Rectangle rect = canvas.getPaintRect();

	if(rect.width() == 0 || rect.size.y == 0)
		return;

	{
		SmartWin::Canvas::Selector select(canvas, *WinUtil::bgBrush);
		::BitBlt(canvas.handle(), rect.x(), rect.y(), rect.width(), rect.height(), NULL, 0, 0, PATCOPY);
	}

	canvas.setTextColor(WinUtil::textColor);
	canvas.setBkColor(WinUtil::bgColor);
	canvas.selectFont(WinUtil::font);
	
	long fontHeight = getTextSize(_T("A")).y;
	int lines = height / (fontHeight * LINE_HEIGHT);
	int lheight = height / (lines+1);

	{
		SmartWin::Canvas::Selector select(canvas, *pen);
		for(int i = 0; i < lines; ++i) {
			int ypos = lheight * (i+1);
			if(ypos > fontHeight + 2) {
				canvas.moveTo(rect.left(), ypos);
				canvas.lineTo(rect.right(), ypos);
			}

			if(rect.x() <= twidth) {
				ypos -= fontHeight + 2;
				if(ypos < 0)
					ypos = 0;
				if(height == 0)
					height = 1;
				tstring txt = Text::toT(Util::formatBytes(max * (height-ypos) / height) + "/s");
				SmartWin::Point txtSize = getTextSize(txt);
				long tw = txtSize.x;
				if(tw + 2 > twidth)
					twidth = tw + 2;
				canvas.drawText(txt, SmartWin::Rectangle(SmartWin::Point(1, ypos), txtSize), DT_LEFT | DT_TOP | DT_SINGLELINE);
			}
		}

		if(rect.x() < twidth) {
			tstring txt = Text::toT(Util::formatBytes(max) + "/s");
			SmartWin::Point txtSize = getTextSize(txt);
			long tw = txtSize.x;
			if(tw + 2 > twidth)
				twidth = tw + 2;
			canvas.drawText(txt, SmartWin::Rectangle(SmartWin::Point(1, 1), txtSize), DT_LEFT | DT_TOP | DT_SINGLELINE);
		}

	}

	long clientRight = getClientAreaSize().x;

	{
		SmartWin::Canvas::Selector select(canvas, *upPen);
		drawLine(canvas, up.begin(), up.end(), rect, clientRight);
	}

	{
		SmartWin::Canvas::Selector select(canvas, *downPen);
		drawLine(canvas, down.begin(), down.end(), rect, clientRight);
	}
}

void StatsFrame::layout() {
	SmartWin::Rectangle r(getClientAreaSize());

	layoutStatus(r);

	width = r.width();
	height = r.size.y - 1;

	invalidateWidget();
}

bool StatsFrame::eachSecond() {
	uint32_t tick = GET_TICK();
	uint32_t tdiff = tick - lastTick;
	if(tdiff == 0)
		return true;

	uint32_t scrollms = (tdiff + scrollTick)*PIX_PER_SEC;
	uint32_t scroll = scrollms / 1000;

	if(scroll == 0)
		return true;

	scrollTick = scrollms - (scroll * 1000);

	SmartWin::Point clientSize = getClientAreaSize();
	RECT rect = { twidth, 0, clientSize.x, clientSize.y };
	::ScrollWindow(handle(), -((int)scroll), 0, &rect, &rect);

	int64_t d = Socket::getTotalDown();
	int64_t ddiff = d - lastDown;
	int64_t u = Socket::getTotalUp();
	int64_t udiff = u - lastUp;

	addTick(ddiff, tdiff, down, downAvg, scroll);
	addTick(udiff, tdiff, up, upAvg, scroll);

	int64_t mspeed = 0;
	StatIter i;
	for(i = down.begin(); i != down.end(); ++i) {
		if(mspeed < i->speed)
			mspeed = i->speed;
	}
	for(i = up.begin(); i != up.end(); ++i) {
		if(mspeed < i->speed)
			mspeed = i->speed;
	}
	if(mspeed > max || ((max * 3 / 4) > mspeed) ) {
		max = mspeed;
		invalidateWidget();
	}

	lastTick = tick;
	lastUp = u;
	lastDown = d;
	return true;
}

void StatsFrame::drawLine(SmartWin::Canvas& canvas, StatIter begin, StatIter end, SmartWin::Rectangle& rect, long clientRight) {
	StatIter i;
	for(i = begin; i != end; ++i) {
		if((clientRight - (long)i->scroll) < rect.right())
			break;
		clientRight -= i->scroll;
	}
	if(i != end) {
		int y = (max == 0) ? 0 : (int)((i->speed * height) / max);
		canvas.moveTo(clientRight, height - y);
		clientRight -= i->scroll;
		++i;

		for(; i != end && clientRight > twidth; ++i) {
			y = (max == 0) ? 0 : (int)((i->speed * height) / max);
			canvas.lineTo(clientRight, height - y);
			if(clientRight < rect.left())
				break;
			clientRight -= i->scroll;
		}
	}
}

void StatsFrame::addTick(int64_t bdiff, int64_t tdiff, StatList& lst, AvgList& avg, int scroll) {
	while((int)lst.size() > ((width / PIX_PER_SEC) + 1) ) {
		lst.pop_back();
	}
	while(avg.size() > AVG_SIZE ) {
		avg.pop_back();
	}
	int64_t bspeed = bdiff * (int64_t)1000 / tdiff;
	avg.push_front(bspeed);

	bspeed = 0;

	for(AvgIter ai = avg.begin(); ai != avg.end(); ++ai) {
		bspeed += *ai;
	}

	bspeed /= avg.size();
	lst.push_front(Stat(scroll, bspeed));
}

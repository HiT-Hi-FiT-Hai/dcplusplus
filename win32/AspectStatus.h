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

#ifndef DCPLUSPLUS_WIN32_ASPECTSTATUS_H_
#define DCPLUSPLUS_WIN32_ASPECTSTATUS_H_

#include <dwt/widgets/StatusBar.h>
#include "WinUtil.h"

template<class WidgetType>
class AspectStatus {
	typedef AspectStatus<WidgetType> ThisType;
protected:

	AspectStatus() : status(0) {
		statusSizes.resize(WidgetType::STATUS_LAST);
		filterIter = dwt::Application::instance().addFilter(std::tr1::bind(&ThisType::filter, this, _1));
	}

	~AspectStatus() {
		dwt::Application::instance().removeFilter(filterIter);
	}

	void initStatus(bool sizeGrip = false) {
		dwt::StatusBar::Seed cs(sizeGrip);
		cs.font = WinUtil::font;
		status = static_cast<WidgetType*>(this)->addChild(cs);

		statusTip = static_cast<WidgetType*>(this)->addChild(dwt::ToolTip::Seed());
		statusTip->setTool(status, std::tr1::bind(&ThisType::handleToolTip, this));
	}
	
	void setStatus(int s, const tstring& text) {
		if(s != WidgetType::STATUS_STATUS) {
			int w = status->getTextSize(text).x + 12;
			if(w > static_cast<int>(statusSizes[s])) {
				dcdebug("Setting status size %d to %d\n", s, w);
				statusSizes[s] = w;
				layoutSections(status->getSize());
			}
		} else {
			lastLines.push_back(text);
			while(lastLines.size() > MAX_LINES) {
				lastLines.erase(lastLines.begin());
			}
		}
		status->setText(text, s);
	}
	
	void layoutStatus(dwt::Rectangle& r) {
		status->refresh();

		dwt::Point sz(status->getSize());
		r.size.y -= sz.y;
		layoutSections(sz);
	}
	
	void layoutSections(const dwt::Point& sz) {
		statusSizes[WidgetType::STATUS_STATUS] = 0;
		statusSizes[WidgetType::STATUS_STATUS] = sz.x - std::accumulate(statusSizes.begin(), statusSizes.end(), 0); 

		status->setSections(statusSizes);
		statusTip->setMaxTipWidth(statusSizes[WidgetType::STATUS_STATUS]);
	}
	
	void mapWidget(int s, dwt::Widget* widget) {
		POINT p[2];
		::SendMessage(status->handle(), SB_GETRECT, s, reinterpret_cast<LPARAM>(p));
		::MapWindowPoints(status->handle(), static_cast<WidgetType*>(this)->handle(), (POINT*)p, 2);
		::MoveWindow(widget->handle(), p[0].x, p[0].y, p[1].x - p[0].x, p[1].y - p[0].y, TRUE);
	}
	
	dwt::StatusBarPtr status;

	std::vector<unsigned> statusSizes;

private:
	dwt::Application::FilterIter filterIter;
	dwt::ToolTipPtr statusTip;
	TStringList lastLines;
	tstring tip;
	
	enum { MAX_LINES = 10 };

	bool filter(const MSG& msg) {
		statusTip->relayEvent(msg);
		return false;
	}

	const tstring& handleToolTip() {
		tip.clear();
		for(size_t i = 0; i < lastLines.size(); ++i) {
			if(i > 0) {
				tip += _T("\r\n");
			}
			tip += lastLines[i];
		}
		return tip;
	}
};

#endif /*ASPECTSTATUS_H_*/

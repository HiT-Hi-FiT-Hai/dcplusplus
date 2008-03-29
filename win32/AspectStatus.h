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

template<class WidgetType>
class AspectStatus {
	typedef AspectStatus<WidgetType> ThisType;
protected:
	typedef SmartWin::StatusBar<SmartWin::Section>::ThisType StatusBarSections;
	typedef StatusBarSections::ObjectType StatusBarSectionsPtr;

	AspectStatus() : status(0) {
		statusSizes.resize(WidgetType::STATUS_LAST);
		filterIter = SmartWin::Application::instance().addFilter(std::tr1::bind(&ThisType::filter, this, _1));
	}

	~AspectStatus() {
		SmartWin::Application::instance().removeFilter(filterIter);
	}

	void initStatus(bool sizeGrip = false) {
		StatusBarSections::Seed cs;
		cs.style = WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
		if(sizeGrip) {
			cs.style |= SBARS_SIZEGRIP;
		}
		status = static_cast<WidgetType*>(this)->createStatusBarSections(cs);

		statusTip = static_cast<WidgetType*>(this)->createToolTip();
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
	
	void layoutStatus(SmartWin::Rectangle& r) {
		status->refresh();

		SmartWin::Point sz(status->getSize());
		r.size.y -= sz.y;
		layoutSections(sz);
	}
	
	void layoutSections(const SmartWin::Point& sz) {
		statusSizes[WidgetType::STATUS_STATUS] = 0;
		statusSizes[WidgetType::STATUS_STATUS] = sz.x - std::accumulate(statusSizes.begin(), statusSizes.end(), 0); 

		status->setSections(statusSizes);
		statusTip->setMaxTipWidth(statusSizes[WidgetType::STATUS_STATUS]);
	}
	
	void mapWidget(int s, SmartWin::Widget* widget) {
		POINT p[2];
		::SendMessage(status->handle(), SB_GETRECT, s, reinterpret_cast<LPARAM>(p));
		::MapWindowPoints(status->handle(), static_cast<WidgetType*>(this)->handle(), (POINT*)p, 2);
		::MoveWindow(widget->handle(), p[0].x, p[0].y, p[1].x - p[0].x, p[1].y - p[0].y, TRUE);
	}
	
	StatusBarSectionsPtr status;

	std::vector<unsigned> statusSizes;

private:
	SmartWin::Application::FilterIter filterIter;
	typename SmartWin::ToolTip::ObjectType statusTip;
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

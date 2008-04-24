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

#include <boost/lambda/lambda.hpp>
#include <dwt/widgets/StatusBar.h>
#include "WinUtil.h"

template<class WidgetType>
class AspectStatus {
	typedef AspectStatus<WidgetType> ThisType;

	WidgetType& W() { return *static_cast<WidgetType*>(this); }
	const WidgetType& W() const { return *static_cast<const WidgetType*>(this); }

	HWND H() const { return W().handle(); }

protected:
	AspectStatus() : status(0), tip(0) {
		statusSizes.resize(WidgetType::STATUS_LAST);
		filterIter = dwt::Application::instance().addFilter(std::tr1::bind(&ThisType::filter, this, _1));
	}

	~AspectStatus() {
		dwt::Application::instance().removeFilter(filterIter);
	}

	void initStatus(bool sizeGrip = false) {
		dwt::StatusBar::Seed cs(sizeGrip);
		cs.font = WinUtil::font;
		status = W().addChild(cs);
		status->onHelp(std::tr1::bind(&ThisType::handleHelp, this, _1, _2));

		tip = W().addChild(dwt::ToolTip::Seed());
		tip->setTool(status, std::tr1::bind(&ThisType::handleToolTip, this, _1));
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

	void setStatusHelpId(int s, unsigned id) {
		helpIds[s] = id;
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
	}

	template<typename A>
	void mapWidget(int s, dwt::AspectSizable<A>* widget) {
		POINT p[2];
		status->sendMessage(SB_GETRECT, s, reinterpret_cast<LPARAM>(p));
		::MapWindowPoints(status->handle(), H(), (POINT*)p, 2);
		widget->setBounds(p[0].x, p[0].y, p[1].x - p[0].x, p[1].y - p[0].y);
	}
	
	dwt::StatusBarPtr status;

	std::vector<unsigned> statusSizes;

private:
	dwt::Application::FilterIter filterIter;
	dwt::ToolTipPtr tip;
	TStringList lastLines;

	enum { MAX_LINES = 10 };

	typedef std::tr1::unordered_map<int, unsigned> HelpIdsMap;
	HelpIdsMap helpIds;

	bool filter(const MSG& msg) {
		tip->relayEvent(msg);
		return false;
	}

	void handleHelp(HWND hWnd, unsigned id) {
		if(!dwt::AspectKeyboardBase::isKeyPressed(VK_F1)) {
			// we have the help id of the whole status bar; convert to the one of the specific part the user just clicked on
			dwt::Point pt = dwt::Point::fromLParam(::GetMessagePos());
			RECT rect = status->getBounds(false);
			if(::PtInRect(&rect, pt)) {
				unsigned x = dwt::ClientCoordinate(dwt::ScreenCoordinate(pt), status).x();
				unsigned total = 0;
				boost::lambda::var_type<unsigned>::type v(boost::lambda::var(total));
				HelpIdsMap::const_iterator i = helpIds.find(find_if(statusSizes.begin(), statusSizes.end(), (v += boost::lambda::_1, v > x)) - statusSizes.begin());
				if(i != helpIds.end())
					id = i->second;
			}
		}
		WinUtil::help(hWnd, id);
	}

	void handleToolTip(tstring& text) {
		tip->setMaxTipWidth(statusSizes[WidgetType::STATUS_STATUS]);
		text.clear();
		for(size_t i = 0; i < lastLines.size(); ++i) {
			if(i > 0) {
				text += _T("\r\n");
			}
			text += lastLines[i];
		}
	}
};

#endif /*ASPECTSTATUS_H_*/

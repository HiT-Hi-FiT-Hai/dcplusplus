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

#ifndef DCPLUSPLUS_WIN32_ASPECTSTATUS_H_
#define DCPLUSPLUS_WIN32_ASPECTSTATUS_H_

template<class WidgetType>
class AspectStatus {
	typedef AspectStatus<WidgetType> ThisType;
protected:
	AspectStatus() : status(0) {
		statusSizes.resize(WidgetType::STATUS_LAST);
	}

	void initStatus() {
		status = static_cast<WidgetType*>(this)->createStatusBarSections();
		statusTip = static_cast<WidgetType*>(this)->createToolTip();
		statusTip->setTool(status, std::tr1::bind(&ThisType::handleToolTip, this));
	}
	
	void setStatus(int s, const tstring& text) {
		if(s != WidgetType::STATUS_STATUS) {
			int w = status->getTextSize(text).x + 12;
			if(w > static_cast<int>(statusSizes[s])) {
				dcdebug("Setting status size %d to %d\n", s, w);
				statusSizes[s] = w;
				layoutStatus();
			}
		} else {
			lastLines.push_back(text);
			while(lastLines.size() > MAX_LINES) {
				lastLines.erase(lastLines.begin());
			}
		}
		status->setText(text, s);
	}
	
	SmartWin::Rectangle layoutStatus() {
		status->refresh();

		SmartWin::Rectangle rs(status->getClientAreaSize());

		{
			statusSizes[WidgetType::STATUS_STATUS] = 0;
			statusSizes[WidgetType::STATUS_STATUS] = rs.size.x - std::accumulate(statusSizes.begin(), statusSizes.end(), 0); 

			status->setSections(statusSizes);
		}
		return rs;
	}
	
	void mapWidget(int s, SmartWin::Widget* widget) {
		RECT sr;
		::SendMessage(status->handle(), SB_GETRECT, s, reinterpret_cast<LPARAM>(&sr));
		::MapWindowPoints(status->handle(), static_cast<WidgetType*>(this)->handle(), (POINT*)&sr, 2);
		::MoveWindow(widget->handle(), sr.left, sr.top, sr.right - sr.left, sr.bottom - sr.top, TRUE);
	}
	
	typename SmartWin::WidgetStatusBar< SmartWin::Section >::ObjectType status;

	std::vector<unsigned> statusSizes;

private:
	typename SmartWin::WidgetToolTip::ObjectType statusTip;
	TStringList lastLines;
	tstring tip;
	
	enum { MAX_LINES = 10 };
	
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

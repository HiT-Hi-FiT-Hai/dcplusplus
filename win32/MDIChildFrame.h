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

#ifndef DCPLUSPLUS_WIN32_MDI_CHILD_FRAME_H_
#define DCPLUSPLUS_WIN32_MDI_CHILD_FRAME_H_

#include "StupidWin.h"
#include "WinUtil.h"

#include "WidgetFactory.h"
#include "AspectSpeaker.h"
#include "AspectStatus.h"
#include <dcpp/SettingsManager.h>

template<typename T>
class MDIChildFrame : 
	public WidgetFactory< SmartWin::WidgetMDIChild, T >,
	public AspectSpeaker<T>, 
	public AspectStatus<T>
{
public:
	typedef MDIChildFrame<T> ThisType;
	
	MDIChildFrame(SmartWin::Widget* mdiClient) : SmartWin::Widget(mdiClient), reallyClose(false) {
		typename ThisType::Seed cs;
		cs.style |= BOOLSETTING(MDI_MAXIMIZED) ? WS_MAXIMIZE : 0;
		
		cs.background = (HBRUSH)(COLOR_3DFACE + 1);
		this->createMDIChild(cs);

		onClosing(std::tr1::bind(&ThisType::handleClosing, this));
		onFocus(std::tr1::bind(&ThisType::handleFocus, this));
		onSized(std::tr1::bind(&ThisType::sized, this, _1));
	}
	
protected:
	
	void handleFocus() {
		if(!controls.empty()) {
			::SetFocus(controls[0]->handle());
		}
	}
	/**
	 * The first of two close phases, used to disconnect from other threads that might affect this window.
	 * This is where all stuff that might be affected by other threads goes - it should make sure
	 * that no more messages can arrive from other threads
	 * @return True if close should be allowed, false otherwise
	 */
	bool preClosing() { return true; }
	/** Second close phase, perform any cleanup that depends only on the UI thread */
	void postClosing() { }
	
	template<typename W>
	void addWidget(W* widget) {
		widget->onKeyDown(std::tr1::bind((bool (T::*)(W*, int))&T::handleKeyDown, static_cast<T*>(this), widget, _1));
		widget->onChar(std::tr1::bind((bool (T::*)(W*, int))&T::handleChar, static_cast<T*>(this), widget, _1));
		addColor(widget);
		//TODO Fix widgets that don't support this... widget->onBackgroundColor(std::tr1::bind(&ThisType::handleBackgroundColor, this, _1));
		controls.push_back(widget); 
	}
	
	template<typename A, typename B, typename C>
	void addColor(SmartWin::AspectBackgroundColor<A, B, C>* widget) {
		dcdebug("Adding background color event for %s", typeid(*widget).name());
		widget->onBackgroundColor(std::tr1::bind(&ThisType::handleBackgroundColor, this, _1));
		
	}
	
	// Catch-rest for the above
	void addColor(void* w) {
		
	}
	
	template<typename W>
	bool handleKeyDown(W* widget, int key) { 
		if(key == VK_TAB && !widget->isControlPressed() && !widget->isAltPressed()) {
			for(WidgetList::size_type i = 0; i < controls.size(); ++i) {
				if(controls[i] == widget) {
					size_t pos = (widget->isShiftPressed() ? i + controls.size() - 1 : i + 1) % controls.size();
					::SetFocus(controls[pos]->handle());
					return true;
				}
			}
		}
		return false; 
	}
	
	template<typename W>
	bool handleChar(W* widget, int key) {
		if(key == VK_TAB && !widget->isControlPressed() && !widget->isAltPressed()) {
			if(std::find(controls.begin(), controls.end(), static_cast<SmartWin::Widget*>(widget)) != controls.end()) {
				// Eat tab chars so they're not passed to the control...
				return true;
			}
		}
		return false;
	}
	
private:
	/** This sets tab order and control coloring */
	typedef std::vector<SmartWin::Widget*> WidgetList;
	WidgetList controls;
	bool reallyClose;

	void sized(const SmartWin::WidgetSizedEventResult& sz) { 
		static_cast<T*>(this)->layout();
		SettingsManager::getInstance()->set(SettingsManager::MDI_MAXIMIZED, sz.isMaximized);
	}
	
	SmartWin::BrushPtr handleBackgroundColor(SmartWin::Canvas& canvas) {
		dcdebug("setting background\n");
		canvas.setBkMode(true);
		canvas.setBkColor(WinUtil::bgColor);
		canvas.setTextColor(WinUtil::textColor);
		return WinUtil::bgBrush;
	}
	
	bool handleClosing() {
		if(reallyClose) {
			static_cast<T*>(this)->postClosing();
			return true;
		} else if(static_cast<T*>(this)->preClosing()) {
			reallyClose = true;
			this->close(true);
			return false;
		}
		return false;
	}
};

#endif

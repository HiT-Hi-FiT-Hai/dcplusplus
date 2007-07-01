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

template<typename T>
class MDIChildFrame : 
	public WidgetFactory< SmartWin::WidgetMDIChild, T, SmartWin::MessageMapPolicyMDIChildWidget >,
	public AspectSpeaker<T>, 
	public AspectStatus<T, SmartWin::MessageMapPolicyMDIChildWidget>
{
public:
	typedef MDIChildFrame<T> ThisType;
	
	typedef SmartWin::WidgetFactory< SmartWin::WidgetMDIChild, T, SmartWin::MessageMapPolicyMDIChildWidget> FactoryType;
	MDIChildFrame() : SmartWin::Widget(0), reallyClose(false) {
		typename FactoryType::Seed cs;
		cs.background = (HBRUSH)(COLOR_3DFACE + 1);
		FactoryType::createMDIChild(cs);

		onClosing(std::tr1::bind(&ThisType::closing, this));
		onFocus(std::tr1::bind(&ThisType::focused, this));
		onSized(std::tr1::bind(&ThisType::sized, this, _1));
		onRaw(std::tr1::bind(&ThisType::ctlColor, this, _1, _2), WM_CTLCOLORSTATIC);
		onRaw(std::tr1::bind(&ThisType::ctlColor, this, _1, _2), WM_CTLCOLOREDIT);
	}
	
protected:
	
	void focused() {
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
		widget->onChar(&T::charred);
		controls.push_back(widget); 
	}
	
	template<typename W>
	bool charred(W* widget, int key) { 
		if(key == VK_TAB) {
			for(WidgetList::size_type i = 0; i < controls.size(); ++i) {
				if(controls[i] == widget) {
					::SetFocus(controls[(i+1) % controls.size()]->handle());
					return true;
				}
			}
		}
		return false; 
	}

	
private:
	/** This sets tab order and control coloring */
	typedef std::vector<SmartWin::Widget*> WidgetList;
	WidgetList controls;
	bool reallyClose;

	void sized(const SmartWin::WidgetSizedEventResult& sz) { static_cast<T*>(this)->layout(); }
	
	HRESULT ctlColor(LPARAM lp, WPARAM wp) {
		HWND hWnd((HWND)lp);
		HDC hDC((HDC)wp);
		T* t(static_cast<T*>(this));
		
		for(size_t i = 0; i < controls.size(); ++i) {
			if(hWnd == t->controls[i]->handle()) {
				::SetBkColor(hDC, WinUtil::bgColor);
				::SetTextColor(hDC, WinUtil::textColor);
				return (LRESULT)WinUtil::bgBrush;
			}
		}
		return 0;				
	}
	
	bool closing() {
		if(reallyClose) {
			static_cast<T*>(this)->postClosing();
			return true;
		} else if(static_cast<T*>(this)->preClosing()) {
			reallyClose = true;
			StupidWin::postMessage(this, WM_CLOSE, 0, 0);
			return false;
		}
		return false;
	}
};

#endif

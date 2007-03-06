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

template<typename T>
class MDIChildFrame : public SmartWin::WidgetFactory< SmartWin::WidgetMDIChild, T, SmartWin::MessageMapPolicyMDIChildWidget > {
public:
	typedef SmartWin::WidgetFactory< SmartWin::WidgetMDIChild, T, SmartWin::MessageMapPolicyMDIChildWidget> FactoryType;
	MDIChildFrame() : reallyClose(false) {
		// If something fails here, you've not called SmartWin::Widget with a good parent
		// (because of smartwin's shitty inheritance) from the most derived class 
		FactoryType::createMDIChild();

		onClosing(&T::closing);
		onFocus(&T::focused);
		onSized(&T::sized);
		onRaw(&T::x_spoken, WM_SPEAKER);
		onRaw(&T::ctlColor, WM_CTLCOLORSTATIC);
		onRaw(&T::ctlColor, WM_CTLCOLOREDIT);
	}
	
protected:
	BOOL speak(WPARAM w = 0, LPARAM l = 0) { return StupidWin::postMessage(this, WM_SPEAKER, w, l); }
	
	/** Override this to catch messages from speak */
	void spoken(WPARAM, LPARAM) { }
	
	/**
	 * The first of two close phases, used to disconnect from other threads that might affect this window
	 * @return True if close should be allowed, false otherwise
	 */
	bool preClosing() { return true; }
	/** Second close phase, perform any additional cleanup here if you need */
	void postClosing() { }
	
	/** Override this with any controls that you have that need coloring */
	HWND controls[0];
private:
	bool reallyClose;

	void sized(const SmartWin::WidgetSizedEventResult& sz) { static_cast<T*>(this)->layout(); }
	
	/// Swap silly smartwin order of arguments...
	HRESULT x_spoken(LPARAM lp, WPARAM wp) { return static_cast<T*>(this)->spoken(wp, lp), 0; }
	
	HRESULT ctlColor(LPARAM lp, WPARAM wp) {
		HWND hWnd((HWND)lp);
		HDC hDC((HDC)wp);
		T* t(static_cast<T*>(this));
		
		for(size_t i = 0; i < sizeof(t->controls)/sizeof(t->controls[0]); ++i) {
			if(hWnd == t->controls[i]) {
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

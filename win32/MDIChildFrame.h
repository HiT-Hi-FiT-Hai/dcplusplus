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

#include "WinUtil.h"

#include "WidgetFactory.h"
#include "AspectSpeaker.h"
#include "AspectStatus.h"
#include <dcpp/SettingsManager.h>
#include "MDITab.h"

template<typename T>
class MDIChildFrame : 
	public WidgetFactory< SmartWin::WidgetMDIChild >,
	public AspectSpeaker<T>, 
	public AspectStatus<T>
{
public:
	typedef MDIChildFrame<T> ThisType;
	
protected:

	MDIChildFrame(SmartWin::WidgetMDIParent* mdiClient, const tstring& title, SmartWin::IconPtr icon = SmartWin::IconPtr(), bool activate = true) : WidgetFactory< SmartWin::WidgetMDIChild >(mdiClient), lastFocus(NULL), reallyClose(false) {
		typename ThisType::Seed cs;
		BOOL max = FALSE;
		if(!mdiClient->sendMessage(WM_MDIGETACTIVE, 0, reinterpret_cast<LPARAM>(&max))) {
			max = BOOLSETTING(MDI_MAXIMIZED);
		}
		if(max)
			cs.style |= WS_MAXIMIZE;
		cs.style |= WS_CLIPCHILDREN;
		cs.caption = title;
		cs.background = (HBRUSH)(COLOR_3DFACE + 1);
		cs.activate = activate;
		cs.icon = icon;
		this->createMDIChild(cs);

		MDITab::getInstance()->addTab(this, icon);

		onClosing(std::tr1::bind(&ThisType::handleClosing, this));
		onFocus(std::tr1::bind(&ThisType::handleFocus, this));
		onSized(std::tr1::bind(&ThisType::handleSized, this, _1));
		onActivate(std::tr1::bind(&ThisType::handleActivate, this, _1));
	}
	
	virtual ~MDIChildFrame() {
		MDITab::getInstance()->removeTab(this);
	}
	
	void handleFocus() {
		if(lastFocus != NULL) {
			::SetFocus(lastFocus);
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
		addColor(widget);
		if(lastFocus == NULL) {
			lastFocus = widget->handle();
		}
	}
	
	void setDirty(SettingsManager::IntSetting setting) {
		if(SettingsManager::getInstance()->getBool(setting)) {
			MDITab::getInstance()->markTab(this);
		}
	}
	
	void onTabContextMenu(const std::tr1::function<bool (const SmartWin::Point&)>& f) {
		MDITab::getInstance()->onTabContextMenu(this, f);
	}
	
private:
	HWND lastFocus;
	bool reallyClose;

	template<typename A>
	void addColor(SmartWin::AspectBackgroundColor<A>* widget) {
		widget->onBackgroundColor(std::tr1::bind(&ThisType::handleBackgroundColor, this, _1));
	}
	
	// Catch-rest for the above
	void addColor(void* w) {
		
	}

	bool handleSized(const SmartWin::WidgetSizedEventResult& sz) { 
		static_cast<T*>(this)->layout();
		BOOL max = FALSE;
		if(this->getParent()->sendMessage(WM_MDIGETACTIVE, 0, reinterpret_cast<LPARAM>(&max))) {
			SettingsManager::getInstance()->set(SettingsManager::MDI_MAXIMIZED, max > 0);
		}
		return false;
	}
	
	void handleActivate(bool active) {
		if(active) {
			// clear dirty...
		} else {
			lastFocus = ::GetFocus();
		}
	}
	
	SmartWin::BrushPtr handleBackgroundColor(SmartWin::Canvas& canvas) {
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

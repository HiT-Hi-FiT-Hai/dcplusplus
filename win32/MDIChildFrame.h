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
#include <dcpp/ResourceManager.h>
#include "resource.h"

template<typename T>
class MDIChildFrame : 
	public WidgetFactory< SmartWin::WidgetChildWindow >,
	public AspectSpeaker<T>, 
	public AspectStatus<T>
{
public:
	typedef MDIChildFrame<T> ThisType;
	typedef WidgetFactory< SmartWin::WidgetChildWindow > BaseType;
protected:

	MDIChildFrame(SmartWin::WidgetTabView* tabView, const tstring& title, SmartWin::IconPtr icon = SmartWin::IconPtr(), bool activate = true) : 
		BaseType(tabView->getTab()), 
		lastFocus(NULL), 
		reallyClose(false) 
	{
		typename ThisType::Seed cs;
		cs.style |= WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
		if(activate) {
			cs.style |= WS_VISIBLE;
		} else {
			cs.style &= ~WS_VISIBLE;
		}
		cs.caption = title;
		cs.background = (HBRUSH)(COLOR_3DFACE + 1);
		cs.icon = icon;
		this->createWindow(cs);

		tabView->add(this, icon);
		this->onTabContextMenu(std::tr1::bind(&ThisType::handleContextMenu, this, _1));

		onClosing(std::tr1::bind(&ThisType::handleClosing, this));
		onFocus(std::tr1::bind(&ThisType::handleFocus, this));
		onSized(std::tr1::bind(&ThisType::handleSized, this, _1));
		onActivate(std::tr1::bind(&ThisType::handleActivate, this, _1));
		onCommand(std::tr1::bind(&ThisType::close, this, true), IDC_CLOSE_WINDOW);
	}
	
	virtual ~MDIChildFrame() {
		getParent()->remove(this);
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
			getParent()->mark(this);
		}
	}
	
	void onTabContextMenu(const std::tr1::function<bool (const SmartWin::ScreenCoordinate&)>& f) {
		getParent()->onTabContextMenu(this, f);
	}
	
	void activate() {
		getParent()->setActive(this);
	}
	
	SmartWin::WidgetTabView* getParent() {
		return static_cast<SmartWin::WidgetTabView*>(BaseType::getParent()->getParent());
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
	
	bool handleContextMenu(const SmartWin::ScreenCoordinate& pt) {
		SmartWin::WidgetMenu::ObjectType menu = SmartWin::WidgetCreator<SmartWin::WidgetMenu>::create(SmartWin::WidgetMenu::Seed(true));
		menu->appendItem(IDC_CLOSE_WINDOW, TSTRING(CLOSE), std::tr1::bind(&ThisType::close, this, true));
		
		menu->trackPopupMenu(this, pt, TPM_LEFTALIGN | TPM_RIGHTBUTTON);

		return true;
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

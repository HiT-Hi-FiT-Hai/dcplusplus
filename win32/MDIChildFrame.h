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

#ifndef DCPLUSPLUS_WIN32_MDI_CHILD_FRAME_H_
#define DCPLUSPLUS_WIN32_MDI_CHILD_FRAME_H_

#include "WinUtil.h"

#include "WidgetFactory.h"
#include "AspectSpeaker.h"
#include "AspectStatus.h"
#include <dcpp/SettingsManager.h>
#include "resource.h"

template<typename T>
class MDIChildFrame : 
	public WidgetFactory< dwt::Container >,
	public AspectSpeaker<T>, 
	public AspectStatus<T>
{
	typedef WidgetFactory< dwt::Container > BaseType;
public:
	typedef MDIChildFrame<T> ThisType;
protected:

	MDIChildFrame(dwt::TabView* tabView, const tstring& title, unsigned helpId = 0, unsigned resourceId = 0, bool activate = true) :
		BaseType(tabView),
		lastFocus(NULL),
		alwaysSameFocus(false),
		reallyClose(false)
	{
		dwt::IconPtr icon = resourceId ? dwt::IconPtr(new dwt::Icon(resourceId)) : dwt::IconPtr();

		typename ThisType::Seed cs;
		cs.style &= ~WS_VISIBLE;
		cs.caption = title;
		cs.background = (HBRUSH)(COLOR_3DFACE + 1);
		cs.icon = icon;
		cs.location = tabView->getClientSize();
		this->create(cs);

		if(helpId)
			setHelpId(helpId);

		tabView->add(this, icon);
		
		if(activate) {
			tabView->setActive(this);
		}

		this->onTabContextMenu(std::tr1::bind(&ThisType::handleContextMenu, this, _1));

		onClosing(std::tr1::bind(&ThisType::handleClosing, this));
		onFocus(std::tr1::bind(&ThisType::handleFocus, this));
		onSized(std::tr1::bind(&ThisType::handleSized, this, _1));
		onActivate(std::tr1::bind(&ThisType::handleActivate, this, _1));
		onCommand(std::tr1::bind(&ThisType::close, this, true), IDC_CLOSE_WINDOW);
		addDlgCodeMessage(this);
		
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
	void addWidget(W* widget, bool alwaysFocus = false, bool autoTab = true) {
		if(autoTab) {
			addDlgCodeMessage(widget);
		}
		
		addColor(widget);

		if(alwaysFocus || (lastFocus == NULL)) {
			lastFocus = widget->handle();
			if(this->getVisible())
				::SetFocus(lastFocus);
		}
		if(alwaysFocus)
			alwaysSameFocus = true;
	}
	
	void setDirty(SettingsManager::IntSetting setting) {
		if(SettingsManager::getInstance()->getBool(setting)) {
			getParent()->mark(this);
		}
	}
	
	void onTabContextMenu(const std::tr1::function<bool (const dwt::ScreenCoordinate&)>& f) {
		getParent()->onTabContextMenu(this, f);
	}
	
	void activate() {
		getParent()->setActive(this);
	}
	
	dwt::TabView* getParent() {
		return static_cast<dwt::TabView*>(BaseType::getParent());
	}

	void setIcon(unsigned resourceId) {
		getParent()->setTabIcon(this, dwt::IconPtr(new dwt::Icon(resourceId)));
	}

private:
	HWND lastFocus; // last focused widget
	bool alwaysSameFocus; // always focus the same widget

	bool reallyClose;

	void addDlgCodeMessage(ComboBox* widget) {
		widget->onRaw(std::tr1::bind(&ThisType::handleGetDlgCode, this, _1), dwt::Message(WM_GETDLGCODE));
		TextBox* text = widget->getTextBox();
		if(text)
			text->onRaw(std::tr1::bind(&ThisType::handleGetDlgCode, this, _1), dwt::Message(WM_GETDLGCODE));
	}

	template<typename W>
	void addDlgCodeMessage(W* widget) {
		widget->onRaw(std::tr1::bind(&ThisType::handleGetDlgCode, this, _1), dwt::Message(WM_GETDLGCODE));
	}

	void addColor(ComboBox* widget) {
		widget->setColor(WinUtil::textColor, WinUtil::bgColor);
		TextBox* text = widget->getTextBox();
		if(text)
			text->setColor(WinUtil::textColor, WinUtil::bgColor);
	}

	// don't handle WM_CTLCOLOR* for Buttons or Button-derived controls
	void addColor(dwt::Button* widget) {
		// empty on purpose
	}

	template<typename A>
	void addColor(dwt::AspectColor<A>* widget) {
		widget->setColor(WinUtil::textColor, WinUtil::bgColor);
	}
	
	// Catch-rest for the above
	void addColor(void* w) {
		
	}

	void handleSized(const dwt::SizedEvent& sz) { 
		static_cast<T*>(this)->layout();
	}
	
	void handleActivate(bool active) {
		if(active) {
			if(lastFocus) {
				::SetFocus(lastFocus);
			}
		} else if(!alwaysSameFocus) {
			HWND focus = ::GetFocus();
			if(focus != NULL && ::IsChild(static_cast<T*>(this)->handle(), focus))
				lastFocus = focus;
		}
	}

	LRESULT handleGetDlgCode(WPARAM wParam) {
		 if(wParam != VK_TAB)
			return DLGC_WANTMESSAGE;
		return 0;
	}

	bool handleContextMenu(const dwt::ScreenCoordinate& pt) {
		dwt::Menu::ObjectType menu = addChild(WinUtil::Seeds::menu);
		menu->setTitle(getParent()->getTabText(this));
		menu->appendItem(IDC_CLOSE_WINDOW, T_("&Close"), std::tr1::bind(&ThisType::close, this, true), dwt::BitmapPtr(new dwt::Bitmap(IDB_EXIT)));
		menu->trackPopupMenu(pt, TPM_LEFTALIGN | TPM_RIGHTBUTTON);
		return true;
	}
	
	bool handleClosing() {
		if(reallyClose) {
			static_cast<T*>(this)->postClosing();
			if(getParent()->getActive() == this) {
				// Prevent flicker by selecting the next tab - WM_DESTROY would already be too late
				getParent()->next();
			}
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

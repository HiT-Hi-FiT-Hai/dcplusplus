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

#include "stdafx.h"

#include "MDITab.h"

MDITab* MDITab::instance;
HHOOK MDITab::hook;

MDITab::MDITab(SmartWin::Widget* parent) : 
	BaseType(parent),
	inTab(false),
	mdi(0)
{
	instance = this;
	hook = ::SetWindowsHookEx(WH_KEYBOARD, &MDITab::keyboardProc, NULL, ::GetCurrentThreadId());
}

MDITab::~MDITab() {
	if(hook) {
		::UnhookWindowsHookEx(hook);
		hook = NULL;
	}
	instance = 0;
}

void MDITab::addTab(SmartWin::WidgetMDIChild* w, const SmartWin::IconPtr& icon) {
	if(!mdi) {
		mdi = w->getParent();
	}

	viewOrder.push_back(w->handle());
	
	int image = -1;
	if(icon) {
		for(size_t i = 0; i < icons.size(); ++i) {
			if(icon == icons[i]) {
				image = i;
				break;
			}
		}
		if(image == -1) {
			image = icons.size();
			icons.push_back(icon);
			getImageList()->add(*icon);
		}
	}
	
	size_t tabs = this->size();
	this->addPage(cutTitle(w->getText()), tabs, reinterpret_cast<LPARAM>(w), image);
	if(w->getParent()->getActive() == w->handle()) {
		this->setSelectedIndex(tabs);
	}
	w->onTextChanging(std::tr1::bind(&MDITab::handleTextChanging, this, w, _1));
	w->onRaw(std::tr1::bind(&MDITab::handleMdiActivate, this, w, _1, _2), SmartWin::Message(WM_MDIACTIVATE));
	w->onSysCommand(std::tr1::bind(&MDITab::handleNext, this, false), SC_NEXTWINDOW);
	w->onSysCommand(std::tr1::bind(&MDITab::handleNext, this, true), SC_PREVWINDOW);

	layout();
}

void MDITab::removeTab(SmartWin::WidgetMDIChild* w) {
	
	viewOrder.remove(w->handle());

	int i = findTab(w);
	if(i != -1) {
		erase(i);
		layout();
	}
}

void MDITab::markTab(SmartWin::WidgetMDIChild* w) {
	int i = findTab(w);
	if(i != -1) {
		setHighlight(i, true);
	}
}

int MDITab::findTab(SmartWin::WidgetMDIChild* w) {
	for(size_t i = 0; i < this->size(); ++i) {
		if(getData(i) == reinterpret_cast<LPARAM>(w)) {
			return static_cast<int>(i);
		}
	}
	return -1;
}

void MDITab::create( const Seed & cs ) {
	BaseType::create(cs);
	
	setImageList(SmartWin::ImageListPtr(new SmartWin::ImageList(16, 16, ILC_COLOR32 | ILC_MASK)));
//	getImageList()->setBkColor(::GetSysColor(COLOR_3DFACE));
	onSelectionChanged(std::tr1::bind(&MDITab::handleSelectionChanged, this, _1));
}

bool MDITab::handleTextChanging(SmartWin::WidgetMDIChild* w, const SmartUtil::tstring& newText) {
	int i = findTab(w);
	if(i != -1) {
		this->setHeader(i, cutTitle(newText));
		layout();
	}
	return true;
}

void MDITab::handleSelectionChanged(size_t i) {
	SmartWin::WidgetMDIChild* w = reinterpret_cast<SmartWin::WidgetMDIChild*>(this->getData(i));
	if(w->getParent()->getActive() != w->handle()) {
		w->activate();
	}
	setHighlight(i, false);
}

LRESULT MDITab::handleMdiActivate(SmartWin::WidgetMDIChild* w, WPARAM wParam, LPARAM lParam) {
	if(reinterpret_cast<LPARAM>(w->handle()) == lParam) {
		int i = findTab(w);
		if(i != -1) {
			setSelectedIndex(i);
		}
	}
	
	if(!inTab) {
		setTop(w->handle());
	}
	return 0;
}

void MDITab::handleNext(bool reverse) {
	if(viewOrder.size() < 2) {
		dcdebug("No view order windows, skipping switch");
		return;
	}
	HWND wnd = mdi->getActive();
	if(!wnd) {
		dcdebug("No active window, skipping switch\n");
		return;
	}
	
	WindowIter i;
	if(inTab) {
		i = std::find(viewOrder.begin(), viewOrder.end(), wnd);
		if(i == viewOrder.end()) {
			dcdebug("Could not find mdi child window, skipping switch\n");
			return;
		}
		
		if(!reverse) {
			if(i == viewOrder.begin()) {
				i = viewOrder.end();
			}
			--i;
		} else {
			if(++i == viewOrder.end()) {
				i = viewOrder.begin();
			}
		}
	} else {
		if(!reverse) {
			i = --(--viewOrder.end());
		} else {
			i = ++viewOrder.begin();
		}
	}
	
	mdi->sendMessage(WM_MDIACTIVATE, reinterpret_cast<WPARAM>(*i));
	return;
}

void MDITab::setTop(HWND wnd) {
	WindowIter i = std::find(viewOrder.begin(), viewOrder.end(), wnd);
	if(i != viewOrder.end()) {
		viewOrder.erase(i);
		viewOrder.push_back(wnd);
	}
}

tstring MDITab::cutTitle(const tstring& title) {
	if(title.length() > MAX_TITLE_LENGTH) {
		return title.substr(0, MAX_TITLE_LENGTH - 3) + _T("...");
	}
	return title;
}

LRESULT CALLBACK MDITab::keyboardProc(int code, WPARAM wParam, LPARAM lParam) {
	if(instance && instance->mdi && code == HC_ACTION) {
		if(wParam == VK_CONTROL && LOWORD(lParam) == 1) {
			if(lParam & 0x80000000) {
				instance->inTab = false;
				instance->setTop(instance->mdi->getActive());
			} else {
				instance->inTab = true;
			}
		}
	}
	return CallNextHookEx(hook, code, wParam, lParam);
}

void MDITab::layout() {
	if(!resized)
		return;
	
	SmartWin::Rectangle tmp = this->getUsableArea();
	if(!(tmp == clientSize)) {
		clientSize = tmp;
		resized(clientSize);
	}
}

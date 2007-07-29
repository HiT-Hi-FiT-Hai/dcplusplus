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

MDITab::MDITab(SmartWin::Widget* parent) : 
	BaseType(parent),
	activating(false),
	mdi(0)
{
	instance = this;
}

MDITab::~MDITab() {
	instance = 0;
}

void MDITab::addTab(SmartWin::WidgetMDIChild::ObjectType w) {
	if(!mdi) {
		mdi = w->getParent();
		//mdi->onRaw(std::tr1::bind(&MDITab::handleMdiNext, this, _1, _2), SmartWin::Message(WM_MDINEXT));
	}

	viewOrder.push_back(w);
	nextTab = --viewOrder.end();

	size_t tabs = this->size();
	this->addPage(cutTitle(w->getText()), tabs, reinterpret_cast<LPARAM>(static_cast<SmartWin::Widget*>(w)));

	if(w->getParent()->getActive() == w->handle()) {
		activating = true;
		this->setSelectedIndex(tabs);
	}
	w->onTextChanging(std::tr1::bind(&MDITab::handleTextChanging, this, w, _1));
	w->onRaw(std::tr1::bind(&MDITab::handleMdiActivate, this, w, _1, _2), SmartWin::Message(WM_MDIACTIVATE));

	if(resized)
		resized();
}

void MDITab::removeTab(SmartWin::Widget* w) {
	
	viewOrder.remove(w);
	nextTab = viewOrder.end();
	if(!viewOrder.empty())
		--nextTab;

	int i = findTab(w);
	if(i != -1) {
		erase(i);
		if(resized)
			resized();
	}
}

int MDITab::findTab(SmartWin::Widget* w) {
	for(size_t i = 0; i < this->size(); ++i) {
		if(getData(i) == reinterpret_cast<LPARAM>(w)) {
			return static_cast<int>(i);
		}
	}
	return -1;
}

void MDITab::create( const Seed & cs ) {
	BaseType::create(cs);
	
	onSelectionChanged(std::tr1::bind(&MDITab::handleSelectionChanged, this, _1));
}

bool MDITab::handleTextChanging(SmartWin::Widget* w, const SmartUtil::tstring& newText) {
	int i = findTab(w);
	if(i != -1) {
		this->setHeader(i, cutTitle(newText));
	}
	return true;
}

void MDITab::handleSelectionChanged(size_t i) {
	SmartWin::Widget* w = reinterpret_cast<SmartWin::Widget*>(this->getData(i));
	if(w->getParent()->sendMessage(WM_MDIGETACTIVE) != reinterpret_cast<HRESULT>(w->handle())) {
		w->getParent()->sendMessage(WM_MDIACTIVATE, reinterpret_cast<WPARAM>(w->handle()));
	}
}

LRESULT MDITab::handleMdiActivate(SmartWin::Widget* w, WPARAM wParam, LPARAM lParam) {
	if(reinterpret_cast<LPARAM>(w->handle()) == lParam) {
		int i = findTab(w);
		if(i != -1) {
			setSelectedIndex(i);
		}
	}
	return 0;
}

LRESULT MDITab::handleMdiNext(WPARAM wParam, LPARAM lParam) {
	
}

tstring MDITab::cutTitle(const tstring& title) {
	if(title.length() > MAX_TITLE_LENGTH) {
		return title.substr(0, MAX_TITLE_LENGTH - 3) + _T("...");
	}
	return title;
}

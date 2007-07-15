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
#include <dcpp/DCPlusPlus.h>

#include "MDITab.h"

MDITab* MDITab::instance;

MDITab::MDITab(SmartWin::Widget* parent) : 
	SmartWin::Widget(parent),
	BaseType(parent)
{
	instance = this;
}

MDITab::~MDITab() {
	instance = 0;
}

void MDITab::removeTab(SmartWin::Widget* w) {
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
		this->setHeader(i, newText);
	}
	return true;
}

void MDITab::handleSelectionChanged(size_t i) {
	SmartWin::Widget* w = reinterpret_cast<SmartWin::Widget*>(this->getData(i));
	if(w->getParent()->sendMessage(WM_MDIGETACTIVE) != reinterpret_cast<HRESULT>(w->handle())) {
		w->getParent()->sendMessage(WM_MDIACTIVATE, reinterpret_cast<WPARAM>(w->handle()));
	}
}

HRESULT MDITab::handleMdiActivate(SmartWin::Widget* w, WPARAM wParam, LPARAM lParam) {
	if(reinterpret_cast<LPARAM>(w->handle()) == lParam) {
		int i = findTab(w);
		if(i != -1) {
			setSelectedIndex(i);
		}
	}
	return 0;
}

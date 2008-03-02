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

#include "stdafx.h"

#include "WidgetTextBox.h"

#include "WinUtil.h"

WidgetTextBox::WidgetTextBox( SmartWin::Widget * parent ) : BaseType(parent), menuOpened(false) {
	this->onLeftMouseDblClick(std::tr1::bind(&WidgetTextBox::handleLeftDblClick, this, _1));

	this->onRaw(std::tr1::bind(&WidgetTextBox::handleEnterIdle, this, _1, _2), SmartWin::Message(WM_ENTERIDLE));
	this->onRaw(std::tr1::bind(&WidgetTextBox::handleMenuSelect, this, _1, _2), SmartWin::Message(WM_MENUSELECT));
}

void WidgetTextBox::handleLeftDblClick(const SmartWin::MouseEventResult& ev) {
	WinUtil::parseDBLClick(textUnderCursor(ev.pos));
}

LRESULT WidgetTextBox::handleEnterIdle(WPARAM wParam, LPARAM lParam) {
	if(wParam == MSGF_MENU && !menuOpened) {
		menu = SmartWin::WidgetCreator<SmartWin::WidgetMenu>::attach(this, reinterpret_cast<HMENU>(::SendMessage(reinterpret_cast<HWND>(lParam), MN_GETHMENU, 0, 0)), WinUtil::Seeds::menu);
		menuOpened = true;
	}
	return 0;
}

LRESULT WidgetTextBox::handleMenuSelect(WPARAM wParam, LPARAM lParam) {
	if((HIWORD(wParam) == 0xFFFF) && (lParam == 0))
		menuOpened = false;
	return 0;
}

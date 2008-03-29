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

#include "TextBox.h"

#include "WinUtil.h"

TextBox::TextBox( SmartWin::Widget * parent ) : BaseType(parent), menuOpened(false) {
	this->onLeftMouseDblClick(std::tr1::bind(&TextBox::handleLeftDblClick, this, _1));

	/*
	* unlike usual controls, the edit control doesn't send WM_INITMENUPOPUP when its standard
	* menu is being opened. however, we can catch WM_ENTERIDLE and sub-class the menu then.
	*
	* method described by Jeff Partch in http://groups.google.com/group/microsoft.public.vc.mfc/msg/5e07dc60be3d3baa
	*/
	this->onRaw(std::tr1::bind(&TextBox::handleEnterIdle, this, _1, _2), SmartWin::Message(WM_ENTERIDLE));
	this->onRaw(std::tr1::bind(&TextBox::handleMenuSelect, this, _1, _2), SmartWin::Message(WM_MENUSELECT));
}

void TextBox::handleLeftDblClick(const SmartWin::MouseEventResult& ev) {
	WinUtil::parseDBLClick(textUnderCursor(ev.pos));
}

LRESULT TextBox::handleEnterIdle(WPARAM wParam, LPARAM lParam) {
	if(wParam == MSGF_MENU && !menuOpened) {
		GUITHREADINFO gti = { sizeof(gti) };
		if(::GetGUIThreadInfo(NULL, &gti) && (gti.flags & GUI_POPUPMENUMODE) && (gti.hwndMenuOwner == handle())) {
			HMENU hMenu = reinterpret_cast<HMENU>(::SendMessage(reinterpret_cast<HWND>(lParam), MN_GETHMENU, 0, 0));
			if(!hMenu)
				return 0;

			menuOpened = true;

			{
				// make sure we're not sub-classing the scrollbar context menu...
				DWORD messagePos = ::GetMessagePos();
				POINT pt = { GET_X_LPARAM(messagePos), GET_Y_LPARAM(messagePos) };
				SCROLLBARINFO sbi = { sizeof(sbi) };
				if(::GetScrollBarInfo(handle(), OBJID_HSCROLL, &sbi) && ::PtInRect(&sbi.rcScrollBar, pt))
					return 0;
				if(::GetScrollBarInfo(handle(), OBJID_VSCROLL, &sbi) && ::PtInRect(&sbi.rcScrollBar, pt))
					return 0;
			}

			menu = SmartWin::WidgetCreator<SmartWin::WidgetMenu>::attach(this, hMenu, WinUtil::Seeds::menu);
		}
	}
	return 0;
}

LRESULT TextBox::handleMenuSelect(WPARAM wParam, LPARAM lParam) {
	if((HIWORD(wParam) == 0xFFFF) && (lParam == 0))
		menuOpened = false;
	return 0;
}

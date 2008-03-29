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

#include "ComboBox.h"

ComboBox::ComboBox( SmartWin::Widget * parent ) : BaseType(parent), textBox(0) {
}

ComboBox::TextBoxPtr ComboBox::getTextBox() {
	if(!textBox) {
		LONG_PTR style = ::GetWindowLongPtr(handle(), GWL_STYLE);
		if((style & CBS_SIMPLE)  == CBS_SIMPLE || (style & CBS_DROPDOWN) == CBS_DROPDOWN) {
			HWND wnd = ::FindWindowEx(handle(), NULL, _T("EDIT"), NULL);
			if(wnd && wnd != handle())
				textBox = SmartWin::WidgetCreator< TextBox >::attach(this, wnd);
		}
	}
	return textBox;
}

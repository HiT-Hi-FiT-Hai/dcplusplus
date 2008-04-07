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

#ifndef DCPLUSPLUS_WIN32_TextBox_H_
#define DCPLUSPLUS_WIN32_TextBox_H_

/** Our own flavour of text boxes that handle double clicks and have fancy menus */
class TextBox : public SmartWin::TextBox {
	typedef SmartWin::TextBox BaseType;

public:
	typedef TextBox ThisType;
	
	typedef ThisType* ObjectType;

	struct Seed : public BaseType::Seed {
		typedef ThisType WidgetType;

		Seed(const SmartUtil::tstring& caption = SmartUtil::tstring());
	};

	explicit TextBox( SmartWin::Widget * parent );

private:
	void handleLeftDblClick(const SmartWin::MouseEventResult& ev);

	LRESULT handleEnterIdle(WPARAM wParam, LPARAM lParam);
	LRESULT handleMenuSelect(WPARAM wParam, LPARAM lParam);

	SmartWin::WidgetMenu::ObjectType menu;
	bool menuOpened;
};

#endif /*TextBox_H_*/
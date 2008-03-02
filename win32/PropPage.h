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

#ifndef DCPLUSPLUS_WIN32_PROP_PAGE_H
#define DCPLUSPLUS_WIN32_PROP_PAGE_H

#define SETTINGS_BUF_LEN 1024

#include "WidgetFactory.h"

class PropPage : public WidgetFactory<SmartWin::WidgetDialog>
{
public:
	PropPage(SmartWin::Widget* parent);
	virtual ~PropPage();

	virtual void write() = 0;
	virtual int getHelpId() = 0;

	enum Type { T_STR, T_INT, T_BOOL, T_CUSTOM, T_END };

	struct Item
	{
		WORD itemID;
		int setting;
		Type type;
	};
	struct ListItem {
		int setting;
		const char* desc;
	};
	struct TextItem {
		WORD itemID;
		const char* translatedString;
	};

protected:

	void read(HWND page, Item const* items, ListItem* listItems = NULL, HWND list = NULL);
	void write(HWND page, Item const* items, ListItem* listItems = NULL, HWND list = NULL);
	void translate(HWND page, TextItem* textItems);
	
	LRESULT handleHelp(WPARAM wParam, LPARAM lParam);
};

#endif // !defined(PROP_PAGE_H)

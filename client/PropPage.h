/* 
 * Copyright (C) 2001 Jacek Sieka, j_s@telia.com
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

#ifndef PROPPAGE_H
#define PROPPAGE_H

#define SETTINGS_BUF_LEN 1024

class SettingsManager;

class PropPage
{
public:
	PropPage(SettingsManager *src);
	virtual ~PropPage();

	virtual PROPSHEETPAGE *getPSP() = 0;
	virtual void write() = 0;

	enum Type { T_STR, T_INT, T_BOOL, T_CUSTOM, T_END };
	struct Item
	{
		WORD itemID;
		int setting;
		Type type;
	};

protected:
	SettingsManager *settings;
	void read(HWND page, Item const* items);
	void write(HWND page, Item const* items);
};

#endif // PROPPAGE_H

/**
 * @file PropPage.h
 * $Id: PropPage.h,v 1.2 2002/01/26 12:52:51 arnetheduck Exp $
 * @if LOG
 * $Log: PropPage.h,v $
 * Revision 1.2  2002/01/26 12:52:51  arnetheduck
 * More minor fixes
 *
 * @endif
 */


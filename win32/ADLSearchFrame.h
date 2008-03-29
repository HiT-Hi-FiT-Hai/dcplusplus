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

#ifndef DCPLUSPLUS_WIN32_ADL_SEARCH_FRAME_H
#define DCPLUSPLUS_WIN32_ADL_SEARCH_FRAME_H

#include "StaticFrame.h"
#include <dcpp/ADLSearch.h>

class ADLSearchFrame : public StaticFrame<ADLSearchFrame> {
public:
	enum Status {
		STATUS_STATUS,
		STATUS_LAST
	};

protected:
	typedef StaticFrame<ADLSearchFrame> BaseType;
	friend class StaticFrame<ADLSearchFrame>;
	friend class MDIChildFrame<ADLSearchFrame>;

	ADLSearchFrame(SmartWin::WidgetTabView* mdiParent);
	virtual ~ADLSearchFrame();

	void layout();

	bool preClosing();
	void postClosing();

private:
	enum {
		COLUMN_FIRST,
		COLUMN_ACTIVE_SEARCH_STRING = COLUMN_FIRST,
		COLUMN_SOURCE_TYPE,
		COLUMN_DEST_DIR,
		COLUMN_MIN_FILE_SIZE,
		COLUMN_MAX_FILE_SIZE,
		COLUMN_LAST
	};

	static int columnSizes[COLUMN_LAST];
	static int columnIndexes[COLUMN_LAST];

	WidgetListViewPtr items;
	ButtonPtr add;
	ButtonPtr properties;
	ButtonPtr up;
	ButtonPtr down;
	ButtonPtr remove;
	ButtonPtr help;

	void handleAdd();
	void handleProperties();
	void handleUp();
	void handleDown();
	void handleRemove();
	void handleDoubleClick();
	bool handleKeyDown(int c);
	LRESULT handleItemChanged(LPARAM lParam);
	bool handleContextMenu(SmartWin::ScreenCoordinate sc);

	void addEntry(ADLSearch& search, int index = -1);
};

#endif // !defined(DCPLUSPLUS_WIN32_ADL_SEARCH_FRAME_H)

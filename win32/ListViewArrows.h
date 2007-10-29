/*
 * Copyright (C) 2001-2006 Jacek Sieka, arnetheduck on gmail point com
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

#if !defined(LIST_VIEW_ARROWS_H)
#define LIST_VIEW_ARROWS_H

#include <SmartWin.h>

template<class T>
class ListViewArrows {
public:
	ListViewArrows() {
		rebuildArrows();
	}

	virtual ~ListViewArrows() { }

	typedef ListViewArrows<T> thisClass;

	void updateArrow() {
		if (upArrow.IsNull())
			return;

		T* pThis = (T*)this;
		HBITMAP bitmap = (pThis->isAscending() ? upArrow : downArrow);

		CHeaderCtrl headerCtrl = pThis->GetHeader();
		const int itemCount = headerCtrl.GetItemCount();
		for (int i=0; i < itemCount; ++i)
		{
			HDITEM item;
			item.mask = HDI_FORMAT;
			headerCtrl.GetItem(i, &item);
			item.mask = HDI_FORMAT | HDI_BITMAP;
			if (i == pThis->getSortColumn()) {
				item.fmt |= HDF_BITMAP | HDF_BITMAP_ON_RIGHT;
				item.hbm = bitmap;
			} else {
				item.fmt &= ~(HDF_BITMAP | HDF_BITMAP_ON_RIGHT);
				item.hbm = 0;
			}
			headerCtrl.SetItem(i, &item);
		}
	}

private:
	SmartWin::BitmapPtr upArrow;
	SmartWin::BitmapPtr downArrow;
};

#endif // !defined(LIST_VIEW_ARROWS_H)

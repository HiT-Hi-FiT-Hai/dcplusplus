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

	void rebuildArrows()
	{
		POINT pathArrowLong[9] = {{0L,7L},{7L,7L},{7L,6L},{6L,6L},{6L,4L},{5L,4L},{5L,2L},{4L,2L},{4L,0L}};
		POINT pathArrowShort[7] = {{0L,6L},{1L,6L},{1L,4L},{2L,4L},{2L,2L},{3L,2L},{3L,0L}};

		SmartWin::FreeCanvas dc(NULL, ::CreateCompatibleDc(NULL));

		const int bitmapWidth = 8;
		const int bitmapHeight = 8;
		const Rectangle rect = {0, 0, bitmapWidth, bitmapHeight};

		SmartWin::Brush brush(dc, COLOR_3DFACE);
		SmartWin::Pen penLight(dc, COLOR_3DHIGHLIGHT, 1);
		SmartWin::Pen penShadow(dc, COLOR_3DSHADOW, 1);

		upArrow = BitmapPtr(new Bitmap(::CreateCompatibleBitmap(dc.getDc(), bitmapWidth, bitmapHeight)));

		downArrow = BitmapPtr(new Bitmap(::CreateCompatibleBitmap(dc.getDc(), bitmapWidth, bitmapHeight)));

		// create up arrow
		::SelectBitmap(dc.getDc(), upArrow.handle());
		dc.FillRect(&rect, brush);
#ifdef PORT_ME
		dc.SelectPen(penLight);
		dc.Polyline(pathArrowLong, sizeof(pathArrowLong)/sizeof(pathArrowLong[0]));
		dc.SelectPen(penShadow);
		dc.Polyline(pathArrowShort, sizeof(pathArrowShort)/sizeof(pathArrowShort[0]));

		// create down arrow
		dc.SelectBitmap(downArrow);
		dc.FillRect(&rect, brush);
		for (int i=0; i < sizeof(pathArrowShort)/sizeof(pathArrowShort[0]); ++i)
		{
			POINT& pt = pathArrowShort[i];
			pt.x = bitmapWidth - pt.x;
			pt.y = bitmapHeight - pt.y;
		}
		dc.SelectPen(penLight);
		dc.Polyline(pathArrowShort, sizeof(pathArrowShort)/sizeof(pathArrowShort[0]));
		for (int i=0; i < sizeof(pathArrowLong)/sizeof(pathArrowLong[0]); ++i)
		{
			POINT& pt = pathArrowLong[i];
			pt.x = bitmapWidth - pt.x;
			pt.y = bitmapHeight - pt.y;
		}
		dc.SelectPen(penShadow);
		dc.Polyline(pathArrowLong, sizeof(pathArrowLong)/sizeof(pathArrowLong[0]));
#endif
	}

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

/* 
 * Copyright (C) 2001-2003 Jacek Sieka, j_s@telia.com
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
#include "../client/DCPlusPlus.h"
#include "Resource.h"

#include "ExListViewCtrl.h"

int ExListViewCtrl::moveItem(int oldPos, int newPos) {

	char buf[512];
	LVITEM lvi;
	lvi.iItem = oldPos;
	lvi.iSubItem = 0;
	lvi.mask = LVIF_IMAGE | LVIF_INDENT | LVIF_PARAM | LVIF_STATE;
	GetItem(&lvi);
	StringList l;

	for(int j = 0; j < GetHeader().GetItemCount(); j++) {
		GetItemText(oldPos, j, buf, 512);
		l.push_back(buf);
	}

	SetRedraw(FALSE);
	
	lvi.iItem = newPos;
	int i = InsertItem(&lvi);
	j = 0;
	for(StringIter k = l.begin(); k != l.end(); ++k, j++) {
		SetItemText(i, j, k->c_str());
	}
	
	if(i < oldPos)
		DeleteItem(oldPos + 1);
	else
		DeleteItem(oldPos);

	SetRedraw(TRUE);

	RECT rc;
	GetItemRect(i, &rc, LVIR_BOUNDS);
	InvalidateRect(&rc);
	
	return i;
}

int ExListViewCtrl::insert(StringList& aList, int iImage, LPARAM lParam) {

	char buf[128];
	int loc = 0;
	int count = GetItemCount();

	LVITEM a;
	a.mask = LVIF_IMAGE | LVIF_INDENT | LVIF_PARAM | LVIF_STATE | LVIF_TEXT;
	a.iItem = -1;
	a.iSubItem = sortColumn;
	a.iImage = iImage;
	a.state = 0;
	a.stateMask = 0;
	a.lParam = lParam;
	a.iIndent = 0;
	a.pszText = const_cast<char*>(sortColumn == -1 ? aList[0].c_str() : aList[sortColumn].c_str());
	a.cchTextMax = sortColumn == -1 ? aList[0].size() : aList[sortColumn].size();
	
	if(sortColumn == -1) {
		loc = count;
	} else if(count == 0) {
		loc = 0;
	} else {

		string& b = aList[sortColumn];
		int c = atoi(b.c_str());
		double f = atof(b.c_str());
		LPARAM data = NULL;			
		int low = 0;
		int high = count-1;
		int comp = 0;
		while(low <= high)
		{
			loc = (low + high)/2;
			
			// This is a trick, so that if fun() returns something bigger than one, use the
			// internal default sort functions
			comp = sortType;
			if(comp == SORT_FUNC) {
				data = GetItemData(loc);
				comp = fun(lParam, data);
			} else if(comp == SORT_FUNC_ITEM) {
				LVITEM b;
				b.mask = LVIF_IMAGE | LVIF_INDENT | LVIF_PARAM | LVIF_STATE;
				b.iItem = loc;
				b.iSubItem = 0;
				GetItem(&b);
				comp = fun((LPARAM)&a, (LPARAM)&b);
			}
			
			if(comp == SORT_STRING) {
				GetItemText(loc, sortColumn, buf, 128);
				comp = compare(b, string(buf));
			} else if(comp == SORT_STRING_NOCASE) {
				GetItemText(loc, sortColumn, buf, 128);
				comp =  Util::stricmp(b.c_str(), buf);
			} else if(comp == SORT_INT) {
				GetItemText(loc, sortColumn, buf, 128);
				comp = compare(c, atoi(buf)); 
			} else if(comp == SORT_FLOAT) {
				GetItemText(loc, sortColumn, buf, 128);
				comp = compare(f, atof(buf));
			}
			
			if(!ascending)
				comp = -comp;

			if(comp == -1) {
				high = loc - 1;
			} else if(comp == 1) {
				low = loc + 1;
			} else {
				break;
			} 
		}

		comp = sortType;
		if(comp == SORT_FUNC) {
			comp = fun(lParam, data);
		} else if(comp == SORT_FUNC_ITEM) {
			LVITEM b;
			b.mask = LVIF_IMAGE | LVIF_INDENT | LVIF_PARAM | LVIF_STATE;
			b.iItem = loc;
			b.iSubItem = 0;
			GetItem(&b);
			comp = fun((LPARAM)&a, (LPARAM)&b);
		}
		
		if(comp == SORT_STRING) {
			comp = compare(b, string(buf));
		} else if(comp == SORT_STRING_NOCASE) {
			comp =  Util::stricmp(b.c_str(), buf);
		} else if(comp == SORT_INT) {
			comp = compare(c, atoi(buf)); 
		} else if(comp == SORT_FLOAT) {
			comp = compare(f, atof(buf));
		}

		if(!ascending)
			comp = -comp;
		
		if(comp == 1)
			loc++;
	}
	dcassert(loc >= 0 && loc <= GetItemCount());
	a.iItem = loc;
	a.iSubItem = 0;
	int i = InsertItem(&a);
	int k = 0;
	for(StringIter j = aList.begin(); j != aList.end(); ++j, k++) {
		SetItemText(i, k, j->c_str());
	}
	return loc;
}

int ExListViewCtrl::insert(int nItem, StringList& aList, int iImage, LPARAM lParam) {

	dcassert(aList.size() > 0);

	int i = insert(nItem, aList[0], iImage, lParam);

	int k = 0;
	for(StringIter j = aList.begin(); j != aList.end(); ++j, k++) {
		SetItemText(i, k, j->c_str());
	}
	return i;
	
}

void ExListViewCtrl::rebuildArrows()
{
	POINT pathArrowLong[9] = {{0L,7L},{7L,7L},{7L,6L},{6L,6L},{6L,4L},{5L,4L},{5L,2L},{4L,2L},{4L,0L}};
	POINT pathArrowShort[7] = {{0L,6L},{1L,6L},{1L,4L},{2L,4L},{2L,2L},{3L,2L},{3L,0L}};

	HDC dc = 0;
	HBRUSH brush = 0;
	HPEN penLight = 0;
	HPEN penShadow = 0;
	
	const int bitmapWidth = 8;
	const int bitmapHeight = 8;
	const RECT rect = {0, 0, bitmapWidth, bitmapHeight};

	dc = ::CreateCompatibleDC(GetDC());
	if (!dc)
		goto Ldestroy;

	brush = ::GetSysColorBrush(COLOR_3DFACE);
	if (!brush)
		goto Ldestroy;

	penLight = ::CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_3DHIGHLIGHT));
	if (!penLight)
		goto Ldestroy;

	penShadow = ::CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_3DSHADOW));
	if (!penShadow)
		goto Ldestroy;

	if (!upArrow)
		upArrow = ::CreateCompatibleBitmap(GetDC(), bitmapWidth, bitmapHeight);

	if (!downArrow)
		downArrow = ::CreateCompatibleBitmap(GetDC(), bitmapWidth, bitmapHeight);

	int i;

	// create up arrow
	::SelectObject(dc, upArrow);
	::FillRect(dc, &rect, brush);
	::SelectObject(dc, penLight);
	::Polyline(dc, pathArrowLong, sizeof(pathArrowLong)/sizeof(pathArrowLong[0]));
	::SelectObject(dc, penShadow);
	::Polyline(dc, pathArrowShort, sizeof(pathArrowShort)/sizeof(pathArrowShort[0]));

	// create down arrow
	::SelectObject(dc, downArrow);
	::FillRect(dc, &rect, brush);
	for (i=0; i < sizeof(pathArrowShort)/sizeof(pathArrowShort[0]); ++i)
	{
		POINT& pt = pathArrowShort[i];
		pt.x = bitmapWidth - pt.x;
		pt.y = bitmapHeight - pt.y;
	}
	::SelectObject(dc, penLight);
	::Polyline(dc, pathArrowShort, sizeof(pathArrowShort)/sizeof(pathArrowShort[0]));
	for (i=0; i < sizeof(pathArrowLong)/sizeof(pathArrowLong[0]); ++i)
	{
		POINT& pt = pathArrowLong[i];
		pt.x = bitmapWidth - pt.x;
		pt.y = bitmapHeight - pt.y;
	}
	::SelectObject(dc, penShadow);
	::Polyline(dc, pathArrowLong, sizeof(pathArrowLong)/sizeof(pathArrowLong[0]));

Ldestroy:
	if (dc)
		::DeleteDC(dc);

	if (penLight)
		::DeleteObject(penLight);

	if (penShadow)
		::DeleteObject(penShadow);
}

void ExListViewCtrl::updateArrow()
{
	if (!upArrow)
		return;

	HBITMAP bitmap = (ascending ? upArrow : downArrow);

	CHeaderCtrl headerCtrl = GetHeader();
	const int itemCount = headerCtrl.GetItemCount();
	for (int i=0; i < itemCount; ++i)
	{
		HDITEM item;
		item.mask = HDI_FORMAT;
		headerCtrl.GetItem(i, &item);
		item.mask = HDI_FORMAT | HDI_BITMAP;
		if (i == sortColumn)
		{
			item.fmt |= HDF_BITMAP | HDF_BITMAP_ON_RIGHT;
			item.hbm = bitmap;
		}
		else
		{
			item.fmt &= ~(HDF_BITMAP | HDF_BITMAP_ON_RIGHT);
			item.hbm = 0;
		}
		headerCtrl.SetItem(i, &item);
	}
}


/**
 * @file
 * $Id: ExListViewCtrl.cpp,v 1.8 2003/08/07 13:28:18 arnetheduck Exp $
 */


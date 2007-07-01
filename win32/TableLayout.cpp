/*
 * Copyright (C) 2001-2007 Jacek Sieka, arnetheduck on gmail point com
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
#include <dcpp/DCPlusPlus.h>

#include "TableLayout.h"

static const int border = 6;

TableLayout::TableLayout(int cols_, int rows_) : cols(cols_), rows(rows_) {

}

void TableLayout::add(SmartWin::Widget* widget, const SmartWin::Point wantedSize, size_t x, size_t y, size_t xspan, size_t yspan, int xoptions, int yoptions) {
	
}

void TableLayout::resize(const SmartWin::Rectangle& clientRect) {
	CellList x(cols), y(rows);
	
	// Set each col/row to wanted size
	for(size_t i = 0; i < widgets.size(); ++i) {
		WidgetInfo& wi = widgets[i];
		// TODO Update xwanted for each column/row
		long xwanted = (wi.wantedSize.x - (wi.xspan-1) * border) / wi.xspan;
		for(size_t j = wi.x; j < wi.x + wi.xspan && j < x.size(); ++j) {
			x[j].size = std::max(x[j].size, xwanted);
			x[j].expand |= (wi.xoptions & EXPAND) > 0;
		}
		long ywanted = (wi.wantedSize.y - (wi.yspan-1) * border) / wi.yspan;
		for(size_t j = wi.y; j < wi.y + wi.yspan && j < y.size(); ++j) {
			y[j].size = std::max(y[j].size, ywanted);
			y[j].expand |= (wi.yoptions & EXPAND) > 0;
		}
	}
	
	// Fill unused space
	size_t xused = 0;
	size_t xexpanders = 0;
	for(size_t i = 0; i < x.size(); ++i) {
		xused += x[i].size;
		xexpanders += x[i].expand;
	}
	size_t xavail = clientRect.size.x - (x.size()-1) * border;
	
	if(xused < xavail && xexpanders > 0) {
		size_t xexpand = (xavail - xused) / xexpanders;
		for(size_t i = 0; i < x.size(); ++i) {
			if(x[i].expand) {
				x[i].size += xexpand;
			}
		}
	}
	
	size_t yused = 0;
	size_t yexpanders = 0;
	for(size_t i = 0; i < y.size(); ++i) {
		yused += y[i].size;
		yexpanders += y[i].expand;
	}
	size_t yavail = clientRect.size.y - (y.size()-1) * border;
	
	if(yused < yavail && yexpanders > 0) {
		size_t yexpand = (yavail - yused) / yexpanders;
		for(size_t i = 0; i < y.size(); ++i) {
			if(y[i].expand) {
				y[i].size += yexpand;
			}
		}
	}

	// Resize widget
	for(size_t i = 0; i < widgets.size(); ++i) {
		WidgetInfo& wi = widgets[i];
		size_t xstart = 0;
		for(size_t j = 0; j < wi.x; ++j) {
			xstart += x[j].size + border;
		}
		size_t xtotal = 0;
		if(wi.xoptions & (FILL | EXPAND)) {
			for(size_t j = wi.x; j < wi.x + wi.xspan && j < x.size(); ++j) {
				xtotal += x[j].size;
			}
			xtotal += (wi.xspan - 1) * border;
		} else {
			xtotal = wi.wantedSize.x;
		}
		
		size_t ystart = 0;
		for(size_t j = 0; j < wi.x; ++j) {
			ystart += y[j].size + border;
		}
		
		size_t ytotal = 0;
		if(wi.yoptions & (FILL | EXPAND)) {
			for(size_t j = wi.y; j < wi.y + wi.yspan && j < y.size(); ++j) {
				ytotal += y[j].size;
			}
			ytotal += (wi.yspan - 1) * border;
		} else {
			ytotal = wi.wantedSize.y;
		}
		
		::MoveWindow(wi.widget->handle(), xstart, ystart, xtotal, ytotal, TRUE);
	}
}

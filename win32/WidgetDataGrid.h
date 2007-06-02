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

#ifndef DCPLUSPLUS_WIN32_WIDGETDATAGRID_H_
#define DCPLUSPLUS_WIN32_WIDGETDATAGRID_H_

template< class EventHandlerClass, class MessageMapPolicy >
class WidgetDataGrid : public SmartWin::WidgetDataGrid<EventHandlerClass, MessageMapPolicy> {
private:
	typedef SmartWin::WidgetDataGrid<EventHandlerClass, MessageMapPolicy> BaseType;
public:
	typedef WidgetDataGrid<EventHandlerClass, MessageMapPolicy>* ObjectType;
	
	// Constructor Taking pointer to parent
	explicit WidgetDataGrid( SmartWin::Widget * parent ) : SmartWin::Widget(parent), BaseType(parent) { }

	using BaseType::addRemoveListViewExtendedStyle;
	
	bool setColumnOrder(const std::vector<int>& columns) {
		return ::SendMessage(this->handle(), LVM_SETCOLUMNORDERARRAY, static_cast<WPARAM>(columns.size()), reinterpret_cast<LPARAM>(&columns[0])) > 0;
	}
	std::vector<int> getColumnOrder() {
		std::vector<int> ret(this->getColumnCount());
		if(!::SendMessage(this->handle(), LVM_GETCOLUMNORDERARRAY, static_cast<WPARAM>(ret.size()), reinterpret_cast<LPARAM>(&ret[0]))) {
			ret.clear();
		}
		return ret;
	}
	
	void setColumnWidths(const std::vector<int>& widths) {
		for(size_t i = 0; i < widths.size(); ++i) {
			this->setColumnWidth(i, widths[i]);
		}
	}
	
	std::vector<int> getColumnWidths() {
		std::vector<int> ret(this->getColumnCount());
		for(size_t i = 0; i < ret.size(); ++i) {
			ret[i] = ::SendMessage(this->handle(), LVM_GETCOLUMNWIDTH, static_cast<WPARAM>(i), 0);
		}			
		return ret;
	}
	
	int getSelectedCount() {
		return ListView_GetSelectedCount(this->handle());
	}
	
	void setListViewStyle(int style) {
		ListView_SetExtendedListViewStyle(this->handle(), style);
	}
	
	int getNextItem(int i, int type) {
		return ListView_GetNextItem(this->handle(), i, type);
	}
	
	LPARAM getItemData(int idx) {
		LVITEM item = { LVIF_PARAM };
		item.iItem = idx;
		if(!::SendMessage(this->handle(), LVM_GETITEM, 0, reinterpret_cast<LPARAM>(&item))) {
			return 0;
		}
		return item.lParam;
	}
	
	POINT getContextMenuPos() {
		int pos = getNextItem(-1, LVNI_SELECTED | LVNI_FOCUSED);
		POINT pt = { 0 };
		if(pos >= 0) {
			RECT lrc = this->getItemRect(pos, LVIR_LABEL);
			pt.x = lrc.left;
			pt.y = lrc.top + ((lrc.bottom - lrc.top) / 2);
		} 
		this->clientToScreen(pt);
		return pt;
	}
	
	void setColor(COLORREF text, COLORREF background) {
		ListView_SetTextColor(this->handle(), text);
		ListView_SetTextBkColor(this->handle(), background);
		ListView_SetBkColor(this->handle(), background);
	}

private:

};
#endif /*WIDGETDATAGRID_H_*/

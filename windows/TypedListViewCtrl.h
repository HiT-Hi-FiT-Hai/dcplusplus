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

#if !defined(AFX_TYPEDLISTVIEWCTRL_H__45847002_68C2_4C8A_9C2D_C4D8F65DA841__INCLUDED_)
#define AFX_TYPEDLISTVIEWCTRL_H__45847002_68C2_4C8A_9C2D_C4D8F65DA841__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ListViewArrows.h"

template<class T, int ctrlId>
class TypedListViewCtrl : public CWindowImpl<TypedListViewCtrl, CListViewCtrl, CControlWinTraits>,
	ListViewArrows<TypedListViewCtrl<T, ctrlId> >
{
public:
	TypedListViewCtrl() : sortColumn(-1), sortAscending(true) { };

	typedef TypedListViewCtrl<T, ctrlId> thisClass;
	typedef CListViewCtrl baseClass;
	typedef ListViewArrows<thisClass> arrowBase;

	BEGIN_MSG_MAP(thisClass)
		REFLECTED_NOTIFY_HANDLER(ctrlId, LVN_GETDISPINFO, onGetDispInfo)
		REFLECTED_NOTIFY_HANDLER(ctrlId, LVN_COLUMNCLICK, onColumnClick)
		CHAIN_MSG_MAP(arrowBase)
	END_MSG_MAP();

	LRESULT onGetDispInfo(int /* idCtrl */, LPNMHDR pnmh, BOOL& /* bHandled */) {
		NMLVDISPINFO* di = (NMLVDISPINFO*)pnmh;
		if(di->item.mask & LVIF_TEXT) {
			di->item.pszText = const_cast<char*>(getItemData(di->item.iItem)->getText(di->item.iSubItem).c_str());
		}
		return 0;
	}

	// Sorting
	LRESULT onColumnClick(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
		NMLISTVIEW* l = (NMLISTVIEW*)pnmh;
		if(l->iSubItem != sortColumn) {
			sortAscending = true;
			sortColumn = l->iSubItem;
		} else if(sortAscending) {
			sortAscending = false;
		} else {
			sortColumn = -1;
		}
		updateArrow();
		resort();
		return 0;
	}
	void resort() {
		if(sortColumn != -1) {
			SortItemsEx(&compareFunc, (LPARAM)this);
		}
	}

	int insertItem(T* item, int image) {
		return insertItem(getSortPos(item), item, image);
	}
	int insertItem(int i, T* item, int image) {
		return InsertItem(LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE, i, 
			LPSTR_TEXTCALLBACK, 0, 0, image, (LPARAM)item);
	}
	T* getItemData(int iItem) { return (T*)GetItemData(iItem); }
	int findItem(T* item) { 
		LVFINDINFO fi = { LVFI_PARAM, NULL, (LPARAM)item };
		return FindItem(&fi, -1);
	}
	int findItem(const string& b, int col = 0) {
		int n = GetItemCount();
		for(int i = 0; i < n; ++i) {
			if(Util::stricmp(getItemData(i)->getText(col), b) == 0)
				return i;
		}
		return -1;
	}
	void forEach(void (T::*func)()) {
		int n = GetItemCount();
		for(int i = 0; i < n; ++i)
			(getItemData(i)->*func)();
	}
	void forEachSelected(void (T::*func)()) {
		int i = -1;
		while( (i = GetNextItem(i, LVNI_SELECTED)) != -1)
			(getItemData(i)->*func)();
	}
	template<class _Function>
	_Function forEachT(_Function pred) {
		int n = GetItemCount();
		for(int i = 0; i < n; ++i)
			pred(getItemData(i));
		return pred;
	}
	template<class _Function>
	_Function forEachSelectedT(_Function pred) {
		int i = -1;
		while( (i = GetNextItem(i, LVNI_SELECTED)) != -1)
			pred(getItemData(i));
		return pred;
	}
	
	void update(int i) {
		int k = GetHeader().GetItemCount();
		for(int j = 0; j < k; ++j)
			SetItemText(i, j, LPSTR_TEXTCALLBACK);
	}
	void updateItem(T* item) { int i = findItem(item); if(i != -1) update(i); };
	void deleteItem(T* item) { int i = findItem(item); if(i != -1) DeleteItem(i); };

	int getSortPos(T* a) {
		int high = GetItemCount();
		if((sortColumn == -1) || (high == 0))
			return high;

		high--;

		int low = 0;
		int mid = 0;
		T* b = NULL;
		int comp = 0;
		while( low <= high ) {
			mid = (low + high) / 2;
			b = getItemData(mid);
			comp = T::compareItems(a, b, sortColumn);
			
			if(!sortAscending)
				comp = -comp;

			if(comp == 0) {
				return mid;
			} else if(comp == -1) {
				high = mid - 1;
			} else if(comp == 1) {
				low = mid + 1;
			}
		}

		comp = T::compareItems(a, b, sortColumn);
		if(!sortAscending)
			comp = -comp;
		if(comp == 1)
			mid++;

		return mid;
	}

	bool isAscending() { return sortAscending; };

	GETSET(int, sortColumn, SortColumn);
private:

	bool sortAscending;

	static int CALLBACK compareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort) {
		thisClass* t = (thisClass*)lParamSort;
		int result = T::compareItems(t->getItemData(lParam1), t->getItemData(lParam2), t->sortColumn);
		return (t->sortAscending ? result : -result);
	}
};

#endif

/**
* @file
* $Id: TypedListViewCtrl.h,v 1.4 2003/11/12 21:45:00 arnetheduck Exp $
*/

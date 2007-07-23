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

#ifndef DCPLUSPLUS_WIN32_TYPED_LIST_VIEW_CTRL_H
#define DCPLUSPLUS_WIN32_TYPED_LIST_VIEW_CTRL_H

template<class T, class ContentType>
class TypedListViewCtrl : public T::WidgetDataGrid
{
private:
	typedef typename T::WidgetDataGrid BaseType;
	typedef TypedListViewCtrl<T, ContentType> ThisType;
	
public:
	typedef ThisType* ObjectType;

	explicit TypedListViewCtrl( SmartWin::Widget * parent ) : BaseType(parent), sortColumn(-1), sortAscending(true) { 
		
	}
	
	virtual void create( const typename BaseType::Seed & cs = BaseType::getDefaultSeed() ) {
		BaseType::create(cs);
		
		this->setCallback(
			SmartWin::Message( WM_NOTIFY, LVN_GETDISPINFO ), &TypedListViewDispatcher
		);
	}
	
	void resort() {
		if(sortColumn != -1) {
			ListView_SortItems(this->handle(), &compareFunc, reinterpret_cast< LPARAM >(this));
		}
	}

	int insertItem(ContentType* item) {
		return insertItem(getSortPos(item), item);
	}
	int insertItem(int i, ContentType* item) {
		return BaseType::insertItem(LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE, i,
			LPSTR_TEXTCALLBACK, 0, 0, I_IMAGECALLBACK, reinterpret_cast<LPARAM>(item));
	}
	
	ContentType* getItemData(int iItem) { 
		LVITEM item = { LVIF_PARAM };
        item.iItem = iItem;
		if(ListView_GetItem(this->handle(), &item)) {
			return reinterpret_cast<ContentType*>(item.lParam);
		}
		return 0;
	}

	BOOL setItemData(int iItem, ContentType* lparam) {
		LVITEM item = { LVIF_PARAM };
		item.iItem = iItem;
		item.lParam = reinterpret_cast<LPARAM>(lparam);
		return ListView_SetItem(this->handle(), &item);
	}

	ContentType* getSelectedItem() { return this->hasSelection() ? getItemData(this->getSelectedIndex()) : 0; }

	int findItem(ContentType* item) {
		LVFINDINFO fi = { LVFI_PARAM, NULL, (LPARAM)item };
		return ListView_FindItem(this->handle(), -1, &fi);
	}
	
	struct CompFirst {
		CompFirst() { }
		bool operator()(ContentType& a, const tstring& b) {
			return Util::stricmp(a.getText(0), b) < 0;
		}
	};
	int findItem(const tstring& b, int start = -1, bool aPartial = false) {
		LVFINDINFO fi = { aPartial ? LVFI_PARTIAL : LVFI_STRING, b.c_str() };
		return ListView_FindItem(this->handle(), start, &fi);
	}

	void forEach(void (ContentType::*func)()) {
		unsigned n = this->getRowCount();
		for(unsigned i = 0; i < n; ++i)
			(getItemData(i)->*func)();
	}
	void forEachSelected(void (ContentType::*func)()) {
		int i = -1;
		while( (i = ListView_GetNextItem(this->handle(), i, LVNI_SELECTED)) != -1)
			(getItemData(i)->*func)();
	}
	template<class _Function>
	_Function forEachT(_Function pred) {
		unsigned n = this->getRowCount();
		for(unsigned i = 0; i < n; ++i)
			pred(getItemData(i));
		return pred;
	}
	template<class _Function>
	_Function forEachSelectedT(_Function pred) {
		int i = -1;
		while( (i = ListView_GetNextItem(this->handle(), i, LVNI_SELECTED)) != -1)
			pred(getItemData(i));
		return pred;
	}

	void updateItem(int i) {
		unsigned k = this->getColumnCount();
		for(unsigned j = 0; j < k; ++j)
			ListView_SetItemText(this->handle(), i, j, LPSTR_TEXTCALLBACK);
	}
	void updateItem(ContentType* item) { int i = findItem(item); if(i != -1) updateItem(i); }
	void deleteItem(ContentType* item) { int i = findItem(item); if(i != -1) this->removeRow(i); }

	int getSortPos(ContentType* a) {
		int high = this->getRowCount();
		if((sortColumn == -1) || (high == 0))
			return high;

		high--;

		int low = 0;
		int mid = 0;
		ContentType* b = NULL;
		int comp = 0;
		while( low <= high ) {
			mid = (low + high) / 2;
			b = getItemData(mid);
			comp = ContentType::compareItems(a, b, sortColumn);

			if(!sortAscending)
				comp = -comp;

			if(comp == 0) {
				return mid;
			} else if(comp < 0) {
				high = mid - 1;
			} else if(comp > 0) {
				low = mid + 1;
			}
		}

		comp = ContentType::compareItems(a, b, sortColumn);
		if(!sortAscending)
			comp = -comp;
		if(comp > 0)
			mid++;

		return mid;
	}

	void setSortColumn(int aSortColumn) {
		sortColumn = aSortColumn;
		//TODO updateArrow();
	}
	int getSortColumn() { return sortColumn; }
	bool isAscending() { return sortAscending; }

private:

	int sortColumn;
	bool sortAscending;

	static int CALLBACK compareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort) {
		ThisType* t = reinterpret_cast<ThisType*>(lParamSort);
		int result = ContentType::compareItems((ContentType*)lParam1, (ContentType*)lParam2, t->sortColumn);
		return (t->sortAscending ? result : -result);
	}
	
	static HRESULT TypedListViewDispatcher(SmartWin::private_::SignalContent& params) {
		NMLVDISPINFO * nm = reinterpret_cast< NMLVDISPINFO * >( params.Msg.LParam );
		if(nm->item.mask & LVIF_TEXT) {
			ContentType* content = reinterpret_cast<ContentType*>(nm->item.lParam);
			const string& text = content->getText(nm->item.iSubItem);
			strncpy(nm->item.pszText, text.data(), std::min(text.size(), (size_t)nm->item.cchTextMax));
			if(text.size() < nm->item.cchTextMax) {
				nm->item.pszText[text.size()] = 0;
			}
		}
		if(nm->item.mask & LVIF_IMAGE) {
			ContentType* content = reinterpret_cast<ContentType*>(nm->item.lParam);
			nm->item.iImage = content->getImage();
		}
		return 0;
	}
	
};

#ifdef PORT_ME
#include "ListViewArrows.h"
#include "memdc.h"

template<class T, class ContentType>
class TypedListViewCtrl : public T::WidgetDataGrid,
	ListViewArrows<TypedListViewCtrl<T, ctrlId> >
{

	typedef TypedListViewCtrl<T, ctrlId> thisClass;
	typedef CListViewCtrl baseClass;
	typedef ListViewArrows<thisClass> arrowBase;

	BEGIN_MSG_MAP(thisClass)
		MESSAGE_HANDLER(WM_CHAR, onChar)
		MESSAGE_HANDLER(WM_ERASEBKGND, onEraseBkgnd)
		MESSAGE_HANDLER(WM_PAINT, onPaint)
		CHAIN_MSG_MAP(arrowBase)
	END_MSG_MAP();

	class iterator : public ::iterator<random_access_iterator_tag, T*> {
	public:
		iterator() : typedList(NULL), cur(0), cnt(0) { }
		iterator(const iterator& rhs) : typedList(rhs.typedList), cur(rhs.cur), cnt(rhs.cnt) { }
		iterator& operator=(const iterator& rhs) { typedList = rhs.typedList; cur = rhs.cur; cnt = rhs.cnt; return *this; }

		bool operator==(const iterator& rhs) const { return cur == rhs.cur; }
		bool operator!=(const iterator& rhs) const { return !(*this == rhs); }
		bool operator<(const iterator& rhs) const { return cur < rhs.cur; }

		int operator-(const iterator& rhs) const {
			return cur - rhs.cur;
		}

		iterator& operator+=(int n) { cur += n; return *this; }
		iterator& operator-=(int n) { return (cur += -n); }

		T& operator*() { return *typedList->getItemData(cur); }
		T* operator->() { return &(*(*this)); }
		T& operator[](int n) { return *typedList->getItemData(cur + n); }

		iterator operator++(int) {
			iterator tmp(*this);
			operator++();
			return tmp;
		}
		iterator& operator++() {
			++cur;
			return *this;
		}

	private:
		iterator(thisClass* aTypedList) : typedList(aTypedList), cur(aTypedList->GetNextItem(-1, LVNI_ALL)), cnt(aTypedList->GetItemCount()) {
			if(cur == -1)
				cur = cnt;
		}
		iterator(thisClass* aTypedList, int first) : typedList(aTypedList), cur(first), cnt(aTypedList->GetItemCount()) {
			if(cur == -1)
				cur = cnt;
		}
		friend class thisClass;
		thisClass* typedList;
		int cur;
		int cnt;
	};

	LRESULT onChar(UINT /*msg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
		if((GetKeyState(VkKeyScan('A') & 0xFF) & 0xFF00) > 0 && (GetKeyState(VK_CONTROL) & 0xFF00) > 0){
			int count = GetItemCount();
			for(int i = 0; i < count; ++i)
				ListView_SetItemState(m_hWnd, i, LVIS_SELECTED, LVIS_SELECTED);

			return 0;
		}

		bHandled = FALSE;
		return 1;
	}

	LRESULT onGetDispInfo(int /* idCtrl */, LPNMHDR pnmh, BOOL& /* bHandled */) {
		NMLVDISPINFO* di = (NMLVDISPINFO*)pnmh;
		if(di->item.mask & LVIF_TEXT) {
			di->item.mask |= LVIF_DI_SETITEM;
			di->item.pszText = const_cast<TCHAR*>(((T*)di->item.lParam)->getText(di->item.iSubItem).c_str());
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
	iterator begin() { return iterator(this); }
	iterator end() { return iterator(this, GetItemCount()); }

	LRESULT onEraseBkgnd(UINT /*msg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		return 0;
	}

	LRESULT onPaint(UINT msg, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		CRect updateRect;
		CRect crc;
		GetClientRect(&crc);
		if(GetUpdateRect(updateRect, FALSE)){
			CPaintDC dc(m_hWnd);
			int oldDCSettings = dc.SaveDC();
			dc.SetBkColor(WinUtil::bgColor);
			CMemDC memDC(&dc, &crc);

			//it seems like the default paint method actually
			//uses the HDC it's passed, saves a lot of work =)
			LRESULT ret = DefWindowProc(msg, (WPARAM)memDC.m_hDC, NULL);

			//make sure to paint before CPaintDC goes out of scope and destroys our hdc
			memDC.Paint();

			dc.RestoreDC(oldDCSettings);

			return ret;
		}

		return 0;
	}

};
#endif

#endif // !defined(TYPED_LIST_VIEW_CTRL_H)

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

#ifndef DCPLUSPLUS_WIN32_TYPED_LIST_VIEW_H
#define DCPLUSPLUS_WIN32_TYPED_LIST_VIEW_H

template<class T, class ContentType>
class TypedListView : public T::WidgetListView
{
private:
	typedef typename T::WidgetListView BaseType;
	typedef TypedListView<T, ContentType> ThisType;
	
public:
	typedef ThisType* ObjectType;

	explicit TypedListView( SmartWin::Widget * parent ) : BaseType(parent) { 
		
	}
	
	void create( const typename BaseType::Seed & cs = BaseType::getDefaultSeed() ) {
		BaseType::create(cs);
		
		this->setCallback(
			SmartWin::Message( WM_NOTIFY, LVN_GETDISPINFO ), &ThisType::TypedListViewDispatcher
		);
		this->onColumnClick(std::tr1::bind(&ThisType::handleColumnClick, this, _1));
		this->onSortItems(std::tr1::bind(&ThisType::handleSort, this, _1, _2));
	}
	
	int insert(ContentType* item) {
		return insert(getSortPos(item), item);
	}
	
	int insert(int i, ContentType* item) {
		return BaseType::insert(LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE, i,
			LPSTR_TEXTCALLBACK, 0, 0, I_IMAGECALLBACK, reinterpret_cast<LPARAM>(item));
	}
	
	ContentType* getData(int iItem) {
		return reinterpret_cast<ContentType*>(BaseType::getData(iItem));
	}

	void setData(int iItem, ContentType* lparam) {
		BaseType::setData(iItem, reinterpret_cast<LPARAM>(lparam));
	}

	ContentType* getSelectedData() { return this->hasSelection() ? getData(this->getSelectedIndex()) : 0; }

	using BaseType::find;
	
	int find(ContentType* item) {
		LVFINDINFO fi = { LVFI_PARAM, NULL, (LPARAM)item };
		return ListView_FindItem(this->handle(), -1, &fi);
	}
	
	struct CompFirst {
		CompFirst() { }
		bool operator()(ContentType& a, const tstring& b) {
			return Util::stricmp(a.getText(0), b) < 0;
		}
	};
	
	void forEach(void (ContentType::*func)()) {
		unsigned n = this->size();
		for(unsigned i = 0; i < n; ++i)
			(getData(i)->*func)();
	}
	void forEachSelected(void (ContentType::*func)()) {
		int i = -1;
		while( (i = ListView_GetNextItem(this->handle(), i, LVNI_SELECTED)) != -1)
			(getData(i)->*func)();
	}
	template<class _Function>
	_Function forEachT(_Function pred) {
		unsigned n = this->size();
		for(unsigned i = 0; i < n; ++i)
			pred(getData(i));
		return pred;
	}
	template<class _Function>
	_Function forEachSelectedT(_Function pred) {
		int i = -1;
		while( (i = ListView_GetNextItem(this->handle(), i, LVNI_SELECTED)) != -1)
			pred(getData(i));
		return pred;
	}

	void update(int i) {
		unsigned k = this->getColumnCount();
		for(unsigned j = 0; j < k; ++j)
			ListView_SetItemText(this->handle(), i, j, LPSTR_TEXTCALLBACK);
	}
	
	void update(ContentType* item) { int i = find(item); if(i != -1) update(i); }

	using BaseType::erase;
	void erase(ContentType* item) { int i = find(item); if(i != -1) this->erase(i); }

	int getSortPos(ContentType* a) {
		int high = this->size();
		if((this->getSortColumn() == -1) || (high == 0))
			return high;

		high--;

		int low = 0;
		int mid = 0;
		ContentType* b = NULL;
		int comp = 0;
		while( low <= high ) {
			mid = (low + high) / 2;
			b = getData(mid);
			comp = ContentType::compareItems(a, b, this->getSortColumn());

			if(!this->isAscending())
				comp = -comp;

			if(comp == 0) {
				return mid;
			} else if(comp < 0) {
				high = mid - 1;
			} else if(comp > 0) {
				low = mid + 1;
			}
		}

		comp = ContentType::compareItems(a, b, this->getSortColumn());
		if(!this->isAscending())
			comp = -comp;
		if(comp > 0)
			mid++;

		return mid;
	}

	void setSort(int col = -1, bool ascending = true) {
		BaseType::setSort(col, BaseType::SORT_CALLBACK, ascending);
	}
private:

	int handleSort(LPARAM lhs, LPARAM rhs) {
		return ContentType::compareItems(reinterpret_cast<ContentType*>(lhs), reinterpret_cast<ContentType*>(rhs), this->getSortColumn());
	}
	
	static bool TypedListViewDispatcher(const MSG& msg, HRESULT& res) {
		NMLVDISPINFO * nm = reinterpret_cast< NMLVDISPINFO * >( msg.lParam );
		if(nm->item.mask & LVIF_TEXT) {
			ContentType* content = reinterpret_cast<ContentType*>(nm->item.lParam);
			const tstring& text = content->getText(nm->item.iSubItem);
			_tcsncpy(nm->item.pszText, text.data(), std::min(text.size(), (size_t)nm->item.cchTextMax));
			if(text.size() < static_cast<size_t>(nm->item.cchTextMax)) {
				nm->item.pszText[text.size()] = 0;
			}
		}
		if(nm->item.mask & LVIF_IMAGE) {
			ContentType* content = reinterpret_cast<ContentType*>(nm->item.lParam);
			nm->item.iImage = content->getImage();
		}
		return true;
	}
	
	void handleColumnClick(int column) {
		if(column != this->getSortColumn()) {
			this->setSort(column, true);
		} else if(this->isAscending()) {
			this->setSort(this->getSortColumn(), false);
		} else {
			this->setSort(-1, true);
		}
	}

};

#ifdef PORT_ME

template<class T, class ContentType>
class TypedListView : public T::WidgetListView,
	ListViewArrows<TypedListView<T, ctrlId> >
{

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
};
#endif

#endif // !defined(TYPED_LIST_VIEW_CTRL_H)

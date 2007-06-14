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

#ifndef DCPLUSPLUS_WIN32_TYPED_TREE_VIEW_H
#define DCPLUSPLUS_WIN32_TYPED_TREE_VIEW_H


template<class T, class ContentType>
class TypedTreeView : public T::WidgetTreeView
{
private:
	typedef typename T::WidgetTreeView BaseType;
	typedef TypedTreeView<T, ContentType> ThisType;
	
public:
	typedef ThisType* ObjectType;

	explicit TypedTreeView( SmartWin::Widget* parent ) : SmartWin::Widget(parent), BaseType(parent) { }
	
	virtual void create( const typename BaseType::Seed & cs = BaseType::getDefaultSeed() ) {
		BaseType::create(cs);
		
		typedef typename BaseType::MessageMapType MessageMapType;
		MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
		ptrThis->addNewSignal(
			typename MessageMapType::SignalTupleType(
				SmartWin::private_::SignalContent(
					SmartWin::Message( WM_NOTIFY, TVN_GETDISPINFO ),
					reinterpret_cast< SmartWin::private_::SignalContent::voidFunctionTakingVoid >( &ContentDispatcher::dispatch ),
					ptrThis
				),
				typename MessageMapType::SignalType(
					typename MessageMapType::SignalType::SlotType( &ContentDispatcher::dispatch )
				)
			)
		);
	}

	HTREEITEM insert(HTREEITEM parent, ContentType* data) {
		TVINSERTSTRUCT item = { parent, TVI_SORT, {TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_TEXT} };
		item.itemex.pszText = LPSTR_TEXTCALLBACK;
		item.itemex.iImage = I_IMAGECALLBACK;
		item.itemex.iSelectedImage = I_IMAGECALLBACK;
		item.itemex.lParam = reinterpret_cast<LPARAM>(data);
		return this->insert(&item);
	}
	
	HTREEITEM insert(TVINSERTSTRUCT* tvis) {
		return TreeView_InsertItem(this->handle(), tvis);
	}
	
	ContentType* getData(HTREEITEM item) {
		TVITEM tvitem = { TVIF_PARAM | TVIF_HANDLE };
		tvitem.hItem = item;
		if(!TreeView_GetItem(this->handle(), &tvitem)) {
			return 0;
		}
		return reinterpret_cast<ContentType*>(tvitem.lParam);
	}
	
	void getItem(TVITEMEX* item) {
		TreeView_GetItem(this->handle(), item);
	}
	
	ContentType* getSelectedData() {
		HTREEITEM item = getSelected();
		return item == NULL ? 0 : getData(item);
	}

	HTREEITEM getRoot() {
		return TreeView_GetRoot(this->handle());
	}
	
	HTREEITEM getSelected() {
		return TreeView_GetSelection(this->handle());
	}
	
	void select(HTREEITEM item) {
		TreeView_SelectItem(this->handle(), item);
	}
	
	HTREEITEM getNextSibling(HTREEITEM item) {
		return TreeView_GetNextSibling(this->handle(), item);
	}

	HTREEITEM getChild(HTREEITEM item) {
		return TreeView_GetChild(this->handle(), item);
	}
	
	HTREEITEM getParent(HTREEITEM item) {
		return TreeView_GetParent(this->handle(), item);
	}
	
	void deleteItem(HTREEITEM item) {
		TreeView_DeleteItem(this->handle(), item);
	}
	
	HTREEITEM hitTest(POINT& pt) {
		return TreeView_HitTest(this->handle(), &pt);
	}
	
	RECT getItemRect(HTREEITEM item) {
		RECT rc;
		TreeView_GetItemRect(this->handle(), item, &rc, TRUE);
		return rc;
	}
	
	POINT getContextMenuPos() {
		HTREEITEM item = getSelected();
		POINT pt = { 0 };
		if(item != NULL) {
			RECT trc = this->getItemRect(item);
			pt.x = trc.left;
			pt.y = trc.top + ((trc.bottom - trc.top) / 2);
		} 
		this->clientToScreen(pt);
		return pt;
	}
	
	void setColor(COLORREF text, COLORREF background) {
		TreeView_SetTextColor(this->handle(), text);
		TreeView_SetBkColor(this->handle(), background);
	}
private:

	struct ContentDispatcher {
		static HRESULT dispatch(SmartWin::private_::SignalContent& params) {
			NMTVDISPINFO * nm = reinterpret_cast< NMTVDISPINFO * >( params.Msg.LParam );
			if(nm->item.mask & TVIF_TEXT) {
				ContentType* content = reinterpret_cast<ContentType*>(nm->item.lParam);
				const string& text = content->getText();
				strncpy(nm->item.pszText, text.data(), std::min(text.size(), (size_t)nm->item.cchTextMax));
				if(text.size() < nm->item.cchTextMax) {
					nm->item.pszText[text.size()] = 0;
				}
			}
			if(nm->item.mask & TVIF_IMAGE) {
				ContentType* content = reinterpret_cast<ContentType*>(nm->item.lParam);
				nm->item.iImage = content->getImage();
			}
			if(nm->item.mask & TVIF_SELECTEDIMAGE) {
				ContentType* content = reinterpret_cast<ContentType*>(nm->item.lParam);
				nm->item.iSelectedImage = content->getSelectedImage();
			}
			return 0;
		}
	};

};

#endif // !defined(TYPED_TREE_VIEW_H)
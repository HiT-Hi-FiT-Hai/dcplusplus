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

#include "WinUtil.h"

template<class ContentType>
class TypedTree : public dwt::Tree
{
	typedef typename dwt::Tree BaseType;
	typedef TypedTree<ContentType> ThisType;
	
public:
	typedef ThisType* ObjectType;

	explicit TypedTree( dwt::Widget* parent ) : BaseType(parent) { }

	struct Seed : public BaseType::Seed {
		typedef ThisType WidgetType;

		Seed() : BaseType::Seed() {
			// @todo find a better way to directly use styles set in WinUtil...
			style = WinUtil::Seeds::treeView.style;
			exStyle = WinUtil::Seeds::treeView.exStyle;
			font = WinUtil::Seeds::treeView.font;
		}
	};

	void create( const typename BaseType::Seed & cs = BaseType::Seed() ) {
		BaseType::create(cs);
		this->addCallback(
			dwt::Message( WM_NOTIFY, TVN_GETDISPINFO ), &TypedTreeDispatcher
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
		return reinterpret_cast<ContentType*>(BaseType::getData(item));
	}
	
	void getItem(TVITEMEX* item) {
		TreeView_GetItem(this->handle(), item);
	}
	
	ContentType* getSelectedData() {
		HTREEITEM item = this->getSelected();
		return item == NULL ? 0 : getData(item);
	}
	
	void setItemState(HTREEITEM item, int state, int mask) {
		TreeView_SetItemState(this->handle(), item, state, mask);
	}
private:

	static bool TypedTreeDispatcher(const MSG& msg, HRESULT& res) {
		NMTVDISPINFO * nm = reinterpret_cast< NMTVDISPINFO * >( msg.lParam );
		if(nm->item.mask & TVIF_TEXT) {
			ContentType* content = reinterpret_cast<ContentType*>(nm->item.lParam);
			const tstring& text = content->getText();
			_tcsncpy(nm->item.pszText, text.data(), std::min(text.size(), (size_t)nm->item.cchTextMax));
			if(text.size() < static_cast<size_t>(nm->item.cchTextMax)) {
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

#endif // !defined(TYPED_TREE_VIEW_H)

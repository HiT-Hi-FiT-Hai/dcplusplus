/*
  DC++ Widget Toolkit

  Copyright (c) 2007-2008, Jacek Sieka

  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, 
  are permitted provided that the following conditions are met:

      * Redistributions of source code must retain the above copyright notice, 
        this list of conditions and the following disclaimer.
      * Redistributions in binary form must reproduce the above copyright notice, 
        this list of conditions and the following disclaimer in the documentation 
        and/or other materials provided with the distribution.
      * Neither the name of the DWT nor the names of its contributors 
        may be used to endorse or promote products derived from this software 
        without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND 
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <dwt/widgets/Tree.h>

namespace dwt {

Tree::Seed::Seed() :
	BaseType::Seed(WC_TREEVIEW, WS_CHILD | WS_TABSTOP),
	font(new Font(DefaultGuiFont))
{
}

void Tree::create( const Seed & cs )
{
	BaseType::create(cs);
	if(cs.font)
		setFont( cs.font );
}

HTREEITEM Tree::insert( const tstring & text, HTREEITEM parent, LPARAM param, int iconIndex, int selectedIconIndex )
{
	TVITEMEX t = { TVIF_TEXT };
	if ( param != 0 )
	{
		t.mask |= TVIF_PARAM;
		t.lParam = param;
	}
	if ( itsNormalImageList )
	{
		t.mask |= TVIF_IMAGE | TVIF_SELECTEDIMAGE;
		t.iImage = ( iconIndex == - 1 ? I_IMAGECALLBACK : iconIndex );
		t.iSelectedImage = ( selectedIconIndex == - 1 ? t.iImage : selectedIconIndex );
	}
	t.pszText = const_cast < TCHAR * >( text.c_str() );

	TVINSERTSTRUCT tv = { parent, TVI_LAST };
#ifdef WINCE
	tv.item = t;
#else
	tv.itemex = t;
#endif
	return TreeView_InsertItem(this->handle(), &tv);
}

tstring Tree::getSelectedText() {
	return getText(TreeView_GetSelection(this->handle()));
}

tstring Tree::getText( HTREEITEM node )
{
	if(node == NULL) {
		return tstring();
	}
	
	TVITEMEX item = { TVIF_HANDLE | TVIF_TEXT, node };
	TCHAR buffer[1024];
	buffer[0] = '\0';
	item.cchTextMax = 1022;
	item.pszText = buffer;
	if ( TreeView_GetItem( this->handle(), & item ) )
	{
		return buffer;
	}
	return tstring();
}

void Tree::eraseChildren( HTREEITEM node )
{
	HTREEITEM next_node, current_node;

	if ( (current_node = getNext( node, TVGN_CHILD ) ) == NULL ) 
		return;

	while ( (next_node = getNext( current_node, TVGN_NEXT )) != NULL )
	{
		erase( current_node );
		current_node = next_node;
	}

	erase( current_node );
}

void Tree::setNormalImageList( ImageListPtr imageList ) {
	  itsNormalImageList = imageList;
	  TreeView_SetImageList( this->Widget::handle(), imageList->getImageList(), TVSIL_NORMAL );
}

void Tree::setStateImageList( ImageListPtr imageList ) {
	  itsStateImageList = imageList;
	  TreeView_SetImageList( this->Widget::handle(), imageList->getImageList(), TVSIL_STATE );
}

LPARAM Tree::getDataImpl(HTREEITEM item) {
	TVITEM tvitem = { TVIF_PARAM | TVIF_HANDLE, item };
	if(!TreeView_GetItem(this->handle(), &tvitem)) {
		return 0;
	}
	return tvitem.lParam;
}

void Tree::setDataImpl(HTREEITEM item, LPARAM lParam) {
	TVITEM tvitem = { TVIF_PARAM | TVIF_HANDLE, item };
	tvitem.lParam = lParam;
	TreeView_SetItem(this->handle(), &tvitem);
}

ScreenCoordinate Tree::getContextMenuPos() {
	HTREEITEM item = getSelected();
	POINT pt = { 0 };
	if(item) {
		RECT trc = this->getItemRect(item);
		pt.x = trc.left;
		pt.y = trc.top + ((trc.bottom - trc.top) / 2);
	} 
	return ClientCoordinate(pt, this);
}

void Tree::select(const ScreenCoordinate& pt) {
	HTREEITEM ht = this->hitTest(pt);
	if(ht != NULL && ht != this->getSelected()) {
		this->setSelected(ht);
	}
}

/// Returns true if fired, else false
bool Tree::tryFire( const MSG & msg, LRESULT & retVal ) {
	bool handled = PolicyType::tryFire(msg, retVal);
	if(!handled && msg.message == WM_RBUTTONDOWN) {
		// Tree view control does strange things to rbuttondown, preventing wm_contextmenu from reaching it
		retVal = ::DefWindowProc(msg.hwnd, msg.message, msg.wParam, msg.lParam);
		return true;
	}
	return handled;
}

}

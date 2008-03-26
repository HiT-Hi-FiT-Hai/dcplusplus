#include "../../include/smartwin/widgets/WidgetTreeView.h"

namespace SmartWin {

WidgetTreeView::Seed::Seed() :
	Widget::Seed(WC_TREEVIEW, WS_CHILD | WS_VISIBLE | WS_TABSTOP),
	font(new Font(DefaultGuiFont))
{
}

void WidgetTreeView::create( const Seed & cs )
{
	ControlType::create(cs);

	if(cs.font)
		setFont( cs.font );
}

HTREEITEM WidgetTreeView::insert( const SmartUtil::tstring & text, HTREEITEM parent, LPARAM param, int iconIndex, int selectedIconIndex )
{
	TVINSERTSTRUCT tv = { 0 };
	tv.hParent = parent;
	tv.hInsertAfter = TVI_LAST;

	TVITEMEX t;
	ZeroMemory( & t, sizeof( TVITEM ) );
	t.mask = TVIF_TEXT;
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
#ifdef WINCE
	tv.item = t;
#else
	tv.itemex = t;
#endif
	return reinterpret_cast< HTREEITEM >( this->sendMessage(TVM_INSERTITEM, 0, reinterpret_cast< LPARAM >( & tv ) ) );
}

SmartUtil::tstring WidgetTreeView::getSelectedText() {
	HTREEITEM hSelItem = TreeView_GetSelection( this->handle() );
	return getText( hSelItem );
}

SmartUtil::tstring WidgetTreeView::getText( HTREEITEM node )
{
	if(node == NULL) {
		return SmartUtil::tstring();
	}
	
	TVITEMEX item;
	item.mask = TVIF_HANDLE | TVIF_TEXT;
	item.hItem = node;
	TCHAR buffer[1024];
	buffer[0] = '\0';
	item.cchTextMax = 1022;
	item.pszText = buffer;
	if ( TreeView_GetItem( this->handle(), & item ) )
	{
		return buffer;
	}
	return SmartUtil::tstring();
}

void WidgetTreeView::eraseChildren( HTREEITEM node )
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

void WidgetTreeView::setNormalImageList( ImageListPtr imageList ) {
	  itsNormalImageList = imageList;
	  TreeView_SetImageList( this->Widget::handle(), imageList->getImageList(), TVSIL_NORMAL );
}

void WidgetTreeView::setStateImageList( ImageListPtr imageList ) {
	  itsStateImageList = imageList;
	  TreeView_SetImageList( this->Widget::handle(), imageList->getImageList(), TVSIL_STATE );
}

LPARAM WidgetTreeView::getDataImpl(HTREEITEM item) {
	TVITEM tvitem = { TVIF_PARAM | TVIF_HANDLE };
	tvitem.hItem = item;
	if(!TreeView_GetItem(this->handle(), &tvitem)) {
		return 0;
	}
	return tvitem.lParam;
}

void WidgetTreeView::setDataImpl(HTREEITEM item, LPARAM lParam) {
	TVITEM tvitem = { TVIF_PARAM | TVIF_HANDLE };
	tvitem.hItem = item;
	tvitem.lParam = lParam;
	TreeView_SetItem(this->handle(), &tvitem);
}

ScreenCoordinate WidgetTreeView::getContextMenuPos() {
	HTREEITEM item = getSelected();
	POINT pt = { 0 };
	if(item) {
		RECT trc = this->getItemRect(item);
		pt.x = trc.left;
		pt.y = trc.top + ((trc.bottom - trc.top) / 2);
	} 
	return ClientCoordinate(pt, this);
}

void WidgetTreeView::select(const ScreenCoordinate& pt) {
	HTREEITEM ht = this->hitTest(pt);
	if(ht != NULL && ht != this->getSelected()) {
		this->setSelected(ht);
	}
}

/// Returns true if fired, else false
bool WidgetTreeView::tryFire( const MSG & msg, LRESULT & retVal ) {
	bool handled = PolicyType::tryFire(msg, retVal);
	if(!handled && msg.message == WM_RBUTTONDOWN) {
		// Tree view control does strange things to rbuttondown, preventing wm_contextmenu from reaching it
		retVal = ::DefWindowProc(msg.hwnd, msg.message, msg.wParam, msg.lParam);
		return true;
	}
	return handled;
}

}

#include "../../include/smartwin/widgets/WidgetTreeView.h"

namespace SmartWin {

const WidgetTreeView::Seed & WidgetTreeView::getDefaultSeed()
{
	static bool d_NeedsInit = true;
	static Seed d_DefaultValues( DontInitializeMe );

	if ( d_NeedsInit )
	{
		d_DefaultValues.className = WC_TREEVIEW;
		d_DefaultValues.style = WS_CHILD | WS_VISIBLE;
		d_DefaultValues.font = createFont( DefaultGuiFont );
		d_NeedsInit = false;
	}
	return d_DefaultValues;
}

void WidgetTreeView::create( const Seed & cs )
{
	xAssert((cs.style & WS_CHILD) == WS_CHILD, _T("Widget must have WS_CHILD style"));
	PolicyType::create(cs);

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

int WidgetTreeView::getSelectedIndex() const
{
	HTREEITEM hSelItem = TreeView_GetSelection( this->handle() );
	TVITEM item;
	ZeroMemory( & item, sizeof( TVITEM ) );
	item.mask = TVIF_HANDLE | TVIF_PARAM;
	item.hItem = hSelItem;
	if ( TreeView_GetItem( this->handle(), & item ) )
		return static_cast< unsigned >( item.lParam );
	return 0;
}

void WidgetTreeView::setSelectedIndex( int idx )
{
	TVITEM item;
	item.mask = TVIF_PARAM;
	item.lParam = idx;
	if ( TreeView_GetItem( this->handle(), & item ) == FALSE )
	{
		throw xCeption( _T( "Couldn't find given item" ) );
	}
	TreeView_Select( this->handle(), item.hItem, TVGN_FIRSTVISIBLE );
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

Point WidgetTreeView::getContextMenuPos() {
	HTREEITEM item = getSelection();
	POINT pt = { 0 };
	if(item != NULL) {
		RECT trc = this->getItemRect(item);
		pt.x = trc.left;
		pt.y = trc.top + ((trc.bottom - trc.top) / 2);
	} 
	this->clientToScreen(pt);
	return pt;
}

}

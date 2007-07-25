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

TreeViewNode WidgetTreeView::insertNode( const SmartUtil::tstring & text, const TreeViewNode & parent, unsigned param, int iconIndex, int selectedIconIndex )
{
	TVINSERTSTRUCT tv;
	ZeroMemory( & tv, sizeof( TVINSERTSTRUCT ) );
	tv.hParent = parent.handle;
	tv.hInsertAfter = TVI_LAST;

	TVITEMEX t;
	ZeroMemory( & t, sizeof( TVITEM ) );
	t.mask = TVIF_TEXT;
	if ( param != 0 )
	{
		t.mask |= TVIF_PARAM;
		t.lParam = static_cast< LPARAM >( param );
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
	TreeViewNode retVal;
	retVal.handle = reinterpret_cast< HTREEITEM >( this->sendMessage(TVM_INSERTITEM, 0, reinterpret_cast< LPARAM >( & tv ) ) );
	return retVal;
}

void WidgetTreeView::deleteChildrenOfNode( const TreeViewNode & node )
{
	TreeViewNode next_node, current_node;

	if ( ! getNode( node, TVGN_CHILD, current_node ) ) return;

	while ( getNode( current_node, TVGN_NEXT, next_node ) )
	{
		deleteNode( current_node );
		current_node = next_node;
	}

	deleteNode( current_node );
}

void WidgetTreeView::create( const Seed & cs )
{
	xAssert((cs.style & WS_CHILD) == WS_CHILD, "Widget must have WS_CHILD style");
	PolicyType::create(cs);

	setFont( cs.font );
}

}

#include "../../include/smartwin/widgets/WidgetDataGrid.h"

namespace SmartWin {

const WidgetDataGrid::Seed & WidgetDataGrid::getDefaultSeed()
{
	static bool d_NeedsInit = true;
	static Seed d_DefaultValues( DontInitializeMe );

	if ( d_NeedsInit )
	{
		d_DefaultValues.className = WC_LISTVIEW;
		d_DefaultValues.style = WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_EDITLABELS;
		d_DefaultValues.exStyle = WS_EX_CLIENTEDGE;
		//TODO: fill the values
		d_NeedsInit = false;
	}
	return d_DefaultValues;
}

void WidgetDataGrid::setSelectedIndex( int idx )
{
	// TODO: Check if this is working right...
	LVITEM it;
	ZeroMemory( & it, sizeof( LVITEM ) );
	it.iItem = idx;
	it.mask = LVIF_STATE;
	it.state = LVIS_SELECTED | LVIS_FOCUSED;
	it.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
	if ( ListView_SetItem( this->handle(), & it ) != TRUE )
	{
		xCeption err( _T( "Something went wrong while trying to set the selected property of the ListView" ) );
		throw err;
	}
	if ( ListView_EnsureVisible( this->handle(), idx, FALSE ) != TRUE )
	{
		xCeption err( _T( "Something went wrong while trying to scroll selected item into view in setSelectedIndex" ) );
		throw err;
	}
}

void WidgetDataGrid::clearSelection()
{
	LVITEM it;
	ZeroMemory( & it, sizeof( LVITEM ) );
	it.mask = LVIF_STATE;
	it.stateMask = LVIS_SELECTED;
	it.state = 0;
	std::vector< unsigned > selectedItems = getSelectedRows();

	std::vector< unsigned >::iterator iter;
	for ( iter = selectedItems.begin(); iter != selectedItems.end(); ++iter )
	{
		it.iItem = * iter;
		if ( ListView_SetItem( this->handle(), & it ) != TRUE )
		{
			xCeption err( _T( "Something went wrong while trying to unset the selected property of the ListView" ) );
			throw err;
		}
	}
}

void WidgetDataGrid::createColumns( const std::vector< SmartUtil::tstring > & colNames )
{
	// Deleting all data
	if ( itsNoColumns != 0 )
	{
		removeAllRows();
		while ( ListView_DeleteColumn( this->handle(), 0 ) == TRUE );
	}

	LV_COLUMN lvColumn =
	{0
	};
	lvColumn.mask = LVCF_WIDTH | LVCF_TEXT;
	lvColumn.cx = 100;
	int x = 0;
	for ( std::vector< SmartUtil::tstring >::const_iterator idx = colNames.begin();
		idx != colNames.end();
		++idx, ++x )
	{
		lvColumn.pszText = const_cast < TCHAR * >( idx->c_str() );
		if ( ListView_InsertColumn( this->handle(), x, & lvColumn ) == - 1 )
		{
			xCeption x( _T( "Error while trying to create Columns in list view" ) );
			throw x;
		}
	}
	itsNoColumns = static_cast< unsigned >( colNames.size() );
}

LPARAM WidgetDataGrid::insertRow(const std::vector< SmartUtil::tstring > & row, LPARAM lPar, int index, int iconIndex) {
	xAssert( itsNoColumns == row.size() && itsNoColumns != 0, _T( "Tried to add a row into a WidgetDataGridView with wrong number of columns" ) );
	if (index == - 1) {
		// Appending at bottom
		index = ListView_GetItemCount( this->handle() );
	}
	int itemCount= ListView_GetItemCount( this->handle() );
	LV_ITEM lvi =
	{	0
	};
	lvi.mask = LVIF_TEXT | LVIF_PARAM;
	if (itsNormalImageList || itsSmallImageList ) {
		lvi.mask |= LVIF_IMAGE;
		lvi.iImage = (iconIndex == - 1 ? I_IMAGECALLBACK : iconIndex );
	}
	lvi.pszText = const_cast < TCHAR * >(row[0].c_str() );
	lvi.lParam = lPar == 0 ? itemCount++ : lPar;
	lvi.cchTextMax = static_cast< int >(row[0].size() );
	lvi.iItem = index;
	if ( ListView_InsertItem( this->handle(), & lvi ) == - 1) {
		xCeption x( _T( "Error while trying to insert row in ListView" ));
		throw x;
	}
	int noCol = 1;
	lvi.mask = LVIF_TEXT;
	for (std::vector< SmartUtil::tstring >::const_iterator idx = row.begin() + 1; idx != row.end(); ++idx ) {
		lvi.iSubItem = noCol++;
		lvi.pszText = const_cast < TCHAR * >(idx->c_str() );
		lvi.cchTextMax = static_cast< int >(idx->size() );
		if ( !ListView_SetItem( this->handle(), & lvi )) {
			xCeption x( _T( "Error while trying to insert row in ListView" ));
			throw x;
		}
	}
	return lvi.lParam;
}

void WidgetDataGrid::insertRow(int index, int iconIndex) {
	if (index == - 1) {
		index = ListView_GetItemCount( this->handle() );
	}
	int itemCount= ListView_GetItemCount( this->handle() );
	LV_ITEM lvi =
	{	0
	};
	lvi.mask = LVIF_TEXT | LVIF_PARAM;
	if (itsNormalImageList || itsSmallImageList ) {
		lvi.mask |= LVIF_IMAGE;
		lvi.iImage = (iconIndex == - 1 ? I_IMAGECALLBACK : iconIndex );
	}
	lvi.pszText = _T( "" );
	lvi.lParam = itemCount++;
	lvi.cchTextMax = 0;
	lvi.iItem = index;
	if ( ListView_InsertItem( this->handle(), & lvi ) == - 1) {
		xCeption x( _T( "Error while trying to insert row in ListView" ));
		throw x;
	}
	int noCol = 1;
	lvi.mask = LVIF_TEXT;
	for (int idx = 1; idx != itsNoColumns; ++idx ) {
		lvi.iSubItem = noCol++;
		if ( !ListView_SetItem( this->handle(), & lvi )) {
			xCeption x( _T( "Error while trying to insert row in ListView" ));
			throw x;
		}
	}
}

void WidgetDataGrid::insertCallbackRow(const LPARAM lParam) {
	//assert( (itsMessageMap.find( Message( WM_NOTIFY, LVN_GETDISPINFO ) ) != itsMessageMap.end() ||
	// itsMessageMapThis.find( Message( WM_NOTIFY, LVN_GETDISPINFO ) ) != itsMessageMapThis.end() )
	// && _T("Can't insert a callback item without handling the GetItem event!") );

	// Appending at bottom
	int index= ListView_GetItemCount( this->handle() );

	LV_ITEM lvi =
	{	0
	};
	lvi.mask = LVIF_TEXT | LVIF_PARAM;
	lvi.pszText = LPSTR_TEXTCALLBACK;
	lvi.lParam = lParam;
	lvi.iItem = index;
	if (itsNormalImageList || itsSmallImageList ) {
		lvi.mask |= LVIF_IMAGE;
		lvi.iImage = I_IMAGECALLBACK;
	}
	if ( ListView_InsertItem( this->handle(), & lvi ) == - 1) {
		xCeption x( _T( "Error while trying to insert row in ListView" ));
		throw x;
	}

	lvi.mask = LVIF_TEXT;
	lvi.lParam = 0;
	for (unsigned idx = 1; idx < itsNoColumns; ++idx ) {
		lvi.iSubItem = idx;
		if ( !ListView_SetItem( this->handle(), & lvi )) {
			xCeption x( _T( "Error while trying to insert row in ListView" ));
			throw x;
		}
	}
}

int CALLBACK WidgetDataGrid::CompareFunc( LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort )
{
#ifdef PORT_ME
	WidgetDataGrid * This = reinterpret_cast< WidgetDataGrid * > ( lParamSort );
	if ( This->itsGlobalSortFunction )
	{
		return This->itsGlobalSortFunction(
			internal_::getTypedParentOrThrow < EventHandlerClass * >( This ), This, lParam1, lParam2 );
	}
	else
	{
		return ( ( * internal_::getTypedParentOrThrow < EventHandlerClass * >( This ) ).*( This->itsMemberSortFunction ) )
			( This, lParam1, lParam2 );
	}
#endif
}

void WidgetDataGrid::create( const Seed & cs )
{
	xAssert((cs.style & WS_CHILD) == WS_CHILD, "Widget must have WS_CHILD style");
	PolicyType::create(cs);
	//TODO: use CreationalInfo parameters

#ifdef WINCE
	// WinCE fix for gridlines...
	ListView_SetImageList( itsHandle, itsHImageList, LVSIL_SMALL );
#endif

	// Setting default event handler for beenValidate to a function returning "read
	// only" property of control Note! If you supply a beenValidate event handler
	// this will have no effect
#ifdef PORT_ME
	onValidate( WidgetDataGrid::defaultValidate );
#endif
}

int WidgetDataGrid::xoffFromColumn( int column, int & logicalColumn )
{
	HWND hWnd = this->handle();

	// Now we must map a absolute column to a logical column
	// Columnns can be moved but they keep their Column Number
	logicalColumn = - 1;
	HWND hHeader = reinterpret_cast< HWND >( ::SendMessage( hWnd, LVM_GETHEADER, 0, 0 ) );
	int noItems = ::SendMessage( hHeader, HDM_GETITEMCOUNT, 0, 0 );
	int * myArrayOfCols = new int[noItems]; // TODO: Use boost::scoped_array or something...
	int xOffset = 0;
	try
	{
		::SendMessage( hHeader, HDM_GETORDERARRAY, static_cast< WPARAM >( noItems ), reinterpret_cast< LPARAM >( myArrayOfCols ) );
		for ( int idx = 0; idx < noItems; ++idx )
		{
			if ( myArrayOfCols[idx] == column )
			{
				logicalColumn = idx;
				break;
			}
			else
				xOffset += ListView_GetColumnWidth( hWnd, myArrayOfCols[idx] );
		}
		delete [] myArrayOfCols;
	}
	catch ( ... )
	{
		delete [] myArrayOfCols;
		throw;
	}

	return xOffset;
}

}

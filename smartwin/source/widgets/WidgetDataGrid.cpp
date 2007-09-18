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

void WidgetDataGrid::create( const Seed & cs )
{
	xAssert((cs.style & WS_CHILD) == WS_CHILD, _T("Widget must have WS_CHILD style"));
	PolicyType::create(cs);
	//TODO: use CreationalInfo parameters

	// Setting default event handler for beenValidate to a function returning "read
	// only" property of control Note! If you supply a beenValidate event handler
	// this will have no effect
#ifdef PORT_ME
	onValidate( WidgetDataGrid::defaultValidate );
#endif
}

WidgetDataGrid::WidgetDataGrid( SmartWin::Widget * parent )
	: PolicyType( parent ),
	itsEditRow(0),
	itsEditColumn(0),
	itsXMousePosition(0),
	itsYMousePosition(0),
	isReadOnly( false ),
	itsEditingCurrently( false ),
	sortColumn(-1),
	sortType(SORT_CALLBACK),
	ascending(true)
{
	// Can't have a list view without a parent...
	xAssert( parent, _T( "Cant have a WidgetDataGrid without a parent..." ) );
}

void WidgetDataGrid::setSort(int aColumn, SortType aType, bool aAscending) {
	bool doUpdateArrow = (aColumn != sortColumn || aAscending != ascending);

	sortColumn = aColumn;
	sortType = aType;
	ascending = aAscending;

	resort();
#ifdef PORT_ME
	if (doUpdateArrow)
		updateArrow();
#endif
}

void WidgetDataGrid::setSelectedIndex( int idx )
{
	// TODO: Check if this is working right...
	LVITEM it = { LVIF_STATE };
	it.iItem = idx;
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
	LVITEM it = { LVIF_STATE };
	it.stateMask = LVIS_SELECTED;
	it.state = 0;
	std::vector< unsigned > selectedItems = getSelected();

	for ( std::vector< unsigned >::iterator iter = selectedItems.begin(); iter != selectedItems.end(); ++iter )
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
	clear();
	while ( ListView_DeleteColumn( this->handle(), 0 ) == TRUE );

	LVCOLUMN lvColumn = { LVCF_WIDTH | LVCF_TEXT };

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
}

int WidgetDataGrid::insert(const std::vector< SmartUtil::tstring > & row, LPARAM lPar, int index, int iconIndex) {
	if (index == - 1) {
		// Appending at bottom
		index = ListView_GetItemCount( this->handle() );
	}
	LVITEM lvi = { LVIF_TEXT | LVIF_PARAM };
	if (itsNormalImageList || itsSmallImageList ) {
		lvi.mask |= LVIF_IMAGE;
		lvi.iImage = (iconIndex == - 1 ? I_IMAGECALLBACK : iconIndex );
	}

	lvi.pszText = const_cast < TCHAR * >(row[0].c_str() );
	lvi.lParam = lPar;
	lvi.iItem = index;
	if ( ListView_InsertItem( this->handle(), & lvi ) == - 1) {
		xCeption x( _T( "Error while trying to insert row in ListView" ));
		throw x;
	}
	lvi.mask = LVIF_TEXT;
	lvi.iSubItem = 1;
	for (std::vector< SmartUtil::tstring >::const_iterator idx = row.begin() + 1; idx != row.end(); ++idx ) {
		lvi.pszText = const_cast < TCHAR * >(idx->c_str() );
		lvi.cchTextMax = static_cast< int >(idx->size() );
		if ( !ListView_SetItem( this->handle(), & lvi )) {
			xCeption x( _T( "Error while trying to insert row in ListView" ));
			throw x;
		}
		lvi.iSubItem++;
	}
	return index;
}

int WidgetDataGrid::insert(int mask, int i, LPCTSTR text, UINT state, UINT stateMask, int image, LPARAM lparam) {
	LVITEM item = { mask };
	item.iItem = i;
	item.state = state;
	item.stateMask = stateMask;
	item.pszText = const_cast<LPTSTR>(text);
	item.iImage = image;
	item.lParam = lparam;
	return ListView_InsertItem(this->handle(), &item);
}

Point WidgetDataGrid::getContextMenuPos() {
	int pos = getNext(-1, LVNI_SELECTED | LVNI_FOCUSED);
	POINT pt = { 0 };
	if(pos >= 0) {
		RECT lrc = this->getRect(pos, LVIR_LABEL);
		pt.x = lrc.left;
		pt.y = lrc.top + ((lrc.bottom - lrc.top) / 2);
	} 
	this->clientToScreen(pt);
	return pt;
}

SmartUtil::tstring WidgetDataGrid::getText( unsigned int row, unsigned int column )
{
	// TODO: Get string length first?
	const int BUFFER_MAX = 2048;
	TCHAR buffer[BUFFER_MAX + 1];
	buffer[0] = '\0';
	ListView_GetItemText( this->handle(), row, column, buffer, BUFFER_MAX );
	return buffer;
}

std::vector< unsigned > WidgetDataGrid::getSelected()
{
	std::vector< unsigned > retVal;
	int tmpIdx = - 1;
	while ( true )
	{
		tmpIdx = ListView_GetNextItem( this->handle(), tmpIdx, LVNI_SELECTED );
		if ( tmpIdx == - 1 )
			break;
		retVal.push_back( static_cast< unsigned >( tmpIdx ) );
	}
	return retVal;
}

unsigned WidgetDataGrid::getColumnCount() {
	HWND header = ListView_GetHeader(handle());
	return Header_GetItemCount(header);
}

void WidgetDataGrid::addRemoveListViewExtendedStyle( DWORD addStyle, bool add ) {
	DWORD newStyle = ListView_GetExtendedListViewStyle( this->handle() );
	if ( add && ( newStyle & addStyle ) != addStyle )
	{
		newStyle |= addStyle;
	}
	else if ( !add && ( newStyle & addStyle ) == addStyle )
	{
		newStyle ^= addStyle;
	}
	ListView_SetExtendedListViewStyle( this->handle(), newStyle );
}

std::vector<int> WidgetDataGrid::getColumnOrder() {
	std::vector<int> ret(this->getColumnCount());
	if(!::SendMessage(this->handle(), LVM_GETCOLUMNORDERARRAY, static_cast<WPARAM>(ret.size()), reinterpret_cast<LPARAM>(&ret[0]))) {
		ret.clear();
	}
	return ret;
}

void WidgetDataGrid::setColumnWidths(const std::vector<int>& widths) {
	for(size_t i = 0; i < widths.size(); ++i) {
		this->setColumnWidth(i, widths[i]);
	}
}

std::vector<int> WidgetDataGrid::getColumnWidths() {
	std::vector<int> ret(this->getColumnCount());
	for(size_t i = 0; i < ret.size(); ++i) {
		ret[i] = ::SendMessage(this->handle(), LVM_GETCOLUMNWIDTH, static_cast<WPARAM>(i), 0);
	}			
	return ret;
}

LPARAM WidgetDataGrid::getDataImpl(int idx) {
	LVITEM item = { LVIF_PARAM };
	item.iItem = idx;
	
	if(!ListView_GetItem(handle(), &item)) {
		return 0;
	}
	return item.lParam;
}

void WidgetDataGrid::setDataImpl(int idx, LPARAM data) {
	LVITEM item = { LVIF_PARAM };
	item.iItem = idx;
	item.lParam = data;
	
	ListView_SetItem(handle(), &item);
}

void WidgetDataGrid::setIcon( unsigned row, int newIconIndex ) {
	LVITEM it = { LVIF_IMAGE };
	it.iItem = row;
	it.iImage = newIconIndex;
	//Set item
	if(ListView_SetItem( this->handle(), &it) != TRUE) {
		xCeption err( _T( "Something went wrong while trying to change the selected item of the ListView" ) );
		throw err;
	}
}

void WidgetDataGrid::setNormalImageList( ImageListPtr imageList ) {
	  itsNormalImageList = imageList;
	  ListView_SetImageList( this->handle(), imageList->getImageList(), LVSIL_NORMAL );
}

void WidgetDataGrid::setSmallImageList( ImageListPtr imageList ) {
	  itsSmallImageList = imageList;
	  ListView_SetImageList( this->handle(), imageList->getImageList(), LVSIL_SMALL );
}

void WidgetDataGrid::setStateImageList( ImageListPtr imageList ) {
	  itsStateImageList = imageList;
	  ListView_SetImageList( this->handle(), imageList->getImageList(), LVSIL_STATE );
}

void WidgetDataGrid::setView( int view ) {
	if ( ( view & LVS_TYPEMASK ) != view )
	{
		xCeption x( _T( "Invalid View type" ) );
		throw x;
	}
	//little hack because there is no way to do this with Widget::addRemoveStyle
	int newStyle = GetWindowLong( this->handle(), GWL_STYLE );
	if ( ( newStyle & LVS_TYPEMASK ) != view )
	{
		SetWindowLong( this->handle(), GWL_STYLE, ( newStyle & ~LVS_TYPEMASK ) | view );
	}
}

void WidgetDataGrid::redraw( int firstRow, int lastRow ) {
	if(lastRow == -1) {
		lastRow = size();
	}
	if( ListView_RedrawItems( this->handle(), firstRow, lastRow ) == FALSE )
	{
		throw xCeption( _T( "Error while redrawing items in ListView" ) );
	}
}

template<typename T>
static int compare(T a, T b) {
	return (a < b) ? -1 : ((a == b) ? 0 : 1);
}

int CALLBACK WidgetDataGrid::compareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort) {
	WidgetDataGrid* p = reinterpret_cast<WidgetDataGrid*>(lParamSort);

	const int BUF_SIZE = 128;
	TCHAR buf[BUF_SIZE];
	TCHAR buf2[BUF_SIZE];

	int na = (int)lParam1;
	int nb = (int)lParam2;
	
	int result = 0;
	
	SortType type = p->sortType;
	if(type == SORT_CALLBACK) {
		result = p->fun(p->getData(na), p->getData(nb));
	} else {
		ListView_GetItemText(p->handle(), na, p->sortColumn, buf, BUF_SIZE);
		ListView_GetItemText(p->handle(), nb, p->sortColumn, buf2, BUF_SIZE);
		
		if(type == SORT_STRING) {
			result = lstrcmp(buf, buf2);
		} else if(type == SORT_STRING_NOCASE) {
			result = lstrcmpi(buf, buf2);
		} else if(type == SORT_INT) {
			result = compare(_ttoi(buf), _ttoi(buf2));
		} else if(type == SORT_FLOAT) {
			double b1, b2;
			_stscanf(buf, _T("%lf"), &b1);
			_stscanf(buf2, _T("%lf"), &b2);
			result = compare(b1, b2);
		}
	}
	if(!p->ascending)
		result = -result;
	return result;
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

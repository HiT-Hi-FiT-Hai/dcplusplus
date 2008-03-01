#include "../../include/smartwin/widgets/WidgetListView.h"

#include "../../include/smartwin/resources/Pen.h"

#include "../../include/smartwin/LibraryLoader.h"

#include <boost/scoped_array.hpp>

namespace SmartWin {

bool WidgetListView::ComCtl6 = false;

WidgetListView::Seed::Seed() : 
	Widget::Seed(WC_LISTVIEW, WS_CHILD | WS_VISIBLE | WS_TABSTOP | LVS_REPORT | LVS_EDITLABELS),
	font(new Font(DefaultGuiFont)),
	lvStyle(0)
{
	ComCtl6 = (LibraryLoader::getCommonControlsVersion() >= PACK_COMCTL_VERSION(6,00));
}

void WidgetListView::create( const Seed & cs )
{
	xAssert((cs.style & WS_CHILD) == WS_CHILD, _T("Widget must have WS_CHILD style"));
	PolicyType::create(cs);

	if(cs.font)
		setFont(cs.font);
	if(cs.lvStyle != 0)
		setListViewStyle(cs.lvStyle);

	// Setting default event handler for beenValidate to a function returning "read
	// only" property of control Note! If you supply a beenValidate event handler
	// this will have no effect
#ifdef PORT_ME
	onValidate( WidgetListView::defaultValidate );
#endif
}

WidgetListView::WidgetListView( SmartWin::Widget * parent )
	: ControlType( parent ),
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
	createArrows();
}

void WidgetListView::setSort(int aColumn, SortType aType, bool aAscending) {
	bool doUpdateArrow = (aColumn != sortColumn || aAscending != ascending);

	sortColumn = aColumn;
	sortType = aType;
	ascending = aAscending;

	resort();
	if (doUpdateArrow)
		updateArrow();
}

void WidgetListView::updateArrow() {
	if(ComCtl6) {
		int flag = isAscending() ? HDF_SORTUP : HDF_SORTDOWN;
		HWND header = ListView_GetHeader(this->handle());
		int count = Header_GetItemCount(header);
		for (int i=0; i < count; ++i)
		{
			HDITEM item;
			item.mask = HDI_FORMAT;
			Header_GetItem(header, i, &item);
			item.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
			if (i == this->getSortColumn())
				item.fmt |= flag;
			Header_SetItem(header, i, &item);
		}
		return;
	}

	if(!upArrow || !downArrow)
		return;
	
	HBITMAP bitmap = (isAscending() ? upArrow : downArrow)->handle();

	HWND header = ListView_GetHeader(this->handle());
	int count = Header_GetItemCount(header);
	for (int i=0; i < count; ++i)
	{
		HDITEM item;
		item.mask = HDI_FORMAT;
		Header_GetItem(header, i, &item);
		item.mask = HDI_FORMAT | HDI_BITMAP;
		if (i == this->getSortColumn()) {
			item.fmt |= HDF_BITMAP | HDF_BITMAP_ON_RIGHT;
			item.hbm = bitmap;
		} else {
			item.fmt &= ~(HDF_BITMAP | HDF_BITMAP_ON_RIGHT);
			item.hbm = 0;
		}
		Header_SetItem(header, i, &item);
	}
}

void WidgetListView::setSelectedIndex( int idx )
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

void WidgetListView::clearSelection()
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

void WidgetListView::createColumns( const std::vector< SmartUtil::tstring > & colNames )
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

int WidgetListView::insert(const std::vector< SmartUtil::tstring > & row, LPARAM lPar, int index, int iconIndex) {
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

int WidgetListView::insert(int mask, int i, LPCTSTR text, UINT state, UINT stateMask, int image, LPARAM lparam) {
	LVITEM item = { mask };
	item.iItem = i;
	item.state = state;
	item.stateMask = stateMask;
	item.pszText = const_cast<LPTSTR>(text);
	item.iImage = image;
	item.lParam = lparam;
	return ListView_InsertItem(this->handle(), &item);
}

ScreenCoordinate WidgetListView::getContextMenuPos() {
	int pos = getNext(-1, LVNI_SELECTED | LVNI_FOCUSED);
	POINT pt = { 0 };
	if(pos >= 0) {
		RECT lrc = this->getRect(pos, LVIR_LABEL);
		pt.x = lrc.left;
		pt.y = lrc.top + ((lrc.bottom - lrc.top) / 2);
	}
	return ClientCoordinate(pt, this);
}

SmartUtil::tstring WidgetListView::getText( unsigned int row, unsigned int column )
{
	// TODO: Get string length first?
	const int BUFFER_MAX = 2048;
	TCHAR buffer[BUFFER_MAX + 1];
	buffer[0] = '\0';
	ListView_GetItemText( this->handle(), row, column, buffer, BUFFER_MAX );
	return buffer;
}

std::vector< unsigned > WidgetListView::getSelected()
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

unsigned WidgetListView::getColumnCount() {
	HWND header = ListView_GetHeader(handle());
	return Header_GetItemCount(header);
}

void WidgetListView::addRemoveListViewExtendedStyle( DWORD addStyle, bool add ) {
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

std::vector<int> WidgetListView::getColumnOrder() {
	std::vector<int> ret(this->getColumnCount());
	if(!::SendMessage(this->handle(), LVM_GETCOLUMNORDERARRAY, static_cast<WPARAM>(ret.size()), reinterpret_cast<LPARAM>(&ret[0]))) {
		ret.clear();
	}
	return ret;
}

void WidgetListView::setColumnWidths(const std::vector<int>& widths) {
	for(size_t i = 0; i < widths.size(); ++i) {
		this->setColumnWidth(i, widths[i]);
	}
}

std::vector<int> WidgetListView::getColumnWidths() {
	std::vector<int> ret(this->getColumnCount());
	for(size_t i = 0; i < ret.size(); ++i) {
		ret[i] = ::SendMessage(this->handle(), LVM_GETCOLUMNWIDTH, static_cast<WPARAM>(i), 0);
	}			
	return ret;
}

LPARAM WidgetListView::getDataImpl(int idx) {
	LVITEM item = { LVIF_PARAM };
	item.iItem = idx;
	
	if(!ListView_GetItem(handle(), &item)) {
		return 0;
	}
	return item.lParam;
}

void WidgetListView::setDataImpl(int idx, LPARAM data) {
	LVITEM item = { LVIF_PARAM };
	item.iItem = idx;
	item.lParam = data;
	
	ListView_SetItem(handle(), &item);
}

void WidgetListView::setIcon( unsigned row, int newIconIndex ) {
	LVITEM it = { LVIF_IMAGE };
	it.iItem = row;
	it.iImage = newIconIndex;
	//Set item
	if(ListView_SetItem( this->handle(), &it) != TRUE) {
		xCeption err( _T( "Something went wrong while trying to change the selected item of the ListView" ) );
		throw err;
	}
}

void WidgetListView::setNormalImageList( ImageListPtr imageList ) {
	  itsNormalImageList = imageList;
	  ListView_SetImageList( this->handle(), imageList->getImageList(), LVSIL_NORMAL );
}

void WidgetListView::setSmallImageList( ImageListPtr imageList ) {
	  itsSmallImageList = imageList;
	  ListView_SetImageList( this->handle(), imageList->getImageList(), LVSIL_SMALL );
}

void WidgetListView::setStateImageList( ImageListPtr imageList ) {
	  itsStateImageList = imageList;
	  ListView_SetImageList( this->handle(), imageList->getImageList(), LVSIL_STATE );
}

void WidgetListView::setView( int view ) {
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

void WidgetListView::redraw( int firstRow, int lastRow ) {
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

int CALLBACK WidgetListView::compareFuncCallback(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort) {
	WidgetListView* p = reinterpret_cast<WidgetListView*>(lParamSort);
	int result = p->fun(lParam1, lParam2);
	if(!p->isAscending())
		result = -result;
	return result;
}

int CALLBACK WidgetListView::compareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort) {
	WidgetListView* p = reinterpret_cast<WidgetListView*>(lParamSort);

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

int WidgetListView::xoffFromColumn( int column, int & logicalColumn )
{
	HWND hWnd = this->handle();

	// Now we must map a absolute column to a logical column
	// Columnns can be moved but they keep their Column Number
	logicalColumn = - 1;
	HWND hHeader = reinterpret_cast< HWND >( ::SendMessage( hWnd, LVM_GETHEADER, 0, 0 ) );
	int noItems = ::SendMessage( hHeader, HDM_GETITEMCOUNT, 0, 0 );
	boost::scoped_array<int> myArrayOfCols(new int[noItems]);
	int xOffset = 0;
	::SendMessage( hHeader, HDM_GETORDERARRAY, static_cast< WPARAM >( noItems ), reinterpret_cast< LPARAM >( myArrayOfCols.get() ) );
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

	return xOffset;
}

void WidgetListView::createArrows() {
	if(ComCtl6)
		return;

	POINT triangle[3] = { {5, 0}, {0, 5}, {10, 5} };

	FreeCanvas dc(this, ::CreateCompatibleDC(NULL));

	const int bitmapWidth = 11;
	const int bitmapHeight = 6;
	const Rectangle rect(0, 0, bitmapWidth, bitmapHeight );

	Brush brush(Brush::Face3D);

	upArrow = BitmapPtr(new Bitmap(::CreateCompatibleBitmap(dc.handle(), bitmapWidth, bitmapHeight)));
	downArrow = BitmapPtr(new Bitmap(::CreateCompatibleBitmap(dc.handle(), bitmapWidth, bitmapHeight)));

	{
		// create up arrow
		Canvas::Selector select(dc, *upArrow);

		dc.fillRectangle(rect, brush);

		::InvertRgn(dc.handle(), ::CreatePolygonRgn(triangle, 3, WINDING));
	}

	{
		// create down arrow
		Canvas::Selector select(dc, *downArrow);

		dc.fillRectangle(rect, brush);

		for (size_t i = 0; i < sizeof(triangle)/sizeof(triangle[0]); ++i) {
			POINT& pt = triangle[i];
			pt.y = bitmapHeight - 1 - pt.y;
		}
		::InvertRgn(dc.handle(), ::CreatePolygonRgn(triangle, 3, WINDING));
	}
}


}

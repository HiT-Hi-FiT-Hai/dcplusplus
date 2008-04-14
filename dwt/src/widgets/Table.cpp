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

#include <dwt/widgets/Table.h>

#include <dwt/resources/Pen.h>
#include <dwt/CanvasClasses.h>
#include <dwt/LibraryLoader.h>

#include <boost/scoped_array.hpp>

namespace dwt {

bool Table::ComCtl6 = false;

Table::Seed::Seed() : 
	BaseType::Seed(WC_LISTVIEW, WS_CHILD | WS_TABSTOP | LVS_REPORT),
	font(new Font(DefaultGuiFont)),
	lvStyle(0)
{
	ComCtl6 = (LibraryLoader::getCommonControlsVersion() >= PACK_COMCTL_VERSION(6,00));
}

void Table::create( const Seed & cs )
{
	xAssert((cs.style & WS_CHILD) == WS_CHILD, _T("Widget must have WS_CHILD style"));
	PolicyType::create(cs);

	if(cs.font)
		setFont(cs.font);
	if(cs.lvStyle != 0)
		setTableStyle(cs.lvStyle);

	// Setting default event handler for beenValidate to a function returning "read
	// only" property of control Note! If you supply a beenValidate event handler
	// this will have no effect
#ifdef PORT_ME
	onValidate( Table::defaultValidate );
#endif
}

Table::Table( dwt::Widget * parent )
	: BaseType( parent ),
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

void Table::setSort(int aColumn, SortType aType, bool aAscending) {
	bool doUpdateArrow = (aColumn != sortColumn || aAscending != ascending);

	sortColumn = aColumn;
	sortType = aType;
	ascending = aAscending;

	resort();
	if (doUpdateArrow)
		updateArrow();
}

void Table::updateArrow() {
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

void Table::setSelectedImpl(int item) {
	ListView_SetItemState(handle(), item, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
}

void Table::clearSelection() {
	int i = -1;
	while((i = getNext(i, LVNI_SELECTED)) != -1) {
		ListView_SetItemState(handle(), i, 0, LVIS_SELECTED | LVIS_FOCUSED);
	}
	while((i = getNext(i, LVNI_FOCUSED)) != -1) {
		ListView_SetItemState(handle(), i, 0, LVIS_FOCUSED);
	}
}

void Table::createColumns( const std::vector< tstring > & colNames )
{
	// Deleting all data
	clear();
	while ( ListView_DeleteColumn( this->handle(), 0 ) == TRUE );

	LVCOLUMN lvColumn = { LVCF_WIDTH | LVCF_TEXT };

	lvColumn.cx = 100;
	int x = 0;
	for ( std::vector< tstring >::const_iterator idx = colNames.begin();
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

int Table::insert(const std::vector< tstring > & row, LPARAM lPar, int index, int iconIndex) {
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
		xCeption x( _T( "Error while trying to insert row in Table" ));
		throw x;
	}
	lvi.mask = LVIF_TEXT;
	lvi.iSubItem = 1;
	for (std::vector< tstring >::const_iterator idx = row.begin() + 1; idx != row.end(); ++idx ) {
		lvi.pszText = const_cast < TCHAR * >(idx->c_str() );
		lvi.cchTextMax = static_cast< int >(idx->size() );
		if ( !ListView_SetItem( this->handle(), & lvi )) {
			xCeption x( _T( "Error while trying to insert row in Table" ));
			throw x;
		}
		lvi.iSubItem++;
	}
	return index;
}

int Table::insert(int mask, int i, LPCTSTR text, UINT state, UINT stateMask, int image, LPARAM lparam) {
	LVITEM item = { mask };
	item.iItem = i;
	item.state = state;
	item.stateMask = stateMask;
	item.pszText = const_cast<LPTSTR>(text);
	item.iImage = image;
	item.lParam = lparam;
	return ListView_InsertItem(this->handle(), &item);
}

ScreenCoordinate Table::getContextMenuPos() {
	int pos = getNext(-1, LVNI_SELECTED | LVNI_FOCUSED);
	POINT pt = { 0 };
	if(pos >= 0) {
		RECT lrc = this->getRect(pos, LVIR_LABEL);
		pt.x = lrc.left;
		pt.y = lrc.top + ((lrc.bottom - lrc.top) / 2);
	}
	return ClientCoordinate(pt, this);
}

tstring Table::getText( unsigned int row, unsigned int column )
{
	// TODO: Get string length first?
	const int BUFFER_MAX = 2048;
	TCHAR buffer[BUFFER_MAX + 1];
	buffer[0] = '\0';
	ListView_GetItemText( this->handle(), row, column, buffer, BUFFER_MAX );
	return buffer;
}

std::vector< unsigned > Table::getSelection() const
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

unsigned Table::getColumnCount() {
	HWND header = ListView_GetHeader(handle());
	return Header_GetItemCount(header);
}

void Table::addRemoveTableExtendedStyle( DWORD addStyle, bool add ) {
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

std::vector<int> Table::getColumnOrder() {
	std::vector<int> ret(this->getColumnCount());
	if(!::SendMessage(this->handle(), LVM_GETCOLUMNORDERARRAY, static_cast<WPARAM>(ret.size()), reinterpret_cast<LPARAM>(&ret[0]))) {
		ret.clear();
	}
	return ret;
}

void Table::setColumnWidths(const std::vector<int>& widths) {
	for(size_t i = 0; i < widths.size(); ++i) {
		this->setColumnWidth(i, widths[i]);
	}
}

std::vector<int> Table::getColumnWidths() {
	std::vector<int> ret(this->getColumnCount());
	for(size_t i = 0; i < ret.size(); ++i) {
		ret[i] = ::SendMessage(this->handle(), LVM_GETCOLUMNWIDTH, static_cast<WPARAM>(i), 0);
	}			
	return ret;
}

LPARAM Table::getDataImpl(int idx) {
	LVITEM item = { LVIF_PARAM };
	item.iItem = idx;
	
	if(!ListView_GetItem(handle(), &item)) {
		return 0;
	}
	return item.lParam;
}

void Table::setDataImpl(int idx, LPARAM data) {
	LVITEM item = { LVIF_PARAM };
	item.iItem = idx;
	item.lParam = data;
	
	ListView_SetItem(handle(), &item);
}

void Table::setIcon( unsigned row, int newIconIndex ) {
	LVITEM it = { LVIF_IMAGE };
	it.iItem = row;
	it.iImage = newIconIndex;
	//Set item
	if(ListView_SetItem( this->handle(), &it) != TRUE) {
		xCeption err( _T( "Something went wrong while trying to change the selected item of the Table" ) );
		throw err;
	}
}

void Table::setNormalImageList( ImageListPtr imageList ) {
	  itsNormalImageList = imageList;
	  ListView_SetImageList( this->handle(), imageList->getImageList(), LVSIL_NORMAL );
}

void Table::setSmallImageList( ImageListPtr imageList ) {
	  itsSmallImageList = imageList;
	  ListView_SetImageList( this->handle(), imageList->getImageList(), LVSIL_SMALL );
}

void Table::setStateImageList( ImageListPtr imageList ) {
	  itsStateImageList = imageList;
	  ListView_SetImageList( this->handle(), imageList->getImageList(), LVSIL_STATE );
}

void Table::setView( int view ) {
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

void Table::redraw( int firstRow, int lastRow ) {
	if(lastRow == -1) {
		lastRow = size();
	}
	if( ListView_RedrawItems( this->handle(), firstRow, lastRow ) == FALSE )
	{
		throw xCeption( _T( "Error while redrawing items in Table" ) );
	}
}

template<typename T>
static int compare(T a, T b) {
	return (a < b) ? -1 : ((a == b) ? 0 : 1);
}

int CALLBACK Table::compareFuncCallback(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort) {
	Table* p = reinterpret_cast<Table*>(lParamSort);
	int result = p->fun(lParam1, lParam2);
	if(!p->isAscending())
		result = -result;
	return result;
}

int CALLBACK Table::compareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort) {
	Table* p = reinterpret_cast<Table*>(lParamSort);

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

int Table::xoffFromColumn( int column, int & logicalColumn )
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

void Table::createArrows() {
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

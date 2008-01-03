/*
  Copyright ( c ) 2005, Thomas Hansen
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification,
  are permitted provided that the following conditions are met :

	  * Redistributions of source code must retain the above copyright notice,
		this list of conditions and the following disclaimer.
	  * Redistributions in binary form must reproduce the above copyright notice,
		this list of conditions and the following disclaimer in the documentation
		and/or other materials provided with the distribution.
	  * Neither the name of the SmartWin++ nor the names of its contributors
		may be used to endorse or promote products derived from this software
		without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  ( INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION ) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  OR TORT ( INCLUDING NEGLIGENCE OR OTHERWISE ) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef WidgetListView_h
#define WidgetListView_h

#include "../Widget.h"
#include "../BasicTypes.h"
#include "../resources/ImageList.h"
#include "../aspects/AspectBorder.h"
#include "../aspects/AspectClickable.h"
#include "../aspects/AspectCollection.h"
#include "../aspects/AspectControl.h"
#include "../aspects/AspectData.h"
#include "../aspects/AspectDblClickable.h"
#include "../aspects/AspectFocus.h"
#include "../aspects/AspectFont.h"
#include "../aspects/AspectScrollable.h"
#include "../aspects/AspectSelection.h"
#include "WidgetListViewEditBox.h"

#include <vector>

namespace SmartWin
{
// begin namespace SmartWin

// Forward declaring friends
template< class WidgetType >
class WidgetCreator;

/// List View Control class
/** \ingroup WidgetControls
  * \WidgetUsageInfo
  * \image html list.PNG
  * Class for creating a List View or a "DataGrid" control Widget. <br>
  * A List View is a "datagrid" with one or more rows and one or more columns where
  * users can edit text inside each cell and respond to events. <br>
  * Note to get to actual rows there are two often interchanged ways of getting
  * there, one is the LPARAM value and the other is the physical rownumber of your
  * wanted row. Have this in mind when using the DataGrid since this is often a
  * source of error when you get unwanted behaviour. This means you often will have
  * to "map" an LPARAM value to a physical rownumber and vice versa.
  */
class WidgetListView :
	// Aspect classes
	public AspectBorder< WidgetListView >,
	public AspectClickable< WidgetListView >,
	public AspectCollection<WidgetListView, int>,
	public AspectControl<WidgetListView>,
	public AspectData<WidgetListView, int>,
	public AspectDblClickable< WidgetListView >,
	public AspectFocus< WidgetListView >,
	public AspectFont< WidgetListView >,
	public AspectScrollable< WidgetListView >,
	public AspectSelection< WidgetListView >
{
	struct HeaderDispatcher {
		typedef std::tr1::function<void (int)> F;

		HeaderDispatcher(const F& f_) : f(f_) { }

		bool operator()(const MSG& msg, LRESULT& ret) {
			LPNMLISTVIEW p = (LPNMLISTVIEW) msg.lParam; 
			f(p->iSubItem);
			return true;
		}

		F f;
	};

	// Need to be friend to access private data...
	friend class WidgetCreator< WidgetListView >;
	friend class AspectCollection<WidgetListView, int>;
	friend class AspectData<WidgetListView, int>;

public:
	typedef std::tr1::function<int (LPARAM a, LPARAM b)> SortFunction;
	
	/// Seed class
	/** This class contains all of the values needed to create the widget. It also
	  * knows the type of the class whose seed values it contains. Every widget
	  * should define one of these.
	  */
	class Seed
		: public Widget::Seed
	{
	public:
		FontPtr font;
		
		/// List view extended styles (LVS_EX_*)
		DWORD lvStyle;

		/// Fills with default parameters
		Seed();
	};
	
	enum SortType {
		SORT_CALLBACK,
 		SORT_STRING,
		SORT_STRING_NOCASE,
		SORT_INT,
		SORT_FLOAT
	};

	// Aspect expectation implementation
	static const Message & getSelectionChangedMessage();

	// Contract needed by AspectClickable Aspect class
	static const Message & getClickMessage();

	// Contract needed by AspectDblClickable Aspect class
	static const Message & getDblClickMessage();
#ifdef PORT_ME
	/// \ingroup EventHandlersWidgetListView
	/// Validation event handler setter
	/** If supplied event handler is called after a cell has been edited but before
	  * the value is actually changed. <br>
	  * If the event handler returns false, the new value is NOT inserted; if the
	  * return value is true, the old value is replaced with the new. Parameters
	  * passed is unsigned column, unsigned row and SmartUtil::tstring reference
	  * which is the new value, the value can be manipulated and changed within your
	  * event handler.
	  */
	void onValidate( typename MessageMapType::itsBoolValidationFunc eventHandler );
	void onValidate( typename MessageMapType::boolValidationFunc eventHandler );

#endif
	/// \ingroup EventHandlersWidgetListView
	/// Event handler for the SortItems event
	/** When you sort a WidgetListView you need to supply a callback function for
	  * comparing items. <br>
	  * Otherwise the grid won't know how it should sort items. <br>
	  * Some items might be compared after their integer values while other will
	  * maybe be compared after their order in the alphabet etc... <br>
	  * The event handler you supply for the SortItems event should return - 1 if the
	  * first row is supposed to be before the second and 1 if the second is supposed
	  * to be before the first, if items are equal it should return 0 <br>
	  * The two first arguments in your event handler are the LPARAM arguments
	  * supplied when inserting items for the first and second items. <br>
	  * This function will be called MANY times when you sort a grid so you probably
	  * will NOT want to run a costly operation within this event handler.
	  */
	void onSortItems( const SortFunction& f );
	
	/// \ingroup EventHandlersWidgetListView
	/// Event Handler for the Column Header Click event
	/** This Event is raised whenever one of the headers is clicked, it is useful to
	  * e.g. sort the WidgetListView according to which header is being clicked. <br>
	  * Parameters passed is int which defines which header from left to right ( zero
	  * indexed ) is being clicked!
	  */
	void onColumnClick( const HeaderDispatcher::F& f );
	
	/// Sorts the list
	/** Call this function to sort the list, it's IMPERATIVE that you before calling
	  * this function defines an event handler for the SortItems event. <br>
	  * Otherwise you will get an exception when calling this function since it
	  * expects to have a compare function to compare items with which you can define
	  * in the onSortItems Event Handler setter
	  */
	void resort();

	void setSort(int aColumn, SortType aType, bool aAscending = true);

	bool isAscending();
	
	int getSortColumn();
	
	SortType getSortType();

	/// Returns the text of the given cell
	/** The column is which column you wish to retrieve the text for. <br>
	  * The row is which row in that column you wish to retrieve the text for. <br>
	  * Note this one returns the text of the "logical" column which means that if
	  * you have moved a column ( e.g. column no 3 ) and you're trying to retrieve
	  * the text of column no. 3 it will still return the text of the OLD column no.
	  * 3 which now might be moved to the beginning of the grid, NOT the NEW column
	  * no. 3 <br>
	  * The above does NOT apply for ROWS, meaning it will get the text of the NEW
	  * row if row is moved due to a sort e.g.
	  */
	SmartUtil::tstring getText( unsigned int row, unsigned int column );

	/// Sets the text of the given cell
	/** Sets a new string value for a given cell.
	  */
	void setText( unsigned row, unsigned column, const SmartUtil::tstring & newVal );

	/// Returns true if grid has got a "current selected item"
	/** If the List View has got a "currently selected" row, this function will
	  * return true, if no row is selected it will return false.
	  */
	bool hasSelection();

	/// Returns a vector containing all the selected rows of the grid.
	/** The return vector contains unsigned integer values, each value defines a row
	  * in the grid that is selected. <br>
	  * If the grid is in "single selection mode" you should rather use the
	  * getSelectedRow function. <br>
	  * If grid does NOT have any selected items the return vector is empty.
	  */
	std::vector< unsigned > getSelected();

	/// Returns an unsigned integer which is the selected row of the grid.
	/** The return value defines the row in the grid that is selected. <br>
	  * If the grid is in "multiple selection mode" you should rather use the
	  * getSelected function. <br>
	  * If grid does NOT have a selected item the return value is - 1. <br>
	  * Note! <br>
	  * This returns the ROW of the selected item and NOT the lparam given when
	  * inserted items meaning if you sort the grid or something this function will
	  * return another number for the same item if its position has moved due to the
	  * sort etc.
	  */
	int getSelectedIndex() const;

	// Commented in AspectSelection
	void setSelectedIndex( int idx );

	/// Clears the selection of the grid.
	void clearSelection();

	/// Change the current icon of an item
	/** Sets a new icon for a given item
	  */
	void setIcon( unsigned row, int newIconIndex );

	/// Returns a boolean indicating if the Grid is in "read only" mode or not
	/** If the return value is true the Grid is in "read only" mode and cannot be
	  * updated ( except programmatically ) <br>
	  * If the return value is false the Grid is "updateable" through double clicking
	  * a cell. <br>
	  * < b >Note! <br>
	  * The "read only" property is pointless if you have defined your own validation
	  * function through calling "beenValidate"< /b >
	  */
	bool getReadOnly();

	/// Sets the "read only" property of the Grid
	/** If passed true the Grid becomes "Read Only" and you cannot change its values
	  * ( except programmatically ) <br>
	  * If passed false the Grid becomes "updateable". <br>
	  * < b >Note! <br>
	  * The "read only" property is pointless if you have defined your own validation
	  * event handler by calling "beenValidate"< /b >
	  */
	void setReadOnly( bool value = true );

	/// Returns the number of column in the grid
	/** Returns the number of columns in the Data Grid. <br>
	  * Useful if you need to know how many values you must use when inserting rows
	  * into the List View
	  */
	unsigned getColumnCount();

	/// Returns the name of a specific column
	/** Which column you wish to retrieve the name for is supplied in the "id" parameter.
	  */
	SmartUtil::tstring getColumnName( unsigned col );

	bool setColumnOrder(const std::vector<int>& columns);
	
	std::vector<int> getColumnOrder();

	void setColumnWidths(const std::vector<int>& widths);
	
	std::vector<int> getColumnWidths();	

	/// Create columns in the grid
	/** Normally this would be called just after creation of the grid, it MUST be
	  * called before adding items to the grid. <br>
	  * The vector parameter is a vector containing the column names of the columns.
	  * <br>
	  * Columns will be added the way they sequentially appear in the vector.
	  */
	void createColumns( const std::vector< SmartUtil::tstring > & colNames );

	/// Deletes the given column
	/** Column zero CANNOT be deleted.
	  */
	void eraseColumn( unsigned columnNo );

	/// Sets the width of a given column
	/** Sets the width of the given column, the columnNo parameter is a zero - based
	  * index of the column you wish to change, the width parameter is number of
	  * pixels you wish the column to be. <br>
	  * If you submit LVSCW_AUTOSIZE as the width parameter the column is being
	  * "autosized" meaning it will aquire the width needed to display the "widest"
	  * cell content in that column, if you submit LVSCW_AUTOSIZE_USEHEADER as the
	  * width parameter it will be as wide as it needs to minimum display the
	  * complete header text of the column. <br>
	  * If you use the LVSCW_AUTOSIZE_USEHEADER on the last column of the list it
	  * will be resized to be as wide as it needs to completely fill the remaining
	  * width of the list which is quite useful to make the Data Grid fill its whole
	  * client area.
	  */
	void setColumnWidth( unsigned columnNo, int width );

	/// Returns the checked state of the given row
	/** A list view can have checkboxes in each row, if the checkbox for the given
	  * row is CHECKED this funtion returns true.
	  */
	bool isChecked( unsigned row );

	/// Sets the checked state of the given row
	/** Every row in a List View can have its own checkbox column.< br >
	  * If this checkbox is selected in the queried row this function will return true.
	  */
	void setChecked( unsigned row, bool value = true );

	/// Sets (or removes) full row select.
	/** Full row select means that when a user press any place in a row the whole row
	  * will be selected instead of the activated cell. <br>
	  * value defines if we're supposed to set the fullrow select property of the
	  * grid or not. <br>
	  * If omitted, parameter defaults to true.
	  */
	void setFullRowSelect( bool value = true );

	/// Sets (or removes) the checkbox style.
	/** If you add this style your List View will get checkboxes in the leftmost
	  * column. <br>
	  * This can be useful for retrieving "selected" properties on items within the
	  * List View. <br>
	  * Note! <br>
	  * You can't set a column to be a "pure" checkbox column, but you CAN avoid to
	  * put text into the column by inserting an empty ( "" ) string.
	  */
	void setCheckBoxes( bool value = true );

	/// Sets (or removes) single row select.
	/** Single row select means that only ONE row can be selected at a time. <br>
	  * Value is parameter value defines if we're supposed to set the single row
	  * select property of the grid or not. <br>
	  * If omitted, parameter defaults to true. <br>
	  * Related functions are getSelectedRow ( single row selection mode ) or
	  * getSelected ( multiple row selection mode )
	  */
	void setSingleRowSelection( bool value = true );

	/// Adds (or removes) grid lines.
	/** A grid with grid lines will have lines surrounding every cell in it. <br>
	  * value defines if we're supposed to add grid lines or remove them. <br>
	  * If omitted, parameter defaults to true.
	  */
	void setGridLines( bool value = true );

#ifndef WINCE
	/// Adds (or removes) the hoover style.
	/** A grid with hoover style will "flash" the text color in the line the cursor
	  * is above. <br>
	  * Value is parameter value defines if we're supposed to add hoover support or
	  * remove it. <br>
	  * If omitted, parameter defaults to true.
	  */
	void setHover( bool value = true );
#endif

	/// Adds (or removes) the header drag drop style.
	/** A grid with header drag drop style will have the possibility for a user to
	  * actually click and hold a column header and "drag" that column to another
	  * place in the column hierarchy. <br>
	  * Value is parameter value defines if we're supposed to add the header drag
	  * drop style or remove it. <br>
	  * If omitted, parameter defaults to true.
	  */
	void setHeaderDragDrop( bool value = true );

	/// Adds (or removes) the always show selection style.
	/** A grid with this style set will always show its selection, even if
	  * it does not have the focus.
	  * If omitted, parameter defaults to true.
	  */
	void setAlwaysShowSelection( bool value = true );

	/// Inserts a row into the grid
	/** The row parameter is a vector containing all the cells of the row. <br>
	  * This vector must ( of course ) be the same size as the number of columns in
	  * the grid. <br>
	  * The index parameter ( optionally ) defines at which index the new row will be
	  * inserted at. <br>
	  * If omitted it defaults to - 1 ( which means at the "end" of the grid ) <br>
	  * The iconIndex parameter ( optionally ) defines the index of the icon on the
	  * image list that will be shown on the row. <br>
	  * Call createColumns before inserting items.
	  */
	int insert( const std::vector< SmartUtil::tstring > & row, LPARAM lPar = 0, int index = - 1, int iconIndex = - 1 );

	/// Reserves a number of items to the list
	/** To be used in combination with the "onGetItem" event <br>
	  * This function reserves a number of items to the list so that the control
	  * knows how many items <br>
	  * it need to give room for. <br>
	  * Has absolutely no meaning unless used in combination with the
	  * insertCallbackRow and the "onGetItem" event in which the application is
	  * queried for items when the control needs to get data to display.
	  */
	void resize( unsigned size );

	/// Set the normal image list for the Data Grid.
	/** normalImageList is the image list that contains the images
	  * for the data grid icons in Icon View (big icons).
	  */
	void setNormalImageList( ImageListPtr normalImageList );

	/// Set the small image list for the Data Grid.
	/** smallImageList is the image list that contains the images
	  * for the data grid icons in Report, List & Small Icon Views.
	  */
	void setSmallImageList( ImageListPtr smallImageList );

	/// Set the state image list for the Data Grid.
	/** stateImageList is the image list that contains the images
	  * for the data grid icons states.
	  */
	void setStateImageList( ImageListPtr stateImageList );

	/// Change the view for the Data Grid.
	/** The view parameter can be one of LVS_ICON, LVS_SMALLICON, LVS_LIST or
	  * LVS_REPORT. <br>
	  * The Data Grid uses the report view for default.
	  */
	void setView( int view );

	/// Force redraw of a range of items.
	/** You may want to call invalidateWidget after the this call to force repaint.
	  */
	void redraw( int firstRow = 0, int lastRow = -1 );

	int getSelectedCount();
	
	void setListViewStyle(int style);
	
	int insert(int mask, int i, LPCTSTR text, UINT state, UINT stateMask, int image, LPARAM lparam);
	
	int getNext(int i, int type) const;
	
    int find(const SmartUtil::tstring& b, int start = -1, bool aPartial = false);

    void select(int row);
    
	ScreenCoordinate getContextMenuPos();
	
	void ensureVisible(int i, bool partial = false);
	
	void setColor(COLORREF text, COLORREF background);

	/// Actually creates the Data Grid Control
	/** You should call WidgetFactory::createListView if you instantiate class
	  * directly. <br>
	  * Only if you DERIVE from class you should call this function directly.
	  */
	void create( const Seed & cs = Seed() );

	// Constructor Taking pointer to parent
	explicit WidgetListView( SmartWin::Widget * parent );

	static bool isValidSelectionChanged( LPARAM lPar );

protected:
	/// Adds or Removes extended list view styles from the list view
	void addRemoveListViewExtendedStyle( DWORD addStyle, bool add );

	// Protected to avoid direct instantiation, you can inherit and use
	// WidgetFactory class which is friend
	virtual ~WidgetListView() {
	}

	// Returns the rect for the item per code (wraps ListView_GetItemRect)
	Rectangle getRect( int item, int code );

	// Returns the rect for the subitem item per code (wraps ListView_GetSubItemRect)
	Rectangle getRect( int item, int subitem, int code );

private:
	// Edit row index and Edit column index, only used when grid is in "edit mode"
	int itsEditRow;
	int itsEditColumn;

	unsigned itsXMousePosition;
	unsigned itsYMousePosition;

	ImageListPtr itsNormalImageList;
	ImageListPtr itsSmallImageList;
	ImageListPtr itsStateImageList;
	
	// If true the grid is in "read only mode" meaning that cell values cannot be edited.
	// A simpler version of defining a beenValidate always returning false
	bool isReadOnly;
	bool itsEditingCurrently; // Inbetween BEGIN and END EDIT

	int sortColumn;
	SortType sortType;
	bool ascending;
	SortFunction fun;

	static bool ComCtl6; // Using Common Controls version >= 6.00
	BitmapPtr upArrow;
	BitmapPtr downArrow;

	static int CALLBACK compareFunc( LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort );
	static int CALLBACK compareFuncCallback( LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort );
	
	void createArrows();
	void updateArrow();
#ifdef PORT_ME
	// Private validate function, this ones returns the "read only" property of the list
	static bool defaultValidate( EventHandlerClass * parent, WidgetListView * list, unsigned int col, unsigned int row, SmartUtil::tstring & newValue );
#endif
	// Calculates the adjustment from the columns of an item.
	int xoffFromColumn( int column, int & logicalColumn );

	// AspectData
	int findDataImpl(LPARAM data, int start = -1);
	LPARAM getDataImpl(int idx);
	void setDataImpl(int i, LPARAM data);
	
	// AspectCollection
	void eraseImpl( int row );
	void clearImpl();
	size_t sizeImpl() const;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline const Message & WidgetListView::getSelectionChangedMessage()
{
	static const Message retVal( WM_NOTIFY, LVN_ITEMCHANGED ); // TODO: Implement LVN_ITEMCHANGING Event Handlers (return bool to indicate allowance)
	return retVal;
}

inline bool WidgetListView::isValidSelectionChanged( LPARAM lPar )
{
	//TODO: Make support for CHOOSING how onSelectedChanged is supposed to behave,
	//TODO: make non static function and pure abstract in base class and override
	//TODO: and add enum param to event handler setter which MUST have default
	//TODO: value to be backwards compatible
	NMLISTVIEW * tmp = reinterpret_cast< NMLISTVIEW * >( lPar );
	if ( ( tmp->uChanged & LVIF_STATE ) == LVIF_STATE &&
		( tmp->uNewState & LVIS_SELECTED ) == LVIS_SELECTED &&
		( tmp->uOldState & LVIS_SELECTED ) != LVIS_SELECTED &&
		( tmp->uOldState & LVIS_FOCUSED ) != LVIS_FOCUSED )
		return true;
	return false;
}

inline const Message & WidgetListView::getClickMessage()
{
	static const Message retVal( WM_NOTIFY, NM_CLICK );
	return retVal;
}

inline const Message & WidgetListView::getDblClickMessage()
{
	static const Message retVal( WM_NOTIFY, NM_DBLCLK );
	return retVal;
}

#ifdef PORT_ME

void WidgetListView::onValidate( typename MessageMapControl< EventHandlerClass, WidgetListView >::itsBoolValidationFunc eventHandler )
{
	if ( this->getReadOnly() )
		this->setReadOnly( false );
	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->setCallback(
		typename MessageMapType::SignalTupleType(
			private_::SignalContent(
				Message( WM_NOTIFY, LVN_ENDLABELEDIT ),
				reinterpret_cast< itsVoidFunction >( eventHandler ),
				ptrThis
			),
			typename MessageMapType::SignalType(
				typename MessageMapType::SignalType::SlotType( & DispatcherList::dispatchBoolIntIntStringThis )
			)
		)
	);
}

void WidgetListView::onValidate( typename MessageMapControl< EventHandlerClass, WidgetListView >::boolValidationFunc eventHandler )
{
	if ( this->getReadOnly() )
		this->setReadOnly( false );
	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->setCallback(
		typename MessageMapType::SignalTupleType(
			private_::SignalContent(
				Message( WM_NOTIFY, LVN_ENDLABELEDIT ),
				reinterpret_cast< private_::SignalContent::voidFunctionTakingVoid >( eventHandler ),
				ptrThis
			),
			typename MessageMapType::SignalType(
				typename MessageMapType::SignalType::SlotType( & DispatcherList::dispatchBoolIntIntString )
			)
		)
	);
}


void WidgetListView::onCustomPainting( typename MessageMapControl< EventHandlerClass, WidgetListView >::itsVoidUnsignedUnsignedBoolCanvasRectangle eventHandler )
{
	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->setCallback(
		typename MessageMapType::SignalTupleType(
			private_::SignalContent(
				Message( WM_NOTIFY, NM_CUSTOMDRAW ),
				reinterpret_cast< itsVoidFunction >( eventHandler ),
				ptrThis
			),
			typename MessageMapType::SignalType(
				typename MessageMapType::SignalType::SlotType( & DispatcherList::dispatchDrawItemThis )
			)
		)
	);
}


	void WidgetListView::onCustomPainting( typename MessageMapControl< EventHandlerClass, WidgetListView >::voidUnsignedUnsignedBoolCanvasRectangle eventHandler )
{
	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->setCallback(
		typename MessageMapType::SignalTupleType(
			private_::SignalContent(
				Message( WM_NOTIFY, NM_CUSTOMDRAW ),
				reinterpret_cast< private_::SignalContent::voidFunctionTakingVoid >( eventHandler ),
				ptrThis
			),
			typename MessageMapType::SignalType(
				typename MessageMapType::SignalType::SlotType( & DispatcherList::dispatchDrawItem )
			)
		)
	);
}
#endif

inline void WidgetListView::onColumnClick( const HeaderDispatcher::F& f ) {
	this->setCallback(
		Message( WM_NOTIFY, LVN_COLUMNCLICK ), HeaderDispatcher(f)
	);
}

inline void WidgetListView::resort() {
	if(sortColumn != -1) {
		if(sortType == SORT_CALLBACK) {
			ListView_SortItems(this->handle(), &WidgetListView::compareFuncCallback, reinterpret_cast<LPARAM>(this));
		} else {
			// Wine 0.9.48 doesn't support this
			ListView_SortItemsEx(this->handle(), &WidgetListView::compareFunc, reinterpret_cast< LPARAM >(this));
		}
	}
}

inline bool WidgetListView::hasSelection() {
	return ListView_GetSelectedCount( this->handle() ) > 0;
}

inline int WidgetListView::getSelectedIndex() const {
	return getNext(-1, LVNI_SELECTED);
}

inline void WidgetListView::setText( unsigned row, unsigned column, const SmartUtil::tstring & newVal ) {
	ListView_SetItemText( this->handle(), row, column, const_cast < TCHAR * >( newVal.c_str() ) );
}

inline bool WidgetListView::getReadOnly() {
	return isReadOnly;
}

inline void WidgetListView::setReadOnly( bool value ) {
	isReadOnly = value;
	this->Widget::addRemoveStyle( LVS_EDITLABELS, !value );
}

inline SmartUtil::tstring WidgetListView::getColumnName( unsigned col ) {
	// TODO: Fix
	const int BUFFER_MAX = 2048;
	TCHAR buffer[BUFFER_MAX + 1];
	LV_COLUMN colInfo;
	colInfo.mask = LVCF_TEXT;
	colInfo.cchTextMax = BUFFER_MAX;
	colInfo.pszText = buffer;
	ListView_GetColumn( this->handle(), col, & colInfo );
	return colInfo.pszText;
}

inline bool WidgetListView::isChecked( unsigned row ) {
	return ListView_GetCheckState( this->handle(), row ) == TRUE;
}

inline void WidgetListView::setChecked( unsigned row, bool value ) {
	ListView_SetCheckState( this->handle(), row, value );
}

inline void WidgetListView::setFullRowSelect( bool value ) {
	addRemoveListViewExtendedStyle( LVS_EX_FULLROWSELECT, value );
}

inline void WidgetListView::resize( unsigned size ) {
	ListView_SetItemCount( this->handle(), size );
}

inline void WidgetListView::setCheckBoxes( bool value ) {
	addRemoveListViewExtendedStyle( LVS_EX_CHECKBOXES, value );
}

inline void WidgetListView::setSingleRowSelection( bool value ) {
	this->Widget::addRemoveStyle( LVS_SINGLESEL, value );
}

inline void WidgetListView::setGridLines( bool value ) {
	addRemoveListViewExtendedStyle( LVS_EX_GRIDLINES, value );
}

inline void WidgetListView::onSortItems(const SortFunction& f) {
	fun = f;
}

#ifndef WINCE

inline void WidgetListView::setHover( bool value ) {
	addRemoveListViewExtendedStyle( LVS_EX_TWOCLICKACTIVATE, value );
}
#endif

inline void WidgetListView::setHeaderDragDrop( bool value ) {
	addRemoveListViewExtendedStyle( LVS_EX_HEADERDRAGDROP, value );
}

inline void WidgetListView::setAlwaysShowSelection( bool value ) {
	this->Widget::addRemoveStyle( LVS_SHOWSELALWAYS, value );
}

inline void WidgetListView::eraseColumn( unsigned columnNo ) {
	xAssert( columnNo != 0, _T( "Can't delete the leftmost column" ) );
	ListView_DeleteColumn( this->handle(), columnNo );
}

inline void WidgetListView::setColumnWidth( unsigned columnNo, int width ) {
	if ( ListView_SetColumnWidth( this->handle(), columnNo, width ) == FALSE )
	{
		xCeption x( _T( "Couldn't resize columns of WidgetListView" ) );
		throw x;
	}
}

inline void WidgetListView::clearImpl() {
	ListView_DeleteAllItems( this->handle() );
}

inline void WidgetListView::eraseImpl( int row ) {
	ListView_DeleteItem( this->handle(), row );
}

inline size_t WidgetListView::sizeImpl() const {
	return ListView_GetItemCount( this->handle() );
}

#ifdef PORT_ME
bool WidgetListView::defaultValidate( EventHandlerClass * parent, WidgetListView * list, unsigned int col, unsigned int row, SmartUtil::tstring & newValue )
{
	list->updateWidget();
	return !list->getReadOnly();
}
#endif
// Calculates the adjustment from the columns of an item.

inline Rectangle WidgetListView::getRect( int item, int code )
{
	RECT r;
	ListView_GetItemRect( this->handle(), item, &r, code );
	return r;
}

inline Rectangle WidgetListView::getRect( int item, int subitem, int code )
{
	RECT r;
	ListView_GetSubItemRect( this->handle(), item, subitem, code, &r );
	return r;
}

inline bool WidgetListView::isAscending() { 
	return ascending; 
}

inline int WidgetListView::getSortColumn() { 
	return sortColumn; 
}

inline WidgetListView::SortType WidgetListView::getSortType() { 
	return sortType; 
}

inline bool WidgetListView::setColumnOrder(const std::vector<int>& columns) {
	return ::SendMessage(this->handle(), LVM_SETCOLUMNORDERARRAY, static_cast<WPARAM>(columns.size()), reinterpret_cast<LPARAM>(&columns[0])) > 0;
}

inline int WidgetListView::getSelectedCount() {
	return ListView_GetSelectedCount(this->handle());
}

inline void WidgetListView::setListViewStyle(int style) {
	ListView_SetExtendedListViewStyle(this->handle(), style);
}

inline int WidgetListView::getNext(int i, int type) const {
	return ListView_GetNextItem(this->handle(), i, type);
}

inline int WidgetListView::find(const SmartUtil::tstring& b, int start, bool aPartial) {
    LVFINDINFO fi = { aPartial ? LVFI_PARTIAL : LVFI_STRING, b.c_str() };
    return ListView_FindItem(this->handle(), start, &fi);
}

inline int WidgetListView::findDataImpl(LPARAM data, int start) {
    LVFINDINFO fi = { LVFI_PARAM, NULL, data };
    return ListView_FindItem(this->handle(), start, &fi);
}

inline void WidgetListView::select(int i) {
	ListView_SetItemState(this->handle(), i, LVIS_SELECTED, LVIS_SELECTED);
}

inline void WidgetListView::ensureVisible(int i, bool partial) {
	ListView_EnsureVisible(this->handle(), i, false);
}

inline void WidgetListView::setColor(COLORREF text, COLORREF background) {
	ListView_SetTextColor(this->handle(), text);
	ListView_SetTextBkColor(this->handle(), background);
	ListView_SetBkColor(this->handle(), background);
}

#ifdef PORT_ME
// TODO: Should these

LRESULT WidgetListView::sendWidgetMessage( HWND hWnd, UINT msg, WPARAM & wPar, LPARAM & lPar )
{
	switch ( msg )
	{
	// WinCE can't retrieve mouse pos in Double Click message handler since mouse
	// isn't around... therefore we need to "store" it like we do here...!!
	case WM_LBUTTONDOWN :
		{
			// TODO: Dispatch events
			itsXMousePosition = LOWORD( lPar );
			itsYMousePosition = HIWORD( lPar );
		} break;

	case WM_PAINT :
		if ( itsEditingCurrently )
		{
			// In order to prevent a mistaken update of the first column which leads
			// to a blank cell while editing an inner cell's contents, we
			// validate this area and thus prevent its repaint.
			// Don't need to update to the left or right of the editing rect.
			RECT editRect = getSubItemRect( itsEditRow, itsEditColumn, LVIR_LABEL );
			RECT rowRect = getItemRect( itsEditRow, LVIR_LABEL );
			RECT validRect;
			if ( editRect.left > rowRect.left )
			{
				validRect = rowRect; validRect.right = editRect.left;
				ValidateRect( this->handle(), & validRect );
			}
			ValidateRect( this->handle(), & rowRect );
		}
		break;

	// TODO: Dispatch events
	case WM_NOTIFY :
		{
			NMHDR * na = reinterpret_cast< NMHDR * >( lPar );
			switch ( na->code )
			{
			case LVN_BEGINLABELEDIT :
				{
					itsEditingCurrently = true;
					int logicalColumn;
					int xOffset = xoffFromColumn( itsEditColumn, logicalColumn );

					// NOW we have found our "sub" label!
					RECT r = getItemRect( itsEditRow, LVIR_BOUNDS );
					RECT rs = getSubItemRect( itsEditRow, itsEditColumn, LVIR_BOUNDS );

#ifndef WINCE // WinCE doesn't support repositioning the edit control anyway...
					// Checking to see if we need to scroll
					RECT cr;
					::GetClientRect( this->handle(), & cr );
					if ( xOffset + r.left < 0 || xOffset + r.left > cr.right )
					{
						int x = xOffset - r.left;
						ListView_Scroll( this->handle(), x, 0 );
						r.left -= x;
					}
					// Get column alignment
					LV_COLUMN lv =
					{0
					};
					lv.mask = LVCF_FMT;
					ListView_GetColumn( this->handle(), logicalColumn, & lv );
					DWORD dwStyle;
					if ( ( lv.fmt & LVCFMT_JUSTIFYMASK ) == LVCFMT_LEFT )
						dwStyle = ES_LEFT;
					else if ( ( lv.fmt & LVCFMT_JUSTIFYMASK ) == LVCFMT_RIGHT )
						dwStyle = ES_RIGHT;
					else
						dwStyle = ES_CENTER;
					r.left += xOffset + 4;
					r.right = r.left + ( ListView_GetColumnWidth( this->handle(), itsEditColumn ) - 3 );
					if ( r.right > cr.right )
						r.right = cr.right;
					dwStyle |= WS_BORDER | WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL;
#endif

					// Creating text Widget and placing it above cell
					HWND editControl = ListView_GetEditControl( this->handle() );
					if ( editControl == 0 )
					{
						xCeption err( _T( "Couldn't attach to List View editcontrol" ) );
						throw err;
					}

					private_::ListViewEditBox< WidgetListView > * text
						= new private_::ListViewEditBox< WidgetListView >( this );
					text->createSubclass( editControl );

#ifndef WINCE
					text->itsRect = SmartWin::Rectangle( r.left, r.top, r.right, r.bottom );
					text->setBounds( text->itsRect );
					text->setScrollBarHorizontally( false );
					text->setScrollBarVertically( false );
					::SetWindowLong( editControl, GWL_STYLE, dwStyle );
#endif
					// Getting text of cell and inserting into text Widget
					const int BUFFER_MAX = 260; // List view cells have a maximum of 260 characters
					TCHAR buffer[BUFFER_MAX];
					ListView_GetItemText( this->handle(), itsEditRow, itsEditColumn, buffer, BUFFER_MAX );
					text->setText( buffer );

					// Select all end give focus
					text->setSelection();
					text->setFocus();
					return FALSE;
				} break;

			case LVN_ENDLABELEDIT :
				{ itsEditingCurrently = false;
					break;
				}

			case NM_CLICK :
				{
					//TODO: only works with comctl v4.71, is this OK?
#ifdef WINCE
					LPNMLISTVIEW item = (LPNMLISTVIEW) lPar;
#else
					LPNMITEMACTIVATE item = (LPNMITEMACTIVATE) lPar;
#endif
					itsXMousePosition = item->ptAction.x;
					itsYMousePosition = item->ptAction.y;
				} break;

			// TODO: Dispatch events
			case NM_DBLCLK :
				{
					LVHITTESTINFO info;
					info.pt.x = itsXMousePosition;
					info.pt.y = itsYMousePosition;
					ListView_SubItemHitTest( this->handle(), & info );
					// User has clicked the ListView
					if ( info.iItem != - 1 )
					{
						// User has clicked an ITEM in ListView
						if ( info.iSubItem >= 0 )
						{
							UINT state = ListView_GetItemState( this->handle(), info.iItem, LVIS_FOCUSED );
							if ( ! ( state & LVIS_FOCUSED ) )
							{
								//SetFocus( itsHandle );   // TODO: This was catched by devcpp ... what was intended?
								SetFocus( this->handle() ); // ASW add
							}

							// Check to verify items are editable
							if ( ::GetWindowLong( this->handle(), GWL_STYLE ) & LVS_EDITLABELS )
							{
								itsEditRow = info.iItem;
								itsEditColumn = info.iSubItem;
								ListView_EditLabel( this->handle(), info.iItem );
								return 0; // Processed
							}
						}
					}
				} break;
			default:
				return MessageMapType::sendWidgetMessage( hWnd, msg, wPar, lPar );
			}
		} break;
	}
	return MessageMapType::sendWidgetMessage( hWnd, msg, wPar, lPar );
}
#endif
// end namespace SmartWin
}

#endif

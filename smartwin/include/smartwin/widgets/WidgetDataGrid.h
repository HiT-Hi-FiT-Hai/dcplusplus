// $Revision: 1.38 $
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
#ifndef WidgetDataGrid_h
#define WidgetDataGrid_h

#include "../WindowsHeaders.h"
#include "../MessageMap.h"
#include "../MessageMapControl.h"
#include "../TrueWindow.h"
#include "../aspects/AspectKeyPressed.h"
#include "../aspects/AspectSizable.h"
#include "../aspects/AspectSelection.h"
#include "../aspects/AspectClickable.h"
#include "../aspects/AspectDblClickable.h"
#include "../aspects/AspectRightClickable.h"
#include "../aspects/AspectFont.h"
#include "../aspects/AspectVisible.h"
#include "../aspects/AspectEnabled.h"
#include "../aspects/AspectFocus.h"
#include "../aspects/AspectGetParent.h"
#include "../aspects/AspectRaw.h"
#include "../aspects/AspectMouseClicks.h"
#include "../aspects/AspectScrollable.h"
#include "../aspects/AspectBorder.h"
#include "../xCeption.h"
#include "SmartUtil.h"
#include "WidgetDataGridEditBox.h"
#include "../ImageList.h"

namespace SmartWin
{
// begin namespace SmartWin

// Forward declaring friends
template< class WidgetType >
class WidgetCreator;

template< class EventHandlerClass, class WidgetType, class MessageMapType >
class AspectListDispatcher
{
public:
	static HRESULT dispatchLparamIntIntString( private_::SignalContent & params )
	{
		NMLVDISPINFO * nm = reinterpret_cast< NMLVDISPINFO * >( params.Msg.LParam );
		if ( nm->item.mask & LVIF_TEXT )
		{
			SmartUtil::tstring insertionString;

			typename MessageMapType::voidGetItemFunc func
				= reinterpret_cast< typename MessageMapType::voidGetItemFunc >( params.Function );

			func
				( internal_::getTypedParentOrThrow < EventHandlerClass * >( params.This )
				, boost::polymorphic_cast< WidgetType * >( params.This )
				, nm->item.lParam
				, nm->item.iSubItem
				, nm->item.iItem
				, insertionString
				);
#if _MSC_VER >= 1400 // Whidbey supports "safe string copy"
			_tcsncpy_s( nm->item.pszText, nm->item.cchTextMax, insertionString.c_str(), insertionString.size() + sizeof( TCHAR ) );
#else
			_tcsncpy( nm->item.pszText, insertionString.c_str(), insertionString.size() + sizeof( TCHAR ) );
#endif
		}
			if ( nm->item.mask & LVIF_IMAGE )
			{
				nm->item.iImage = dispatchGetIcon( params, nm->item.lParam, nm->item.iItem );
			}
		return 0;
	}

	static HRESULT dispatchLparamIntIntStringThis( private_::SignalContent & params )
	{
		NMLVDISPINFO * nm = reinterpret_cast< NMLVDISPINFO * >( params.Msg.LParam );
		if ( nm->item.mask & LVIF_TEXT )
		{
			SmartUtil::tstring insertionString;
			typename MessageMapType::itsVoidGetItemFunc func
				= reinterpret_cast< typename MessageMapType::itsVoidGetItemFunc >( params.FunctionThis );
			( ( * internal_::getTypedParentOrThrow < EventHandlerClass * >( params.This ) ).*func )
				( boost::polymorphic_cast< WidgetType * >( params.This )
				, nm->item.lParam
				, nm->item.iSubItem
				, nm->item.iItem
				, insertionString
				);
#if _MSC_VER >= 1400 // Whidbey supports "safe string copy"
			_tcsncpy_s( nm->item.pszText, nm->item.cchTextMax, insertionString.c_str(), insertionString.size() + sizeof( TCHAR ) );
#else
			_tcsncpy( nm->item.pszText, insertionString.c_str(), insertionString.size() + sizeof( TCHAR ) );
#endif
			}
			if ( nm->item.mask & LVIF_IMAGE )
			{
				nm->item.iImage = dispatchGetIcon( params, nm->item.lParam, nm->item.iItem );
		}
		return 0;
	}

	static HRESULT dispatchDrawItem( private_::SignalContent & params )
	{
		NMLVCUSTOMDRAW * nmLVCustomDraw = reinterpret_cast< NMLVCUSTOMDRAW * >( params.Msg.LParam );

		if ( nmLVCustomDraw->nmcd.dwDrawStage == CDDS_PREPAINT || nmLVCustomDraw->nmcd.dwDrawStage == CDDS_ITEMPREPAINT )
			return CDRF_NOTIFYSUBITEMDRAW;
		else if ( nmLVCustomDraw->nmcd.dwDrawStage == ( CDDS_ITEMPREPAINT | CDDS_SUBITEM ) )
		{
			int subItem = nmLVCustomDraw->iSubItem;
			int lParam = nmLVCustomDraw->nmcd.lItemlParam;

			RECT rc;
			LV_FINDINFO find;
			find.flags = LVFI_PARAM;
			find.lParam = lParam;
			int idxOfItem = ListView_FindItem( params.This->handle(), - 1, & find );
			ListView_GetSubItemRect( params.This->handle(), idxOfItem, subItem, LVIR_BOUNDS, & rc );

			typename MessageMapType::itsVoidUnsignedUnsignedBoolCanvasRectangle func
				= reinterpret_cast< typename MessageMapType::itsVoidUnsignedUnsignedBoolCanvasRectangle >( params.FunctionThis );
			FreeCanvas canvas( params.This->handle(), nmLVCustomDraw->nmcd.hdc );
			SmartWin::Rectangle rect
				( rc.left
				, rc.top
				, rc.right - rc.left
				, rc.bottom - rc.top
				);
			rect = rect.shrink( 2, 0 );
			if ( subItem == 0 )
				rect.size.x = ListView_GetColumnWidth( params.This->handle(), subItem );
			func
				( internal_::getTypedParentOrThrow < WidgetType * >( params.This )
				, lParam
				, subItem
				, ( nmLVCustomDraw->nmcd.uItemState & CDIS_SELECTED ) == CDIS_SELECTED && ( nmLVCustomDraw->nmcd.uItemState & ODS_FOCUS ) == ODS_FOCUS
				, canvas
				, rect
				);
		}
		return CDRF_SKIPDEFAULT;
	}

	static HRESULT dispatchDrawItemThis( private_::SignalContent & params )
	{
		NMLVCUSTOMDRAW * nmLVCustomDraw = reinterpret_cast< NMLVCUSTOMDRAW * >( params.Msg.LParam );

		if ( nmLVCustomDraw->nmcd.dwDrawStage == CDDS_PREPAINT || nmLVCustomDraw->nmcd.dwDrawStage == CDDS_ITEMPREPAINT )
			return CDRF_NOTIFYSUBITEMDRAW;
		else if ( nmLVCustomDraw->nmcd.dwDrawStage == ( CDDS_ITEMPREPAINT | CDDS_SUBITEM ) )
		{
			int subItem = nmLVCustomDraw->iSubItem;
			int lParam = nmLVCustomDraw->nmcd.lItemlParam;

			RECT rc; // = nmLVCustomDraw->nmcd.rc;
			LV_FINDINFO find;
			find.flags = LVFI_PARAM;
			find.lParam = lParam;
			int idxOfItem = ListView_FindItem( params.This->handle(), - 1, & find );
			ListView_GetSubItemRect( params.This->handle(), idxOfItem, subItem, LVIR_BOUNDS, & rc );

			typename MessageMapType::itsVoidUnsignedUnsignedBoolCanvasRectangle func
				= reinterpret_cast< typename MessageMapType::itsVoidUnsignedUnsignedBoolCanvasRectangle >( params.FunctionThis );
			FreeCanvas canvas( params.This->handle(), nmLVCustomDraw->nmcd.hdc );
			SmartWin::Rectangle rect
				( rc.left
				, rc.top
				, rc.right - rc.left
				, rc.bottom - rc.top
				);
			rect = rect.shrink( 2, 0 );
			if ( subItem == 0 )
				rect.size.x = ListView_GetColumnWidth( params.This->handle(), subItem );
			( ( * internal_::getTypedParentOrThrow < EventHandlerClass * >( params.This ) ).*func )
				( boost::polymorphic_cast< WidgetType * >( params.This )
				, lParam
				, subItem
				, ( nmLVCustomDraw->nmcd.uItemState & CDIS_SELECTED ) == CDIS_SELECTED && ( nmLVCustomDraw->nmcd.uItemState & ODS_FOCUS ) == ODS_FOCUS
				, canvas
				, rect
				);
		}
		return CDRF_SKIPDEFAULT;
	}

	static HRESULT dispatchColumnClick( private_::SignalContent & params )
	{
		NMLISTVIEW * nm = reinterpret_cast< NMLISTVIEW * >( params.Msg.LParam );
		typename MessageMapType::voidFunctionTakingInt func
			= reinterpret_cast< typename MessageMapType::voidFunctionTakingInt >( params.Function );

		func
			( internal_::getTypedParentOrThrow < EventHandlerClass * >( params.This )
			, boost::polymorphic_cast< WidgetType * >( params.This )
			, nm->iSubItem
			);
		return 0;
	}

	static HRESULT dispatchColumnClickThis( private_::SignalContent & params )
	{
		NMLISTVIEW * nm = reinterpret_cast< NMLISTVIEW * >( params.Msg.LParam );
		typename MessageMapType::itsVoidFunctionTakingInt func
			= reinterpret_cast< typename MessageMapType::itsVoidFunctionTakingInt >( params.FunctionThis );
		( ( * internal_::getTypedParentOrThrow < EventHandlerClass * >( params.This ) ).*func )
			( boost::polymorphic_cast< WidgetType * >( params.This )
			, nm->iSubItem
			);
		return 0;
	}

	static HRESULT dispatchBoolIntIntString( private_::SignalContent & params )
	{
		// TODO: Check if value has actually changed before initiating the event
		NMLVDISPINFO * nm = reinterpret_cast< NMLVDISPINFO * >( params.Msg.LParam );
		if ( nm->item.pszText == NULL )
			return FALSE;
		else
		{
			WidgetType * This = boost::polymorphic_cast< WidgetType * >( params.This );
			SmartUtil::tstring updateNewVal = nm->item.pszText;

			typename MessageMapType::boolValidationFunc func =
				reinterpret_cast< typename MessageMapType::boolValidationFunc >( ( void( * )() ) params.Function );

			bool update =
			func
				( internal_::getTypedParentOrThrow < EventHandlerClass * >( params.This )
				, boost::polymorphic_cast< WidgetType * >( params.This )
				, This->itsEditColumn
				, This->itsEditRow
				, updateNewVal
				);

			if ( update )
			{
				// Faking a fresh on the item/subItem
				ListView_SetItemText
					( This->handle()
					, This->itsEditRow
					, This->itsEditColumn
					, const_cast < TCHAR * >( updateNewVal.c_str() )
					);
			}
			return 0;
		}
	}

	static HRESULT dispatchBoolIntIntStringThis( private_::SignalContent & params )
	{
		// TODO: Check if value has actually changed before initiating the event
		NMLVDISPINFO * nm = reinterpret_cast< NMLVDISPINFO * >( params.Msg.LParam );
		if ( nm->item.pszText == NULL )
			return FALSE;
		else
		{
			WidgetType * This = boost::polymorphic_cast< WidgetType * >( params.This );
			SmartUtil::tstring updateNewVal = nm->item.pszText;

			typename MessageMapType::itsBoolValidationFunc func =
				reinterpret_cast< typename MessageMapType::itsBoolValidationFunc >( params.FunctionThis );

			bool update =
			( ( * internal_::getTypedParentOrThrow < EventHandlerClass * >( params.This ) ).*func )
				( boost::polymorphic_cast< WidgetType * >( params.This )
				, This->itsEditColumn
				, This->itsEditRow
				, updateNewVal
				);

			if ( update )
			{
				// Faking a fresh on the item/subItem
				ListView_SetItemText
					( This->handle()
					, This->itsEditRow
					, This->itsEditColumn
					, const_cast < TCHAR * >( updateNewVal.c_str() )
					);
			}
			return 0;
		}
	}

	private:
		//helper function for dispatchLParamIntIntString[This], handles both member/global callbacks
		static int dispatchGetIcon( private_::SignalContent & params, int lParam, int item )
		{
			int iconIndex = I_IMAGECALLBACK;

			WidgetType * widget = boost::polymorphic_cast< WidgetType * >( params.This );
			typename MessageMapType::itsVoidGetIconFunc memberFunc = widget->itsMemberGetIconFunction;
			typename MessageMapType::voidGetIconFunc globalFunc = widget->itsGlobalGetIconFunction;

			if ( memberFunc )
			{
				( ( * internal_::getTypedParentOrThrow < EventHandlerClass * >( params.This ) ).*memberFunc )
					( widget
					, lParam
					, item
					, iconIndex
					);
			}
			else if ( globalFunc )
			{
				globalFunc
					( internal_::getTypedParentOrThrow < EventHandlerClass * >( params.This )
					, widget
					, lParam
					, item
					, iconIndex
					);
			}
			return iconIndex;
		}
};

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
template< class EventHandlerClass, class MessageMapPolicy >
class WidgetDataGrid :
	public MessageMapControl< EventHandlerClass, WidgetDataGrid< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy >,
	private virtual TrueWindow,

	// Aspect classes
	public AspectBorder< WidgetDataGrid< EventHandlerClass, MessageMapPolicy > >,
	public AspectClickable< EventHandlerClass, WidgetDataGrid< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetDataGrid< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectDblClickable< EventHandlerClass, WidgetDataGrid< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetDataGrid< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectEnabled< EventHandlerClass, WidgetDataGrid< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetDataGrid< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectFocus< EventHandlerClass, WidgetDataGrid< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetDataGrid< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectFont< WidgetDataGrid< EventHandlerClass, MessageMapPolicy > >,
	public AspectKeyPressed< EventHandlerClass, WidgetDataGrid< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetDataGrid< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectMouseClicks< EventHandlerClass, WidgetDataGrid< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetDataGrid< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectRaw< EventHandlerClass, WidgetDataGrid< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetDataGrid< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectRightClickable< EventHandlerClass, WidgetDataGrid< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetDataGrid< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectScrollable< EventHandlerClass, WidgetDataGrid< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetDataGrid< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectSelection< EventHandlerClass, WidgetDataGrid< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetDataGrid< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectSizable< EventHandlerClass, WidgetDataGrid< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetDataGrid< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectThreads< EventHandlerClass, WidgetDataGrid< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetDataGrid< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectVisible< EventHandlerClass, WidgetDataGrid< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetDataGrid< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >
{
	typedef MessageMap< EventHandlerClass, MessageMapPolicy > MessageMapTypeParam;
	typedef MessageMapControl< EventHandlerClass, WidgetDataGrid, MessageMapPolicy > MessageMapType;
	typedef AspectListDispatcher< EventHandlerClass, WidgetDataGrid, MessageMapType > DispatcherList;

	// Need to be friend to access private data...
	friend class AspectListDispatcher< EventHandlerClass, WidgetDataGrid, MessageMapType >;
	friend class WidgetCreator< WidgetDataGrid >;

	typedef MessageMapControl< EventHandlerClass, WidgetDataGrid, MessageMapPolicy > ThisMessageMap;

public:
	/// Class type
	typedef WidgetDataGrid< EventHandlerClass, MessageMapPolicy > ThisType;

	/// Object type
	typedef WidgetDataGrid< EventHandlerClass, MessageMapPolicy > * ObjectType;

	/// Seed class
	/** This class contains all of the values needed to create the widget. It also
	  * knows the type of the class whose seed values it contains. Every widget
	  * should define one of these.
	  */
	class Seed
		: public SmartWin::Seed
	{
	public:
		typedef typename WidgetDataGrid::ThisType WidgetType;

		//TODO: put variables to be filled here

		/// Fills with default parameters
		// explicit to avoid conversion through SmartWin::CreationalStruct
		explicit Seed();

		/// Doesn't fill any values
		Seed( DontInitialize )
		{}
	};

	/// Default values for creation
	static const Seed & getDefaultSeed();

	// Aspect expectation implementation
	static Message & getSelectionChangedMessage();

	// Contract needed by AspectClickable Aspect class
	static inline Message & getClickMessage();

	// Contract needed by AspectDblClickable Aspect class
	static Message & getDblClickMessage();

	/// \ingroup EventHandlersWidgetDataGrid
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

	/// \ingroup EventHandlersWidgetDataGrid
	/// Event handler for the GetItem event
	/** If you insert callback items this function will be called whenever the grid
	  * needs data to display in a row. <br>
	  * The row and column will be given along with the LPARAM to the event handler,
	  * normally the LPARAM will be either some sort of application defined ID of
	  * some sort ( maybe a primary key to a record in a database ) or a pointer to
	  * an object of some sort casted to an LPARAM!
	  */
	void onGetItem( typename MessageMapType::itsVoidGetItemFunc eventHandler );
	void onGetItem( typename MessageMapType::voidGetItemFunc eventHandler );

		/// \ingroup EventHandlersWidgetDataGrid
		/// Event handler for the GetIcon event
		/** If you insert callback items, this function will be called whenever the grid needs an icon to display in
		  * a row (you will need to specify the icon index in the associated image list, if you don't, no icon will
		  * be displayed).< br >
		  * The row will be given along with the LPARAM to the event handler, normally the LPARAM will
		  * be either some sort of application defined ID of some sort ( maybe a primary key to a record in a database )
		  * or a pointer to an object of some sort casted to an LPARAM!< br >
		  */
		void onGetIcon( typename MessageMapType::itsVoidGetIconFunc eventHandler );
		void onGetIcon( typename MessageMapType::voidGetIconFunc eventHandler );
	// TODO: Rename to onCustomPainting, but this is DEADLY since it WILL break EXISTING code!!
	// Find some solution around, maybe mark as deprecated or something...
	/// \ingroup EventHandlersWidgetDataGrid
	/// Event handler for the Custom Draw Event
	/** If you want to do custom drawing for the WidgetDataGrid use this event
	  * handler. Note that the given unsigned row number is NOT the PHYSICAL row but
	  * rather the n'th inserted row if rows are inserted with default LPARAM values
	  * or the LPARAM value given when row was inserted if LPARAM value was
	  * explicitly given. This means that if you for instance inserts 5 rows with the
	  * default given LPARAM value and removes the three rows in the middle the rows
	  * remaining will be the 0'th and the 4'th row, this will be also the values
	  * given to you in your Custom Draw Event Handler... If you need to get the
	  * PHYSICAL row number use the getRowNumberFromLParam function.
	  */
		void onCustomPainting( typename MessageMapType::itsVoidUnsignedUnsignedBoolCanvasRectangle eventHandler );
		void onCustomPainting( typename MessageMapType::voidUnsignedUnsignedBoolCanvasRectangle eventHandler );

	/// \ingroup EventHandlersWidgetDataGrid
	/// Event handler for the SortItems event
	/** When you sort a WidgetDataGrid you need to supply a callback function for
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
	void onSortItems( typename MessageMapType::itsIntLparamLparam );
	void onSortItems( typename MessageMapType::intCallbackCompareFunc );

	/// \ingroup EventHandlersWidgetDataGrid
	/// Event Handler for the Column Header Click event
	/** This Event is raised whenever one of the headers is clicked, it is useful to
	  * e.g. sort the WidgetDataGrid according to which header is being clicked. <br>
	  * Parameters passed is int which defines which header from left to right ( zero
	  * indexed ) is being clicked!
	  */
	void onColumnHeaderClick( typename MessageMapType::itsVoidFunctionTakingInt );
	void onColumnHeaderClick( typename MessageMapType::voidFunctionTakingInt );

	/// Sorts the list
	/** Call this function to sort the list, it's IMPERATIVE that you before calling
	  * this function defines an event handler for the SortItems event. <br>
	  * Otherwise you will get an exception when calling this function since it
	  * expects to have a compare function to compare items with which you can define
	  * in the onSortItems Event Handler setter
	  */
	void sortList();

	/// Returns the text of the given cell
	/** The column is which column you wish to retrieve the text for. <br>
	  * The row is which row in that column you wish to retrieve the text for. <br>
	  * Note this one returns the text of the "logical" column which means that if
	  * you have moved a column ( e.g. column no 3 ) and you're trying to retrieve
	  * the text of column no. 3 it will still return the text of the OLD column no.
	  * 3 which now might be moved to the beginning of the grid, NOT the NEW column
	  * no. 3 <br>
	  * The above does NOT apply for ROWS, meaning it will get the text of the NEW
	  * row if row is moved due to a sort e.g. <br>
	  * If you need to get "logical" row use getCellTextByLParam instead.
	  */
	SmartUtil::tstring getCellText( unsigned int column, unsigned int row );

	/// Returns the text of the given cell
	/** The column is which column you wish to retrieve the text for. <br>
	  * The lParam is what the LPARAM value when inserted was for the specific item,
	  * mark here that if you insert items with the default parameter the LPARAM will
	  * start from 0 and increase, meaning you can get this to map between rows which
	  * are moved due to a sort or something! <br>
	  * Note this one returns the text of the "logical" column which means that if
	  * you have moved a column ( e.g. column no 3 ) and you're trying to retrieve
	  * the text of column no. 3 it will still return the text of the OLD column no.
	  * 3, NOT the NEW column no. 3 <br>
	  * Note that this function throws if it can't find the item searching for!!
	  */
	SmartUtil::tstring getCellTextByLParam( unsigned int column, LPARAM lParam );

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
	std::vector< unsigned > getSelectedRows();

	/// Returns an unsigned integer which is the selected row of the grid.
	/** The return value defines the row in the grid that is selected. <br>
	  * If the grid is in "multiple selection mode" you should rather use the
	  * getSelectedRows function. <br>
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

	/// Sets the text of the given cell
	/** Sets a new string value for a given cell.
	  */
	void setCellText( unsigned column, unsigned row, const SmartUtil::tstring & newVal );

	/// Change the current icon of an item
	/** Sets a new icon for a given item
	  */
	void setItemIcon( unsigned row, int newIconIndex );

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

	/// Sets the background color of the DataGrid
	/** Sets the background color of the rows that does NOT have content! If you need
	  * to set the background color of the rows in the DataGrid that does actually
	  * have CONTENT you need to handle the onCustomDraw Event!
	  */
	void setBackgroundColor( COLORREF bgColor );

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

	/// Returns the checked state of the given row
	/** A list view can have checkboxes in each row, if the checkbox for the given
	  * row is CHECKED this funtion returns true.
	  */
	bool getIsRowChecked( unsigned row );

	/// Sets the checked state of the given row
	/** Every row in a List View can have its own checkbox column.< br >
	  * If this checkbox is selected in the queried row this function will return true.
	  */
	void setRowChecked( unsigned row, bool value = true );

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
	  * getSelectedRows ( multiple row selection mode )
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
	void setHoover( bool value = true );
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
	void deleteColumn( unsigned columnNo );

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

	/// Removes all rows contained in the grid.
	/** Removes ALL rows in the Data Grid
	  */
	void removeAllRows();

	/// Remove a specific row in the grid
	/** Removes a SPECIFIC row from the Data Grid
	  */
	void removeRow( unsigned row );

	/// Returns the number of rows in the grid
	/** Returns the number of rows in the Data Grid
	  */
	unsigned getRowCount();

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
	LPARAM insertRow( const std::vector< SmartUtil::tstring > & row, LPARAM lPar = 0, int index = - 1, int iconIndex = - 1 );

	/// Inserts an empty row into the grid
	/** Inserts an empty row into the grid. <br>
	  * Will insert an empty string as the value of all cells in the row. <br>
	  * The index parameter ( optionally ) defines at which index the new row will be
	  * inserted at. <br>
	  * If omitted it defaults to - 1 ( which means at the "end" of the grid ) <br>
	  * The iconIndex parameter ( optionally ) defines the index of the icon on the
	  * image list that will be shown on the row. <br>
	  * Call createColumns before inserting items.
	  */
	void insertRow( int index = - 1, int iconIndex = - 1 );

	/// Inserts a callback row
	/** A callback row is a row that doesn't contain the actual data to display. <br>
	  * Instead your Event Handler object will be called when the control needs data
	  * to display in a cell. <br>
	  * Useful for grids containing very large amounts of data since this way you
	  * don't have to store the data in memory, you can for instance use a reader
	  * from disc and read items on "demand basis" or maybe from a database etc...
	  * <br>
	  * Note! <br>
	  * If you insert callback rows you MUST handle the onGetItem event since this
	  * event handler will be called whenever the control needs data to display! <br>
	  * The lParam can be ANYTHING, often you cast an application defined Id or maybe
	  * a pointer of some sort to an LPARAM and pass to this function!
	  */
	void insertCallbackRow( const LPARAM lParam );

	/// Reserves a number of items to the list
	/** To be used in combination with the "onGetItem" event <br>
	  * This function reserves a number of items to the list so that the control
	  * knows how many items <br>
	  * it need to give room for. <br>
	  * Has absolutely no meaning unless used in combination with the
	  * insertCallbackRow and the "onGetItem" event in which the application is
	  * queried for items when the control needs to get data to display.
	  */
	void setItemCount( unsigned size );

	/// Returns the physical rownumber from a given LPARAM number
	/** Use this e.g. in the onCustomDraw Event Handler to find the physical row
	  * number of a given LPARAM ID. Returns - 1 if unsucessful
	  */
	unsigned getRowNumberFromLParam( unsigned lParam );

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
	void redrawItems( int firstRow, int lastRow );

	/// Force redraw all items.
	/** You may want to call invalidateWidget after the this call to force repaint.
	  */
	void redrawItems();

	/// Actually creates the Data Grid Control
	/** You should call WidgetFactory::createDataGrid if you instantiate class
	  * directly. <br>
	  * Only if you DERIVE from class you should call this function directly.
	  */
	virtual void create( const Seed & cs = getDefaultSeed() );

	// Constructor Taking pointer to parent
	explicit WidgetDataGrid( SmartWin::Widget * parent );

	static bool isValidSelectionChanged( LPARAM lPar );

protected:
	/// Adds or Removes extended list view styles from the list view
	void addRemoveListViewExtendedStyle( DWORD addStyle, bool add );

	// Protected to avoid direct instantiation, you can inherit and use
	// WidgetFactory class which is friend
	virtual ~WidgetDataGrid()
	{
#ifdef WINCE
		ImageList_Destroy( itsHImageList );
#endif
	}

private:
	// Edit row index and Edit column index, only used when grid is in "edit mode"
	int itsEditRow;
	int itsEditColumn;

	unsigned itsXMousePosition;
	unsigned itsYMousePosition;

		ImageListPtr itsNormalImageList;
	ImageListPtr itsSmallImageList;
	ImageListPtr itsStateImageList;

	typename MessageMapType::itsIntLparamLparam itsMemberSortFunction;
	typename MessageMapType::intCallbackCompareFunc itsGlobalSortFunction;

		typename MessageMapType::itsVoidGetIconFunc itsMemberGetIconFunction;
		typename MessageMapType::voidGetIconFunc itsGlobalGetIconFunction;
	static int CALLBACK CompareFunc( LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort );

	// If true the grid is in "read only mode" meaning that cell values cannot be edited.
	// A simpler version of defining a beenValidate always returning false
	bool isReadOnly;

	// Number of columns in grid
	unsigned int itsNoColumns;

	bool itsEditingCurrently; // Inbetween BEGIN and END EDIT

	// Private validate function, this ones returns the "read only" property of the list
	static bool defaultValidate( EventHandlerClass * parent, WidgetDataGrid * list, unsigned int col, unsigned int row, SmartUtil::tstring & newValue );

	// Calculates the adjustment from the columns of an item.
	int xoffFromColumn( int column, int & logicalColumn );

	// Returns the rect for the item per code (wraps ListView_GetItemRect)
	RECT getItemRect( int item, int code );

	// Returns the rect for the subitem item per code (wraps ListView_GetSubItemRect)
	RECT getSubItemRect( int item, int subitem, int code );

	// Must override to get the LVN_BEGINLABELEDIT && NM_DBLCLK event handler (we
	// don't have an event handler for them)...
	virtual LRESULT sendWidgetMessage( HWND hWnd, UINT msg, WPARAM & wPar, LPARAM & lPar );
#ifdef WINCE
	HIMAGELIST itsHImageList;
#endif
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template< class EventHandlerClass, class MessageMapPolicy >
const typename WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::Seed & WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::getDefaultSeed()
{
	static bool d_NeedsInit = true;
	static Seed d_DefaultValues( DontInitializeMe );

	if ( d_NeedsInit )
	{
		Application::instance().setSystemClassName( d_DefaultValues, WC_LISTVIEW );
		d_DefaultValues.style = WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_EDITLABELS;
		d_DefaultValues.exStyle = WS_EX_CLIENTEDGE;
		//TODO: fill the values
		d_NeedsInit = false;
	}
	return d_DefaultValues;
}

template< class EventHandlerClass, class MessageMapPolicy >
WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::Seed::Seed()
{
	* this = WidgetDataGrid::getDefaultSeed();
}

template< class EventHandlerClass, class MessageMapPolicy >
Message & WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::getSelectionChangedMessage()
{
	static Message retVal = Message( WM_NOTIFY, LVN_ITEMCHANGED ); // TODO: Implement LVN_ITEMCHANGING Event Handlers (return bool to indicate allowance)
	return retVal;
}

template< class EventHandlerClass, class MessageMapPolicy >
bool WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::isValidSelectionChanged( LPARAM lPar )
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

template< class EventHandlerClass, class MessageMapPolicy >
Message & WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::getClickMessage()
{
	static Message retVal = Message( WM_NOTIFY, NM_CLICK );
	return retVal;
}

template< class EventHandlerClass, class MessageMapPolicy >
Message & WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::getDblClickMessage()
{
	static Message retVal = Message( WM_NOTIFY, NM_DBLCLK );
	return retVal;
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::onValidate( typename MessageMapControl< EventHandlerClass, WidgetDataGrid, MessageMapPolicy >::itsBoolValidationFunc eventHandler )
{
	if ( this->getReadOnly() )
		this->setReadOnly( false );
	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->addNewSignal(
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

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::onValidate( typename MessageMapControl< EventHandlerClass, WidgetDataGrid, MessageMapPolicy >::boolValidationFunc eventHandler )
{
	if ( this->getReadOnly() )
		this->setReadOnly( false );
	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->addNewSignal(
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

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::onGetItem( typename MessageMapControl< EventHandlerClass, WidgetDataGrid, MessageMapPolicy >::itsVoidGetItemFunc eventHandler )
{
	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->addNewSignal(
		typename MessageMapType::SignalTupleType(
			private_::SignalContent(
				Message( WM_NOTIFY, LVN_GETDISPINFO ),
				reinterpret_cast< itsVoidFunction >( eventHandler ),
				ptrThis
			),
			typename MessageMapType::SignalType(
				typename MessageMapType::SignalType::SlotType( & DispatcherList::dispatchLparamIntIntStringThis )
			)
		)
	);
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::onGetItem( typename MessageMapControl< EventHandlerClass, WidgetDataGrid, MessageMapPolicy >::voidGetItemFunc eventHandler )
{
	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->addNewSignal(
		typename MessageMapType::SignalTupleType(
			private_::SignalContent(
				Message( WM_NOTIFY, LVN_GETDISPINFO ),
				reinterpret_cast< private_::SignalContent::voidFunctionTakingVoid >( eventHandler ),
				ptrThis
			),
			typename MessageMapType::SignalType(
				typename MessageMapType::SignalType::SlotType( & DispatcherList::dispatchLparamIntIntString )
			)
		)
	);
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::onGetIcon( typename MessageMapControl< EventHandlerClass, WidgetDataGrid, MessageMapPolicy >::itsVoidGetIconFunc eventHandler )
{
	itsGlobalGetIconFunction = 0;
	itsMemberGetIconFunction = eventHandler;
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::onGetIcon( typename MessageMapControl< EventHandlerClass, WidgetDataGrid, MessageMapPolicy >::voidGetIconFunc eventHandler )
{
	itsGlobalGetIconFunction = eventHandler;
	itsMemberGetIconFunction = 0;
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::onCustomPainting( typename MessageMapControl< EventHandlerClass, WidgetDataGrid, MessageMapPolicy >::itsVoidUnsignedUnsignedBoolCanvasRectangle eventHandler )
{
	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->addNewSignal(
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

template< class EventHandlerClass, class MessageMapPolicy >
	void WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::onCustomPainting( typename MessageMapControl< EventHandlerClass, WidgetDataGrid, MessageMapPolicy >::voidUnsignedUnsignedBoolCanvasRectangle eventHandler )
{
	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->addNewSignal(
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

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::onSortItems( typename MessageMapControl< EventHandlerClass, WidgetDataGrid, MessageMapPolicy >::itsIntLparamLparam eventHandler )
{
	itsGlobalSortFunction = 0;
	itsMemberSortFunction = eventHandler;
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::onSortItems( typename MessageMapControl< EventHandlerClass, WidgetDataGrid, MessageMapPolicy >::intCallbackCompareFunc eventHandler )
{
	itsMemberSortFunction = 0;
	itsGlobalSortFunction = eventHandler;
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::onColumnHeaderClick( typename MessageMapControl< EventHandlerClass, WidgetDataGrid, MessageMapPolicy >::itsVoidFunctionTakingInt eventHandler )
{
	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->addNewSignal(
		typename MessageMapType::SignalTupleType(
			private_::SignalContent(
				Message( WM_NOTIFY, LVN_COLUMNCLICK ),
				reinterpret_cast< itsVoidFunction >( eventHandler ),
				ptrThis
			),
			typename MessageMapType::SignalType(
				typename MessageMapType::SignalType::SlotType( & DispatcherList::dispatchColumnClickThis )
			)
		)
	);
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::onColumnHeaderClick( typename MessageMapControl< EventHandlerClass, WidgetDataGrid, MessageMapPolicy >::voidFunctionTakingInt eventHandler )
{
	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->addNewSignal(
		typename MessageMapType::SignalTupleType(
			private_::SignalContent(
				Message( WM_NOTIFY, LVN_COLUMNCLICK ),
				reinterpret_cast< private_::SignalContent::voidFunctionTakingVoid >( eventHandler ),
				ptrThis
			),
			typename MessageMapType::SignalType(
				typename MessageMapType::SignalType::SlotType( & DispatcherList::dispatchColumnClick )
			)
		)
	);
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::sortList()
{
	xAssert( itsGlobalSortFunction || itsMemberSortFunction, _T( "No sort event handlers defined" ) );

	xAssert( ListView_SortItems( this->Widget::itsHandle, CompareFunc, reinterpret_cast< LPARAM >( this ) ) == TRUE,
		_T( "ListView_SortItems fizzled" ) );
}

template< class EventHandlerClass, class MessageMapPolicy >
SmartUtil::tstring WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::getCellText( unsigned int column, unsigned int row )
{
	// TODO: Fix
	const int BUFFER_MAX = 2048;
	TCHAR buffer[BUFFER_MAX + 1];
	buffer[0] = '\0';
	ListView_GetItemText( this->Widget::itsHandle, row, column, buffer, BUFFER_MAX );
	return buffer;
}

template< class EventHandlerClass, class MessageMapPolicy >
SmartUtil::tstring WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::getCellTextByLParam( unsigned int column, LPARAM lParam )
{
	LVFINDINFO lvfi;
	lvfi.flags = LVFI_PARAM;
	lvfi.lParam = lParam;
	int row = ListView_FindItem( this->Widget::itsHandle, - 1, & lvfi );
	if ( row == - 1 )
		throw xCeption( _T( "Couldn't find WidgetDataGrid item with given LPARAM" ) );
	return this->getCellText( column, row );
}

template< class EventHandlerClass, class MessageMapPolicy >
bool WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::hasSelection()
{
	return ListView_GetSelectedCount( this->Widget::itsHandle ) > 0;
}

template< class EventHandlerClass, class MessageMapPolicy >
std::vector< unsigned > WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::getSelectedRows()
{
	std::vector< unsigned > retVal;
	int tmpIdx = - 1;
	while ( true )
	{
		tmpIdx = ListView_GetNextItem( this->Widget::itsHandle, tmpIdx, LVNI_SELECTED );
		if ( tmpIdx == - 1 )
			break;
		retVal.push_back( static_cast< unsigned >( tmpIdx ) );
	}
	return retVal;
}

template< class EventHandlerClass, class MessageMapPolicy >
int WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::getSelectedIndex() const
{
	unsigned tmpIdx = - 1;
	tmpIdx = ListView_GetNextItem( this->Widget::itsHandle, tmpIdx, LVNI_SELECTED );
	return tmpIdx;
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::setSelectedIndex( int idx )
{
	// TODO: Check if this is working right...
	LVITEM it;
	ZeroMemory( & it, sizeof( LVITEM ) );
	it.iItem = idx;
	it.mask = LVIF_STATE;
	it.state = LVIS_SELECTED | LVIS_FOCUSED;
	it.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
	if ( ListView_SetItem( this->Widget::itsHandle, & it ) != TRUE )
	{
		xCeption err( _T( "Something went wrong while trying to set the selected property of the ListView" ) );
		throw err;
	}
	if ( ListView_EnsureVisible( this->Widget::itsHandle, idx, FALSE ) != TRUE )
	{
		xCeption err( _T( "Something went wrong while trying to scroll selected item into view in setSelectedIndex" ) );
		throw err;
	}
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::clearSelection()
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
		if ( ListView_SetItem( this->Widget::itsHandle, & it ) != TRUE )
		{
			xCeption err( _T( "Something went wrong while trying to unset the selected property of the ListView" ) );
			throw err;
		}
	}
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::setCellText( unsigned column, unsigned row, const SmartUtil::tstring & newVal )
{
	// const bug inn Windows API
	ListView_SetItemText( this->Widget::itsHandle, row, column, const_cast < TCHAR * >( newVal.c_str() ) );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::setItemIcon( unsigned row, int newIconIndex )
{
	LVITEM it;
	ZeroMemory( & it, sizeof( LVITEM ) );
	it.iItem = row;
	it.mask = LVIF_IMAGE;
	//Get item
	if(ListView_GetItem( this->Widget::itsHandle, &it) != TRUE)
	{
		xCeption err( _T( "Something went wrong while trying to receive the selected item of the ListView" ) );
		throw err;
	}
	//Modify item
	it.iImage = newIconIndex;
	//Set item
	if(ListView_SetItem( this->Widget::itsHandle, &it) != TRUE)
	{
		xCeption err( _T( "Something went wrong while trying to change the selected item of the ListView" ) );
		throw err;
	}
}

template< class EventHandlerClass, class MessageMapPolicy >
bool WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::getReadOnly()
{
	return isReadOnly;
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::setReadOnly( bool value )
{
	isReadOnly = value;
	this->Widget::addRemoveStyle( LVS_EDITLABELS, !value );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::setBackgroundColor( COLORREF bgColor )
{
	ListView_SetBkColor( this->Widget::handle(), bgColor );
}

template< class EventHandlerClass, class MessageMapPolicy >
unsigned WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::getColumnCount()
{
	return itsNoColumns;
}

template< class EventHandlerClass, class MessageMapPolicy >
SmartUtil::tstring WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::getColumnName( unsigned col )
{
	// TODO: Fix
	const int BUFFER_MAX = 2048;
	TCHAR buffer[BUFFER_MAX + 1];
	LV_COLUMN colInfo;
	colInfo.mask = LVCF_TEXT;
	colInfo.cchTextMax = BUFFER_MAX;
	colInfo.pszText = buffer;
	ListView_GetColumn( this->Widget::itsHandle, col, & colInfo );
	return colInfo.pszText;
}

template< class EventHandlerClass, class MessageMapPolicy >
bool WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::getIsRowChecked( unsigned row )
{
	return ListView_GetCheckState( this->Widget::itsHandle, row ) == TRUE;
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::setRowChecked( unsigned row, bool value )
{
	ListView_SetCheckState( this->Widget::itsHandle, row, value );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::setFullRowSelect( bool value )
{
	addRemoveListViewExtendedStyle( LVS_EX_FULLROWSELECT, value );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::setItemCount( unsigned size )
{
	ListView_SetItemCount( this->Widget::itsHandle, size );
}

template< class EventHandlerClass, class MessageMapPolicy >
unsigned WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::getRowNumberFromLParam( unsigned lParam )
{
	LVFINDINFO lv;
	lv.flags = LVFI_PARAM;
	lv.lParam = lParam;
	return ( unsigned ) ListView_FindItem( this->Widget::itsHandle, - 1, & lv );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::setNormalImageList( ImageListPtr imageList )
{
	  itsNormalImageList = imageList;
	  ListView_SetImageList( this->Widget::handle(), imageList->getImageList(), LVSIL_NORMAL );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::setSmallImageList( ImageListPtr imageList )
{
	  itsSmallImageList = imageList;
	  ListView_SetImageList( this->Widget::handle(), imageList->getImageList(), LVSIL_SMALL );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::setStateImageList( ImageListPtr imageList )
{
	  itsStateImageList = imageList;
	  ListView_SetImageList( this->Widget::handle(), imageList->getImageList(), LVSIL_STATE );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::setView( int view )
{
	if ( ( view & LVS_TYPEMASK ) != view )
	{
		xCeption x( _T( "Invalid View type" ) );
		throw x;
	}
	//little hack because there is no way to do this with Widget::addRemoveStyle
	int newStyle = GetWindowLong( this->Widget::handle(), GWL_STYLE );
	if ( ( newStyle & LVS_TYPEMASK ) != view )
	{
		SetWindowLong( this->Widget::handle(), GWL_STYLE, ( newStyle & ~LVS_TYPEMASK ) | view );
	}
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::redrawItems( int firstRow, int lastRow )
{
	if( ListView_RedrawItems( this->Widget::handle(), firstRow, lastRow ) == FALSE )
	{
		throw xCeption( _T( "Error while redrawing items in ListView" ) );
	}
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::redrawItems()
{
	this->redrawItems( 0, this->getRowCount() );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::setCheckBoxes( bool value )
{
	addRemoveListViewExtendedStyle( LVS_EX_CHECKBOXES, value );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::setSingleRowSelection( bool value )
{
	this->Widget::addRemoveStyle( LVS_SINGLESEL, value );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::setGridLines( bool value )
{
	addRemoveListViewExtendedStyle( LVS_EX_GRIDLINES, value );
}

#ifndef WINCE
template< class EventHandlerClass, class MessageMapPolicy >
void WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::setHoover( bool value )
{
	addRemoveListViewExtendedStyle( LVS_EX_TWOCLICKACTIVATE, value );
}
#endif

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::setHeaderDragDrop( bool value )
{
	addRemoveListViewExtendedStyle( LVS_EX_HEADERDRAGDROP, value );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::setAlwaysShowSelection( bool value )
{
	this->Widget::addRemoveStyle( LVS_SHOWSELALWAYS, value );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::createColumns( const std::vector< SmartUtil::tstring > & colNames )
{
	// Deleting all data
	if ( itsNoColumns != 0 )
	{
		removeAllRows();
		while ( ListView_DeleteColumn( this->Widget::itsHandle, 0 ) == TRUE );
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
		if ( ListView_InsertColumn( this->Widget::itsHandle, x, & lvColumn ) == - 1 )
		{
			xCeption x( _T( "Error while trying to create Columns in list view" ) );
			throw x;
		}
	}
	itsNoColumns = static_cast< unsigned >( colNames.size() );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::deleteColumn( unsigned columnNo )
{
	xAssert( columnNo != 0, _T( "Can't delete the leftmost column" ) );
	ListView_DeleteColumn( this->Widget::itsHandle, columnNo );
	--itsNoColumns;
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::setColumnWidth( unsigned columnNo, int width )
{
	if ( ListView_SetColumnWidth( this->Widget::itsHandle, columnNo, width ) == FALSE )
	{
		xCeption x( _T( "Couldn't resize columns of WidgetDataGrid" ) );
		throw x;
	}
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::removeAllRows()
{
	ListView_DeleteAllItems( this->Widget::itsHandle );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::removeRow( unsigned row )
{
	ListView_DeleteItem( this->Widget::itsHandle, row );
}

template< class EventHandlerClass, class MessageMapPolicy >
unsigned WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::getRowCount()
{
	return ListView_GetItemCount( this->Widget::itsHandle );
}

template< class EventHandlerClass, class MessageMapPolicy >
LPARAM WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::insertRow( const std::vector< SmartUtil::tstring > & row, LPARAM lPar, int index, int iconIndex )
{
		xAssert( itsNoColumns == row.size() && itsNoColumns != 0, _T( "Tried to add a row into a WidgetDataGridView with wrong number of columns" ) );
		if ( index == - 1 )
		{
			// Appending at bottom
			index = ListView_GetItemCount( this->Widget::itsHandle );
		}
		int itemCount = ListView_GetItemCount( this->Widget::itsHandle );
		LV_ITEM lvi =
		{0
		};
		lvi.mask = LVIF_TEXT | LVIF_PARAM;
		if ( itsNormalImageList || itsSmallImageList )
		{
			lvi.mask |= LVIF_IMAGE;
			lvi.iImage = ( iconIndex == - 1 ? I_IMAGECALLBACK : iconIndex );
		}
		lvi.pszText = const_cast < TCHAR * >( row[0].c_str() );
		lvi.lParam = lPar == 0 ? itemCount++ : lPar;
		lvi.cchTextMax = static_cast< int >( row[0].size() );
		lvi.iItem = index;
		if ( ListView_InsertItem( this->Widget::itsHandle, & lvi ) == - 1 )
		{
			xCeption x( _T( "Error while trying to insert row in ListView" ) );
			throw x;
		}
		int noCol = 1;
		lvi.mask = LVIF_TEXT;
		for ( std::vector< SmartUtil::tstring >::const_iterator idx = row.begin() + 1;
			idx != row.end();
			++idx )
		{
			lvi.iSubItem = noCol++;
			lvi.pszText = const_cast < TCHAR * >( idx->c_str() );
			lvi.cchTextMax = static_cast< int >( idx->size() );
			if ( !ListView_SetItem( this->Widget::itsHandle, & lvi ) )
			{
				xCeption x( _T( "Error while trying to insert row in ListView" ) );
				throw x;
			}
		}
		return lvi.lParam;
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::insertRow( int index, int iconIndex )
{
		if ( index == - 1 )
		{
			index = ListView_GetItemCount( this->Widget::itsHandle );
		}
		int itemCount = ListView_GetItemCount( this->Widget::itsHandle );
		LV_ITEM lvi =
		{0
		};
		lvi.mask = LVIF_TEXT | LVIF_PARAM;
		if ( itsNormalImageList || itsSmallImageList )
		{
			lvi.mask |= LVIF_IMAGE;
			lvi.iImage = ( iconIndex == - 1 ? I_IMAGECALLBACK : iconIndex );
		}
		lvi.pszText = _T( "" );
		lvi.lParam = itemCount++;
		lvi.cchTextMax = 0;
		lvi.iItem = index;
		if ( ListView_InsertItem( this->Widget::itsHandle, & lvi ) == - 1 )
		{
			xCeption x( _T( "Error while trying to insert row in ListView" ) );
			throw x;
		}
		int noCol = 1;
		lvi.mask = LVIF_TEXT;
		for ( int idx = 1;
			idx != itsNoColumns;
			++idx )
		{
			lvi.iSubItem = noCol++;
			if ( !ListView_SetItem( this->Widget::itsHandle, & lvi ) )
			{
				xCeption x( _T( "Error while trying to insert row in ListView" ) );
				throw x;
			}
		}
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::insertCallbackRow( const LPARAM lParam )
{
		//assert( (itsMessageMap.find( Message( WM_NOTIFY, LVN_GETDISPINFO ) ) != itsMessageMap.end() ||
		// itsMessageMapThis.find( Message( WM_NOTIFY, LVN_GETDISPINFO ) ) != itsMessageMapThis.end() )
		// && _T("Can't insert a callback item without handling the GetItem event!") );

		// Appending at bottom
		int index = ListView_GetItemCount( this->Widget::itsHandle );

		LV_ITEM lvi =
		{0
		};
		lvi.mask = LVIF_TEXT | LVIF_PARAM;
		lvi.pszText = LPSTR_TEXTCALLBACK;
		lvi.lParam = lParam;
		lvi.iItem = index;
		if ( itsNormalImageList || itsSmallImageList )
		{
			lvi.mask |= LVIF_IMAGE;
			lvi.iImage = I_IMAGECALLBACK;
		}
		if ( ListView_InsertItem( this->Widget::itsHandle, & lvi ) == - 1 )
		{
			xCeption x( _T( "Error while trying to insert row in ListView" ) );
			throw x;
		}

		lvi.mask = LVIF_TEXT;
		lvi.lParam = 0;
		for ( unsigned idx = 1; idx < itsNoColumns; ++idx )
		{
			lvi.iSubItem = idx;
			if ( !ListView_SetItem( this->Widget::itsHandle, & lvi ) )
			{
				xCeption x( _T( "Error while trying to insert row in ListView" ) );
				throw x;
			}
		}
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::addRemoveListViewExtendedStyle( DWORD addStyle, bool add )
{
	DWORD newStyle = ListView_GetExtendedListViewStyle( this->Widget::itsHandle );
	if ( add && ( newStyle & addStyle ) != addStyle )
	{
		newStyle |= addStyle;
	}
	else if ( !add && ( newStyle & addStyle ) == addStyle )
	{
		newStyle ^= addStyle;
	}
	ListView_SetExtendedListViewStyle( this->Widget::itsHandle, newStyle );
}

// Constructor
template< class EventHandlerClass, class MessageMapPolicy >
WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::WidgetDataGrid( SmartWin::Widget * parent )
	: Widget( parent, 0 ),
	itsGlobalSortFunction( 0 ),
	itsMemberSortFunction( 0 ),
		itsGlobalGetIconFunction( 0 ),
		itsMemberGetIconFunction( 0 ),
	isReadOnly( false ),
	itsEditingCurrently( false ),
	itsNoColumns( 0 )
{
#ifdef WINCE
	itsHImageList = ImageList_Create( 1, 18, ILC_COLOR | ILC_MASK, 0, 2 );
#endif
	// Can't have a list view without a parent...
	xAssert( parent, _T( "Cant have a WidgetDataGrid without a parent..." ) );
}

template< class EventHandlerClass, class MessageMapPolicy >
int CALLBACK WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::CompareFunc( LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort )
{
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
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::create( const Seed & cs )
{
	//// Dirty hack!!
	////this->Widget::itsCtrlId = (HMENU)++this->Widget::itsInstanceNo;
	if ( cs.style & WS_CHILD )
		Widget::create( cs );
	else
	{
		typename WidgetDataGrid::Seed d_YouMakeMeDoNastyStuff = cs;

		d_YouMakeMeDoNastyStuff.style |= WS_CHILD;
		Widget::create( d_YouMakeMeDoNastyStuff );
	}
	ThisMessageMap::createMessageMap();
	//TODO: use CreationalInfo parameters

#ifdef WINCE
	// WinCE fix for gridlines...
	ListView_SetImageList( itsHandle, itsHImageList, LVSIL_SMALL );
#endif

	// Setting default event handler for beenValidate to a function returning "read
	// only" property of control Note! If you supply a beenValidate event handler
	// this will have no effect
	onValidate( WidgetDataGrid::defaultValidate );
}

template< class EventHandlerClass, class MessageMapPolicy >
bool WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::defaultValidate( EventHandlerClass * parent, WidgetDataGrid * list, unsigned int col, unsigned int row, SmartUtil::tstring & newValue )
{
	list->updateWidget();
	return !list->getReadOnly();
}

// Calculates the adjustment from the columns of an item.
template< class EventHandlerClass, class MessageMapPolicy >
int WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::xoffFromColumn( int column, int & logicalColumn )
{
	HWND hWnd = this->Widget::itsHandle;

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

template< class EventHandlerClass, class MessageMapPolicy >
RECT WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::getItemRect( int item, int code )
{
	RECT r;
	ListView_GetItemRect( this->Widget::itsHandle, item, & r, code );
	return r;
}

template< class EventHandlerClass, class MessageMapPolicy >
RECT WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::getSubItemRect( int item, int subitem, int code )
{
	RECT r;
	ListView_GetSubItemRect( this->Widget::itsHandle, item, subitem, code, & r );
	return r;
}

// TODO: Should these
template< class EventHandlerClass, class MessageMapPolicy >
LRESULT WidgetDataGrid< EventHandlerClass, MessageMapPolicy >::sendWidgetMessage( HWND hWnd, UINT msg, WPARAM & wPar, LPARAM & lPar )
{
	static int lastLine;
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
				ValidateRect( this->Widget::itsHandle, & validRect );
			}
			ValidateRect( this->Widget::itsHandle, & rowRect );
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
					::GetClientRect( this->Widget::itsHandle, & cr );
					if ( xOffset + r.left < 0 || xOffset + r.left > cr.right )
					{
						int x = xOffset - r.left;
						ListView_Scroll( this->Widget::itsHandle, x, 0 );
						r.left -= x;
					}
					// Get column alignment
					LV_COLUMN lv =
					{0
					};
					lv.mask = LVCF_FMT;
					ListView_GetColumn( this->Widget::itsHandle, logicalColumn, & lv );
					DWORD dwStyle;
					if ( ( lv.fmt & LVCFMT_JUSTIFYMASK ) == LVCFMT_LEFT )
						dwStyle = ES_LEFT;
					else if ( ( lv.fmt & LVCFMT_JUSTIFYMASK ) == LVCFMT_RIGHT )
						dwStyle = ES_RIGHT;
					else
						dwStyle = ES_CENTER;
					r.left += xOffset + 4;
					r.right = r.left + ( ListView_GetColumnWidth( this->Widget::itsHandle, itsEditColumn ) - 3 );
					if ( r.right > cr.right )
						r.right = cr.right;
					dwStyle |= WS_BORDER | WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL;
#endif

					// Creating text Widget and placing it above cell
					HWND editControl = ListView_GetEditControl( this->Widget::itsHandle );
					if ( editControl == 0 )
					{
						xCeption err( _T( "Couldn't attach to List View editcontrol" ) );
						throw err;
					}

					private_::ListViewEditBox< WidgetDataGrid, MessageMapPolicy > * text
						= new private_::ListViewEditBox< WidgetDataGrid, MessageMapPolicy >( this );
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
					ListView_GetItemText( this->Widget::itsHandle, itsEditRow, itsEditColumn, buffer, BUFFER_MAX );
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
					NMHDR * nm = reinterpret_cast< NMHDR * >( lPar );
					LVHITTESTINFO info;
					info.pt.x = itsXMousePosition;
					info.pt.y = itsYMousePosition;
					ListView_SubItemHitTest( this->Widget::itsHandle, & info );
					// User has clicked the ListView
					if ( info.iItem != - 1 )
					{
						// User has clicked an ITEM in ListView
						if ( info.iSubItem >= 0 )
						{
							UINT state = ListView_GetItemState( this->Widget::itsHandle, info.iItem, LVIS_FOCUSED );
							if ( ! ( state & LVIS_FOCUSED ) )
							{
								//SetFocus( itsHandle );   // TODO: This was catched by devcpp ... what was intended?
								SetFocus( this->Widget::itsHandle ); // ASW add
							}

							// Check to verify items are editable
							if ( ::GetWindowLong( this->Widget::itsHandle, GWL_STYLE ) & LVS_EDITLABELS )
							{
								itsEditRow = info.iItem;
								itsEditColumn = info.iSubItem;
								ListView_EditLabel( this->Widget::itsHandle, info.iItem );
								return 0; // Processed
							}
						}
					}
				} break;
			default:
				return ThisMessageMap::sendWidgetMessage( hWnd, msg, wPar, lPar );
			}
		} break;
	}
	return ThisMessageMap::sendWidgetMessage( hWnd, msg, wPar, lPar );
}

// end namespace SmartWin
}

#endif

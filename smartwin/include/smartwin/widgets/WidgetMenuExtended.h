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
#ifndef WINCE
#ifndef WidgetMenuExtended_h
#define WidgetMenuExtended_h

#include "../BasicTypes.h"
#include "../CanvasClasses.h"
#include <boost/cast.hpp>
#ifdef PORT_ME
namespace SmartWin
{
// begin namespace SmartWin

// Forward declaring friends
template< class WidgetType >
class WidgetCreator;

/// Contains extended information about extended menu item
struct MenuItemData
{
	/// Menu item text color
	COLORREF TextColor;

	/// Menu item font
	FontPtr Font;

	/// Menu item image
	BitmapPtr Image;

	/// Creates new menu item with specified data
	MenuItemData(
		FontPtr font = FontPtr( new SmartWin::Font( ( HFONT )::GetStockObject( DEFAULT_GUI_FONT ), false ) ), // defaults to SYSTEM_FONT
		BitmapPtr image = BitmapPtr( new Bitmap( ( HBITMAP ) NULL ) ), // defaults to empty bitmap
		COLORREF textColor = ::GetSysColor( COLOR_MENUTEXT ) )
		: TextColor( textColor ),
		Font( font ),
		Image( image )
	{}
};

/// \ingroup GlobalStuff
// MenuItemDataPtr type, contains rendering data for e.g. WidgetMenuExtended
/** Helps easily create color values and so on for a WidgetMenuExtended item! <br>
  * Each Menu Item can have different colors and so on, use this smart pointer to set
  * those values!
  */
typedef std::tr1::shared_ptr< MenuItemData > MenuItemDataPtr;

namespace private_
{
	// ////////////////////////////////////////////////////////////////////////
	// Menu item data wrapper, used internally
	// MENUITEMINFO's dwItemData *should* point to it
	// ////////////////////////////////////////////////////////////////////////
	struct ItemDataWrapper
	{
		// The menu item belongs to
		// For some messages (e.g. WM_MEASUREITEM),
		// Windows doesn't specify it, so
		// we need to keep this
		HMENU menu;

		// Item index in the menu
		// This is needed, because ID's for items
		// are not unique (although Windows claims)
		// e.g. we can have an item with ID 0,
		// that is either separator or popup menu
		int index;

		// Specifies if item is menu title
		bool isMenuTitleItem;

		// Contains item data
		MenuItemDataPtr data;

		// Wrapper  Constructor
		ItemDataWrapper( HMENU owner, int itemIndex, MenuItemDataPtr itemData, bool isTitleItem = false )
		: menu( owner )
		, index( itemIndex )
		, isMenuTitleItem( isTitleItem )
		, data( itemData )
		{}

		~ItemDataWrapper()
		{}
	};
}

/// Struct for coloring different areas of WidgetMenuExtended
/** Contains the different color settings of the WidgetMenuExtended <br>
  * Default values to constructor makes menu look roughly like MSVC++7.1 menus
  */
struct MenuColorInfo
{
	/// Menu color
	COLORREF colorMenu;

	/// Strip bar color
	COLORREF colorStrip;

	/// Title background color
	COLORREF colorTitle;

	/// Menu bar color
	COLORREF colorMenuBar;

	/// Highlighted menu item color
	COLORREF colorHighlight;

	/// Title text color
	COLORREF colorTitleText;

	/// Item image background color, used for transparency effects
	COLORREF colorImageBackground;

	/// Constructs MenuColorInfo objects
	/** If all the default arguments are used it will construct an object making
	  * menus look roughly like they do in MSVC++ 7.1 <br>
	  * Pass your own arguments to construct other color effects
	  */
	MenuColorInfo( COLORREF menuColor = ColorUtilities::darkenColor( ::GetSysColor( COLOR_WINDOW ), 0.02 ),
		COLORREF stripColor = ColorUtilities::darkenColor( ::GetSysColor( COLOR_3DFACE ), 0.02 ),
		COLORREF titleColor = ColorUtilities::darkenColor( ::GetSysColor( COLOR_MENUBAR ), 0.1 ),
		COLORREF menuBarColor = ::GetSysColor( COLOR_MENUBAR ),
		COLORREF highlightColor = ::GetSysColor( COLOR_HIGHLIGHT ),
		COLORREF titleTextColor = ::GetSysColor( COLOR_MENUTEXT ),
		COLORREF imageBackground = RGB( 0, 0, 0 ) ) // black
		: colorMenu( menuColor ),
		colorStrip( stripColor ),
		colorTitle( titleColor ),
		colorMenuBar( menuBarColor ),
		colorHighlight( highlightColor ),
		colorTitleText( titleTextColor ),
		colorImageBackground( imageBackground )
	{}
};

// ///////////////////////////////////////////////////////////////////////////////
// Default Menu Renderer, create your own by copying this and modyfying
// if you want your menus to be different!
// Basic process is to add Event Handlers for onDrawItem and onMeasureItem
// ///////////////////////////////////////////////////////////////////////////////
template< class MenuType >
class DefaultMenuRenderer
{
public:
	/// Rendering settting settings
	static const int borderGap = 3; /// Gap between the border and item
	static const int pointerGap = 5; /// Gap between item text and sub - menu pointer
	static const int textIconGap = 8; /// Gap between text and icon
	static const int textBorderGap = 4; /// Gap between text and rectangel border
	static const int separatorHeight = 8; /// Defines default height for rectangle containing separator
	static const int minSysMenuItemWidth = 130; /// Minimum width for system menu items

	static Point defaultImageSize; /// Default image size, used when no image is available

	// Default callback for WM_MEASUREITEM message processing
	static bool measureItem( EventHandlerClass * parent, typename MenuType::ObjectType menu, MEASUREITEMSTRUCT * measureInfo )
	{
		if ( measureInfo->CtlType != ODT_MENU ) // if not intended for us
			return false;

		// get data wrapper
		private_::ItemDataWrapper * wrapper = reinterpret_cast< private_::ItemDataWrapper * >( measureInfo->itemData );

		xAssert( wrapper != 0, _T( "Unsupported menu item type in measureItem()" ) );

		// this will contain item size
		UINT & itemWidth = measureInfo->itemWidth;
		UINT & itemHeight = measureInfo->itemHeight;

		// init struct for item info
		MENUITEMINFO info;
		memset( & info, 0, sizeof( MENUITEMINFO ) );
		info.cbSize = sizeof( MENUITEMINFO );

		// set up flags
		info.fMask = MIIM_FTYPE | MIIM_DATA | MIIM_CHECKMARKS | MIIM_STRING;

		// get menu handle
		HMENU handle = reinterpret_cast< HMENU >( menu->handle() );

		// try to get item info
		if ( ::GetMenuItemInfo( handle, wrapper->index, TRUE, & info ) == FALSE )
			throw xCeption ( _T( "Couldn't get item info in measureItem()" ) );

		// check if item is owner drawn
		xAssert( ( info.fType & MFT_OWNERDRAW ) != 0, _T( "Not owner - drawn item encountered in measureItem()" ) );

		// check if separator
		if ( info.fType & MFT_SEPARATOR )
		{
			itemWidth = 60;
			itemHeight = separatorHeight;
			return true;
		}

		// are we processing menu bar ?
		bool isMenuBar = ::GetMenu( parent->handle() ) == handle;

		// compute text width and height by simulating write to dc
		// get its DC
		HDC hdc = ::GetDC( menu->getParent()->handle() );

		// get the item data
		MenuItemDataPtr data = wrapper->data;
		xAssert( data != 0, _T( "Couldn't find item data in measureItem()" ) );

		// get item text
		const int length = info.cch + 1;
		std::vector< TCHAR > buffer ( length );
		int count = ::GetMenuString( handle, wrapper->index, & buffer[0], length, MF_BYPOSITION );
		SmartUtil::tstring itemText ( buffer.begin(), buffer.begin() + count );

		// now get text extents
		SIZE textSize;
		memset( & textSize, 0, sizeof( SIZE ) );

		HGDIOBJ oldFont = ::SelectObject( hdc, data->Font->getHandle() );
		::GetTextExtentPoint32( hdc, itemText.c_str(), ( int ) itemText.size(), & textSize );
		::SelectObject( hdc, oldFont );

		// release DC
		::ReleaseDC( menu->handle(), hdc );

		// adjust item size
		itemWidth = textSize.cx + borderGap;
		itemHeight = textSize.cy + borderGap;

		// check to see if item has an image
		Point imageSize = data->Image->getBitmapSize();

		// this will contain checked/unchecked image size
		Point checkImageSize;

		// if item has check/unchecked state images, then get their sizes
		if ( ( info.hbmpChecked != NULL ) && ( info.hbmpUnchecked != NULL ) )
			checkImageSize = ( info.fState & MFS_CHECKED ) == 0
			? Bitmap::getBitmapSize( info.hbmpUnchecked )
			: Bitmap::getBitmapSize( info.hbmpChecked );

		// take the maximum of all available images or set default image size
		imageSize.x = (std::max)( imageSize.x, checkImageSize.x );
		imageSize.y = (std::max)( imageSize.y, checkImageSize.y );

		bool hasImage = ( imageSize.x != 0 ) && ( imageSize.y != 0 );

		// set default image size if no image is available
		if ( !hasImage )
		{
			imageSize.x = (std::max)( defaultImageSize.x, (std::max)( imageSize.x, checkImageSize.x ) );
			imageSize.y = (std::max)( defaultImageSize.y, (std::max)( imageSize.y, checkImageSize.y ) );
		}

		// adjust default image size
		defaultImageSize.x = (std::max)( defaultImageSize.x, imageSize.x );
		defaultImageSize.y = (std::max)( defaultImageSize.y, imageSize.y );

		// adjust width
		if ( !isMenuBar || // if not menu bar item
		( isMenuBar && hasImage ) ) // or menu bar item with image
		{
			// adjust item width
			itemWidth += imageSize.x + textIconGap + pointerGap;

			// adjust item height
			itemHeight = (std::max)( itemHeight, ( UINT ) imageSize.y + borderGap );
		}

		// adjust width for system menu items
		if ( menu->isSysMenu )
			itemWidth = (std::max)( ( UINT ) minSysMenuItemWidth, itemWidth );

		// adjust width for sidebar
		if ( menu->drawSidebar )
		{
			// get title text extents
			SIZE textSize;
			memset( & textSize, 0, sizeof( SIZE ) );

			::GetTextExtentPoint32( hdc, menu->itsTitle.c_str(), ( int ) menu->itsTitle.size(), & textSize );

			itemWidth += textSize.cy;
		}

		// adjust item height
		itemHeight = (std::max)( itemHeight, ( UINT )::GetSystemMetrics( SM_CYMENU ) );
		return true;
	}

	// Default callback for WM_DRAWITEM message processing
	static bool drawItem( EventHandlerClass * parent, typename MenuType::ObjectType menu, int controlID, const DRAWITEMSTRUCT & drawInfo )
	{
		if ( ( controlID != 0 ) || ( drawInfo.CtlType != ODT_MENU ) ) // if not intended for us
			return false;

		// setup colors
		MenuColorInfo colorInfo = menu->itsColorInfo;
		COLORREF colorMenuBar = colorInfo.colorMenuBar;
		COLORREF colorTitle = colorInfo.colorTitle;
		COLORREF colorMenuDraw = colorInfo.colorMenu; // color for drawing menu
		COLORREF colorFillHighlighted = ColorUtilities::lightenColor( colorInfo.colorHighlight, 0.7 );

		// get menu handle
		HMENU handle = reinterpret_cast< HMENU >( menu->handle() );

		// if processing menu bar
		const bool isMenuBar = ::GetMenu( parent->handle() ) == handle;

		// change menu draw color for menubars
		if ( isMenuBar )
			colorMenuDraw = colorMenuBar;

		// get item data wrapper
		private_::ItemDataWrapper * wrapper = reinterpret_cast< private_::ItemDataWrapper * >( drawInfo.itemData );
		xAssert( wrapper != 0, _T( "Unsupported menu item in drawItem()" ) )

		// init struct for menu item info
		MENUITEMINFO info;
		memset( & info, 0, sizeof( MENUITEMINFO ) );
		info.cbSize = sizeof( MENUITEMINFO );

		// set flags
		info.fMask = MIIM_CHECKMARKS | MIIM_FTYPE | MIIM_DATA | MIIM_STATE | MIIM_STRING;

		if ( ::GetMenuItemInfo( handle, wrapper->index, TRUE, & info ) == FALSE )
			throw xCeption ( _T( "Couldn't get menu item info in drawItem()" ) );

		// check if item is owner drawn
		xAssert( ( info.fType & MFT_OWNERDRAW ) != 0, _T( "Not a owner - drawn item in drawItem()" ) )

		// get item data
		MenuItemDataPtr data ( wrapper->data );
		xAssert( data != 0, _T( "Couldn't find item data in drawItem()" ) )

		// get state info
		bool isGrayed = ( drawInfo.itemState & ODS_GRAYED ) == ODS_GRAYED;
		bool isChecked = ( drawInfo.itemState & ODS_CHECKED ) == ODS_CHECKED;
		bool isDisabled = ( drawInfo.itemState & ODS_DISABLED ) == ODS_DISABLED;
		bool isSelected = ( drawInfo.itemState & ODS_SELECTED ) == ODS_SELECTED;
		bool isHighlighted = ( drawInfo.itemState & ODS_HOTLIGHT ) == ODS_HOTLIGHT;

		// this will contain item image
		HBITMAP image = NULL;

		// if checked/unchecked image is avaiable
		if ( ( info.hbmpChecked != NULL ) && ( info.hbmpUnchecked != NULL ) )
			image = isChecked ? info.hbmpChecked : info.hbmpUnchecked;
		else // get normal image
			image = data->Image->getBitmap();

		// this will contain image size
		Point imageSize = data->Image->getBitmapSize();

		if ( ( imageSize.x == 0 ) && ( imageSize.y == 0 ) ) // no image
			imageSize = defaultImageSize; // set default image size

		// compute strip width
		int stripWidth = imageSize.x + textIconGap;

		// prepare item rectangle
		Rectangle itemRectangle( drawInfo.rcItem.left, drawInfo.rcItem.top, // position
		drawInfo.rcItem.right - drawInfo.rcItem.left, // width
		drawInfo.rcItem.bottom - drawInfo.rcItem.top ); // height

		// setup buffered canvas
		BufferedCanvas< FreeCanvas > canvas( menu->handle(), drawInfo.hDC );

		// this will conain adjusted sidebar width
		int sidebarWidth = 0;

		// this will contain logical information
		// about title font
		LOGFONT lf;
		memset( & lf, 0, sizeof( LOGFONT ) );

		// this will contain adjusted(rotated) title font for sidebar
		HFONT titleFont = NULL;

		// get title font info and adjust item rectangle
		if ( menu->drawSidebar )
		{
			// get title font
			std::tr1::shared_ptr< Font > font = menu->itsTitleFont;

			// get logical info for title font
			::GetObject( font->getHandle(), sizeof( LOGFONT ), & lf );

			// 90 degree rotation and bold
			lf.lfOrientation = lf.lfEscapement = 900;

			// create title font from logical info
			titleFont = ::CreateFontIndirect( & lf );

			// get title text size
			SIZE textSize;
			memset( & textSize, 0, sizeof( SIZE ) );

			HGDIOBJ oldFont = ::SelectObject( canvas.getDc(), titleFont );
			::GetTextExtentPoint32( canvas.getDc(), menu->itsTitle.c_str(), ( int ) menu->itsTitle.size(), & textSize );
			::SelectObject( canvas.getDc(), oldFont );

			// set sidebar width to text height
			sidebarWidth = textSize.cy;

			// adjust item rectangle and item background
			itemRectangle.pos.x += sidebarWidth;
			itemRectangle.size.x -= sidebarWidth;
		}

		// draw sidebar with menu title
		if ( ( drawInfo.itemAction & ODA_DRAWENTIRE ) && ( menu->drawSidebar ) && !menu->itsTitle.empty() )
		{
			// select title font and color
			HGDIOBJ oldFont = ::SelectObject ( canvas.getDc(), titleFont );
			COLORREF oldColor = canvas.setTextColor( colorInfo.colorTitleText );

			// set background mode to transparent
			bool oldMode = canvas.setBkMode( true );

			// get rect for sidebar
			RECT rect;
			::GetClipBox( drawInfo.hDC, & rect );
			//rect.left -= borderGap;

			// set title rectangle
			Rectangle textRectangle( 0, 0, sidebarWidth, rect.bottom - rect.top );

			// draw background
			Brush brush( canvas, colorInfo.colorTitle );
			canvas.fillRectangle( textRectangle, brush );

			// draw title
			textRectangle.pos.y += 10;
			canvas.drawText( menu->itsTitle, textRectangle, DT_BOTTOM | DT_SINGLELINE );

			// clear
			canvas.setTextColor( oldColor );
			canvas.setBkMode( oldMode );

			// set back old font
			::SelectObject( canvas.getDc(), oldFont );
		}

		// destroy title font
		::DeleteObject( titleFont );

		// set item background
		if ( wrapper->isMenuTitleItem ) // for title
		{
			Brush brush( canvas, colorTitle );
			canvas.fillRectangle( itemRectangle, brush );

			// draw raised border
			RECT rc( itemRectangle );
			::DrawEdge( canvas.getDc(), & rc, EDGE_RAISED, BF_RECT );
		}
		else // for normal items
		{
			Brush brush( canvas, colorMenuDraw );
			canvas.fillRectangle( itemRectangle, brush );
		}

		if ( isMenuBar && isSelected ) // draw selected menu bar item
		{
			// TODO: Simulate shadow

			// select pen for drawing broder
			// and brush for filling item
			COLORREF colorBorder = 0;
			Pen pen ( canvas, colorBorder );
			Brush brush ( canvas, ColorUtilities::lightenColor( colorMenuBar, 0.5 ) );

			canvas.rectangle( itemRectangle );
		} // end if
		else if ( ( isSelected || isHighlighted ) && !isDisabled ) // draw selected or highlighted menu item (if not inactive)
		{
			// select pen for drawing broder
			// and brush for filling item
			Pen pen ( canvas, colorInfo.colorHighlight );
			Brush brush ( canvas, colorFillHighlighted );

			canvas.rectangle( itemRectangle );
		} // end if
		else if ( !isMenuBar && !wrapper->isMenuTitleItem ) // draw strip bar for menu items (except menu title item)
		{
			// create rectangle for strip bar
			Rectangle stripRectangle ( itemRectangle );
			stripRectangle.size.x = stripWidth;

			// draw strip bar
			Brush brush( canvas, colorInfo.colorStrip );
			canvas.fillRectangle( stripRectangle, brush );
		} // end if

		if ( !isMenuBar && info.fType & MFT_SEPARATOR ) // draw separator
		{
			// set up separator rectangle
			Rectangle rectangle ( itemRectangle );

			// center in the item rectangle
			rectangle.pos.x += stripWidth + textIconGap;
			rectangle.pos.y += rectangle.size.y / 2 - 1;

			// select color
			Pen pen( canvas, ::GetSysColor( COLOR_GRAYTEXT ) );

			// draw separator
			canvas.moveTo( rectangle.pos.x, rectangle.pos.y );
			canvas.lineTo( rectangle.size.x, rectangle.pos.y );
		} // end if
		else // not a seperator, then draw item text and icon
		{
			// get item text
			const int length = info.cch + 1;
			std::vector< TCHAR > buffer( length );
			int count = ::GetMenuString( handle, wrapper->index, & buffer[0], length, MF_BYPOSITION );
			SmartUtil::tstring itemText( buffer.begin(), buffer.begin() + count );

			// index will contain accelerator position
			size_t index = itemText.find_last_of( _T( '\t' ) );

			// split item text to draw accelerator correctly
			SmartUtil::tstring text = itemText.substr( 0, index );

			// get accelerator
			SmartUtil::tstring accelerator;

			if ( index != itemText.npos )
				accelerator = itemText.substr( index + 1 );

			// set mode to transparent
			bool oldMode = canvas.setBkMode( true );

			// select item text color
			canvas.setTextColor( isGrayed ? ::GetSysColor( COLOR_GRAYTEXT ) : wrapper->isMenuTitleItem ? colorInfo.colorTitleText : data->TextColor );

			// Select item font if available
			FontPtr font( data->Font );

			HGDIOBJ oldFont = ::SelectObject( canvas.getDc(), font->getHandle() );

			if ( !isMenuBar && !wrapper->isMenuTitleItem && !itemText.empty() ) // if menu item
			{
				// compute text rectangle
				Rectangle textRectangle( itemRectangle );

				// adjust rectangle
				textRectangle.pos.x += stripWidth + textIconGap;
				textRectangle.size.x -= stripWidth + textIconGap + borderGap;

				canvas.drawText( text, textRectangle, DT_LEFT | DT_VCENTER | DT_SINGLELINE );

				// draw accelerator
				if ( !accelerator.empty() )
					canvas.drawText( accelerator, textRectangle, DT_RIGHT | DT_VCENTER | DT_SINGLELINE );
			} // end if
			else if ( !itemText.empty() ) // draw menu bar item text
			{
				Rectangle textRectangle( itemRectangle );

				if ( image != NULL ) // has icon
					textRectangle.pos.x += textIconGap;

				canvas.drawText( text, textRectangle, DT_CENTER | DT_VCENTER | DT_SINGLELINE );
			} // end if

			// set back old font
			::SelectObject( canvas.getDc(), oldFont );

			// reset old mode
			canvas.setBkMode( oldMode );

			// set up image rectangle
			Rectangle imageRectangle( itemRectangle.pos, imageSize );

			// adjust icon rectangle
			imageRectangle.pos.x += ( stripWidth - imageSize.x ) / 2;
			imageRectangle.pos.y += ( itemRectangle.size.y - imageSize.y ) / 2;

			if ( image == NULL ) // drawing item without icon
			{
				if ( isChecked ) // needs checkmark
				{
					// draw the check mark or radio bullet
					// prepare background
					Brush brush( canvas, colorInfo.colorStrip );
					canvas.fillRectangle( imageRectangle, brush );

					// create memory DC and set bitmap on it
					HDC memoryDC = ::CreateCompatibleDC( canvas.getDc() );
					HGDIOBJ old = ::SelectObject( memoryDC, ::CreateCompatibleBitmap( canvas.getDc(), imageSize.x, imageSize.y ) );

					// draw into memory
					RECT rc( Rectangle( 0, 0, imageSize.x, imageSize.y ) );
					::DrawFrameControl( memoryDC, & rc, DFC_MENU, ( info.fType & MFT_RADIOCHECK ) == 0 ? DFCS_MENUCHECK : DFCS_MENUBULLET );

					const int adjustment = 2; // adjustment for mark to be in the center

					// bit - blast into out canvas
					::BitBlt( canvas.getDc(), imageRectangle.pos.x + adjustment, imageRectangle.pos.y, imageSize.x, imageSize.y, memoryDC, 0, 0, SRCAND );

					// delete memory dc
					::DeleteObject( ::SelectObject( memoryDC, old ) );
					::DeleteDC( memoryDC );
				}
			}
			else // drawing item with icon
			{
				if ( isSelected && !isDisabled ) // if selected and active, then imitate icon shadow
				{
					// adjust icon position for later drawing
					imageRectangle.pos.x -= 1;
					imageRectangle.pos.y -= 1;

					// setup brush for shadow emulation
					Brush brush( canvas, ColorUtilities::darkenColor( colorInfo.colorStrip, 0.7 ) );

					// draw the icon shadow
					Rectangle shadowRectangle( imageRectangle );
					shadowRectangle.pos.x += 2;
					shadowRectangle.pos.y += 2;
					canvas.drawBitmap( image, shadowRectangle, colorInfo.colorImageBackground, true );
				}

				// draw normal icon
				canvas.drawBitmap( image, imageRectangle, colorInfo.colorImageBackground, isGrayed );
			}

			if ( isChecked ) // draw surrounding rectangle for checked items
			{
				/*if ( image != NULL ) // adjust for icon
				iconRectangle = iconRectangle.shrink( 1.20 );*/

				// draw the surrounding rectangle
				Pen pen( canvas, colorInfo.colorHighlight );
				canvas.line( imageRectangle );
			}
		}

		// blast buffer into screen
		if ( ( drawInfo.itemAction & ODA_DRAWENTIRE ) && menu->drawSidebar ) // adjustment for sidebar
		{
			itemRectangle.pos.x -= sidebarWidth;
			itemRectangle.size.x += sidebarWidth;
		}

		canvas.blast( itemRectangle );
		return true;
	}
};

// Since this is a Dispatcher ONLY for Menus it CAN'T be a "non control", therefore
// we DON'T need the partial specialization for it...
template< class EventHandlerClass, class WidgetType, class MessageMapType >
class WidgetMenuExtendedDispatcher
{
	static EventHandlerClass * getParent( Widget * parentGiven )
	{
		Widget * tmpParent = parentGiven;
		EventHandlerClass * ptrMainParent;
		while ( true )
		{
			ptrMainParent = dynamic_cast< EventHandlerClass * >( tmpParent );
			if ( ptrMainParent != 0 )
				break;
			tmpParent = tmpParent->getParent();
			if ( 0 == tmpParent )
				throw xCeption( _T( "Serious error while trying to get MainWindow parent to menu, menu probably not attached to Main WidgetFactory..." ) );
		}
		return ptrMainParent;
	}

public:
#ifdef PORT_ME
	static HRESULT dispatch( private_::SignalContent & params )
	{
		EventHandlerClass * ptrMainParent = getParent( params.This->getParent() );
		typename WidgetType::menuExtendedVoidFunctionTakingUInt func =
			reinterpret_cast< typename WidgetType::menuExtendedVoidFunctionTakingUInt >( params.Function );

		StayAliveDeleter< WidgetType > deleter;
		std::tr1::shared_ptr< WidgetType > ptrThis( boost::polymorphic_cast< WidgetType * >( params.This ), deleter );

		func
			( dynamic_cast< EventHandlerClass * >( 0 )
			, ptrThis
			, LOWORD( params.Msg.WParam )
			);
		return 1;
	}

	static HRESULT dispatchThis( private_::SignalContent & params )
	{
		EventHandlerClass * ptrMainParent = getParent( params.This->getParent() );
		typename WidgetType::itsVoidMenuExtendedFunctionTakingUInt func =
			reinterpret_cast< typename WidgetType::itsVoidMenuExtendedFunctionTakingUInt >( params.FunctionThis );

		StayAliveDeleter< WidgetType > deleter;
		std::tr1::shared_ptr< WidgetType > ptrThis( boost::polymorphic_cast< WidgetType * >( params.This ), deleter );

		( ( * boost::polymorphic_cast< EventHandlerClass * >( ptrMainParent ) ).*func )
			( ptrThis
			, LOWORD( params.Msg.WParam )
			);
		return 1;
	}

	static HRESULT dispatchDrawItem( private_::SignalContent & params )
	{
		// get callback
		typename WidgetType::boolDrawItemFunction func =
			reinterpret_cast< typename WidgetType::boolDrawItemFunction >( params.Function );

		StayAliveDeleter< WidgetType > deleter;
		std::tr1::shared_ptr< WidgetType > ptrThis( boost::polymorphic_cast< WidgetType * >( params.This ), deleter );

		// call the callback
		bool handled = func
			( dynamic_cast< EventHandlerClass * >( params.This->getParent() )
			, ptrThis
			, params.Msg.WParam // control id
			, * reinterpret_cast< DRAWITEMSTRUCT * >( params.Msg.LParam )
			);
		return handled;
	}

	static HRESULT dispatchDrawItemThis( private_::SignalContent & params )
	{
		// get method pointer
		typename WidgetType::itsBoolDrawItemFunction func =
			reinterpret_cast< typename WidgetType::itsBoolDrawItemFunction >( params.FunctionThis );

		StayAliveDeleter< WidgetType > deleter;
		std::tr1::shared_ptr< WidgetType > ptrThis( boost::polymorphic_cast< WidgetType * >( params.This ), deleter );

		// call method
		bool handled = ( ( * boost::polymorphic_cast< EventHandlerClass * >( params.This->getParent() ) ).*func )
			( ptrThis
			, params.Msg.WParam // control id
			, * reinterpret_cast< DRAWITEMSTRUCT * >( params.Msg.LParam )
			);
		return handled;
	}

	static HRESULT dispatchMeasureItem( private_::SignalContent & params )
	{
		// get callback
		typename WidgetType::boolMeasureItemFunction func =
			reinterpret_cast< typename WidgetType::boolMeasureItemFunction >( params.Function );

		StayAliveDeleter< WidgetType > deleter;
		std::tr1::shared_ptr< WidgetType > ptrThis( boost::polymorphic_cast< WidgetType * >( params.This ), deleter );

		// call the callback
		bool handled = func
			( dynamic_cast< EventHandlerClass * >( params.This->getParent() )
			, ptrThis
			, reinterpret_cast< MEASUREITEMSTRUCT * >( params.Msg.LParam )
			);
		return handled;
	}

	static HRESULT dispatchMeasureItemThis( private_::SignalContent & params )
	{
		// get method pointer
		typename WidgetType::itsBoolMeasureItemFunction func =
			reinterpret_cast< typename WidgetType::itsBoolMeasureItemFunction >( params.FunctionThis );

		StayAliveDeleter< WidgetType > deleter;
		std::tr1::shared_ptr< WidgetType > ptrThis( boost::polymorphic_cast< WidgetType * >( params.This ), deleter );

		// call method
		bool handled = ( ( * boost::polymorphic_cast< EventHandlerClass * >( params.This->getParent() ) ).*func )
			( ptrThis
			, reinterpret_cast< MEASUREITEMSTRUCT * >( params.Msg.LParam )
			);
		return handled;
	}

	static HRESULT dispatchPopup( private_::SignalContent & params )
	{
		// get callback
		typename WidgetType::voidPopupFunction func =
			reinterpret_cast< typename WidgetType::voidPopupFunction >( params.Function );

		StayAliveDeleter< WidgetType > deleter;
		std::tr1::shared_ptr< WidgetType > ptrThis( boost::polymorphic_cast< WidgetType * >( params.This ), deleter );

		// call the callback
		func
			( dynamic_cast< EventHandlerClass * >( params.This->getParent() )
			, ptrThis
			);
		return 0;
	}

	static HRESULT dispatchPopupThis( private_::SignalContent & params )
	{
		// get method pointer
		typename WidgetType::itsVoidPopupFunction func =
			reinterpret_cast< typename WidgetType::itsVoidPopupFunction >( params.FunctionThis );

		StayAliveDeleter< WidgetType > deleter;
		std::tr1::shared_ptr< WidgetType > ptrThis( boost::polymorphic_cast< WidgetType * >( params.This ), deleter );

		// call method
		( ( * boost::polymorphic_cast< EventHandlerClass * >( params.This->getParent() ) ).*func )
			( ptrThis
			);
		return 0;
	}
#endif
};

// Menu Renderer static data members initialization
template< class MenuType >
Point DefaultMenuRenderer< MenuType >::defaultImageSize = Point( 16, 16 );

// Platform specific implementation
template< class MenuType, Platform >
class WidgetMenuExtendedPlatformImplementation;

/// Specialized functions in menu for desktop Windows API version
/** This class contains all the functions in the WidgetMenuExtended which only works
  * in the Desktop Version of the OS. <br>
  * Though WidgetMenuExtended class does not actually WORK on WinCE we plan to MAKE
  * it work in future versions, therefore we have created the CurrentPlatform
  * specialization classes for it here...!!
  */
template< class MenuType >
class WidgetMenuExtendedPlatformImplementation< MenuType, SmartWinDesktop > 
{
	// friends
	friend class DefaultMenuRenderer< MenuType >;
public:

	typedef std::tr1::shared_ptr< MenuType > WidgetMenuExtendedPtr;
	typedef WidgetMenuExtendedDispatcher< MenuType > Dispatcher;

	/// Attaches the menu to a parent window
	void attach( EventHandlerClass * mainWindow );

	/// Actually creates the menu
	/** Creates the menu, the menu will be created initially empty!
	  */
	void create( bool isPopup = false );

	/// Actually creates the menu
	/** Copies the menu if copy is true, otherwise just hooks it
	  */
	void create( HMENU source, bool copy );

	/// Appends a popup to the menu
	/** Everything you "append" to a menu is added sequentially to the menu <br>
	  * This specific "append" function appends a "popup" menu which is a menu
	  * containing other menus. <br>
	  * With other words a menu which is not an "option" but rather a new "subgroup".
	  * <br>
	  * The "File" menu of most application is for instance a "popup" menu while the
	  * File/Print is often NOT a popup. <br>
	  * To append items to the popup created call one of the appendItem overloaded
	  * functions on the returned value of this function. <br>
	  * Also, although references to all menu objects must be kept ( since they're
	  * not collected automatically like other Widgets ) <br>
	  * you don't have to keep a reference to the return value of this function since
	  * it's being added as a reference to the children list of the "this" object.
	  * <br>
	  * A popup is basically another branch in the menu hierarchy <br>
	  * See the WidgetMenu project for a demonstration.
	  */
	WidgetMenuExtendedPtr appendPopup( const SmartUtil::tstring & text, MenuItemDataPtr itemData = MenuItemDataPtr( new MenuItemData() ) );

	/// Returns the "System Menu"
	/** The system menu is a special menu that ( normally ) is accessed by pressing
	  * the "window icon" at the top left of the window. <br>
	  * In SmartWin++ this menu can ALSO be easily manipulated and added items to
	  * etc... <br>
	  * Also, although references to all menu objects must be kept ( since they're
	  * not collected automatically like other Widgets ) <br>
	  * you don't have to keep a reference to the return value of this function since
	  * it's being added as a reference to the children list <br>
	  * of the "this" object. <br>
	  * See the WidgetMenu sample project for a demonstration.
	  */
	WidgetMenuExtendedPtr getSystemMenu();

protected:
	// Initializes menu with given handle
	void init( HMENU handle );

	// its sub menus
	std::vector< WidgetMenuExtendedPtr > itsChildren;

	// its item data
	std::vector < private_::ItemDataWrapper * > itsItemData;
};

/// Extended Menu class
/** \ingroup WidgetControls
  * \WidgetUsageInfo
  * \image html menuextended.png
  * Class for creating an Extended Menu Control which then can be attached to e.g. a
  * WidgetWindow. <br>
  * Note for Desktop version only! <br>
  * After you have created a menu you must call WidgetMenu::attach() to make it
  * "attach" to the WidgetWindow you want it to belong to. <br>
  * Do not be fooled, a WidgetMenuExtended is a much more advanced menu type then the
  * "normal" WidgetMenu and contains support for visualizations far beyond the
  * capabilities of the WidgetMenu. <br>
  * If you need those truly awesome visual menu effects use this menu control instead
  * of the WidgetMenu.
  */
class WidgetMenuExtended :
	public WidgetMenuExtendedPlatformImplementation< CurrentPlatform >
{
	// friends
	friend class DefaultMenuRenderer< WidgetMenuExtended >;
	friend class WidgetMenuExtendedPlatformImplementation< CurrentPlatform >;
	friend class WidgetCreator< WidgetMenuExtended >;

	typedef WidgetMenuExtendedPlatformImplementation< CurrentPlatform > Implementation;
	typedef SmartWin::DefaultMenuRenderer< WidgetMenuExtended > DefaultMenuRenderer;
	typedef typename WidgetMenuExtendedPlatformImplementation< CurrentPlatform >::Dispatcher Dispatcher;
public:
	/// Type of object
	typedef WidgetMenuExtended ThisType;

	/// Object type
	typedef typename WidgetMenuExtendedPlatformImplementation< CurrentPlatform >::WidgetMenuExtendedPtr ObjectType;

	/// Creational info
	//TODO: empty because it is not used anywhere ...
	class Seed
	{};

	// Event Handlers signature typedefs

	/// \ingroup eventsSignatures
	/// \typedef Typedef of a member function to the original class taking pointer to the this Widget and an unsigned int returning void
	typedef void ( EventHandlerClass::* itsVoidMenuExtendedFunctionTakingUInt )( ObjectType, unsigned );

	/// \ingroup eventsSignatures
	/// Typedef of a static/global function taking a pointer to the original class, a pointer to the this Widget class and an unsigned int returning void
	typedef void ( * menuExtendedVoidFunctionTakingUInt )( EventHandlerClass *, ObjectType, unsigned );

	/// \ingroup eventsSignatures
	/// \typedef Typedef of a member function to the original class taking pointer to the this Widget and an unsigned int returning void
	typedef bool ( EventHandlerClass::* itsBoolDrawItemFunction )( ObjectType, int, const DRAWITEMSTRUCT & );

	/// \ingroup eventsSignatures
	/// Typedef of a static/global function taking a pointer to the original class, a pointer to the this Widget class and an unsigned int returning void
	typedef bool ( * boolDrawItemFunction )( EventHandlerClass *, ObjectType, int, const DRAWITEMSTRUCT & );

	/// \ingroup eventsSignatures
	/// \typedef Typedef of a member function to the original class taking pointer to the this Widget and an unsigned int returning void
	typedef bool ( EventHandlerClass::* itsBoolMeasureItemFunction )( ObjectType, MEASUREITEMSTRUCT * );

	/// \ingroup eventsSignatures
	/// Typedef of a static/global function taking a pointer to the original class, a pointer to the this Widget class and an unsigned int returning void
	typedef bool ( * boolMeasureItemFunction )( EventHandlerClass *, ObjectType, MEASUREITEMSTRUCT * );

	/// \ingroup eventsSignatures
	/// \typedef Typedef of a member function to the original class taking pointer to the this Widget returning void
	typedef void ( EventHandlerClass::* itsVoidPopupFunction )( ObjectType );

	/// \ingroup eventsSignatures
	/// Typedef of a static/global function taking a pointer to the original class, a pointer to the this Widget class returning void
	typedef void ( * voidPopupFunction )( EventHandlerClass *, ObjectType );

	// Overriden to set default drawing
	void create( bool isPopup = false );

	// Overriden to set default drawing
	void create( HMENU source, bool copy );

	/// Setting event handler for Draw Item Event
	/** The Draw Item Event will be raised when the menu needs to draw itself, if you
	  * wish to truly be creative and be 100% in control you must handle this Event
	  * and do the actualy drawing of the Menu yourself, but for most people it will
	  * be enough to just manipulate the background colors etc of the MenuItemData
	  * given to the menu in the appendItem or to call the setColorInfo function <br>
	  * Note! <br>
	  * If this event is handled you also MUST handle the Measure Item Event!!
	  */
	void onDrawItem( boolDrawItemFunction eventHandler );
	void onDrawItem( itsBoolDrawItemFunction eventHandler );

	/// Setting event handler for Draw Item Event
	/** The Measure Item Event is nessecary to handle if you want to draw the menu
	  * yourself since it is inside this Event Handler you're telling the system how
	  * much space you need to actually do the drawing <br>
	  * Note! <br>
	  * If this event is handled you also MUST handle the Draw Item Event!!
	  */
	void onMeasureItem( boolMeasureItemFunction eventHandler );
	void onMeasureItem( itsBoolMeasureItemFunction eventHandler );

	/// Sets the event handler for the Popup event
	void onPopup( voidPopupFunction eventHandler );

	/// Sets the event handler for the Popup event
	void onPopup( itsVoidPopupFunction eventHandler );

	/// Appends a separator item to the menu
	/** A menu separator is basically just "air" between menu items.< br >
	  * A separator cannot be "clicked" or "chosen".
	  */
	void appendSeparatorItem();

	/// Appends a Menu Item
	/** eventHandler is the function that will receive the "click" event from the
	  * menu item. <br>
	  * Event handler's signature must be "void foo( WidgetMenuExtendedPtr, unsigned
	  * int )" and it must be contained as a member <br>
	  * of the class that is defined as the EventHandlerClass, normally either the
	  * WidgetWindow derived class or the class derived from WidgetMenu. <br>
	  * See e.g. WidgetMenu for an example. <br>
	  * The reason to why we have this "id" is because the same event handler can be
	  * defined for several menu items even in fact across menu objects, therefore
	  * this number should be unique across the application.
	  */
	void appendItem( unsigned itemID, const SmartUtil::tstring & text, MenuItemDataPtr itemData, itsVoidMenuExtendedFunctionTakingUInt eventHandler );
	void appendItem( unsigned itemID, const SmartUtil::tstring & text, MenuItemDataPtr itemData, menuExtendedVoidFunctionTakingUInt eventHandler );

	/// Removes specified item from this menu
	/** Call this function to actually DELETE a menu item from the menu hierarchy.
	  * Note that you have to specify the item position; and whenever you remove an item,
	  * all subsequent items change positions. To remove a range of items, remove from
	  * end to start.
	  */
	void removeItem( unsigned itemIndex );

	/// Remove all items from the menu
	/** Will also delete any submenus.
	  */
	void removeAllItems();

	/// Return the number of items in the menu
	int getCount();

	/// Displays and handles a menu which can appear anywhere in the window.
	/** Typically called by a Right Mouse click. If both the x and the y coordinate
	  * is - 1 ( default ) it'll show the context menu on the position the mouse was
	  * at when the system last recieved a message, basically the "right" place...
	  * <br>
	  * Depending on the flags it might return the id of the menu item selected, or 0
	  * if none was chosen. Flags with TPM_RETURNCMD will return the menu - item, but
	  * not do the menu command.
	  * < ul >
	  * < li >TPM_CENTERALIGN : Centers the shortcut menu horizontally relative to the coordinate specified by the x parameter< /li >
	  * < li >TPM_LEFTALIGN : Function positions the shortcut menu so that its left side is aligned with the coordinate specified by the x parameter< /li >
	  * < li >TPM_RIGHTALIGN : Opposite of LEFTALIGN< /li >
	  * < li >TPM_BOTTOMALIGN : Aligns menu bottoms to the coordinate specified by the y parameter< /li >
	  * < li >TPM_TOPALIGN : Opposite of BOTTOMALIGN< /li >
	  * < li >TPM_VCENTERALIGN : Centers vertically relative to the y parameter< /li >
	  * < li >TPM_NONOTIFY : Restricts the menu from sending notifications when user clicks item< /li >
	  * < li >TPM_RETURNCMD : returns the menu item identifier of the user's selection in the return value but DOES NOT carry out the event handler< /li >
	  * < li >TPM_LEFTBUTTON  : Restricts users to selecting menu items with only left mouse button< /li >
	  * < li >TPM_RIGHTBUTTON : User can choose menu item with both mouse buttons< /li >
	  * < /ul >
	  * None of the following are used by default but can be manually chosen if you
	  * manually call SystemParametersInfo
	  * < ul >
	  * < li >TPM_HORNEGANIMATION : Animates the menu from right to left< /li >
	  * < li >TPM_HORPOSANIMATION : Animates the menu from left to right< /li >
	  * < li >TPM_NOANIMATION : Displays menu without animation< /li >
	  * < li >TPM_VERNEGANIMATION : Animates the menu from bottom to top< /li >
	  * < li >TPM_VERPOSANIMATION : Animates the menu from top to bottom< /li >
	  * < /ul >
	  */
	unsigned trackPopupMenu( EventHandlerClass * mainWindow, int x = - 1, int y = - 1, unsigned flags = 0 );

	/// Sets menu title
	/** A WidgetMenuExtended can have a title, this function sets that title
	  */
	void setTitle( const SmartUtil::tstring & title, bool drawSidebar = false );

	/// Sets title font
	/** Create a font through e.g. createFont in WidgetFactory or similar and set the
	  * title font to the menu title through using this function
	  */
	void setTitleFont( FontPtr font );

	/// Removes menu title
	/** If clearSidebar is true, sidebar is removed
	  */
	void clearTitle( bool clearSidebar = false );

	/// Enables or disables menu item
	/** If the second argument is true the menu item will be enabled, otherwise it'll
	  * be disabled
	  */
	void enableItem( unsigned itemID, bool setEnabled );

	/// Set item state to checked/unchecked
	/** If second parameter is true the menu item will be checked, otherwise it'll be
	  * unchecked
	  */
	void checkItem( unsigned itemID, bool setChecked, bool radioMark );

	/// Returns true if item is checked
	bool isItemEnabled( unsigned itemID );

	/// Returns true if item is checked
	bool isItemChecked( unsigned itemID );

	/// Returns true if menu is "system menu" (icon in top left of window)
	bool isSystemMenu()
	{
		return isSysMenu;
	}

	/// Returns item text
	SmartUtil::tstring getItemText( unsigned itemID );

	/// Sets item text
	void setItemText( unsigned itemID, SmartUtil::tstring text );

	/// Sets color information for the menu
	/** The MenuColorInfo declares which colors will be used for drawing the menu (
	  * items ) <br>
	  * Have no effect if you override the onDrawItem/onMeasureItem
	  */
	void setColorInfo( const MenuColorInfo & info );

	/// Returns menu color information
	MenuColorInfo getColorInfo();

	/// Returns item data
	MenuItemDataPtr getData( int itemIndex );

	virtual ~WidgetMenuExtended();

protected:
	/// Constructor Taking pointer to parent
	explicit WidgetMenuExtended( SmartWin::Widget * parent );

	// This is used during menu destruction
	static void destroyItemDataWrapper( private_::ItemDataWrapper * wrapper );

	// True is menu is "system menu" (icon in top left of window)
	bool isSysMenu;

	// Menu title
	SmartUtil::tstring itsTitle;

	// Menu title font
	FontPtr itsTitleFont;

	// if true title is drawn as sidebar
	bool drawSidebar;

	// Contains information about menu colors
	MenuColorInfo itsColorInfo;

	// work around for gcc
	std::vector< typename WidgetMenuExtendedPlatformImplementation< EventHandlerClass, CurrentPlatform >
																		  ::WidgetMenuExtendedPtr > & itsChildrenRef;
	// work around for gcc
	std::vector < private_::ItemDataWrapper * > & itsItemDataRef;

private:
	// Returns item index in the menu item list
	// If no item with specified id is found, - 1 is returned
	int getItemIndex( unsigned itemID );

	// Sets event handler for specified item (to process WM_COMMAND)
	void setItemCommandHandler( unsigned itemID, menuExtendedVoidFunctionTakingUInt eventHandler );
	void setItemCommandHandler( unsigned itemID, itsVoidMenuExtendedFunctionTakingUInt eventHandler );

	// TODO: Basically we have a copy constructor which is create( HMENU, bool )
	WidgetMenuExtended( const WidgetMenuExtended & ); // Never implemented intentionally
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template< class EventHandlerClass >
void WidgetMenuExtendedPlatformImplementation< EventHandlerClass, SmartWinDesktop >::attach( EventHandlerClass * mainWindow )
{
	// get my handle
	HMENU handle = reinterpret_cast< HMENU >( this->Widget::itsHandle );

	// set menu
	if ( ::SetMenu( mainWindow->handle(), handle ) == FALSE )
		throw xCeption( _T( "Couldn't attach menu to given parent" ) );
}

template< class EventHandlerClass >
void WidgetMenuExtendedPlatformImplementation< EventHandlerClass, SmartWinDesktop >::create( bool isPopup )
{
	HMENU handle = NULL;

	// Create menu
	if ( isPopup )
		handle = ::CreatePopupMenu();
	else
		handle = ::CreateMenu();

	// init newly created menu
	init( handle );
}

template< class EventHandlerClass >
void WidgetMenuExtendedPlatformImplementation< EventHandlerClass, SmartWinDesktop >::create( HMENU source, bool copy )
{
	if ( !::IsMenu( source ) ) // if source is not a valid menu, cancel
		return;

	if ( copy )
		void create(); // create empty menu
	else
		init( source ); // init with the source

	// get handle to this menu
	HMENU handle = reinterpret_cast< HMENU >( this->Widget::itsHandle );

	// get source item count
	int itemCount = ::GetMenuItemCount( source );

	// variables used in the loop
	int index = 0;
	int length = 0;
	MENUITEMINFO info;
	for ( index = 0; index < itemCount; ++index )
	{
		// init struct for menu item info
		memset( & info, 0, sizeof( MENUITEMINFO ) );
		info.cbSize = sizeof( MENUITEMINFO );

		// set mask
		info.fMask = MIIM_CHECKMARKS | MIIM_DATA | MIIM_STRING | MIIM_FTYPE |
						MIIM_ID | MIIM_STATE | MIIM_SUBMENU;

		if ( ::GetMenuItemInfo( source, index, TRUE, & info ) == FALSE )
			throw xCeption( _T( "Couldn't get menu item info in create()" ) );

		// set item to owner - drawn
		info.fType |= MFT_OWNERDRAW;

		// create item extended info
		MenuItemDataPtr data( new MenuItemData() );
		std::auto_ptr< private_::ItemDataWrapper > wrapper( new private_::ItemDataWrapper( handle, index, data ) );

		// modify item info data
		info.dwItemData = reinterpret_cast< ULONG_PTR >( wrapper.get() );

		// set item text
		length = info.cch + 1;
		boost::scoped_ptr< TCHAR > tmp( new TCHAR[ length ] );
		info.dwTypeData = tmp.get();
		memset( info.dwTypeData, 0, length );
		::GetMenuString( source, index, info.dwTypeData, length, MF_BYPOSITION );

		// process submenus
		if ( info.hSubMenu != NULL )
		{
			// create popup menu
			WidgetMenuExtendedPtr popup ( new WidgetMenuExtended< EventHandlerClass >( this->getParent() ) );
			popup->create( info.hSubMenu, true /* copy */ );

			// get new handle
			info.hSubMenu = reinterpret_cast< HMENU >( popup->handle() );

			// store as child
			itsChildren.push_back( popup );
		}

		// set back item info
		if ( ( copy && ::InsertMenuItem( handle, index, TRUE, & info ) ) || // insert new item or
				( !copy && ::SetMenuItemInfo( handle, index, TRUE, & info ) ) ) // change existing item info
		{
			itsItemData.push_back( wrapper.release() );
		}
		else
			throw xCeption( _T( "Couldn't insert/modify item in create()" ) );
	}
}

template< class EventHandlerClass >
typename WidgetMenuExtendedPlatformImplementation< EventHandlerClass, SmartWinDesktop >::WidgetMenuExtendedPtr
WidgetMenuExtendedPlatformImplementation< EventHandlerClass, SmartWinDesktop >::appendPopup( const SmartUtil::tstring & text, MenuItemDataPtr itemData )
{
	// create popup menu pointer
	WidgetMenuExtendedPtr retVal ( new WidgetMenuExtended< EventHandlerClass >( this->getParent() ) );
	retVal->create( true );

	// init structure for new item
	MENUITEMINFO info;
	memset( & info, 0, sizeof( info ) );
	info.cbSize = sizeof( MENUITEMINFO );

	// set flags
	info.fMask = MIIM_DATA | MIIM_FTYPE | MIIM_SUBMENU |
					MIIM_CHECKMARKS | MIIM_STRING;
	info.fType = MFT_OWNERDRAW;

	// set item text
	info.dwTypeData = const_cast< LPTSTR >( text.c_str() );

	// set sub menu
	info.hSubMenu = reinterpret_cast< HMENU >( retVal->handle() );

	// get my handle
	HMENU handle = reinterpret_cast< HMENU >( this->Widget::itsHandle );

	// get position to insert
	int position = ::GetMenuItemCount( handle );

	// create extended item data
	private_::ItemDataWrapper * data = new private_::ItemDataWrapper( handle, position, itemData );
	info.dwItemData = reinterpret_cast< UINT_PTR >( data );

	// append to this menu at the end
	if ( ::InsertMenuItem( handle, position, TRUE, & info ) )
	{
		itsItemData.push_back( data );
		itsChildren.push_back( retVal );
	}
	return retVal;
}

template< class EventHandlerClass >
typename WidgetMenuExtendedPlatformImplementation< EventHandlerClass, SmartWinDesktop >::WidgetMenuExtendedPtr
WidgetMenuExtendedPlatformImplementation< EventHandlerClass, SmartWinDesktop >::getSystemMenu()
{
	// get system menu for the utmost parent
	HMENU handle = ::GetSystemMenu( internal_::getTypedParentOrThrow < EventHandlerClass * >( this )->handle(), FALSE );

	// create pointer to system menu
	WidgetMenuExtendedPtr sysMenu( new WidgetMenuExtended< EventHandlerClass >( this->getParent() ) );

	// create(take) system menu
	sysMenu->isSysMenu = true;
	sysMenu->create( handle, false );

	// We're assuming that the system menu has the same lifespan as the "this" menu, we must keep a reference to te system menu
	// otherwise it will be "lost", therefore we add it up as a child to the "this" menu...
	itsChildren.push_back( sysMenu );

	return sysMenu;
}

template< class EventHandlerClass >
void WidgetMenuExtendedPlatformImplementation< EventHandlerClass, SmartWinDesktop >::init( HMENU handle )
{
	// check if successful
	if ( !handle || !::IsMenu( handle ) )
		throw xCeption ( _T( "CreateMenu in WidgetMenuExtended::create fizzled..." ) );

	// TODO: Is this safe ? ! ?
	// At least we should verify it...
	BOOST_STATIC_ASSERT( sizeof( HWND ) == sizeof( HMENU ) );
	this->Widget::itsHandle = reinterpret_cast< HWND >( handle );

	// Menu controls uses the Windows Message Procedure of the parent window
	this->MessageMapType::itsDefaultWindowProc = NULL;

	// Register the menu
	this->Widget::registerWidget();
}

template< class EventHandlerClass >
void WidgetMenuExtended< EventHandlerClass >::setColorInfo( const MenuColorInfo & info )
{
	itsColorInfo = info;
}

template< class EventHandlerClass >
	MenuColorInfo WidgetMenuExtended< EventHandlerClass >::getColorInfo()
{
	return itsColorInfo;
}

template< class EventHandlerClass >
int WidgetMenuExtended< EventHandlerClass >::getItemIndex( unsigned itemID )
{
	int index = 0;
	HMENU handle = reinterpret_cast< HMENU >( this->Widget::itsHandle );
	const int itemCount = ::GetMenuItemCount( handle );

	for ( index = 0; index < itemCount; ++index )
		if ( ::GetMenuItemID( handle, index ) == itemID ) // exit the loop if found
			return index;

	return - 1;
}

template< class EventHandlerClass >
MenuItemDataPtr WidgetMenuExtended< EventHandlerClass >::getData( int itemIndex )
{
	size_t i = 0;

	for ( i = 0; i < itsItemDataRef.size(); ++i )
		if ( itsItemDataRef[i]->index == itemIndex )
			return itsItemDataRef[i]->data;

	return MenuItemDataPtr();
}

template< class EventHandlerClass >
WidgetMenuExtended< EventHandlerClass >::~WidgetMenuExtended()
{
	// Destroy this menu
	::DestroyMenu( reinterpret_cast< HMENU >( this->handle() ) );
	std::for_each( itsItemDataRef.begin(), itsItemDataRef.end(), destroyItemDataWrapper );
}

template< class EventHandlerClass >
void WidgetMenuExtended< EventHandlerClass >::destroyItemDataWrapper( private_::ItemDataWrapper * wrapper )
{
	if ( 0 != wrapper )
		delete wrapper;

	wrapper = 0;
}

template< class EventHandlerClass >
void WidgetMenuExtended< EventHandlerClass >::setTitleFont( FontPtr font )
{
	itsTitleFont = font;
	setTitle( itsTitle, this->drawSidebar ); // Easy for now, should be refactored...
}

template< class EventHandlerClass >
void WidgetMenuExtended< EventHandlerClass >::clearTitle( bool clearSidebar /* = false */)
{
	// get my handle
	HMENU handle = reinterpret_cast< HMENU >( this->Widget::itsHandle );

	if ( !clearSidebar && !itsTitle.empty() )
		removeItem( 0 );

	// clear title text
	itsTitle.clear();
}

template< class EventHandlerClass >
void WidgetMenuExtended< EventHandlerClass >::enableItem( unsigned itemID, bool setEnabled )
{
	// get my handle
	HMENU handle = reinterpret_cast< HMENU >( this->Widget::itsHandle );

	if ( ::EnableMenuItem( handle, itemID, MF_BYCOMMAND | setEnabled ? MF_ENABLED : MF_GRAYED ) == -1 )
		throw xCeption( _T( "Couldn't enable/disable item in enableItem()" ) );
}

template< class EventHandlerClass >
void WidgetMenuExtended< EventHandlerClass >::checkItem( unsigned itemID, bool setChecked, bool radioMark )
{
	// get my handle
	HMENU handle = reinterpret_cast< HMENU >( this->Widget::itsHandle );

	::CheckMenuItem( handle, itemID, MF_BYCOMMAND | setChecked ? MF_CHECKED : MF_UNCHECKED );

	if ( radioMark )
		::CheckMenuRadioItem( handle, itemID, itemID, itemID, itemID );
}

template< class EventHandlerClass >
bool WidgetMenuExtended< EventHandlerClass >::isItemEnabled( unsigned itemID )
{
	// get my handle
	HMENU handle = reinterpret_cast< HMENU >( this->Widget::itsHandle );

	// init struct for item info
	MENUITEMINFO info;
	memset( & info, 0, sizeof( MENUITEMINFO ) );
	info.cbSize = sizeof( MENUITEMINFO );

	// set flag
	info.fMask = MIIM_STATE;

	// get item info
	if ( ::GetMenuItemInfo( handle, itemID, FALSE, & info ) == FALSE )
		throw xCeption( _T( "Couldn't get item info in isItemEnabled()" ) );

	return ( info.fState & MFS_ENABLED ) == MFS_ENABLED;
}

template< class EventHandlerClass >
bool WidgetMenuExtended< EventHandlerClass >::isItemChecked( unsigned itemID )
{
	HMENU handle = reinterpret_cast< HMENU >( this->Widget::itsHandle );

	// init struct for item info
	MENUITEMINFO info;
	memset( & info, 0, sizeof( MENUITEMINFO ) );
	info.cbSize = sizeof( MENUITEMINFO );

	// set flag
	info.fMask = MIIM_STATE;

	// get item info
	if ( ::GetMenuItemInfo( handle, itemID, FALSE, & info ) == FALSE )
		throw xCeption( _T( "Couldn't get item info in isItemChecked()" ) );

	return ( info.fState & MF_CHECKED ) == MF_CHECKED;
}

template< class EventHandlerClass >
SmartUtil::tstring WidgetMenuExtended< EventHandlerClass >::getItemText( unsigned itemID )
{
	MENUITEMINFO info;
	memset( & info, 0, sizeof( MENUITEMINFO ) );
	info.cbSize = sizeof( MENUITEMINFO );

	// set flag
	info.fMask = MIIM_STRING;

	if ( ::GetMenuItemInfo( reinterpret_cast< HMENU >( this->Widget::itsHandle ), itemID, FALSE, & info ) == FALSE )
		throw xCeption( _T( "Couldn't get item info in getItemText()" ) );

	boost::scoped_array< TCHAR > buffer( new TCHAR[++info.cch] );
	info.dwTypeData = buffer.get();

	if ( ::GetMenuItemInfo( reinterpret_cast< HMENU >( this->Widget::itsHandle ), itemID, FALSE, & info ) == FALSE )
		throw xCeption( _T( "Couldn't get item info in getItemText()" ) );

	SmartUtil::tstring retVal = info.dwTypeData;
	return retVal;
}

template< class EventHandlerClass >
void WidgetMenuExtended< EventHandlerClass >::setItemText( unsigned itemID, SmartUtil::tstring text )
{
	MENUITEMINFO info;
	memset( & info, 0, sizeof( MENUITEMINFO ) );
	info.cbSize = sizeof( MENUITEMINFO );

	// set flag
	info.fMask = MIIM_STRING;
	info.dwTypeData = (TCHAR*) text.c_str();

	if ( ::SetMenuItemInfo( reinterpret_cast< HMENU >( this->Widget::itsHandle ), itemID, FALSE, & info ) == FALSE )
		throw xCeption( _T( "Couldn't set item info in setItemText()" ) );
}

template< class EventHandlerClass >
void WidgetMenuExtended< EventHandlerClass >::setTitle( const SmartUtil::tstring & title, bool drawSidebar /* = false */)
{
	this->drawSidebar = drawSidebar;
	const bool hasTitle = !itsTitle.empty();

	// set its title
	itsTitle = title;

	if ( !drawSidebar )
	{
		// get handle to the menu
		HMENU handle = ( HMENU ) this->Widget::itsHandle;

		// init struct for title info
		MENUITEMINFO info;
		memset( & info, 0, sizeof( MENUITEMINFO ) );
		info.cbSize = sizeof( MENUITEMINFO );

		// set flags
		info.fMask = MIIM_STATE | MIIM_STRING | MIIM_FTYPE | MIIM_DATA;
		info.fType = MFT_OWNERDRAW;
		info.fState = MF_DISABLED;

		// set title text
		info.dwTypeData = const_cast< LPTSTR >( title.c_str() );

		// created extended info for title item
		MenuItemDataPtr data( new MenuItemData( itsTitleFont ) );
		private_::ItemDataWrapper * wrapper = new private_::ItemDataWrapper( handle, 0, data, true );

		// set item data
		info.dwItemData = ( ULONG_PTR ) wrapper;

		if ( ( !hasTitle && ::InsertMenuItem( handle, 0, TRUE, & info ) ) ||
			 ( hasTitle && ::SetMenuItemInfo( handle, 0, TRUE, & info ) ) )
		{
			size_t i = 0;

			// adjust item data wrappers for all existing items
			for ( i = 0; i < itsItemDataRef.size(); ++i )
				if ( itsItemDataRef[i] )
					++itsItemDataRef[i]->index;

			// push back title
			itsItemDataRef.push_back( wrapper );
		}
	}
}

template< class EventHandlerClass >
void WidgetMenuExtended< EventHandlerClass >::create( bool isPopup )
{
	// call base method
	Implementation::create( isPopup );

	// set default drawing
	onDrawItem( & DefaultMenuRenderer::drawItem );
	onMeasureItem( & DefaultMenuRenderer::measureItem );
}

template< class EventHandlerClass >
void WidgetMenuExtended< EventHandlerClass >::create( HMENU source, bool copy )
{
	// call base method
	Implementation::create( source, copy );

	// set default drawing
	onDrawItem( & DefaultMenuRenderer::drawItem );
	onMeasureItem( & DefaultMenuRenderer::measureItem );
}

#ifdef PORT_ME
template< class EventHandlerClass >
void WidgetMenuExtended< EventHandlerClass >::onDrawItem( boolDrawItemFunction eventHandler )
{
	// get pointer to this message map
	MessageMapType * This = boost::polymorphic_cast< MessageMapType * >( this );

	// setup slot
	This->setCallback
		( typename MessageMapType::SignalTupleType
			( private_::SignalContent
				( Message( WM_DRAWITEM )
				, reinterpret_cast< private_::SignalContent::voidFunctionTakingVoid >( eventHandler )
				, This
				)
			, typename MessageMapType::SignalType
				( typename MessageMapType::SignalType::SlotType( & Dispatcher::dispatchDrawItem )
				)
			)
		);
}

template< class EventHandlerClass >
void WidgetMenuExtended< EventHandlerClass >::onDrawItem( itsBoolDrawItemFunction eventHandler )
{
	// get pointer to this message map
	MessageMapType * This = boost::polymorphic_cast< MessageMapType * >( this );

	// setup slot
	This->setCallback
		( typename MessageMapType::SignalTupleType
			( private_::SignalContent
				( Message( WM_DRAWITEM )
				, reinterpret_cast< itsVoidFunction >( eventHandler )
				, This
				)
			, typename MessageMapType::SignalType
				( typename MessageMapType::SignalType::SlotType( & Dispatcher::dispatchDrawItemThis )
				)
			)
		);
}

template< class EventHandlerClass >
void WidgetMenuExtended< EventHandlerClass >::onMeasureItem( boolMeasureItemFunction eventHandler )
{
	// get this message map
	MessageMapType * This = boost::polymorphic_cast< MessageMapType * >( this );

	// setup slot
	This->setCallback
		( typename MessageMapType::SignalTupleType
			( private_::SignalContent
				( Message( WM_MEASUREITEM )
				, reinterpret_cast< private_::SignalContent::voidFunctionTakingVoid >( eventHandler )
				, This
				)
			, typename MessageMapType::SignalType
				( typename MessageMapType::SignalType::SlotType( & Dispatcher::dispatchMeasureItem )
				)
			)
		);
}

template< class EventHandlerClass >
void WidgetMenuExtended< EventHandlerClass >::onMeasureItem( itsBoolMeasureItemFunction eventHandler )
{
	// get this message map
	MessageMapType * This = boost::polymorphic_cast< MessageMapType * >( this );

	// setup slot
	This->setCallback
		( typename MessageMapType::SignalTupleType
			( private_::SignalContent
				( Message( WM_MEASUREITEM )
				, reinterpret_cast< itsVoidFunction >( eventHandler )
				, This
				)
			, typename MessageMapType::SignalType
				( typename MessageMapType::SignalType::SlotType( & Dispatcher::dispatchMeasureItemThis )
				)
			)
		);
}

template< class EventHandlerClass >
void WidgetMenuExtended< EventHandlerClass >::onPopup( voidPopupFunction eventHandler )
{
	// get pointer to this message map
	MessageMapType * This = boost::polymorphic_cast< MessageMapType * >( this );

	// setup slot
	This->setCallback
		( typename MessageMapType::SignalTupleType
			( private_::SignalContent
				( Message( WM_INITMENUPOPUP )
				, reinterpret_cast< private_::SignalContent::voidFunctionTakingVoid >( eventHandler )
				, This
				)
			, typename MessageMapType::SignalType
				( typename MessageMapType::SignalType::SlotType( & Dispatcher::dispatchPopup )
				)
			)
		);
}

template< class EventHandlerClass >
void WidgetMenuExtended< EventHandlerClass >::onPopup( itsVoidPopupFunction eventHandler )
{
	// get pointer to this message map
	MessageMapType * This = boost::polymorphic_cast< MessageMapType * >( this );

	// setup slot
	This->setCallback
		( typename MessageMapType::SignalTupleType
			( private_::SignalContent
				( Message( WM_INITMENUPOPUP )
				, reinterpret_cast< itsVoidFunction >( eventHandler )
				, This
				)
			, typename MessageMapType::SignalType
				( typename MessageMapType::SignalType::SlotType( & Dispatcher::dispatchPopupThis )
				)
			)
		);
}
#endif

template< class EventHandlerClass >
void WidgetMenuExtended< EventHandlerClass >::appendSeparatorItem()
{
	// init structure for new item
	MENUITEMINFO itemInfo;
	memset( & itemInfo, 0, sizeof( itemInfo ) );
	itemInfo.cbSize = sizeof( MENUITEMINFO );

	// set flags
	itemInfo.fMask = MIIM_DATA | MIIM_FTYPE;
	itemInfo.fType = MFT_OWNERDRAW | MFT_SEPARATOR;

	// get my handle
	HMENU handle = reinterpret_cast< HMENU >( this->Widget::itsHandle );

	// create item data wrapper
	int position = ::GetMenuItemCount( handle );
	private_::ItemDataWrapper * wrapper = new private_::ItemDataWrapper( handle, position, MenuItemDataPtr( new MenuItemData() ) );

	// set fields
	itemInfo.dwItemData = reinterpret_cast< ULONG_PTR >( wrapper );

	if ( ::InsertMenuItem( handle, position, TRUE, & itemInfo ) )
		itsItemDataRef.push_back( wrapper );
}

template< class EventHandlerClass >
void WidgetMenuExtended< EventHandlerClass >::setItemCommandHandler( unsigned itemID, itsVoidMenuExtendedFunctionTakingUInt eventHandler )
{
	// set up slot
	if ( eventHandler != 0 )
	{
		MessageMapType * This = boost::polymorphic_cast< MessageMapType * >( this );
		This->setCallback
			( typename MessageMapType::SignalTupleType
				( private_::SignalContent
					( Message( isSysMenu ? WM_SYSCOMMAND : WM_COMMAND, itemID )
					, reinterpret_cast< itsVoidFunction >( eventHandler )
					, This
					)
				, typename MessageMapType::SignalType
					( typename MessageMapType::SignalType::SlotType( & Dispatcher::dispatchThis )
					)
				)
			);
	}
}

#ifdef PORT_ME
template< class EventHandlerClass >
void WidgetMenuExtended< EventHandlerClass >::setItemCommandHandler( unsigned itemID, menuExtendedVoidFunctionTakingUInt eventHandler )
{
	// set up slot
	if ( eventHandler != 0 )
	{
		MessageMapType * This = boost::polymorphic_cast< MessageMapType * >( this );
		This->setCallback
			( typename MessageMapType::SignalTupleType
				( private_::SignalContent
					( Message( isSysMenu ? WM_SYSCOMMAND : WM_COMMAND, itemID )
					, reinterpret_cast< private_::SignalContent::voidFunctionTakingVoid >( eventHandler )
					, This
					)
				, typename MessageMapType::SignalType
					( typename MessageMapType::SignalType::SlotType( & Dispatcher::dispatch )
					)
				)
			);
	}
}
#endif
template< class EventHandlerClass >
void WidgetMenuExtended< EventHandlerClass >::removeItem( unsigned itemIndex )
{
	// get my handle
	HMENU handle = reinterpret_cast< HMENU >( this->Widget::itsHandle );

	// has sub menus ?
	HWND popup = reinterpret_cast< HWND >( ::GetSubMenu( handle, itemIndex ) );

	// try to remove item
	if ( ::RemoveMenu( handle, itemIndex, MF_BYPOSITION ) == TRUE )
	{
		size_t i = 0;
		private_::ItemDataWrapper * wrapper = 0;
		int itemRemoved = -1;

		for ( i = 0; i < itsItemDataRef.size(); ++i )
		{
			// get current data wrapper
			wrapper = itsItemDataRef[i];

			if ( wrapper->index == int(itemIndex) ) // if found
			{
				itemRemoved = int(i);
				delete wrapper;
				itsItemDataRef[i] = 0;
			}
			else if ( wrapper->index > int(itemIndex) )
				--wrapper->index; // adjust succeeding item indices
		}

		if( itemRemoved != -1 )
			itsItemDataRef.erase( itsItemDataRef.begin() + itemRemoved );

		if ( popup != NULL ) // remove sub menus if any
		{
			for ( i = 0; i < itsChildrenRef.size(); ++i )
			{
				if ( itsChildrenRef[i]->itsHandle == popup )
					itsChildrenRef[i].reset();
			}
		}
	}
	else
		throw xCeption( _T( "Couldn't remove item in removeItem()" ) );
}

template< class EventHandlerClass >
void WidgetMenuExtended< EventHandlerClass >::removeAllItems()
{
	//must be backwards, since bigger indexes change on remove
	for( int i = this->getCount() - 1; i >= 0; i-- )
	{
		this->removeItem( i );
	}
}

template< class EventHandlerClass >
int WidgetMenuExtended< EventHandlerClass >::getCount()
{
	HMENU handle = reinterpret_cast< HMENU >( this->Widget::itsHandle );
	int count = ::GetMenuItemCount( handle );
	if( count == -1 )
		throw xCeption( _T( "Couldn't get item count in getCount()" ) );
	return count;
}

template< class EventHandlerClass >
void WidgetMenuExtended< EventHandlerClass >::appendItem(
	unsigned itemID,
	const SmartUtil::tstring & text, MenuItemDataPtr itemData,
	itsVoidMenuExtendedFunctionTakingUInt eventHandler )
{
	// init structure for new item
	MENUITEMINFO info;
	memset( & info, 0, sizeof( info ) );
	info.cbSize = sizeof( MENUITEMINFO );

	// set flags
	info.fMask = MIIM_DATA | MIIM_FTYPE | MIIM_CHECKMARKS | MIIM_ID | MIIM_STRING;
	info.fType = MFT_OWNERDRAW;

	// set fields
	xAssert( !isSysMenu || itemID < SC_SIZE, _T( "Can't add sysmenu item with that high value, value can not be higher then SC_SIZE - 1" ) );
	info.wID = itemID;

	// set text
	info.dwTypeData = const_cast< LPTSTR >( text.c_str() );

	// get my handle
	HMENU handle = reinterpret_cast< HMENU >( this->Widget::itsHandle );

	// find item index
	int index = getItemIndex( itemID );

	// set position to insert
	bool itemExists = index != - 1;
	index = itemExists ? index : ::GetMenuItemCount( handle );

	// set item data
	private_::ItemDataWrapper * wrapper = new private_::ItemDataWrapper( handle, index, itemData );
	info.dwItemData = reinterpret_cast< ULONG_PTR >( wrapper );

	if ( ( !itemExists && ::InsertMenuItem( handle, itemID, FALSE, & info ) == TRUE ) ||
		  ( itemExists && ::SetMenuItemInfo( handle, itemID, FALSE, & info ) == TRUE ) )
	{
		itsItemDataRef.push_back( wrapper );
	}
	else
		throw xCeption( _T( "Couldn't insert/update item in the appendItem()" ) );

	setItemCommandHandler( itemID, eventHandler );
}

template< class EventHandlerClass >
void WidgetMenuExtended< EventHandlerClass >::appendItem(
	unsigned itemID,
	const SmartUtil::tstring & text, MenuItemDataPtr itemData,
	menuExtendedVoidFunctionTakingUInt eventHandler )
{
	appendItem( itemID, text, itemData, ( itsVoidMenuExtendedFunctionTakingUInt ) 0 );
	setItemCommandHandler( itemID, eventHandler );
}

template< class EventHandlerClass >
unsigned WidgetMenuExtended< EventHandlerClass >::trackPopupMenu( EventHandlerClass * mainWindow, int x, int y, unsigned flags )
{
	xAssert( mainWindow != 0, _T( "EventHandlerClass can't be null while trying to display Popup Menu" ) );
	if ( x == - 1 && y == - 1 )
	{
		DWORD pos = ::GetMessagePos();
		x = LOWORD( pos );
		y = HIWORD( pos );
	}
	int retVal = ::TrackPopupMenu(
		reinterpret_cast< HMENU >( this->Widget::itsHandle ),
		flags, x, y, 0,
		mainWindow->handle(), 0 );
	return retVal;
}

template< class EventHandlerClass >
WidgetMenuExtended< EventHandlerClass >::WidgetMenuExtended( SmartWin::Widget * parent )
	: Widget( parent ),
	isSysMenu( false ),
	drawSidebar( false ),
	itsTitleFont( new Font( _T( "Tahoma" ), 20, 10 ) ),
	itsChildrenRef( WidgetMenuExtendedPlatformImplementation< EventHandlerClass, CurrentPlatform >::itsChildren ),
	itsItemDataRef( WidgetMenuExtendedPlatformImplementation< EventHandlerClass, CurrentPlatform >::itsItemData )
{
	// Can't have a menu without a parent...
	xAssert( parent, _T( "Can't have a Menu without a parent..." ) );
}

// end namespace SmartWin
}

#endif
#endif
#endif

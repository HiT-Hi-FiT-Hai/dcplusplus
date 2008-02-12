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

#include "../../include/smartwin/widgets/WidgetMenu.h"

#include "../../include/smartwin/resources/Brush.h"
#include "../../include/smartwin/resources/Pen.h"

namespace SmartWin {

const int WidgetMenu::borderGap = 3;
const int WidgetMenu::pointerGap = 5;
const int WidgetMenu::textIconGap = 8;
const int WidgetMenu::textBorderGap = 4;
const int WidgetMenu::separatorHeight = 8;
const int WidgetMenu::minSysMenuItemWidth = 130;
Point WidgetMenu::defaultImageSize = Point( 16, 16 );

WidgetMenu::WidgetMenu( SmartWin::Widget* parent ) :
isSysMenu(false),
itsChildrenRef(itsChildren),
itsItemDataRef(itsItemData),
itsParent(parent),
drawSidebar(false)
{
}

void WidgetMenu::create(const Seed& cs)
{
	// save settings provided through the Seed
	ownerDrawn = cs.ownerDrawn;
	itsColorInfo = cs.colorInfo;

	if(ownerDrawn) {
		{
			LOGFONT lf;
			::GetObject((HFONT)GetStockObject(DEFAULT_GUI_FONT), sizeof(lf), &lf);
			lf.lfWeight = FW_BOLD;
			itsTitleFont = SmartWin::FontPtr(new SmartWin::Font(::CreateFontIndirect(&lf), true));
		}

		// set default drawing
		itsParent->setCallback(Message(WM_DRAWITEM), DrawItemDispatcher(std::tr1::bind(&WidgetMenu::handleDrawItem, this, _1, _2)));
		itsParent->setCallback(Message(WM_MEASUREITEM), MeasureItemDispatcher(std::tr1::bind(&WidgetMenu::handleMeasureItem, this, _1)));
	}

	// Create menu
	if(cs.popup)
		itsHandle = ::CreatePopupMenu();
	else
		itsHandle = ::CreateMenu();
	if ( !itsHandle )
	{
		xCeption x( _T( "CreateMenu in WidgetMenu::create fizzled..." ) );
		throw x;
	}
}

void WidgetMenu::attach()
{
	addCommands(itsParent);
	if ( ::SetMenu( getParent(), this->itsHandle ) == FALSE )
		throw xCeption( _T( "Couldn't attach menu to the parent window" ) );
}

WidgetMenu::ObjectType WidgetMenu::appendPopup( const SmartUtil::tstring & text, MenuItemDataPtr itemData )
{
	// create popup menu pointer
	ObjectType retVal ( new WidgetMenu(this->itsParent) );
	retVal->create( Seed(ownerDrawn, itsColorInfo) );

	// init structure for new item
	MENUITEMINFO info;
	memset( & info, 0, sizeof( info ) );
	info.cbSize = sizeof( MENUITEMINFO );

	// set flags
	info.fMask = MIIM_SUBMENU | MIIM_CHECKMARKS | MIIM_STRING;

	// set item text
	info.dwTypeData = const_cast< LPTSTR >( text.c_str() );

	// set sub menu
	info.hSubMenu = retVal->handle();

	// get position to insert
	int position = ::GetMenuItemCount( this->itsHandle );

	private_::ItemDataWrapper * wrapper;
	if(ownerDrawn) {
		info.fMask |= MIIM_DATA | MIIM_FTYPE;

		info.fType = MFT_OWNERDRAW;

		// create item data
		wrapper = new private_::ItemDataWrapper( this->itsHandle, position, itemData );
		info.dwItemData = reinterpret_cast< UINT_PTR >( wrapper );
	}

	// append to this menu at the end
	if ( ::InsertMenuItem( this->itsHandle, position, TRUE, & info ) )
	{
		if(ownerDrawn)
			itsItemData.push_back( wrapper );
		itsChildren.push_back( retVal );
	}
	return retVal;
}

#ifdef PORT_ME
WidgetMenu::ObjectType WidgetMenu::getSystemMenu()
{
	// get system menu for the utmost parent
	HMENU handle = ::GetSystemMenu( this->getParent(), FALSE );

	// create pointer to system menu
	ObjectType sysMenu( new WidgetMenu( this->getParent() ) );

	// create(take) system menu
	sysMenu->isSysMenu = true;
	sysMenu->create( handle, false );

	// We're assuming that the system menu has the same lifespan as the "this" menu, we must keep a reference to te system menu
	// otherwise it will be "lost", therefore we add it up as a child to the "this" menu...
	itsChildren.push_back( sysMenu );

	return sysMenu;
}
#endif

void WidgetMenu::addCommands(Widget* widget) {
	for(CallbackMap::iterator i = callbacks.begin(); i != callbacks.end(); ++i) {
		widget->setCallback(Message(WM_COMMAND, i->first), i->second);
	}
	for(std::vector< ObjectType >::iterator i = itsChildren.begin(); i != itsChildren.end(); ++i) {
		(*i)->addCommands(widget);
	}
}

int WidgetMenu::getItemIndex( unsigned int id )
{
	int index = 0;
	const int itemCount = ::GetMenuItemCount( this->itsHandle );

	for ( index = 0; index < itemCount; ++index )
		if ( ::GetMenuItemID( this->itsHandle, index ) == id ) // exit the loop if found
			return index;

	return - 1;
}

MenuItemDataPtr WidgetMenu::getData( int itemIndex )
{
	size_t i = 0;

	for ( i = 0; i < itsItemDataRef.size(); ++i )
		if ( itsItemDataRef[i]->index == itemIndex )
			return itsItemDataRef[i]->data;

	return MenuItemDataPtr();
}

WidgetMenu::~WidgetMenu()
{
	// Destroy this menu
	::DestroyMenu( this->handle() );
	std::for_each( itsItemDataRef.begin(), itsItemDataRef.end(), destroyItemDataWrapper );
}

void WidgetMenu::destroyItemDataWrapper( private_::ItemDataWrapper * wrapper )
{
	if ( 0 != wrapper )
		delete wrapper;

	wrapper = 0;
}

void WidgetMenu::setTitleFont( FontPtr font )
{
	itsTitleFont = font;
	setTitle( itsTitle, this->drawSidebar ); // Easy for now, should be refactored...
}

void WidgetMenu::clearTitle( bool clearSidebar /* = false */)
{
	if(!ownerDrawn)
		return;

	if ( !clearSidebar && !itsTitle.empty() )
		removeItem( 0 );

	// clear title text
	itsTitle.clear();
}

void WidgetMenu::checkItem( unsigned id, bool value )
{
	::CheckMenuItem( this->handle(), id, value ? MF_CHECKED : MF_UNCHECKED );
}

void WidgetMenu::setItemEnabled( unsigned id, bool byPosition, bool value )
{
	if ( ::EnableMenuItem( this->handle(), id, (byPosition ? MF_BYPOSITION : MF_BYCOMMAND) | (value ? MF_ENABLED : MF_GRAYED) ) == - 1 )
	{
		xCeption x( _T( "Couldn't enable/disable the menu item, item doesn't exist" ) );
		throw x;
	}
}

UINT WidgetMenu::getMenuState( UINT id, bool byPosition )
{
	return ::GetMenuState(this->handle(), id, byPosition ? MF_BYPOSITION : MF_BYCOMMAND); 
}

bool WidgetMenu::isSeparator( UINT id, bool byPosition )
{
	return (getMenuState(id, byPosition) & MF_SEPARATOR) == MF_SEPARATOR; 
}

bool WidgetMenu::isChecked( UINT id, bool byPosition )
{
	return (getMenuState(id, byPosition) & MF_CHECKED) == MF_CHECKED; 
}

bool WidgetMenu::isPopup( UINT id, bool byPosition )
{
	return (getMenuState(id, byPosition) & MF_POPUP) == MF_POPUP; 
}

bool WidgetMenu::isEnabled( UINT id, bool byPosition )
{
	return !(getMenuState(id, byPosition) & (MF_DISABLED | MF_GRAYED)); 
}

void WidgetMenu::setDefaultItem( UINT id, bool byPosition )
{
	::SetMenuDefaultItem(this->handle(), id, byPosition);
}

SmartUtil::tstring WidgetMenu::getText( unsigned id, bool byPosition )
{
	MENUITEMINFO mi = { sizeof(MENUITEMINFO) };

	// set flag
	mi.fMask = MIIM_STRING;

	if ( ::GetMenuItemInfo( this->itsHandle, id, byPosition, & mi ) == FALSE )
		throw xCeption( _T( "Couldn't get item info in WidgetMenu::getText" ) );

	boost::scoped_array< TCHAR > buffer( new TCHAR[++mi.cch] );
	mi.dwTypeData = buffer.get();
	if ( ::GetMenuItemInfo( this->itsHandle, id, byPosition, & mi ) == FALSE )
		throw xCeption( _T( "Couldn't get item info in WidgetMenu::getText" ) );
	return mi.dwTypeData;
}

void WidgetMenu::setText( unsigned id, const SmartUtil::tstring& text )
{
	MENUITEMINFO mi = { sizeof(MENUITEMINFO) };

	// set flag
	mi.fMask = MIIM_STRING;
	mi.dwTypeData = (TCHAR*) text.c_str();

	if ( ::SetMenuItemInfo( this->itsHandle, id, FALSE, & mi ) == FALSE )
		throw xCeption( _T( "Couldn't set item info in WidgetMenu::setText" ) );
}

void WidgetMenu::setTitle( const SmartUtil::tstring & title, bool drawSidebar /* = false */)
{
	if(!ownerDrawn)
		return;

	this->drawSidebar = drawSidebar;
	const bool hasTitle = !itsTitle.empty();

	// set its title
	itsTitle = title;

	if ( !drawSidebar )
	{
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

		// created info for title item
		MenuItemDataPtr data( new MenuItemData( itsTitleFont ) );
		private_::ItemDataWrapper * wrapper = new private_::ItemDataWrapper( this->itsHandle, 0, data, true );

		// set item data
		info.dwItemData = ( ULONG_PTR ) wrapper;

		if ( ( !hasTitle && ::InsertMenuItem( this->itsHandle, 0, TRUE, & info ) ) ||
			( hasTitle && ::SetMenuItemInfo( this->itsHandle, 0, TRUE, & info ) ) )
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

bool WidgetMenu::handleDrawItem(int id, LPDRAWITEMSTRUCT drawInfo) {
	if ( ( id != 0 ) || ( drawInfo->CtlType != ODT_MENU ) ) // if not intended for us
		return false;

	// setup colors
	MenuColorInfo colorInfo = this->itsColorInfo;
	COLORREF colorMenuBar = colorInfo.colorMenuBar;
	COLORREF colorMenuDraw = colorInfo.colorMenu; // color for drawing menu
	COLORREF colorFillHighlighted = ColorUtilities::lightenColor( colorInfo.colorHighlight, 0.7 );

	// get item data wrapper
	private_::ItemDataWrapper * wrapper = reinterpret_cast< private_::ItemDataWrapper * >( drawInfo->itemData );
	xAssert( wrapper != 0, _T( "Unsupported menu item in drawItem()" ) );

	// if processing menu bar
	const bool isMenuBar = ::GetMenu( getParent() ) == wrapper->menu;

	// change menu draw color for menubars
	if ( isMenuBar )
		colorMenuDraw = colorMenuBar;

	// init struct for menu item info
	MENUITEMINFO info;
	memset( & info, 0, sizeof( MENUITEMINFO ) );
	info.cbSize = sizeof( MENUITEMINFO );

	// set flags
	info.fMask = MIIM_CHECKMARKS | MIIM_FTYPE | MIIM_DATA | MIIM_STATE | MIIM_STRING;

	if ( ::GetMenuItemInfo( wrapper->menu, wrapper->index, TRUE, & info ) == FALSE )
		throw xCeption ( _T( "Couldn't get menu item info in drawItem()" ) );

	// check if item is owner drawn
	xAssert( ( info.fType & MFT_OWNERDRAW ) != 0, _T( "Not a owner - drawn item in drawItem()" ) );

	// get item data
	MenuItemDataPtr data ( wrapper->data );
	xAssert( data != 0, _T( "Couldn't find item data in drawItem()" ) );

	// get state info
	bool isGrayed = ( drawInfo->itemState & ODS_GRAYED ) == ODS_GRAYED;
	bool isChecked = ( drawInfo->itemState & ODS_CHECKED ) == ODS_CHECKED;
	bool isDisabled = ( drawInfo->itemState & ODS_DISABLED ) == ODS_DISABLED;
	bool isSelected = ( drawInfo->itemState & ODS_SELECTED ) == ODS_SELECTED;
	bool isHighlighted = ( drawInfo->itemState & ODS_HOTLIGHT ) == ODS_HOTLIGHT;

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
	Rectangle itemRectangle( drawInfo->rcItem.left, drawInfo->rcItem.top, // position
		drawInfo->rcItem.right - drawInfo->rcItem.left, // width
		drawInfo->rcItem.bottom - drawInfo->rcItem.top ); // height

	// setup buffered canvas
	BufferedCanvas< FreeCanvas > canvas( reinterpret_cast<HWND>(wrapper->menu), drawInfo->hDC );

	// this will conain adjusted sidebar width
	int sidebarWidth = 0;

	// this will contain logical information
	// about title font
	LOGFONT lf;
	memset( & lf, 0, sizeof( LOGFONT ) );

	// this will contain adjusted(rotated) title font for sidebar
	HFONT titleFont = NULL;

	// get title font info and adjust item rectangle
	if ( this->drawSidebar )
	{
		// get title font
		FontPtr font = this->itsTitleFont;

		// get logical info for title font
		::GetObject( font->handle(), sizeof( LOGFONT ), & lf );

		// 90 degree rotation and bold
		lf.lfOrientation = lf.lfEscapement = 900;

		// create title font from logical info
		titleFont = ::CreateFontIndirect( & lf );

		// get title text size
		SIZE textSize;
		memset( & textSize, 0, sizeof( SIZE ) );

		HGDIOBJ oldFont = ::SelectObject( canvas.handle(), titleFont );
		::GetTextExtentPoint32( canvas.handle(), this->itsTitle.c_str(), ( int ) this->itsTitle.size(), & textSize );
		::SelectObject( canvas.handle(), oldFont );

		// set sidebar width to text height
		sidebarWidth = textSize.cy;

		// adjust item rectangle and item background
		itemRectangle.pos.x += sidebarWidth;
		itemRectangle.size.x -= sidebarWidth;
	}

	// draw sidebar with menu title
	if ( ( drawInfo->itemAction & ODA_DRAWENTIRE ) && ( this->drawSidebar ) && !this->itsTitle.empty() )
	{
		// select title font and color
		HGDIOBJ oldFont = ::SelectObject ( canvas.handle(), titleFont );
		COLORREF oldColor = canvas.setTextColor( colorInfo.colorTitleText );

		// set background mode to transparent
		bool oldMode = canvas.setBkMode( true );

		// get rect for sidebar
		RECT rect;
		::GetClipBox( drawInfo->hDC, & rect );
		//rect.left -= borderGap;

		// set title rectangle
		Rectangle textRectangle( 0, 0, sidebarWidth, rect.bottom - rect.top );

		// draw background
		Brush brush ( colorInfo.colorMenuBar );
		canvas.fillRectangle( textRectangle, brush );

		// draw title
		textRectangle.pos.y += 10;
		canvas.drawText( this->itsTitle, textRectangle, DT_BOTTOM | DT_SINGLELINE );

		// clear
		canvas.setTextColor( oldColor );
		canvas.setBkMode( oldMode );

		// set back old font
		::SelectObject( canvas.handle(), oldFont );
	}

	// destroy title font
	::DeleteObject( titleFont );

	// set item background
	if ( wrapper->isMenuTitleItem ) // for title
	{
		Brush brush ( colorMenuBar );
		canvas.fillRectangle( itemRectangle, brush );

		// draw raised border
		RECT rc( itemRectangle );
		::DrawEdge( canvas.handle(), & rc, EDGE_RAISED, BF_RECT );
	}
	else // for normal items
	{
		Brush brush ( colorMenuDraw );
		canvas.fillRectangle( itemRectangle, brush );
	}

	if ( isMenuBar && isSelected ) // draw selected menu bar item
	{
		// TODO: Simulate shadow

		// select pen for drawing broder
		// and brush for filling item
		COLORREF colorBorder = 0;
		Canvas::Selector select_pen(canvas, *PenPtr(new Pen(colorBorder)));
		Canvas::Selector select_brush(canvas, *BrushPtr(new Brush(ColorUtilities::lightenColor( colorMenuBar, 0.5 ))));

		canvas.rectangle( itemRectangle );
	} // end if
	else if ( ( isSelected || isHighlighted ) && !isDisabled ) // draw selected or highlighted menu item (if not inactive)
	{
		// select pen for drawing broder
		// and brush for filling item
		Canvas::Selector select_pen(canvas, *PenPtr(new Pen(colorInfo.colorHighlight)));
		Canvas::Selector select_brush(canvas, *BrushPtr(new Brush(colorFillHighlighted)));

		canvas.rectangle( itemRectangle );
	} // end if
	else if ( !isMenuBar && !wrapper->isMenuTitleItem ) // draw strip bar for menu items (except menu title item)
	{
		// create rectangle for strip bar
		Rectangle stripRectangle ( itemRectangle );
		stripRectangle.size.x = stripWidth;

		// draw strip bar
		Brush brush ( colorInfo.colorStrip );
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
		Canvas::Selector select(canvas, *PenPtr(new Pen(::GetSysColor( COLOR_GRAYTEXT ))));

		// draw separator
		canvas.moveTo( rectangle.pos.x, rectangle.pos.y );
		canvas.lineTo( rectangle.size.x, rectangle.pos.y );
	} // end if
	else // not a seperator, then draw item text and icon
	{
		// get item text
		const int length = info.cch + 1;
		std::vector< TCHAR > buffer( length );
		int count = ::GetMenuString( wrapper->menu, wrapper->index, & buffer[0], length, MF_BYPOSITION );
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

		// Select item font
		FontPtr font((static_cast<int>(::GetMenuDefaultItem(wrapper->menu, TRUE, GMDI_USEDISABLED)) == wrapper->index) ? itsTitleFont : data->Font);

		HGDIOBJ oldFont = ::SelectObject( canvas.handle(), font->handle() );

		unsigned drawTextFormat = DT_VCENTER | DT_SINGLELINE;
		if((drawInfo->itemState & ODS_NOACCEL) == ODS_NOACCEL)
			drawTextFormat |= DT_HIDEPREFIX;

		if ( !isMenuBar && !wrapper->isMenuTitleItem && !itemText.empty() ) // if menu item
		{
			// compute text rectangle
			Rectangle textRectangle( itemRectangle );

			// adjust rectangle
			textRectangle.pos.x += stripWidth + textIconGap;
			textRectangle.size.x -= stripWidth + textIconGap + borderGap;

			canvas.drawText( text, textRectangle, DT_LEFT | drawTextFormat );

			// draw accelerator
			if ( !accelerator.empty() )
				canvas.drawText( accelerator, textRectangle, DT_RIGHT | drawTextFormat );
		} // end if
		else if ( !itemText.empty() ) // draw menu bar item text
		{
			Rectangle textRectangle( itemRectangle );

			if ( image != NULL ) // has icon
				textRectangle.pos.x += textIconGap;

			canvas.drawText( text, textRectangle, DT_CENTER | drawTextFormat );
		} // end if

		// set back old font
		::SelectObject( canvas.handle(), oldFont );

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
				Brush brush( colorInfo.colorStrip );
				canvas.fillRectangle( imageRectangle, brush );

				// create memory DC and set bitmap on it
				HDC memoryDC = ::CreateCompatibleDC( canvas.handle() );
				HGDIOBJ old = ::SelectObject( memoryDC, ::CreateCompatibleBitmap( canvas.handle(), imageSize.x, imageSize.y ) );

				// draw into memory
				RECT rc( Rectangle( 0, 0, imageSize.x, imageSize.y ) );
				::DrawFrameControl( memoryDC, & rc, DFC_MENU, ( info.fType & MFT_RADIOCHECK ) == 0 ? DFCS_MENUCHECK : DFCS_MENUBULLET );

				const int adjustment = 2; // adjustment for mark to be in the center

				// bit - blast into out canvas
				::BitBlt( canvas.handle(), imageRectangle.pos.x + adjustment, imageRectangle.pos.y, imageSize.x, imageSize.y, memoryDC, 0, 0, SRCAND );

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
				imageRectangle.pos.x--;
				imageRectangle.pos.y--;

				// setup brush for shadow emulation
				Brush brush( ColorUtilities::darkenColor( colorInfo.colorStrip, 0.2 ) );

				// draw the icon shadow
				Rectangle shadowRectangle( imageRectangle );
				shadowRectangle.pos.x++;
				shadowRectangle.pos.y++;
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
			Canvas::Selector select(canvas, *PenPtr(new Pen(colorInfo.colorHighlight)));
			canvas.line( imageRectangle );
		}
	}

	// blast buffer into screen
	if ( ( drawInfo->itemAction & ODA_DRAWENTIRE ) && this->drawSidebar ) // adjustment for sidebar
	{
		itemRectangle.pos.x -= sidebarWidth;
		itemRectangle.size.x += sidebarWidth;
	}

	canvas.blast( itemRectangle );
	return true;
}

bool WidgetMenu::handleMeasureItem(LPMEASUREITEMSTRUCT measureInfo) {
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

	// try to get item info
	if ( ::GetMenuItemInfo( wrapper->menu, wrapper->index, TRUE, & info ) == FALSE )
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
	const bool isMenuBar = ::GetMenu( getParent() ) == wrapper->menu;

	// compute text width and height by simulating write to dc
	// get its DC
	HDC hdc = ::GetDC( getParent() );

	// get the item data
	MenuItemDataPtr data = wrapper->data;
	xAssert( data != 0, _T( "Couldn't find item data in measureItem()" ) );

	// get item text
	const int length = info.cch + 1;
	std::vector< TCHAR > buffer ( length );
	int count = ::GetMenuString( wrapper->menu, wrapper->index, & buffer[0], length, MF_BYPOSITION );
	SmartUtil::tstring itemText ( buffer.begin(), buffer.begin() + count );

	// now get text extents
	SIZE textSize;
	memset( & textSize, 0, sizeof( SIZE ) );

	HGDIOBJ oldFont = ::SelectObject( hdc, data->Font->handle() );
	::GetTextExtentPoint32( hdc, itemText.c_str(), ( int ) itemText.size(), & textSize );
	::SelectObject( hdc, oldFont );

	// release DC
	::ReleaseDC( reinterpret_cast<HWND>(wrapper->menu), hdc );

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
	if ( this->isSysMenu )
		itemWidth = (std::max)( ( UINT ) minSysMenuItemWidth, itemWidth );

	// adjust width for sidebar
	if ( this->drawSidebar )
	{
		// get title text extents
		SIZE textSize;
		memset( & textSize, 0, sizeof( SIZE ) );

		::GetTextExtentPoint32( hdc, this->itsTitle.c_str(), ( int ) this->itsTitle.size(), & textSize );

		itemWidth += textSize.cy;
	}

	// adjust item height
	itemHeight = (std::max)( itemHeight, ( UINT )::GetSystemMetrics( SM_CYMENU ) );
	return true;
}

void WidgetMenu::appendSeparatorItem()
{
	// init structure for new item
	MENUITEMINFO itemInfo;
	memset( & itemInfo, 0, sizeof( itemInfo ) );
	itemInfo.cbSize = sizeof( MENUITEMINFO );

	// set flags
	itemInfo.fMask = MIIM_FTYPE;
	itemInfo.fType = MFT_SEPARATOR;

	// get position to insert
	int position = ::GetMenuItemCount( this->itsHandle );

	private_::ItemDataWrapper * wrapper;
	if(ownerDrawn) {
		itemInfo.fMask |= MIIM_DATA;
		itemInfo.fType |= MFT_OWNERDRAW;

		// create item data wrapper
		wrapper = new private_::ItemDataWrapper( this->itsHandle, position, MenuItemDataPtr( new MenuItemData() ) );
		itemInfo.dwItemData = reinterpret_cast< ULONG_PTR >( wrapper );
	}

	if ( ::InsertMenuItem( this->itsHandle, position, TRUE, & itemInfo ) && ownerDrawn )
		itsItemDataRef.push_back( wrapper );
}

void WidgetMenu::removeItem( unsigned itemIndex )
{
	// has sub menus ?
	HMENU popup = ::GetSubMenu( this->itsHandle, itemIndex );

	// try to remove item
	if ( ::RemoveMenu( this->itsHandle, itemIndex, MF_BYPOSITION ) == TRUE )
	{
		size_t i = 0;

		if(ownerDrawn) {
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
		}

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

void WidgetMenu::removeAllItems()
{
	//must be backwards, since bigger indexes change on remove
	for( int i = this->getCount() - 1; i >= 0; i-- )
	{
		this->removeItem( i );
	}
}

int WidgetMenu::getCount()
{
	int count = ::GetMenuItemCount( this->itsHandle );
	if( count == -1 )
		throw xCeption( _T( "Couldn't get item count in getCount()" ) );
	return count;
}

void WidgetMenu::appendItem(unsigned int id, const SmartUtil::tstring & text, MenuItemDataPtr itemData)
{
	// init structure for new item
	MENUITEMINFO info;
	memset( & info, 0, sizeof( info ) );
	info.cbSize = sizeof( MENUITEMINFO );

	// set flags
	info.fMask = MIIM_DATA | MIIM_FTYPE | MIIM_CHECKMARKS | MIIM_ID | MIIM_STRING;

	// set fields
	xAssert( !isSysMenu || id < SC_SIZE, _T( "Can't add sysmenu item with that high value, value can not be higher then SC_SIZE - 1" ) );
	info.wID = id;

	// set text
	info.dwTypeData = const_cast< LPTSTR >( text.c_str() );

	// find item index
	int index = getItemIndex( id );

	// set position to insert
	bool itemExists = index != - 1;
	index = itemExists ? index : ::GetMenuItemCount( this->itsHandle );

	private_::ItemDataWrapper * wrapper;
	if(ownerDrawn) {
		info.fMask |= MIIM_DATA | MIIM_FTYPE;
		info.fType = MFT_OWNERDRAW;

		// set item data
		wrapper = new private_::ItemDataWrapper( this->itsHandle, index, itemData );
		info.dwItemData = reinterpret_cast< ULONG_PTR >( wrapper );
	}

	if ( ( !itemExists && ::InsertMenuItem( this->itsHandle, id, FALSE, & info ) == TRUE ) ||
		( itemExists && ::SetMenuItemInfo( this->itsHandle, id, FALSE, & info ) == TRUE ) )
	{
		if(ownerDrawn)
			itsItemDataRef.push_back( wrapper );
	}
	else
		throw xCeption( _T( "Couldn't insert/update item in the WidgetMenu::appendItem" ) );
}

void WidgetMenu::appendItem(unsigned int id, const SmartUtil::tstring & text, BitmapPtr image)
{
	MenuItemDataPtr itemData(new MenuItemData());
	if(ownerDrawn)
		itemData->Image = image;
	appendItem(id, text, itemData);
}

unsigned WidgetMenu::trackPopupMenu( Widget * mainWindow, const ScreenCoordinate& sc, unsigned flags )
{
	xAssert( mainWindow != 0, _T( "Widget can't be null while trying to display Popup Menu" ) );
	addCommands(mainWindow);

	long x = sc.getPoint().x, y = sc.getPoint().y;

	if ( x == - 1 && y == - 1 )
	{
		DWORD pos = ::GetMessagePos();
		x = LOWORD( pos );
		y = HIWORD( pos );
	}

	int retVal = ::TrackPopupMenu(this->itsHandle, flags, x, y, 0, mainWindow->handle(), 0 );
	return retVal;
}

WidgetMenu::ObjectType WidgetMenu::getChild( unsigned position ) {
	HMENU h = ::GetSubMenu(handle(), position);
	for(size_t i = 0; i < this->itsChildren.size(); ++i) {
		ObjectType& menu = this->itsChildren[i];
		if(menu->handle() == h) {
			return menu;
		}
	}
	return ObjectType();
}

}

#endif

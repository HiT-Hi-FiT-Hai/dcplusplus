// $Revision: 1.26 $
/*
  Copyright (c) 2005, Thomas Hansen
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification,
  are permitted provided that the following conditions are met:

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
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND 
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef AspectSizable_h
#define AspectSizable_h

#include "boost.h"
#include "../SignalParams.h"
#include "../Place.h"
#include "AspectGetParent.h"

namespace SmartWin
{
// begin namespace SmartWin

// Dispatcher class with specializations for dispatching event to event handlers of
// the AspectSize Since AspectSize is used both in WidgetWindowBase (container
// widgets) and Control Widgets we need to specialize which implementation to use
// here!!
template< class EventHandlerClass, class WidgetType, class MessageMapType, bool IsControl >
class AspectSizeDispatcher
{
};

template< class EventHandlerClass, class WidgetType, class MessageMapType >
class AspectSizeDispatcher<EventHandlerClass, WidgetType, MessageMapType, true/*Control Widget*/>
{
public:
	static HRESULT dispatch( private_::SignalContent & params )
	{
		typename MessageMapType::voidFunctionTakingWindowSizedEventResult func =
			reinterpret_cast< typename MessageMapType::voidFunctionTakingWindowSizedEventResult >( params.Function );

		EventHandlerClass * ThisParent = internal_::getTypedParentOrThrow < EventHandlerClass * >( params.This );
		WidgetType * This = boost::polymorphic_cast< WidgetType * >( params.This );

		func(
			ThisParent,
			This,
			private_::createWindowSizedEventResultFromMessageParams( params.Msg.LParam, params.Msg.WParam )
			);

		return 0;
	}

	static HRESULT dispatchThis( private_::SignalContent & params )
	{
		typename MessageMapType::itsVoidFunctionTakingWindowSizedEventResult func =
			reinterpret_cast< typename MessageMapType::itsVoidFunctionTakingWindowSizedEventResult >( params.FunctionThis );

		EventHandlerClass * ThisParent = internal_::getTypedParentOrThrow < EventHandlerClass * >( params.This );
		WidgetType * This = boost::polymorphic_cast< WidgetType * >( params.This );

		( ( * ThisParent ).*func )(
			This,
			private_::createWindowSizedEventResultFromMessageParams( params.Msg.LParam, params.Msg.WParam )
			);

		return 0;
	}
};

template< class EventHandlerClass, class WidgetType, class MessageMapType >
class AspectSizeDispatcher<EventHandlerClass, WidgetType, MessageMapType, false/*Container Widget*/>
{
public:
	static HRESULT dispatch( private_::SignalContent & params )
	{
		typename MessageMapType::voidFunctionTakingWindowSizedEventResult func =
			reinterpret_cast< typename MessageMapType::voidFunctionTakingWindowSizedEventResult >( params.Function );

		EventHandlerClass * ThisParent = internal_::getTypedParentOrThrow < EventHandlerClass * >( params.This );

		func(
			ThisParent,
			private_::createWindowSizedEventResultFromMessageParams( params.Msg.LParam, params.Msg.WParam )
			);

		return ThisParent->returnFromHandledWindowProc( reinterpret_cast< HWND >( params.Msg.Handle ), params.Msg.Msg, params.Msg.WParam, params.Msg.LParam );
	}

	static HRESULT dispatchThis( private_::SignalContent & params )
	{
		typename MessageMapType::itsVoidFunctionTakingWindowSizedEventResult func =
			reinterpret_cast< typename MessageMapType::itsVoidFunctionTakingWindowSizedEventResult >( params.FunctionThis );

		EventHandlerClass * ThisParent = internal_::getTypedParentOrThrow < EventHandlerClass * >( params.This );

		( ( * ThisParent ).*func )(
			private_::createWindowSizedEventResultFromMessageParams( params.Msg.LParam, params.Msg.WParam )
			);

		return ThisParent->returnFromHandledWindowProc( reinterpret_cast< HWND >( params.Msg.Handle ), params.Msg.Msg, params.Msg.WParam, params.Msg.LParam );
	}
};

// Dispatcher class with specializations for dispatching event to event handlers of
// the AspectSize Since AspectSize is used both in WidgetWindowBase (container
// widgets) and Control Widgets we need to specialize which implementation to use
// here!!
template< class EventHandlerClass, class WidgetType, class MessageMapType, bool IsControl >
class AspectMoveDispatcher
{
};

template< class EventHandlerClass, class WidgetType, class MessageMapType >
class AspectMoveDispatcher< EventHandlerClass, WidgetType, MessageMapType, true >
{
public:
	static HRESULT dispatch( private_::SignalContent & params )
	{
		typename MessageMapType::voidFunctionTakingPoint func =
			reinterpret_cast< typename MessageMapType::voidFunctionTakingPoint >( params.Function );

		func(
			internal_::getTypedParentOrThrow < EventHandlerClass * >( params.This->getParent() ),
			boost::polymorphic_cast< WidgetType * >( params.This ),
			Point( GET_X_LPARAM( params.Msg.LParam ), GET_Y_LPARAM( params.Msg.LParam ) )
			);

		return 0;
	}

	static HRESULT dispatchThis( private_::SignalContent & params )
	{
		typename MessageMapType::itsVoidFunctionTakingPoint func =
			reinterpret_cast< typename MessageMapType::itsVoidFunctionTakingPoint >( params.FunctionThis );

		( ( * internal_::getTypedParentOrThrow < EventHandlerClass * >( params.This->getParent() ) ).*func )(
			boost::polymorphic_cast< WidgetType * >( params.This ),
			Point( GET_X_LPARAM( params.Msg.LParam ), GET_Y_LPARAM( params.Msg.LParam ) )
			);

		return 0;
	}
};

template< class EventHandlerClass, class WidgetType, class MessageMapType >
class AspectMoveDispatcher< EventHandlerClass, WidgetType, MessageMapType, false >
{
public:
	static HRESULT dispatch( private_::SignalContent & params )
	{
		typename MessageMapType::voidFunctionTakingPoint func =
			reinterpret_cast< typename MessageMapType::voidFunctionTakingPoint >( params.Function );

		func(
			internal_::getTypedParentOrThrow < EventHandlerClass * >( params.This ),
			Point( GET_X_LPARAM( params.Msg.LParam ), GET_Y_LPARAM( params.Msg.LParam ) )
			);

		MessageMapType * This = boost::polymorphic_cast< MessageMapType * >( params.This );
		return This->returnFromHandledWindowProc( reinterpret_cast< HWND >( params.Msg.Handle ), params.Msg.Msg, params.Msg.WParam, params.Msg.LParam );
	}

	static HRESULT dispatchThis( private_::SignalContent & params )
	{
		typename MessageMapType::itsVoidFunctionTakingPoint func =
			reinterpret_cast< typename MessageMapType::itsVoidFunctionTakingPoint >( params.FunctionThis );

		( ( * internal_::getTypedParentOrThrow < EventHandlerClass * >( params.This ) ).*func )(
			Point( GET_X_LPARAM( params.Msg.LParam ), GET_Y_LPARAM( params.Msg.LParam ) )
			);

		MessageMapType * This = boost::polymorphic_cast< MessageMapType * >( params.This );
		return This->returnFromHandledWindowProc( reinterpret_cast< HWND >( params.Msg.Handle ), params.Msg.Msg, params.Msg.WParam, params.Msg.LParam );
	}
};

/// \ingroup AspectClasses
/// \ingroup WidgetLayout
/// Aspect class used by Widgets that have the possibility of setting and getting a
/// "size" property of their objects.
/** E.g. the WidgetTextBox have a "size" Aspect therefore it realizes the
  * AspectSizable through inheritance. <br>
  * Note! <br>
  * All coordinates have zenith top-left corner of either the desktop display or the
  * client area of the parent Widget. <br>
  * Note! <br>
  * There are two different ways to calculate the position of a Widget, one is in
  * screen coordinates which starts top left of the desktop window, the other way is
  * relative to its parent Widget which starts at the top left of the parent Widgets
  * client area which is the total area of the Widget after the border, menu, toolbar
  * etc have been taken away. <br>
  * In addition all bounding Rectangles dealt with through this class are giving
  * their down right coordinates in SIZES and not in POSITIONS!
  */
template< class EventHandlerClass, class WidgetType, class MessageMapType >
class AspectSizable
{
	typedef AspectSizeDispatcher< EventHandlerClass, WidgetType, MessageMapType, MessageMapType::IsControl > DispatcherSize;
	typedef AspectMoveDispatcher< EventHandlerClass, WidgetType, MessageMapType, MessageMapType::IsControl > DispatcherMove;
public:
	/// Sets the new size and position of the window
	/** The input parameter Rectangle defines the new size (and position) of the
	  * window. <br>
	  * The pos member of the Rectangle is the position and the size member is the
	  * size. <br>
	  * So a call to this function will (probably) also MOVE your Widget too.
	  *
	  * For a top-level window, the position and dimensions are relative to the
	  * upper-left corner of the screen. For a child window, they are relative
	  * to the upper-left corner of the parent window's client area. 
	  */
	void setBounds( const Rectangle & rect, bool updateWindow = true );

	/// Sets the new size and position of the window
	/** The input parameter newPos of type Point defines the new position of the
	  * window. <br>
	  * The newSize member of type Point is the new size of the window. <br>
	  * A call to this function will (probably) also MOVE your Widget too.
	  */
	void setBounds( const Point & newPos, const Point & newSize, bool updateWindow = true );

	/// Sets the new size and position of the window
	/** x is the new horizontal position of your window. <br>
	  * y is the new vertical position of your window. <br>
	  * width is the new width and height is the new height of your window. <br>
	  * Zenith is as in all other bounds function top/left. <br>
	  * A call to this function will (probably) also MOVE your Widget too.
	  */
	void setBounds( int x, int y, int width, int height, bool updateWindow = true );

	/// Given a bounding rectangle with rows, put this Widget in the rownum position
	/** The rect defines a column made up of a number of rows.  The rownum specifies a
	  * zero based index of the row to place the Widget.
	  * [ row0 ]   <br>
	  * [ row1 ]   <br>
	  * ...        <br>
	  * [ rownum ] <br>
	  * [      ]   <br>
	  * <br>
	  * Of course you could just generate a new bounding rectangle, but this is easier.
	  */
	void setSizeAsCol( const Rectangle & rect, int rows, int rownum, int border = 0, bool updateWindow = true );

	/// Given a bounding rectangle with cols, put this Widget in the colnum position
	/** The rect defines a row made up of a number of columns.  The colnum specifies a
	  * zero based index of the column to place the Widget.<br>
	  * [ col0 ]  [ col1 ]  ... [ colnum ]  [   ]  [   ] <br>
	  */
	void setSizeAsRow( const Rectangle & rect, int cols, int colnum, int border = 0, bool updateWindow = true );

	/// Given a bounding Place class, divided into rows and columns, resize and put
	/// this Widget in the next cell.
	/** Bound determines the bounding rectangle, and borders. <br>
	  * rows and cols determine the size and position of each cell. <br>
	  * The internal position of bound is updated. <br>
	  * The Widgets are sized and placed according to the current cell, from left to
	  * right until a row is full, and then continues with the next row.
	  */
	void setSizeAsGridPerPlace( SmartWin::Place & bound, int rows, int cols );

	/// Given a bounding Place class, place this Widget and adjust to the next position.
	/** This function places the Widget into the bounding rectangle specified by bound. <br>
	  * The size of the Widget is preserved. <br>
	  * The Widgets are sized and placed from left to right until
	  * a row is full, and then continues with the next row. <br>
	  * The internal position of bound is updated. <br>
	  */
	void setPositionPerPlace( SmartWin::Place & bound );

	/// Place after sizing for the Widget's text, and adjust to the next position.
	/** This function places the Widget into the bounding rectangle specified by
	  * bound. <br>
	  * The idea is that the size of certain Widgets should really be large enough to
	  * show their text.  Buttons and text areas are examples. <br>
	  * The size of the Widget is calculated from the size of getText(). <br>
	  * It is optionally adjusted by the extraX and extraY. <br>
	  * The Widgets are sized and placed from left to right until a row is full, and
	  * then continues with the next row. <br>
	  * The internal position of bound is updated.
	  */
	void setSizePerTextPerPlace( SmartWin::Place & bound, const SmartUtil::tstring & text,
								 int extraX = 0, int extraY = 0 );

	/// Returns the screen size.
	/** This is the screen size, and useful for making applications that must adapt
	  * to different screen sizes.
	  */
	static Point getDesktopSize();

	/// Returns the position and size of the window.
	/** Note that this is in screen coordinates meaning the position returned is
	  * relative to the upper left corner  of the desktop screen, the function also
	  * returns in the size member of the Rectangle the size of the window and not
	  * the position of the lower right point. Values includes borders, frames and
	  * toolbar etc of the window.
	  * Note that if you don't override the default parameter adjustForParent it will 
	  * adjust for the parent meaning it will return in client coordinates instead of screen coordinates.
	  */
	Rectangle getBounds( bool adjustForParent = true ) const;

	/// Returns the size of the window.
	/** Includes the border, frame and toolbar etc of the window.
	  */
	Point getSize() const;

	/// Returns the position of the window relative to the parent window.
	/** Note that this is in client coordinates.
	*/
	Point getPosition() const;

	/// Returns the screen position of the window.
	Point getScreenPosition() const;


	/// Returns the size of the client area of the window.
	/** This differs from getSize because it disregards the border and headers, this
	  * function only returns the client area of the Widget meaning the area which it
	  * is possible to draw on.
	  */
	Point getClientAreaSize() const;

	/// Fills a Point with the size of text to be drawn in the Widget's font.
	/** getTextSize determines the height and width that text will take. <br>
	  * This is useful if you want to allocate enough space to fit known text. <br>
	  * It accounts for the set font too.
	  */
	Point getTextSize( const SmartUtil::tstring & text );

	/// Maximize your window
	/** This will make the window fill the whole area that the window has available.
	  * <br>
	  * This function cannot be called for any Widget types other than those derived
	  * from WidgetWindowBase.
	  */
	void maximize();

	/// Minimize your window
	/** This will make the window become minimized. <br>
	  * This function cannot be called for any Widget types other than those derived
	  * from WidgetWindowBase.
	  */
	void minimize();

	/// Restores your window
	/** This will make the window become restored. <br>
	  * This function cannot be called for any Widget types other than those derived
	  * from WidgetWindowBase.
	  */
	void restore();

	/// Brings the widget to the front
	/** Makes the widget become the front most widget meaning it will not be obscured
	  * by other widgets which are contained in the same container widget. <br>
	  * For instance if you have two widgets which partially hides eachother and you
	  * call bringToFront on one of them it will make sure that the widget you call
	  * bringToFront on will be the one which will be all visible and the other one
	  * will be partially hidden by the parts which are obscured by the this widget.
	  */
	void bringToFront();

	/// Brings the widget to the bottom
	/** Makes the widget become the bottom most widget meaning it will be obscured by
	  * all other widgets which are contained in the same container widget. <br>
	  * For instance if you have two widgets which partially hides eachother and you
	  * call bringToBottom on one of them it will make sure that the widget you call
	  * bringToBottom on will be the one which will be invisible and the other one
	  * will be all visible by the parts which are obscured by the this widget.
	  */
	void bringToBottom();

	/// \ingroup EventHandlersAspectSizable
	// Setting the event handler for the "sized" event
	/** The size submitted to the event handler is the new client area size. The
	  * parameter passed is WidgetSizedEventResult which contains the new size
	  * information.
	  */
	void onSized( typename MessageMapType::itsVoidFunctionTakingWindowSizedEventResult eventHandler );
	void onSized( typename MessageMapType::voidFunctionTakingWindowSizedEventResult eventHandler );

	/// \ingroup EventHandlersAspectSizable
	// Setting the event handler for the "moved" event
	/** This event will be raised when the Widget is being moved. The parameter
	  * passed is Point which is the new position of the Widget
	  */
	void onMoved( typename MessageMapType::itsVoidFunctionTakingPoint eventHandler );
	void onMoved( typename MessageMapType::voidFunctionTakingPoint eventHandler );

protected:
	virtual ~AspectSizable()
	{}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template< class EventHandlerClass, class WidgetType, class MessageMapType >
void AspectSizable< EventHandlerClass, WidgetType, MessageMapType >::setBounds( const Rectangle & rect, bool updateWindow )
{
	if ( ::MoveWindow( static_cast< WidgetType * >( this )->handle(),
		rect.pos.x, rect.pos.y, rect.size.x, rect.size.y, updateWindow ? TRUE : FALSE ) == 0 )
	{
		xCeption err( _T( "Couldn't reposition windows" ) );
		throw err;
	}
}

template< class EventHandlerClass, class WidgetType, class MessageMapType >
void AspectSizable< EventHandlerClass, WidgetType, MessageMapType >::setBounds( const Point & newPos, const Point & newSize, bool updateWindow )
{
	if ( ::MoveWindow( static_cast< WidgetType * >( this )->handle(), newPos.x, newPos.y, newSize.x, newSize.y, updateWindow ? TRUE : FALSE ) == 0 )
	{
		xCeption err( _T( "Couldn't reposition windows" ) );
		throw err;
	}
}

template< class EventHandlerClass, class WidgetType, class MessageMapType >
void AspectSizable< EventHandlerClass, WidgetType, MessageMapType >::setBounds( int x, int y, int width, int height, bool updateWindow )
{
	if ( ::MoveWindow( static_cast< WidgetType * >( this )->handle(), x, y, width, height, updateWindow ? TRUE : FALSE ) == 0 )
	{
		xCeption err( _T( "Couldn't reposition windows" ) );
		throw err;
	}
}

template< class EventHandlerClass, class WidgetType, class MessageMapType >
void AspectSizable< EventHandlerClass, WidgetType, MessageMapType >::setSizeAsCol( const Rectangle & rect, int rows, int rownum,
	int border, bool updateWindow )
{
	int totBorder = border * ( rows + 1 ); // All borders together determine
	int ySize = ( rect.size.y - totBorder ) / rows; // the space left for each row.

	int yPos = rect.pos.y + border; // Start with current y and first border.
	yPos += rownum * ( border + ySize ); // Accumulate other rows and borders

	::MoveWindow( static_cast< WidgetType * >( this )->handle(), rect.pos.x, yPos,
					rect.size.x, ySize, updateWindow ? TRUE : FALSE );
}

template< class EventHandlerClass, class WidgetType, class MessageMapType >
void AspectSizable< EventHandlerClass, WidgetType, MessageMapType >::setSizeAsRow( const Rectangle & rect, int cols, int colnum,
	int border, bool updateWindow )
{
	int totBorder = border * ( cols + 1 );
	int xSize = ( rect.size.x - totBorder ) / cols;
	int xPos = rect.pos.x + border; // Start with current X and first border
	xPos += colnum * ( border + xSize ); // Accumulate other columns and borders

	::MoveWindow( static_cast< WidgetType * >( this )->handle(), xPos, rect.pos.y, xSize, rect.size.y, updateWindow ? TRUE : FALSE );
}

template< class EventHandlerClass, class WidgetType, class MessageMapType >
void AspectSizable< EventHandlerClass, WidgetType, MessageMapType >::setSizeAsGridPerPlace( SmartWin::Place & bound, int rows, int cols )
{
	SmartWin::Rectangle posSize;
	bound.sizeOfCell( rows, cols, posSize.size ); // Calculate the desired size.
	bound.positionToRight( posSize ); // pos_size.pos= Current place position,
													// and update Place's position
	setBounds( posSize ); // Reposition with a new size.
}

template< class EventHandlerClass, class WidgetType, class MessageMapType >
void AspectSizable< EventHandlerClass, WidgetType, MessageMapType >::setPositionPerPlace( SmartWin::Place & bound )
{
	SmartWin::Rectangle posSize( getSize() ); // Get the current size
	bound.positionToRight( posSize ); // pos_size.pos= Current place position,
											// and update Place's position
	setBounds( posSize ); // Reposition with the same size.
}

template< class EventHandlerClass, class WidgetType, class MessageMapType >
void AspectSizable< EventHandlerClass, WidgetType, MessageMapType >
::setSizePerTextPerPlace( SmartWin::Place & bound, const SmartUtil::tstring & text,
						  int extraX, int extraY )
{
	Point textSize = getTextSize( text );
	textSize.x += extraX;
	textSize.y += extraY;

	// Now Place the Widget according to the calculated size
	SmartWin::Rectangle posSize( textSize ); // Use the  for text.
	bound.positionToRight( posSize ); // pos_size.pos = Current place position,
											// and update Place's position
	setBounds( posSize ); // Reposition with the calculated size.
}

template< class EventHandlerClass, class WidgetType, class MessageMapType >
Point AspectSizable< EventHandlerClass, WidgetType, MessageMapType >::getDesktopSize()
{
	RECT rc;
	::GetWindowRect( ::GetDesktopWindow(), & rc );
	return Point( rc.right - rc.left, rc.bottom - rc.top );
}

template< class EventHandlerClass, class WidgetType, class MessageMapType >
Rectangle AspectSizable< EventHandlerClass, WidgetType, MessageMapType >::getBounds( bool adjustForParent ) const
{
	int width, height;
	RECT rc;
	POINT pt;
	HWND hwnd = const_cast < WidgetType * >( static_cast< const WidgetType * >( this ) )->handle();
	::GetWindowRect( hwnd, & rc );
	width = rc.right - rc.left;
	height = rc.bottom - rc.top;
	pt.x = rc.left;
	pt.y = rc.top;
	if( adjustForParent )
	{
		Widget* parent = static_cast< const WidgetType * >( this )->getParent();
		if( parent )
		{
			//if it's a child, adjust coordinates relative to parent
			::ScreenToClient( parent->handle(), &pt );
		}
	}
	return Rectangle( pt.x, pt.y, width, height );
}

template< class EventHandlerClass, class WidgetType, class MessageMapType >
Point AspectSizable< EventHandlerClass, WidgetType, MessageMapType >::getSize() const
{
  Rectangle rc = this->getBounds();
	return Point( rc.size.x, rc.size.y );
}

template< class EventHandlerClass, class WidgetType, class MessageMapType >
Point AspectSizable< EventHandlerClass, WidgetType, MessageMapType >::getPosition() const
{
	Rectangle rc = this->getBounds();
	return Point( rc.pos.x, rc.pos.y );
}


template< class EventHandlerClass, class WidgetType, class MessageMapType >
Point AspectSizable< EventHandlerClass, WidgetType, MessageMapType >::getScreenPosition() const
{
	RECT rc;
	::GetWindowRect( const_cast < WidgetType * >( static_cast< const WidgetType * >( this ) )->handle(), & rc );
	return Point( rc.left, rc.top );
}



template< class EventHandlerClass, class WidgetType, class MessageMapType >
Point AspectSizable< EventHandlerClass, WidgetType, MessageMapType >::getClientAreaSize() const
{
	RECT rc;
	::GetClientRect( const_cast < WidgetType * >( static_cast< const WidgetType * >( this ) )->handle(), & rc );
	return Point( rc.right, rc.bottom );
}

template< class EventHandlerClass, class WidgetType, class MessageMapType >
Point AspectSizable< EventHandlerClass, WidgetType, MessageMapType >
::getTextSize( const SmartUtil::tstring & text )
{
	// Some win32 api code to determine the actual size of the string
	HWND hWnd = static_cast< WidgetType * >( this )->handle();
	HDC hDC = ::GetDC( hWnd );
	HFONT hf = ( HFONT ) ::SendMessage( hWnd, WM_GETFONT, 0, 0 );
	if ( 0 != hf )
	{
		SelectFont( hDC, hf );
	}

	RECT wRect =
	{ 0, 0, 0, 0
	};
	DrawText( hDC, text.c_str(), ( int ) text.size(), & wRect, DT_CALCRECT );
	::ReleaseDC( hWnd, hDC );

	return( Point( wRect.right, wRect.bottom ) );
}

template< class EventHandlerClass, class WidgetType, class MessageMapType >
void AspectSizable< EventHandlerClass, WidgetType, MessageMapType >::maximize()
{
	// Magic Enum construct!!
	// If you get a compile time error in the next line you are trying to maximize
	// a window which doesn't support being maximized, e.g. a WidgetButton or WidgetTreeView
#ifdef _MSC_VER
	typename WidgetType::MaxiMiniRestorable;
#else
	typename WidgetType::MaxiMiniRestorable checker;
#endif
	::ShowWindow( static_cast< WidgetType * >( this )->handle(), SW_SHOWMAXIMIZED );
}

template< class EventHandlerClass, class WidgetType, class MessageMapType >
void AspectSizable< EventHandlerClass, WidgetType, MessageMapType >::minimize()
{
	// Magic Enum construct!!
	// If you get a compile time error in the next line you are trying to minimize
	// a window which soesn't support being minimized, e.g. a WidgetButton or WidgetTreeView
#ifdef _MSC_VER
	typename WidgetType::MaxiMiniRestorable;
#else
	typename WidgetType::MaxiMiniRestorable checker;
#endif
	::ShowWindow( static_cast< WidgetType * >( this )->handle(), SW_MINIMIZE );
}

template< class EventHandlerClass, class WidgetType, class MessageMapType >
void AspectSizable< EventHandlerClass, WidgetType, MessageMapType >::restore()
{
	// Magic Enum construct!!
	// If you get a compile time error in the next line you are trying to restore
	// a window which soesn't support being restored, e.g. a WidgetButton or WidgetTreeView
#ifdef _MSC_VER
	typename WidgetType::MaxiMiniRestorable;
#else
	typename WidgetType::MaxiMiniRestorable checker;
#endif
	::ShowWindow( static_cast< WidgetType * >( this )->handle(), SW_RESTORE );
}

template< class EventHandlerClass, class WidgetType, class MessageMapType >
void AspectSizable< EventHandlerClass, WidgetType, MessageMapType >::bringToFront()
{
	::SetWindowPos( static_cast< WidgetType * >( this )->handle(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
}

template< class EventHandlerClass, class WidgetType, class MessageMapType >
void AspectSizable< EventHandlerClass, WidgetType, MessageMapType >::bringToBottom()
{
	::SetWindowPos( static_cast< WidgetType * >( this )->handle(), HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
}

// Preparation of event handlers

template< class EventHandlerClass, class WidgetType, class MessageMapType >
void AspectSizable< EventHandlerClass, WidgetType, MessageMapType >::onSized( typename MessageMapType::itsVoidFunctionTakingWindowSizedEventResult eventHandler )
{
	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->addNewSignal(
		typename MessageMapType::SignalTupleType(
			private_::SignalContent(
				Message( WM_SIZE ),
				reinterpret_cast< itsVoidFunction >( eventHandler ),
				ptrThis
			),
			typename MessageMapType::SignalType(
				typename MessageMapType::SignalType::SlotType( & DispatcherSize::dispatchThis )
			)
		)
	);
}

template< class EventHandlerClass, class WidgetType, class MessageMapType >
void AspectSizable< EventHandlerClass, WidgetType, MessageMapType >::onSized( typename MessageMapType::voidFunctionTakingWindowSizedEventResult eventHandler )
{
	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->addNewSignal(
		typename MessageMapType::SignalTupleType(
			private_::SignalContent(
				Message( WM_SIZE ),
				reinterpret_cast< private_::SignalContent::voidFunctionTakingVoid >( eventHandler ),
				ptrThis
			),
			typename MessageMapType::SignalType(
				typename MessageMapType::SignalType::SlotType( & DispatcherSize::dispatch )
			)
		)
	);
}

template< class EventHandlerClass, class WidgetType, class MessageMapType >
void AspectSizable< EventHandlerClass, WidgetType, MessageMapType >::onMoved( typename MessageMapType::itsVoidFunctionTakingPoint eventHandler )
{
	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->addNewSignal(
		typename MessageMapType::SignalTupleType(
			private_::SignalContent(
				Message( WM_MOVE ),
				reinterpret_cast< itsVoidFunction >( eventHandler ),
				ptrThis
			),
			typename MessageMapType::SignalType(
				typename MessageMapType::SignalType::SlotType( & DispatcherMove::dispatchThis )
			)
		)
	);
}

template< class EventHandlerClass, class WidgetType, class MessageMapType >
void AspectSizable< EventHandlerClass, WidgetType, MessageMapType >::onMoved( typename MessageMapType::voidFunctionTakingPoint eventHandler )
{
	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->addNewSignal(
		typename MessageMapType::SignalTupleType(
			private_::SignalContent(
				Message( WM_MOVE ),
				reinterpret_cast< private_::SignalContent::voidFunctionTakingVoid >( eventHandler ),
				ptrThis
			),
			typename MessageMapType::SignalType(
				typename MessageMapType::SignalType::SlotType( & DispatcherMove::dispatch )
			)
		)
	);
}

// end namespace SmartWin
}

#endif

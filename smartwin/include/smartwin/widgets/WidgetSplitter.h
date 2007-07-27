// $Revision: 1.19 $
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
#ifndef WidgetSplitter_h
#define WidgetSplitter_h

#ifndef WINCE // Doesn't exist in Windows CE based systems

#include "../CanvasClasses.h"
#include "../MessageMapPolicyClasses.h"
#include "../WindowClass.h"
#include "../aspects/AspectClickable.h"
#include "../aspects/AspectRaw.h"
#include "../aspects/AspectSizable.h"
#include "../aspects/AspectVisible.h"
#include "../xCeption.h"
#include <boost/scoped_ptr.hpp>

namespace SmartWin
{
// begin namespace SmartWin

// Forward declaring friends
template< class WidgetType >
class WidgetCreator;

/// Aspect class for a WidgetSplitter
/** If the Painter parameter to the WidgetSplitter instance is this class the
  * splitter will get the "this" look. <br>
  * Related classes
  * < ul >
  * < li >SplitterCoolPaint< /li >
  * < li >WidgetSplitter< /li >
  * < li >WidgetSplitterBase< /li >
  * < /ul >
  */
class SplitterThinPaint
{
protected:
	/// Actually paints the splitter
	void paintSplitter( unsigned width, unsigned ySize, HWND handle );

	/// Paints the "moving splitter"
	/** This function basically gets the position of the cursor in client coordinates
	  * of the parent Widget. <br>
	  * It gets the size ( rect ) of the parent window in client coordinates. In
	  * addition it gets an update canvas. <br>
	  * Also SmartWin++ creates a HdcModeSetter in a "R2_NOTXORPEN" before it calls
	  * this function. <br>
	  * This means that you may draw stuff and unless you set your own mode and
	  * overrides the default it will automatically be erased.
	  */
	void paintSplitterMoving( Canvas & canvas, unsigned cursorX, unsigned cursorY, const SmartWin::Rectangle & rect );

	/// Virtual DTOR to ensure deletion through base class pointer
	virtual ~SplitterThinPaint()
	{}
};

/// Aspect class for a WidgetSplitter
/** If the Painter parameter to the WidgetSplitter instance is this class the
  * splitter will get the "cool" look. <br>
  * Related classes
  * < ul >
  * < li >SplitterThinPaint< /li >
  * < li >WidgetSplitter< /li >
  * < li >WidgetSplitterBase< /li >
  * < /ul >
  */
class SplitterCoolPaint
{
protected:
	/// Actually paints the splitter
	void paintSplitter( unsigned width, unsigned ySize, HWND handle );

	/// Paints the "moving splitter"
	/** This function basically gets the position of the cursor in client coordinates
	  * of the parent Widget. <br>
	  * It get the size ( rect ) of the parent window in client coordinates In
	  * addition it gets an update canvas. <br>
	  * Also SmartWin creates a HdcModeSetter in a "R2_NOTXORPEN" before it calls
	  * this function. <br>
	  * This means that you may draw stuff and unless you set your own mode it will
	  * automatically be erased with a second call with the same cursoro position.
	  */
	void paintSplitterMoving( SmartWin::Canvas & canvas, unsigned cursorX, unsigned cursorY, const SmartWin::Rectangle & rect );

	/// Virtual DTOR to ensure deletion through base pointer
	virtual ~SplitterCoolPaint()
	{}
};

/// Window Splitter Control class
/** \ingroup WidgetControls
  * \WidgetUsageInfo
  * \image html splitter.PNG
  * Class for creating a Splitter Window, normally you would want to create two
  * windows and attach them at the left and right <br>
  * of this window so that you only have to resize those windows and not every single
  * Widgets on either side of the splitter. <br>
  * The Painter template argument defines what type of painting logic you would want
  * the splitter to use. <br>
  * the default ones supplied by the library is WidgetCoolPaint and WidgetThinPaint.
  * <br>
  * Related classes
  * < ul >
  * < li > SplitterCoolPaint< /li >
  * < li > SplitterThinPaint< /li >
  * < /ul >
  * If you wish to inject your own Painting policy you should make a class which AT
  * LEAST contains a function called paintSplitter <br>
  * and that function MUST take two unsigned integers and a HWND. <br>
  * The function should also return void. <br>
  * The complete method signature would look something like : <br>
  * void paintSplitter( unsigned width, unsigned ySize, HWND handle ); <br>
  * Look at the WidgetSplitter solution for a quick example.
  */
template< typename Painter = SplitterThinPaint >
class WidgetSplitter :
	public MessageMapPolicy<Policies::Normal>,
	public Painter,

	// Aspects
	public AspectSizable< WidgetSplitter< Painter > >,
	public AspectVisible< WidgetSplitter< Painter > >,
	public AspectRaw< WidgetSplitter< Painter > >
{
	friend class WidgetCreator< WidgetSplitter >;
public:
	/// Class type
	typedef WidgetSplitter< Painter > ThisType;

	/// Object type
	typedef ThisType * ObjectType;
	
	typedef MessageMapPolicy<Policies::Normal> PolicyType;

	/// Seed class
	/** This class contains all of the values needed to create the widget. It also
	  * knows the type of the class whose seed values it contains. Every widget
	  * should define one of these.
	  */
	class Seed
		: public SmartWin::Seed
	{
	public:
		typedef typename WidgetSplitter::ThisType WidgetType;

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

	/// Returns the width of the splitter
	/** Returns the widht of the splitter in pixels.
	  */
	unsigned getWidth();

	// TODO: Redraw stuff and resize...
	/// Sets the width of the splitter.
	/** Unit is pixels. <br>
	  * Call this function to change the widt of your splitter, make sure you update
	  * the splitter control afterwards since by default it wont automatically redraw
	  * itself unless you call Widget::updateWidget().
	  */
	void setWidth( unsigned newWidth );

	/// Returns the X position of the splitter
	/** The unit returned is pixels.
	  */
	unsigned getXPos();

	/// Actually creates the WidgetSplitter Control
	/** You should call WidgetFactory::createSplitter function or one of the
	  * WidgetFactory::createSplitterThin or WidgetFactory::createSplitterCool if you
	  * instantiate this class directly. <br>
	  * Only if you DERIVE from class you should call this function directly.
	  */
	virtual void create( const Seed & cs = getDefaultSeed() );

protected:
	// Constructor Taking pointer to parent
	explicit WidgetSplitter( SmartWin::Widget * parent );

	/// Protected to avoid direct instantiation, you can inherit and use
	/// WidgetFactory class which is friend
	virtual ~WidgetSplitter()
	{}

private:
	unsigned itsXPos;
	unsigned itsYPos;
	unsigned itsYSize;
	unsigned itsWidth;
	SmartWin::Point itsPoint;
	unsigned itsOldX;
	unsigned itsOldY;

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template< typename Painter >
const typename WidgetSplitter< Painter >::Seed & WidgetSplitter< Painter >::getDefaultSeed()
{
	static bool d_NeedsInit = true;
	static Seed d_DefaultValues( DontInitializeMe );
	static boost::scoped_ptr<WindowClass> windowClass;
	if ( d_NeedsInit )
	{
		windowClass.reset(new WindowClass(_T("WidgetSplitter"), &ThisType::wndProc, NULL, ( HBRUSH )( COLOR_GRAYTEXT + 1 ), NULL, NULL, ::LoadCursor( 0, IDC_SIZEWE )));
		d_DefaultValues.className = windowClass->getClassName();
		d_NeedsInit = false;
	}
	return d_DefaultValues;
}

template< typename Painter >
WidgetSplitter< Painter >::Seed::Seed()
{
	* this = WidgetSplitter::getDefaultSeed();
}

template< typename Painter >
unsigned WidgetSplitter< Painter >::getWidth()
{
	return itsWidth;
}

template< typename Painter >
void WidgetSplitter< Painter >::setWidth( unsigned newWidth )
{
	itsWidth = newWidth;
	//::InvalidateRect( itsHandle, NULL, TRUE );
	//::UpdateWindow( itsHandle );
}

template< typename Painter >
unsigned WidgetSplitter< Painter >::getXPos()
{
	return itsXPos;
}

template< typename Painter >
WidgetSplitter< Painter >::WidgetSplitter( SmartWin::Widget * parent )
	: Widget( parent, 0 )
	, itsWidth( 8 )
{
	// Can't have a text box without a parent...
	xAssert( parent, _T( "Can't have a Splitter without a parent..." ) );
}

template< typename Painter >
void WidgetSplitter< Painter >::create( const Seed & cs )
{
	PolicyType::create( cs );
	RECT rc;
	::GetWindowRect( this->getParent()->handle(), & rc );
	if ( !::MoveWindow( this->handle(), rc.right / 2, 0, getWidth(), rc.bottom, TRUE ) )
	{
	  xCeption x( _T( "Error while trying to initially move WidgetSplitter" ) );
	  throw x;
	}
}
#ifdef PORT_ME
template< typename Painter >
LRESULT WidgetSplitter< Painter >::sendWidgetMessage( HWND hWnd, UINT msg, WPARAM & wPar, LPARAM & lPar )
{
	switch ( msg )
	{
		case WM_LBUTTONDOWN :
		{
			// "Locking" cursor in Widget
			::SetCapture( this->handle() );

			// Creating an "update" canvas on the parent Widget
			UpdateCanvas can( this->getParent()->handle() );

			// We need the client size of the parent Widget since we need to tell our policy class how much "space" it can draw onto
			RECT splWndPos;
			::GetClientRect( this->getParent()->handle(), & splWndPos );
			SmartWin::Rectangle rect( splWndPos.left, splWndPos.top, splWndPos.right, splWndPos.bottom );

			// We also need the position of our parent Widget since we need to calculate the cursor position in "client" coordinates
			RECT splParentWndPos;
			::GetWindowRect( this->getParent()->handle(), & splParentWndPos );
			SmartWin::Rectangle rectParentSize( splParentWndPos.left, splParentWndPos.top, splParentWndPos.right, splParentWndPos.bottom );

			// Setting mode to "XOR" kind of mode
			HdcModeSetter mode( can, R2_NOTXORPEN );

			// Retrieving cursor position...
			POINT curPos;
			::GetCursorPos( & curPos );
			itsPoint.x = curPos.x;
			itsPoint.y = curPos.y;

			// Saving the splitter moving position to be able to remove it later
			itsOldX = itsPoint.x - rectParentSize.pos.x;
			itsOldY = itsPoint.y - rectParentSize.pos.y - 20;

			// Painting splitter mover
			Painter::paintSplitterMoving( can, itsOldX, itsOldY, rect );

			return 0;
		}
		case WM_MOUSEMOVE :
		{
			if ( wPar & MK_LBUTTON )
			{
				// Creating an "update" canvas on the parent Widget
				UpdateCanvas can( this->getParent()->handle() );

				// We need the client size of the parent Widget since we need to tell our policy class how much "space" it can draw onto
				RECT splWndPos;
				::GetClientRect( this->getParent()->handle(), & splWndPos );
				SmartWin::Rectangle rect( splWndPos.left, splWndPos.top, splWndPos.right, splWndPos.bottom );

				// We also need the position of our parent Widget since we need to calculate the cursor position in "client" coordinates
				RECT splParentWndPos;
				::GetWindowRect( this->getParent()->handle(), & splParentWndPos );
				SmartWin::Rectangle rectParentSize( splParentWndPos.left, splParentWndPos.top, splParentWndPos.right, splParentWndPos.bottom );

				// Setting mode to "XOR" kind of mode
				HdcModeSetter mode( can, R2_NOTXORPEN );

				// Retrieving cursor position...
				POINT curPos;
				::GetCursorPos( & curPos );
				itsPoint.x = curPos.x;
				itsPoint.y = curPos.y;

				// Erasing old splitter mover
				Painter::paintSplitterMoving( can, itsOldX, itsOldY, rect );

				// Updating the "old" x position to be able to remove the splitter mover window later
				itsOldX = itsPoint.x - rectParentSize.pos.x;
				itsOldY = itsPoint.y - rectParentSize.pos.y - 20;

				// Painting splitter mover
				Painter::paintSplitterMoving( can, itsOldX, itsOldY, rect );
			}
			return 0;
		}
		case WM_LBUTTONUP :
		{
			// Relasing cursor
			::ReleaseCapture();

			// Creating an "update" canvas on the parent Widget
			UpdateCanvas can( this->getParent()->handle() );

			// Can't call "size" since parent is a Widget * and size member is in the AspectSizable Aspect
			RECT splWndPos;
			::GetWindowRect( this->getParent()->handle(), & splWndPos );
			SmartWin::Rectangle rect( splWndPos.left, splWndPos.top, splWndPos.right, splWndPos.bottom );

			// Setting mode to "XOR" kind of mode
			HdcModeSetter mode( can, R2_NOTXORPEN );

			// Erasing old splitter mover
			Painter::paintSplitterMoving( can, itsOldX, itsOldY, rect );

			SmartWin::Rectangle oldWndPos( this->ThisType::getSize() );
			int newXPos = itsOldX - ( itsWidth / 2 );
			::GetClientRect( this->getParent()->handle(), & splWndPos );

			// Bounds checking
			if ( newXPos < 0 )
				newXPos = 0;
			else if ( newXPos > static_cast< int >( splWndPos.right - itsWidth ) )
				newXPos = splWndPos.right - itsWidth;

			// Resizing window
			this->ThisType::setBounds( newXPos, itsYPos, itsWidth, oldWndPos.size.y );
			return 0;
		}
		case WM_SIZE :
		{
			itsYSize = HIWORD( lPar );
			return MessageMapType::sendWidgetMessage( hWnd, msg, wPar, lPar );
		} break;
		case WM_MOVE :
		{
			itsXPos = LOWORD( lPar );
			itsYPos = HIWORD( lPar );
			return MessageMapType::sendWidgetMessage( hWnd, msg, wPar, lPar );
		} break;
		case WM_PAINT :
		{
			paintSplitter( itsWidth, itsYSize, this->handle() );
			return 0; // TODO: Not sure what we should do here, should probably let fallthrough to event handlers or something... ?
		} break;
		default:
			return MessageMapType::sendWidgetMessage( hWnd, msg, wPar, lPar );
	}

	// Removing compiler hickup...
	return 0;
}
#endif
// end namespace SmartWin
}

#endif

#endif

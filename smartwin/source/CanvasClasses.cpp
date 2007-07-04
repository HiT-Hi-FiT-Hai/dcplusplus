// $Revision: 1.16 $
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
#include "../include/smartwin/WindowsHeaders.h"
#include "../include/smartwin/CanvasClasses.h"
#include "../include/smartwin/xCeption.h"

namespace SmartWin
{
// begin namespace SmartWin

HDC Canvas::getDc()
{
	return itsHdc;
}

void Canvas::selectFont( FontPtr font )
{
	SelectFont( itsHdc, font->getHandle() );
}

int Canvas::getDeviceCaps( int nIndex )
{
	return( ::GetDeviceCaps( itsHdc, nIndex ) );
}

void Canvas::moveTo( int x, int y )
{
	if ( !::MoveToEx( itsHdc, x, y, 0 ) )
	{
		xCeption x( _T( "Error in CanvasClasses moveTo" ) );
		throw x;
	}
}

void Canvas::moveTo( const Point & coord )
{
	moveTo( coord.x, coord.y );
}

void Canvas::lineTo( int x, int y )
{
	if ( !::LineTo( itsHdc, x, y ) )
	{
		xCeption x( _T( "Error in CanvasClasses lineTo" ) );
		throw x;
	}
}

void Canvas::lineTo( const Point & coord )
{
	lineTo( coord.x, coord.y );
}

void Canvas::line( int xStart, int yStart, int xEnd, int yEnd )
{
	moveTo( xStart, yStart );
	lineTo( xEnd, yEnd );
}

void Canvas::line( const Point & start, const Point & end )
{
	moveTo( start.x, start.y );
	lineTo( end.x, end.y );
}

// Draw the outline of a rectangle.
void Canvas::line( const SmartWin::Rectangle & rect )
{
	moveTo( rect.pos );
	SmartWin::Point lr( rect.lowRight() );
	lineTo( lr.x, rect.pos.y );
	lineTo( lr.x, lr.y );
	lineTo( rect.pos.x, lr.y );
	lineTo( rect.pos );
}

void Canvas::polygon( const Point points[], unsigned count )
{
	if ( !::Polygon( itsHdc, reinterpret_cast< POINT * >( const_cast < Point * >( & points[0] ) ), count ) )
	{
		xCeption x( _T( "Error in CanvasClasses polygon" ) );
		throw x;
	}
}

void Canvas::polygon( POINT points[], unsigned count )
{
	if ( !::Polygon( itsHdc, points, count ) )
	{
		xCeption x( _T( "Error in CanvasClasses polygon" ) );
		throw x;
	}
}

void Canvas::ellipse( int left, int top, int right, int bottom )
{
	if ( ! ::Ellipse( itsHdc, left, top, right, bottom ) )
	{
		xCeption x( _T( "Error in CanvasClasses ellipse" ) );
		throw x;
	}
}

void Canvas::rectangle( int left, int top, int right, int bottom )
{
	if ( ! ::Rectangle( itsHdc, left, top, right, bottom ) )
	{
		xCeption x( _T( "Error in CanvasClasses Rectangle" ) );
		throw x;
	}
}

void Canvas::rectangle( const SmartWin::Rectangle & rect )
{
	rectangle( rect.pos.x,
			   rect.pos.y,
			   rect.pos.x + rect.size.x,
			   rect.pos.y + rect.size.y );
}

void Canvas::ellipse( const SmartWin::Rectangle & rect )
{
	if ( ! ::Ellipse( itsHdc, rect.pos.x, rect.pos.y,
					rect.pos.x + rect.size.x, rect.pos.y + rect.size.y ) )
					{
		xCeption x( _T( "Error in CanvasClasses ellipse" ) );
		throw x;
	}
}

void Canvas::fillRectangle( int left, int top, int right, int bottom, Brush & brush )
{
	RECT rc;
	rc.bottom = bottom;
	rc.left = left;
	rc.right = right;
	rc.top = top;

	::FillRect( itsHdc, & rc, brush.getBrushHandle() );
}

void Canvas::fillRectangle( const SmartWin::Rectangle & rect, Brush & brush )
{
	// FIXED: RECT takes right/bottom for 3rd and 4th args
	RECT rc =
	{ rect.pos.x, rect.pos.y, rect.pos.x + rect.size.x, rect.pos.y + rect.size.y
	};
	::FillRect( itsHdc, & rc, brush.getBrushHandle() );
}

COLORREF Canvas::setPixel( int x, int y, COLORREF pixcolor )
{
	return ::SetPixel( itsHdc, x, y, pixcolor );
}

COLORREF Canvas::getPixel( int x, int y )
{
	return ::GetPixel( itsHdc, x, y );
}

COLORREF Canvas::getPixel( const Point & coord )
{
	return ::GetPixel( itsHdc, coord.x, coord.y );
}

#ifndef WINCE
bool Canvas::extFloodFill( int x, int y, COLORREF color, bool fillTilColorFound )
{
	// Can't check for errors here since if no filling was done (which obviously is no error) it'll also  return 0
	return ::ExtFloodFill( itsHdc, x, y, color, fillTilColorFound ? FLOODFILLBORDER : FLOODFILLSURFACE ) != FALSE;
}
#endif //!WINCE

int Canvas::drawText( const SmartUtil::tstring & text, const SmartWin::Rectangle & rect, unsigned format )
{
	RECT rc =
	{ rect.pos.x, rect.pos.y, rect.pos.x + rect.size.x, rect.pos.y + rect.size.y
	};
	int retVal = ::DrawText( itsHdc, text.c_str(), ( int ) text.length(), & rc, format );
	if ( 0 == retVal )
	{
		xCeption x( _T( "Error while trying to draw text to canvas" ) );
		throw x;
	}
	return retVal;
}

void Canvas::extTextOut( const SmartUtil::tstring & text, unsigned x, unsigned y )
{
	if ( 0 == ::ExtTextOut( itsHdc, x, y, 0, NULL, text.c_str(), ( unsigned ) text.length(), 0 ) )
	{
		xCeption xc( _T( "Error while trying to do TextOut operation" ) );
		throw xc;
	}
}

COLORREF Canvas::setTextColor( COLORREF crColor )
{
	return ::SetTextColor( itsHdc, crColor );
}

COLORREF Canvas::setBkColor( COLORREF crColor )
{
	return ::SetBkColor( itsHdc, crColor );
}

bool Canvas::setBkMode( bool transparent )
{
	return ::SetBkMode( itsHdc, transparent ? TRANSPARENT : OPAQUE ) == TRANSPARENT;
}

COLORREF Canvas::getBkColor()
{
	return ::GetBkColor( itsHdc );
}

COLORREF Canvas::getSysColor( int index )
{
	return ::GetSysColor( index );
}

unsigned Canvas::setTextAlign( unsigned fMode )
{
	return ::SetTextAlign( itsHdc, fMode );
}

Canvas::Canvas( HWND hWnd )
	: itsHandle( hWnd )
{}

Canvas::Canvas( Widget * widget )
	: itsHandle( widget->handle() )
{}

PaintCanvas::PaintCanvas( HWND hWnd )
	: Canvas( hWnd )
{
	initialize();
}

PaintCanvas::PaintCanvas( Widget * widget )
	: Canvas( widget )
{
	initialize();
}

PaintCanvas::~PaintCanvas()
{
	::EndPaint( itsHandle, & itsPaint );
}

void PaintCanvas::initialize()
{
	itsHdc = ::BeginPaint( itsHandle, & itsPaint );
}

UpdateCanvas::UpdateCanvas( HWND hWnd )
	: Canvas( hWnd )
{
	initialize();
}

UpdateCanvas::UpdateCanvas( Widget * widget )
	: Canvas( widget )
{
	initialize();
}

UpdateCanvas::~UpdateCanvas()
{
	::ReleaseDC( itsHandle, itsHdc );
}

void UpdateCanvas::initialize()
{
	itsHdc = ::GetDC( itsHandle );
}

FreeCanvas::FreeCanvas( HWND hWnd, HDC hdc )
	: Canvas( hWnd )
{
	itsHdc = hdc;
}

FreeCanvas::FreeCanvas( Widget * widget, HDC hdc )
	: Canvas( widget )
{
	itsHdc = hdc;
}

#ifndef WINCE
HdcModeSetter::HdcModeSetter( Canvas & canvas, int mode )
	: itsOldMode( ::GetROP2( canvas.getDc() ) ),
	itsCanvas( canvas )
{
	::SetROP2( itsCanvas.getDc(), mode );
}

HdcModeSetter::~HdcModeSetter()
{
	::SetROP2( itsCanvas.getDc(), itsOldMode );
}
#endif //! WINCE

Canvas & Pen::getCanvas()
{
	return itsCanvas;
}

HPEN Pen::getPenHandle()
{
	return itsPen;
}

Pen::Pen( Canvas & canvas, COLORREF color, int width )
	: itsCanvas( canvas )
{
	itsPen = ::CreatePen( PS_SOLID, width, color );
	itsPenOld = ( HPEN )::SelectObject( itsCanvas.getDc(), itsPen );
}

Pen::~Pen()
{
	::SelectObject( itsCanvas.getDc(), itsPenOld );
	::DeleteObject( itsPen );
}

Canvas & Brush::getCanvas()
{
	return *itsCanvas;
}

HBRUSH Brush::getBrushHandle()
{
	return itsBrush;
}

Brush::Brush(COLORREF color) : itsBrushOld(NULL), isSysColor(false), itsBrush(NULL), itsCanvas(0) {
	itsBrush = ::CreateSolidBrush(color);
}

Brush::Brush( Canvas & canvas, COLORREF color )
	: isSysColor( false ),
	itsCanvas( &canvas )
{
	itsBrush = ::CreateSolidBrush( color );
	itsBrushOld = ( HBRUSH )::SelectObject( itsCanvas->getDc(), itsBrush );
}

Brush::Brush( Canvas & canvas, BitmapPtr bitmap )
	: isSysColor( false ),
	itsCanvas( &canvas )
{
	itsBrush = ::CreatePatternBrush( bitmap->getBitmap() );
	itsBrushOld = ( HBRUSH )::SelectObject( itsCanvas->getDc(), itsBrush );
}

Brush::Brush( Canvas & canvas, Brush::SysColor color )
	: isSysColor( true ),
	itsCanvas( &canvas )
{
	itsBrush = ::GetSysColorBrush( ( int ) color );
	itsBrushOld = ( HBRUSH )::SelectObject( itsCanvas->getDc(), itsBrush );
}

Brush::Brush( Canvas & canvas )
	: isSysColor( true ),
	itsCanvas( &canvas )
{
	itsBrush = ( HBRUSH )::GetStockObject( NULL_BRUSH );
}

Brush::~Brush()
{
	if ( !isSysColor )
	{
		if(itsCanvas)
			::SelectObject( itsCanvas->getDc(), itsBrushOld );
		::DeleteObject( itsBrush );
	}
}

TextPen::TextPen( Canvas & canvas, COLORREF color )
	: itsCanvas( canvas )
{
	itsColorOld = itsCanvas.setTextColor( color );
}

TextPen::~TextPen()
{
	::SetTextColor( itsCanvas.getDc(), itsColorOld );
}

// end namespace SmartWin
}

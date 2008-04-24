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

#include <dwt/CanvasClasses.h>
#include <dwt/util/check.h>
#include <dwt/resources/Brush.h>

namespace dwt {

void Canvas::selectFont( FontPtr font )
{
	SelectFont( itsHdc, font->handle() );
}

int Canvas::getDeviceCaps( int nIndex )
{
	return( ::GetDeviceCaps( itsHdc, nIndex ) );
}

void Canvas::moveTo( int x, int y )
{
	if ( !::MoveToEx( itsHdc, x, y, 0 ) ) {
		dwtWin32DebugFail("Error in CanvasClasses moveTo");
	}
}

void Canvas::moveTo( const Point & coord )
{
	moveTo( coord.x, coord.y );
}

void Canvas::lineTo( int x, int y )
{
	if ( !::LineTo( itsHdc, x, y ) ) {
		dwtWin32DebugFail("Error in CanvasClasses lineTo");
	}
}

void Canvas::lineTo( const Point & coord ) {
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
void Canvas::line( const dwt::Rectangle & rect )
{
	moveTo( rect.pos );
	dwt::Point lr( rect.lowRight() );
	lineTo( lr.x, rect.pos.y );
	lineTo( lr.x, lr.y );
	lineTo( rect.x(), lr.y );
	lineTo( rect.pos );
}

void Canvas::polygon( const Point points[], unsigned count )
{
	if ( !::Polygon( itsHdc, reinterpret_cast< POINT * >( const_cast < Point * >( & points[0] ) ), count ) ) {
		dwtWin32DebugFail("Error in CanvasClasses polygon");
	}
}

void Canvas::polygon( POINT points[], unsigned count )
{
	if ( !::Polygon( itsHdc, points, count ) ) {
		dwtWin32DebugFail("Error in CanvasClasses polygon" );
	}
}

void Canvas::ellipse( int left, int top, int right, int bottom )
{
	if ( ! ::Ellipse( itsHdc, left, top, right, bottom ) ) {
		dwtWin32DebugFail("Error in CanvasClasses ellipse");
	}
}

void Canvas::rectangle( int left, int top, int right, int bottom )
{
	if ( ! ::Rectangle( itsHdc, left, top, right, bottom ) ) {
		dwtWin32DebugFail("Error in CanvasClasses Rectangle");
	}
}

void Canvas::rectangle( const dwt::Rectangle & rect )
{
	rectangle( rect.left(),
			   rect.top(),
			   rect.right(),
			   rect.bottom() );
}

void Canvas::ellipse( const dwt::Rectangle & rect )
{
	if ( ! ::Ellipse( itsHdc, rect.left(), rect.top(), rect.right(), rect.bottom() ) ) {
		dwtWin32DebugFail("Error in CanvasClasses ellipse");
	}
}

void Canvas::fillRectangle( int left, int top, int right, int bottom, Brush & brush )
{
	RECT rc;
	rc.bottom = bottom;
	rc.left = left;
	rc.right = right;
	rc.top = top;

	::FillRect( itsHdc, & rc, brush.handle() );
}

void Canvas::fillRectangle( const dwt::Rectangle & rect, Brush & brush )
{
	RECT rc = rect;
	::FillRect( itsHdc, & rc, brush.handle() );
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

int Canvas::drawText( const tstring & text, const dwt::Rectangle & rect, unsigned format )
{
	RECT rc = rect;
	int retVal = ::DrawText( itsHdc, text.c_str(), ( int ) text.length(), & rc, format );
	if ( 0 == retVal )
	{
		dwtWin32DebugFail("Error while trying to draw text to canvas");
	}
	return retVal;
}

void Canvas::extTextOut( const tstring & text, unsigned x, unsigned y )
{
	if ( 0 == ::ExtTextOut( itsHdc, x, y, 0, NULL, text.c_str(), ( unsigned ) text.length(), 0 ) ) {
		dwtWin32DebugFail("Error while trying to do TextOut operation");
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

Rectangle PaintCanvas::getPaintRect()
{
	return itsPaint.rcPaint;
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
	: itsOldMode( ::GetROP2( canvas.handle() ) ),
	itsCanvas( canvas )
{
	::SetROP2( itsCanvas.handle(), mode );
}

HdcModeSetter::~HdcModeSetter()
{
	::SetROP2( itsCanvas.handle(), itsOldMode );
}
#endif //! WINCE

TextPen::TextPen( Canvas & canvas, COLORREF color )
	: itsCanvas( canvas )
{
	itsColorOld = itsCanvas.setTextColor( color );
}

TextPen::~TextPen()
{
	::SetTextColor( itsCanvas.handle(), itsColorOld );
}

}

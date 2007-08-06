// $Revision: 1.17 $
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
#ifndef CanvasClasses_h
#define CanvasClasses_h

#include "Widget.h"
#include "BasicTypes.h"
#include "resources/Bitmap.h"
#include "Font.h"

namespace SmartWin
{
// begin namespace SmartWin

/// Helper class for manipulating colors
/** Helper class for manipulating COLORREF values, contains static functions for
  * darken and lightening colors etc. COLORREF values (color variables) etc.
  */
class ColorUtilities
{
public:
	/// Darkens given color by specified factor
	/** Factor is in (0..1] range, the higher the factor the more dark the result
	  * will be. <br>
	  * Returns the manipulated value
	  */
	static COLORREF darkenColor( COLORREF color, double factor )
	{
		if ( factor > 0.0 && factor <= 1.0 )
		{
			BYTE red = GetRValue( color );
			BYTE green = GetGValue( color );
			BYTE blue = GetBValue( color );

			BYTE lightred = ( BYTE )( red - ( factor * red ) );
			BYTE lightgreen = ( BYTE )( green - ( factor * green ) );
			BYTE lightblue = ( BYTE )( blue - ( factor * blue ) );

			color = RGB( lightred, lightgreen, lightblue );
		}
		return color;
	}

	/// Lightens given color by specified factor
	/** Factor is in (0..1] range, the higher the factor the more light the result
	  * will be. <br>
	  * Returns the manipulated value
	  */
	static COLORREF lightenColor( COLORREF color, double factor )
	{
		if ( factor > 0.0 && factor <= 1.0 )
		{
			BYTE red = GetRValue( color );
			BYTE green = GetGValue( color );
			BYTE blue = GetBValue( color );

			BYTE lightred = ( BYTE )( ( factor * ( 255 - red ) ) + red );
			BYTE lightgreen = ( BYTE )( ( factor * ( 255 - green ) ) + green );
			BYTE lightblue = ( BYTE )( ( factor * ( 255 - blue ) ) + blue );

			color = RGB( lightred, lightgreen, lightblue );
		}
		return color;
	}

	/// Alpha blends color
	/** Factor is in R/255, G/255 and B/255 meaning a value of 255, 255, 255 or
	  * 0xFFFFFF will not change change source color at all while a value of 0xFF00FF
	  * will keep all read and blue and discard all green. <br>
	  * Returns the manipulated value.
	  */
	static COLORREF alphaBlend( COLORREF color, COLORREF factor )
	{
		float aRed = GetRValue( factor );
		float aGreen = GetGValue( factor );
		float aBlue = GetBValue( factor );
		float red = GetRValue( color );
		float green = GetGValue( color );
		float blue = GetBValue( color );

		BYTE lightred = ( BYTE )( ( aRed / 255.0 ) * red );
		BYTE lightgreen = ( BYTE )( ( aGreen / 255.0 ) * green );
		BYTE lightblue = ( BYTE )( ( aBlue / 255.0 ) * blue );

		color = RGB( lightred, lightgreen, lightblue );
		return color;
	}
};

// Forward declaration since Brush is used in Canvas class
class Brush;

/// Buffered Canvas, useful for e.g. double buffering updates to a Widget
/** A BufferedCanvas is a Canvas which you can draw upon but only resides in memory
  * and will not update the actual visible Canvas before you call blast, only then
  * the results of the drawing operations will be visible, useful to avoid "tearing"
  * which is an effect which might occur if your drawing operations are lengthy. <br>
  * Class does provide RAII semantics!
  */
template< class CanvasType >
class BufferedCanvas
	: public CanvasType
{
public:
	/// Constructor initializing the given source
	BufferedCanvas( HWND window, HDC source )
		: CanvasType( window, source )
	{
		init( source );
	}

	/// Constructor initializing the given source
	BufferedCanvas( HWND window )
		: CanvasType( window )
	{
		init( this->CanvasType::itsHdc );
	}

	/// Destructor will free up the contained HDC
	/** Note!<br>
	  * Destructor will not flush the contained operations to the contained Canvas
	  */
	virtual ~BufferedCanvas()
	{
		// delete buffer bitmap
		::DeleteObject( ::SelectObject( this->CanvasType::itsHdc, itsOldBitmap ) );

		// delete buffer
		::DeleteDC( this->CanvasType::itsHdc );

		// set back source
		this->CanvasType::itsHdc = itsSource;
	}

	/// BitBlasts buffer into specified rectangle of source
	void blast( const Rectangle & rectangle )
	{
		if ( ::BitBlt( itsSource, rectangle.pos.x, rectangle.pos.y, rectangle.size.x, rectangle.size.y, this->CanvasType::itsHdc, rectangle.pos.x, rectangle.pos.y, SRCCOPY ) == FALSE )
			throw xCeption( _T( "Couldn't bit blast in blast()" ) );
	}

	/// Transparently draws bitmap
	/** Bitmap background color should be the color of the image that should be
	  * transparent
	  */
	void drawBitmap( HBITMAP bitmap, const Rectangle & imageRectangle, COLORREF bitmapBackgroundColor, bool drawDisabled )
	{
		// bitmap size
		int width = imageRectangle.size.x;
		int height = imageRectangle.size.y;

		// memory buffer for bitmap
		HDC memoryDC = ::CreateCompatibleDC( this->CanvasType::itsHdc );
		HGDIOBJ oldMemoryBitmap = ::SelectObject( memoryDC, ::CreateCompatibleBitmap( this->CanvasType::itsHdc, width, height ) );
		HGDIOBJ oldBitmap = ::SelectObject( this->CanvasType::itsHdc, bitmap );
		::BitBlt( memoryDC, 0, 0, width, height, this->CanvasType::itsHdc, 0, 0, SRCCOPY );
		::SelectObject( this->CanvasType::itsHdc, oldBitmap );

		// memory buffer for AND mask
		HDC maskDC = ::CreateCompatibleDC( this->CanvasType::itsHdc );
		HGDIOBJ oldMaskBitmap = ::SelectObject( maskDC, ::CreateBitmap( width, height, 1, 1, NULL ) );

		// create AND mask
		COLORREF oldColor = ::SetBkColor( memoryDC, bitmapBackgroundColor );
		::BitBlt( maskDC, 0, 0, width, height, memoryDC, 0, 0, SRCCOPY );
		::SetBkColor( memoryDC, oldColor );

		if ( drawDisabled )     // draw bitmap disabled
		{
			// BitBlt the black bits in the monochrome bitmap into COLOR_3DHILIGHT
			// bits in the destination DC. The magic ROP comes from the Charles
			// Petzold's book
			HGDIOBJ oldBrush = ::SelectObject( this->CanvasType::itsHdc, ::CreateSolidBrush( ::GetSysColor( COLOR_3DHILIGHT ) ) );
			::BitBlt( this->CanvasType::itsHdc, imageRectangle.pos.x, imageRectangle.pos.y, width, height, maskDC, 0, 0, 0xB8074A );

			// BitBlt the black bits in the monochrome bitmap into COLOR_3DSHADOW
			// bits in the destination DC
			::DeleteObject( ::SelectObject( this->CanvasType::itsHdc, ::CreateSolidBrush( ::GetSysColor( COLOR_3DSHADOW ) ) ) );
			::BitBlt( this->CanvasType::itsHdc, imageRectangle.pos.x, imageRectangle.pos.y, width, height, maskDC, 0, 0, 0xB8074A );
			::DeleteObject( ::SelectObject( this->CanvasType::itsHdc, oldBrush ) );
		}
		else    // draw bitmap with transparency
		{
			// create memory buffer for another mask
			HDC backMaskDC = ::CreateCompatibleDC( memoryDC );
			HGDIOBJ oldBackMaskBitmap = ::SelectObject( backMaskDC, ::CreateBitmap( width, height, 1, 1, NULL ) );

			// create that mask
			::BitBlt( backMaskDC, 0, 0, width, height, maskDC, 0, 0, SRCCOPY );
			::BitBlt( backMaskDC, 0, 0, width, height, backMaskDC, 0, 0, NOTSRCCOPY );

			// set bitmap background to black
			::BitBlt( memoryDC, 0, 0, width, height, backMaskDC, 0, 0, SRCAND );

			::BitBlt( this->CanvasType::itsHdc, imageRectangle.pos.x, imageRectangle.pos.y, width, height, maskDC, 0, 0, SRCAND );
			::BitBlt( this->CanvasType::itsHdc, imageRectangle.pos.x, imageRectangle.pos.y, width, height, memoryDC, 0, 0, SRCPAINT );

			// clear
			::DeleteObject( ::SelectObject( backMaskDC, oldBackMaskBitmap ) );
			::DeleteDC( backMaskDC );
		}

		// clear
		::DeleteObject( ::SelectObject( memoryDC, oldMemoryBitmap ) );
		::DeleteObject( ::SelectObject( maskDC, oldMaskBitmap ) );

		::DeleteDC( maskDC );
		::DeleteDC( memoryDC );
	}

private:
	/// Creates and inits back-buffer for the given source
	void init( HDC source )
	{
		// get screen size
		int width = this->getDeviceCaps( HORZRES );
		int height = this->getDeviceCaps( VERTRES );

		// create memory buffer for the source and reset itsHDC
		itsSource = source;
		this->CanvasType::itsHdc = ::CreateCompatibleDC( source );

		// create and select bitmap for buffer
		itsOldBitmap = ( HBITMAP )::SelectObject( this->CanvasType::itsHdc, ::CreateCompatibleBitmap( source, width, height ) );
	}

	HDC itsSource; /// Buffer source
	HBITMAP itsOldBitmap; /// Buffer old bitmap
};

/// Class for painting objects and drawing in a windows
/** Helper class containing functions for drawing lines and objects inside a window.
  * <br>
  * Not meant to be directly instantiated, but rather instantiated through e.g. the
  * PaintCanvas or UpdateCanvas classes. <br>
  * Related classes <br>
  * <ul>
  * <li>UpdateCanvas</li>
  * <li>PaintCanvas</li>
  * <li>FreeCanvas</li>
  * <li>Pen</li>
  * </ul>
  */
class Canvas
{
public:
	/// Returns the Device Context for the Canvas
	/** Can be used to construct e.g. a Pen object or a HdcModeSetter object
	  */
	HDC getDc();

	/// Selects the given font
	/** Selects the given font for later text operations
	  */
	void selectFont( FontPtr font );

	/// Gets the device capabilities.
	/** HORZRES, VERTRES give pixels
	  */
	int getDeviceCaps( int nIndex );

	/// Moves to a X,Y point.  (But does not draw).
	/** Moves to x,y in the Device Context of the object.
	  */
	void moveTo( int x, int y );

	/// Moves to a specific Point.  (But does not draw).
	/** Moves to Point in the Device Context of the object.
	  */
	void moveTo( const Point & coord );

	/// Draws a line from the current position to a X,Y point.
	/** Draws to x,y in the Device Context of the object.
	  * Use line (below) if you know two coordinates.
	  */
	void lineTo( int x, int y );

	/// Draws a line from the current position to a specific Point.
	/** Draws to Point in the Device Context of the object.
	  * Use line (below) if you know two coordinates.
	  */
	void lineTo( const Point & coord );

	/// Draws a line in the Device Context.
	/** Draws a line from (xStart, yStart) to (xEnd, yEnd).
	  */
	void line( int xStart, int yStart, int xEnd, int yEnd );

	/// Draws a line from Start to End in the Device Context.
	/** Draws a line from Start to End. <br>
	  * An alternate for line( int xStart, int yStart, int xEnd, int yEnd )
	  */
	void line( const Point & start, const Point & end );

	/// Draws a line around a Rectangle without filling it.
	/** Draws a line from rect.pos to rect.pos + rect.size <br>
	  * (Use Rectangle if you want to fill it.)
	  */
	void line( const SmartWin::Rectangle & rect );

	/// Fills a polygon defined by vertices.
	/** Fills a polygon defined by vertices.
	  */
	void polygon( const Point points[], unsigned count );

	/// Fills a polygon defined by vertices.
	/** Fills a polygon defined by vertices.
	  */
	void polygon( POINT points[], unsigned count );

	/// Draws an ellipse in the Device Context.
	/** Draws an ellipse from (left, top) to (right, bottom).
	  */
	void ellipse( int left, int top, int right, int bottom );

	/// Draws an ellipse in the Device Context.
	/** Draws an ellipse within the given rectangle.
	  */
	void ellipse( const SmartWin::Rectangle & rect );

	/// Draws a Rectangle in the Device Context.
	/** Draws a Rectangle from (left, top) to (right, bottom).
	  * Uses the current Pen to outline, and the current Brush to fill it.
	  */
	void rectangle( int left, int top, int right, int bottom );

	/// Draws a Rectangle in the Device Context.
	/** Draws a Rectangle from (pos) to ( pos + size ).
	  * Uses the current Pen to outline, and the current Brush to fill it.
	  */
	void rectangle( const SmartWin::Rectangle & rect );

	/// Fills a Rectangle in the Device Context with the given brush.
	/** Fills a Rectangle from (left, top) to (right, bottom).
	  */
	void fillRectangle( int left, int top, int right, int bottom, Brush & brush );

	/// Fills a Rectangle in the Device Context with the given brush.
	/** Fills a Rectangle within the given Rectangle.
	  */
	void fillRectangle( const SmartWin::Rectangle & rect, Brush & brush );

	/// Sets the pixel at (x,y) to be pixcolor. Returns the old pixel color.
	/** Sets the pixel at (x,y) to be pixcol
	  */
	COLORREF setPixel( int x, int y, COLORREF pixcolor );

	/// Returns the pixel's color at (x,y).
	/** Returns the pixel's color at (x,y) in the Device Context of the object.
	  */
	COLORREF getPixel( int x, int y );

	/// Returns the pixel's color at given point.
	/** Returns the pixel's color at coord in the Device Context of the object.
	  */
	COLORREF getPixel( const Point & coord );

#ifndef WINCE
	/// Fills an area starting at (x,y) with the current brush.
	/// crColor specifies when to stop or what to fill depending on the filltype
	/// parameter.
	/** If fillTilColorFound is true filling continues outwards from (x,y) until we
	  * find the given color and stops there. <br>
	  * If it is false it will fill AS LONG as it finds the given color and stop when
	  * it finds another color. <br>
	  * Function returns true if any filling was done, if no filling was done at all
	  * it'll return false.
	  */
	bool extFloodFill( int x, int y, COLORREF color, bool fillTilColorFound );
#endif //!WINCE

	/// Draws given string within given Rectangle.
	/** Draw text within a rectangle according to given format.<br>
	  * The format can be any combination of:
	  * <ul>
	  * <li>DT_BOTTOM</li>
	  * <li>DT_CALCRECT</li>
	  * <li>DT_CENTER</li>
	  * <li>DT_EDITCONTROL</li>
	  * <li>DT_END_ELLIPSIS</li>
	  * <li>DT_EXPANDTABS</li>
	  * <li>DT_EXTERNALLEADING</li>
	  * <li>DT_HIDEPREFIX</li>
	  * <li>DT_INTERNAL</li>
	  * <li>DT_LEFT</li>
	  * <li>DT_MODIFYSTRING</li>
	  * <li>DT_NOCLIP</li>
	  * <li>DT_NOFULLWIDTHCHARBREAK</li>
	  * <li>DT_NOPREFIX</li>
	  * <li>DT_PATH_ELLIPSIS</li>
	  * <li>DT_PREFIXONLY</li>
	  * <li>DT_RIGHT</li>
	  * <li>DT_RTLREADING</li>
	  * <li>DT_SINGLELINE</li>
	  * <li>DT_TABSTOP</li>
	  * <li>DT_TOP</li>
	  * <li>DT_VCENTER</li>
	  * <li>DT_WORDBREAK</li>
	  * <li>DT_WORD_ELLIPSIS</li>
	  * </ul>
	  * Google for or look at MSDN what their different meaning are.
	  */
	int drawText( const SmartUtil::tstring & text, const SmartWin::Rectangle & rect, unsigned format );

	/// Draws given text inside given Rectangle
	/** Draw text within coordinates of given Rectangle according to <br>
	  * setTextColor, setTextAlign, SetTextJustification
	  */
	void extTextOut( const SmartUtil::tstring & text, unsigned x, unsigned y );

	/// Sets the TextColor of the this Canvas.
	/** Sets the TextColor for future TextOut() calls. Returns the previous color.
	  */
	COLORREF setTextColor( COLORREF crColor );

	/// Sets the background color for the this Canvas
	/** Sets the background color for extTextOut() calls.
	  * Returns the previous background color
	  */
	COLORREF setBkColor( COLORREF crColor );

	/// Sets the background mode
	/** Can either be transparent (true) or opaque (false). Returns true if the
	  * previous background mode was transparent, false if it was opaque
	  */
	bool setBkMode( bool transparent = false );

	/// Gets the background color for the this Canvas
	/** Gets the background color for extTextOut() calls.
	  * Returns the current background color.
	  */
	COLORREF getBkColor();

	/// Gets the color for an system display object.
	/** Example: getSysColor( COLOR_WINDOW )
	  */
	COLORREF getSysColor( int index );

	/// Sets the alignment mode for text operations
	/** Returns the previous alignement mode and changes the current mode of text
	  * operations. Possible values can be any combination of these:
	  * <ul>
	  * <li>TA_CENTER</li>
	  * <li>TA_LEFT</li>
	  * <li>TA_RIGHT</li>
	  * <li>TA_BASELINE</li>
	  * <li>TA_BOTTOM</li>
	  * <li>TA_TOP</li>
	  * <li>TA_NOUPDATECP</li>
	  * <li>TA_UPDATECP</li>
	  * </ul>
	  */
	unsigned setTextAlign( unsigned fMode );

protected:
	/// Not meant for directly instantiation
	/** Class basically serves as an "abstract" base class for PaintCanvas and
	  * UpdateCanvas. <br>
	  * You should not directly instantiate this class but instead use one of the
	  * derived classes.
	  */
	explicit Canvas( HWND hWnd );

	/// Not meant for directly instantiation
	/** Class basically serves as an "abstract" base class for PaintCanvas and
	  * UpdateCanvas. <br>
	  * You should not directly instantiate this class but instead use one of the
	  * derived classes.
	  */
	explicit Canvas( Widget * widget );

	/// Protected Constructor to prevent deletion of class directly
	/** Derived class should delete, basically a hack to prevent deletion of a base
	  * class pointer
	  */
	virtual ~Canvas()
	{}

	/// Handle to the Device Context of the object.
	/** Derived classes needs access to this to e.g. call BeginPaint and EndPaint
	  */
	HDC itsHdc;

	/// Handle to the window of the object
	/** Handle of the window that the Canvas is for
	  */
	HWND itsHandle;
};

/// Class for painting within a WM_PAINT message
/** Helper class for securing that EndPaint is called for every BeginPaint we call.
  * <br>
  * BeginPaint is called in Constructor and EndPaint in DTOR. <br>
  * Inside a beenPainting event handler you have access to an object of this type.
  * <br>
  * Related classes<br>
  * <ul>
  * <li>UpdateCanvas</li>
  * <li>Canvas</li>
  * <li>Pen</li>
  * </ul>
  */
class PaintCanvas : public Canvas
{
public:
	/// Constructor, automatically calls BeginPaint
	/** Takes the handle to the window we're supposed to paint.<br>
	  */
	explicit PaintCanvas( HWND hWnd );

	/// Constructor, automatically calls BeginPaint
	/** Takes a pointer to the Widget we're supposed to paint.<br>
	  */
	explicit PaintCanvas( Widget * widget );

	/// DTOR, automatically calls EndPaint
	/** Automatically calls end paint when object goes out of scope.
	  */
	virtual ~PaintCanvas();

private:
	PAINTSTRUCT itsPaint;
	PaintCanvas( const PaintCanvas & ); // Never defined, class cannot be copied!

	// Initializes the object
	// Common for all ConstructorS
	void initialize();
};

/// Class for painting outside a WM_PAINT message
/** Helper class for securing that GetDC and ReleaseDC is called in Constructor and
  * DTOR. <br>
  * Instantiate an object if this type where ever you want except within a WM_PAINT
  * event handler (where you <br>
  * should rather use PaintCanvas) to get access to draw objects within a window.
  * <br>
  * Then when Widget is invalidated or updated your "painting" will show. <br>
  * Related classes<br>
  * <ul>
  * <li> PaintCanvas</li>
  * <li> Canvas</li>
  * <li> Pen</li>
  * </ul>
  */
class UpdateCanvas : public Canvas
{
public:
	/// Constructor, automatically calls GetDC
	/** Takes the handle to the window we're supposed to paint.
	  */
	explicit UpdateCanvas( HWND hWnd );

	/// Constructor, automatically calls GetDC
	/** Takes a pointer to the Widget we're supposed to paint.
	  */
	UpdateCanvas( Widget * widget );

	/// DTOR, automatically calls ReleaseDC
	/** Automtaically releases the Device Context
	  */
	virtual ~UpdateCanvas();

private:
	// Initializes the object, common for all Constructors
	void initialize();
};

/// Class for painting on an already created canvas which we don't own ourself
/** Note this class does NOT create or instantiate a HDC like the PaintCanvas and the
  * UpdateCanvas. It assumes that the given HDC is already valid and instantiated. If
  * you need to paint or update a Widget and you don't have a valid HDC use the
  * UpdateCanvas or within a onPainting event handler use the PaintCanvas. Related
  * classes<br>
  * <ul>
  * <li>PaintCanvas</li>
  * <li>UpdateCanvas</li>
  * <li>Canvas</li>
  * <li>Pen</li>
  * </ul>
  */
class FreeCanvas : public Canvas
{
public:
	/// Constructor, assigns given HWND and HDC to object
	/** Takes the handle to the window we're supposed to paint.
	  */
	FreeCanvas( HWND hWnd, HDC hdc );

	/// Constructor, assigns the given HDC to the object
	/** Takes a pointer to the Widget we're supposed to paint.
	  */
	FreeCanvas( Widget * widget, HDC hdc );

	virtual ~FreeCanvas()
	{}
};

#ifndef WINCE
// TODO: Create custom enums for typesafety... ?
/// Helper class for setting and resetting the ROP2 mode
/** The ROP2 mode is used for telling windows which type of brush you want while
  * painting. <br>
  * this class ensures we reset the ROP2 mode after we have fiinished with it. <br>
  * Used in combination with e.g. UpdateCanvas or PaintCanvas. <br>
  * Supported modes are those supported in Windows API
  * <ul>
  * <li>R2_BLACK Pixel is always 0. </li>
  * <li>R2_COPYPEN Pixel is the pen color. </li>
  * <li>R2_MASKNOTPEN Pixel is a combination of the colors common to both the screen and the inverse of the pen. </li>
  * <li>R2_MASKPEN Pixel is a combination of the colors common to both the pen and the screen. </li>
  * <li>R2_MASKPENNOT Pixel is a combination of the colors common to both the pen and the inverse of the screen. </li>
  * <li>R2_MERGENOTPEN Pixel is a combination of the screen color and the inverse of the pen color. </li>
  * <li>R2_MERGEPEN Pixel is a combination of the pen color and the screen color. </li>
  * <li>R2_MERGEPENNOT Pixel is a combination of the pen color and the inverse of the screen color. </li>
  * <li>R2_NOP Pixel remains unchanged. </li>
  * <li>R2_NOT Pixel is the inverse of the screen color. </li>
  * <li>R2_NOTCOPYPEN Pixel is the inverse of the pen color. </li>
  * <li>R2_NOTMASKPEN Pixel is the inverse of the R2_MASKPEN color. </li>
  * <li>R2_NOTMERGEPEN Pixel is the inverse of the R2_MERGEPEN color. </li>
  * <li>R2_NOTXORPEN Pixel is the inverse of the R2_XORPEN color. </li>
  * <li>R2_WHITE Pixel is always 1. </li>
  * <li>R2_XORPEN Pixel is a combination of the colors in the pen and in the screen, but not in both.</li>
  * </ul>
  * Related classes
  * <ul>
  * <li>UpdateCanvas</li>
  * <li>PaintCanvas</li>
  * <li>FreeCanvas</li>
  * <li>Canvas</li>
  * <li>Pen</li>
  * </ul>
  */
class HdcModeSetter
{
public:
	/// Constructor setting the ROP2 of the given Device Context.
	/** The mode is any of the above enums.
	  */
	HdcModeSetter( Canvas & canvas, int mode );

	/// Automatically resets the ROP2
	/** Resets the mode back to what it was before this object was created. Note this
	  * object cannot have longer lifetime than the Canvas given to the Constructor
	  * since this will cause undefined behavior.
	  */
	~HdcModeSetter();

private:
	int itsOldMode;
	Canvas & itsCanvas;
};
#endif //! WINCE

// TODO: Create a custom object for creating COLORREF...?
/// Class for control of a pens lifetime
/** Constructor takes a COLORREF which can be obtained by using e.g. the RGB macro.
  * <br>
  * Class ensures that when finished with your pen your pen is released and the
  * former one is reactivated. <br>
  * Automatically resets back to the former used pen when object is destroyed. <br>
  * The hdc parameter to the Constructor is the Device Context you wish to create the
  * pen in. <br>
  * Related classes<br>
  * <ul>
  * <li>UpdateCanvas</li>
  * <li>PaintCanvas</li>
  * <li>FreeCanvas</li>
  * <li>Canvas</li>
  * <li>Brush</li>
  * <li>TextPen</li>
  * <li>HdcModeSetter</li>
  * </ul>
  */
class Pen
{
public:
	/// Returns the Canvas the pen belongs to
	/** Normally this would be the same as the "parent" object used to construct the
	  * Pen
	  */
	Canvas & getCanvas();

	/// Returns the Handle to the pen
	/** Returns the Handle to the pen
	  */
	HPEN getPenHandle();

	/// Constructor taking a Canvas, a COLORREF, and optionally a width.
	/** Build a COLORREF argument with windows.h's RGB( red, green, blue ). The
	  * optional width is the width in pixels of the line drawn by the pen.
	  */
	Pen( Canvas & canvas, COLORREF color, int width = 0 );

	/// Automatically resets the pen back to the former pen and frees the given pen.
	/** Freeing the current pen and resetting back to the former pen. Note that the
	  * Pen object cannot have a longer lifetime than the Canvas it belongs to (given
	  * to the Constructor) since this will cause undefined behaviour.
	  */
	~Pen();

private:
	// Handle to the old pen (this pen will be restored in the DTOR of the object)
	HPEN itsPenOld;

	// Handle to the actual pen.
	HPEN itsPen;

	// Handle to its Device Context.
	Canvas & itsCanvas;
};

/// Class for control of a brush lifetime
/** Constructor takes a COLORREF which can be obtained by using e.g. the RGB macro.
  * <br>
  * Class ensures that your brush is released and the former one is reactivated. <br>
  * Automatically resets back to the former used brush when object is destroyed. <br>
  * The hdc parameter to the Constructor is the Device Context for the brush. <br>
  * Related classes<br>
  * <ul>
  * <li>UpdateCanvas</li>
  * <li>PaintCanvas</li>
  * <li>Canvas</li>
  * <li>Pen</li>
  * <li>HdcModeSetter</li>
  * </ul>
  */
class Brush
{
public:

	enum SysColor
	{
		Scrollbar = COLOR_SCROLLBAR,
		Background = COLOR_BACKGROUND,
		ActiveCaption = COLOR_ACTIVECAPTION,
		InActiveCaption = COLOR_INACTIVECAPTION,
		Menu = COLOR_MENU,
		Window = COLOR_WINDOW,
		WindowFrame = COLOR_WINDOWFRAME,
		MenuText = COLOR_MENUTEXT,
		WindowText = COLOR_WINDOWTEXT,
		CaptionText = COLOR_CAPTIONTEXT,
		ActiveBorder = COLOR_ACTIVEBORDER,
		InActiveBorder = COLOR_INACTIVEBORDER,
		AppWorkSpace = COLOR_APPWORKSPACE,
		HighLight = COLOR_HIGHLIGHT,
		HighLightText = COLOR_HIGHLIGHTTEXT,
		BtnFace = COLOR_BTNFACE,
		BtnShadow = COLOR_BTNSHADOW,
		GrayText = COLOR_GRAYTEXT,
		BtnText = COLOR_BTNTEXT,
		InActiveCaptionText = COLOR_INACTIVECAPTIONTEXT,
		BtnHighLight = COLOR_BTNHIGHLIGHT,
		//3DDkShadow                        = COLOR_3DDKSHADOW,
		//3DLight                           = COLOR_3DLIGHT,
		InfoText = COLOR_INFOTEXT,
		InfoBk = COLOR_INFOBK,
#ifdef WINCE
		Static = COLOR_STATIC,
		StaticText = COLOR_STATICTEXT,
#else  //! WINCE
		Static = COLOR_BACKGROUND, // try ?
		StaticText = COLOR_BTNTEXT,
#endif
		GradientActiveCaption = COLOR_GRADIENTACTIVECAPTION,
		GradientInActiveCaption = COLOR_GRADIENTINACTIVECAPTION
	};

	/// Returns the Device Context of the brush.
	/** Normally the same as the "parent" object used to construct the Pen
	  */
	Canvas & getCanvas();

	/// Returns the Handle to the brush
	/** Returns the Handle to the brush
	  */
	HBRUSH getBrushHandle();
	
	/// Constructor for a free brush (not tied to a canvas)
	Brush(COLORREF color);

	/// Constructor taking a Canvas and a COLORREF
	/** Build a COLORREF argument with windows.h's RGB( red, green, blue )
	  */
	Brush( Canvas & canvas, COLORREF color );

	/// Constructor taking a Canvas and a COLORREF
	/** Build a COLORREF argument with windows.h's RGB( red, green, blue )
	  */
	Brush( Canvas & canvas, BitmapPtr bitmap );

	/// Constructor taking a Canvas and a syscolorbrush enum
	/** A syscolorbrush is one of the system brushes used to draw e.g. Buttons, Text,
	  * etc. Use this Constructor if you wish to use one of those cached brushes
	  * instead of creating your own
	  */
	Brush( Canvas & canvas, SysColor color );

	/// Static Constructor returning a "NULL_BRUSH"
	Brush( Canvas & canvas );

	/// Automatically restores the old brush and frees the given brush.
	/** Freeing the current brush and resetting back to the former brush. Note that
	  * the Pen object cannot have a longer lifetime than the Canvas it belongs to
	  * (given to the Constructor) since this will cause undefined behaviour.
	  */
	~Brush();

private:
	// NEVER IMPLEMENTED INTENTIONALLY, DENY COPYING!
	Brush( const Brush & rhs );

	// Handle to the old brush (this brush will be restored in the DTOR of the object)
	HBRUSH itsBrushOld;

	// true if SysColorBrush
	bool isSysColor;

	// Handle to the actual brush.
	HBRUSH itsBrush;

	// Handle to its Device Context.
	Canvas* itsCanvas;
};

/// Class for control of the text color lifetime
/** Constructor takes a COLORREF. (Use the RGB macro. <br>
  * Class ensures that the previous text color is restored on object destruction.
  * <br>
  * Related classes<br>
  * <ul>
  * <li>Canvas</li>
  * <li>Pen</li>
  * </ul>
  */
class TextPen
{
public:
	/// Constructor taking a Canvas and a COLORREF
	/** Build a COLORREF argument with windows.h's RGB( red, green, blue )
	  */
	TextPen( Canvas & canvas, COLORREF color );

	/// Automatically restores the old TextColor.
	/** Since this is a RAII structure it ensures the old pen is restored upon
	  * destruction. Be careful though since if this oulives the Canvas object given
	  * undefined behaviour will occur!
	  */
	~TextPen();

private:
	// Handle to the old color which will be restored in the DTOR of the object)
	COLORREF itsColorOld;

	// Handle to its Device Context.
	Canvas & itsCanvas;
};

/// \ingroup GlobalStuff
/// Brush pointer
/** Use this typedef instead to ensure compatibility in future versions of SmartWin!!
  */
typedef std::tr1::shared_ptr< Brush > BrushPtr;

// end namespace SmartWin
}

#endif

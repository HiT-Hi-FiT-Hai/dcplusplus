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

#ifndef DWT_SMARTWIN_RECTANGLE_H_
#define DWT_SMARTWIN_RECTANGLE_H_

#include "Point.h"

namespace dwt {
/// Data structure for defining a rectangle
/** \ingroup WidgetLayout
  * The member functions are helpful in dividing large rectangles into smaller ones,
  * which is exactly what is needed to layout widgets in windows.
  */
struct Rectangle {
	Point pos;
	
	Point size;
	
	/// Constructor initializing the rectangle to (0, 0, 0, 0).
	/** Default constructor initializing everything to zero (0)
	  */
	Rectangle() { };
	
	Rectangle(const ::RECT& rc) : pos(rc.left, rc.top), size(rc.right - rc.left, rc.bottom - rc.top) { }

	/// Constructor initializing the rectangle with a position and size.
	/** Note that the pSize is actually a size and NOT the lower right Point.
	  */
	Rectangle( const Point & pPos, const Point & pSize ) : pos(pPos), size(pSize) { }

	/// Constructor initializing the rectangle with a size.
	/** Top-left becomes (0, 0), while bottom-right is set to pSize.
	  */
	explicit Rectangle( const Point & pSize ) : pos(0, 0), size(pSize) { }
	
	operator ::RECT() const;

	/// Constructor initializing the rectangle with longs instead of Points.
	/** ( x,y ) defines the upper right corner, ( x+width, y+height ) defines the
	  * lower left corner.
	  */
	Rectangle( long x, long y, long width, long height );
		
	long left() const { return pos.x; }
	long x() const { return left(); }
	
	long top() const { return pos.y; }
	long y() const {return top(); }
	
	long right() const { return left() + width(); }
	
	long bottom() const { return top() + height(); }
	
	long width() const { return size.x; }

	long height() const { return size.y; }
	
	const Point& upperLeft() const { return pos; }
	
	/// Return the lower right point of the rectangle.
	/** 
	  * Example: <br>
	  * Rectangle r1( 10, 10, 100, 200 ); <br>
	  * Point lr = r1.LowRight() <br>
	  * gives lr.x = 110, lr.y = 210
	  */
	Point lowRight() const { return Point(x() + width(), y() + height()); }

	/// Creates a sub rectangle from an old rectangle.
	/** The pos is adjusted by the xFraction and yFraction of the
	  * width and height of r.  The size is also shrunk.
	  *  r1.SubRect( 0, 0, 1, 0.5 )     gives the upper half of r1 <br>
	  *  r1.SubRect( 0.5, 0, 0.5, 1 )   gives the right half of r1 <br>
	  *  r1.SubRect( .334, .334, .334, .334 ) centers  r2 inside r1 <br>
	  *  r1.SubRect( .2, .2, .6, .6 ) also centers  r2 inside r1 <br>
	  * <pre>
	  * OOOOOOOOOO    is given by subRect( 0.3, 0.5, 0.4, 0.5 );
	  * OOOOOOOOOO    (x moved 30%, y moved 50% )
	  * OOO++++OOO    (x resized to 40%, y resized to 50% )
	  * OOO++++OOO
	  * </pre>
	  */
	Rectangle subRect( double xFraction, double yFraction,
				  double widthFraction, double heightFraction ) const;

	/// Size of the rectangle will be * factor, Position adjusted for the same center.
	/** Creates a smaller rectangle from the old rectangle. <br>
	  * size.x *= factor, and pos.x is adjusted inwards to compensate. <br>
	  * <pre>
	  * ####
	  * ####    ->  ##
	  * ####        ##
	  * ####
	  * </pre>
	  * shows the effect of shrink( 0.5 ) <br>
	  * shrink( long border ) is similar, but removes a constant border amount of
	  * pixels on all sides.
	  */
	Rectangle shrink( double factor ) const;

	/// Move inwards by xBorder and shrink the size by 2*xBorder
	/** A rectangle of #### changes to ##.<br>
	  * The rectangle becomes smaller at the left and right, but has the same center.
	  */
	Rectangle shrinkWidth( long xBorder ) const;

	/// Move inwards by yBorder and shrink the size by 2*yBorder
	/** <pre>
	  * ####
	  * ####    ->  ####
	  * ####        ####
	  * ####
	  *
	  * </pre>
	  * The rectangle becomes smaller at the top and bottom, but has the same center.
	  */
	Rectangle shrinkHeight( long yBorder ) const;

	/// Move inwards by both xBorder and yBorder and shrink the size by 2*yBorder
	/// and 2*xBorder
	/** <pre>
	  * ####
	  * ####    ->  ##
	  * ####        ##
	  * ####
	  * </pre>
	  * The rectangle shrinks, but has the same center.
	  */
	Rectangle shrink( long xBorder, long yBorder ) const;

	/// For both dimensions, move inwards by Border and shrink the size by 2*Border
	/** We add border to the position, and subtract it twice from the size. Same as
	  * shrink( long xBorder, long yBorder ); except the x and y border are the same.
	  * shrink( double factor ) is similar, but expresses the new rectangle as a
	  * fraction of the old.
	  */
	Rectangle shrink( long border ) const;

	/// Return the upper rectangle of height y
	/** We return the upper rectangle of height y.<br>
	  * Example: <br>
	  * Rectangle rect( 0,0, 100, 100 );<br>
	  * Rectangle t = rect.getTop( 10 );
	  * Now: t.pos = 0,0     t.size = 100,10<br>
	  */
	Rectangle getTop( long y ) const;

	/// Return the lower rectangle starting from y
	/** We return the lower rectangle of height y<br>
	  * Example: <br>
	  * Rectangle rect( 0,0, 100, 100 );<br>
	  * Rectangle t = rect.getBottom( 10 );
	  * Now: t.pos = 0,90     t.size = 100,10<br>
	  */
	Rectangle getBottom( long y ) const;

	/// Return the left rectangle of widght x.
	/** We return the left rectangle of width x<br>
	  * Example: <br>
	  * Rectangle rect( 0, 0, 100, 100 );<br>
	  * Rectangle t = rect.getLeft( 10 );
	  * Now: t.pos= 0,0     t.size= 10,100<br>
	  */
	Rectangle getLeft( long x ) const;

	/// Return the right rectangle of widght x.
	/** We return the right rectangle of width x<br>
	  * Example: <br>
	  * Rectangle rect( 0, 0, 100, 100 );<br>
	  * Rectangle t = rect.getRight( 10 );
	  * Now: t.pos= 90,0     t.size= 10,100<br>
	  */
	Rectangle getRight( long x ) const;

	/// Move the Upper Left position by adjust, and keep the same Lower Right corner.
	/** <pre>
	  * OOOOOO
	  * OOOO++
	  * OOOO++
	  * </pre>
	  * upperLeftAdjust( Point( 4, 1 ) ); will give the + rectangle afterwards.
	  */
	Rectangle upperLeftAdjust( const Point & adjust ) const;

	/// Move the Lower Right position by adjust, and keep the same Upper Left corner.
	/** <pre>
	  * ++++O
	  * ++++O
	  * OOOOO
	  * </pre>
	  * lowerRightAdjust( Point( -1, -1 ) ); will give the + rectangle afterwards.
	  */
	Rectangle lowerRightAdjust( const Point & adjust ) const;

	/// Produce the Left portion of a Rectangle with portion width, same height,
	/// same position.
	/** If the original rectangle is as below: <br>
	  * <pre>
	  * XXXOOOOOOO
	  * XXXOOOOOOO
	  * XXXOOOOOOO
	  * XXXOOOOOOO
	  * </pre>
	  * left( 0.3 )returns the X rectangle.  <br>
	  */
	Rectangle toleft( double portion = 0.5 ) const;

	/// Produce a Rectangle with the portion % width, same height, position moved to right.
	/** Produce a Rectangle with the portion % width, same height, position moved to right. <br>
	  * <pre>
	  * OOOOOOO###
	  * OOOOOOO###
	  * OOOOOOO###
	  * OOOOOOO###
	  * </pre>
	  * right( 0.3 )returns the # rectangle.  <br>
	  */
	Rectangle toright( double portion = 0.5 ) const;

	/// Produce a Rectangle with the portion % height, same width, same position.
	/** Produce a Rectangle with the portion % height, same width, same position. <br>
	  * <pre>
	  * ##########
	  * ##########
	  * OOOOOOOOOO
	  * OOOOOOOOOO
	  * </pre>
	  * top( 0.5 ) or Top() returns the # rectangle.  <br>
	  */
	Rectangle upper( double portion = 0.5 ) const;

	/// Produce a Rectangle with the portion % height, same width, position moved downwards.
	/** Produce a Rectangle with the portion % height, same width, position moved downwards. <br>
	  * <pre>
	  * OOOOOOOOOO
	  * ##########
	  * ##########
	  * ##########
	  * </pre>
	  * Bottom( 0.75 ) returns the # rectangle.  <br>
	  */
	Rectangle lower( double portion = 0.5 ) const;

	/// Produce a Rectangle with the 1/rows % height, same width, rowth position.
	/** Produce a Rectangle with the 1/rows % height, same width, rowth position. <br>
	  *    row0 <br>
	  *    row1   <-- Row( 1, 3 ) will return the "row1" rectangle.<br>
	  *    row2 <br>
	  */
	Rectangle row( int row, int rows ) const;

	/// Produce a Rectangle with the 1/cols % width, same height, colth position.
	/** Produce a Rectangle with the 1/cols % width, same height, colth position. <br>
	  *<br>
	  *    col0  col1  col2  col3 <br>
	  *<br>
	  *Col( 2, 4 ) will return the "col2" rectangle.<br>
	  */
	Rectangle col( int column, int columns ) const;

	/// Produce a Rectangle with the top portion removed
	Rectangle cropTop( const int a_ToRemove ) const;

	/// Produce a Rectangle with the bottom portion removed
	Rectangle cropBottom( const int a_ToRemove ) const;

	/// Produce a Rectangle with the left portion removed
	Rectangle cropLeft( const int a_ToRemove ) const;

	/// Produce a Rectangle with the right portion removed
	Rectangle cropRight( const int a_ToRemove ) const;
};

bool operator==(const Rectangle& lhs, const Rectangle& rhs);

/// \ingroup GlobalStuff
/// "Default" Rectangle for window creation
/**  The system selects the default position/size for the window.
  */
extern const Rectangle letTheSystemDecide;

}

#endif /*RECTANGLE_H_*/

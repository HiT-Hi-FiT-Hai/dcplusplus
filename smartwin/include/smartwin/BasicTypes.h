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
#ifndef BasicTypes_h
#define BasicTypes_h

#include "WindowsHeaders.h"
#include "../../SmartUtil/tstring.h"

namespace SmartWin
{
// begin namespace SmartWin

/// POD structure for defining a point
/** Used in e.g. functions that take a mouse position etc...
  */
struct Point : POINT
{
	/// Constructor initializing the point with the given arguments.
	/** Constructor initializing the structure with the given arguments. Takes x and
	  * y coordinate to be used.
	  */
	Point( long x, long y );

	/// Constructor Initializing the point to (0,0)
	/** Default constructor initializing x and y member to 0 and 0
	  */
	Point();

	Point(const POINT& pt);
	
	operator POINT() const;
	
	/// Sets this Point to the maximum value for each x y dimension.
	/** Each x,y dimension is adjusted by the p Point.
	  */
	void maxOf( const Point & p );

	/// Sets this Point to the minimum value for each x y dimension.
	/** Each x,y dimension is adjusted by the p Point.
	  */
	void minOf( const Point & p );
};

class ScreenCoordinate {
public:
	ScreenCoordinate() { }
	ScreenCoordinate(const ScreenCoordinate& sc) : point(sc.point) { }
	
	explicit ScreenCoordinate(const Point& pt) : point(pt) { }
	
	const Point& getPoint() const { return point; }
	Point& getPoint() { return point; }
	
	long x() const { return getPoint().x; }
	long y() const { return getPoint().y; }
	
	ScreenCoordinate& operator=(const ScreenCoordinate& rhs) { point = rhs.point; return *this; }
private:
	Point point;
};

class Widget;

class ClientCoordinate {
public:
	explicit ClientCoordinate(const ClientCoordinate& cc, Widget* w_);
	
	explicit ClientCoordinate(const ScreenCoordinate& sc, Widget* w_);
	
	explicit ClientCoordinate(const Point& pt, Widget* w_) : point(pt), w(w_) { }

	operator ScreenCoordinate() const;

	const Point& getPoint() const { return point; }
	Point& getPoint() { return point; }

	long x() const { return getPoint().x; }
	long y() const { return getPoint().y; }

private:
	Point point;
	Widget* w;
};

/// \ingroup GlobalStuff
/// Adds a Point to the left hand side point
/** Basically just adds up the x and y coordinates of the two points and puts the
  * result into the left hand side Point
  */
Point & operator += ( Point & lhs, const Point & rhs );

/// \ingroup GlobalStuff
/// Adds two Points and returns the result
/** Basically just adds up the x and y coordinates of the two points and puts the
  * result into a new Point and returns that Point
  */
Point operator +( const Point & lhs, const Point & rhs );

/// \ingroup GlobalStuff
/// Subtracts a Point to the left hand side point
/** Basically just subtracts the x and y coordinates of the two points and puts the
  * result into the left hand side Point
  */
Point & operator -= ( Point & lhs, const Point & rhs );

/// \ingroup GlobalStuff
/// Subtracts two Points and returns the result
/** Basically just subtracts up the x and y coordinates of the two points and puts
  * the result into a new Point and returns that Point
  */
Point operator -( const Point & lhs, const Point & rhs );

/// \ingroup GlobalStuff
/// Returns true if the two Points are equal
/** Operator == for checking for equality
  */
bool operator == ( const Point & lhs, const Point & rhs );

/// \ingroup GlobalStuff
/// Returns false if the two Points are equal
/** Operator != for checking for equality
  */
bool operator != ( const Point & lhs, const Point & rhs );

/// Data structure for defining a rectangle
/** \ingroup WidgetLayout
  * The two Point data members, pos and size, define a rectangle. <br>
  * pos has the x,y position of the upper-left rectangle coordinate. <br>
  * size defines the x and y distance between the upper left and the lower right
  * coordinate. <br>
  * (pos + size) defines the lower right coordinate. <br>
  * Many window functions takes or returns a Rectangle. <br>
  * See the Layout module documentation for the use of Rectangle in positioning
  * widgets. <br>
  * The member functions are helpful in dividing large rectangles into smaller ones,
  * which is exactly what is needed to layout widgets in windows.
  */
struct Rectangle
{
	/// Position of the upper left corner of the Rectangle
	Point pos;

	/// Size of the Rectangle
	Point size;

	/// Constructor initializing the rectangle to (0, 0, 0, 0).
	/** Default constructor initializing everything to zero (0)
	  */
	Rectangle();
	
	Rectangle(const RECT& rc);

	/// Constructor initializing the rectangle with a position and size.
	/** Note that the pSize is actually a size and NOT the lower right Point.
	  */
	Rectangle( const Point & pPos, const Point & pSize );

	/// Constructor initializing the rectangle with a size.
	/** Note that the pSize is actually a size and NOT the lower right Point,
	  * position is defaulted to 0,0.
	  */
	explicit Rectangle( const Point & pSize );

	/// Constructor initializing the rectangle with longs instead of Points.
	/** ( x,y ) defines the upper right corner, ( x+width, y+height ) defines the
	  * lower left corner.
	  */
	Rectangle( long x, long y, long width, long height );

	/// Constructor creates a new rectangle with a fractional position and size of
	/// the old rect.
	/** It computes the new rectangle by using subrect(): <br>
	  *    (xFraction * size.x) is the amount to add to pos.x <br>
	  *    size.x *= widthFraction <br>
	  * <br>
	  * <pre>
	  * Examples :
	  *  upper half is r2( r1, 0, 0, 1, 0.5 )   (Same position, same width, 0.5 height )
	  *  lower half is r2( r1, 0, 0.5, 1, 0.5 ) (y half down, same width, 0.5 height )
	  *  left  half is r2( r1, 0, 0, 0.5, 1 )   (Same position, 0.5 width, same height )
	  *  right half is r2( r1, 0.5, 0, 0.5, 1 )
	  *  Lower right quarter is r2( r1, .5, .5, .5, .5)
	  *  center r2 inside r1 is r2( r1, .334, .334, .334, .334)
	  * </pre>
	  */
	Rectangle( const Rectangle & rect,
				double xFraction, double yFraction,
				double widthFraction, double heightFraction );

	operator RECT() const;

	/// Return the lower right point of the rectangle.
	/** Note that the rectangle is defined with pos, and a size, so we need this
	  * function. <br>
	  * Example: <br>
	  * Rectangle r1( 10, 10, 100, 200 ); <br>
	  * Point lr = r1.LowRight() <br>
	  * gives lr.x = 110, lr.y = 210
	  */
	Point lowRight() const;

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
	Rectangle left( double portion = 0.5 ) const;

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
	Rectangle right( double portion = 0.5 ) const;

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
	Rectangle top( double portion = 0.5 ) const;

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
	Rectangle bottom( double portion = 0.5 ) const;

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

// end namespace SmartWin
}

#endif

#include "../include/smartwin/Rectangle.h"

namespace SmartWin {

Rectangle::Rectangle( long x, long y, long width, long height )
	: pos( x, y ), size( width, height )
{}

Rectangle::operator RECT() const
{
	RECT retVal = { left(), top(), right(), bottom() };
	return retVal;
}

Rectangle Rectangle::subRect( double xFraction, double yFraction,
				double widthFraction, double heightFraction ) const
{
	Rectangle retVal;
	retVal.pos.x = pos.x + ( long )( xFraction * size.x );
	retVal.pos.y = pos.y + ( long )( yFraction * size.y );

	retVal.size.x = ( long )( size.x * widthFraction );
	retVal.size.y = ( long )( size.y * heightFraction );
	return retVal;
}

// Size of the rectangle will be * factor.
// Position adjusted for the same center.
Rectangle Rectangle::shrink( double factor ) const
{
	double posFactor = ( 1.0 - factor ) * 0.5;
	return subRect( posFactor, posFactor, factor, factor );
}

Rectangle Rectangle::shrinkWidth( long xBorder ) const
{
	Rectangle retVal = * this;
	retVal.pos.x = pos.x + xBorder;
	retVal.size.x = size.x - ( xBorder + xBorder );
	return retVal;
}

Rectangle Rectangle::shrinkHeight( long yBorder ) const
{
	Rectangle retVal = * this;
	retVal.pos.y += yBorder;
	retVal.size.y -= ( yBorder + yBorder );
	return retVal;
}

Rectangle Rectangle::shrink( long border ) const
{
	Rectangle retVal = shrinkWidth( border );
	retVal = retVal.shrinkHeight( border );
	return retVal;
}

Rectangle Rectangle::shrink( long xBorder, long yBorder ) const
{
	Rectangle retVal = shrinkWidth( xBorder );
	retVal = retVal.shrinkHeight( yBorder );
	return retVal;
}

Rectangle Rectangle::getTop( long y ) const
{
	Rectangle top = Rectangle( pos, Point( size.x, y ) );
	return top;
}

Rectangle Rectangle::getBottom( long y ) const
{
	Rectangle bottom = Rectangle(
		Point( pos.x, pos.y + size.y - y ),
		Point( size.x, y ) );
	return bottom;
}

Rectangle Rectangle::getLeft( long x ) const
{
	Rectangle left = Rectangle( pos, Point( x, size.y ) );
	return left;
}

Rectangle Rectangle::getRight( long x ) const
{
	Rectangle bottom = Rectangle(
		Point( pos.x + size.x - x, pos.y ),
		Point( x, size.y ) );
	return bottom;
}

Rectangle Rectangle::upperLeftAdjust( const Point & adjust ) const
{
	Rectangle retVal = * this;
	retVal.pos.x += adjust.x;
	retVal.pos.y += adjust.y;
	retVal.size.x -= adjust.x;
	retVal.size.y -= adjust.y;
	return retVal;
}

Rectangle Rectangle::lowerRightAdjust( const Point & adjust ) const
{
	Rectangle retVal = * this;
	retVal.size.x += adjust.x;
	retVal.size.y += adjust.y;
	return retVal;
}

Rectangle Rectangle::row( int row, int rows ) const
{
	int rowheight = size.y / rows;
	return Rectangle( pos.x, pos.y + ( row * rowheight ), size.x, rowheight );
}

Rectangle Rectangle::col( int column, int columns ) const
{
	int colwidth = size.x / columns;
	return( Rectangle( pos.x + ( colwidth * column ), pos.y, colwidth, size.y ) );
}

Rectangle Rectangle::cropTop( const int a_ToRemove ) const
{
	register int d_NewSize = size.y - a_ToRemove;

	return Rectangle( pos.x, pos.y + a_ToRemove, size.x, d_NewSize > 0 ? d_NewSize : 0 );
}

Rectangle Rectangle::cropBottom( const int a_ToRemove ) const
{
	register int d_NewSize = size.y - a_ToRemove;

	return Rectangle( pos.x, pos.y, size.x, d_NewSize > 0 ? d_NewSize : 0 );
}

Rectangle Rectangle::cropLeft( const int a_ToRemove ) const
{
	register int d_NewSize = size.x - a_ToRemove;

	return Rectangle( pos.x + a_ToRemove, pos.y, d_NewSize > 0 ? d_NewSize : 0, size.y );
}

Rectangle Rectangle::cropRight( const int a_ToRemove ) const
{
	register int d_NewSize = size.x - a_ToRemove;

	return Rectangle( pos.x, pos.y, d_NewSize > 0 ? d_NewSize : 0, size.y );
}

bool operator==(const Rectangle& lhs, const Rectangle& rhs) {
	return lhs.pos == rhs.pos && lhs.size == rhs.size;
}

const SmartWin::Rectangle letTheSystemDecide( CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT );
}

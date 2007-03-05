// $Revision: 1.9 $
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
#ifndef Place_h
#define Place_h

namespace SmartWin
{
// begin namespace SmartWin

/// A class to position objects in a rectangular area.
/** \ingroup WidgetLayout
  * You start by using one of the setBoundsBorders calls to establish the rectangular
  * area. This call gives you the option of setting x and y borders around the
  * objects.
  *
  * The size of the objects are assumed to be known, but you can use sizeOfCell to
  * calculate what the size of an object should be to fit perfectly into a grid
  * arrangement in the area.
  *
  * Then you use  positionToRight( struct Rectangle & obj ), passing only the
  * obj.size, and receiving the calculated obj.pos. This gives a rectangle suitable
  * for the AspectSizable functions.
  *
  * positionToRight's layout approach is left to right until the next line of objects
  * starts just below the previous line.
  *
  * These optional functions let you customize the placement operation: <br>
  * void newRow() <br>
  * void tabTo( double xfract ) <br>
  * void sizeForRemainingSpace( struct Rectangle &obj )
  *
  * positionBelow() and newCol() are similar functions in case you want to layout a
  * column of objects
  */
class Place
{
public:
	/// Constructor without parameters.
	/** In this constructor you can place Widgets without constraints. The parent
	  * window may not show them though. You add constraints with the
	  * SetBoundsBorders functions.
	  */
	Place()
	{
		SmartWin::Point lowright; lowright.x = lowright.y = 100000;
		setBoundsBorders( lowright, 0, 0 );
	}

	/// Take a Rectangle, which has a position and a size, and set the bounds
	/// and borders.
	/** This is suitable for cases in which you want place objects inside a rect
	  * that does NOT have to start at 0,0.
	  */
	void setBoundsBorders( const SmartWin::Rectangle & rect, const int borderX = 0,
		const int borderY = 0 )
	{
		setBoundsBorders( rect.pos, rect.lowRight(), borderX, borderY );
	}

	/// Set the bounds and borders from a size. (implied 0,0 position)
	/** Set an area to place things between an uppper left corner of (0,0) and the
	  * lowright corner. <br>
	  * Set the inter-object gap for x and y. <br>
	  * Use this for those cases in which you want to discard the current position
	  * and start over again at 0,0.  An example is using WidgetChildWindow. Putting
	  * widgets inside WidgetChildWindow requires positioning at 0,0 since the
	  * coordinates are now relative to the WindowChildWindow.
	  */
	void setBoundsBorders( const SmartWin::Point & sizefromZenith,
		const int borderX = 0, const int borderY = 0 )
	{
		SmartWin::Point upleft;
		upleft.x = upleft.y = 0;
		setBoundsBorders( upleft, sizefromZenith, borderX, borderY );
	}

	/// Set an area to place things between upleft and lowright corners.
	/** Given two coordinates, prepare the bounds for subsequent placement. upleft
	  * does not have to be 0,0, but it usually is. Set the inter-object gap for x
	  * and y
	  */
	void setBoundsBorders( const SmartWin::Point & upleft,
		const SmartWin::Point & lowright, const int borderX = 0,
		const int borderY = 0 )
	{
		// Record the bounding parameters
		itsUpLeft = upleft;
		itsLowRight = lowright;
		itsBorder.x = borderX;
		itsBorder.y = borderY;

		// Initialize the current position
		itsPos = itsUpLeft;
		itsPos.x += itsBorder.x;
		itsPos.y += itsBorder.y;
		itsMaxYInLine = 0;
	}

	/// Sets obj.pos to the current position, and updates the position by obj.size.
	/** Sets the obj.pos to the current position, and calculates the next position
	  * based upon obj.size. <br>
	  * The next object will be placed to the right of the current object, or in the
	  * next row if needed.
	  */
	void positionToRight( struct Rectangle & obj )
	{
		newRowIfNeeded( obj );

		obj.pos = itsPos; // Return current position

		itsPos.x += obj.size.x + itsBorder.x; // Update next position.
		if ( itsMaxYInLine < obj.size.y )
		{
			itsMaxYInLine = obj.size.y; // Update max y
		}
	}

	/// Move to next row of objects.
	/** Allows the caller to decide when to move to the next row. or used internally
	  * when a object would go past the X bound.
	  */
	void newRow()
	{
		itsPos.x = itsUpLeft.x + itsBorder.x; // CR
		itsPos.y += itsMaxYInLine + itsBorder.y; // Linefeed
		itsMaxYInLine = 0;
	}

	/// Move to a 'tab' column expressed as a fraction of the width of the
	/// bounding rectangle.
	/** Allows the caller to control how Widgets are placed in the bounding
	  * rectangle. For example, if the bounding rectangle was ( pos(100, 0),
	  * size(100, 300) ), <br>
	  * and the curent position was (120, 200), then tabTo(0.5) <br>
	  * would result in the current position becoming (150, 200)
	  */
	void tabTo( double xfract )
	{
		itsPos.x = itsUpLeft.x + ( long )( xfract * ( itsLowRight.x - itsUpLeft.x ) );
	}

	/// Sets obj.pos to the current position, and updates the position by obj.size.
	/** Sets the obj.pos to the current position, and calculates the next position
	  * based upon obj.size. <br>
	  * The next object will be placed under the previous object, or in the next
	  * column if needed.
	  */
	void positionBelow( struct Rectangle & obj )
	{
		if ( obj.size.y <= itsLowRight.y )
		{
			newColIfNeeded( obj );
		}

		obj.pos = itsPos; // Return current position

		itsPos.y += obj.size.y + itsBorder.y; // Update next position.
		if ( itsMaxXInCol < obj.size.x )
		{
			itsMaxXInCol = obj.size.x; // Update max x
		}
	}

	/// Move to next column of objects.
	/** Allows the caller to decide when to move to the next column. or used
	  * internally when a object would go past the Y bound.
	  */
	void newCol()
	{
		itsPos.y = itsUpLeft.y + itsBorder.y; // Start at top
		itsPos.x += itsMaxXInCol + itsBorder.x; // Move to right
		itsMaxXInCol = 0; // Start of new column
	}

	/// Set obj.pos and obj.size to fill the rest of the bounds.
	/** Set obj.pos and obj.size to fill the rest of the bounds. The object need to
	  * be positioned at the current position, and sized to take all the remaining
	  * space in the area.
	  */
	void sizeForRemainingSpace( struct Rectangle & obj )
	{
		newRowIfNeeded( obj );

		obj.pos = itsPos; // Return current position
		obj.size.x = itsLowRight.x - ( itsPos.x + itsBorder.x );
		obj.size.y = itsLowRight.y - ( itsPos.y + itsBorder.y );
	}

	/// Considering the whole area to be divided into rows and columns,
	/// calculate the size of one cell.
	/** Calculate the size of one cell of the bounds. <br>
	  * It considers the borders between the cells. <br>
	  * OUTPUT: size
	  */
	void sizeOfCell( int rows, int cols, Point & size )
	{
		size.x = sizeOfDim( itsLowRight.x - itsUpLeft.x, cols, itsBorder.x );
		size.y = sizeOfDim( itsLowRight.y - itsUpLeft.y, rows, itsBorder.y );
	}

private:
	// Calculate the size of one dimension divided up into divs with borders
	// around them.
	int sizeOfDim( int bound, int divs, int border )
	{
		int lost_in_border = ( divs + 1 ) * border;
		return ( bound - lost_in_border ) / divs;
	}

	void newRowIfNeeded( struct Rectangle & obj )
	{
		// If the obj's size is larger than the area's size, then skip the new row.
		if ( obj.size.x > ( itsLowRight.x - itsUpLeft.x ) ) return;

		// If the object would extend past the area, then do a new row.
		if ( ( itsPos.x + obj.size.x + itsBorder.x ) > itsLowRight.x )
		{
			newRow();
		}
	}

	void newColIfNeeded( struct Rectangle & obj )
	{
		if ( ( itsPos.y + obj.size.y + itsBorder.y ) > itsLowRight.y )
		{
			newCol();
		}
	}

	// Member variables.
	Point itsPos; // x,y of the next available position.
	Point itsBorder; // x,y of the interwidget border.
	int itsMaxYInLine; // Largest Y of the current line of objects.
	int itsMaxXInCol; // Largest X of the current column of objects.

	// Original bounds of the area.
	Point itsUpLeft;
	Point itsLowRight;
};

// end namespace SmartWin
}

#endif

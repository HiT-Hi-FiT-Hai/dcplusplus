
/* Copyright (c) 2006 - Carmi Grushko (venndigram@users.sf.net)
 * Implements behaviour similar to Delphi's Control.Anchor
 * Permission to use under the BSD license
 */

#ifndef ANCHORS_H
#define ANCHORS_H

#include "Widget.h"
#include <stdexcept>

using std::vector;

namespace SmartWin
{

/// AnchoredItem defined the possible parameters for the addAnchored function.
/** Each value describes where the widget will be anchored.  "top" for example
 *  indicated that the widget will always remain the same distance from its parent's top.
*/
struct AnchoredItem {
	enum {top=1, bottom=2, left=4, right=8};		// Attach to that side.
	enum { box = (top | bottom | left | right) };	// Attach to all sides; ie adjust to shape of parent.
	enum { dialog = (bottom | right) };				// Adjust from lower right corner
	enum { row = (left | right) };					// Span across the parent in a row.
	enum { col = (top | bottom) };					// Span as a column on the parent.

	//Widget* widget;
	HWND wnd;
	HWND parent;
	int anchors;
	::RECT edgeDistances;	
};

/// Anchor fixed sized widgets to the sides of their parent window.
/** \ingroup WidgetLayout
* A helper class to do some auto-resizings of widgets in SmartWin++. 
*  
* Take for example the common dialog Open File. 
* Whenever resized, the "Open" and "Cancel" buttons remain in the same location relative
* to the lower-right corner of the dialog.  On the other hand, the control that lists all
* the files in the current directory, grows or shrinks according to the size of the dialog. 
*  
* That's what Anchors does. After the widgets were created, you add them to a special list,
* and in the event handler of OnSize (of your WidgetWindow), you call a special method that
* resizes all the widgets you've added before.  
*  
* Supposed to be simple. 
* Credit Carmi Grushko (venndigram) - 2006-10-02 20:38  
*
*/
class AnchorManager {
public:

/// Adds a widget to the vector of "anchored" widgets, according to the anchors bitmap.
/** The addAnchored() function measures and stores the distances from the sides of
  * the widget's parent. It also stores the anchors bitmap which specifies which sides
  * of the parent should be used in resizing the widget later on. <br>
  * IN: widget:  Any widget that has already been sized and placed in its parent's window. <br>
  *     anchors:  a combination of AnchoredItem::left, AnchoredItem::right, AnchoredItem::top, AnchoredItem::bottom <br>
  * Specifing "AnchoredItem::right" will ensure that the widget will always be the
  * same distance from its parent's right edge.  Specifing "AnchoredItem::row" will cause the widget to go
  * from the left to the right side of the parent, keeping its original size.
  * See the AnchoredItem structure above.
  */
void addAnchored( SmartWin::Widget* widget, int anchors )
{
	if( widget == NULL || widget->handle() == 0 || 
		widget->getParent() == NULL || widget->getParent()->handle() == 0 )
		throw std::runtime_error( "AnchorManager: Invalid widget" );
		
	AnchoredItem item;
	item.wnd = widget->handle();
	item.parent = widget->getParent()->handle();
	item.anchors = anchors;
	
	// Calculating edgeDistances
	::RECT r1, r2;
	::POINT p1, p2;
	GetWindowRect( item.wnd, &r1 );
	GetClientRect( item.parent, &r2 );
	p1.x = r2.left;
	p1.y = r2.top;
	p2.x = r2.right;
	p2.y = r2.bottom;
	ClientToScreen( item.parent, &p1 );
	ClientToScreen( item.parent, &p2 );
	r2.left = p1.x;
	r2.top = p1.y;
	r2.right = p2.x;
	r2.bottom = p2.y;
	
	item.edgeDistances.top = r1.top - r2.top;
	item.edgeDistances.left = r1.left - r2.left;
	item.edgeDistances.bottom = r2.bottom - r1.bottom;
	item.edgeDistances.right = r2.right - r1.right;
	
	anchored.push_back( item );
}

/// Resizes each Widget according to the new size of its parent window.
/** The addAnchored() function measures and stores the distances from the sides of
  * the widget's parent. It also stores the anchors bitmap which specifies which sides
  * of the parent should be used in resizing the widget later on. <br>
  * IN: anchors:  a combination of AnchoredItem::left, AnchoredItem::right, AnchoredItem::top, AnchoredItem::bottom
  * See the AnchoredItem structure above for AnchoredItem::box and AnchoredItem::dialog
  */
void resizeAnchored()
{
	for( vector<AnchoredItem>::const_iterator i = anchored.begin();
		 i != anchored.end(); ++i )
	{
		resizeAnchoredItem( *i );
	}
}



private:
	vector<AnchoredItem> anchored;

	
void resizeAnchoredItem( const AnchoredItem& item )
{
	::RECT r1, r2, newR;

	int flags = SWP_NOZORDER;
	if( ( (item.anchors & (AnchoredItem::left | AnchoredItem::right) ) == 0) && 
		( (item.anchors & (AnchoredItem::top | AnchoredItem::bottom) ) == 0) )
		flags |= SWP_NOSIZE;
	if( (item.anchors & AnchoredItem::left) && (item.anchors & AnchoredItem::top) )
		flags |= SWP_NOMOVE;
		
	if( (flags & SWP_NOSIZE) && (flags & SWP_NOMOVE) )
		return; // Nothing to do
		
	GetClientRect( item.parent, &r1 );
	GetWindowRect( item.wnd, &r2 );
	r2.right -= r2.left;
	r2.bottom -= r2.top;	
	
	if( item.anchors & AnchoredItem::left )
	{
		if( item.anchors & AnchoredItem::right )
		{
			newR.left = item.edgeDistances.left;
			newR.right = r1.right - item.edgeDistances.right - item.edgeDistances.left;
		}
		else
		{
			newR.left = item.edgeDistances.left;
			newR.right = r2.right;
		}
	}
	else
	{
		if( item.anchors & AnchoredItem::right )
		{
			newR.left = r1.right - item.edgeDistances.right - r2.right;
			newR.right = r2.right;
		}
		else
		{
		
		}
	}

	if( item.anchors & AnchoredItem::top )
	{
		if( item.anchors & AnchoredItem::bottom )
		{
			newR.top = item.edgeDistances.top;
			newR.bottom = r1.bottom - item.edgeDistances.bottom - item.edgeDistances.top;
		}
		else
		{
			newR.top = item.edgeDistances.top;
			newR.bottom = r2.bottom;
		}
	}
	else
	{
		if( item.anchors & AnchoredItem::bottom )
		{
			newR.top = r1.bottom - item.edgeDistances.bottom - r2.bottom;
			newR.bottom = r2.bottom;
		}
		else
		{
		
		}
	}

	
	SetWindowPos( item.wnd, 0, 
		newR.left, newR.top, newR.right, newR.bottom,
		flags );
}


};

/*
* An example project (working with the Sally IDE & with a mingw32 makefile) is available online.
*  
* (The anchors-sample.zip contains everything needed to compile, including the anchors source, and a compiled exe.
*  
* http://t2.technion.ac.il/~scg/anchors-sample.zip 
* http://t2.technion.ac.il/~scg/anchors-source.zip 
* By: Carmi Grushko (venndigram) - 2006-10-02 20:38  
*/

}


#endif
 //region SallyIDE Text Editor Settings
//Caret=(0,0)
//endregion


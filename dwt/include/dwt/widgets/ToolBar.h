/*
  DC++ Widget Toolkit

  Copyright (c) 2007-2008, Jacek Sieka

  SmartWin++

  Copyright (c) 2005 Thomas Hansen

  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, 
  are permitted provided that the following conditions are met:

      * Redistributions of source code must retain the above copyright notice, 
        this list of conditions and the following disclaimer.
      * Redistributions in binary form must reproduce the above copyright notice, 
        this list of conditions and the following disclaimer in the documentation 
        and/or other materials provided with the distribution.
      * Neither the name of the DWT nor SmartWin++ nor the names of its contributors 
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

#ifndef DWT_ToolBar_h
#define DWT_ToolBar_h

#ifndef WINCE // Doesn't exist in Windows CE based systems

#include "../Dispatchers.h"
#include "../aspects/AspectFocus.h"
#include "../aspects/AspectFont.h"
#include "../resources/ImageList.h"
#include "Control.h"

#include <vector>

namespace dwt {

// TODO: Give support for multiple bitmaps...
/// Toolbar Control class
/** \ingroup WidgetControls
  * \WidgetUsageInfo
  * \image html toolbar.PNG
  * A toolbar is a strip of buttons normally associated with menu commands, like for 
  * instance Internet Explorer has ( unless you have made them invisible ) a toolbar 
  * of buttons, one for going "home", one to stop rendering of the current page, one 
  * to view the log of URL's you have been to etc...   
  */
class ToolBar :
	public CommonControl,
	// Aspects
	public AspectFocus< ToolBar >,
	public AspectFont< ToolBar >
{
	typedef CommonControl BaseType;
	typedef Dispatchers::VoidVoid<> Dispatcher;
	friend class WidgetCreator< ToolBar >;
public:
	/// Class type
	typedef ToolBar ThisType;

	/// Object type
	typedef ThisType* ObjectType;

	/// Seed class
	/** This class contains all of the values needed to create the widget. It also
	  * knows the type of the class whose seed values it contains. Every widget
	  * should define one of these.       
	  */
	struct Seed : public BaseType::Seed {
		typedef ThisType WidgetType;

		/// Fills with default parameters
		Seed();
	};

	// TODO: Outfactor into Aspect, also StatusBar...
	/// Refreshes the toolbar, must be called after main window has been resized
	/** Refreshes the toolbar, call this one whenever you need to redraw the toolbar,
	  * typical example is when you have resized the main window.
	  */
	void refresh();

	/// Sets the size of buttons in the toolbar
	/** This should be relative to the size of the bitmap used (if you use a bitmap)
	  */
	void setButtonSize( unsigned int width, unsigned int height );

	/// Adds a bitmap to the toolbar that later can be referenced while adding buttons
	/** Loads a bitmap that is contained in a BitmapPtr. <br>
	  * noButtonsInBitmap is how many buttons there actually exists in the bitmap
	  */
		//void addBitmap( BitmapPtr bitmap, unsigned int noButtonsInBitmap );

	/// Adds a separator to the toolbar
	/** A separator is an "empty space" that adds air between buttons
	  */
	void appendSeparator();

	void appendItem(int image, const tstring& toolTip, const Dispatcher::F& f = Dispatcher::F());
	
	/// Set the image list with the normal button images.
	/** normalImageList is the image list that contains the images
	  * for the toolbar buttons in "normal" state.
	  */
	void setNormalImageList( ImageListPtr normalImageList );

	/// Set the image list with the hot button images.
	/** hotImageList is the image list that contains the images for the toolbar
	  * buttons in "hot" state (being hovered / pressed). <br>
	  * Note, hot button images requires the TBSTYLE_FLAT, TBSTYLE_LIST or
	  * TBSTYLE_TRANSPARENT style upon Toolbar creation.
	  */
	void setHotImageList( ImageListPtr hotImageList );

	/// Set the image list with the normal button images.
	/** disabledImageList is the image list that contains the images for the
	  * toolbar buttons in "disabled" state.
	  */
	void setDisabledImageList( ImageListPtr disabledImageList );
	/// Shows (or hides) the button in the toolbar with the given id
	/** id is the identification of which button you want to show.
	  */
	void setButtonVisible( unsigned int id, bool show );

	/// Returns a boolean indicating if the button with the current id is visible or not
	/** id is the identification you supplied when you called addButton.
	  */
	bool getButtonVisible( unsigned id );

	/// Enables (or disables) the button in the toolbar with the given id
	/** id is the identification of which button you want to enable.
	  */
	void setButtonEnabled( unsigned id, bool enable );

	/// Returns a boolean indicating if the button with the current id is enabled or not
	/** id is the identification you supplied when you called addButton.
	  */
	bool getButtonEnabled( unsigned int id );

	/// Returns a boolean indicating if the button with the current id is checked or not
	/** id is the identification you supplied when you called addButton.
	  */
	bool getButtonChecked( unsigned int id );

	int size();
	
	/// Actually creates the Toolbar
	/** You should call WidgetFactory::createToolbar if you instantiate class
	  * directly. <br>
	  * Only if you DERIVE from class you should call this function directly.       
	  */
	void create( const Seed & cs = Seed() );

protected:
	// Constructor Taking pointer to parent
	explicit ToolBar( Widget * parent );

	// To assure nobody accidentally deletes any heaped object of this type, parent
	// is supposed to do so when parent is killed...
	virtual ~ToolBar()
	{}

	virtual bool tryFire( const MSG & msg, LRESULT & retVal );
private:
	// Keep references
	ImageListPtr itsNormalImageList;
	ImageListPtr itsHotImageList;
	ImageListPtr itsDisabledImageList;

	std::vector<Dispatcher::F> commands;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline void ToolBar::refresh()
{
	this->sendMessage(TB_AUTOSIZE);
}

inline void ToolBar::setButtonSize( unsigned int width, unsigned int height )
{
	if ( this->sendMessage(TB_SETBUTTONSIZE, 0, static_cast< LPARAM >( MAKELONG( width, height ) ) ) != TRUE ||
		this->sendMessage(TB_SETBITMAPSIZE, 0, static_cast< LPARAM >( MAKELONG( width, height ) ) ) != TRUE )
	{
		xCeption x( _T( "Error while trying to set toolbar button size..." ) );
		throw x;
	}
}

/*

void ToolBar::addBitmap( HBITMAP hBit, unsigned int noButtonsInBitmap )
{
	TBADDBITMAP tb;
	tb.hInst = NULL;
	tb.nID = ( UINT_PTR )hBit;
	if(this->sendMessage(TB_ADDBITMAP, static_cast< WPARAM >( noButtonsInBitmap ), reinterpret_cast< LPARAM >(&tb ) ) == - 1 )
	{
		xCeption x( _T("Error while trying to add a bitmap to toolbar...") );
		throw x;
	}
}


	void ToolBar::addBitmap( BitmapPtr bitmap, unsigned int noButtonsInBitmap )
{
	itsBitmaps.push_back( bitmap );
	this->addBitmap( bitmap->getBitmap(), noButtonsInBitmap );
}
*/

inline void ToolBar::setNormalImageList( ImageListPtr normalImageList )
{
	itsNormalImageList = normalImageList;
	this->sendMessage(TB_SETIMAGELIST, 0, reinterpret_cast< LPARAM >( itsNormalImageList->getImageList() ) );
}

inline void ToolBar::setHotImageList( ImageListPtr hotImageList )
{
	itsHotImageList = hotImageList;
	this->sendMessage(TB_SETHOTIMAGELIST, 0, reinterpret_cast< LPARAM >( itsHotImageList->getImageList() ) );
}

inline void ToolBar::setDisabledImageList( ImageListPtr disabledImageList )
{
	itsDisabledImageList = disabledImageList;
	this->sendMessage(TB_SETDISABLEDIMAGELIST, 0, reinterpret_cast< LPARAM >( itsDisabledImageList->getImageList() ) );
}

inline void ToolBar::setButtonVisible( unsigned int id, bool show )
{
	this->sendMessage(TB_HIDEBUTTON, static_cast< LPARAM >( id ), MAKELONG( ( show ? FALSE : TRUE ), 0 ) );
}

inline int ToolBar::size( )
{
	return this->sendMessage(TB_BUTTONCOUNT);
}

inline bool ToolBar::getButtonVisible( unsigned int id )
{
	TBBUTTONINFO tb = { sizeof( TBBUTTONINFO ) };
	tb.dwMask = TBIF_STATE;
	tb.idCommand = id;
	this->sendMessage(TB_GETBUTTONINFO, id, reinterpret_cast< LPARAM >( & tb ) );
	return ( tb.fsState & TBSTATE_HIDDEN ) == 0;
}

inline void ToolBar::setButtonEnabled( unsigned id, bool enable )
{
	this->sendMessage(TB_ENABLEBUTTON, static_cast< LPARAM >( id ), MAKELONG( ( enable ? TRUE : FALSE ), 0 ) );
}

inline bool ToolBar::getButtonEnabled( unsigned int id )
{
	TBBUTTONINFO tb =
	{0
	};
	tb.cbSize = sizeof( TBBUTTONINFO );
	tb.dwMask = TBIF_STATE;
	tb.idCommand = id;
	this->sendMessage(TB_GETBUTTONINFO, id, reinterpret_cast< LPARAM >( & tb ) );
	return ( tb.fsState & TBSTATE_ENABLED ) == TBSTATE_ENABLED;
}

inline bool ToolBar::getButtonChecked( unsigned int id )
{
	TBBUTTONINFO tb =
	{0
	};
	tb.cbSize = sizeof( TBBUTTONINFO );
	tb.dwMask = TBIF_STATE;
	tb.idCommand = id;
	this->sendMessage(TB_GETBUTTONINFO, id, reinterpret_cast< LPARAM >( & tb ) );
	return ( tb.fsState & TBSTATE_CHECKED ) == TBSTATE_CHECKED;
}

inline ToolBar::ToolBar( dwt::Widget * parent )
	: BaseType( parent )
{
}

// end namespace dwt
}
#endif
#endif

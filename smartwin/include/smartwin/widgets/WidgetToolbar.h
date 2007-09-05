// $Revision: 1.32 $
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
#ifndef WidgetToolbar_h
#define WidgetToolbar_h

#ifndef WINCE // Doesn't exist in Windows CE based systems

#include "../BasicTypes.h"
#include "../Dispatchers.h"
#include "../MessageMapPolicyClasses.h"
#include "../aspects/AspectEnabled.h"
#include "../aspects/AspectFocus.h"
#include "../aspects/AspectFont.h"
#include "../aspects/AspectRaw.h"
#include "../aspects/AspectSizable.h"
#include "../aspects/AspectVisible.h"
#include "../resources/ImageList.h"
#include "../xCeption.h"

namespace SmartWin
{
// begin namespace SmartWin

// Forward declaring friends
template< class WidgetType >
class WidgetCreator;

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
class WidgetToolbar :
	public MessageMapPolicy< Policies::Subclassed >,

	// Aspects
	public AspectEnabled< WidgetToolbar >,
	public AspectFocus< WidgetToolbar >,
	public AspectFont< WidgetToolbar >,
	public AspectRaw< WidgetToolbar >,
	private AspectSizable< WidgetToolbar >,
	public AspectVisible< WidgetToolbar >
{
	typedef Dispatchers::VoidVoid<> Dispatcher;
	typedef SmartWin::AspectSizable< WidgetToolbar > AspectSizable;
	friend class WidgetCreator< WidgetToolbar >;
	friend class SmartWin::AspectSizable<WidgetToolbar>;
public:
	// Including the stuff we need from AspectSizable to make it accessible.
	// Note here that since we DON'T want the setBounds functions we must inherit
	// privately from AspectSizable and include the stuff we WAN'T to expose from
	// AspectSizable in a public block of the class.
	using AspectSizable::getBounds;
	using AspectSizable::getSize;
	using AspectSizable::getPosition;
	using AspectSizable::getClientAreaSize;
	using AspectSizable::getTextSize;
	using AspectSizable::bringToFront;
	using AspectSizable::onSized;
	using AspectSizable::onMoved;

	/// Class type
	typedef WidgetToolbar ThisType;

	/// Object type
	typedef ThisType * ObjectType;

	typedef MessageMapPolicy<Policies::Subclassed> PolicyType;

	/// Seed class
	/** This class contains all of the values needed to create the widget. It also
	  * knows the type of the class whose seed values it contains. Every widget
	  * should define one of these.       
	  */
	class Seed
		: public SmartWin::Seed
	{
	public:
		typedef WidgetToolbar::ThisType WidgetType;

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

	// TODO: Outfactor into Aspect, also WidgetStatusBar...
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

	void appendItem(unsigned int id, int image, const SmartUtil::tstring& toolTip, const Dispatcher::F& f = Dispatcher::F());
	
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
	virtual void create( const Seed & cs = getDefaultSeed() );

protected:
	// Constructor Taking pointer to parent
	explicit WidgetToolbar( SmartWin::Widget * parent );

	// To assure nobody accidentally deletes any heaped object of this type, parent
	// is supposed to do so when parent is killed...
	virtual ~WidgetToolbar()
	{}

	virtual bool tryFire( const MSG & msg, LRESULT & retVal );
private:
	std::map< unsigned int, SmartUtil::tstring > itsToolTips;

	// Keep references
	ImageListPtr itsNormalImageList;
	ImageListPtr itsHotImageList;
	ImageListPtr itsDisabledImageList;

		//void addBitmap( HBITMAP hBit, unsigned int noButtonsInBitmap );
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline WidgetToolbar::Seed::Seed()
{
	* this = WidgetToolbar::getDefaultSeed();
}

inline void WidgetToolbar::refresh()
{
	this->sendMessage(TB_AUTOSIZE);
}

inline void WidgetToolbar::setButtonSize( unsigned int width, unsigned int height )
{
	if ( this->sendMessage(TB_SETBUTTONSIZE, 0, static_cast< LPARAM >( MAKELONG( width, height ) ) ) != TRUE ||
		this->sendMessage(TB_SETBITMAPSIZE, 0, static_cast< LPARAM >( MAKELONG( width, height ) ) ) != TRUE )
	{
		xCeption x( _T( "Error while trying to set toolbar button size..." ) );
		throw x;
	}
}

/*

void WidgetToolbar::addBitmap( HBITMAP hBit, unsigned int noButtonsInBitmap )
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


	void WidgetToolbar::addBitmap( BitmapPtr bitmap, unsigned int noButtonsInBitmap )
{
	itsBitmaps.push_back( bitmap );
	this->addBitmap( bitmap->getBitmap(), noButtonsInBitmap );
}
*/

inline void WidgetToolbar::setNormalImageList( ImageListPtr normalImageList )
{
	itsNormalImageList = normalImageList;
	this->sendMessage(TB_SETIMAGELIST, 0, reinterpret_cast< LPARAM >( itsNormalImageList->getImageList() ) );
}

inline void WidgetToolbar::setHotImageList( ImageListPtr hotImageList )
{
	itsHotImageList = hotImageList;
	this->sendMessage(TB_SETHOTIMAGELIST, 0, reinterpret_cast< LPARAM >( itsHotImageList->getImageList() ) );
}

inline void WidgetToolbar::setDisabledImageList( ImageListPtr disabledImageList )
{
	itsDisabledImageList = disabledImageList;
	this->sendMessage(TB_SETDISABLEDIMAGELIST, 0, reinterpret_cast< LPARAM >( itsDisabledImageList->getImageList() ) );
}

inline void WidgetToolbar::setButtonVisible( unsigned int id, bool show )
{
	this->sendMessage(TB_HIDEBUTTON, static_cast< LPARAM >( id ), MAKELONG( ( show ? FALSE : TRUE ), 0 ) );
}

inline int WidgetToolbar::size( )
{
	return this->sendMessage(TB_BUTTONCOUNT);
}

inline bool WidgetToolbar::getButtonVisible( unsigned int id )
{
	TBBUTTONINFO tb = { sizeof( TBBUTTONINFO ) };
	tb.dwMask = TBIF_STATE;
	tb.idCommand = id;
	this->sendMessage(TB_GETBUTTONINFO, id, reinterpret_cast< LPARAM >( & tb ) );
	return ( tb.fsState & TBSTATE_HIDDEN ) == 0;
}

inline void WidgetToolbar::setButtonEnabled( unsigned id, bool enable )
{
	this->sendMessage(TB_ENABLEBUTTON, static_cast< LPARAM >( id ), MAKELONG( ( enable ? TRUE : FALSE ), 0 ) );
}

inline bool WidgetToolbar::getButtonEnabled( unsigned int id )
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

inline bool WidgetToolbar::getButtonChecked( unsigned int id )
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

inline WidgetToolbar::WidgetToolbar( SmartWin::Widget * parent )
	: PolicyType( parent )
{
	// Can't have a text box without a parent...
	xAssert( parent, _T( "Can't have a Button without a parent..." ) );
}

// end namespace SmartWin
}
#endif
#endif

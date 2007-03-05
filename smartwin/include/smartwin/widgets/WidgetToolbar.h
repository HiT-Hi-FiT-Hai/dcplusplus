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

#include "boost.h"
#include "SmartUtil.h"
#include "../Widget.h"
#include "../MessageMapControl.h"
#include "../TrueWindow.h"
#include "../aspects/AspectFont.h"
#include "../aspects/AspectVisible.h"
#include "../aspects/AspectEnabled.h"
#include "../aspects/AspectFocus.h"
#include "../aspects/AspectGetParent.h"
#include "../aspects/AspectRaw.h"
#include "../xCeption.h"

namespace SmartWin
{
// begin namespace SmartWin

// Forward declaring friends
template< class WidgetType >
class WidgetCreator;

template< class EventHandlerClass, class WidgetType, class MessageMapType >
class ToolbarDispatcher
{
public:
	static HRESULT dispatch( private_::SignalContent & params )
	{
		typename MessageMapType::voidFunctionTakingUInt func =
			reinterpret_cast< typename MessageMapType::voidFunctionTakingUInt >( params.Function );

		func
			( internal_::getTypedParentOrThrow < EventHandlerClass * >( params.This )
			, boost::polymorphic_cast< WidgetType * >( params.This )
			, params.Msg.WParam
			);

		return 0;
	}

	static HRESULT dispatchThis( private_::SignalContent & params )
	{
		typename MessageMapType::itsVoidFunctionTakingUInt func =
			reinterpret_cast< typename MessageMapType::itsVoidFunctionTakingUInt >( params.FunctionThis );

		( ( * internal_::getTypedParentOrThrow < EventHandlerClass * >( params.This ) ).*func )
			( boost::polymorphic_cast< WidgetType * >( params.This )
			, params.Msg.WParam
			);

		return 0;
	}
};

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
template< class EventHandlerClass, class MessageMapPolicy >
class WidgetToolbar :
	public MessageMapControl< EventHandlerClass, WidgetToolbar< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy >,
	private virtual TrueWindow,

	// Aspects
	public AspectEnabled< EventHandlerClass, WidgetToolbar< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetToolbar< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectFocus< EventHandlerClass, WidgetToolbar< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetToolbar< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectFont< WidgetToolbar< EventHandlerClass, MessageMapPolicy > >,
	public AspectRaw< EventHandlerClass, WidgetToolbar< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetToolbar< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	private AspectSizable< EventHandlerClass, WidgetToolbar< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetToolbar< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectVisible< EventHandlerClass, WidgetToolbar< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetToolbar< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >
{
	typedef MessageMapControl< EventHandlerClass, WidgetToolbar, MessageMapPolicy > MessageMapType;
	typedef MessageMapControl< EventHandlerClass, WidgetToolbar, MessageMapPolicy > ThisMessageMap;
	typedef ToolbarDispatcher< EventHandlerClass, WidgetToolbar, MessageMapType > DispatcherToolbar;
	typedef AspectSizable< EventHandlerClass, WidgetToolbar< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetToolbar< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > > AspectSizable;
	friend class WidgetCreator< WidgetToolbar >;
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
	typedef WidgetToolbar< EventHandlerClass, MessageMapPolicy > ThisType;

	/// Object type
	typedef WidgetToolbar< EventHandlerClass, MessageMapPolicy > * ObjectType;

	/// Seed class
	/** This class contains all of the values needed to create the widget. It also
	  * knows the type of the class whose seed values it contains. Every widget
	  * should define one of these.       
	  */
	class Seed
		: public SmartWin::Seed
	{
	public:
		typedef typename WidgetToolbar::ThisType WidgetType;

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
	void addSeparator();

	/// \ingroup EventHandlersWidgetToolbar
	/// Adds a button to the Toolbar
	/** eventHandler is the event handler function that will be called when the
	  * button is clicked id is an identification number that will be passed into the
	  * event handler when somebody clicks your button. <br>
	  * The reason to why we have this "id" is because the same event handler can be 
	  * defined for several buttons even in fact across toolbar objects, therefore 
	  * this number should be unique across the application. <br>
	  * text is the text that will appear on your button. <br>
	  * toolTip is the tooltip that will be associated with your button ( when 
	  * someone hoovers the mouse over your button ) <br>
	  * Parameters passed expected by event handler is unsigned int which is the id 
	  * of the toolbar button.       
	  */
	void addButton( unsigned int id, const SmartUtil::tstring & text, const SmartUtil::tstring & toolTip, typename ThisMessageMap::itsVoidFunctionTakingUInt eventHandler );
	void addButton( unsigned int id, const SmartUtil::tstring & text, const SmartUtil::tstring & toolTip, typename ThisMessageMap::voidFunctionTakingUInt eventHandler );

	/// \ingroup EventHandlersWidgetToolbar
	/// Adds a button to the toolbar
	/** eventHandler is the event handler function that will be called when the
	  * button is clicked id is an identification number that will be passed into the
	  * event handler when somebody clicks your button. <br>
	  * The reason to why we have this "id" is because the same event handler can be 
	  * defined for several buttons even in fact across toolbar objects, therefore 
	  * this number should be unique across the application. <br>
	  * text is the text that will appear on your button. <br>
	  * Parameters passed expected by event handler is unsigned int which is the id 
	  * of the toolbar button.       
	  */
	void addButton( unsigned int id, const SmartUtil::tstring & text, typename ThisMessageMap::itsVoidFunctionTakingUInt eventHandler );
	void addButton( unsigned int id, const SmartUtil::tstring & text, typename ThisMessageMap::voidFunctionTakingUInt eventHandler );

	/// \ingroup EventHandlersWidgetToolbar
	/// Adds a button to the toolbar
	/** eventHandler is the event handler function that will be called when the
	  * button is clicked id is an identification number that will be passed into the
	  * event handler when somebody clicks your button. <br>
	  * The reason to why we have this "id" is because the same event handler can be 
	  * defined for several buttons even in fact across toolbar objects, therefore 
	  * this number should be unique across the application. <br>
	  * toolTip is the tooltip that will be associated with your button ( when 
	  * someone hoovers the mouse over your button ) <br>
	  * iconIndex is the ( zero indexed ) index of icon on the previously associated
	  * image list. <br>
	  * Parameters passed expected by event handler is unsigned int which is the id 
	  * of the toolbar button. <br>
	  * You must call setNormalImageList BEFORE setting event handlers with iconIndex's
	  */
		void addButton( unsigned int id, int iconIndex, const SmartUtil::tstring & toolTip, typename ThisMessageMap::itsVoidFunctionTakingUInt eventHandler );
		void addButton( unsigned int id, int iconIndex, const SmartUtil::tstring & toolTip, typename ThisMessageMap::voidFunctionTakingUInt eventHandler );

	/// \ingroup EventHandlersWidgetToolbar
	/// Adds a button to the toolbar
	/** eventHandler is the event handler function that will be called when the
	  * button is clicked id is an identification number that will be passed into the
	  * event handler when somebody clicks your button. <br>
	  * The reason to why we have this "id" is because the same event handler can be
	  * defined for several buttons even in fact across toolbar objects, therefore
	  * this number should be unique across the application. <br>
	  * iconIndex is the ( zero indexed ) index of icon on the previously associated
	  * image list. <br>
	  * Parameters passed expected by event handler is unsigned int which is the id
	  * of the toolbar button. <br>
	  * You must call setNormalImageList BEFORE setting event handlers with
	  * iconIndex's
	  */
		void addButton( unsigned int id, int iconIndex, typename ThisMessageMap::itsVoidFunctionTakingUInt eventHandler );
		void addButton( unsigned int id, int iconIndex, typename ThisMessageMap::voidFunctionTakingUInt eventHandler );

	/// \ingroup EventHandlersWidgetToolbar
	/// Adds a button to the toolbar
	/** eventHandler is the event handler function that will be called when the
	  * button is clicked id is an identification number that will be passed into the
	  * event handler when somebody clicks your button. <br>
	  * The reason to why we have this "id" is because the same event handler can be
	  * defined for several buttons even in fact across toolbar objects, therefore
	  * this number should be unique across the application. <br>
	  * iconIndex is the ( zero indexed ) index of icon on the previously associated
	  * image list. <br>
	  * text is the text that will appear on your button. <br>
	  * toolTip is the tooltip that will be associated with your button ( when
	  * someone hoovers the mouse over your button ) <br>
	  * Parameters passed expected by event handler is unsigned int which is the id
	  * of the toolbar button. <br>
	  * You must call setNormalImageList BEFORE setting event handlers with
	  * iconIndex's
	  */
		void addButton( unsigned int id, int iconIndex, const SmartUtil::tstring & text, const SmartUtil::tstring & toolTip, bool checkButton, typename ThisMessageMap::itsVoidFunctionTakingUInt eventHandler );
		void addButton( unsigned int id, int iconIndex, const SmartUtil::tstring & text, const SmartUtil::tstring & toolTip, bool checkButton, typename ThisMessageMap::voidFunctionTakingUInt eventHandler );

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

	// We MUST override this one, however little I want to since it's NOT a MENU
	// item and neither holds any notification and therefore have got the "menu
	// item code" in the LOWORD of the WParam (xxx Microsoft and their counter
	// intuitive interfaces...)
	virtual bool tryFire( const Message & msg, HRESULT & retVal )
	{
		MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );

		// First we must create a "comparable" message...
		for ( typename MessageMapType::SignalCollection::iterator idx = this->getSignals().begin();
			idx != this->getSignals().end();
			++idx )
		{
			if ( idx->template get< 0 >().Msg.Msg == msg.Msg && idx->template get< 0 >().Msg.WParam == msg.WParam )
			{
				private_::SignalContent params( msg, idx->template get< 0 >().Function, idx->template get< 0 >().FunctionThis, idx->template get< 0 >().This, true );
				retVal = idx->template get< 1 >().fire( params );
				return true;
			}
		}
		return false;
	}

private:
	std::map< unsigned int, SmartUtil::tstring > itsToolTips;
	std::vector< BitmapPtr > itsBitmaps;

		ImageListPtr itsNormalImageList;
		ImageListPtr itsHotImageList;
		ImageListPtr itsDisabledImageList;

		//void addBitmap( HBITMAP hBit, unsigned int noButtonsInBitmap );
	virtual LRESULT sendWidgetMessage( HWND hWnd, UINT msg, WPARAM & wPar, LPARAM & lPar );
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template< class EventHandlerClass, class MessageMapPolicy >
const typename WidgetToolbar< EventHandlerClass, MessageMapPolicy >::Seed & WidgetToolbar< EventHandlerClass, MessageMapPolicy >::getDefaultSeed()
{
	static bool d_NeedsInit = true;
	static Seed d_DefaultValues( DontInitializeMe );

	if ( d_NeedsInit )
	{
		Application::instance().setSystemClassName( d_DefaultValues, TOOLBARCLASSNAME );
#ifndef WINCE
		d_DefaultValues.exStyle = TBSTYLE_EX_MIXEDBUTTONS;
#else
		d_DefaultValues.exStyle = 0;
#endif
		d_DefaultValues.style = WS_CHILD | WS_VISIBLE | TBSTYLE_TOOLTIPS | TBSTYLE_SEP | CCS_ADJUSTABLE | TBSTYLE_ALTDRAG;
		d_NeedsInit = false;
	}
	return d_DefaultValues;
}

template< class EventHandlerClass, class MessageMapPolicy >
WidgetToolbar< EventHandlerClass, MessageMapPolicy >::Seed::Seed()
{
	* this = WidgetToolbar::getDefaultSeed();
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetToolbar< EventHandlerClass, MessageMapPolicy >::refresh()
{
	 SmartWin::Rectangle rect;
	if ( ::MoveWindow( this->handle(),
		rect.pos.x, rect.pos.y, rect.size.x, rect.size.y, TRUE ) == 0 )
	{
		xCeption err( _T( "Couldn't reposition windows" ) );
		throw err;
	}
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetToolbar< EventHandlerClass, MessageMapPolicy >::setButtonSize( unsigned int width, unsigned int height )
{
	if ( ::SendMessage( this->Widget::itsHandle, TB_SETBUTTONSIZE, 0, static_cast< LPARAM >( MAKELONG( width, height ) ) ) != TRUE ||
		::SendMessage( this->Widget::itsHandle, TB_SETBITMAPSIZE, 0, static_cast< LPARAM >( MAKELONG( width, height ) ) ) != TRUE )
	{
		xCeption x( _T( "Error while trying to set toolbar button size..." ) );
		throw x;
	}
}

/*
template< class EventHandlerClass, class MessageMapPolicy >
void WidgetToolbar< EventHandlerClass, MessageMapPolicy >::addBitmap( HBITMAP hBit, unsigned int noButtonsInBitmap )
{
	TBADDBITMAP tb;
	tb.hInst = NULL;
	tb.nID = ( UINT_PTR )hBit;
	if( ::SendMessage( this->Widget::itsHandle, TB_ADDBITMAP, static_cast< WPARAM >( noButtonsInBitmap ), reinterpret_cast< LPARAM >(&tb ) ) == - 1 )
	{
		xCeption x( _T("Error while trying to add a bitmap to toolbar...") );
		throw x;
	}
}

template< class EventHandlerClass, class MessageMapPolicy >
	void WidgetToolbar< EventHandlerClass, MessageMapPolicy >::addBitmap( BitmapPtr bitmap, unsigned int noButtonsInBitmap )
{
	itsBitmaps.push_back( bitmap );
	this->addBitmap( bitmap->getBitmap(), noButtonsInBitmap );
}
*/

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetToolbar< EventHandlerClass, MessageMapPolicy >::addSeparator()
{
	TBBUTTON tb[1];
	tb[0].iBitmap = - 2;
	tb[0].idCommand = 0;
	tb[0].fsState = TBSTATE_ENABLED;
	tb[0].fsStyle = BTNS_SEP;
	tb[0].iString = 0;
	if ( ::SendMessage( this->Widget::itsHandle, TB_ADDBUTTONS, 1, reinterpret_cast< LPARAM >( tb ) ) == FALSE )
	{
		xCeption x( _T( "Error while trying to add a button to toolbar..." ) );
		throw x;
	}
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetToolbar< EventHandlerClass, MessageMapPolicy >::addButton
	( unsigned int id, const SmartUtil::tstring & text, const SmartUtil::tstring & toolTip
	, typename ThisMessageMap::itsVoidFunctionTakingUInt eventHandler
	)
{
	addButton( id, - 2, text, toolTip, false, eventHandler );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetToolbar< EventHandlerClass, MessageMapPolicy >::addButton
	( unsigned int id, const SmartUtil::tstring & text, const SmartUtil::tstring & toolTip
	, typename ThisMessageMap::voidFunctionTakingUInt eventHandler
	)
{
	addButton( id, - 2, text, toolTip, false, eventHandler );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetToolbar< EventHandlerClass, MessageMapPolicy >::addButton
	( unsigned int id, const SmartUtil::tstring & text
	, typename ThisMessageMap::itsVoidFunctionTakingUInt eventHandler
	)
{
	addButton( id, - 2, text, _T( "" ), false, eventHandler );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetToolbar< EventHandlerClass, MessageMapPolicy >::addButton
	( unsigned int id, const SmartUtil::tstring & text
	, typename ThisMessageMap::voidFunctionTakingUInt eventHandler
	)
{
	addButton( id, - 2, text, _T( "" ), false, eventHandler );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetToolbar< EventHandlerClass, MessageMapPolicy >::addButton
	( unsigned int id, int bitmapIdx, const SmartUtil::tstring & toolTip
	, typename ThisMessageMap::itsVoidFunctionTakingUInt eventHandler
	)
{
	addButton( id, bitmapIdx, _T( "" ), toolTip, false, eventHandler );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetToolbar< EventHandlerClass, MessageMapPolicy >::addButton
	( unsigned int id, int bitmapIdx, const SmartUtil::tstring & toolTip
	, typename ThisMessageMap::voidFunctionTakingUInt eventHandler
	)
{
	addButton( id, bitmapIdx, _T( "" ), toolTip, false, eventHandler );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetToolbar< EventHandlerClass, MessageMapPolicy >::addButton
	( unsigned int id, int bitmapIdx
	, typename ThisMessageMap::itsVoidFunctionTakingUInt eventHandler
	)
{
	addButton( id, bitmapIdx, _T( "" ), _T( "" ), false, eventHandler );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetToolbar< EventHandlerClass, MessageMapPolicy >::addButton
	( unsigned int id, int bitmapIdx
	, typename ThisMessageMap::voidFunctionTakingUInt eventHandler
	)
{
	addButton( id, bitmapIdx, _T( "" ), _T( "" ), false, eventHandler );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetToolbar< EventHandlerClass, MessageMapPolicy >::addButton
	( unsigned int id, int bitmapIdx, const SmartUtil::tstring & text, const SmartUtil::tstring & toolTip
	, bool checkButton, typename ThisMessageMap::itsVoidFunctionTakingUInt eventHandler
	)
{
	// Checking if tooltip id exists from before
	if ( itsToolTips.find( id ) != itsToolTips.end() )
	{
		xCeption x( _T( "Tried to add a button with an ID that already exists..." ) );
		throw x;
	}

	// Adding tooltip UNLESS tooltip is empty
	if ( toolTip != _T( "" ) )
		itsToolTips[id] = toolTip;

	// Adding bitmap
	TBBUTTON tb[1];
	tb[0].iBitmap = bitmapIdx;
	tb[0].idCommand = id;
	tb[0].fsState = TBSTATE_ENABLED;
	tb[0].fsStyle = BTNS_AUTOSIZE;
	if ( checkButton )
		tb[0].fsStyle |= BTNS_CHECK;
	tb[0].iString = text == _T( "" ) ? 0 : reinterpret_cast< INT_PTR >( text.c_str() );
	if ( ::SendMessage( this->Widget::itsHandle, TB_ADDBUTTONS, 1, reinterpret_cast< LPARAM >( tb ) ) == FALSE )
	{
		xCeption x( _T( "Error while trying to add a button to toolbar..." ) );
		throw x;
	}

	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->addNewSignal
		( typename MessageMapType::SignalTupleType
			( private_::SignalContent
				( Message( WM_COMMAND, id )
				, reinterpret_cast< itsVoidFunction >( eventHandler )
				, ptrThis
				)
			, typename MessageMapType::SignalType
				( typename MessageMapType::SignalType::SlotType( & DispatcherToolbar::dispatchThis )
				)
			)
		);
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetToolbar< EventHandlerClass, MessageMapPolicy >::addButton
	( unsigned id, int bitmapIdx, const SmartUtil::tstring & text, const SmartUtil::tstring & toolTip
	, bool checkButton, typename ThisMessageMap::voidFunctionTakingUInt eventHandler
	)
{
	// Checking if tooltip id exists from before
	if ( itsToolTips.find( id ) != itsToolTips.end() )
	{
		xCeption x( _T( "Tried to add a button with an ID that already exists..." ) );
		throw x;
	}

	// Adding tooltip UNLESS tooltip is empty
	if ( toolTip != _T( "" ) )
		itsToolTips[id] = toolTip;

	// Adding bitmap
	TBBUTTON tb[1];
	tb[0].iBitmap = bitmapIdx;
	tb[0].idCommand = id;
	tb[0].fsState = TBSTATE_ENABLED;
	tb[0].fsStyle = BTNS_AUTOSIZE;
	if ( checkButton )
		tb[0].fsStyle |= BTNS_CHECK;
	tb[0].iString = text == _T( "" ) ? 0 : reinterpret_cast< INT_PTR >( text.c_str() );
	if ( ::SendMessage( this->Widget::itsHandle, TB_ADDBUTTONS, 1, ( LPARAM ) tb ) == FALSE )
	{
		xCeption x( _T( "Error while trying to add a button to toolbar..." ) );
		throw x;
	}

	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->addNewSignal
		( typename MessageMapType::SignalTupleType
			( private_::SignalContent
				( Message( WM_COMMAND, id )
				, reinterpret_cast< private_::SignalContent::voidFunctionTakingVoid >( eventHandler )
				, ptrThis
				)
			, typename MessageMapType::SignalType
				( typename MessageMapType::SignalType::SlotType( & DispatcherToolbar::dispatch )
				)
			)
		);
}

template< class EventHandlerClass, class MessageMapPolicy >
	void WidgetToolbar< EventHandlerClass, MessageMapPolicy >::setNormalImageList( ImageListPtr normalImageList )
	{
		itsNormalImageList = normalImageList;
		::SendMessage( this->Widget::itsHandle, TB_SETIMAGELIST, 0, reinterpret_cast< LPARAM >( itsNormalImageList->getImageList() ) );
	}

	template< class EventHandlerClass, class MessageMapPolicy >
	void WidgetToolbar< EventHandlerClass, MessageMapPolicy >::setHotImageList( ImageListPtr hotImageList )
	{
		itsHotImageList = hotImageList;
		::SendMessage( this->Widget::itsHandle, TB_SETHOTIMAGELIST, 0, reinterpret_cast< LPARAM >( itsHotImageList->getImageList() ) );
	}

	template< class EventHandlerClass, class MessageMapPolicy >
	void WidgetToolbar< EventHandlerClass, MessageMapPolicy >::setDisabledImageList( ImageListPtr disabledImageList )
	{
		itsDisabledImageList = disabledImageList;
		::SendMessage( this->Widget::itsHandle, TB_SETDISABLEDIMAGELIST, 0, reinterpret_cast< LPARAM >( itsDisabledImageList->getImageList() ) );
	}

	template< class EventHandlerClass, class MessageMapPolicy >
void WidgetToolbar< EventHandlerClass, MessageMapPolicy >::setButtonVisible( unsigned int id, bool show )
{
	::SendMessage( this->Widget::itsHandle, TB_HIDEBUTTON, static_cast< LPARAM >( id ), MAKELONG( ( show ? FALSE : TRUE ), 0 ) );
}

template< class EventHandlerClass, class MessageMapPolicy >
bool WidgetToolbar< EventHandlerClass, MessageMapPolicy >::getButtonVisible( unsigned int id )
{
	TBBUTTONINFO tb =
	{0
	};
	tb.cbSize = sizeof( TBBUTTONINFO );
	tb.dwMask = TBIF_STATE;
	tb.idCommand = id;
	::SendMessage( this->Widget::itsHandle, TB_GETBUTTONINFO, id, reinterpret_cast< LPARAM >( & tb ) );
	return ( tb.fsState & TBSTATE_HIDDEN ) == 0;
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetToolbar< EventHandlerClass, MessageMapPolicy >::setButtonEnabled( unsigned id, bool enable )
{
	::SendMessage( this->Widget::itsHandle, TB_ENABLEBUTTON, static_cast< LPARAM >( id ), MAKELONG( ( enable ? TRUE : FALSE ), 0 ) );
}

template< class EventHandlerClass, class MessageMapPolicy >
bool WidgetToolbar< EventHandlerClass, MessageMapPolicy >::getButtonEnabled( unsigned int id )
{
	TBBUTTONINFO tb =
	{0
	};
	tb.cbSize = sizeof( TBBUTTONINFO );
	tb.dwMask = TBIF_STATE;
	tb.idCommand = id;
	::SendMessage( this->Widget::itsHandle, TB_GETBUTTONINFO, id, reinterpret_cast< LPARAM >( & tb ) );
	return ( tb.fsState & TBSTATE_ENABLED ) == TBSTATE_ENABLED;
}

template< class EventHandlerClass, class MessageMapPolicy >
bool WidgetToolbar< EventHandlerClass, MessageMapPolicy >::getButtonChecked( unsigned int id )
{
	TBBUTTONINFO tb =
	{0
	};
	tb.cbSize = sizeof( TBBUTTONINFO );
	tb.dwMask = TBIF_STATE;
	tb.idCommand = id;
	::SendMessage( this->Widget::itsHandle, TB_GETBUTTONINFO, id, reinterpret_cast< LPARAM >( & tb ) );
	return ( tb.fsState & TBSTATE_CHECKED ) == TBSTATE_CHECKED;
}

template< class EventHandlerClass, class MessageMapPolicy >
WidgetToolbar< EventHandlerClass, MessageMapPolicy >::WidgetToolbar( SmartWin::Widget * parent )
	: Widget( parent, 0 )
{
	// Can't have a text box without a parent...
	xAssert( parent, _T( "Can't have a Button without a parent..." ) );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetToolbar< EventHandlerClass, MessageMapPolicy >::create( const Seed & cs )
{
	if ( cs.style & WS_CHILD )
		Widget::create( cs );
	else
	{
		typename WidgetToolbar::Seed d_YouMakeMeDoNastyStuff = cs;

		d_YouMakeMeDoNastyStuff.style |= WS_CHILD;
		Widget::create( d_YouMakeMeDoNastyStuff );
	}
	ThisMessageMap::createMessageMap();

	//// Telling the toolbar what the size of TBBUTTON struct is
	::SendMessage( this->Widget::itsHandle, TB_BUTTONSTRUCTSIZE, ( WPARAM ) sizeof( TBBUTTON ), 0 );
	::SendMessage( this->Widget::itsHandle, TB_SETIMAGELIST, 0, 0 );
	////TODO: use CreationalInfo parameters
}

template< class EventHandlerClass, class MessageMapPolicy >
LRESULT WidgetToolbar< EventHandlerClass, MessageMapPolicy >::sendWidgetMessage( HWND hWnd, UINT msg, WPARAM & wPar, LPARAM & lPar )
{
	// First the stuff we HAVE to do something about...
	switch ( msg )
	{
		case WM_NOTIFY :
		{
			switch ( ( reinterpret_cast< LPNMHDR >( lPar ) )->code )
			{
				// TODO: Outfactor into Tooltip Aspect...
				case TTN_GETDISPINFO :
				{
					LPTOOLTIPTEXT lpttt = reinterpret_cast< LPTOOLTIPTEXT >( lPar );
					lpttt->lpszText = const_cast < TCHAR * >( itsToolTips[lpttt->hdr.idFrom].c_str() );
				} break;
				default:
					return ThisMessageMap::sendWidgetMessage( hWnd, msg, wPar, lPar );
			}
		} break;
		default:
			return ThisMessageMap::sendWidgetMessage( hWnd, msg, wPar, lPar );
	}
	// Removing compiler hickup...
	return 0;
}

// end namespace SmartWin
}

#endif
#endif

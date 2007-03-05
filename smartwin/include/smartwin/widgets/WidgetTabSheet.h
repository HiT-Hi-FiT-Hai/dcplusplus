// $Revision: 1.31 $
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
#ifndef WidgetTabSheet_h
#define WidgetTabSheet_h

#include <commctrl.h>
#include "../MessageMapControl.h"
#include "../TrueWindow.h"
#include "../aspects/AspectSizable.h"
#include "../aspects/AspectSelection.h"
#include "../aspects/AspectFont.h"
#include "../aspects/AspectVisible.h"
#include "../aspects/AspectEnabled.h"
#include "../aspects/AspectFocus.h"
#include "../aspects/AspectGetParent.h"
#include "../aspects/AspectRaw.h"
#include "../aspects/AspectThreads.h"
#include "../aspects/AspectMouseClicks.h"
#include "../aspects/AspectPainting.h"
#include "../aspects/AspectBorder.h"
#include "../xCeption.h"
#include "SmartUtil.h"

namespace SmartWin
{
// begin namespace SmartWin

// Forward declaring friends
template< class WidgetType >
class WidgetCreator;

template< class EventHandlerClass, class WidgetType, class MessageMapType >
class WidgetTabSheetDispatcher
{
public:
	static HRESULT dispatch( private_::SignalContent & params )
	{
		MessageMapType * This = boost::polymorphic_cast< MessageMapType * >( params.This );
		bool retVal;
		unsigned param = TabCtrl_GetCurSel( This->handle() );
		typename MessageMapType::boolFunctionTakingUnsigned func =
			reinterpret_cast< typename MessageMapType::boolFunctionTakingUnsigned >( params.Function );

		retVal = func
			( internal_::getTypedParentOrThrow < EventHandlerClass * >( params.This )
			, boost::polymorphic_cast< WidgetType * >( params.This )
			, param
			);

		return retVal ? FALSE : TRUE;
	}

	static HRESULT dispatchThis( private_::SignalContent & params )
	{
		MessageMapType * This = boost::polymorphic_cast< MessageMapType * >( params.This );
		bool retVal;
		unsigned param = TabCtrl_GetCurSel( This->handle() );
		typename MessageMapType::itsBoolFunctionTakingUnsigned func =
			reinterpret_cast< typename MessageMapType::itsBoolFunctionTakingUnsigned >( params.FunctionThis );

		retVal = ( ( * internal_::getTypedParentOrThrow < EventHandlerClass * >( params.This ) ).*func )
			( boost::polymorphic_cast< WidgetType * >( params.This )
			, param
			);

		return retVal ? FALSE : TRUE;
	}
};

/// Tab Sheet Control class
/** \ingroup WidgetControls
  * \WidgetUsageInfo
  * \image html tabsheet.png
  * Class for creating a Tab Control Widget. <br>
  * A Tab Control is a control consisting of "tab buttons" normally on the top of the
  * Widget where the user can select different pages to switch between to group
  * related information within. E.g. Visual Studio has got tab controls on the top of
  * the code area where the user can switch between the different opened files. Use
  * the onSelectionChanged event to make visible/invisible the different controls you
  * wish to use in the different tab pages! <br>
  * Normally you would add up one WidgetChildWindow for each Tab Page the Tab Control
  * has.
  */
template< class EventHandlerClass, class MessageMapPolicy >
class WidgetTabSheet :
	public MessageMapControl< EventHandlerClass, WidgetTabSheet< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy >,
	private virtual TrueWindow,

	// Aspects
	public AspectBorder< WidgetTabSheet< EventHandlerClass, MessageMapPolicy > >,
	public AspectEnabled< EventHandlerClass, WidgetTabSheet< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetTabSheet< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectFocus< EventHandlerClass, WidgetTabSheet< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetTabSheet< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectFont< WidgetTabSheet< EventHandlerClass, MessageMapPolicy > >,
	public AspectMouseClicks< EventHandlerClass, WidgetTabSheet< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetTabSheet< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectPainting< EventHandlerClass, WidgetTabSheet< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetTabSheet< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectRaw< EventHandlerClass, WidgetTabSheet< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetTabSheet< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectSelection< EventHandlerClass, WidgetTabSheet< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetTabSheet< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectSizable< EventHandlerClass, WidgetTabSheet< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetTabSheet< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectText< EventHandlerClass, WidgetTabSheet< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetTabSheet< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectThreads< EventHandlerClass, WidgetTabSheet< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetTabSheet< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectVisible< EventHandlerClass, WidgetTabSheet< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetTabSheet< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >
{
	typedef MessageMapControl< EventHandlerClass, WidgetTabSheet, MessageMapPolicy > MessageMapType;
	typedef WidgetTabSheetDispatcher< EventHandlerClass, WidgetTabSheet, MessageMapType > Dispatcher;
	friend class WidgetCreator< WidgetTabSheet >;

public:
	/// Class type
	typedef WidgetTabSheet< EventHandlerClass, MessageMapPolicy > ThisType;

	/// Object type
	typedef WidgetTabSheet< EventHandlerClass, MessageMapPolicy > * ObjectType;

	/// Seed class
	/** This class contains all of the values needed to create the widget. It also
	  * knows the type of the class whose seed values it contains. Every widget
	  * should define one of these.       
	  */
	class Seed
		: public SmartWin::Seed
	{
	public:
		typedef typename WidgetTabSheet::ThisType WidgetType;

		FontPtr font;

		/// Fills with default parameters
		// explicit to avoid conversion through SmartWin::CreationalStruct
		explicit Seed();

		/// Doesn't fill any values
		Seed( DontInitialize )
		{}
	};

	/// Default values for creation
	static const Seed & getDefaultSeed();

	typedef MessageMapControl< EventHandlerClass, WidgetTabSheet, MessageMapPolicy > ThisMessageMap;

	// Removing compiler hickup...
	virtual LRESULT sendWidgetMessage( HWND hWnd, UINT msg, WPARAM & wPar, LPARAM & lPar );

	// AspectSelection expectation implementation
	static Message & getSelectionChangedMessage();

	// Commented in AspectSelection
	int getSelectedIndex() const;

	/// Returns the text of the currently selected tab
	/** Use this function to retrieve the header text of the currently selected tab.
	  */
	SmartUtil::tstring getSelectedHeader() const;

	/// Setting the event handler for the "selection changing" event
	/** The event handler must have the signature "bool foo( WidgetTabSheet * Widget,
	  * unsigned indexNo )" whereby if you return true the user will be allowed to
	  * actually CHANGE the page but if you return false the page will not be allowed
	  * to change and the onSelectionChanged event will not fire ( good for
	  * validation of fields etc...)
	  */
	void onSelectionChanging( typename ThisMessageMap::itsBoolFunctionTakingUnsigned eventHandler );
	void onSelectionChanging( typename ThisMessageMap::boolFunctionTakingUnsigned eventHandler );

	// Commented in AspectSelection
	void setSelectedIndex( int idx );

	/// Appends a "page" to the Tab Sheet
	/** The return value is the index of the new item appended. The input index is
	  * where you wish to put the new page
	  */
	// the negative values are already covered by throwing an exception
	unsigned int addPage( const SmartUtil::tstring & header, unsigned index );

	/// Actually creates the Tab Sheet Control
	/** You should call WidgetFactory::createTabSheet if you instantiate class
	  * directly. <br>
	  * Only if you DERIVE from class you should call this function directly.       
	  */
	virtual void create( const Seed & cs = getDefaultSeed() );

	/// Set tab buttons at bottom of control
	/** If passed true to this function tabs will appear at the bottom of the control
	  */
	void setTabsAtBottom( bool value = true );

	/// Set tabs to "button" style
	/** If passed true to this function tabs will appear as buttons instead of
	  * default as pages
	  */
	void setButtonStyle( bool value = true );

	/// Set tabs to "flat button" style
	/** If passed true to this function tabs will appear as flat buttons instead of
	  * default as pages
	  */
	void setFlatButtonStyle( bool value = true );

	/// Turns hot tracking of tabs on or off
	/** If passed true hottracking of items will be turned on
	  */
	void setHotTrack( bool value = true );

	/// Set tabs to "multiline" style
	/** If passed true to this function tabs will be able to span across multiple
	  * lines
	  */
	void setMultiline( bool value = true );

	/// Set tabs to "ragged right" style
	/** If passed true to this function tabs be ragged to the right to make pages
	  * "span" across whole area if multiple lines are inserted
	  */
	void setRaggedRight( bool value = true );

	/// Set tabs to appear vertically instead of horizontally which is the default style
	/** If passed true to this function tabs will appear vertically instead of
	  * horizontally
	  */
	void setVerticalTabs( bool value = true );

	/// Set tabs to appear vertically to the right
	/** This one also turns on vertical style
	  */
	void setRightTabs( bool value = true );

	/// Set tabs to appear with a flat separator between different tabs
	/** If true passed flat separator style will be turned ON else OFF
	  */
	void setFlatSeparators( bool value = true );

	static bool isValidSelectionChanged( LPARAM lPar )
	{ return true;
	}

/// Get the area not used by the tabs
/** This function should be used after adding the pages, so that the area not used by
  * the tabs can be calculated accurately. It returns coordinates respect to the
  * TabControl, this is, you have to adjust for the position of the control itself.   
  */
SmartWin::Rectangle getUsableArea() const;
protected:
	// Constructor Taking pointer to parent
	explicit WidgetTabSheet( SmartWin::Widget * parent );

	// Protected to avoid direct instantiation, you can inherit and use
	// WidgetFactory class which is friend
	virtual ~WidgetTabSheet()
	{}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template< class EventHandlerClass, class MessageMapPolicy >
const typename WidgetTabSheet< EventHandlerClass, MessageMapPolicy >::Seed & WidgetTabSheet< EventHandlerClass, MessageMapPolicy >::getDefaultSeed()
{
	static bool d_NeedsInit = true;
	static Seed d_DefaultValues( DontInitializeMe );

	if ( d_NeedsInit )
	{
		Application::instance().setSystemClassName( d_DefaultValues, WC_TABCONTROL );
		d_DefaultValues.style = WS_CHILD | WS_VISIBLE;
		d_DefaultValues.font = createFont( DefaultGuiFont );
		d_NeedsInit = false;
	}
	return d_DefaultValues;
}

template< class EventHandlerClass, class MessageMapPolicy >
WidgetTabSheet< EventHandlerClass, MessageMapPolicy >::Seed::Seed()
{
	* this = WidgetTabSheet::getDefaultSeed();
}

template< class EventHandlerClass, class MessageMapPolicy >
LRESULT WidgetTabSheet< EventHandlerClass, MessageMapPolicy >::sendWidgetMessage( HWND hWnd, UINT msg, WPARAM & wPar, LPARAM & lPar )
{
	return ThisMessageMap::sendWidgetMessage( hWnd, msg, wPar, lPar );
}

template< class EventHandlerClass, class MessageMapPolicy >
Message & WidgetTabSheet< EventHandlerClass, MessageMapPolicy >::getSelectionChangedMessage()
{
	static Message retVal = Message( WM_NOTIFY, TCN_SELCHANGE );
	return retVal;
}

template< class EventHandlerClass, class MessageMapPolicy >
int WidgetTabSheet< EventHandlerClass, MessageMapPolicy >::getSelectedIndex() const
{
	int retVal = TabCtrl_GetCurSel( this->Widget::itsHandle );
	return retVal;
}

template< class EventHandlerClass, class MessageMapPolicy >
SmartUtil::tstring WidgetTabSheet< EventHandlerClass, MessageMapPolicy >::getSelectedHeader() const
{
	TCITEM item;
	item.mask = TCIF_TEXT;
	TCHAR buffer[200];
	item.cchTextMax = 198;
	item.pszText = buffer;
	if ( !TabCtrl_GetItem( this->Widget::itsHandle, getSelectedIndex(), & item ) )
	{
		throw xCeption( _T( "Couldn't retrieve text of currently selected TabSheet item." ) );
	}
	return buffer;
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetTabSheet< EventHandlerClass, MessageMapPolicy >::onSelectionChanging(
	typename MessageMapControl< EventHandlerClass, WidgetTabSheet, MessageMapPolicy >::itsBoolFunctionTakingUnsigned
	eventHandler )
{
	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->addNewSignal
		( typename MessageMapType::SignalTupleType
			( private_::SignalContent
				( Message( WM_NOTIFY, TCN_SELCHANGING )
				, reinterpret_cast< itsVoidFunction >( eventHandler )
				, ptrThis
				)
			, typename MessageMapType::SignalType
				( typename MessageMapType::SignalType::SlotType( & Dispatcher::dispatchThis )
				)
			)
		);
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetTabSheet< EventHandlerClass, MessageMapPolicy >::onSelectionChanging(
	typename MessageMapControl< EventHandlerClass, WidgetTabSheet, MessageMapPolicy >::boolFunctionTakingUnsigned
	eventHandler )
{
	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->addNewSignal
		( typename MessageMapType::SignalTupleType
			( private_::SignalContent
				( Message( WM_NOTIFY, TCN_SELCHANGING )
				, reinterpret_cast< private_::SignalContent::voidFunctionTakingVoid >( eventHandler )
				, ptrThis
				)
			, typename MessageMapType::SignalType
				( typename MessageMapType::SignalType::SlotType( & Dispatcher::dispatch )
				)
			)
		);
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetTabSheet< EventHandlerClass, MessageMapPolicy >::setSelectedIndex( int idx )
{
	TabCtrl_SetCurSel( this->Widget::itsHandle, idx );
}

template< class EventHandlerClass, class MessageMapPolicy >
unsigned int WidgetTabSheet< EventHandlerClass, MessageMapPolicy >::addPage( const SmartUtil::tstring & header, unsigned index )
{
	TCITEM item;
	item.mask = TCIF_TEXT;
	item.pszText = const_cast < TCHAR * >( header.c_str() );
	int newIdx = TabCtrl_InsertItem( this->Widget::itsHandle, index, & item );
	if ( newIdx == - 1 )
	{
		xCeption x( _T( "Error while trying to add page into Tab Sheet" ) );
		throw x;
	}
	return ( unsigned int ) newIdx;
}

template< class EventHandlerClass, class MessageMapPolicy >
WidgetTabSheet< EventHandlerClass, MessageMapPolicy >::WidgetTabSheet( SmartWin::Widget * parent )
	: Widget( parent, 0 )
{
	// Can't have a ComboBox without a parent...
	xAssert( parent, _T( "Cant have a WidgetTabSheet without a parent..." ) );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetTabSheet< EventHandlerClass, MessageMapPolicy >::create( const Seed & cs )
{
	if ( cs.style & WS_CHILD )
		Widget::create( cs );
	else
	{
		typename WidgetTabSheet::Seed d_YouMakeMeDoNastyStuff = cs;

		d_YouMakeMeDoNastyStuff.style |= WS_CHILD;
		Widget::create( d_YouMakeMeDoNastyStuff );
	}
	ThisMessageMap::createMessageMap();
	setFont( cs.font );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetTabSheet< EventHandlerClass, MessageMapPolicy >::setTabsAtBottom( bool value )
{
	this->addRemoveStyle( TCS_BOTTOM, value );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetTabSheet< EventHandlerClass, MessageMapPolicy >::setButtonStyle( bool value )
{
	this->addRemoveStyle( TCS_BUTTONS, value );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetTabSheet< EventHandlerClass, MessageMapPolicy >::setFlatButtonStyle( bool value )
{
	this->addRemoveStyle( TCS_BUTTONS, value );
	this->addRemoveStyle( TCS_FLATBUTTONS, value );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetTabSheet< EventHandlerClass, MessageMapPolicy >::setHotTrack( bool value )
{
	this->addRemoveStyle( TCS_HOTTRACK, value );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetTabSheet< EventHandlerClass, MessageMapPolicy >::setMultiline( bool value )
{
	this->addRemoveStyle( TCS_MULTILINE, value );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetTabSheet< EventHandlerClass, MessageMapPolicy >::setRaggedRight( bool value )
{
	this->addRemoveStyle( TCS_RAGGEDRIGHT, value );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetTabSheet< EventHandlerClass, MessageMapPolicy >::setVerticalTabs( bool value )
{
	this->addRemoveStyle( TCS_VERTICAL, value );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetTabSheet< EventHandlerClass, MessageMapPolicy >::setRightTabs( bool value )
{
	this->addRemoveStyle( TCS_VERTICAL | TCS_RIGHT, value );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetTabSheet< EventHandlerClass, MessageMapPolicy >::setFlatSeparators( bool value )
{
	setFlatButtonStyle();
	sendWidgetMessage( this->Widget::itsHandle, TCM_SETEXTENDEDSTYLE, TCS_EX_FLATSEPARATORS, TCS_EX_FLATSEPARATORS );
}

template< class EventHandlerClass, class MessageMapPolicy >
SmartWin::Rectangle WidgetTabSheet< EventHandlerClass, MessageMapPolicy >::getUsableArea() const
{
	::RECT d_Answer;
	Point d_Size = this->getClientAreaSize();

	d_Answer.left = d_Answer.top = 0;
	d_Answer.right = d_Size.x;
	d_Answer.bottom = d_Size.y;
	TabCtrl_AdjustRect( this->handle(), false, & d_Answer );
	return Rectangle::FromRECT( d_Answer );
}

// end namespace SmartWin
}

#endif

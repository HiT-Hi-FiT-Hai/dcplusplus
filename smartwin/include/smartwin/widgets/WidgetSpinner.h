// $Revision: 1.18 $
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
#ifndef WidgetSpinner_h
#define WidgetSpinner_h

#include "../Widget.h"
#include "../aspects/AspectSizable.h"
#include "../aspects/AspectVisible.h"
#include "../aspects/AspectEnabled.h"
#include "../aspects/AspectScrollable.h"
#include "../aspects/AspectFocus.h"
#include "../aspects/AspectGetParent.h"
#include "../aspects/AspectRaw.h"
#include "../aspects/AspectBorder.h"
#include "../MessageMapControl.h"
#include "../xCeption.h"

namespace SmartWin
{
// begin namespace SmartWin

// Forward declaring friends
template< class WidgetType >
class WidgetCreator;

/** sideeffect=\par Side Effects:
  */
/// Spinner Control class
/** \ingroup WidgetControls
  * \WidgetUsageInfo
  * \image html spinner.PNG
  * Class for creating a Spinner Control Widget. <br>
  * A Spinner is a Widget used to manipulate an integer value. A good example is the
  * volume control of a music  player which has two buttons, one for louder and the
  * other for softer.
  */
template< class EventHandlerClass, class MessageMapPolicy >
class WidgetSpinner :
	public MessageMapControl< EventHandlerClass, WidgetSpinner< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy >,

	// Aspects
	public AspectBorder< WidgetSpinner< EventHandlerClass, MessageMapPolicy > >,
	public AspectEnabled< EventHandlerClass, WidgetSpinner< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetSpinner< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectFocus< EventHandlerClass, WidgetSpinner< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetSpinner< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectKeyPressed< EventHandlerClass, WidgetSpinner< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetSpinner< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectMouseClicks< EventHandlerClass, WidgetSpinner< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetSpinner< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectPainting< EventHandlerClass, WidgetSpinner< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetSpinner< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectRaw< EventHandlerClass, WidgetSpinner< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetSpinner< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectScrollable< EventHandlerClass, WidgetSpinner< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetSpinner< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectSizable< EventHandlerClass, WidgetSpinner< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetSpinner< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectVisible< EventHandlerClass, WidgetSpinner< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetSpinner< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >
{
	typedef MessageMapControl< EventHandlerClass, WidgetSpinner, MessageMapPolicy > ThisMessageMap;
	friend class WidgetCreator< WidgetSpinner >;
public:
	/// Class type
	typedef WidgetSpinner< EventHandlerClass, MessageMapPolicy > ThisType;

	/// Object type
	typedef WidgetSpinner< EventHandlerClass, MessageMapPolicy > * ObjectType;

	/// Seed class
	/** This class contains all of the values needed to create the widget. It also
	  * knows the type of the class whose seed values it contains. Every widget
	  * should define one of these.
	  */
	class Seed
		: public SmartWin::Seed
	{
		// A spinner has no caption. Hide it.
		using SmartWin::Seed::caption;
	public:
		typedef typename WidgetSpinner::ThisType WidgetType;

		//TODO: put variables to be filled here
		int minValue, maxValue;

		/// Fills with default parameters
		// explicit to avoid conversion through SmartWin::CreationalStruct
		explicit Seed();

		/// Doesn't fill any values
		Seed( DontInitialize )
		{}
	};

	/// Default values for creation
	static const Seed & getDefaultSeed();

	// Removing compiler hickup...
	virtual LRESULT sendWidgetMessage( HWND hWnd, UINT msg, WPARAM & wPar, LPARAM & lPar );

	/// Sets the range of the Spinner
	/** The range is the unique values of the control, use this function to set the
	  * range of the control. Maximum values are 65000 and minimum are -65000
	  */
	void setRange( int minimum, int maximum );

	/// Assigns a buddy control
	/** A "buddy control" can theoretically be any type of control, but the most
	  * usually type of control to assign is probably either a WidgetTextBox or a
	  * WidgetStatic. <br>
	  * A buddy control is a control which (can) receive automatically the text value
	  * (setText) automatically when the value of the spinner changes. <br>
	  * And if you change the value of the buddy control the Spinner Control will
	  * automatically also change its value.
	  */
	void assignBuddy( Widget * buddy );

	/// Returns the value of the control
	/** The value can be any value between the minimum and maximum range defined in
	  * the setRange function
	  */
	int getValue();

	/// Sets the value of the control
	/** The value can be any value between the minimum and maximum range defined in
	  * the setRange function
	  */
	int setValue( int v );

	/// Actually creates the Spinner Control
	/** You should call WidgetFactory::createSpinner if you instantiate class
	  * directly. <br>
	  * Only if you DERIVE from class you should call this function directly.
	  */
	virtual void create( const Seed & cs = getDefaultSeed() );

protected:
	// Constructor Taking pointer to parent
	explicit WidgetSpinner( SmartWin::Widget * parent );

	// Protected to avoid direct instantiation, you can inherit and use
	// WidgetFactory class which is friend
	virtual ~WidgetSpinner()
	{}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template< class EventHandlerClass, class MessageMapPolicy >
const typename WidgetSpinner< EventHandlerClass, MessageMapPolicy >::Seed & WidgetSpinner< EventHandlerClass, MessageMapPolicy >::getDefaultSeed()
{
	static bool d_NeedsInit = true;
	static Seed d_DefaultValues( DontInitializeMe );

	if ( d_NeedsInit )
	{
		Application::instance().setSystemClassName( d_DefaultValues, UPDOWN_CLASS );
		d_DefaultValues.minValue = 0;
		d_DefaultValues.maxValue = 100;

		d_DefaultValues.style = WS_CHILD | WS_VISIBLE;
		//TODO: initialize the values here
		d_NeedsInit = false;
	}
	return d_DefaultValues;
}

template< class EventHandlerClass, class MessageMapPolicy >
WidgetSpinner< EventHandlerClass, MessageMapPolicy >::Seed::Seed()
{
	* this = WidgetSpinner::getDefaultSeed();
}

template< class EventHandlerClass, class MessageMapPolicy >
LRESULT WidgetSpinner< EventHandlerClass, MessageMapPolicy >::sendWidgetMessage( HWND hWnd, UINT msg, WPARAM & wPar, LPARAM & lPar )
{
	return ThisMessageMap::sendWidgetMessage( hWnd, msg, wPar, lPar );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetSpinner< EventHandlerClass, MessageMapPolicy >::setRange( int minimum, int maximum )
{
	::SendMessage( this->Widget::itsHandle, UDM_SETRANGE32, static_cast< WPARAM >( minimum ), static_cast< LPARAM >( maximum ) );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetSpinner< EventHandlerClass, MessageMapPolicy >::assignBuddy( Widget * buddy )
{
	assert( buddy && buddy->handle() );
	::SendMessage( this->Widget::itsHandle, UDM_SETBUDDY, reinterpret_cast< WPARAM >( buddy->handle() ), 0 );
}

template< class EventHandlerClass, class MessageMapPolicy >
int WidgetSpinner< EventHandlerClass, MessageMapPolicy >::getValue()
{
#ifdef WINCE
	LRESULT retVal = ::SendMessage( this->Widget::itsHandle, UDM_GETPOS, 0, 0 );
	if ( HIWORD( retVal ) != 0 )
	{
		xCeption err( _T( " Something went wrong while trying to retrieve value if WidgetSpinner" ) );
		throw err;
	}
	return LOWORD( retVal );
#else
	return ::SendMessage( this->Widget::itsHandle, UDM_GETPOS32, 0, 0 );
#endif //! WINCE
}

template< class EventHandlerClass, class MessageMapPolicy >
int WidgetSpinner< EventHandlerClass, MessageMapPolicy >::setValue( int v )
{
#ifdef WINCE
	return ::SendMessage( this->Widget::itsHandle, UDM_SETPOS, 0, v );
#else
	return ::SendMessage( this->Widget::itsHandle, UDM_SETPOS32, 0, v );
#endif
}

template< class EventHandlerClass, class MessageMapPolicy >
WidgetSpinner< EventHandlerClass, MessageMapPolicy >::WidgetSpinner( SmartWin::Widget * parent )
	: Widget( parent, 0 )
{
	// Can't have a text box without a parent...
	xAssert( parent, _T( "Can't have a Spinner without a parent..." ) );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetSpinner< EventHandlerClass, MessageMapPolicy >::create( const Seed & cs )
{
	if ( cs.style & WS_CHILD )
		Widget::create( cs );
	else
	{
		typename WidgetSpinner::Seed d_YouMakeMeDoNastyStuff = cs;

		d_YouMakeMeDoNastyStuff.style |= WS_CHILD;
		Widget::create( d_YouMakeMeDoNastyStuff );
	}
	ThisMessageMap::createMessageMap();
	setRange( cs.minValue, cs.maxValue );
	//TODO: use CreationalInfo parameters
}

// end namespace SmartWin
}

#endif

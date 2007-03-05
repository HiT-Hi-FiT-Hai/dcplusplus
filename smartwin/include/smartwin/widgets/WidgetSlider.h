// $Revision: 1.19 $
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
#ifndef WidgetSlider_h
#define WidgetSlider_h

#include "../Widget.h"
#include "../aspects/AspectSizable.h"
#include "../aspects/AspectVisible.h"
#include "../aspects/AspectEnabled.h"
#include "../aspects/AspectFocus.h"
#include "../aspects/AspectScrollable.h"
#include "../aspects/AspectGetParent.h"
#include "../aspects/AspectRaw.h"
#include "../aspects/AspectKeyPressed.h"
#include "../aspects/AspectMouseClicks.h"
#include "../aspects/AspectPainting.h"
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
/// Slider Control class
/** \ingroup WidgetControls
  * \WidgetUsageInfo
  * \image html slider.PNG
  * Class for creating a Slider control Widget. <br>
  * A Slider is a Widget which can be used to give e.g. percentage input. For
  * instance a volume control of a stereo system often has several of these ones, one
  * each for the volume, bass and tone. It contains of a thumbtrack and an axis which
  * you can move the thumbtrack up and down, it is quite similar in functionality to
  * the WidgetSpinner control, but have another visual appearance.
  */
template< class EventHandlerClass, class MessageMapPolicy >
class WidgetSlider :
	public MessageMapControl< EventHandlerClass, WidgetSlider< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy >,

	// Aspects
	public AspectBorder< WidgetSlider< EventHandlerClass, MessageMapPolicy > >,
	public AspectEnabled< EventHandlerClass, WidgetSlider< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetSlider< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectFocus< EventHandlerClass, WidgetSlider< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetSlider< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectKeyPressed< EventHandlerClass, WidgetSlider< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetSlider< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectMouseClicks< EventHandlerClass, WidgetSlider< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetSlider< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectPainting< EventHandlerClass, WidgetSlider< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetSlider< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectRaw< EventHandlerClass, WidgetSlider< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetSlider< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectScrollable< EventHandlerClass, WidgetSlider< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetSlider< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectSizable< EventHandlerClass, WidgetSlider< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetSlider< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectVisible< EventHandlerClass, WidgetSlider< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetSlider< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >
{
	typedef MessageMapControl< EventHandlerClass, WidgetSlider, MessageMapPolicy > ThisMessageMap;
	friend class WidgetCreator< WidgetSlider >;
public:
	/// Class type
	typedef WidgetSlider< EventHandlerClass, MessageMapPolicy > ThisType;

	/// Object type
	typedef WidgetSlider< EventHandlerClass, MessageMapPolicy > * ObjectType;

	/// Seed class
	/** This class contains all of the values needed to create the widget. It also
	  * knows the type of the class whose seed values it contains. Every widget
	  * should define one of these.
	  */
	class Seed
		: public SmartWin::Seed
	{
	public:
		typedef typename WidgetSlider::ThisType WidgetType;

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

	// Removing compiler hickup...
	virtual LRESULT sendWidgetMessage( HWND hWnd, UINT msg, WPARAM & wPar, LPARAM & lPar );

	/// Sets the Auto Ticks property of the control
	/** Auto ticks means that the control will have a tick note for each increment in
	  * its range of values.
	  */
	void setAutoTicks( bool value = true );

	/// Sets the horizontal property of the control
	/** If you want the slider to be horizontally aligned (default) then call this
	  * function with true, if you call it with false the slider will show up
	  * vertically instead.
	  */
	void setHorizontal( bool value = true );

	/// Sets the placement of the ticks
	/** If true is passed then the ticks of the slider will be displayed to the left
	  * of the slider, else ticks will show up to the right. Only call this function
	  * if you have called the setHorizontal with false.
	  */
	void setShowTicksLeft( bool value = true );

	/// Sets the placement of the ticks
	/** Sets the placement of the ticks to either top (true) or bottom (false). Only
	  * call this function if you have called the setHorizontal with true
	  */
	void setShowTicksTop( bool value = true );

	/// Sets the placement of the ticks to be BOTH (above/bottom or right/left)
	/** If you call this function with true it will show ticks on BOTH sides of the
	  * control depending on if the slider is aligned horizontally or vertically the
	  * ticks will show up either above and below or to the right and the left of the
	  * slider.
	  */
	void setShowTicksBoth( bool value = true );

	/// Removes ticks
	/** If you call this function with true the ticks will be REMOVED from the
	  * control. If you call it with false the ticks will appear again in its former
	  * positions
	  */
	void setShowTicks( bool value = true );

	/// Sets the range of the slider
	/** The range is the unique values of the control, use this function to set the
	  * range of the control. Maximum values are 65000 and minimum are -65000
	  */
	void setRange( short minimum, short maximum );

	/// Retrieves the maximum position of the Slider
	/** The return value from this function is the maximum value of the Slider
	  */
	int getMaxValue();

	/// Retrieves the minimum position of the Slider
	/** The return value from this function is the minimum value of the Slider
	  */
	int getMinValue();

	/// Sets the position of the thumb
	/** This is the "value" of the control, pass in the new position/value and the
	  * control will set the thumb to that position.
	  */
	void setPosition( int newPosition );

	/// Sets tick frequency
	/** Sets the frequency of the ticks, e.g. if five is given every fifth value of
	  * the slider will have a tick, default value is one.
	  */
	void setTickFrequency( unsigned frequency );

	/// Returns the "value" of the Slider
	/** Returns the "value" of the Slider or the position of the thumb. If you for
	  * instance have defined the minimum/maximum value to be -10 and 10 and the
	  * thumb is in the 3/4 of the max position this function will return 5.
	  */
	int getPosition();

	/// Assigns a buddy control
	/** A "buddy control" may be of any type but most often a WidgetTextBox or a
	  * WidgetStatic is used. <br>
	  * It is normally used for displaying "maximum" and "minimum" text labels. <br>
	  * The buddy control will be positioned either to the left or right of a
	  * vertical Slider or the top or bottom of a horizontal Slider depending on the
	  * boolean value of the first argument. If the first argument is true, the buddy
	  * control will be placed to the left of a vertical Slider or above a horizontal
	  * Slider. <br>
	  * The buddy will be repositioned according to the position of the associated
	  * slider, BUT you MUST set the size of it. Don't burn braincells over the
	  * position setting. Only the SIZE matters. <br>
	  * A Slider may have TWO buddy controls, one with first argument set to true and
	  * one set to false. <br>
	  * If you assign another buddy control with the same first argument, the earlier
	  * instance will no longer be a buddy control, but it will still be there, it
	  * will not cease to exist or be destroyed. See the WidgetSlider sample
	  * application for an example of the buddy control in use.
	  */
	void assignBuddy( bool beginning, Widget * buddy );

	/// Actually creates the Slider Control
	/** You should call WidgetFactory::createSlider if you instantiate class
	  * directly. <br>
	  * Only if you DERIVE from class you should call this function directly.
	  */
	virtual void create( const Seed & cs = getDefaultSeed() );

protected:
	// Constructor Taking pointer to parent
	explicit WidgetSlider( SmartWin::Widget * parent );

	// Protected to avoid direct instantiation, you can inherit and use
	// WidgetFactory class which is friend
	virtual ~WidgetSlider()
	{}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template< class EventHandlerClass, class MessageMapPolicy >
const typename WidgetSlider< EventHandlerClass, MessageMapPolicy >::Seed & WidgetSlider< EventHandlerClass, MessageMapPolicy >::getDefaultSeed()
{
	static bool d_NeedsInit = true;
	static Seed d_DefaultValues( DontInitializeMe );

	if ( d_NeedsInit )
	{
		Application::instance().setSystemClassName( d_DefaultValues, TRACKBAR_CLASS );
		d_DefaultValues.style = WS_CHILD | WS_VISIBLE;
		//TODO: initialize the values here
		d_NeedsInit = false;
	}
	return d_DefaultValues;
}

template< class EventHandlerClass, class MessageMapPolicy >
WidgetSlider< EventHandlerClass, MessageMapPolicy >::Seed::Seed()
{
	* this = WidgetSlider::getDefaultSeed();
}

template< class EventHandlerClass, class MessageMapPolicy >
LRESULT WidgetSlider< EventHandlerClass, MessageMapPolicy >::sendWidgetMessage( HWND hWnd, UINT msg, WPARAM & wPar, LPARAM & lPar )
{
	return ThisMessageMap::sendWidgetMessage( hWnd, msg, wPar, lPar );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetSlider< EventHandlerClass, MessageMapPolicy >::setAutoTicks( bool value )
{
	this->Widget::addRemoveStyle( TBS_AUTOTICKS, value );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetSlider< EventHandlerClass, MessageMapPolicy >::setHorizontal( bool value )
{
	this->Widget::addRemoveStyle( TBS_HORZ, value );
	this->Widget::addRemoveStyle( TBS_VERT, !value );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetSlider< EventHandlerClass, MessageMapPolicy >::setShowTicksLeft( bool value )
{
	// TODO: Add assertion that the TBS_VERT is set!
	this->Widget::addRemoveStyle( TBS_LEFT, value );
	this->Widget::addRemoveStyle( TBS_RIGHT, !value );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetSlider< EventHandlerClass, MessageMapPolicy >::setShowTicksTop( bool value )
{
	// TODO: Add assertion that the TBS_HORZ is set!
	this->Widget::addRemoveStyle( TBS_TOP, value );
	this->Widget::addRemoveStyle( TBS_BOTTOM, !value );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetSlider< EventHandlerClass, MessageMapPolicy >::setShowTicksBoth( bool value )
{
	this->Widget::addRemoveStyle( TBS_BOTH, value );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetSlider< EventHandlerClass, MessageMapPolicy >::setShowTicks( bool value )
{
	this->Widget::addRemoveStyle( TBS_NOTICKS, !value );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetSlider< EventHandlerClass, MessageMapPolicy >::setRange( short minimum, short maximum )
{
	::SendMessage( this->Widget::itsHandle, TBM_SETRANGE, static_cast< WPARAM >( TRUE ), MAKELONG( minimum, maximum ) );
}

template< class EventHandlerClass, class MessageMapPolicy >
int WidgetSlider< EventHandlerClass, MessageMapPolicy >::getMaxValue()
{
	return ( int )::SendMessage( this->Widget::itsHandle, TBM_GETRANGEMAX, 0, 0 );
}

template< class EventHandlerClass, class MessageMapPolicy >
int WidgetSlider< EventHandlerClass, MessageMapPolicy >::getMinValue()
{
	return ( int )::SendMessage( this->Widget::itsHandle, TBM_GETRANGEMIN, 0, 0 );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetSlider< EventHandlerClass, MessageMapPolicy >::setPosition( int newPosition )
{
	::SendMessage( this->Widget::itsHandle, TBM_SETPOS, static_cast< WPARAM >( TRUE ), static_cast< LPARAM >( newPosition ) );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetSlider< EventHandlerClass, MessageMapPolicy >::setTickFrequency( unsigned frequency )
{
	::SendMessage( this->Widget::itsHandle, TBM_SETTICFREQ, static_cast< LPARAM >( frequency ), 0 );
}

template< class EventHandlerClass, class MessageMapPolicy >
int WidgetSlider< EventHandlerClass, MessageMapPolicy >::getPosition()
{
	return ::SendMessage( this->Widget::itsHandle, TBM_GETPOS, 0, 0 );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetSlider< EventHandlerClass, MessageMapPolicy >::assignBuddy( bool beginning, Widget * buddy )
{
	assert( buddy && buddy->handle() );
	::SendMessage( this->Widget::itsHandle, TBM_SETBUDDY, static_cast< WPARAM >( beginning ? TRUE : FALSE ),
		reinterpret_cast< LPARAM >( buddy->handle() ) );
}

template< class EventHandlerClass, class MessageMapPolicy >
WidgetSlider< EventHandlerClass, MessageMapPolicy >::WidgetSlider( SmartWin::Widget * parent )
	: Widget( parent, 0 )
{
	// Can't have a text box without a parent...
	xAssert( parent, _T( "Can't have a Slider without a parent..." ) );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetSlider< EventHandlerClass, MessageMapPolicy >::create( const Seed & cs )
{
	if ( cs.style & WS_CHILD )
		Widget::create( cs );
	else
	{
		typename WidgetSlider::Seed d_YouMakeMeDoNastyStuff = cs;

		d_YouMakeMeDoNastyStuff.style |= WS_CHILD;
		Widget::create( d_YouMakeMeDoNastyStuff );
	}
	ThisMessageMap::createMessageMap();
}

// end namespace SmartWin
}

#endif

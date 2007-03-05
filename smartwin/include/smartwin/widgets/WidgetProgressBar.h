// $Revision: 1.19 $
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
#ifndef WidgetProgressBar_h
#define WidgetProgressBar_h

#include "../Widget.h"
#include "../aspects/AspectSizable.h"
#include "../aspects/AspectVisible.h"
#include "../aspects/AspectEnabled.h"
#include "../aspects/AspectFocus.h"
#include "../aspects/AspectScrollable.h"
#include "../aspects/AspectGetParent.h"
#include "../aspects/AspectRaw.h"
#include "../aspects/AspectMouseClicks.h"
#include "../aspects/AspectPainting.h"
#include "../aspects/AspectThreads.h"
#include "../aspects/AspectBorder.h"
#include "../MessageMapControl.h"
#include "../xCeption.h"

namespace SmartWin
{
// begin namespace SmartWin

// Forward declaring friends
template< class WidgetType >
class WidgetCreator;

/** sideeffect= \par Side Effects :
*/
/// ProgressBar Control class
/** \ingroup WidgetControls
  * \WidgetUsageInfo
  * \image html progressbar.png
  * Class for creating a ProgressBar control Widget. <br>
  * A ProgressBar is a Widget which can be used to show e.g. percentage of lengthy 
  * jobs, often used when downloading from internet or installing applications etc.   
  */
template< class EventHandlerClass, class MessageMapPolicy >
class WidgetProgressBar :
	public MessageMapControl< EventHandlerClass, WidgetProgressBar< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy >,

	// Aspects
	public AspectBorder< WidgetProgressBar< EventHandlerClass, MessageMapPolicy > >,
	public AspectMouseClicks< EventHandlerClass, WidgetProgressBar< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetProgressBar< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectPainting< EventHandlerClass, WidgetProgressBar< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetProgressBar< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectRaw< EventHandlerClass, WidgetProgressBar< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetProgressBar< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectSizable< EventHandlerClass, WidgetProgressBar< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetProgressBar< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectThreads< EventHandlerClass, WidgetProgressBar< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetProgressBar< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectVisible< EventHandlerClass, WidgetProgressBar< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetProgressBar< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >
{
	typedef MessageMapControl< EventHandlerClass, WidgetProgressBar, MessageMapPolicy > ThisMessageMap;
	friend class WidgetCreator< WidgetProgressBar >;
public:
	/// Class type
	typedef WidgetProgressBar< EventHandlerClass, MessageMapPolicy > ThisType;

	/// Object type
	typedef WidgetProgressBar< EventHandlerClass, MessageMapPolicy > * ObjectType;

	/// Seed class
	/** This class contains all of the values needed to create the widget. It also
	  * knows the type of the class whose seed values it contains. Every widget
	  * should define one of these.       
	  */
	class Seed
		: public SmartWin::Seed
	{
	public:
		typedef typename WidgetProgressBar::ThisType WidgetType;

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
	//virtual LRESULT sendWidgetMessage( HWND hWnd, UINT msg, WPARAM wPar, LPARAM lPar );

#ifdef COMCTRL_V6
	/// Sets the Vertical property of the control
	/** Progress is shown from left to right or from bottom to top
	  */
	void setHorizontal( bool value = true );
#endif

	/// Sets the range of a progress bar control to a 32 - bit value.
	/** The range is the unique values of the control, use this function to set the
	  * range of the control. The progress bar high limit, low limit, and position
	  * parameters are signed integers. To make full use of the 32 - bit range, set 
	  * the range to -0x7FFFFFFF to 0x7FFFFFFF and treat the position as a signed 
	  * integer.       
	  */
	void setRange( int minimum, int maximum );

	/// Sets the position of the thumb
	/** This is the "value" of the control, pass in the new position/value and the
	  * control will set the thumb to that position.
	  */
	void setPosition( int newPosition );

	/// Sets the Step size
	/** Specifies the step increment for a progress bar. The step increment is the
	  * amount by which the progress bar increases its current position whenever
	  * AddStep is called. By default, the step increment is set to 10.
	  */
	void setStep( unsigned stepsize );

	/// returns the Step size
	unsigned getStep( void );

	/// Returns the "value" of the ProgressBar
	/** Retrieves the current position of the progress bar
	  */
	int getPosition( void );

	/// Adds a Value to the current ProgressBar Position
	/** *Advances the current position of a progress bar by a specified increment and
	  * redraws the bar to reflect the new position.
	  */
	void addToPosition( int positiondelta );

	/// Adds the Value of StepSize to the current Position of the Progressbar
	/** Advances the current position for a progress bar by the step increment and
	  * redraws the bar to reflect the new position. An application sets the step
	  * increment by calling the setStep function.
	  */
	void addStep( void );

	/// Retrieves the maximum value of the ProgressBar.
	/** The maximum value is the value the ProgressBar have if it is "full"
	  */
	int getMaxValue();

	/// Retrieves the minimum value of the ProgressBar.
	/** The minimum value is the value the ProgressBar have if it is "empty"
	  */
	int getMinValue();

	/// Actually creates the Progress Bar Control
	/** You should call WidgetFactory::createProgressBar if you instantiate class
	  * directly. <br>
	  * Only if you DERIVE from class you should call this function directly.
	  */
	virtual void create( const Seed & cs = getDefaultSeed() );

protected:
	/// CTOR Taking pointer to parent
	explicit WidgetProgressBar( SmartWin::Widget * parent );

	// Protected to avoid direct instantiation, you can inherit and use
	// WidgetFactory class which is friend
	virtual ~WidgetProgressBar()
	{}
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template< class EventHandlerClass, class MessageMapPolicy >
const typename WidgetProgressBar< EventHandlerClass, MessageMapPolicy >::Seed & WidgetProgressBar< EventHandlerClass, MessageMapPolicy >::getDefaultSeed()
{
	static bool d_NeedsInit = true;
	static Seed d_DefaultValues( DontInitializeMe );

	if ( d_NeedsInit )
	{
		Application::instance().setSystemClassName( d_DefaultValues, PROGRESS_CLASS );
		d_DefaultValues.style = WS_CHILD | WS_VISIBLE;
		//TODO: initialize the values here
		d_NeedsInit = false;
	}
	return d_DefaultValues;
}

template< class EventHandlerClass, class MessageMapPolicy >
WidgetProgressBar< EventHandlerClass, MessageMapPolicy >::Seed::Seed()
{
	* this = WidgetProgressBar::getDefaultSeed();
}

#ifdef COMCTRL_V6
template< class EventHandlerClass, class MessageMapPolicy >
void WidgetProgressBar< EventHandlerClass, MessageMapPolicy >::setHorizontal( bool value )
{
	this->Widget::addRemoveStyle( PBS_VERTICAL, !value );
}
#endif

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetProgressBar< EventHandlerClass, MessageMapPolicy >::addToPosition( int positiondelta )
{
	::SendMessage( this->Widget::itsHandle, PBM_DELTAPOS, static_cast< WPARAM >( positiondelta ), static_cast< LPARAM >( 0 ) );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetProgressBar< EventHandlerClass, MessageMapPolicy >::addStep( void )
{
	::SendMessage( this->Widget::itsHandle, PBM_STEPIT, static_cast< WPARAM >( 0 ), static_cast< LPARAM >( 0 ) );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetProgressBar< EventHandlerClass, MessageMapPolicy >::setRange( int minimum, int maximum )
{
	::SendMessage( this->Widget::itsHandle, PBM_SETRANGE32, static_cast< WPARAM >( minimum ), static_cast< LPARAM >( maximum ) );
}

template< class EventHandlerClass, class MessageMapPolicy >
int WidgetProgressBar< EventHandlerClass, MessageMapPolicy >::getMaxValue()
{
	return ( int )::SendMessage( this->Widget::itsHandle, PBM_GETRANGE, static_cast< WPARAM >( FALSE ), static_cast< LPARAM >( 0 ) );
}

template< class EventHandlerClass, class MessageMapPolicy >
int WidgetProgressBar< EventHandlerClass, MessageMapPolicy >::getMinValue()
{
	return ( int )::SendMessage( this->Widget::itsHandle, PBM_GETRANGE, static_cast< WPARAM >( TRUE ), static_cast< LPARAM >( 0 ) );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetProgressBar< EventHandlerClass, MessageMapPolicy >::setPosition( int newPosition )
{
	::SendMessage( this->Widget::itsHandle, PBM_SETPOS, static_cast< WPARAM >( newPosition ), static_cast< LPARAM >( 0 ) );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetProgressBar< EventHandlerClass, MessageMapPolicy >::setStep( unsigned stepsize )
{
	::SendMessage( this->Widget::itsHandle, PBM_SETSTEP, static_cast< WPARAM >( stepsize ), 0 );
}

template< class EventHandlerClass, class MessageMapPolicy >
unsigned int WidgetProgressBar< EventHandlerClass, MessageMapPolicy >::getStep( void )
{
	unsigned int stepsize = ::SendMessage( this->Widget::itsHandle, PBM_SETSTEP, static_cast< WPARAM >( 1 ), 0 );
	::SendMessage( this->Widget::itsHandle, PBM_SETSTEP, static_cast< WPARAM >( stepsize ), 0 );
	return stepsize;
}

template< class EventHandlerClass, class MessageMapPolicy >
int WidgetProgressBar< EventHandlerClass, MessageMapPolicy >::getPosition()
{
	return ::SendMessage( this->Widget::itsHandle, PBM_GETPOS, 0, 0 );
}

template< class EventHandlerClass, class MessageMapPolicy >
WidgetProgressBar< EventHandlerClass, MessageMapPolicy >::WidgetProgressBar( SmartWin::Widget * parent )
	: Widget( parent, 0 )
{
	// Can't have a text box without a parent...
	xAssert( parent, _T( "Can't have a Progressbar without a parent..." ) );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetProgressBar< EventHandlerClass, MessageMapPolicy >::create( const Seed & cs )
{
	if ( cs.style & WS_CHILD )
		Widget::create( cs );
	else
	{
		typename WidgetProgressBar::Seed d_YouMakeMeDoNastyStuff = cs;

		d_YouMakeMeDoNastyStuff.style |= WS_CHILD;
		Widget::create( d_YouMakeMeDoNastyStuff );
	}
	ThisMessageMap::createMessageMap();
}

// end namespace SmartWin
}

#endif

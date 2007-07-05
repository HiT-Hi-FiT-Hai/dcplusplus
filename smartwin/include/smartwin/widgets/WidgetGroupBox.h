// $Revision: 1.24 $
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
#ifndef WidgetGroupBox_h
#define WidgetGroupBox_h

#include "../MessageMapControl.h"
#include "../MessageMapPolicyClasses.h"
#include "../TrueWindow.h"
#include "../aspects/AspectSizable.h"
#include "../aspects/AspectText.h"
#include "../aspects/AspectFont.h"
#include "../aspects/AspectVisible.h"
#include "../aspects/AspectEnabled.h"
#include "../aspects/AspectFocus.h"
#include "../aspects/AspectGetParent.h"
#include "../aspects/AspectRaw.h"
#include "../aspects/AspectBackgroundColor.h"
#include "../aspects/AspectClickable.h"
#include "../aspects/AspectDblClickable.h"
#include "../aspects/AspectEraseBackground.h"
#include "../aspects/AspectThreads.h"
#include "../aspects/AspectBorder.h"
#include "../xCeption.h"
#include "WidgetRadioButton.h"

namespace SmartWin
{
// begin namespace SmartWin

// Forward declaring friends
template< class WidgetType >
class WidgetCreator;

/** sideeffect= \par Side Effects :
  */
/// Button Control class
/** \ingroup WidgetControls
  * \WidgetUsageInfo
  * \image html radiogroup.PNG
  * Class for creating a Group Box control Widget. <br>
  * A Group Box Widget is a Widget which can contain other Widgets, normally you would 
  * add up your WidgetRadioButtons into an object of this type   
  */
template< class EventHandlerClass >
class WidgetGroupBox :
	public MessageMapPolicy< Policies::Subclassed >,
	public virtual TrueWindow,

	// Aspects
	public AspectBackgroundColor< EventHandlerClass, WidgetGroupBox< EventHandlerClass >, MessageMapControl< EventHandlerClass, WidgetGroupBox< EventHandlerClass > > >,
	public AspectBorder< WidgetGroupBox< EventHandlerClass > >,
	public AspectClickable< EventHandlerClass, WidgetGroupBox< EventHandlerClass >, MessageMapControl< EventHandlerClass, WidgetGroupBox< EventHandlerClass > > >,
	public AspectDblClickable< EventHandlerClass, WidgetGroupBox< EventHandlerClass >, MessageMapControl< EventHandlerClass, WidgetGroupBox< EventHandlerClass > > >,
	public AspectEnabled< EventHandlerClass, WidgetGroupBox< EventHandlerClass >, MessageMapControl< EventHandlerClass, WidgetGroupBox< EventHandlerClass > > >,
	public AspectEraseBackground< EventHandlerClass, WidgetGroupBox< EventHandlerClass >, MessageMapControl< EventHandlerClass, WidgetGroupBox< EventHandlerClass > > >,
	public AspectFocus< EventHandlerClass, WidgetGroupBox< EventHandlerClass >, MessageMapControl< EventHandlerClass, WidgetGroupBox< EventHandlerClass > > >,
	public AspectFont< WidgetGroupBox< EventHandlerClass > >,
	public AspectRaw< EventHandlerClass, WidgetGroupBox< EventHandlerClass >, MessageMapControl< EventHandlerClass, WidgetGroupBox< EventHandlerClass > > >,
	public AspectSizable< EventHandlerClass, WidgetGroupBox< EventHandlerClass >, MessageMapControl< EventHandlerClass, WidgetGroupBox< EventHandlerClass > > >,
	public AspectText< EventHandlerClass, WidgetGroupBox< EventHandlerClass >, MessageMapControl< EventHandlerClass, WidgetGroupBox< EventHandlerClass > > >,
	public AspectThreads< EventHandlerClass, WidgetGroupBox< EventHandlerClass >, MessageMapControl< EventHandlerClass, WidgetGroupBox< EventHandlerClass > > >,
	public AspectVisible< EventHandlerClass, WidgetGroupBox< EventHandlerClass >, MessageMapControl< EventHandlerClass, WidgetGroupBox< EventHandlerClass > > >
{
	typedef MessageMapControl< EventHandlerClass, WidgetGroupBox > MessageMapType;
	friend class WidgetCreator< WidgetGroupBox >;
public:
	/// Class type
	typedef WidgetGroupBox< EventHandlerClass > ThisType;

	/// Object type
	typedef ThisType * ObjectType;

	/// Seed class
	/** This class contains all of the values needed to create the widget. It also
	  * knows the type of the class whose seed values it contains. Every widget
	  * should define one of these.       
	  */
	class Seed
		: public SmartWin::Seed
	{
	public:
		typedef typename WidgetGroupBox::ThisType WidgetType;

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

	// Contract needed by AspectBackgroundColor Aspect class
	static inline Message & getBackgroundColorMessage();

	// Contract needed by AspectBackgroundColor Aspect class
	static inline Message & getClickMessage();

	// Contract needed by AspectBackgroundColor Aspect class
	static inline Message & getDblClickMessage();

	/// Actually creates the Button Control
	/** You should call WidgetFactory::createButton if you instantiate class
	  * directly. <br>
	  * Only if you DERIVE from class you should call this function directly.       
	  */
	virtual void create( const Seed & cs = getDefaultSeed() );

	/// Add a radio button to the group box
	void addChild( typename WidgetRadioButton< EventHandlerClass >::ObjectType btn );

protected:
	/// Constructor Taking pointer to parent
	explicit WidgetGroupBox( SmartWin::Widget * parent );

	// Protected to avoid direct instantiation, you can inherit and use
	// WidgetFactory class which is friend
	virtual ~WidgetGroupBox()
	{}

private:
	std::list< typename WidgetRadioButton< EventHandlerClass >::ObjectType > itsChildrenBtns;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template< class EventHandlerClass >
const typename WidgetGroupBox< EventHandlerClass >::Seed & WidgetGroupBox< EventHandlerClass >::getDefaultSeed()
{
	static bool d_NeedsInit = true;
	static Seed d_DefaultValues( DontInitializeMe );

	if ( d_NeedsInit )
	{
		Application::instance().setSystemClassName( d_DefaultValues, _T( "BUTTON" ) );
		d_DefaultValues.style = WS_CHILD | WS_VISIBLE | BS_GROUPBOX;
		d_DefaultValues.font = createFont( DefaultGuiFont );
		d_NeedsInit = false;
	}
	return d_DefaultValues;
}

template< class EventHandlerClass >
WidgetGroupBox< EventHandlerClass >::Seed::Seed()
{
	* this = WidgetGroupBox::getDefaultSeed();
}

template< class EventHandlerClass >
Message & WidgetGroupBox< EventHandlerClass >::getBackgroundColorMessage()
{
	static Message retVal = Message( WM_CTLCOLORBTN );
	return retVal;
}

template< class EventHandlerClass >
Message & WidgetGroupBox< EventHandlerClass >::getClickMessage()
{
	static Message retVal = Message( WM_COMMAND, BN_CLICKED );
	return retVal;
}

template< class EventHandlerClass >
Message & WidgetGroupBox< EventHandlerClass >::getDblClickMessage()
{
	static Message retVal = Message( WM_COMMAND, BN_DBLCLK );
	return retVal;
}

template< class EventHandlerClass >
WidgetGroupBox< EventHandlerClass >::WidgetGroupBox( SmartWin::Widget * parent )
	: Widget( parent, 0 )
{
	// Can't have a text box without a parent...
	xAssert( parent, _T( "Can't have a Button without a parent..." ) );
}

template< class EventHandlerClass >
void WidgetGroupBox< EventHandlerClass >::create( const Seed & cs )
{
	if ( cs.style & WS_CHILD )
		Widget::create( cs );
	else
	{
		typename WidgetGroupBox::Seed d_YouMakeMeDoNastyStuff = cs;

		d_YouMakeMeDoNastyStuff.style |= WS_CHILD;
		Widget::create( d_YouMakeMeDoNastyStuff );
	}
	ThisType::createMessageMap();
	setFont( cs.font );
}

template< class EventHandlerClass >
	void WidgetGroupBox< EventHandlerClass >::addChild( typename WidgetRadioButton< EventHandlerClass >::ObjectType btn )
{
	itsChildrenBtns.push_back( btn );
}

#ifdef PORT_ME
template< class EventHandlerClass >
LRESULT WidgetGroupBox< EventHandlerClass >::sendWidgetMessage( HWND hWnd, UINT msg, WPARAM & wPar, LPARAM & lPar )
{
	switch ( msg )
	{
		// Checking to see if it's a click event which should be routed to one of the children
		case WM_COMMAND :
		{
			for ( typename std::list< typename WidgetRadioButton< EventHandlerClass >::ObjectType >::iterator idx = itsChildrenBtns.begin();
				idx != itsChildrenBtns.end();
				++idx )
			{
				if ( reinterpret_cast< HANDLE >( lPar ) == ( * idx )->handle() )
					return ( * idx )->sendWidgetMessage( hWnd, msg, wPar, lPar );
			}
		}
	}
	return MessageMapType::sendWidgetMessage( hWnd, msg, wPar, lPar );
}
#endif
// end namespace SmartWin
}

#endif

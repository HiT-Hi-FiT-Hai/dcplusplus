// $Revision: 1.28 $
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
#ifndef WidgetButton_h
#define WidgetButton_h

#include "../MessageMapControl.h"
#include "../TrueWindow.h"
#include "../aspects/AspectSizable.h"
#include "../aspects/AspectClickable.h"
#include "../aspects/AspectDblClickable.h"
#include "../aspects/AspectText.h"
#include "../aspects/AspectFont.h"
#include "../aspects/AspectVisible.h"
#include "../aspects/AspectEnabled.h"
#include "../aspects/AspectFocus.h"
#include "../aspects/AspectGetParent.h"
#include "../aspects/AspectKeyPressed.h"
#include "../aspects/AspectBackgroundColor.h"
#include "../aspects/AspectPainting.h"
#include "../aspects/AspectRaw.h"
#include "../aspects/AspectThreads.h"
#include "../aspects/AspectBorder.h"
#include "../xCeption.h"

namespace SmartWin
{
// begin namespace SmartWin

// Forward declaring friends
template< class WidgetType >
class WidgetCreator;

/** sideeffect = \par Side Effects :
  */
/// Button Control class
/** \ingroup WidgetControls
  * \WidgetUsageInfo
  * \image html button.PNG
  * Class for creating a button control Widget. <br>
  * A button is a Widget which can be pressed, it can contain descriptive text etc.
  */
template< class EventHandlerClass, class MessageMapPolicy >
class WidgetButton :
	public MessageMapControl< EventHandlerClass, WidgetButton< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy >,
	public virtual TrueWindow,

	// Aspects
	public AspectBackgroundColor< EventHandlerClass, WidgetButton< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetButton< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectBorder< WidgetButton< EventHandlerClass, MessageMapPolicy > >,
	public AspectClickable< EventHandlerClass, WidgetButton< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetButton< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectDblClickable< EventHandlerClass, WidgetButton< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetButton< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectEnabled< EventHandlerClass, WidgetButton< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetButton< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectFocus< EventHandlerClass, WidgetButton< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetButton< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectFont< WidgetButton< EventHandlerClass, MessageMapPolicy > >,
	public AspectKeyPressed< EventHandlerClass, WidgetButton< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetButton< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectPainting< EventHandlerClass, WidgetButton< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetButton< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectRaw< EventHandlerClass, WidgetButton< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetButton< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectSizable< EventHandlerClass, WidgetButton< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetButton< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectText< EventHandlerClass, WidgetButton< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetButton< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectThreads< EventHandlerClass, WidgetButton< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetButton< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >,
	public AspectVisible< EventHandlerClass, WidgetButton< EventHandlerClass, MessageMapPolicy >, MessageMapControl< EventHandlerClass, WidgetButton< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy > >
{
	typedef MessageMapControl< EventHandlerClass, WidgetButton, MessageMapPolicy > ThisMessageMap;
	friend class WidgetCreator< WidgetButton >;
public:
	/// Class type
	typedef WidgetButton< EventHandlerClass, MessageMapPolicy > ThisType;

	/// Object type
	typedef WidgetButton< EventHandlerClass, MessageMapPolicy > * ObjectType;

	/// Seed class
	/** This class contains all of the values needed to create the widget. It also
	  * knows the type of the class whose seed values it contains. Every widget
	  * should define one of these.
	  */
	class Seed
		: public SmartWin::Seed
	{
	public:
		typedef typename WidgetButton::ThisType WidgetType;

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

	// Removing compiler hickup...
	virtual LRESULT sendWidgetMessage( HWND hWnd, UINT msg, WPARAM & wPar, LPARAM & lPar );

	// Contract needed by AspectClickable Aspect class
	static inline Message & getClickMessage();

	// Contract needed by AspectClickable Aspect class
	static inline Message & getDblClickMessage();

	// Contract needed by AspectBackgroundColor Aspect class
	static inline Message & getBackgroundColorMessage();

	/// Actually creates the Button Control
	/** You should call WidgetFactory::createButton if you instantiate class
	  * directly. <br>
	  * Only if you DERIVE from class you should call this function directly.
	  */
	virtual void create( const Seed & cs = getDefaultSeed() );

protected:
	/// Constructor Taking pointer to parent
	explicit WidgetButton( SmartWin::Widget * parent );

	// Protected to avoid direct instantiation, you can inherit and use
	// WidgetCreator class which is friend
	virtual ~WidgetButton()
	{}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template< class EventHandlerClass, class MessageMapPolicy >
const typename WidgetButton< EventHandlerClass, MessageMapPolicy >::Seed & WidgetButton< EventHandlerClass, MessageMapPolicy >::getDefaultSeed()
{
	static bool d_NeedsInit = true;
	static Seed d_DefaultValues( DontInitializeMe );

	if ( d_NeedsInit )
	{
		Application::instance().setSystemClassName( d_DefaultValues, _T("Button") );
		d_DefaultValues.style = WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON;
		d_DefaultValues.font = createFont( DefaultGuiFont );
		d_NeedsInit = false;
	}
	return d_DefaultValues;
}

template< class EventHandlerClass, class MessageMapPolicy >
WidgetButton< EventHandlerClass, MessageMapPolicy >::Seed::Seed()
{
	* this = WidgetButton::getDefaultSeed();
}

// Contract needed by AspectClickable Aspect class
template< class EventHandlerClass, class MessageMapPolicy >
LRESULT WidgetButton< EventHandlerClass, MessageMapPolicy >::sendWidgetMessage( HWND hWnd, UINT msg, WPARAM & wPar, LPARAM & lPar )
{
	return ThisMessageMap::sendWidgetMessage( hWnd, msg, wPar, lPar );
}

template< class EventHandlerClass, class MessageMapPolicy >
Message & WidgetButton< EventHandlerClass, MessageMapPolicy >::getClickMessage()
{
	static Message retVal = Message( WM_COMMAND, BN_CLICKED );
	return retVal;
}

template< class EventHandlerClass, class MessageMapPolicy >
Message & WidgetButton< EventHandlerClass, MessageMapPolicy >::getDblClickMessage()
{
	static Message retVal = Message( WM_COMMAND, BN_DBLCLK );
	return retVal;
}

template< class EventHandlerClass, class MessageMapPolicy >
Message & WidgetButton< EventHandlerClass, MessageMapPolicy >::getBackgroundColorMessage()
{
	static Message retVal = Message( WM_CTLCOLORBTN );
	return retVal;
}

template< class EventHandlerClass, class MessageMapPolicy >
WidgetButton< EventHandlerClass, MessageMapPolicy >::WidgetButton( SmartWin::Widget * parent )
	: Widget( parent, 0 )
{
	// Can't have a text box without a parent...
	xAssert( parent, _T( "Can't have a Button without a parent..." ) );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetButton< EventHandlerClass, MessageMapPolicy >::create( const Seed & cs )
{
	if ( cs.style & WS_CHILD )
		Widget::create( cs );
	else
	{
		typename WidgetButton::Seed d_YouMakeMeDoNastyStuff = cs;

		d_YouMakeMeDoNastyStuff.style |= WS_CHILD;
		Widget::create( d_YouMakeMeDoNastyStuff );
	}
	ThisMessageMap::createMessageMap();
	setFont( cs.font );
}

// end namespace SmartWin
}

#endif

// $Revision: 1.18 $
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
#ifndef WidgetMDIParent_h
#define WidgetMDIParent_h

#include "../MessageMapControl.h"
#include "../aspects/AspectSizable.h"
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

/** sideeffect= \par Side Effects :
  */
/// MDI Parent Control class
/** \ingroup WidgetControls
  * \WidgetUsageInfo
  * \image html mdi.PNG
  * Class for creating a MDI Parent Widget. <br>
  * An MDI Parent is a Widget which can contain several "inner Widgets" which must be 
  * of type WidgetMDIChild, think of it as a special "container" Widget for only 
  * WidgetMDIChild widgets. <br>
  * Note! <br>
  * You can either inherit your own WidgetMDIChild Widgets or use the factory method 
  * createMDIChild to create MDI Children Widgets. <br>
  * Related class : <br>
  * WidgetMDIChild 
  */
template< class EventHandlerClass, class unUsed >
class WidgetMDIParent :
	public MessageMapControl< EventHandlerClass, WidgetMDIParent< EventHandlerClass, unUsed >, MessageMapPolicyNormalWidget >,

	// Aspects
	public AspectSizable< EventHandlerClass, WidgetMDIParent< EventHandlerClass, unUsed >, MessageMapControl< EventHandlerClass, WidgetMDIParent< EventHandlerClass, unUsed >, MessageMapPolicyNormalWidget > >,
	public AspectVisible< EventHandlerClass, WidgetMDIParent< EventHandlerClass, unUsed >, MessageMapControl< EventHandlerClass, WidgetMDIParent< EventHandlerClass, unUsed >, MessageMapPolicyNormalWidget > >,
	public AspectEnabled< EventHandlerClass, WidgetMDIParent< EventHandlerClass, unUsed >, MessageMapControl< EventHandlerClass, WidgetMDIParent< EventHandlerClass, unUsed >, MessageMapPolicyNormalWidget > >,
	public AspectFocus< EventHandlerClass, WidgetMDIParent< EventHandlerClass, unUsed >, MessageMapControl< EventHandlerClass, WidgetMDIParent< EventHandlerClass, unUsed >, MessageMapPolicyNormalWidget > >,
	public AspectRaw< EventHandlerClass, WidgetMDIParent< EventHandlerClass, unUsed >, MessageMapControl< EventHandlerClass, WidgetMDIParent< EventHandlerClass, unUsed >, MessageMapPolicyNormalWidget > >
{
	typedef MessageMapControl< EventHandlerClass, WidgetMDIParent, MessageMapPolicyNormalWidget > ThisMessageMap;
	friend class WidgetCreator< WidgetMDIParent >;
public:
	/// Class type
	typedef WidgetMDIParent< EventHandlerClass, unUsed > ThisType;

	/// Object type
	typedef WidgetMDIParent< EventHandlerClass, unUsed > * ObjectType;

	/// Seed class
	/** This class contains all of the values needed to create the widget. It also
	  * knows the type of the class whose seed values it contains. Every widget
	  * should define one of these.       
	  */
	class Seed
		: public SmartWin::Seed
	{
	public:
		typedef typename WidgetMDIParent::ThisType WidgetType;

		// TODO: put variables to be filled here

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

	/// Actually creates the MDI EventHandlerClass Control
	/** You should call WidgetFactory::createMDIParent if you instantiate class
	  * directly. <br>
	  * Only if you DERIVE from class you should call this function directly.
	  */
	virtual void create( const Seed & cs = getDefaultSeed() );

protected:
	/// Constructor Taking pointer to parent
	explicit WidgetMDIParent( SmartWin::Widget * parent );

	// Protected to avoid direct instantiation, you can inherit and use WidgetFactory class which is friend
	virtual ~WidgetMDIParent()
	{}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template< class EventHandlerClass, class unUsed >
const typename WidgetMDIParent< EventHandlerClass, unUsed >::Seed & WidgetMDIParent< EventHandlerClass, unUsed >::getDefaultSeed()
{
	static bool d_NeedsInit = true;
	static Seed d_DefaultValues( DontInitializeMe );

	if ( d_NeedsInit )
	{
		Application::instance().setSystemClassName( d_DefaultValues, _T( "MDICLIENT" ) );
		d_DefaultValues.style = WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL;
		d_DefaultValues.exStyle = WS_EX_CLIENTEDGE;
		//TODO: initialize the values here
		d_NeedsInit = false;
	}
	return d_DefaultValues;
}

template< class EventHandlerClass, class unUsed >
WidgetMDIParent< EventHandlerClass, unUsed >::Seed::Seed()
{
	* this = WidgetMDIParent::getDefaultSeed();
}

template< class EventHandlerClass, class unUsed >
LRESULT WidgetMDIParent< EventHandlerClass, unUsed >::sendWidgetMessage( HWND hWnd, UINT msg, WPARAM & wPar, LPARAM & lPar )
{
	return ThisMessageMap::sendWidgetMessage( hWnd, msg, wPar, lPar );
}

template< class EventHandlerClass, class unUsed >
WidgetMDIParent< EventHandlerClass, unUsed >::WidgetMDIParent( SmartWin::Widget * parent )
	: Widget( parent, 0 )
{
	// Can't have a text box without a parent...
	xAssert( parent, _T( "Can't have a Button without a parent..." ) );
}

template< class EventHandlerClass, class unUsed >
void WidgetMDIParent< EventHandlerClass, unUsed >::create( const Seed & cs )
{
	if ( cs.style & WS_CHILD )
		Widget::create( cs );
	else
	{
		typename WidgetMDIParent::Seed d_YouMakeMeDoNastyStuff = cs;

		d_YouMakeMeDoNastyStuff.style |= WS_CHILD;
		Widget::create( d_YouMakeMeDoNastyStuff );
	}
	ThisMessageMap::createMessageMap();
	//TODO: use Seed parameters
}

// end namespace SmartWin
}

#endif

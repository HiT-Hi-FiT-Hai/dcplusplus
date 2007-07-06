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
#include "../BasicTypes.h"

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
template< class EventHandlerClass >
class WidgetMDIParent :
	public MessageMapPolicy< Policies::Subclassed >,

	// Aspects
	public AspectSizable< EventHandlerClass, WidgetMDIParent< EventHandlerClass >, MessageMapControl< EventHandlerClass, WidgetMDIParent< EventHandlerClass > > >,
	public AspectVisible< EventHandlerClass, WidgetMDIParent< EventHandlerClass >, MessageMapControl< EventHandlerClass, WidgetMDIParent< EventHandlerClass > > >,
	public AspectEnabled< EventHandlerClass, WidgetMDIParent< EventHandlerClass >, MessageMapControl< EventHandlerClass, WidgetMDIParent< EventHandlerClass > > >,
	public AspectFocus< EventHandlerClass, WidgetMDIParent< EventHandlerClass >, MessageMapControl< EventHandlerClass, WidgetMDIParent< EventHandlerClass > > >,
	public AspectRaw< EventHandlerClass, WidgetMDIParent< EventHandlerClass >, MessageMapControl< EventHandlerClass, WidgetMDIParent< EventHandlerClass > > >
{
	typedef MessageMapControl< EventHandlerClass, WidgetMDIParent > MessageMapType;
	friend class WidgetCreator< WidgetMDIParent >;
public:
	/// Class type
	typedef WidgetMDIParent< EventHandlerClass > ThisType;

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
		typedef typename WidgetMDIParent::ThisType WidgetType;

		// TODO: put variables to be filled here

		/// Fills with default parameters
		// explicit to avoid conversion through SmartWin::CreationalStruct
		explicit Seed();

		/// Doesn't fill any values
		Seed( DontInitialize )
		{}
		
		/** 
		 * First child id for mdi menu, must be different from any other main menu id. 
		 * Also, the menuHandle parameter of cs should point to the menu that will receive 
		 * 
		 **/
		UINT idFirstChild;
		
		HMENU windowMenu;
	};

	/// Default values for creation
	static const Seed & getDefaultSeed();

	/// Actually creates the MDI EventHandlerClass Control
	/** You should call WidgetFactory::createMDIParent if you instantiate class
	  * directly. <br>
	  * Only if you DERIVE from class you should call this function directly.
	  */
	virtual void create( const Seed & cs = getDefaultSeed() );

	
	void cascade() {
		this->sendMessage(WM_MDICASCADE);
	}
	
	void tile(bool horizontal) {
		this->sendMessage(WM_MDITILE, horizontal ? MDITILE_HORIZONTAL : MDITILE_VERTICAL);
	}
	
	void arrange() {
		this->sendMessage(WM_MDIICONARRANGE);
	}
	
	HWND getActive() {
		return reinterpret_cast<HWND>(this->sendMessage(WM_MDIGETACTIVE));
	}
	
	void setActive(SmartWin::Widget* widget) {
		// TODO check that this is an instance of mdichild
		this->sendMessage(WM_MDIACTIVATE, reinterpret_cast<WPARAM>(widget->handle()));
	}
	
	void next() {
		this->sendMessage(WM_MDINEXT);
	}
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

template< class EventHandlerClass >
const typename WidgetMDIParent< EventHandlerClass >::Seed & WidgetMDIParent< EventHandlerClass >::getDefaultSeed()
{
	static bool d_NeedsInit = true;
	static Seed d_DefaultValues( DontInitializeMe );

	if ( d_NeedsInit )
	{
		Application::instance().setSystemClassName( d_DefaultValues, _T( "MDICLIENT" ) );
		d_DefaultValues.style = WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL;
		d_DefaultValues.exStyle = WS_EX_CLIENTEDGE;
		d_DefaultValues.idFirstChild = 0;
		d_DefaultValues.windowMenu = NULL;
		//TODO: initialize the values here
		d_NeedsInit = false;
	}
	return d_DefaultValues;
}

template< class EventHandlerClass >
WidgetMDIParent< EventHandlerClass >::Seed::Seed()
{
	* this = WidgetMDIParent::getDefaultSeed();
}

template< class EventHandlerClass >
WidgetMDIParent< EventHandlerClass >::WidgetMDIParent( SmartWin::Widget * parent )
	: Widget( parent, 0 )
{
	// Can't have a text box without a parent...
	xAssert( parent, _T( "Can't have a Button without a parent..." ) );
}

template< class EventHandlerClass >
void WidgetMDIParent< EventHandlerClass >::create( const Seed & cs )
{
	CLIENTCREATESTRUCT ccs;
	ccs.hWindowMenu = cs.windowMenu;
	ccs.idFirstChild = cs.idFirstChild;
	
	this->Widget::itsHandle = ::CreateWindowEx( cs.exStyle,
		cs.getClassName().c_str(),
		cs.caption.c_str(),
		cs.style,
		cs.location.pos.x, cs.location.pos.y, cs.location.size.x, cs.location.size.y,
		this->Widget::itsParent ? this->Widget::itsParent->handle() : 0,
		NULL,
		Application::instance().getAppHandle(),
		reinterpret_cast< LPVOID >( &ccs ) );
	if ( !this->Widget::itsHandle )
	{
		// The most common error is to forget WS_CHILD in the styles
		xCeption x( _T( "CreateWindowEx in Widget::create fizzled ..." ) );
		throw x;
	}
	this->Widget::isChild = ( ( cs.style & WS_CHILD ) == WS_CHILD );
	Application::instance().registerWidget( this );

	ThisType::createMessageMap();
}

// end namespace SmartWin
}

#endif

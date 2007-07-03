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
#ifndef WidgetMDIChild_h
#define WidgetMDIChild_h

#ifndef WINCE

#include "SmartUtil.h"
#include "WidgetWindow.h"
#include "../BasicTypes.h"

#include <sstream>

namespace SmartWin
{
// begin namespace SmartWin

/** sideeffect=\par Side Effects:
  */
/// MDI Child Control class
/** \ingroup WidgetControls
  * \WidgetUsageInfo
  * \image html mdi.PNG
  * Class for creating a MDI Child Widget. <br>
  * An MDI Child is a Widget which is kind of like a special case of WidgetWindow, it 
  * exists only for two purposes which is 1. Contained in the WidgetMDIParent class 
  * and 2. To serve as a container widget for your control widgets. <br>
  * Use either the WidgetMDIParent::createMDIChild or inherit from this class and roll 
  * your own logic. <br>
  * Related classes: <br>
  * WidgetMDIParent 
  */
template< class EventHandlerClass >
class WidgetMDIChild
	: public WidgetWindowBase< EventHandlerClass, MessageMapPolicyMDIChildWidget >
{
public:
	/// Class type
	typedef WidgetMDIChild< EventHandlerClass > ThisType;

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
		typedef typename WidgetMDIChild::ThisType WidgetType;

		HICON smallIcon;
		HICON icon;
		HBRUSH background;
		

		/// Fills with default parameters
		Seed();

		/// Doesn't fill any values
		Seed( DontInitialize )
		{}
	};

	/// Default values for creation
	static const Seed & getDefaultSeed();

	/// Creates a MDIChild Window
	/** This version creates a MessageMapPolicyMDIChildWidget to plug into MDIParent
	  * container window.
	  */
	virtual void createMDIChild( Seed cs = getDefaultSeed() );

protected:
	// Protected since this Widget we HAVE to inherit from
	explicit WidgetMDIChild( Widget * parent = 0 );

	virtual ~WidgetMDIChild();

	SmartUtil::tstring getNewClassName();

private:
	SmartUtil::tstring itsRegisteredClassName;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template< class EventHandlerClass >
const typename WidgetMDIChild< EventHandlerClass >::Seed & WidgetMDIChild< EventHandlerClass >::getDefaultSeed()
{
	static bool d_NeedsInit = true;
	static Seed d_DefaultValues( DontInitializeMe );

	if ( d_NeedsInit )
	{
		d_DefaultValues.exStyle = WS_EX_MDICHILD;
		d_DefaultValues.style = WS_CHILD | WS_VISIBLE;
		d_DefaultValues.background = ( HBRUSH )( COLOR_WINDOW + 1 );
		d_DefaultValues.icon = NULL;
		d_DefaultValues.smallIcon = NULL;
		//TODO: initialize the values here
		d_NeedsInit = false;
	}
	return d_DefaultValues;
}

template< class EventHandlerClass >
WidgetMDIChild< EventHandlerClass >::Seed::Seed()
{
	* this = WidgetMDIChild::getDefaultSeed();
}

template< class EventHandlerClass >
WidgetMDIChild< EventHandlerClass >::~WidgetMDIChild()
{
	::UnregisterClass( itsRegisteredClassName.c_str(), Application::instance().getAppHandle() );
}

template< class EventHandlerClass >
WidgetMDIChild< EventHandlerClass >::WidgetMDIChild( Widget * parent )
	: Widget(parent), WidgetWindowBase< EventHandlerClass, MessageMapPolicyMDIChildWidget >( parent )
{}

template< class EventHandlerClass >
void WidgetMDIChild< EventHandlerClass >::createMDIChild( Seed cs )
{
	Application::instance().generateLocalClassName( cs );
	itsRegisteredClassName = cs.getClassName();

	//TODO: use CreationalInfo parameters
	SMARTWIN_WNDCLASSEX wc;
	wc.cbSize = sizeof( SMARTWIN_WNDCLASSEX );
	wc.style = 0;
	wc.lpfnWndProc = EventHandlerClass::wndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = Application::instance().getAppHandle();
	wc.hIcon = cs.icon;
	wc.hCursor = NULL;
	wc.hbrBackground = cs.background;
	wc.lpszMenuName = 0;
	wc.lpszClassName = itsRegisteredClassName.c_str();
	wc.hIconSm = cs.smallIcon;

	ATOM registeredClass = SmartWinRegisterClass( & wc );
	if ( 0 == registeredClass )
	{
		xCeption x( _T( "WidgetMDIChild::createMDIChild() SmartWinRegisterClass fizzled..." ) );
		throw x;
	}

	this->Widget::itsHandle = ::CreateMDIWindow( itsRegisteredClassName.c_str(),
		cs.caption.c_str(),
		cs.style,
		cs.location.pos.x, cs.location.pos.y, cs.location.size.x, cs.location.size.y,
		this->Widget::itsParent->handle(),
		Application::instance().getAppHandle(),
		reinterpret_cast< LPARAM >( dynamic_cast< Widget * >( this ) ) );
	if ( !this->Widget::itsHandle )
	{
		xCeption x( _T( "CreateWindowEx in WidgetMDIChild::createMDIChild fizzled..." ) );
		throw x;
	}
	Application::instance().addLocalWindowClassToUnregister( cs );
	//::ShowWindow( this->Widget::itsHandle, SW_SHOW );
	//this->Widget::isChild = true;
	this->registerWidget();
}

template< class EventHandlerClass >
SmartUtil::tstring WidgetMDIChild< EventHandlerClass >::getNewClassName()
{
	std::basic_stringstream< TCHAR > className;
	className << _T( "WidgetFactory" ) << ++this->Widget::itsInstanceNo;
	return className.str();
}

// end namespace SmartWin
}

#endif //! WINCE

#endif

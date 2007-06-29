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
#ifndef WidgetMenu_h
#define WidgetMenu_h

#include "../MessageMapControl.h"
#include "../WindowsHeaders.h"
#include "../aspects/AspectGetParent.h"
#include "SmartUtil.h"
#include "../aspects/AspectAdapter.h"
#include <vector>
#include <functional>
#include <boost/cast.hpp>

namespace SmartWin
{
// begin namespace SmartWin

// Forward declaring friends
template< class WidgetType >
class WidgetCreator;

struct WidgetMenuSimpleDispatcher
{
	typedef std::tr1::function<void ()> F;

	WidgetMenuSimpleDispatcher(const F& f_) : f(f_) { }

	HRESULT operator()(private_::SignalContent& params) {
		f();
		return 1;
	}

	F f;
};

struct WidgetMenuIdDispatcher
{
	typedef std::tr1::function<void (unsigned)> F;

	WidgetMenuIdDispatcher(const F& f_) : f(f_) { }

	HRESULT operator()(private_::SignalContent& params) {
		f(LOWORD(params.Msg.WParam ));
		return 1;
	}

	F f;
};

template< class EventHandlerClass, class MessageMapPolicy >
class WidgetMenu;

template< class EventHandlerClass, class MessageMapPolicy, Platform >
class WidgetMenuPlatformImplementation;

/// Specialized functions in menu for Windows CE Windows API version
template< class EventHandlerClass, class MessageMapPolicy >
class WidgetMenuPlatformImplementation< EventHandlerClass, MessageMapPolicy, SmartWinCE > :
	public MessageMapControl< EventHandlerClass, WidgetMenu< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy >
{
protected:
	typedef WidgetMenu<EventHandlerClass, MessageMapPolicy> WidgetMenuType;
	typedef MessageMapControl< EventHandlerClass, WidgetMenuType, MessageMapPolicy > MessageMapType;

public:
	struct Seed {
		Seed() { }
	};

	typedef std::tr1::shared_ptr< WidgetMenu< EventHandlerClass, MessageMapPolicy > > WidgetMenuPtr;

	HMENU handle() { return reinterpret_cast<HMENU>(this->handle()); }
	
	/// Actually creates the Menu
	/** You should call WidgetFactory::createMenu if you instantiate class directly.
	  * <br>
	  * Only if you DERIVE from class you should call this function directly.       
	  */
	virtual void create(const typename WidgetMenuType::Seed cs)
	{
		itsCmdBar = CommandBar_Create( Application::instance().getAppHandle(), this->Widget::itsParent->handle(), 1 );
		if ( !itsCmdBar )
		{
			xCeption x( _T( "CommandBar_Create in WidgetMenu::create fizzled..." ) );
			throw x;
		}
		HMENU menu = ::CreateMenu();
		if ( !menu )
		{
			xCeption x( _T( "CreateMenu in WidgetMenu::create fizzled..." ) );
			throw x;
		}
		// Why on EARTH this works I have no idea whatsoever, but apparently it does... (casting to string!)
		LPTSTR menuName = reinterpret_cast< LPTSTR >( menu );
		CommandBar_InsertMenubarEx( itsCmdBar, reinterpret_cast< HINSTANCE >( 0 ), menuName, 0 );

		// Since we're defaulting the window creation to no capture and no title we must add the X in the command bar...
		CommandBar_AddAdornments( itsCmdBar, 0, 0 );

		// TODO: Is this safe ? ! ?
		// At least we should verify it...
		BOOST_STATIC_ASSERT( sizeof( HWND ) == sizeof( HMENU ) );
		this->Widget::itsHandle = reinterpret_cast< HWND >( menu );

		this->Widget::registerWidget();
	}

	/// Appends a popup to the menu
	/** Everything you "append" to a menu is added sequentially to the menu! <br>
	  * This specific "append" function appends a "popup" menu which is a menu
	  * containg other menus. <br>
	  * With other words a menu which is not an "option" but rather a new "subgroup".
	  * <br>
	  * The "File" menu of most application is for instance a "popup" menu while the
	  * File/Print is NOT a popup. <br>
	  * To append items to the popup created call one of the appendItem overloaded
	  * functions on the returned value of this function. <br>
	  * Also, although references to all menu objects must be kept ( since they're
	  * not collected automatically like other Widgets ) <br>
	  * you don't have to keep a reference to the return value of this function since
	  * it's being added as a reference to the children list <br>
	  * of the "this" object. <br>
	  * See the WidgetMenu sample project for a demonstration.
	  */
	WidgetMenuPtr appendPopup( const SmartUtil::tstring & name )
	{
		HMENU handle = reinterpret_cast< HMENU >( this->Widget::itsHandle );
		WidgetMenuPtr retVal = WidgetMenuPtr( new WidgetMenu< EventHandlerClass, MessageMapPolicy >( this->Widget::itsParent ) );
		HMENU popup = CreatePopupMenu();
		retVal->itsHandle = reinterpret_cast< HWND >( popup );
		::AppendMenu( handle, MF_POPUP, reinterpret_cast< unsigned int >( retVal->handle() ), name.c_str() );
		itsChildren.push_back( retVal );
		retVal->Widget::registerWidget();
		return retVal;
	}

	WidgetMenuPlatformImplementation()
		: Widget(0), itsCmdBar( 0 )
	{}

	virtual ~WidgetMenuPlatformImplementation()
	{
		if ( itsCmdBar )
		{
			CommandBar_Destroy( itsCmdBar );
		}
	}
private:
	HWND itsCmdBar;

	// Children, only "popup" menus are supposed to have children
	std::vector< WidgetMenuPtr > itsChildren;
};

/// Specialized functions in menu for desktop Windows API version
template< class EventHandlerClass, class MessageMapPolicy >
class WidgetMenuPlatformImplementation< EventHandlerClass, MessageMapPolicy, SmartWinDesktop > :
	public MessageMapControl< EventHandlerClass, WidgetMenu< EventHandlerClass, MessageMapPolicy >, MessageMapPolicy >
{
protected:
	typedef WidgetMenu<EventHandlerClass, MessageMapPolicy> WidgetMenuType;
	typedef MessageMapControl< EventHandlerClass, WidgetMenuType, MessageMapPolicy > MessageMapType;
	WidgetMenuPlatformImplementation() : Widget(0) { }
public:
	struct Seed {
		Seed(bool popup_) : popup(popup) { }
		Seed() : popup(false) { }
		bool popup;
	};

	typedef std::tr1::shared_ptr< WidgetMenuType > WidgetMenuPtr;
	HMENU handle() { return reinterpret_cast<HMENU>(this->Widget::handle()); }

	/// Attaches the menu to a parent window
	/** Note! Menus can be switched between at runtime, you can have several menus
	  * that the EventHandlerClass switches between. <br>
	  * This can be done by attaching another menu object. <br>
	  * For an example of this see the WidgetMenu project.       
	  */
	void attach( EventHandlerClass * mainWindow )
	{
		::SetMenu( mainWindow->handle(), handle() );
	}

	/// Actually creates the Menu
	/** You should call WidgetFactory::createMenu if you instantiate class directly.
	  * <br>
	  * Only if you DERIVE from class you should call this function directly.       
	  */
	void create(const Seed& cs)
	{
		HMENU handle;
		if(cs.popup) {
			handle = ::CreatePopupMenu();
		} else {
			handle = ::CreateMenu();
		}
		if ( !handle )
		{
			xCeption x( _T( "CreateMenu in WidgetManu::create fizzled..." ) );
			throw x;
		}

		// TODO: Is this safe ? ! ?
		// At least we should verify it...
		BOOST_STATIC_ASSERT( sizeof( HWND ) == sizeof( HMENU ) );
		this->Widget::itsHandle = reinterpret_cast< HWND >( handle );

		// Menu controls uses the Windows Message Procedure of the parent window
		this->MessageMapType::itsDefaultWindowProc = NULL;

		this->Widget::registerWidget();
	}

	/// Appends a popup to the menu
	/** Everything you "append" to a menu is added sequentially to the menu <br>
	  * This specific "append" function appends a "popup" menu which is a menu
	  * containg other menus. <br>
	  * With other words a menu which is not an "option" but rather a new "subgroup". 
	  * <br>
	  * The "File" menu of most application is for instance a "popup" menu while the 
	  * File/Print is often NOT a popup. <br>
	  * To append items to the popup created call one of the appendItem overloaded 
	  * functions on the returned value of this function. <br>
	  * Also, although references to all menu objects must be kept ( since they're 
	  * not collected automatically like other Widgets ) <br>
	  * you don't have to keep a reference to the return value of this function since 
	  * it's being added as a reference to the children list <br>
	  * of the "this" object. <br>
	  * See the WidgetMenu project for a demonstration.       
	  */
	WidgetMenuPtr appendPopup( const SmartUtil::tstring & name )
	{

//		WidgetMenuPtr retVal = WidgetMenuPtr( new WidgetMenu< EventHandlerClass, MessageMapPolicy >( this->Widget::itsParent ) );
// Should it be the below instead ?
		WidgetMenuPtr retVal = WidgetMenuPtr( new WidgetMenu< EventHandlerClass, MessageMapPolicy >( this ) );

		retVal->create(Seed(true));
		::AppendMenu( handle(), MF_POPUP, reinterpret_cast< unsigned int >( retVal->handle() ), name.c_str() );
		itsChildren.push_back( retVal );
		return retVal;
	}
	
	/// Returns the "System Menu"
	/** The system menu is a special menu that ( normally ) is accessed by pressing
	  * the "window icon" at the top left of the window. <br>
	  * In SmartWin++ this menu can ALSO be easily manipulated and added items to 
	  * etc... <br>
	  * Also, although references to all menu objects must be kept ( since they're 
	  * not collected automatically like other Widgets ) <br>
	  * you don't have to keep a reference to the return value of this function since 
	  * it's being added as a reference to the children list <br>
	  * of the "this" object. <br>
	  * See the WidgetMenu sample project for a demonstration.       
	  */
	WidgetMenuPtr getSystemMenu()
	{
		HMENU h = ::GetSystemMenu( internal_::getTypedParentOrThrow < EventHandlerClass * >( this )->handle(), FALSE );
		WidgetMenuPtr sysMenu( new WidgetMenu< EventHandlerClass, MessageMapPolicy >( this->Widget::itsParent ) );
		sysMenu->Widget::itsHandle = reinterpret_cast< HWND >( h );
		sysMenu->Widget::registerWidget();
		sysMenu->isSysMenu = true;

		// We're assuming that the system menu has the same lifespan as the "this"
		// menu, we must keep a reference to te system menu otherwise it will be
		// "lost", therefore we add it up as a child to the "this" menu...
		itsChildren.push_back( sysMenu );
		return sysMenu;
	}

protected:
	// Children, only "popup" menus are supposed to have children
	std::vector< WidgetMenuPtr > itsChildren;
};

/// Menu class
/** \ingroup WidgetControls
  * \WidgetUsageInfo
  * \image html menu.PNG
  * Class for creating a Menu Control which then can be attached to a WidgetWindow. 
  * <br>
  * Note for Desktop applications only! <br>
  * After you have created a menu you must call WidgetMenu::attach() to make it 
  * "attach" to the WidgetWindow you want it to belong to. <br>
  * Related class : <br>
  * WidgetMenuExtended   
  */
template< class EventHandlerClass, class MessageMapPolicy >
class WidgetMenu :
	public WidgetMenuPlatformImplementation< EventHandlerClass, MessageMapPolicy, CurrentPlatform >,
	public boost::enable_shared_from_this<WidgetMenu<EventHandlerClass, MessageMapPolicy> >
{
protected:
	typedef WidgetMenuPlatformImplementation< EventHandlerClass, MessageMapPolicy, CurrentPlatform > PlatformImplementation;
	typedef typename PlatformImplementation::MessageMapType MessageMapType;
	typedef WidgetMenuSimpleDispatcher SimpleDispatcher;
	typedef WidgetMenuIdDispatcher IdDispatcher;
	typedef AspectAdapter<IdDispatcher::F, EventHandlerClass, MessageMapType::IsControl> Adapter;

	// friends
	friend class WidgetMenuPlatformImplementation< EventHandlerClass, MessageMapPolicy, CurrentPlatform >;
	friend class WidgetCreator< WidgetMenu >;
public:
	/// Class type
	typedef WidgetMenu< EventHandlerClass, MessageMapPolicy > ThisType;

	/// Object type
	typedef typename PlatformImplementation::WidgetMenuPtr ObjectType;

	/// Creational info
	typedef typename PlatformImplementation::Seed Seed;
		/// \ingroup eventsSignatures
	/// \typedef Typedef of a member function to the original class taking pointer to the this Widget and an unsigned int returning void
	typedef void ( EventHandlerClass::* itsVoidMenuFunctionTakingUInt )( ObjectType, unsigned );

	/// \ingroup eventsSignatures
	/// Typedef of a static/global function taking a pointer to the original class, a pointer to the this Widget class and an unsigned int returning void
	typedef void ( * menuVoidFunctionTakingUInt )( EventHandlerClass *, ObjectType, unsigned );

	/// \ingroup EventHandlersWidgetMenu
	/// Appends a "normal" menu item
	/** eventHandler is the function that will receive the "click" event from the
	  * menu item. <br>
	  * Event handler's signature must be "void foo(unsigned int )" 
	  * and it must be contained as a member <br>
	  * of the class that is defined as the EventHandlerClass, normally either the 
	  * WidgetWindow derived class or the class derived from WidgetMenu. <br>
	  * See e.g. WidgetFun for an example. <br>
	  * The reason to why we have this "id" is because the same event handler can be 
	  * defined for several menu items <br>
	  * even in fact across menu objects, therefore this number should be unique 
	  * across the application. 
	  */
	void appendItem( unsigned int id, const SmartUtil::tstring & name, itsVoidMenuFunctionTakingUInt eventHandler ) {
		appendItem(id, name, Adapter::adapt1(ThisType::shared_from_this(), eventHandler));
	}
	void appendItem( unsigned int id, const SmartUtil::tstring & name, menuVoidFunctionTakingUInt eventHandler ) {
		appendItem(id, name, Adapter::adapt1(ThisType::shared_from_this(), eventHandler));
	}
	void appendItem( unsigned int id, const SmartUtil::tstring & name, const IdDispatcher::F& f ) {
		appendItem(id, name, NULL, f);
	}
	void appendItem( unsigned int id, const SmartUtil::tstring & name, ULONG_PTR data, itsVoidMenuFunctionTakingUInt eventHandler ) {
		appendItem(id, name, data, Adapter::adapt1(ThisType::shared_from_this(), eventHandler));
	}
	void appendItem( unsigned int id, const SmartUtil::tstring & name, ULONG_PTR data, menuVoidFunctionTakingUInt eventHandler ) {
		appendItem(id, name, data, Adapter::adapt1(ThisType::shared_from_this(), eventHandler));
	}
	void appendItem( unsigned int id, const SmartUtil::tstring & name, ULONG_PTR data, const IdDispatcher::F& f );
	void appendItem( unsigned int id, const SmartUtil::tstring & name, ULONG_PTR data, const SimpleDispatcher::F& f );

	ULONG_PTR getData(unsigned int id);

	/// \ingroup EventHandlersWidgetMenu
	/// Appends a checked menu item
	/** eventHandler is the function that will receive the "click" event from the
	  * menu item. <br>
	  * Event handler's signature must be "void foo( WidgetMenuPtr, unsigned int )" 
	  * and it must be contained as a member <br>
	  * of the class that is defined as the EventHandlerClass, normally either the 
	  * WidgetWindow derived class or the class derived from WidgetMenu. <br>
	  * See e.g. WidgetFun for an example. <br>
	  * The reason to why we have this "id" is because the same event handler can be 
	  * defined for several menu items <br>
	  * even in fact across menu objects, therefore this number should be unique 
	  * across the application. 
	  */
	void appendCheckedItem( unsigned id, const SmartUtil::tstring & name, itsVoidMenuFunctionTakingUInt eventHandler );
	void appendCheckedItem( unsigned id, const SmartUtil::tstring & name, menuVoidFunctionTakingUInt eventHandler );

	/// Appends a separator item to the menu
	/** A menu separator is basically just "air" between menu items. <br>
	  * A separator cannot be "clicked" or "chosen".
	  */
	void appendSeparatorItem();

	/// Returns the text of a specific menu item
	/** Which menu item you wish to retrieve the text for is defined by the "id"
	  * parameter of the function.
	  */
	SmartUtil::tstring getText( unsigned idOrPos, bool byPos );

	/// Sets the text of a specific menu item
	/** Which menu item you wish to set the text is defined by the "id"
	  * parameter of the function.
	  */
	void setText( unsigned id, const SmartUtil::tstring& text );

	/// Checks (or uncheck) a specific menu item
	/** Which menu item you wish to check ( or uncheck ) is passed in as the "id"
	  * parameter. <br>
	  * If the "value" parameter is true the item will be checked, otherwise it will 
	  * be unchecked       
	  */
	void checkItem( unsigned id, bool value = true );

	/// Returns a boolean indicating if a specific menu item is checked or not
	/** Which menu item you wish to check must be passed as the "id" parameter of the
	  * function
	  */
	bool getCheckedState( unsigned id );

	/// Enables (or disables) a specific menu item
	/** Which menu item you wish to enable ( or disable ) is passed in as the "id"
	  * parameter. <br>
	  * If the "value" parameter is true the item becomes enabled, otherwise disabled       
	  */
	void setItemEnabled( unsigned id, bool value = true );

	/// Returns a boolean indicating if a specific menu item is enabled or not
	/** Which menu item you wish to check must be passed as the "id" parameter to the
	  * function.
	  */
	bool getItemEnabled( unsigned id );
	
	/// Return the id associated with a certain position
	UINT getId(unsigned postition);

	/// Return the number of items in the menu
	int getCount();

	UINT getMenuState(UINT id, bool byPosition = false);
	
	/// Return true if the item is a separator (by position)
	bool isSeparator(UINT id, bool byPosition = false);
	/// Return true if the menu item is checked
	bool isChecked(UINT id, bool byPosition = false);
	/// Return true if the menu item is a popup menu
	bool isPopup(UINT id, bool byPosition = false);
	/// Return true if the menu item is enabled (not grey and not disabled)
	bool isEnabled(UINT id, bool byPosition = false);
	
	void setDefaultItem(UINT id, bool byPosition = false);
	
	ObjectType getChild(UINT position);
	
	/// Displays and handles a menu which can appear anywhere in the window.
	/** Typically called by a Right Mouse click. If both the x and the y coordinates
	  * are - 1 ( default ), it'll show the context menu at the mouse position when
	  * the system last received a message, basically the "right" place. Depending on 
	  * the flags it might return the id of the menu item selected, or 0 if none was 
	  * chosen. Flags with TPM_RETURNCMD will return the menu - item, but not call 
	  * the Event Handler.       
	  * < ul >
	  * < li >TPM_CENTERALIGN : Centers the shortcut menu horizontally relative to the coordinate specified by the x parameter< /li >
	  * < li >TPM_LEFTALIGN : Function positions the shortcut menu so that its left side is aligned with the coordinate specified by the x parameter< /li >
	  * < li >TPM_RIGHTALIGN : Opposite of LEFTALIGN< /li >
	  * < li >TPM_BOTTOMALIGN : Aligns menu bottoms to the coordinate specified by the y parameter< /li >
	  * < li >TPM_TOPALIGN : Opposite of BOTTOMALIGN< /li >
	  * < li >TPM_VCENTERALIGN : Centers vertically relative to the y parameter< /li >
	  * < li >TPM_NONOTIFY : Restricts the menu from sending notifications when user clicks item< /li >
	  * < li >TPM_RETURNCMD : returns the menu item identifier of the user's selection in the return value but DOES NOT carry out the event handler< /li >
	  * < li >TPM_LEFTBUTTON  : Restricts users to selecting menu items with only left mouse button< /li >
	  * < li >TPM_RIGHTBUTTON : User can choose menu item with both mouse buttons< /li >
	  * < /ul >
	  * None of the following are used by default but can be manually chosen if you 
	  * manually call SystemParametersInfo       
	  * < ul >
	  * < li >TPM_HORNEGANIMATION : Animates the menu from right to left< /li >
	  * < li >TPM_HORPOSANIMATION : Animates the menu from left to right< /li >
	  * < li >TPM_NOANIMATION : Displays menu without animation< /li >
	  * < li >TPM_VERNEGANIMATION : Animates the menu from bottom to top< /li >
	  * < li >TPM_VERPOSANIMATION : Animates the menu from top to bottom< /li >
	  * < /ul >
	  */
	unsigned trackPopupMenu( EventHandlerClass * mainWindow, int x = - 1, int y = - 1, unsigned flags = 0 );

	bool isSystemMenu()
	{
		return isSysMenu;
	}

	// We CAN'T have this one protected since boost needs to be able to destroy
	// objects of this type
	// TODO: Fix...!!
	virtual ~WidgetMenu()
	{
		::DestroyMenu( reinterpret_cast< HMENU >( MessageMapPolicy::itsHandle ) );
	}

protected:
	/// Constructor Taking pointer to parent
	explicit WidgetMenu( SmartWin::Widget * parent );

private:
	// True is menu is "system menu" (icon in top left of window)
	bool isSysMenu;

	WidgetMenu( const WidgetMenu & ); // Never implemented intentionally
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template< class EventHandlerClass, class MessageMapPolicy >
void WidgetMenu< EventHandlerClass, MessageMapPolicy >::appendItem
	( unsigned int id, const SmartUtil::tstring & name
	, ULONG_PTR data, const IdDispatcher::F& f
	)
{
	MENUITEMINFO mii = { sizeof(MENUITEMINFO) };
	
	mii.fMask = MIIM_ID | MIIM_TYPE | MIIM_DATA;
	mii.fType = MFT_STRING;
	mii.dwTypeData = const_cast<LPTSTR>(name.c_str());
	mii.dwItemData = data;
	mii.wID = id;
	::InsertMenuItem(this->handle(), this->getCount(), TRUE, &mii);

	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->setCallback(
		Message( WM_COMMAND, id ), IdDispatcher(f)
	);
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetMenu< EventHandlerClass, MessageMapPolicy >::appendItem
	( unsigned int id, const SmartUtil::tstring & name
	, ULONG_PTR data, const SimpleDispatcher::F& f
	)
{
	MENUITEMINFO mii = { sizeof(MENUITEMINFO) };
	
	mii.fMask = MIIM_ID | MIIM_TYPE | MIIM_DATA;
	mii.fType = MFT_STRING;
	mii.dwTypeData = const_cast<LPTSTR>(name.c_str());
	mii.dwItemData = data;
	mii.wID = id;
	::InsertMenuItem(this->handle(), this->getCount(), TRUE, &mii);

	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->setCallback(
		Message( WM_COMMAND, id ), SimpleDispatcher(f)
	);
}

template< class EventHandlerClass, class MessageMapPolicy >
ULONG_PTR WidgetMenu< EventHandlerClass, MessageMapPolicy >::getData(unsigned int id) {
	MENUITEMINFO mii = { sizeof(MENUITEMINFO) };
	mii.fMask = MIIM_DATA;
	::GetMenuItemInfo(this->handle(), id, FALSE, &mii);
	return mii.dwItemData;
}


template< class EventHandlerClass, class MessageMapPolicy >
void WidgetMenu< EventHandlerClass, MessageMapPolicy >::appendCheckedItem
	( unsigned id, const SmartUtil::tstring & name
	, itsVoidMenuFunctionTakingUInt eventHandler
	)
{
	/// @todo This is not quite the same as appending a real checked item...
	appendItem(id, name, eventHandler);
	checkItem(id, true);
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetMenu< EventHandlerClass, MessageMapPolicy >::appendCheckedItem
	( unsigned id, const SmartUtil::tstring & name
	, menuVoidFunctionTakingUInt eventHandler
	)
{
	appendItem(id, name, eventHandler);
	checkItem(id, true);
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetMenu< EventHandlerClass, MessageMapPolicy >::appendSeparatorItem()
{
	::AppendMenu( this->handle(), MF_SEPARATOR, 0, 0 );
}

template< class EventHandlerClass, class MessageMapPolicy >
UINT WidgetMenu< EventHandlerClass, MessageMapPolicy >::getId(UINT position)
{
	return ::GetMenuItemID( this->handle(), position );
}

template< class EventHandlerClass, class MessageMapPolicy >
SmartUtil::tstring WidgetMenu< EventHandlerClass, MessageMapPolicy >::getText( unsigned id, bool byPosition )
{
	MENUITEMINFO mi;
	memset( & mi, 0, sizeof( MENUITEMINFO ) );

	mi.cbSize = sizeof( MENUITEMINFO );
	mi.fMask = MIIM_TYPE;
	if ( ::GetMenuItemInfo( this->handle(), id, byPosition, & mi ) == 0 )
	{
		xAssert( false, _T( "Error while trying to get MenuItemInfo in WidgetMenu::getText..." ) );
	}
	boost::scoped_array< TCHAR > buffer( new TCHAR[++mi.cch] );
	mi.dwTypeData = buffer.get();
	if ( ::GetMenuItemInfo( this->handle(), id, FALSE, & mi ) == 0 )
	{
		xAssert( false, _T( "Error while trying to get MenuItemInfo in WidgetMenu::getText..." ) );
	}
	SmartUtil::tstring retVal = mi.dwTypeData;
	return retVal;
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetMenu< EventHandlerClass, MessageMapPolicy >::setText( unsigned id, const SmartUtil::tstring& text )
{
	MENUITEMINFO info = { sizeof( MENUITEMINFO ) };

	// set flag
	info.fMask = MIIM_STRING;
	info.dwTypeData = (TCHAR*) text.c_str();

	if ( ::SetMenuItemInfo( this->handle(), id, FALSE, & info ) == FALSE )
		throw xCeption( _T( "Couldn't set item info in setItemText()" ) );
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetMenu< EventHandlerClass, MessageMapPolicy >::checkItem( unsigned id, bool value )
{
	::CheckMenuItem( this->handle(), id, value ? MF_CHECKED : MF_UNCHECKED );
}

template< class EventHandlerClass, class MessageMapPolicy >
typename WidgetMenu< EventHandlerClass, MessageMapPolicy >::ObjectType WidgetMenu< EventHandlerClass, MessageMapPolicy >::getChild( unsigned position ) {
	HMENU h = reinterpret_cast<HMENU>(getId(position));
	for(size_t i = 0; i < this->itsChildren.size(); ++i) {
		ObjectType& menu = this->itsChildren[i];
		if(menu->handle() == h) {
			return menu;
		}
	}
	return ObjectType();
}

template< class EventHandlerClass, class MessageMapPolicy >
bool WidgetMenu< EventHandlerClass, MessageMapPolicy >::getCheckedState( unsigned id )
{
	return isChecked();
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetMenu< EventHandlerClass, MessageMapPolicy >::setItemEnabled( unsigned id, bool value )
{
	if ( ::EnableMenuItem( this->handle(), id, value ? MF_ENABLED : MF_GRAYED ) == - 1 )
	{
		xCeption x( _T( "Couldn't enable/disable the menu item, item doesn't exist" ) );
		throw x;
	}
}

template< class EventHandlerClass, class MessageMapPolicy >
bool WidgetMenu< EventHandlerClass, MessageMapPolicy >::getItemEnabled( unsigned id )
{
	return isEnabled(id);
}

template< class EventHandlerClass, class MessageMapPolicy >
int WidgetMenu< EventHandlerClass, MessageMapPolicy >::getCount()
{
	int count = ::GetMenuItemCount( this->handle() );
	if( count == -1 )
		throw xCeption( _T( "Couldn't get item count in getCount()" ) );
	return count;
}

template< class EventHandlerClass, class MessageMapPolicy >
UINT WidgetMenu< EventHandlerClass, MessageMapPolicy >::getMenuState( UINT id, bool byPosition )
{
	return ::GetMenuState(this->handle(), id, byPosition ? MF_BYPOSITION : MF_BYCOMMAND); 
}

template< class EventHandlerClass, class MessageMapPolicy >
void WidgetMenu< EventHandlerClass, MessageMapPolicy >::setDefaultItem( UINT id, bool byPosition )
{
	::SetMenuDefaultItem(this->handle(), id, byPosition); 
}

template< class EventHandlerClass, class MessageMapPolicy >
bool WidgetMenu< EventHandlerClass, MessageMapPolicy >::isChecked( UINT id, bool byPosition )
{
	return (getMenuState(id, byPosition) & MF_CHECKED) == MF_CHECKED; 
}
template< class EventHandlerClass, class MessageMapPolicy >
bool WidgetMenu< EventHandlerClass, MessageMapPolicy >::isEnabled( UINT id, bool byPosition )
{
	return !(getMenuState(id, byPosition) & (MF_DISABLED | MF_GRAYED)); 
}
template< class EventHandlerClass, class MessageMapPolicy >
bool WidgetMenu< EventHandlerClass, MessageMapPolicy >::isPopup( UINT id, bool byPosition )
{
	return (getMenuState(id, byPosition) & MF_POPUP) == MF_POPUP; 
}
template< class EventHandlerClass, class MessageMapPolicy >
bool WidgetMenu< EventHandlerClass, MessageMapPolicy >::isSeparator( UINT id, bool byPosition )
{
	return (getMenuState(id, byPosition) & MF_SEPARATOR) == MF_SEPARATOR; 
}

template< class EventHandlerClass, class MessageMapPolicy >
unsigned WidgetMenu< EventHandlerClass, MessageMapPolicy >::
trackPopupMenu( EventHandlerClass * mainWindow, int x, int y, unsigned flags )
{
	xAssert( mainWindow != 0, _T( "EventHandlerClass can't be null while trying to display Popup Menu" ) );
	if ( x == - 1 && y == - 1 )
	{
		DWORD pos = ::GetMessagePos();
		x = LOWORD( pos );
		y = HIWORD( pos );
	}
	int retVal = ::TrackPopupMenu
		( this->handle()
		, flags, x, y, 0
		, mainWindow->handle(), 0
		);
	return retVal;
}

template< class EventHandlerClass, class MessageMapPolicy >
WidgetMenu< EventHandlerClass, MessageMapPolicy >::WidgetMenu( SmartWin::Widget * parent )
	: Widget( 0 )
	, isSysMenu( false )
{
	// We construct Widget(0) so that the WidgetMenu is not placed in parent->itsChildren
	// (If a WidgetMenu is in parent->Children then WidgetModalDialog crashs during cleanup.
	xAssert( parent, _T( "Can't have a Menu without a parent..." ) );
    this->Widget::itsParent= parent;	// But eventually we do want to know its parent.
}
// end namespace SmartWin
}

#endif

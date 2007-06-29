// $Revision: 1.27 $
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
#ifndef MessageMap_h
#define MessageMap_h

#include <list>
#include "WindowsHeaders.h"
#include "Application.h"
#include "MessageMapPolicyClasses.h"
#include "Widget.h"
#include "BasicTypes.h"
#include "Message.h"
#include "Command.h"
#include "widgets/WidgetMenu.h"
#include "widgets/WidgetMenuExtended.h"
#include "MessageMapBase.h"
#include "CanvasClasses.h"
#include <vector>

namespace SmartWin
{
// begin namespace SmartWin

/// The "fallback" WndMsgProc handler
/** Is the "default" message handler class for all Container Widgets (every Widget
  * which inherits from WidgetWindowBase). <br>
  * If a message is not handled by anything else it goes into this class to check if
  * we should handle this message or not. <br>
  * If the message is NOT handled it is returned to windows OS for handling. <br>
  * Shouldn't be of much interest directly to a user of SmartWin unless it is being
  * overridden to handle messages not possible to reach with event handlers in
  * SmartWin. Even this is possible through using the AspectRaw::onRaw instead of
  * overriding functions in this class! <br>
  * Related classes
  * <ul>
  * <li>MessageMapControl</li>
  * </ul>
  */
template< class EventHandlerClass, class MessageMapPolicy >
class MessageMap
	: public virtual Widget
	, public MessageMapPolicy
	, public MessageMapBase
{
public:
	typedef MessageMapBase::CallbackType SignalType;
	typedef std::pair< Message, SignalType > SignalTupleType;
	typedef MessageMapBase::CallbackCollectionType SignalCollection;

	// Contract needed by Aspects in order to know which instantiation of template
	// aspect dispatcher class is needed
	const static bool IsControl = false;

	// Member event handler definitions (here you have the "this" pointer so
	// there's no need for passing the EventHandlerClass *)

	/// \ingroup eventsSignatures
	/// Typedef of a member function to the original class taking void returning bool
	typedef bool ( EventHandlerClass::* itsBoolFunctionTakingVoid )();

	/// \ingroup eventsSignatures
	/// Typedef of a member function to the original class taking const MouseEventResult & returning void
	typedef void ( EventHandlerClass::* itsVoidFunctionTakingMouseEventResult )( const MouseEventResult & );

	/// \ingroup eventsSignatures
	/// Typedef of a member function to the original class taking void returning void
	typedef void ( EventHandlerClass::* itsVoidFunctionTakingVoid )();

	/// \ingroup eventsSignatures
	/// Typedef of a member function to the original class taking one bool returning void
	typedef void ( EventHandlerClass::* itsVoidFunctionTakingBool )( bool );

	/// \ingroup eventsSignatures
	/// Typedef of a member function to the original class taking LPARAM and WPARAM returning HRESULT
	typedef HRESULT ( EventHandlerClass::* itsHresultFunctionTakingLparamWparam )( LPARAM, WPARAM );

	/// \ingroup eventsSignatures
	/// Typedef of a member function to the original class taking tstring-vector and Point returning void
	typedef void ( EventHandlerClass::* itsVoidFunctionTakingVectorPoint )( std::vector< SmartUtil::tstring>, Point );

	/// \ingroup eventsSignatures
	/// Typedef of a member function to the original class taking one SmartUtil::tstring & returning void
	typedef void ( EventHandlerClass::* itsVoidFunctionTakingString )( SmartUtil::tstring & );

	/// \ingroup eventsSignatures
	/// Typedef of a member function to the original class taking one SmartUtil::tstring & returning void
	typedef void ( EventHandlerClass::* itsVoidFunctionTakingConstString )( const SmartUtil::tstring & );

	/// \ingroup eventsSignatures
	/// Typedef of a member function to the original class taking one int returning void
	typedef void ( EventHandlerClass::* itsVoidFunctionTakingInt )( int );

	/// \ingroup eventsSignatures
	/// Typedef of a member function to the original class taking one int returning bool
	typedef bool ( EventHandlerClass::* itsBoolFunctionTakingInt )( int );

	// TODO: Rename, its name is Window but it takes an object named Widget...!!!!!
	/// \ingroup eventsSignatures
	/// Typedef of a member function to the original class taking const WidgetSizedEventResult & returning void
	typedef void ( EventHandlerClass::* itsVoidFunctionTakingWindowSizedEventResult )( const WidgetSizedEventResult & );

	/// \ingroup eventsSignatures
	/// Typedef of a member function to the original class taking const Point & returning void
	typedef void ( EventHandlerClass::* itsVoidFunctionTakingPoint )( const Point & );

	/// \ingroup eventsSignatures
	/// Typedef of a member function to the original class taking const CommandPtr & returning void
	typedef void ( EventHandlerClass::* itsVoidFunctionTakingCommand )( const CommandPtr & );

	/// \ingroup eventsSignatures
	/// Typedef of a member function to the original class taking const CreationalStruct & returning void
	typedef void ( EventHandlerClass::* itsVoidFunctionTakingSeedPointer )( const SmartWin::Seed & );

	/// \ingroup eventsSignatures
	/// Typedef of a member function to the original class taking const SmartUtil::tstring & returning bool
	typedef bool ( EventHandlerClass::* itsBoolFunctionTakingTstring )( const SmartUtil::tstring & );

	/// \ingroup eventsSignatures
	/// Typedef of a member function to the original class taking Canvas & returning void
	typedef void ( EventHandlerClass::* itsVoidFunctionTakingCanvas )( Canvas & );

	// Static/global event handlers definitions(here we need to have the "this"
	// pointer to the WidgetWindow that triggered the event)

	/// \ingroup eventsSignatures
	/// Typedef of a static/global function taking a pointer to the original class returning bool
	typedef bool ( * boolFunctionTakingVoid )( EventHandlerClass * );

	/// \ingroup eventsSignatures
	/// Typedef of a static/global function taking a pointer to the original class and a const MouseEventResult & returning void
	typedef void ( * voidFunctionTakingMouseEventResult )( EventHandlerClass *, const MouseEventResult & );

	/// \ingroup eventsSignatures
	/// Typedef of a static/global function taking a pointer to the original class returning void
	typedef void ( * voidFunctionTakingVoid )( EventHandlerClass * );

	/// \ingroup eventsSignatures
	/// Typedef of a static/global function taking a pointer to the original class and one bool returning void
	typedef void ( * voidFunctionTakingBool )( EventHandlerClass *, bool );

	/// \ingroup eventsSignatures
	/// Typedef of a static/global function taking a pointer to the original class, LPARAM and WPARAM returning HRESULT
	typedef HRESULT ( * hresultFunctionTakingLparamWparam )( EventHandlerClass *, LPARAM, WPARAM );

	/// \ingroup eventsSignatures
	/// Typedef of a static/global function taking a pointer to the original class and tstring-vector and Point returning void
	typedef void ( * voidFunctionTakingVectorPoint )( EventHandlerClass *, std::vector< SmartUtil::tstring>, Point );

	/// \ingroup eventsSignatures
	/// Typedef of a static/global function taking a pointer to the original class and one SmartUtil::tstring & returning void
	typedef void ( * voidFunctionTakingString )( EventHandlerClass *, SmartUtil::tstring & );

	/// \ingroup eventsSignatures
	/// Typedef of a static/global function taking a pointer to the original class and one SmartUtil::tstring & returning void
	typedef void ( * voidFunctionTakingConstString )( EventHandlerClass *, const SmartUtil::tstring & );

	/// \ingroup eventsSignatures
	/// Typedef of a static/global function taking a pointer to the original class and one int returning void
	typedef void ( * voidFunctionTakingInt )( EventHandlerClass *, int );

	/// \ingroup eventsSignatures
	/// Typedef of a static/global function taking a pointer to the original class and one int returning bool
	typedef bool ( * boolFunctionTakingInt )( EventHandlerClass *, int );

	/// \ingroup eventsSignatures
	/// Typedef of a static/global function taking a pointer to the original class and two bool returning void
	typedef void ( * voidFunctionTaking2Bool )( EventHandlerClass *, bool, bool );

	/// \ingroup eventsSignatures
	/// Typedef of a static/global function taking a pointer to the original class and a const WidgetSizedEventResult & returning void
	typedef void ( * voidFunctionTakingWindowSizedEventResult )( EventHandlerClass *, const WidgetSizedEventResult & );

	/// \ingroup eventsSignatures
	/// Typedef of a static/global function taking a pointer to the original class and a const Point & returning void
	typedef void ( * voidFunctionTakingPoint )( EventHandlerClass *, const Point & );

	/// \ingroup eventsSignatures
	/// Typedef of a static/global function taking a pointer to the original class and a const CommandPtr & returning void
	typedef void ( * voidFunctionTakingCommand )( EventHandlerClass *, const CommandPtr & );

	/// \ingroup eventsSignatures
	/// Typedef of a static/global function taking a pointer to the original class and a const CreationalStruct & returning void
	typedef void ( * voidFunctionTakingSeedPointer )( EventHandlerClass *, const SmartWin::Seed & );

	/// \ingroup eventsSignatures
	/// Typedef of a static/global function taking a pointer to the original class and a const SmartUtil::tstring & returning bool
	typedef bool ( * boolFunctionTakingTstring )( EventHandlerClass *, const SmartUtil::tstring & );

	/// \ingroup eventsSignatures
	/// Typedef of a static/global function taking a pointer to the original class and a Canvas & returning void
	typedef void ( * voidFunctionTakingCanvas )( EventHandlerClass *, Canvas & );

protected:
	// Note; SmartWin::Widget won't actually be initialized here because of the virtual inheritance
	MessageMap() : SmartWin::Widget(0)
	{}

	virtual ~MessageMap()
	{}

	virtual LRESULT sendWidgetMessage( HWND hWnd, UINT msg, WPARAM & wPar, LPARAM & lPar )
	{
		HRESULT retVal = 0;
		Message msgObj( hWnd, msg, wPar, lPar, true );
		if ( tryFire( msgObj, retVal ) )
		{
			wPar = msgObj.WParam;
			lPar = msgObj.LParam;
			return retVal;
		}

		// Then the stuff we HAVE to do something about...
		switch ( msg )
		{
			case WM_DESTROY :
			{
					if ( Application::instance().lastWidget( this ) )
					{
						::PostQuitMessage( EXIT_SUCCESS );
					}
#ifdef WINCE
				return MessageMapPolicy::kill();
#else
				return 0;
#endif
			} break;
#ifndef WINCE
			case WM_NCDESTROY :
			{
				return MessageMapPolicy::kill();
			}
#endif
			// Messages that's supposed to be dispatched to child Widgets!
			case WM_CTLCOLORSTATIC :
			case WM_CTLCOLORBTN :
			case WM_CTLCOLOREDIT :
			case WM_CTLCOLORLISTBOX :
			case WM_CTLCOLORSCROLLBAR :
			case WM_DRAWITEM :
			case WM_SYSCOMMAND :
			case WM_COMMAND :
			case WM_NOTIFY :
			case WM_HSCROLL :
			case WM_VSCROLL :
			case WM_MEASUREITEM :
			case WM_INITMENUPOPUP :
			{
				return mainWndRouteChild( hWnd, msg, wPar, lPar );
			}
			case WM_CLOSE :
				return MessageMapPolicy::returnFromCloseMsg( hWnd, msg, wPar, lPar );
		}

		// Nobody was interested in this one...
		return MessageMapPolicy::returnFromUnhandledWindowProc( hWnd, msg, wPar, lPar );
	}

private:
	// Routes a message from a "normal" window to one of its embedded control windows.
	virtual LRESULT mainWndRouteChild( HWND hWnd, UINT msg, WPARAM wPar, LPARAM lPar )
	{
		// Checking to see if this is a message to a menu
		if ( ( msg == WM_COMMAND && lPar == 0 && HIWORD( wPar ) == 0 ) || msg == WM_SYSCOMMAND || msg == WM_INITMENUPOPUP )
		{
			// We can have "user defined" sysmenu commands (by e.g. adding items to the sysmenu) but none will be
			// of higher or same value as SC_SIZE
			if ( WM_SYSCOMMAND == msg )
			{
				if ( wPar >= SC_SIZE )
					return MessageMapPolicy::returnFromUnhandledWindowProc( hWnd, msg, wPar, lPar );
			}
			// Is Menu or extended menu message!
			for ( std::list < Widget * >::iterator idx = private_::getApplicationWidgets().begin();
				idx != private_::getApplicationWidgets().end();
				++idx )
			{
				// TODO: Should we implement our own RTTI for uses such as this?
				// TODO: Should also become a specialization...
				// to make things go faster etc...
				WidgetMenu< EventHandlerClass, MessageMapPolicy > * ptr = dynamic_cast< WidgetMenu< EventHandlerClass, MessageMapPolicy > * >( * idx );
#ifndef WINCE
				WidgetMenuExtended< EventHandlerClass, MessageMapPolicy > * extPtr = dynamic_cast< WidgetMenuExtended< EventHandlerClass, MessageMapPolicy > * >( * idx );
#endif
				if ( ( ptr )
#ifndef WINCE
					|| ( extPtr )
#endif
					)
				{
					if( msg == WM_INITMENUPOPUP && extPtr )
					{
						HMENU handle = reinterpret_cast< HMENU >( extPtr->handle() );
						if( handle != (HMENU) wPar )
							continue;
					}
					// function returns 1 of message was handled...
					// Need to iterate ALL menus until Event Handler is found...
					LRESULT retVal = ( * idx )->sendWidgetMessage( hWnd, msg, wPar, lPar );
					if ( retVal != 0 )
					{
						return retVal;
					}
				}
			}
		}
		else
		{
			for ( std::list < Widget * >::iterator idx = private_::getApplicationWidgets().begin();
				idx != private_::getApplicationWidgets().end();
				++idx )
			{
				// BUG in compiler (need to dereference iterator to pointer)
				if ( msg == WM_NOTIFY && ( * idx )->handle() == ( ( NMHDR * ) lPar )->hwndFrom )
				{
					return ( * idx )->sendWidgetMessage( hWnd, msg, wPar, lPar );
				}
				else if ( msg == WM_COMMAND && ( * idx )->handle() == ( HANDLE ) lPar )
				{
					return ( * idx )->sendWidgetMessage( hWnd, msg, wPar, lPar );
				}
				else if ( ( msg == WM_VSCROLL || msg == WM_HSCROLL ) && ( * idx )->handle() == ( HANDLE ) lPar )
				{
					int scrollReq = ( wPar & 0xf ); // SB_THUMBPOSITION(4) or SB_ENDSCROLL(8)
					if ( SB_ENDSCROLL == scrollReq )
					{
						return MessageMapPolicy::returnFromUnhandledWindowProc( hWnd, msg, wPar, lPar );
					}
					else
					{
						return ( * idx )->sendWidgetMessage( hWnd, msg, wPar, lPar );
					}
				}
				else if ( msg == WM_DRAWITEM && ( * idx )->handle() == reinterpret_cast< DRAWITEMSTRUCT * >( lPar )->hwndItem )
				{
				#ifndef WINCE
					// Trick to fix BUG #1598489
					// During Fast-User-Switching ::GetForegroundWindows() returns NULL,
					// And we should not try to draw on locked DC
					if (::GetForegroundWindow() != NULL)
				#endif
						return ( * idx )->sendWidgetMessage( hWnd, msg, wPar, lPar );
				}
				else if (
					( msg == WM_CTLCOLORSTATIC ||
					  msg == WM_CTLCOLORBTN ||
					  msg == WM_CTLCOLOREDIT ||
					  msg == WM_CTLCOLORLISTBOX ||
					  msg == WM_CTLCOLORSCROLLBAR )
					 && ( * idx )->handle() == reinterpret_cast< HANDLE >( lPar ) )
				{
					return ( * idx )->sendWidgetMessage( hWnd, msg, wPar, lPar );
				}
#ifndef WINCE
				else if ( msg == WM_MEASUREITEM )
				{
					WidgetMenuExtended< EventHandlerClass, MessageMapPolicy > * menuPtr = dynamic_cast< WidgetMenuExtended< EventHandlerClass, MessageMapPolicy > * >( * idx );

					if ( menuPtr != 0 )   // if processing extended menu
					{
						// check to see if the item belongs to this menu
						HMENU handle = reinterpret_cast< HMENU >( menuPtr->handle() );

						// get message data
						MEASUREITEMSTRUCT * ptr = reinterpret_cast< MEASUREITEMSTRUCT * >( lPar );

						// get item data
						private_::ItemDataWrapper * data = reinterpret_cast< private_::ItemDataWrapper * >( ptr->itemData );

						if ( ( data != 0 ) && ( data->menu == handle ) ) // if item belongs to current menu
							return ( * idx )->sendWidgetMessage( hWnd, msg, wPar, lPar );
					}
				}
#endif
			}
		}

		// Nobody was interested in this one...
		return MessageMapPolicy::returnFromUnhandledWindowProc( hWnd, msg, wPar, lPar );
	}
};

// end namespace SmartWin
}

#endif

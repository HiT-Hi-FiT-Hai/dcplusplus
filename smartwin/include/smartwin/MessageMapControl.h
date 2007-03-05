// $Revision: 1.20 $
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
#ifndef MessageMapControl_h
#define MessageMapControl_h

#include "boost.h"
#include "WindowsHeaders.h"
#include "MessageMapPolicyClasses.h"
#include "Widget.h"
#include "BasicTypes.h"
#include "Message.h"
#include "DestructionClass.h"
#include "MessageMapBase.h"
#include "CanvasClasses.h"
#include "SignalParams.h"
#include "SigSlots.h"

namespace SmartWin
{
// begin namespace SmartWin

/// The "fallback" WndMsgProc handler
/** Is the "default" message handler class for all Control Widgets. (those which does
  * NOT derive from WidgetWindowBase) <br>
  * If a message is not handled by anything else it goes into this class to check if
  * we should handle this message or not. <br>
  * If the message is NOT handled it is returned to windows OS for handling. <br>
  * Shouldn't be of much interest directly to a user of SmartWin unless derived from
  * to make your own message dispatching logic, and even that's easy to achieve
  * WITHOUT fiddling with this class by using AspectRaw::onRaw Event Handler
  * Setters...
  */
template< class EventHandlerClass, class WidgetType, class MessageMapPolicy >
class MessageMapControl
	: public virtual Widget,
	public MessageMapPolicy,
	public MessageMapBase< boost::tuple< private_::SignalContent, SigSlot::Signal< HRESULT, private_::SignalContent > >,
		std::vector< boost::tuple < private_::SignalContent, SigSlot::Signal< HRESULT, private_::SignalContent > > > >
{
public:
	typedef SigSlot::Signal< HRESULT, private_::SignalContent > SignalType;
	typedef boost::tuple< private_::SignalContent, SignalType > SignalTupleType;
	typedef std::vector< SignalTupleType > SignalCollection;
	const static bool IsControl = true;

	// Member event handler definitions (here you have the "this" pointer so
	// there's no need for passing the EventHandlerClass *). In opposition of the
	// MessageMap class all of these function signatures contains a pointer to the
	// this Widget

	/// \ingroup eventsSignatures
	/// Typedef of a member function to the original class taking pointer to the this Widget and unsigned and returning bool
	typedef bool ( EventHandlerClass::* itsBoolFunctionTakingUnsigned )( WidgetType *, unsigned );

	/// \ingroup eventsSignatures
	/// Typedef of a member function to the original class taking pointer to the this Widget returning void
	typedef void ( EventHandlerClass::* itsVoidFunctionTakingVoid )( WidgetType * );

	/// \ingroup eventsSignatures
	/// Typedef of a member function to the original class taking pointer to the this Widget and an unsigned int returning void
	typedef void ( EventHandlerClass::* itsVoidFunctionTakingUInt )( WidgetType *, unsigned );

	/// \ingroup eventsSignatures
	/// Typedef of a member function to the original class taking pointer to the this Widget and a SmartUtil::tstring & returning void
	typedef void ( EventHandlerClass::* itsVoidFunctionTakingString )( WidgetType *, SmartUtil::tstring & );

	/// \ingroup eventsSignatures
	/// Typedef of a member function to the original class taking pointer to the this Widget and a SmartUtil::tstring & returning void
	typedef void ( EventHandlerClass::* itsVoidFunctionTakingConstString )( WidgetType *, const SmartUtil::tstring & );

	/// \ingroup eventsSignatures
	/// Typedef of a member function to the original class taking pointer to the this Widget and a const MouseEventResult & returning void
	typedef void ( EventHandlerClass::* itsVoidFunctionTakingMouseEventResult )( WidgetType *, const MouseEventResult & );

	/// \ingroup eventsSignatures
	/// Typedef of a member function to the original class taking pointer to the this Widget and two bool returning void
	typedef void ( EventHandlerClass::* itsVoidFunctionTaking2Bool )( WidgetType *, bool, bool );

	/// \ingroup eventsSignatures
	/// Typedef of a member function to the original class taking pointer to the this Widget and a bool returning void
	typedef void ( EventHandlerClass::* itsVoidFunctionTakingBool )( WidgetType *, bool );

	/// \ingroup eventsSignatures
	/// Typedef of a member function to the original class taking pointer to the this Widget, an LPARAM and a WPARAM returning HRESULT
	typedef HRESULT ( EventHandlerClass::* itsHresultFunctionTakingLparamWparam )( WidgetType *, LPARAM, WPARAM );

	/// \ingroup eventsSignatures
	/// Typedef of a member function to the original class taking pointer to the this Widget and const WidgetSizedEventResult & returning void
	typedef void ( EventHandlerClass::* itsVoidFunctionTakingWindowSizedEventResult )( WidgetType *, const WidgetSizedEventResult & );

	/// \ingroup eventsSignatures
	/// Typedef of a member function to the original class taking pointer to the this Widget and const Point & returning void
	typedef void ( EventHandlerClass::* itsVoidFunctionTakingPoint )( WidgetType *, const Point & );

	/// \ingroup eventsSignatures
	/// Typedef of a member function to the original class taking pointer to the this Widget and an int returning void
	typedef void ( EventHandlerClass::* itsVoidFunctionTakingInt )( WidgetType *, int );

	/// \ingroup eventsSignatures
	/// Typedef of a member function to the original class taking pointer to the this Widget and an int returning bool
	typedef bool ( EventHandlerClass::* itsBoolFunctionTakingInt )( WidgetType *, int );

	/// \ingroup eventsSignatures
	/// Typedef of a member function to the original class taking pointer to the this Widget and a const SYSTEMTIME & returning void
	typedef void ( EventHandlerClass::* itsVoidFunctionTakingSystemTime )( WidgetType *, const SYSTEMTIME & );

	/// \ingroup eventsSignatures
	/// Typedef of a member function to the original class taking pointer to the this Widget, two unsigned int and SmartUtil::tstring & returning bool
	typedef bool ( EventHandlerClass::* itsBoolValidationFunc )( WidgetType *, unsigned, unsigned, SmartUtil::tstring & );

	/// \ingroup eventsSignatures
	/// Typedef of a member function to the original class taking pointer to the this Widget, two unsigned int and SmartUtil::tstring & returning void
	typedef void ( EventHandlerClass::* itsVoidGetItemFunc )( WidgetType *, LPARAM, unsigned, unsigned, SmartUtil::tstring & );

	/// \ingroup eventsSignatures
	/// Typedef of a member function to the original class taking pointer to the this Widget, an unsigned int and an int & returning void
	typedef void ( EventHandlerClass::* itsVoidGetIconFunc )( WidgetType *, LPARAM, unsigned, int & );

	/// \ingroup eventsSignatures
	/// Typedef of a member function to the original class taking pointer to the this Widget, two unsigned int, a Canvas & and a const Rectangle & returning void
	typedef void ( EventHandlerClass::* itsVoidUnsignedUnsignedBoolCanvasRectangle )( WidgetType *, unsigned, unsigned, bool, Canvas &, const SmartWin::Rectangle & );

	/// \ingroup eventsSignatures
	/// Typedef of a member function to the original class taking pointer to the this Widget, two LPARAM returning int
	typedef int ( EventHandlerClass::* itsIntLparamLparam )( WidgetType *, LPARAM, LPARAM );

	/// \ingroup eventsSignatures
	/// Typedef of a member function to the original class taking pointer to the this Widget and a Canvas & returning BrushPtr
	typedef BrushPtr ( EventHandlerClass::* itsBrushFunctionTakingCanvas )( WidgetType *, Canvas & );

	/// \ingroup eventsSignatures
	/// Typedef of a member function to the original class taking pointer to the this Widget and a const SmartUtil::tstring & returning bool
	typedef bool ( EventHandlerClass::* itsBoolFunctionTakingTstring )( WidgetType *, const SmartUtil::tstring & );

	/// \ingroup eventsSignatures
	/// Typedef of a member function to the original class taking pointer to the this Widget and a Canvas & returning void
	typedef void ( EventHandlerClass::* itsVoidFunctionTakingCanvas )( WidgetType *, Canvas & );

	/// \ingroup eventsSignatures
	/// Typedef of a member function to the original class taking pointer to the this Widget and a const DRAWITEMSTRUCT & returning void
	typedef void ( EventHandlerClass::* itsVoidDrawItemFunction ) ( WidgetType *, const DRAWITEMSTRUCT & );

	/// \ingroup eventsSignatures
	/// Typedef of a member function to the original class taking pointer to the this Widget and a MEASUREITEMSTRUCT * returning void
	typedef void ( EventHandlerClass::* itsVoidMeasureItemFunction ) ( WidgetType *, MEASUREITEMSTRUCT * );

	// Static/global event handlers definitions(here we need to have the "this"
	// pointer to the WidgetWindow that triggered the event)

	/// \ingroup eventsSignatures
	///  Typedef of a static/global function taking a pointer to the original class and a pointer to the this Widget class pluss an unsigned int returning void
	typedef bool ( * boolFunctionTakingUnsigned )( EventHandlerClass *, WidgetType *, unsigned );

	/// \ingroup eventsSignatures
	///  Typedef of a static/global function taking a pointer to the original class and a pointer to the this Widget class returning void
	typedef void ( * voidFunctionTakingVoid )( EventHandlerClass *, WidgetType * );

	/// \ingroup eventsSignatures
	///  Typedef of a static/global function taking a pointer to the original class, a pointer to the this Widget class and a SmartUtil::tstring & returning void
	typedef void ( * voidFunctionTakingString )( EventHandlerClass *, WidgetType *, SmartUtil::tstring & );

	/// \ingroup eventsSignatures
	///  Typedef of a static/global function taking a pointer to the original class, a pointer to the this Widget class and a SmartUtil::tstring & returning void
	typedef void ( * voidFunctionTakingConstString )( EventHandlerClass *, WidgetType *, const SmartUtil::tstring & );

	/// \ingroup eventsSignatures
	///  Typedef of a static/global function taking a pointer to the original class, a pointer to the this Widget class and an unsigned int returning void
	typedef void ( * voidFunctionTakingUInt )( EventHandlerClass *, WidgetType *, unsigned );

	/// \ingroup eventsSignatures
	///  Typedef of a static/global function taking a pointer to the original class, a pointer to the this Widget class and a const MouseEventResult & returning void
	typedef void ( * voidFunctionTakingMouseEventResult )( EventHandlerClass *, WidgetType *, const MouseEventResult & );

	/// \ingroup eventsSignatures
	///  Typedef of a static/global function taking a pointer to the original class, a pointer to the this Widget class and two bool returning void
	typedef void ( * voidFunctionTaking2Bool )( EventHandlerClass *, WidgetType *, bool, bool );

	/// \ingroup eventsSignatures
	///  Typedef of a static/global function taking a pointer to the original class, a pointer to the this Widget class and a bool returning void
	typedef void ( * voidFunctionTakingBool )( EventHandlerClass *, WidgetType *, bool );

	/// \ingroup eventsSignatures
	///  Typedef of a static/global function taking a pointer to the original class, a pointer to the this Widget, an LPARAM and a WPARAM returning HRESULT
	typedef HRESULT ( * hresultFunctionTakingLparamWparam )( EventHandlerClass *, WidgetType *, LPARAM, WPARAM );

	/// \ingroup eventsSignatures
	///  Typedef of a static/global function taking a pointer to the original class, a pointer to the this Widget class and a const WidgetSizedEventResult & returning void
	typedef void ( * voidFunctionTakingWindowSizedEventResult )( EventHandlerClass *, WidgetType *, const WidgetSizedEventResult & );

	/// \ingroup eventsSignatures
	///  Typedef of a static/global function taking a pointer to the original class, a pointer to the this Widget class and a const Point & returning void
	typedef void ( * voidFunctionTakingPoint )( EventHandlerClass *, WidgetType *, const Point & );

	/// \ingroup eventsSignatures
	///  Typedef of a static/global function taking a pointer to the original class, a pointer to the this Widget class and an int returning void
	typedef void ( * voidFunctionTakingInt )( EventHandlerClass *, WidgetType *, int );

	/// \ingroup eventsSignatures
	///  Typedef of a static/global function taking a pointer to the original class, a pointer to the this Widget class and an int returning bool
	typedef bool ( * boolFunctionTakingInt )( EventHandlerClass *, WidgetType *, int );

	/// \ingroup eventsSignatures
	///  Typedef of a static/global function taking a pointer to the original class, a pointer to the this Widget class and a const SYSTEMTIME & returning void
	typedef void ( * voidFunctionTakingSystemTime )( EventHandlerClass *, WidgetType *, const SYSTEMTIME & );

	/// \ingroup eventsSignatures
	///  Typedef of a static/global function taking a pointer to the original class, a pointer to the this Widget class, two unsigned int and a SmartUtil::tstring & returning bool
	typedef bool ( * boolValidationFunc )( EventHandlerClass *, WidgetType *, unsigned, unsigned, SmartUtil::tstring & );

	/// \ingroup eventsSignatures
	///  Typedef of a static/global function taking a pointer to the original class, a pointer to the this Widget class, an LPARAM, two unsigned int and a SmartUtil::tstring & returning void
	typedef void ( * voidGetItemFunc )( EventHandlerClass *, WidgetType *, LPARAM, unsigned, unsigned, int & );

	/// \ingroup eventsSignatures
	///  Typedef of a static/global function taking a pointer to the original class, a pointer to the this Widget class, an LPARAM, an unsigned int and an int & returning void
	typedef void ( * voidGetIconFunc )( EventHandlerClass *, WidgetType *, LPARAM, unsigned, int & );

	/// \ingroup eventsSignatures
	///  Typedef of a static/global function taking a pointer to the original class, a pointer to the this Widget class, two unsigned int, a bool, a Canvas & and a const Rectangle & returning void
	typedef void ( * voidUnsignedUnsignedBoolCanvasRectangle )( EventHandlerClass *, WidgetType *, unsigned, unsigned, bool, Canvas &, const SmartWin::Rectangle & );

	/// \ingroup eventsSignatures
	///  Typedef of a static/global function taking a pointer to the original class, a pointer to the this Widget class and two LPARAM returning int
	typedef int ( * intCallbackCompareFunc )( EventHandlerClass *, WidgetType *, LPARAM, LPARAM );

	/// \ingroup eventsSignatures
	///  Typedef of a static/global function taking a pointer to the original class and a pointer to the this Widget class and a Canvas & returning BrushPtr
	typedef BrushPtr ( * brushFunctionTakingCanvas )( EventHandlerClass *, WidgetType *, Canvas & canvas );

	/// \ingroup eventsSignatures
	///  Typedef of a static/global function taking a pointer to the original class and a pointer to the this Widget class and a const SmartUtil::tstring & returning bool
	typedef bool ( * boolFunctionTakingTstring )( EventHandlerClass *, WidgetType *, const SmartUtil::tstring & );

	/// \ingroup eventsSignatures
	///  Typedef of a static/global function taking a pointer to the original class and a pointer to the this Widget class and a Canvas & returning void
	typedef void ( * voidFunctionTakingCanvas )( EventHandlerClass *, WidgetType *, Canvas & );

	/// \ingroup eventsSignatures
	///  Typedef of a static/global function taking a pointer to the original class and a pointer to the this Widget class and a const DRAWITEMSTRUCT & returning void
	typedef void ( * voidDrawItemFunction ) ( EventHandlerClass *, WidgetType *, const DRAWITEMSTRUCT & );

	/// \ingroup eventsSignatures
	///  Typedef of a static/global function taking a pointer to the original class and a pointer to the this Widget class and a MEASUREITEMSTRUCT * returning void
	typedef void ( * voidMeasureItemFunction ) ( EventHandlerClass *, WidgetType *, MEASUREITEMSTRUCT * );

protected:
	// Function pointer to the default (windows) WndMsgProc
	WNDPROC itsDefaultWindowProc;

	// False if not control window (normal window), true if control is a subclassed dialog item
	// TODO: Aspect class or maybe inherit WidgetSpliter from MessageMap instead of MessageMap?!?!?
	bool isSubclassed;

	MessageMapControl()
		: itsDefaultWindowProc( 0 ),
		isSubclassed( true )
	{}

	virtual ~MessageMapControl()
	{}

	// Main Window Message Procedure.
	// This is the fallback event handler checker, the final line before message is
	// (eventually) passed on to windows
	virtual LRESULT sendWidgetMessage( HWND hWnd, UINT msg, WPARAM & wPar, LPARAM & lPar )
	{
		HRESULT retVal;
		if ( tryFire( Message( hWnd, msg, wPar, lPar, true ), retVal ) )
		{
			return retVal; // If the message was handled ...
		}

		// Either there was no handler for the msg or the handler choose not to
		// handle it. The KeyPress handler can choose to not handle some keys, for
		// example.
		switch ( msg )
		{
			// First the stuff we HAVE to do something about...
#ifdef WINCE
			case WM_DESTROY :
#else
			case WM_NCDESTROY :
#endif
			{
				return MessageMapPolicy::kill();
			}
		}

		// Nobody was interested in this one...
		// TODO: Make this policy some way or another...!!
		if ( this->isSubclassed && itsDefaultWindowProc != 0 )
		{
			// If control is subclassed we need to dispatch message to the original Windows Message Procedure
			return ::CallWindowProc( itsDefaultWindowProc, hWnd, msg, wPar, lPar );
		}
		else
		{
			// otherwise we can call the default Windows Message Procedure
			return MessageMapPolicy::returnFromUnhandledWindowProc( hWnd, msg, wPar, lPar );
		}
	}

	/// Call this function from your overridden create() if you add a new Widget to
	/// make the Windows Message Procedure dispatching map right.
	void createMessageMap()
	{
		::SetProp( itsHandle, _T( "_mainWndProc" ), reinterpret_cast< HANDLE >( dynamic_cast< Widget * >( this ) ) );
		itsDefaultWindowProc = reinterpret_cast< WNDPROC >( ::SetWindowLongPtr( itsHandle, GWL_WNDPROC, ( LONG_PTR ) this->mainWndProc_ ) );
	}
};

// end namespace SmartWin
}

#endif

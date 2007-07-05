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

#include "WindowsHeaders.h"
#include "Application.h"
#include "Widget.h"
#include "BasicTypes.h"
#include "Message.h"
#include "Command.h"
#include "CanvasClasses.h"
#include <vector>

namespace SmartWin
{
// begin namespace SmartWin

/// Legacy support class
template< class EventHandlerClass >
class MessageMap
{
public:
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

private:
	// We no longer instantiate this
	MessageMap();
	~MessageMap();
};

// end namespace SmartWin
}

#endif

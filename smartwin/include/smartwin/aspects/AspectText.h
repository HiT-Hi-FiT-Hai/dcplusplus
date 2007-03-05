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
#ifndef AspectText_h
#define AspectText_h

#include "boost.h"
#include "SmartUtil.h"
#include "../SignalParams.h"

namespace SmartWin
{
// begin namespace SmartWin

// Dispatcher class with specializations for dispatching event to event handlers of
// the AspectText Since AspectText is used both in WidgetWindowBase (container
// widgets) and Control Widgets we need to specialize which implementation to use
// here!!
template< class EventHandlerClass, class WidgetType, class MessageMapType, bool IsControl >
class AspectTextDispatcher
{
};

template< class EventHandlerClass, class WidgetType, class MessageMapType >
class AspectTextDispatcher< EventHandlerClass, WidgetType, MessageMapType, true >
{
public:
	static HRESULT dispatch( private_::SignalContent & params )
	{
		typename MessageMapType::voidFunctionTakingConstString func =
			reinterpret_cast< typename MessageMapType::voidFunctionTakingConstString >( params.Function );

		EventHandlerClass * ThisParent = internal_::getTypedParentOrThrow < EventHandlerClass * >( params.This );
		WidgetType * This = boost::polymorphic_cast< WidgetType * >( params.This );

		func( ThisParent,
			This,
			SmartUtil::tstring( reinterpret_cast< TCHAR * >( params.Msg.LParam ) )
			);

		params.RunDefaultHandling = true;
		return 0;
	}

	static HRESULT dispatchThis( private_::SignalContent & params )
	{
		typename MessageMapType::itsVoidFunctionTakingConstString func =
			reinterpret_cast< typename MessageMapType::itsVoidFunctionTakingConstString >( params.FunctionThis );

		EventHandlerClass * ThisParent = internal_::getTypedParentOrThrow < EventHandlerClass * >( params.This );
		WidgetType * This = boost::polymorphic_cast< WidgetType * >( params.This );

		( ( * ThisParent ).*func )(
			This,
			SmartUtil::tstring( reinterpret_cast< TCHAR * >( params.Msg.LParam ) )
			);

		params.RunDefaultHandling = true;
		return 0;
	}
};

template< class EventHandlerClass, class WidgetType, class MessageMapType >
class AspectTextDispatcher< EventHandlerClass, WidgetType, MessageMapType, false >
{
public:
	static HRESULT dispatch( private_::SignalContent & params )
	{
		typename MessageMapType::voidFunctionTakingConstString func =
			reinterpret_cast< typename MessageMapType::voidFunctionTakingConstString >( params.Function );

		EventHandlerClass * ThisParent = internal_::getTypedParentOrThrow < EventHandlerClass * >( params.This );

		func(
			ThisParent,
			SmartUtil::tstring( reinterpret_cast< TCHAR * >( params.Msg.LParam ) )
			);

		params.RunDefaultHandling = true;
		return ThisParent->returnFromHandledWindowProc( reinterpret_cast< HWND >( params.Msg.Handle ), params.Msg.Msg, params.Msg.WParam, params.Msg.LParam );
	}

	static HRESULT dispatchThis( private_::SignalContent & params )
	{
		typename MessageMapType::itsVoidFunctionTakingConstString func =
			reinterpret_cast< typename MessageMapType::itsVoidFunctionTakingConstString >( params.FunctionThis );

		EventHandlerClass * ThisParent = internal_::getTypedParentOrThrow < EventHandlerClass * >( params.This );

		( ( * ThisParent ).*func )(
			SmartUtil::tstring( reinterpret_cast< TCHAR * >( params.Msg.LParam ) )
			);

		params.RunDefaultHandling = true;
		return ThisParent->returnFromHandledWindowProc( reinterpret_cast< HWND >( params.Msg.Handle ), params.Msg.Msg, params.Msg.WParam, params.Msg.LParam );
	}
};

/// Aspect class used by Widgets that have the possibility of setting the "text"
/// property of their objects.
/** \ingroup AspectClasses
  * E.g. the AspectTextBox have a "text" Aspect therefore they realize the AspectText
  * through inheritance.
  */
template< class EventHandlerClass, class WidgetType, class MessageMapType >
class AspectText
{
	typedef AspectTextDispatcher< EventHandlerClass, WidgetType, MessageMapType, MessageMapType::IsControl > Dispatcher;
public:
	/// Sets the text of the AspectText realizing class
	/** The txt parameter is the new text to put into the realizing object.
	  */
	void setText( const SmartUtil::tstring & txt );

	/// Sets the text in the Edit Control so that endl causes a new line.
	/** Just the same as setText except that CR are expanded to LF CR
	  * Replaces \n with \r\n so that Windows textbox understands "endl"
	  */
	void setTextLines( const SmartUtil::tstring & txt );

	/// Returns a string in which \n is replaced with with \r\n
	/** The purpose is to enable Windows textboxs to understand "endl"
	  */
	SmartUtil::tstring replaceEndlWithLfCr( const SmartUtil::tstring & txt );

	/// Gets the text of the AspectText realizing class
	/** The Return value is the text of the realizing class.
	  */
	SmartUtil::tstring getText() const;

	/// \ingroup EventHandlersAspectText
	/// Setting the event handler for the "setText" event
	/** When the text changes in the Widget this event will be raised. <br>
	  * The parameter passed is SmartUtil::tstring & which is the new text of the
	  * Widget.
	  */
	void onTextChanging( typename MessageMapType::itsVoidFunctionTakingConstString eventHandler );
	void onTextChanging( typename MessageMapType::voidFunctionTakingConstString eventHandler );

protected:
	virtual ~AspectText()
	{}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template< class EventHandlerClass, class WidgetType, class MessageMapType >
void AspectText< EventHandlerClass, WidgetType, MessageMapType >::setText( const SmartUtil::tstring & txt )
{
	::SendMessage( static_cast< WidgetType * >( this )->handle(), WM_SETTEXT, ( WPARAM ) 0, ( LPARAM ) txt.c_str() );
}


template< class EventHandlerClass, class WidgetType, class MessageMapType >
SmartUtil::tstring AspectText< EventHandlerClass, WidgetType, MessageMapType >::replaceEndlWithLfCr( const SmartUtil::tstring & txt )
{
	// Replaces \n with \r\n so that Windows textbox understands "endl"
	SmartUtil::tstring	txtEndl= txt;

	std::string::size_type	pos= txtEndl.find( '\n', 0 );
	while ( std::string::npos != pos ) { 
		txtEndl.replace( pos, 1, _T("\r\n") );
		pos += 2;	// Don't find the replacement \n.
		pos= txtEndl.find( '\n', pos );
	}  
	return txtEndl;
}

template< class EventHandlerClass, class WidgetType, class MessageMapType >
void AspectText< EventHandlerClass, WidgetType, MessageMapType >::setTextLines( const SmartUtil::tstring & inTxt )
{
	setText( replaceEndlWithLfCr( inTxt ) );
}



template< class EventHandlerClass, class WidgetType, class MessageMapType >
SmartUtil::tstring AspectText< EventHandlerClass, WidgetType, MessageMapType >::getText() const
{
	LRESULT textLength = ::SendMessage( static_cast< const WidgetType * >( this )->handle(), WM_GETTEXTLENGTH, 0, 0 );
	if ( textLength == 0 )
		return _T( "" );
	boost::scoped_array< TCHAR > txt( new TCHAR[++textLength] );
	::SendMessage( static_cast< const WidgetType * >( this )->handle(), WM_GETTEXT, ( WPARAM ) textLength, ( LPARAM ) txt.get() );
	SmartUtil::tstring retVal = txt.get();
	return retVal;
}

template< class EventHandlerClass, class WidgetType, class MessageMapType >
void AspectText< EventHandlerClass, WidgetType, MessageMapType >::onTextChanging( typename MessageMapType::itsVoidFunctionTakingConstString eventHandler )
{
	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->addNewSignal(
		typename MessageMapType::SignalTupleType(
			private_::SignalContent(
				Message( WM_SETTEXT ),
				reinterpret_cast< itsVoidFunction >( eventHandler ),
				ptrThis
			),
			typename MessageMapType::SignalType( typename MessageMapType::SignalType::SlotType( & AspectText::Dispatcher::dispatchThis ) )
		)
	);
}

template< class EventHandlerClass, class WidgetType, class MessageMapType >
void AspectText< EventHandlerClass, WidgetType, MessageMapType >::onTextChanging( typename MessageMapType::voidFunctionTakingConstString eventHandler )
{
	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->addNewSignal(
		typename MessageMapType::SignalTupleType(
			private_::SignalContent(
				Message( WM_SETTEXT ),
				reinterpret_cast< private_::SignalContent::voidFunctionTakingVoid >( eventHandler ),
				ptrThis
			),
			typename MessageMapType::SignalType(
				MessageMapType::SignalType::SlotType( & AspectText::Dispatcher::dispatch )
			)
		)
	);
}

// end namespace SmartWin
}

#endif

// $Revision: 1.4 $
/*
  Copyright (c) 2006, Thomas Hansen
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
#ifndef AspectDragDrop_h
#define AspectDragDrop_h

#ifndef WINCE // Not supported on WINCE platform

#include "boost.h"
#include "../SignalParams.h"
#include <vector>
#include <shellapi.h>

namespace SmartWin
{
// begin namespace SmartWin

// Dispatcher class with specializations for dispatching event to event handlers of
// the AspectDragDrop
template< class EventHandlerClass, class WidgetType, class MessageMapType, bool IsControl >
class AspectDragDropDispatcher
{
};

template< class EventHandlerClass, class WidgetType, class MessageMapType >
class AspectDragDropDispatcher<EventHandlerClass, WidgetType, MessageMapType, true/*Control Widget*/>
{
public:
	static HRESULT dispatch( private_::SignalContent & params )
	{
		typename MessageMapType::voidFunctionTakingVectorPoint func =
			reinterpret_cast< typename MessageMapType::voidFunctionTakingVectorPoint >( params.Function );

		EventHandlerClass * ThisParent = internal_::getTypedParentOrThrow < EventHandlerClass * >( params.This );
		WidgetType * This = boost::polymorphic_cast< WidgetType * >( params.This );
		std::vector<SmartUtil::tstring> f;
		Point pt;
		HDROP handle = (HDROP)params.Msg.WParam;
		if (handle) { 
			int iFiles = DragQueryFile(handle, (UINT)-1, NULL, 0);
			TCHAR pFilename[MAX_PATH];
			for(int i=0;i<iFiles;i++) {
				memset(pFilename,0,MAX_PATH * sizeof(TCHAR));
				DragQueryFile(handle, i, pFilename, MAX_PATH);
				f.push_back(pFilename);
			}
			POINT p;
			DragQueryPoint(handle,&p);
			pt = Point(p.x,p.y);
			DragFinish(handle); 
		}
		handle = 0;
		func( ThisParent,
			This,
			f,pt
			);

		params.RunDefaultHandling = true;
		return 0;
	}

	static HRESULT dispatchThis( private_::SignalContent & params )
	{
		typename MessageMapType::itsVoidFunctionTakingVectorPoint func =
			reinterpret_cast< typename MessageMapType::itsVoidFunctionTakingVectorPoint >( params.FunctionThis );

		EventHandlerClass * ThisParent = internal_::getTypedParentOrThrow < EventHandlerClass * >( params.This );
		WidgetType * This = boost::polymorphic_cast< WidgetType * >( params.This );
		std::vector<SmartUtil::tstring> f;
		Point pt;
		HDROP handle = (HDROP)params.Msg.WParam;
		if (handle) { 
			int iFiles = DragQueryFile(handle, (UINT)-1, NULL, 0);
			TCHAR pFilename[MAX_PATH];
			for(int i=0;i<iFiles;i++) {
				memset(pFilename,0,MAX_PATH * sizeof(TCHAR));
				DragQueryFile(handle, i, pFilename, MAX_PATH);
				f.push_back(pFilename);
			}
			POINT p;
			DragQueryPoint(handle,&p);
			pt = Point(p.x,p.y);
			DragFinish(handle); 
		}
		handle = 0;
		( ( * ThisParent ).*func )(
			This,
			f,pt
			);

		params.RunDefaultHandling = true;
		return 0;
	}
};

template< class EventHandlerClass, class WidgetType, class MessageMapType >
class AspectDragDropDispatcher<EventHandlerClass, WidgetType, MessageMapType, false/*Container Widget*/>
{
public:
	static HRESULT dispatch( private_::SignalContent & params )
	{
		typename MessageMapType::boolFunctionTakingInt func =
			reinterpret_cast< typename MessageMapType::voidFunctionTakingVectorPoint >( params.Function );

		EventHandlerClass * ThisParent = internal_::getTypedParentOrThrow < EventHandlerClass * >( params.This );
		std::vector<SmartUtil::tstring> f;
		Point pt;
		HDROP handle = (HDROP)params.Msg.WParam;
		if (handle) { 
			int iFiles = DragQueryFile(handle, (UINT)-1, NULL, 0);
			TCHAR pFilename[MAX_PATH];
			for(int i=0;i<iFiles;i++) {
				memset(pFilename,0,MAX_PATH * sizeof(TCHAR));
				DragQueryFile(handle, i, pFilename, MAX_PATH);
				f.push_back(pFilename);
			}
			POINT p;
			DragQueryPoint(handle,&p);
			pt = Point(p.x,p.y);
			DragFinish(handle); 
		}
		handle = 0;
		func(
			ThisParent,
			f,pt
			);

		params.RunDefaultHandling = true;
		return 0;
	}

	static HRESULT dispatchThis( private_::SignalContent & params )
	{
		typename MessageMapType::itsVoidFunctionTakingVectorPoint func =
			reinterpret_cast< typename MessageMapType::itsVoidFunctionTakingVectorPoint >( params.FunctionThis );

		EventHandlerClass * ThisParent = internal_::getTypedParentOrThrow < EventHandlerClass * >( params.This );
		std::vector<SmartUtil::tstring> f;
		Point pt;
		HDROP handle = (HDROP)params.Msg.WParam;
		if (handle) { 
			int iFiles = DragQueryFile(handle, (UINT)-1, NULL, 0);
			TCHAR pFilename[MAX_PATH];
			for(int i=0;i<iFiles;i++) {
				memset(pFilename,0,MAX_PATH * sizeof(TCHAR));
				DragQueryFile(handle, i, pFilename, MAX_PATH);
				f.push_back(pFilename);
			}
			POINT p;
			DragQueryPoint(handle,&p);
			pt = Point(p.x,p.y);
			DragFinish(handle); 
		}
		handle = 0;
		( ( * ThisParent ).*func )(
			f,pt
			);

		params.RunDefaultHandling = true;
		return 0;
	}
};

/// Aspect class used by dialog Widgets that have the possibility of trapping "drop files events".
/** \ingroup AspectClasses
  * E.g. the WidgetModalDialog can trap "drop files events" therefore they realize the AspectDragDrop through inheritance.
  */
template< class EventHandlerClass, class WidgetType, class MessageMapType >
class AspectDragDrop
{
	typedef AspectDragDropDispatcher< EventHandlerClass, WidgetType, MessageMapType, MessageMapType::IsControl > Dispatcher;

public:
	/// \ingroup EventHandlersAspectAspectDragDrop
	/// Setting the event handler for the "drop files" event
	/** If supplied event handler is called when a file is dropped over the widget.
	  * The function would receive a vector with all file paths and the coordinats where the files were dropped <br>
	  *
	  * Example:
	  *
	  * void DropFile(std::vector<SmartUtil::tstring> files, Point droppoint) {
	  * 	SmartUtil::tstring path = files.at(0);
	  * 	setText(path); 
	  * 	int x = droppoint.x;
	  * 	int y = droppoint.y;
	  * }
	  */
	void onDragDrop( typename MessageMapType::itsVoidFunctionTakingVectorPoint eventHandler );
	void onDragDrop( typename MessageMapType::voidFunctionTakingVectorPoint eventHandler );

	/// Setup Drag & Drop for this dialog 
	/** This setup the ability to receive an WM_DROPFILES msg if you drop a file on dialog 
	*/ 
	void setDragAcceptFiles(bool accept = true); 

protected:
	virtual ~AspectDragDrop()
	{}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template< class EventHandlerClass, class WidgetType, class MessageMapType >
void AspectDragDrop< EventHandlerClass, WidgetType, MessageMapType >::onDragDrop( typename MessageMapType::itsVoidFunctionTakingVectorPoint eventHandler )
{
	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->addNewSignal(
		typename MessageMapType::SignalTupleType(
			private_::SignalContent(
				Message( WM_DROPFILES ),
				reinterpret_cast< itsVoidFunction >( eventHandler ),
				ptrThis
			),
			typename MessageMapType::SignalType(
				typename MessageMapType::SignalType::SlotType( & Dispatcher::dispatchThis ) )
		)
	);
}

template< class EventHandlerClass, class WidgetType, class MessageMapType >
void AspectDragDrop< EventHandlerClass, WidgetType, MessageMapType >::onDragDrop( typename MessageMapType::voidFunctionTakingVectorPoint eventHandler )
{
	MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
	ptrThis->addNewSignal(
		typename MessageMapType::SignalTupleType(
			private_::SignalContent(
				Message( WM_DROPFILES ),
				reinterpret_cast< private_::SignalContent::voidFunctionTakingVoid >( eventHandler ),
				ptrThis
			),
			typename MessageMapType::SignalType(
				MessageMapType::SignalType::SlotType( & Dispatcher::dispatch )
			)
		)
	);
}

template< class EventHandlerClass, class WidgetType, class MessageMapType >
void AspectDragDrop< EventHandlerClass, WidgetType, MessageMapType >::setDragAcceptFiles(bool accept)
{
	DragAcceptFiles(static_cast< WidgetType * >( this )->handle(),accept); 
}

// end namespace SmartWin
}

#else

template< class EventHandlerClass, class WidgetType, class MessageMapType >
class AspectDragDrop { };

#endif

#endif

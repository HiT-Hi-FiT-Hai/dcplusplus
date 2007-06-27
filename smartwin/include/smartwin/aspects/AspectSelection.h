// $Revision: 1.14 $
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
#ifndef AspectSelection_h
#define AspectSelection_h

#include "boost.h"
#include "../SignalParams.h"

namespace SmartWin
{
// begin namespace SmartWin
template<typename WidgetType>
struct AspectSelectionDispatcher
{
	typedef boost::function<void ()> F;
	
	AspectSelectionDispatcher(const F& f_) : f(f_) { }

	HRESULT operator()(private_::SignalContent& params) {
		if ( !WidgetType::isValidSelectionChanged( params.Msg.LParam ) )
			return 0;
		f();
		return 0;
	}

	F f;
};

/// Aspect class used by Widgets that have the possibility of being "selecting"
/// item(s).
/** \ingroup AspectClasses
  * E.g. the WidgetComboBox have a "selected" Aspect therefore it realizes the
  * AspectSelection through inheritance.
  */
template< class EventHandlerClass, class WidgetType, class MessageMapType >
class AspectSelection
{
	typedef AspectSelectionDispatcher<WidgetType> Dispatcher;
	typedef AspectAdapter<typename Dispatcher::F, EventHandlerClass, MessageMapType::IsControl> Adapter;
public:
	/// \ingroup EventHandlersAspectSelection
	/// Setting the event handler for the "selection changed" event
	/** This event will be raised when the selected property of the Widget have
	  * changed either due to user interaction or due to some other reason. <br>
	  * No parameters are passed.
	  */
	void onSelectionChanged( typename MessageMapType::itsVoidFunctionTakingVoid eventHandler ) {
		onSelectionChanged(Adapter::adapt0(boost::polymorphic_cast<WidgetType*>(this), eventHandler));		
	}
	void onSelectionChanged( typename MessageMapType::voidFunctionTakingVoid eventHandler ) {
		onSelectionChanged(Adapter::adapt0(boost::polymorphic_cast<WidgetType*>(this), eventHandler));		
	}
	void onSelectionChanged(const typename Dispatcher::F& f) {
		MessageMapType * ptrThis = boost::polymorphic_cast< MessageMapType * >( this );
		ptrThis->setCallback(
			WidgetType::getSelectionChangedMessage(), Dispatcher(f)
		);
	}

	/// Sets the selected index of the Widget
	/** The idx parameter is the (zero indexed) value of the item to set as the
	  * selected item.         You must add the items before you set the selected
	  * index.
	  */
	virtual void setSelectedIndex( int idx ) = 0;

	/// Return the selected index of the Widget
	/** The return value is the selected items index of the Widget, if no item is
	  * selected the return value will be -1. <br>
	  * Note! <br>
	  * Some Widgets have the possibillity of selecting multiple items, if so you
	  * should not use this function but rather the multiple selection value getter.
	  */
	virtual int getSelectedIndex() const = 0;

protected:
	virtual ~AspectSelection()
	{}
};

// end namespace SmartWin
}

#endif

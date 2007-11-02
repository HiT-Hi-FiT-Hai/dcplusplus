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
#ifndef AspectDblClickable_h
#define AspectDblClickable_h

#include "../Dispatchers.h"

namespace SmartWin
{
// begin namespace SmartWin

/// \ingroup AspectClasses
/// Aspect class used by Widgets that have the possibility of being "Double
/// Clicked".
/** E.g. the WidgetStatic have a "Double Clicked" Aspect therefore it realizes the
  * AspectDblClickable through inheritance.
  */
template< class WidgetType >
class AspectDblClickable
{
	typedef Dispatchers::VoidVoid<> Dispatcher;
public:
	/// \ingroup EventHandlersAspectDblClickable
	/// Setting the event handler for the "Double Clicked" event
	/** All Widgets that realize this Aspect will raise this event when Widget is
	  * being Double Clicked. No parameters are passed.
	  */

	void onDblClicked(const Dispatcher::F& f) {
		static_cast<WidgetType*>(this)->setCallback(
			WidgetType::getDblClickMessage(), Dispatcher(f)
		);
	}

protected:
	virtual ~AspectDblClickable()
	{}
};

// end namespace SmartWin
}

#endif

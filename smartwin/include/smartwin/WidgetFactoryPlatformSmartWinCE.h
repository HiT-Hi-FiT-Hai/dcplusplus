// $Revision: 1.5 $
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
#ifdef WINCE
#ifndef WidgetFactoryPlatformSmartWinDesktop_h
#define WidgetFactoryPlatformSmartWinDesktop_h

#include "WidgetFactoryPlatformCommon.h"

namespace SmartWin
{
// begin namespace SmartWin

/// Class for WidgetFactoryCode which only exists in WinCE version of Windows API
/** Windows CE dependant functions which does not exist in Desktop version of Windows
  * API will be here
  */
template< template< class, class > class ContainerWidgetType, class EventHandlerClass, class MessageMapPolicy >
class WidgetFactoryPlatformImplementation< ContainerWidgetType, EventHandlerClass, MessageMapPolicy, SmartWinCE >
	: public ContainerWidgetType< EventHandlerClass, MessageMapPolicy >
{
public:
	WidgetFactoryPlatformImplementation()
		: ContainerWidgetType< EventHandlerClass, MessageMapPolicy >( 0 )
	{}

	WidgetFactoryPlatformImplementation( SmartWin::Widget * parent )
		: ContainerWidgetType< EventHandlerClass, MessageMapPolicy >( parent )
	{}
};

// end namespace SmartWin
}

#endif //! WidgetFactoryPlatformSmartWinDesktop_h
#endif //! WINCE

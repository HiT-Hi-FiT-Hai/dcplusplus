/*
  DC++ Widget Toolkit

  Copyright (c) 2007-2008, Jacek Sieka

  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, 
  are permitted provided that the following conditions are met:

      * Redistributions of source code must retain the above copyright notice, 
        this list of conditions and the following disclaimer.
      * Redistributions in binary form must reproduce the above copyright notice, 
        this list of conditions and the following disclaimer in the documentation 
        and/or other materials provided with the distribution.
      * Neither the name of the DWT nor the names of its contributors 
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

#ifndef ASPECTCOMMAND_
#define ASPECTCOMMAND_

#include "../Message.h"
#include "../Dispatchers.h"

namespace dwt {

template<typename WidgetType>
class AspectCommand {
	WidgetType& W() { return *static_cast<WidgetType*>(this); }
	typedef Dispatchers::VoidVoid<> Dispatcher;
public:
	void onCommand(const Dispatcher::F& f, unsigned id) {
		W().addCallback(Message(WM_COMMAND, id), Dispatcher(f));
	}

	void onCommand(const Dispatcher::F& f, unsigned controlId, unsigned code) {
		W().addCallback(Message(WM_COMMAND, MAKEWPARAM(controlId, code)), Dispatcher(f));
	}

	void onSysCommand(const Dispatcher::F& f, unsigned id) {
		W().addCallback(Message(WM_SYSCOMMAND, id), Dispatcher(f));
	}
};

}

#endif /*ASPECTCOMMAND_*/

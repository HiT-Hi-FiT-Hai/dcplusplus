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

#ifndef DWT_CONTROL_H_
#define DWT_CONTROL_H_

#include "../Policies.h"

#include "../aspects/AspectBorder.h"
#include "../aspects/AspectCloseable.h"
#include "../aspects/AspectContextMenu.h"
#include "../aspects/AspectEnabled.h"
#include "../aspects/AspectHelp.h"
#include "../aspects/AspectKeyboard.h"
#include "../aspects/AspectMouse.h"
#include "../aspects/AspectRaw.h"
#include "../aspects/AspectSizable.h"
#include "../aspects/AspectTimer.h"
#include "../aspects/AspectVisible.h"

namespace dwt {

/** Base class for all windows */
template<typename Policy>
class Control: 
	public MessageMap<Policy>,

	public AspectBorder<Control<Policy> >,
	public AspectCloseable<Control<Policy> >,
	public AspectContextMenu<Control<Policy> >,
	public AspectEnabled<Control<Policy> >,
	public AspectHelp<Control<Policy> >,
	public AspectKeyboard<Control<Policy> >,
	public AspectMouse<Control<Policy> >,
	public AspectRaw<Control<Policy> >,
	public AspectSizable<Control<Policy> >,
	public AspectTimer<Control<Policy> >,
	public AspectVisible<Control<Policy> >
{
	typedef MessageMap<Policy> BaseType;
public:

protected:
	struct Seed : public BaseType::Seed {
		Seed(LPCTSTR className, DWORD style, DWORD exStyle = 0, const tstring& caption = tstring());
	};

	typedef Control<Policy> ControlType;

	Control(Widget* parent);
};

typedef Control<Policies::Subclassed> CommonControl;

template<typename Policy>
Control<Policy>::Control(Widget* parent) : BaseType(parent) {
	
}

template<typename Policy>
Control<Policy>::Seed::Seed(LPCTSTR className, DWORD style, DWORD exStyle, const tstring& caption) : 
	BaseType::Seed(className, style | WS_VISIBLE, exStyle, caption)
{
	
}

}

#endif /*CONTROL_H_*/

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

#ifndef DWT_ASPECTDIALOG_H_
#define DWT_ASPECTDIALOG_H_

#include "../tstring.h"
#include "../WidgetCreator.h"

#include <type_traits>

namespace dwt {

template<typename WidgetType>
class AspectDialog {
	WidgetType& W() { return *static_cast<WidgetType*>(this); }
	HWND H() { return W().handle(); }

public:
	HWND getItem(int id) {
		return ::GetDlgItem(H(), id);
	}

	void setItemText(int id, const tstring& text) {
		::SetDlgItemText(H(), id, text.c_str());
	}
	
	template<typename T>
	void attachChild(T& childPtr, int id) {
		childPtr = attachChild<typename std::tr1::remove_pointer<T>::type >(id);
	}
	
	template<typename T>
	typename T::ObjectType attachChild(int id) {
		return WidgetCreator<T>::attach(&W(), id);
	}

};

}

#endif /*ASPECTDIALOG_H_*/

/*
  DC++ Widget Toolkit

  Copyright (c) 2007-2008, Jacek Sieka

  SmartWin++

  Copyright (c) 2005 Thomas Hansen

  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, 
  are permitted provided that the following conditions are met:

      * Redistributions of source code must retain the above copyright notice, 
        this list of conditions and the following disclaimer.
      * Redistributions in binary form must reproduce the above copyright notice, 
        this list of conditions and the following disclaimer in the documentation 
        and/or other materials provided with the distribution.
      * Neither the name of the DWT nor SmartWin++ nor the names of its contributors 
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

#ifndef DWT_ASPECTBUTTON_H_
#define DWT_ASPECTBUTTON_H_

#include "../aspects/AspectColor.h"
#include "../aspects/AspectClickable.h"
#include "../aspects/AspectDblClickable.h"
#include "../aspects/AspectFocus.h"
#include "../aspects/AspectFont.h"
#include "../aspects/AspectPainting.h"
#include "../aspects/AspectText.h"
#include "Control.h"

namespace dwt {

/** Common stuff for all buttons */
class Button :
	public CommonControl,
	public AspectClickable<Button>,
	public AspectColor<Button>,
	public AspectColorCtlImpl<Button>,
	public AspectDblClickable<Button>,
	public AspectFocus< Button >,
	public AspectFont< Button >,
	public AspectPainting< Button >,
	public AspectText< Button >
{
	typedef CommonControl BaseType;
	friend class AspectClickable<Button>;
	friend class AspectDblClickable<Button>;
	friend class WidgetCreator<Button>;
public:
	/// Class type
	typedef Button ThisType;

	/// Object type
	typedef ThisType* ObjectType;

	/// Seed class
	/** This class contains all of the values needed to create the widget. It also
	  * knows the type of the class whose seed values it contains. Every widget
	  * should define one of these.
	  */
	struct Seed : public BaseType::Seed {
		typedef ThisType WidgetType;
		
		FontPtr font;

		/// Fills with default parameters
		Seed(const tstring& caption_ = tstring(), DWORD style = 0);
	};

	template<typename SeedType>
	void create(const SeedType& cs = SeedType());

protected:
	typedef Button ButtonType;
	
	Button(Widget* parent);
	
private:
	// Contract needed by AspectClickable Aspect class
	static Message getClickMessage();

	// Contract needed by AspectDblClickable Aspect class
	static Message getDblClickMessage();

};

inline Message Button::getClickMessage() {
	return Message( WM_COMMAND, MAKEWPARAM(0, BN_CLICKED) );
}

inline Message Button::getDblClickMessage() {
	return Message( WM_COMMAND, MAKEWPARAM(0, BN_DBLCLK) );
}

inline Button::Button(Widget* parent) : BaseType(parent) {
	
}

template<typename SeedType>
void Button::create( const SeedType & cs ) {
	BaseType::create(cs);
	if(cs.font)
		setFont( cs.font );
}

}

#endif /*ASPECTBUTTON_H_*/

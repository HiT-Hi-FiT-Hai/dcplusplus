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

#ifndef AspectBorder_h
#define AspectBorder_h

namespace dwt {

/// Aspect class used by Widgets that have borders which can have multiple styles.
/** \ingroup AspectClasses
  * E.g. the Table have a "border" Aspect therefore it realizes the AspectBorder
  * through inheritance.
  */
template< class WidgetType >
class AspectBorder
{
	WidgetType& W() { return *static_cast<WidgetType*>(this); }
public:
	/// Set or remove the simple border (solid line)
	void setBorder( bool value = true );

	/// Set or remove the sunken border (like in text box widgets)
	void setSunkenBorder( bool value = true );

	/// Set or remove the smooth sunken border (generally used in read only text boxes)
	void setSmoothSunkenBorder( bool value = true );

	/// Set or remove the raised border (like in buttons)
	void setRaisedBorder( bool value = true );
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template< class WidgetType >
void AspectBorder< WidgetType >::setBorder( bool value )
{
	W().addRemoveStyle( WS_BORDER, value );
}

template< class WidgetType >
void AspectBorder< WidgetType >::setSunkenBorder( bool value )
{
	W().addRemoveExStyle( WS_EX_CLIENTEDGE, value );
}

template< class WidgetType >
void AspectBorder< WidgetType >::setSmoothSunkenBorder( bool value )
{
	W().addRemoveExStyle( WS_EX_STATICEDGE, value );
}

template< class WidgetType >
void AspectBorder< WidgetType >::setRaisedBorder( bool value )
{
	W().addRemoveStyle( WS_THICKFRAME, value );
}

}

#endif

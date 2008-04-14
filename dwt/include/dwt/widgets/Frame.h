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

#ifndef DWT_Frame_h
#define DWT_Frame_h

#include "../resources/Icon.h"
#include "../aspects/AspectMinMax.h"
#include "Composite.h"

namespace dwt {

/// Main Window class
/** \ingroup WidgetControls
  * \WidgetUsageInfo
  * \image html widgetwindow.png
  * This class defines a "normal" window or the most commonly used "container 
  * Widget", normally you would define your own class which (indirectly) derives from 
  * this one. <br>
  * You would normally derive directly from WidgetFactory and then supply this class 
  * as the first template parameter. <br>
  * The second parameter would then be YOUR CLASS. <br>
  * Example <br>
  * <b>class MyMainWindow : public dwt::WidgetFactory<dwt::Window, 
  * MyMainWindow> { ... };</b> <br>
  * Note especially that the second template argument to the WidgetFactory template 
  * class would almost ALWAYS be the name of your class derived from WidgetFactory. 
  * <br>
  * You can also derive directly from Window and skip around the WidgetFactory 
  * factory class, the inheritance string would then become: <br>
  * <b>class MyMainWindow : public dwt::Window<MyMainWindow></b> <br>
  * But then you wouldn't have access to all the "createxxx" functions from class 
  * WidgetFactory which automatically gurantees that your Widgets get the right parent 
  * etc. <br>
  * Look at (almost) any of the example projects distributed with the main download of 
  * the library residing in the dwtUnitTests directory for an example of how to 
  * use  this class with the factory class WidgetFactory.   
  */
template< class Policy >
class Frame :
	public Composite< Policy >,
	public AspectMinMax<Frame<Policy> >
{
	typedef Composite< Policy > BaseType;
public:
	/// Class type
	typedef Frame< Policy > ThisType;

	/// Object type
	typedef ThisType * ObjectType;
	
#ifndef WINCE
	/// Animates a window
	/** Slides the window into view from either right or left depending on the
	  * parameter "left". If "left" is true, then from the left,  otherwise from the
	  * right. <br>
	  * Show defines if the window shall come INTO view or go OUT of view. <br>
	  * The "time" parameter is the total duration of the function in milliseconds. 
	  */
	void animateSlide( bool show, bool left, unsigned int msTime );

	/// Animates a window
	/** Blends the window INTO view or OUT of view. <br>
	  * Show defines if the window shall come INTO view or go OUT of view. <br>
	  * The "time" parameter is the total duration of the function in milliseconds. 
	  */
	void animateBlend( bool show, int msTime );

	/// Animates a window
	/** Collapses the window INTO view or OUT of view. The collapse can be thought of
	  * as either an "explosion" or an "implosion". <br>
	  * Show defines if the window shall come INTO view or go OUT of view. <br>
	  * The "time" parameter is the total duration of the function in milliseconds. 
	  */
	void animateCollapse( bool show, int msTime );
#endif

	/// Adds or removes the minimize box from the Widget
	void setMinimizeBox( bool value = true );

	/// Adds or removes the maximize box from the Widget
	void setMaximizeBox( bool value = true );

	/// Sets the small icon for the Widget (the small icon appears typically in the top left corner of the Widget)
	void setIconSmall( const IconPtr& icon );

	/// Sets the large icon for the Widget (the large icon appears e.g. when you press ALT+Tab)
	void setIconLarge( const IconPtr& icon );

protected:
	struct Seed : public BaseType::Seed {
		Seed(const tstring& caption, DWORD style, DWORD exStyle);
	};

	// Protected since this Widget we HAVE to inherit from
	explicit Frame( Widget * parent = 0 );

	// Protected to avoid direct instantiation, you can inherit but NOT instantiate
	// directly
	virtual ~Frame()
	{}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename Policy>
Frame<Policy>::Seed::Seed(const tstring& caption, DWORD style, DWORD exStyle) : 
	BaseType::Seed(caption, style | WS_OVERLAPPEDWINDOW, exStyle)
{
	
}

#ifndef WINCE
template< class Policy >
void Frame< Policy >::animateSlide( bool show, bool left, unsigned int time )
{
	::AnimateWindow( this->handle(), static_cast< DWORD >( time ),
		show ?
			left ? AW_SLIDE | AW_HOR_NEGATIVE :
				AW_SLIDE | AW_HOR_POSITIVE
			:
			left ? AW_HIDE | AW_SLIDE | AW_HOR_NEGATIVE :
				AW_HIDE | AW_SLIDE | AW_HOR_POSITIVE
			);
}

//HC: This function gives problems with some non-Microsoft visual styles
template< class Policy >
void Frame< Policy >::animateBlend( bool show, int msTime )
{
	::AnimateWindow( this->handle(), static_cast< DWORD >( msTime ), show ? AW_BLEND : AW_HIDE | AW_BLEND );
}

template< class Policy >
void Frame< Policy >::animateCollapse( bool show, int msTime )
{
	::AnimateWindow( this->handle(), static_cast< DWORD >( msTime ), show ? AW_CENTER : AW_HIDE | AW_CENTER );
}
#endif

template< class Policy >
void Frame< Policy >::setMinimizeBox( bool value )
{
	Widget::addRemoveStyle( WS_MINIMIZEBOX, value );
}

template< class Policy >
void Frame< Policy >::setMaximizeBox( bool value )
{
	Widget::addRemoveStyle( WS_MAXIMIZEBOX, value );
}

template< class Policy >
void Frame< Policy >::setIconSmall( const IconPtr& icon )
{
	::SendMessage( this->handle(), WM_SETICON, ICON_SMALL, reinterpret_cast< LPARAM >( icon->handle() ) );
}

template< class Policy >
void Frame< Policy >::setIconLarge( const IconPtr& icon )
{
	::SendMessage( this->handle(), WM_SETICON, ICON_BIG, reinterpret_cast< LPARAM >( icon->handle() ) );
}

template< class Policy >
Frame< Policy >::Frame( Widget * parent )
	: Composite<Policy>( parent )
{
}

}

#endif

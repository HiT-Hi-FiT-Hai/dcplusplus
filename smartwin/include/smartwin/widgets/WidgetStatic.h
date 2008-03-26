/*
  Copyright ( c ) 2005, Thomas Hansen
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification,
  are permitted provided that the following conditions are met :

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
  ( INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION ) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  OR TORT ( INCLUDING NEGLIGENCE OR OTHERWISE ) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef WidgetStatic_h
#define WidgetStatic_h

#include "../Widget.h"
#include "../aspects/AspectColor.h"
#include "../aspects/AspectClickable.h"
#include "../aspects/AspectControl.h"
#include "../aspects/AspectDblClickable.h"
#include "../aspects/AspectFocus.h"
#include "../aspects/AspectFont.h"
#include "../aspects/AspectPainting.h"
#include "../aspects/AspectText.h"
#include "../resources/Bitmap.h"

namespace SmartWin
{
// begin namespace SmartWin

// Forward declaring friends
template< class WidgetType >
class WidgetCreator;

/// Static Control class
/** \ingroup WidgetControls
  * \WidgetUsageInfo
  * \image html static.PNG
  * Class for creating a Static control. <br>
  * A static control is kind of like a TextBox which cannot be written into, it's
  * basically just a control to put text into though you can also put other things
  * into it, e.g. Icons or Bitmaps. <br>
  * But it does have some unique properties : <br>
  * It can be clicked and double clicked which a TextBox cannot! <br>
  * It can load a bitmap.
  */
class WidgetStatic :
	// Aspects
	public AspectClickable< WidgetStatic >,
	public AspectColor< WidgetStatic >,
	public AspectColorCtlImpl<WidgetStatic>,
	public AspectControl<WidgetStatic>,
	public AspectDblClickable< WidgetStatic >,
	public AspectFocus< WidgetStatic >,
	public AspectFont< WidgetStatic >,
	public AspectPainting< WidgetStatic >,
	public AspectText< WidgetStatic >
{
	friend class WidgetCreator< WidgetStatic >;
public:
	/// Seed class
	/** This class contains all of the values needed to create the widget. It also
	  * knows the type of the class whose seed values it contains. Every widget
	  * should define one of these.
	  */
	class Seed
		: public Widget::Seed
	{
	public:
		FontPtr font;

		/// Fills with default parameters
		Seed(const SmartUtil::tstring& caption_ = SmartUtil::tstring());
	};

	// Contract needed by AspectClickable Aspect class
	Message getClickMessage();

	// Contract needed by AspectDblClickable Aspect class
	Message getDblClickMessage();

	/// Actually creates the Static Control
	/** You should call WidgetFactory::createStatic if you instantiate class
	  * directly. <br>
	  * Only if you DERIVE from class you should call this function directly.       
	  */
	void create( const Seed & cs = Seed() );

	/// Assigns a Bitmap (BMP) to the static control
	/** Use the Bitmap class and the BitmapPtr to load a Bitmap and call this
	  * function to assign that Bitmap to your WidgetStatic
	  */
	void setBitmap( const BitmapPtr& bitmap );

protected:
	// Constructor Taking pointer to parent
	explicit WidgetStatic( SmartWin::Widget * parent );

	// Protected to avoid direct instantiation, you can inherit and use
	// WidgetFactory class which is friend
	virtual ~WidgetStatic()
	{}

private:
	BitmapPtr itsBitmap;

	void setBitmap( HBITMAP bitmap );
};

inline Message WidgetStatic::getClickMessage()
{
	return Message( WM_COMMAND, MAKEWPARAM(this->getControlId(), STN_CLICKED) );
}

inline Message WidgetStatic::getDblClickMessage()
{
	return Message( WM_COMMAND, MAKEWPARAM(this->getControlId(), STN_DBLCLK) );
}

inline WidgetStatic::WidgetStatic( Widget * parent )
	: ControlType( parent )
{
}

inline void WidgetStatic::setBitmap( HBITMAP bitmap )
{
	this->addRemoveStyle( SS_BITMAP, true );
	this->sendMessage(STM_SETIMAGE, ( WPARAM ) IMAGE_BITMAP, ( LPARAM ) bitmap );
}

inline void WidgetStatic::setBitmap( const BitmapPtr& bitmap )
{
	this->setBitmap( bitmap->handle() );
	itsBitmap = bitmap;
}

// end namespace SmartWin
}

#endif

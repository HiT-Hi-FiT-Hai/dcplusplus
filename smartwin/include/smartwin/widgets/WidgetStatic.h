// $Revision: 1.31 $
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

#include "../MessageMapPolicyClasses.h"
#include "../aspects/AspectBackgroundColor.h"
#include "../aspects/AspectBorder.h"
#include "../aspects/AspectClickable.h"
#include "../aspects/AspectDblClickable.h"
#include "../aspects/AspectEnabled.h"
#include "../aspects/AspectFocus.h"
#include "../aspects/AspectFont.h"
#include "../aspects/AspectMouseClicks.h"
#include "../aspects/AspectPainting.h"
#include "../aspects/AspectRaw.h"
#include "../aspects/AspectSizable.h"
#include "../aspects/AspectText.h"
#include "../aspects/AspectVisible.h"
#include "../resources/Bitmap.h"
#include "../xCeption.h"

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
	public MessageMapPolicy< Policies::Subclassed >,

	// Aspects
	public AspectBackgroundColor< WidgetStatic >,
	public AspectBorder< WidgetStatic >,
	public AspectClickable< WidgetStatic >,
	public AspectDblClickable< WidgetStatic >,
	public AspectEnabled< WidgetStatic >,
	public AspectFocus< WidgetStatic >,
	public AspectFont< WidgetStatic >,
	public AspectMouseClicks< WidgetStatic >,
	public AspectPainting< WidgetStatic >,
	public AspectRaw< WidgetStatic >,
	public AspectSizable< WidgetStatic >,
	public AspectText< WidgetStatic >,
	public AspectVisible< WidgetStatic >
{
	friend class WidgetCreator< WidgetStatic >;
public:
	/// Class type
	typedef WidgetStatic ThisType;

	/// Object type
	typedef ThisType * ObjectType;

	typedef MessageMapPolicy<Policies::Subclassed> PolicyType;

	/// Seed class
	/** This class contains all of the values needed to create the widget. It also
	  * knows the type of the class whose seed values it contains. Every widget
	  * should define one of these.
	  */
	class Seed
		: public SmartWin::Seed
	{
	public:
		typedef WidgetStatic::ThisType WidgetType;

		FontPtr font;

		/// Fills with default parameters
		// explicit to avoid conversion through SmartWin::CreationalStruct
		explicit Seed();

		/// Doesn't fill any values
		Seed( DontInitialize )
		{}
	};

	/// Default values for creation
	static const Seed & getDefaultSeed();

	// Contract needed by AspectClickable Aspect class
	static const Message& getClickMessage();

	// Contract needed by AspectBackgroundColor Aspect class
	static const Message & getBackgroundColorMessage();

	// Contract needed by AspectDblClickable Aspect class
	static const Message& getDblClickMessage();

	/// Actually creates the Static Control
	/** You should call WidgetFactory::createStatic if you instantiate class
	  * directly. <br>
	  * Only if you DERIVE from class you should call this function directly.       
	  */
	virtual void create( const Seed & cs = getDefaultSeed() );

	/// Assigns a Bitmap (BMP) to the static control
	/** Use the Bitmap class and the BitmapPtr to load a Bitmap and call this
	  * function to assign that Bitmap to your WidgetStatic
	  */
	void setBitmap( BitmapPtr bitmap );

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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline WidgetStatic::Seed::Seed()
{
	* this = WidgetStatic::getDefaultSeed();
}

inline const Message & WidgetStatic::getClickMessage()
{
	static Message retVal = Message( WM_COMMAND, STN_CLICKED );
	return retVal;
}

inline const Message & WidgetStatic::getBackgroundColorMessage()
{
	static Message retVal = Message( WM_CTLCOLORSTATIC );
	return retVal;
}

inline const Message & WidgetStatic::getDblClickMessage()
{
	static Message retVal = Message( WM_COMMAND, STN_DBLCLK );
	return retVal;
}

inline WidgetStatic::WidgetStatic( SmartWin::Widget * parent )
	: PolicyType( parent )
{
	// Can't have a text box without a parent...
	xAssert( parent, _T( "Can't have a Static without a parent..." ) );
}

inline void WidgetStatic::setBitmap( HBITMAP bitmap )
{
	this->addRemoveStyle( SS_BITMAP, true );
	::SendMessage( this->handle(), STM_SETIMAGE, ( WPARAM ) IMAGE_BITMAP, ( LPARAM ) bitmap );
}

inline void WidgetStatic::setBitmap( BitmapPtr bitmap )
{
	this->setBitmap( bitmap->getBitmap() );
	itsBitmap = bitmap;
}

// end namespace SmartWin
}

#endif

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
#ifndef WINCE
#ifndef WidgetFactoryPlatformSmartWinDesktop_h
#define WidgetFactoryPlatformSmartWinDesktop_h

#include "WidgetFactoryPlatformCommon.h"
#include "widgets/WidgetRichTextBox.h"
#include "widgets/WidgetChooseFont.h"
#include "widgets/WidgetMenuExtended.h"
#include "widgets/WidgetToolbar.h"
#include "widgets/WidgetCoolbar.h"
#include "WidgetCreator.h"

namespace SmartWin
{
// begin namespace SmartWin

/// Class for WidgetFactoryCode which only exists in Desktop version of Windows API
/** Desktop version dependant functions which does not exist in Windows CE version of
  * Windows API will be here
  */
template< typename ContainerWidgetType >
class WidgetFactoryPlatformImplementation< ContainerWidgetType,  SmartWinDesktop >
	: public ContainerWidgetType
{
public:
	/// RichEditBox class type.
	typedef SmartWin::WidgetRichTextBox WidgetRichTextBox;

	/// RichEditBox object type.
	typedef typename WidgetRichTextBox::ObjectType WidgetRichTextBoxPtr;

	/// ExtendedMenu class type.
	typedef SmartWin::WidgetMenuExtended WidgetMenuExtended;

	/// ExtendedMenu object type.
	typedef typename WidgetMenuExtended::ObjectType WidgetMenuExtendedPtr;

	/// ChooseFont class and object type.
	typedef SmartWin::WidgetChooseFont< SmartWin::Widget > WidgetChooseFont;

	/// Toolbar class type.
	typedef SmartWin::WidgetToolbar WidgetToolbar;

	/// Toolbar object type.
	typedef typename WidgetToolbar::ObjectType WidgetToolbarPtr;

	/// Coolbar class type.
	typedef SmartWin::WidgetCoolbar WidgetCoolbar;

	/// Coolbar object type.
	typedef typename WidgetCoolbar::ObjectType WidgetCoolbarPtr;

	/// Constructor taking a pointer to it's parent.
	/** If you for instance create a WidgetChildWindow then use this Constructor
	  * since it explicitly sets the parent of the Widget.
	  */
	WidgetFactoryPlatformImplementation( Widget * parent )
		: ContainerWidgetType( parent )
	{}

	/// Creates a WidgetChooseFont and returns it.
	/** Usable to let user choose font from the system installed fonts.
	  */
	WidgetChooseFont createChooseFont()
	{
		WidgetChooseFont retVal( this );
		return retVal;
	}

	/// Creates a Rich Edit Control and returns a pointer to it.
	/** DON'T delete the returned pointer!!!
	  */
	WidgetRichTextBoxPtr createRichTextBox( const typename WidgetRichTextBox::Seed & cs = WidgetRichTextBox::Seed() )
	{
		return WidgetCreator< WidgetRichTextBox >::create( this, cs );
	}

	/// \ingroup SubclassDialog
	/// Subclasses a Rich Edit Control from the given resource id.
	/** DON'T delete the returned pointer!!! <br>
	  * Use e.g. the Dialog Designer to design a dialog and attach the controls
	  * with this function.
	  */
	WidgetRichTextBoxPtr attachRichTextBox( unsigned id )
	{
		return WidgetCreator< WidgetRichTextBox >::attach( this, id );
	}

	/// Creates an Extended Menu
	/** The returned object is of type std::tr1::shared_ptr< WidgetMenuExtended >, but
	  * you should use the typedef WidgetMenuExtendedPtr and not <br>
	  * the shared_ptr itself since this may change in future releases.
	  */
	WidgetMenuExtendedPtr createExtendedMenu(const typename WidgetMenuExtended::Seed& cs = WidgetMenuExtended::Seed())
	{
		return WidgetCreator< WidgetMenuExtended >::create( this, cs );
	}

	/// Creates a Tool Bar and returns a pointer to it.
	/** DON'T delete the returned pointer!!!
	  */
	WidgetToolbarPtr createToolbar( const typename WidgetToolbar::Seed & cs = WidgetToolbar::Seed() )
	{
		return WidgetCreator< WidgetToolbar >::create( this, cs );
	}

	/// Creates a Cool Bar and returns a pointer to it.
	/** DON'T delete the returned pointer!!!
	  */
	WidgetCoolbarPtr createCoolbar( const typename WidgetCoolbar::Seed & cs = WidgetCoolbar::Seed() )
	{
		return WidgetCreator< WidgetCoolbar >::create( this, cs );
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// end namespace SmartWin
}

#endif //! WidgetFactoryPlatformSmartWinDesktop_h
#endif //! WINCE

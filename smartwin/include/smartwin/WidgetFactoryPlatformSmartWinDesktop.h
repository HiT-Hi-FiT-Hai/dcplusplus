// $Revision: 1.23 $
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
#include "widgets/WidgetSplitter.h"
#include "widgets/WidgetToolbar.h"
#include "widgets/WidgetCoolbar.h"

namespace SmartWin
{
// begin namespace SmartWin

/// Class for WidgetFactoryCode which only exists in Desktop version of Windows API
/** Desktop version dependant functions which does not exist in Windows CE version of
  * Windows API will be here
  */
template< template< class, class > class ContainerWidgetType, class EventHandlerClass, class MessageMapPolicy >
class WidgetFactoryPlatformImplementation< ContainerWidgetType, EventHandlerClass, MessageMapPolicy, SmartWinDesktop >
	: public ContainerWidgetType< EventHandlerClass, MessageMapPolicy >
{
public:
	/// RichEditBox class type.
	typedef WidgetRichTextBox< EventHandlerClass, MessageMapPolicy, RichTextBox< EventHandlerClass, MessageMapPolicy > > WidgetRichTextBox;

	/// RichEditBox object type.
	typedef typename WidgetRichTextBox::ObjectType WidgetRichTextBoxPtr;

	/// ExtendedMenu class type.
	typedef WidgetMenuExtended< EventHandlerClass, MessageMapPolicy > WidgetMenuExtended;

	/// ExtendedMenu object type.
	typedef typename WidgetMenuExtended::ObjectType WidgetMenuExtendedPtr;

	/// ChooseFont class and object type.
	typedef WidgetChooseFont< SmartWin::Widget > WidgetChooseFont;

	/// Splitter class type.
	typedef WidgetSplitter< ContainerWidgetType, EventHandlerClass, SplitterThinPaint, MessageMapPolicyNormalWidget > WidgetSplitterThin;

	/// Splitter object type.
	typedef typename WidgetSplitterThin::ObjectType WidgetSplitterThinPtr;

	/// CoolSplitter class type.
	typedef WidgetSplitter< ContainerWidgetType, EventHandlerClass, SplitterCoolPaint, MessageMapPolicyNormalWidget > WidgetSplitterCool;

	/// CoolSplitter object type.
	typedef typename WidgetSplitterCool::ObjectType WidgetSplitterCoolPtr;

	/// Toolbar class type.
	typedef WidgetToolbar< EventHandlerClass, MessageMapPolicy > WidgetToolbar;

	/// Toolbar object type.
	typedef typename WidgetToolbar::ObjectType WidgetToolbarPtr;

	/// Coolbar class type.
	typedef WidgetCoolbar< EventHandlerClass, MessageMapPolicy > WidgetCoolbar;

	/// Coolbar object type.
	typedef typename WidgetCoolbar::ObjectType WidgetCoolbarPtr;

	/// Default Constructor setting parent to nothing!
	/** Use this Constructor for a Desktop application window which does not need a
	  * parent. If you need a parent for your window e.g. your window is a
	  * WidgetChildWindow then use the Constructor taking the Widget argument
	  */
	WidgetFactoryPlatformImplementation();

	/// Constructor taking a pointer to it's parent.
	/** If you for instance create a WidgetChildWindow then use this Constructor
	  * since it explicitly sets the parent of the Widget.
	  */
	WidgetFactoryPlatformImplementation( Widget * parent )
		: ContainerWidgetType< EventHandlerClass, MessageMapPolicy >( parent )
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
	WidgetRichTextBoxPtr createRichTextBox( const typename WidgetRichTextBox::Seed & cs = WidgetRichTextBox::getDefaultSeed() )
	{
		return WidgetCreator< WidgetRichTextBox >::create( this, cs );
	}

	/// \ingroup SubclassDialog
	/// Subclasses a Rich Edit Control from the given resource id.
	/** DON'T delete the returned pointer!!! <br>
	  * Use e.g. the Dialog Designer to design a dialog and subclass the controls
	  * with this function.
	  */
	WidgetRichTextBoxPtr subclassRichTextBox( unsigned id )
	{
		// If this one fizzles you have tried to call this function from a derived
		// class which not is derived from MessageMapPolicyDialogWidget, like for
		// instance both the MessageMapPolicyNormalWidget and the
		// MessageMapPolicyMDIChildWidget classes cannot logically subclass a
		// dialog item since they're NOT dialog Widgets therefore they will give
		// you a compile error if you try to call this function from Widgets
		// implementing the those classes! Only the MessageMapPolicyDialogWidget
		// can logically call this function and therefore it's the only one which
		// will not give you a compile time error here...
		typename MessageMapPolicy::canSubclassControls checker;
		//TODO: this error was catched by devcpp ... what was intended?
		//return WidgetCreator< WidgetRichTextBox >::subclass( this, cs );
		return WidgetCreator< WidgetRichTextBox >::subclass( this, id );
	}

	/// Creates an Extended Menu
	/** The returned object is of type boost::shared_ptr< WidgetMenuExtended >, but
	  * you should use the typedef WidgetMenuExtendedPtr and not <br>
	  * the shared_ptr itself since this may change in future releases.
	  */
	WidgetMenuExtendedPtr createExtendedMenu()
	{
		return WidgetCreator< WidgetMenuExtended >::create( this );
	}

	/// Creates a Splitter and returns a pointer to it.
	/** DON'T delete the returned pointer!!!
	  */
	WidgetSplitterThinPtr createSplitterThin( const typename WidgetSplitterCool::Seed & cs = WidgetSplitterThin::getDefaultSeed() )
	{
		return WidgetCreator< WidgetSplitterThin >::create( this, cs );
	}

	/// Creates a Cool Splitter and returns a pointer to it.
	/** DON'T delete the returned pointer!!!
	  */
	WidgetSplitterCoolPtr createSplitterCool( const typename WidgetSplitterCool::Seed & cs = WidgetSplitterCool::getDefaultSeed() )
	{
		return WidgetCreator< WidgetSplitterCool >::create( this, cs );
	}

	/// Generic Splitter Creation Method
	/** Creates a splitter with the given Painter Aspect. <br>
	  * Useful if you wish to be able to easily change the splitter type without
	  * doing massive recoding of application logic. <br>
	  * See WidgetSplitter Solution for an Example.
	  */
	template< class SplitterType >
	typename WidgetSplitter< ContainerWidgetType, EventHandlerClass, SplitterType >::ObjectType createSplitter(
			 const typename WidgetSplitter< ContainerWidgetType, EventHandlerClass, SplitterType >::Seed & cs )// =
				   // Commented out since GCC chokes on default parameters to template functions inside template template classes...!!
				   //WidgetSplitter< ContainerWidgetType, EventHandlerClass, SplitterType >::getDefaultSeed() )
	{
		return WidgetCreator< WidgetSplitter< ContainerWidgetType, EventHandlerClass, SplitterType > >::create( this, cs );
	}

	/// Creates a Tool Bar and returns a pointer to it.
	/** DON'T delete the returned pointer!!!
	  */
	WidgetToolbarPtr createToolbar( const typename WidgetToolbar::Seed & cs = WidgetToolbar::getDefaultSeed() )
	{
		return WidgetCreator< WidgetToolbar >::create( this, cs );
	}

	/// Creates a Cool Bar and returns a pointer to it.
	/** DON'T delete the returned pointer!!!
	  */
	WidgetCoolbarPtr createCoolbar( const typename WidgetCoolbar::Seed & cs = WidgetCoolbar::getDefaultSeed() )
	{
		return WidgetCreator< WidgetCoolbar >::create( this, cs );
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template< template< class, class > class ContainerWidgetType, class EventHandlerClass, class MessageMapPolicy >
WidgetFactoryPlatformImplementation< ContainerWidgetType, EventHandlerClass, MessageMapPolicy, SmartWinDesktop >::WidgetFactoryPlatformImplementation()
	: ContainerWidgetType< EventHandlerClass, MessageMapPolicy >( 0 )
{}

// end namespace SmartWin
}

#endif //! WidgetFactoryPlatformSmartWinDesktop_h
#endif //! WINCE

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

#ifndef DWT_WidgetFactory_h
#define DWT_WidgetFactory_h

#include "widgets/ColorDialog.h"
#include "widgets/FolderDialog.h"
#include "widgets/MessageBox.h"
#include "widgets/LoadDialog.h"
#include "widgets/SaveDialog.h"
#include "WidgetFactoryPlatformImplementation.h"

namespace dwt {

/// Factory class for creating Widgets from a derived custom class
/** This is the class you would normally derive from in your own application. <br>
  * < p >Derive directly from WidgetFactory and then supply Window as the first
  * template parameter. The second parameter would then be YOUR CLASS ( this is
  * needed for the SmartWin type system to function ) Example : < b >class
  * MyMainWindow : public SmartWin::WidgetFactory< SmartWin::Window,
  * MyMainWindow >   { ... };< /b > The third template argument is for declaring what
  * type of Widget you're declaring, for a "normal Widget" this defaults to
  * MessageMapNormalWidget, if this is a Widget constructed from a dialog
  * resource, you must state so by adding SmartWin::MessageMapDialogWidget and
  * if it is a MDI Child you must add SmartWin::MessageMapMDIChildWidget as the
  * third argument Then when you need e.g. a Button you would create that
  * button by calling createButton. Class contains type defs for your
  * convenience for all Widget types that exists in the SmartWin library.< /p > Note!
  * <br>
  * It is CRUCIAL that you DON'T explicitly delete any of the objects returned from
  * any of these functions. <br>
  * SmartWin will itself keep a track of which objects are alive and which it should
  * delete <br>
  * this is with purpose to make the library easier to use and more transparent for
  * C#, Java and newbie developers. <br>
  * Some functions returns stack objects, e.g. createMenu which returns a
  * std::tr1::shared_ptr and createMessageBox which returns a stack object, these also
  * should just get to "live their own life" and should not be tampered with in any
  * "memory ways".
  */
template<typename ContainerWidgetType>
class WidgetFactory
	: public WidgetFactoryPlatformImplementation< ContainerWidgetType, CurrentPlatform >
{
public:
	// Bring widgets into the namespace of the class that inherits from us
	
	typedef dwt::FolderDialog FolderDialog;

	/// MessageBox class and object type.
	typedef dwt::MessageBox MessageBox;

	/// LoadFileDialog class type.
	typedef dwt::LoadDialog LoadDialog;

	/// SaveFileDialog class and object type.
	typedef dwt::SaveDialog SaveDialog;

	/// ColorDialog class and object type.
	typedef dwt::ColorDialog ColorDialog;

	/// Default Constructor creating a factory Widget
	/** Default Constructor creating a factory Widget without a parent, if you need
	  * to explicitly state a parent like for instance you often would want in a
	  * WidgetChildWindow you should use the one taking a Widget * instead
	  */
	explicit WidgetFactory( Widget * parent );

	/// Creates a ChooseFolderDialog and returns a pointer to it.
	/** Use this one to construct a ( stack object ) to show a Choose folder Dialog
	  */
	FolderDialog createFolderDialog();

	/// Creates a LoadFileDialog and returns a pointer to it.
	/** Use this one to construct a ( stack object ) to show a Load File Dialog
	  */
	LoadDialog createLoadDialog();

	/// Creates a SaveFileDialog and returns a pointer to it.
	/** Use this one to construct a ( stack object ) to show a Save File Dialog
	  */
	SaveDialog createSaveDialog();

	/// Creates a ColorDialog and returns it.
	/** Usable to let user choose font from the system installed fonts.
	  */
	ColorDialog createColorDialog();

	/// Creates a Message Box returns it.
	/** Use this one to construct a ( stack object ) to show a message box
	  */
	MessageBox createMessageBox();

protected:
	// Protected to try to avoid stack creation...
	virtual ~WidgetFactory()
	{}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename ContainerWidgetType>
WidgetFactory< ContainerWidgetType >::WidgetFactory( dwt::Widget * parent )
		: WidgetFactoryPlatformImplementation< ContainerWidgetType, CurrentPlatform >( parent )
{}

template<typename ContainerWidgetType>
typename WidgetFactory< ContainerWidgetType >::FolderDialog
WidgetFactory< ContainerWidgetType >::createFolderDialog()
{
	return FolderDialog ( this );
}

template<typename ContainerWidgetType>
typename WidgetFactory< ContainerWidgetType >::LoadDialog
WidgetFactory< ContainerWidgetType >::createLoadDialog()
{
	return LoadDialog ( this );
}

template<typename ContainerWidgetType>
typename WidgetFactory< ContainerWidgetType >::SaveDialog
WidgetFactory< ContainerWidgetType >::createSaveDialog()
{
	return SaveDialog ( this );
}

template<typename ContainerWidgetType>
typename WidgetFactory< ContainerWidgetType >::ColorDialog
WidgetFactory< ContainerWidgetType >::createColorDialog()
{
	return ColorDialog( this );
}

template<typename ContainerWidgetType>
typename WidgetFactory< ContainerWidgetType >::MessageBox
WidgetFactory< ContainerWidgetType >::createMessageBox()
{
	return MessageBox( this );
}

}

#endif

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
#ifndef WidgetFactory_h
#define WidgetFactory_h

#include "SmartUtil.h"
#include "widgets/WidgetButton.h"
#include "widgets/WidgetCheckBox.h"
#include "widgets/WidgetComboBox.h"
#include "widgets/WidgetDateTimePicker.h"
#include "widgets/WidgetGroupBox.h"
#include "widgets/WidgetDataGrid.h"
#include "widgets/WidgetChooseFolder.h"
#include "widgets/WidgetLoadFile.h"
#include "widgets/WidgetMDIChild.h"
#include "widgets/WidgetMDIParent.h"
#include "widgets/WidgetTabSheet.h"
#include "widgets/WidgetMenu.h"
#include "widgets/WidgetMessageBox.h"
#include "widgets/WidgetRadioButton.h"
#include "widgets/WidgetSaveFile.h"
#include "widgets/WidgetChooseColor.h"
#include "widgets/WidgetSlider.h"
#include "widgets/WidgetSpinner.h"
#include "widgets/WidgetStatic.h"
#include "widgets/WidgetStatusBar.h"
#include "widgets/WidgetTextBox.h"
#include "widgets/WidgetTreeView.h"
#include "widgets/WidgetWindowBase.h"
#include "widgets/WidgetWindow.h"
#include "widgets/WidgetDialog.h"
#include "widgets/WidgetProgressBar.h"
#include "WidgetFactoryPlatformImplementation.h"
#include "aspects/AspectGetParent.h"
#include "WidgetCreator.h"
#include <memory>

namespace SmartWin
{
// begin namespace SmartWin

/// Factory class for creating Widgets from a derived custom class
/** This is the class you would normally derive from in your own application. <br>
  * < p >Derive directly from WidgetFactory and then supply WidgetWindow as the first
  * template parameter. The second parameter would then be YOUR CLASS ( this is
  * needed for the SmartWin type system to function ) Example : < b >class
  * MyMainWindow : public SmartWin::WidgetFactory< SmartWin::WidgetWindow,
  * MyMainWindow >   { ... };< /b > The third template argument is for declaring what
  * type of Widget you're declaring, for a "normal Widget" this defaults to
  * MessageMapPolicyNormalWidget, if this is a Widget constructed from a dialog
  * resource, you must state so by adding SmartWin::MessageMapPolicyDialogWidget and
  * if it is a MDI Child you must add SmartWin::MessageMapPolicyMDIChildWidget as the
  * third argument Then when you need e.g. a WidgetButton you would create that
  * button by calling createWidgetButton. Class contains type defs for your
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
template< template< class > class ContainerWidgetType, class EventHandlerClass >
class WidgetFactory
	: public WidgetFactoryPlatformImplementation< ContainerWidgetType, EventHandlerClass, CurrentPlatform >
{
public:
	/// MessageBox class and object type.
	typedef SmartWin::WidgetMessageBox< SmartWin::Widget > WidgetMessageBox;

	/// DataGrid class type.
	typedef SmartWin::WidgetDataGrid< EventHandlerClass > WidgetDataGrid;

	/// DataGrid object type.
	typedef typename WidgetDataGrid::ObjectType WidgetDataGridPtr;

	/// TreeView class type.
	typedef SmartWin::WidgetTreeView< EventHandlerClass > WidgetTreeView;

	/// TreeView object type.
	typedef typename WidgetTreeView::ObjectType WidgetTreeViewPtr;

	/// TextBox class type.
	typedef SmartWin::WidgetTextBox< EventHandlerClass > WidgetTextBox;

	/// TextBox object type.
	typedef typename WidgetTextBox::ObjectType WidgetTextBoxPtr;

	/// StatusBar class type.
	typedef SmartWin::WidgetStatusBar< EventHandlerClass > WidgetStatusBar;

	/// StatusBar object type.
	typedef typename WidgetStatusBar::ObjectType WidgetStatusBarPtr;

	/// StatusBarSections class type.
	typedef SmartWin::WidgetStatusBar< EventHandlerClass, Section< EventHandlerClass > > WidgetStatusBarSections;

	/// StatusBarSections object type.
	typedef typename WidgetStatusBarSections::ObjectType WidgetStatusBarSectionsPtr;

	/// Button class type.
	typedef SmartWin::WidgetButton< EventHandlerClass > WidgetButton;

	/// Button object type.
	typedef typename WidgetButton::ObjectType WidgetButtonPtr;

	/// MDIWindow class type.
	typedef SmartWin::WidgetMDIParent< EventHandlerClass > WidgetMDIParent;

	/// MDIWindow object type.
	typedef typename WidgetMDIParent::ObjectType WidgetMDIParentPtr;

	/// TabSheet class type.
	typedef SmartWin::WidgetTabSheet< EventHandlerClass > WidgetTabSheet;

	/// TabSheet object type.
	typedef typename WidgetTabSheet::ObjectType WidgetTabSheetPtr;

	/// Slider class type.
	typedef SmartWin::WidgetSlider< EventHandlerClass > WidgetSlider;

	/// Slider object type.
	typedef typename WidgetSlider::ObjectType WidgetSliderPtr;

	/// Spinner class type.
	typedef SmartWin::WidgetSpinner< EventHandlerClass > WidgetSpinner;

	/// Spinner object type.
	typedef typename WidgetSpinner::ObjectType WidgetSpinnerPtr;

	/// GroupBox class type.
	typedef SmartWin::WidgetGroupBox< EventHandlerClass > WidgetGroupBox;

	/// GroupBox object type.
	typedef typename WidgetGroupBox::ObjectType WidgetGroupBoxPtr;

	/// RadioButton class type.
	typedef SmartWin::WidgetRadioButton< EventHandlerClass > WidgetRadioButton;

	/// RadioButton object type.
	typedef typename WidgetRadioButton::ObjectType WidgetRadioButtonPtr;

	/// WidgetChooseFolder class type.
	typedef SmartWin::WidgetChooseFolder< SmartWin::Widget > WidgetChooseFolder;

	/// LoadFileDialog class type.
	typedef SmartWin::WidgetLoadFile< SmartWin::Widget > WidgetLoadFile;

	/// SaveFileDialog class and object type.
	typedef SmartWin::WidgetSaveFile< SmartWin::Widget > WidgetSaveFile;

	/// WidgetChooseColor class and object type.
	typedef SmartWin::WidgetChooseColor< SmartWin::Widget > WidgetChooseColor;

	/// ComboBox class type.
	typedef SmartWin::WidgetComboBox< EventHandlerClass > WidgetComboBox;

	/// ComboBox object type.
	typedef typename WidgetComboBox::ObjectType WidgetComboBoxPtr;

	/// Static class type.
	typedef SmartWin::WidgetStatic< EventHandlerClass > WidgetStatic;

	/// Static object type.
	typedef typename WidgetStatic::ObjectType WidgetStaticPtr;

	/// Menu class type.
	typedef SmartWin::WidgetMenu< EventHandlerClass > WidgetMenu;

	/// Menu object type.
	typedef typename WidgetMenu::ObjectType WidgetMenuPtr;

	/// CheckBox class type.
	typedef SmartWin::WidgetCheckBox< EventHandlerClass > WidgetCheckBox;

	/// CheckBox object type.
	typedef typename WidgetCheckBox::ObjectType WidgetCheckBoxPtr;

	/// DateTimePicker class type.
	typedef SmartWin::WidgetDateTimePicker< EventHandlerClass > WidgetDateTimePicker;

	/// DateTimePicker object type.
	typedef typename WidgetDateTimePicker::ObjectType WidgetDateTimePickerPtr;

	/// WidgetChildWindow class type.
	typedef SmartWin::WidgetChildWindow<EventHandlerClass > WidgetChildWindow;

	/// WidgetChildWindow object type.
	typedef typename WidgetChildWindow::ObjectType WidgetChildWindowPtr;

	/// WidgetWindow class type.
	typedef SmartWin::WidgetWindow< EventHandlerClass > WidgetWindow;

	/// WidgetWindow object type.
	typedef typename WidgetWindow::ObjectType WidgetWindowPtr;

	/// ProgressBar class type.
	typedef SmartWin::WidgetProgressBar< EventHandlerClass > WidgetProgressBar;

	/// ProgressBar object type.
	typedef typename WidgetProgressBar::ObjectType WidgetProgressBarPtr;

	/// Default Constructor creating a factory Widget
	/** Default Constructor creating a factory Widget without a parent, if you need
	  * to explicitly state a parent like for instance you often would want in a
	  * WidgetChildWindow you should use the one taking a Widget * instead
	  */
	WidgetFactory();

	explicit WidgetFactory( Widget * parent );

	/// Creates a ChooseFolderDialog and returns a pointer to it.
	/** Use this one to construct a ( stack object ) to show a Choose folder Dialog
	  */
	WidgetChooseFolder createChooseFolder();

	/// Creates a LoadFileDialog and returns a pointer to it.
	/** Use this one to construct a ( stack object ) to show a Load File Dialog
	  */
	WidgetLoadFile createLoadFile();

	/// Creates a SaveFileDialog and returns a pointer to it.
	/** Use this one to construct a ( stack object ) to show a Save File Dialog
	  */
	WidgetSaveFile createSaveFile();

	/// Creates a WidgetChooseColor and returns it.
	/** Usable to let user choose font from the system installed fonts.
	  */
	WidgetChooseColor createChooseColor();

	// TODO: Update, this isn't an automated collected Widget anymore...
	/// Creates a Message Box and returns a pointer to it.
	/** Use this one to construct a ( stack object ) to show a message box
	  */
	WidgetMessageBox createMessageBox();

	/// Creates a List View and returns a pointer to it.
	/** DON'T delete the returned pointer!!!
	  */
	WidgetDataGridPtr createDataGrid( const typename WidgetDataGrid::Seed & cs = WidgetDataGrid::getDefaultSeed() );

	/// \ingroup SubclassDialog
	/// Subclasses a Check Box from the given resource id.
	/** DON'T delete the returned pointer!!! <br>
	  * Use e.g. the Dialog Designer to design a dialog and subclass the controls
	  * with this function.
	  */
	WidgetDataGridPtr subclassList( unsigned id );

	/// Creates a Check Box and returns a pointer to it.
	/** DON'T delete the returned pointer!!!
	  */
	WidgetCheckBoxPtr createCheckBox( const typename WidgetCheckBox::Seed & cs = WidgetCheckBox::getDefaultSeed() );

	/// \ingroup SubclassDialog
	/// Subclasses a Check Box from the given resource id.
	/** DON'T delete the returned pointer!!! <br>
	  * Use e.g. the Dialog Designer to design a dialog and subclass the controls
	  * with this function.
	  */
	WidgetCheckBoxPtr subclassCheckBox( unsigned id );

	/// Creates a child window and returns a pointer to it.
	/** DON'T delete the returned pointer!!!
	  */
	WidgetChildWindowPtr createWidgetChildWindow( const typename WidgetChildWindow::Seed & cs = WidgetChildWindow::getDefaultSeed() );

	/// Creates a Tree View and returns a pointer to it.
	/** DON'T delete the returned pointer!!!
	  */
	WidgetTreeViewPtr createTreeView( const typename WidgetTreeView::Seed & cs = WidgetTreeView::getDefaultSeed() );

	/// \ingroup SubclassDialog
	/// Subclasses a Tree View Control from the given resource id.
	/** DON'T delete the returned pointer!!! <br>
	  * Use e.g. the Dialog Designer to design a dialog and subclass the controls
	  * with this function.
	  */
	WidgetTreeViewPtr subclassTreeView( unsigned id );

	/// Creates a Menu and returns a pointer to it.
	/** The returned object is of type std::tr1::shared_ptr< WidgetMenu >, but you should use the typedef WidgetMenuPtr and not < br >
	  * the shared_ptr itself since this may change in future releases.
	  */
	WidgetMenuPtr createMenu(const typename WidgetMenu::Seed& cs = WidgetMenu::Seed());

	/// Creates a Edit Control and returns a pointer to it.
	/** DON'T delete the returned pointer!!!
	  */
	WidgetTextBoxPtr createTextBox( const typename WidgetTextBox::Seed & cs = WidgetTextBox::getDefaultSeed() );

	/// \ingroup SubclassDialog
	/// Subclasses a Text Box Control from the given resource id.
	/** DON'T delete the returned pointer!!! <br>
	  * Use e.g. the Dialog Designer to design a dialog and subclass the controls
	  * with this function.
	  */
	WidgetTextBoxPtr subclassTextBox( unsigned id );

	// TODO: Is there any point in subclassing a status bar ? ! ?
	/// Creates a Status Bar and returns a pointer to it.
	/** DON'T delete the returned pointer!!!
	  */
	WidgetStatusBarPtr createStatusBar( const typename WidgetStatusBar::Seed & cs = WidgetStatusBar::getDefaultSeed() );

	/// Creates a Status Bar and returns a pointer to it.
	/** DON'T delete the returned pointer!!!
	  */
	WidgetStatusBarSectionsPtr createStatusBarSections( const typename WidgetStatusBarSections::Seed & cs = WidgetStatusBarSections::getDefaultSeed() );

	/// Creates a Button Control and returns a pointer to it.
	/** DON'T delete the returned pointer!!!
	  */
	WidgetButtonPtr createButton( const typename WidgetButton::Seed & cs = WidgetButton::getDefaultSeed() );

	/// Creates a Button Control and returns a pointer to it.
	/** DON'T delete the returned pointer!!!
	  */
	WidgetMDIParentPtr createMDIParent( const typename WidgetMDIParent::Seed & cs = WidgetMDIParent::getDefaultSeed() );

	/// Creates a Button Control and returns a pointer to it.
	/** DON'T delete the returned pointer!!!
	  */
	WidgetTabSheetPtr createTabSheet( const typename WidgetTabSheet::Seed & cs = WidgetTabSheet::getDefaultSeed() );

	/// Creates a Slider Control and returns a pointer to it.
	/** DON'T delete the returned pointer!!!
	  */
	WidgetSliderPtr createSlider( const typename WidgetSlider::Seed & cs = WidgetSlider::getDefaultSeed() );

	/// Creates a Spinner Control and returns a pointer to it.
	/** DON'T delete the returned pointer!!!
	  */
	WidgetSpinnerPtr createSpinner( const typename WidgetSpinner::Seed & cs = WidgetSpinner::getDefaultSeed() );

	/// Creates a Progress Bar Control and returns a pointer to it.
	/** DON'T delete the returned pointer!!!
	  */
	WidgetProgressBarPtr createProgressBar( const typename WidgetProgressBar::Seed & cs = WidgetProgressBar::getDefaultSeed() );

	/// \ingroup SubclassDialog
	/// Subclasses a Button Control from the given resource id.
	/** DON'T delete the returned pointer!!!< br >
	  * Use e.g. the Dialog Designer to design a dialog and subclass the controls with this function.
	  */
	WidgetButtonPtr subclassButton( unsigned id );


	
	/// \ingroup SubclassDialog
	/// Subclasses a Progress Bar Control from the given resource id.
	/** DON'T delete the returned pointer!!!< br >
	  * Use e.g. the Dialog Designer to design a dialog and subclass the controls with this function.
	  */
	WidgetProgressBarPtr subclassProgressBar( unsigned id );


	/// \ingroup SubclassDialog
	/// Subclasses a Slider Control from the given resource id.
	/** DON'T delete the returned pointer!!! <br>
	  * Use e.g. the Dialog Designer to design a dialog and subclass the controls
	  * with this function.
	  */
	WidgetSliderPtr subclassSlider( unsigned id );

	/// \ingroup SubclassDialog
	/// Subclasses a Spinner Control from the given resource id.
	/** DON'T delete the returned pointer!!! <br>
	  * Use e.g. the Dialog Designer to design a dialog and subclass the controls
	  * with this function.
	  */
	WidgetSpinnerPtr subclassSpinner( unsigned id );

	/// Creates a Group Box Control and returns a pointer to it.
	/** DON'T delete the returned pointer!!!
	  */
	WidgetGroupBoxPtr createGroupBox( const typename WidgetGroupBox::Seed & cs = WidgetGroupBox::getDefaultSeed() );

	/// Subclasses a Group Box Control and returns a pointer to it.
	/** DON'T delete the returned pointer!!!
	  */
	WidgetGroupBoxPtr subclassGroupBox( unsigned id );

	/// Creates a DateTime Picker Control and returns a pointer to it.
	/** DON'T delete the returned pointer!!!
	  */
	WidgetDateTimePickerPtr createDateTimePicker( const typename WidgetDateTimePicker::Seed & cs = WidgetDateTimePicker::getDefaultSeed() );

	/// Suvclasses a DateTime Picker Control and returns a pointer to it.
	/** DON'T delete the returned pointer!!!
	  */
	WidgetDateTimePickerPtr subclassDateTimePicker( unsigned id );

	/// Creates a Radio Button Control and returns a pointer to it.
	/** DON'T delete the returned pointer!!!
	  */
	WidgetRadioButtonPtr createRadioButton( WidgetGroupBoxPtr parent, const typename WidgetRadioButton::Seed & cs = WidgetRadioButton::getDefaultSeed() );

	/// Subclasses a Radio Button Control and returns a pointer to it.
	/** DON'T delete the returned pointer!!!
	  */
	WidgetRadioButtonPtr subclassRadioButton( unsigned id );

	/// Creates a Comb Box and returns a pointer to it.
	/** DON'T delete the returned pointer!!!
	  */
	WidgetComboBoxPtr createComboBox( const typename WidgetComboBox::Seed & cs = WidgetComboBox::getDefaultSeed() );

	/// \ingroup SubclassDialog
	/// Subclasses a ComboBox Control from the given resource id.
	/** DON'T delete the returned pointer!!! <br>
	  * Use e.g. the Dialog Designer to design a dialog and subclass the controls
	  * with this function.
	  */
	WidgetComboBoxPtr subclassComboBox( unsigned id );

	/// Creates a Static Control and returns a pointer to it.
	/** DON'T delete the returned pointer!!!
	  */
	WidgetStaticPtr createStatic( const typename WidgetStatic::Seed & cs = WidgetStatic::getDefaultSeed() );

	/// \ingroup SubclassDialog
	/// Subclasses a Static Control from the given resource id.
	/** DON'T delete the returned pointer!!! <br>
	  * Use e.g. the Dialog Designer to design a dialog and subclass the controls
	  * with this function.
	  */
	WidgetStaticPtr subclassStatic( unsigned id );

protected:
	// Protected to try to avoid stack creation...
	virtual ~WidgetFactory()
	{}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template< template< class > class ContainerWidgetType, class EventHandlerClass >
WidgetFactory< ContainerWidgetType, EventHandlerClass >::WidgetFactory()
		: Widget( 0 )
		, WidgetFactoryPlatformImplementation< ContainerWidgetType, EventHandlerClass, CurrentPlatform >()
{}

template< template< class > class ContainerWidgetType, class EventHandlerClass >
WidgetFactory< ContainerWidgetType, EventHandlerClass >::WidgetFactory( SmartWin::Widget * parent )
		: Widget( parent )
		, WidgetFactoryPlatformImplementation< ContainerWidgetType, EventHandlerClass, CurrentPlatform >( parent )
{}

template< template< class > class ContainerWidgetType, class EventHandlerClass >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass >::WidgetChooseFolder
WidgetFactory< ContainerWidgetType, EventHandlerClass >::createChooseFolder()
{
	WidgetChooseFolder retVal( this );
	return retVal;
}

template< template< class > class ContainerWidgetType, class EventHandlerClass >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass >::WidgetLoadFile
WidgetFactory< ContainerWidgetType, EventHandlerClass >::createLoadFile()
{
	WidgetLoadFile retVal( this );
	return retVal;
}

template< template< class > class ContainerWidgetType, class EventHandlerClass >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass >::WidgetSaveFile
WidgetFactory< ContainerWidgetType, EventHandlerClass >::createSaveFile()
{
	WidgetSaveFile retVal( this );
	return retVal;
}

template< template< class > class ContainerWidgetType, class EventHandlerClass >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass >::WidgetChooseColor
WidgetFactory< ContainerWidgetType, EventHandlerClass >::createChooseColor()
{
	WidgetChooseColor retVal( this );
	return retVal;
}

template< template< class > class ContainerWidgetType, class EventHandlerClass >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass >::WidgetMessageBox
WidgetFactory< ContainerWidgetType, EventHandlerClass >::createMessageBox()
{
	WidgetMessageBox retVal( this );
	return retVal;
}

template< template< class > class ContainerWidgetType, class EventHandlerClass >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass >::WidgetDataGridPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass >::createDataGrid( const typename WidgetDataGrid::Seed & cs )
{
	return WidgetCreator< WidgetDataGrid >::create( this, cs );
}

template< template< class > class ContainerWidgetType, class EventHandlerClass >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass >::WidgetDataGridPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass >::subclassList( unsigned id )
{
	return WidgetCreator< WidgetDataGrid >::subclass( this, id );
}

template< template< class > class ContainerWidgetType, class EventHandlerClass >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass >::WidgetCheckBoxPtr
	WidgetFactory< ContainerWidgetType, EventHandlerClass >::createCheckBox( const typename WidgetCheckBox::Seed & cs )
{
	return WidgetCreator< WidgetCheckBox >::create( this, cs );
}

template< template< class > class ContainerWidgetType, class EventHandlerClass >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass >::WidgetCheckBoxPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass >::subclassCheckBox( unsigned id )
{
	return WidgetCreator< WidgetCheckBox >::subclass( this, id );
}

template< template< class > class ContainerWidgetType, class EventHandlerClass >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass >::WidgetChildWindowPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass >::createWidgetChildWindow( const typename WidgetChildWindow::Seed & cs )
{
	WidgetChildWindowPtr retVal = new WidgetChildWindow( this );
	retVal->createWindow( cs );
	return retVal;
}

template< template< class > class ContainerWidgetType, class EventHandlerClass >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass >::WidgetTreeViewPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass >::createTreeView( const typename WidgetTreeView::Seed & cs )
{
	return WidgetCreator< WidgetTreeView >::create( this, cs );
}

template< template< class > class ContainerWidgetType, class EventHandlerClass >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass >::WidgetTreeViewPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass >::subclassTreeView( unsigned id )
{
	return WidgetCreator< WidgetTreeView >::subclass( this, id );
}

template< template< class > class ContainerWidgetType, class EventHandlerClass >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass >::WidgetMenuPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass >::createMenu(const typename WidgetMenu::Seed & cs)
{
	return WidgetCreator< WidgetMenu >::create( this, cs );
}

template< template< class > class ContainerWidgetType, class EventHandlerClass >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass >::WidgetTextBoxPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass >::createTextBox( const typename WidgetTextBox::Seed & cs )
{
	return WidgetCreator< WidgetTextBox >::create( this, cs );
}

template< template< class > class ContainerWidgetType, class EventHandlerClass >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass >::WidgetTextBoxPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass >::subclassTextBox( unsigned id )
{
	return WidgetCreator< WidgetTextBox >::subclass( this, id );
}

template< template< class > class ContainerWidgetType, class EventHandlerClass >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass >::WidgetStatusBarPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass >::createStatusBar( const typename WidgetStatusBar::Seed & cs )
{
	return WidgetCreator< WidgetStatusBar >::create( this, cs );
}

template< template< class > class ContainerWidgetType, class EventHandlerClass >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass >::WidgetStatusBarSectionsPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass >::createStatusBarSections( const typename WidgetStatusBarSections::Seed & cs )
{
	return WidgetCreator< WidgetStatusBarSections >::create( this, cs );
}

template< template< class > class ContainerWidgetType, class EventHandlerClass >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass >::WidgetButtonPtr
	WidgetFactory< ContainerWidgetType, EventHandlerClass >::createButton( const typename WidgetButton::Seed & cs )
{
	return WidgetCreator< WidgetButton >::create( this, cs );
}

template< template< class > class ContainerWidgetType, class EventHandlerClass >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass >::WidgetMDIParentPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass >::createMDIParent( const typename WidgetMDIParent::Seed & cs )
{
	return WidgetCreator< WidgetMDIParent >::create( this, cs );
}

template< template< class > class ContainerWidgetType, class EventHandlerClass >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass >::WidgetTabSheetPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass >::createTabSheet( const typename WidgetTabSheet::Seed & cs )
{
	return WidgetCreator< WidgetTabSheet >::create( this, cs );
}

template< template< class > class ContainerWidgetType, class EventHandlerClass >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass >::WidgetSliderPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass >::createSlider( const typename WidgetSlider::Seed & cs )
{
	return WidgetCreator< WidgetSlider >::create( this, cs );
}

template< template< class > class ContainerWidgetType, class EventHandlerClass >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass >::WidgetSpinnerPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass >::createSpinner( const typename WidgetSpinner::Seed & cs )
{
	return WidgetCreator< WidgetSpinner >::create( this, cs );
}

template< template< class > class ContainerWidgetType, class EventHandlerClass >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass >::WidgetProgressBarPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass >::createProgressBar( const typename WidgetProgressBar::Seed & cs )
{
	return WidgetCreator< WidgetProgressBar >::create( this, cs );
}

template< template< class > class ContainerWidgetType, class EventHandlerClass >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass >::WidgetButtonPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass >::subclassButton( unsigned id )
{
	return WidgetCreator< WidgetButton >::subclass( this, id );
}


template< template< class > class ContainerWidgetType, class EventHandlerClass >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass >::WidgetProgressBarPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass >::subclassProgressBar( unsigned id )
{
	return WidgetCreator< WidgetProgressBar >::subclass( this, id );
}



template< template< class > class ContainerWidgetType, class EventHandlerClass >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass >::WidgetSliderPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass >::subclassSlider( unsigned id )
{
	return WidgetCreator< WidgetSlider >::subclass( this, id );
}

template< template< class > class ContainerWidgetType, class EventHandlerClass >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass >::WidgetSpinnerPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass >::subclassSpinner( unsigned id )
{
	return WidgetCreator< WidgetSpinner >::subclass( this, id );
}

template< template< class > class ContainerWidgetType, class EventHandlerClass >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass >::WidgetGroupBoxPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass >::createGroupBox( const typename WidgetGroupBox::Seed & cs )
{
	return WidgetCreator< WidgetGroupBox >::create( this, cs );
}

template< template< class > class ContainerWidgetType, class EventHandlerClass >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass >::WidgetGroupBoxPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass >::subclassGroupBox( unsigned id )
{
	return WidgetCreator< WidgetGroupBox >::subclass( this, id );
}

template< template< class > class ContainerWidgetType, class EventHandlerClass >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass >::WidgetDateTimePickerPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass >::createDateTimePicker( const typename WidgetDateTimePicker::Seed & cs )
{
	return WidgetCreator< WidgetDateTimePicker >::create( this, cs );
}

template< template< class > class ContainerWidgetType, class EventHandlerClass >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass >::WidgetDateTimePickerPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass >::subclassDateTimePicker( unsigned id )
{
	return WidgetCreator< WidgetDateTimePicker >::subclass( this, id );
}

template< template< class > class ContainerWidgetType, class EventHandlerClass >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass >::WidgetRadioButtonPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass >::createRadioButton( WidgetGroupBoxPtr parent, const typename WidgetRadioButton::Seed & cs )
{
	WidgetRadioButtonPtr retVal = WidgetCreator< WidgetRadioButton >::create( parent, internal_::getTypedParentOrThrow < EventHandlerClass * >( this ), cs );
	parent->addChild( retVal );
	return retVal;
}

template< template< class > class ContainerWidgetType, class EventHandlerClass >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass >::WidgetRadioButtonPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass >::subclassRadioButton( unsigned id )
{
	WidgetRadioButtonPtr retVal = WidgetCreator< WidgetRadioButton >::subclass( this, id );
	return retVal;
}

template< template< class > class ContainerWidgetType, class EventHandlerClass >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass >::WidgetComboBoxPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass >::createComboBox( const typename WidgetComboBox::Seed & cs )
{
	return WidgetCreator< WidgetComboBox >::create( this, cs );
}

template< template< class > class ContainerWidgetType, class EventHandlerClass >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass >::WidgetComboBoxPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass >::subclassComboBox( unsigned id )
{
	return WidgetCreator< WidgetComboBox >::subclass( this, id );
}

template< template< class > class ContainerWidgetType, class EventHandlerClass >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass >::WidgetStaticPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass >::createStatic( const typename WidgetStatic::Seed & cs )
{
	return WidgetCreator< WidgetStatic >::create( this, cs );
}

template< template< class > class ContainerWidgetType, class EventHandlerClass >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass >::WidgetStaticPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass >::subclassStatic( unsigned id )
{
	return WidgetCreator< WidgetStatic >::subclass( this, id );
}

// end namespace SmartWin
}

#endif

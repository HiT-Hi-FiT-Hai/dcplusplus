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
  * boost::shared_ptr and createMessageBox which returns a stack object, these also
  * should just get to "live their own life" and should not be tampered with in any
  * "memory ways".
  */
template< template< class, class > class ContainerWidgetType, class EventHandlerClass, class MessageMapPolicy = MessageMapPolicyNormalWidget >
class WidgetFactory
	: public WidgetFactoryPlatformImplementation< ContainerWidgetType, EventHandlerClass, MessageMapPolicy, CurrentPlatform >
{
public:
	/// MessageBox class and object type.
	typedef WidgetMessageBox< SmartWin::Widget > WidgetMessageBox;

	/// DataGrid class type.
	typedef WidgetDataGrid< EventHandlerClass, MessageMapPolicy > WidgetDataGrid;

	/// DataGrid object type.
	typedef typename WidgetDataGrid::ObjectType WidgetDataGridPtr;

	/// TreeView class type.
	typedef WidgetTreeView< EventHandlerClass, MessageMapPolicy > WidgetTreeView;

	/// TreeView object type.
	typedef typename WidgetTreeView::ObjectType WidgetTreeViewPtr;

	/// TextBox class type.
	typedef WidgetTextBox< EventHandlerClass, MessageMapPolicy > WidgetTextBox;

	/// TextBox object type.
	typedef typename WidgetTextBox::ObjectType WidgetTextBoxPtr;

	/// StatusBar class type.
	typedef WidgetStatusBar< EventHandlerClass, MessageMapPolicy > WidgetStatusBar;

	/// StatusBar object type.
	typedef typename WidgetStatusBar::ObjectType WidgetStatusBarPtr;

	/// StatusBarSections class type.
	typedef SmartWin::WidgetStatusBar< EventHandlerClass, MessageMapPolicy, Section< EventHandlerClass, MessageMapPolicy > > WidgetStatusBarSections;

	/// StatusBarSections object type.
	typedef typename WidgetStatusBarSections::ObjectType WidgetStatusBarSectionsPtr;

	/// Button class type.
	typedef WidgetButton< EventHandlerClass, MessageMapPolicy > WidgetButton;

	/// Button object type.
	typedef typename WidgetButton::ObjectType WidgetButtonPtr;

	/// MDIWindow class type.
	typedef WidgetMDIParent< EventHandlerClass, MessageMapPolicy > WidgetMDIParent;

	/// MDIWindow object type.
	typedef typename WidgetMDIParent::ObjectType WidgetMDIParentPtr;

	/// TabSheet class type.
	typedef WidgetTabSheet< EventHandlerClass, MessageMapPolicy > WidgetTabSheet;

	/// TabSheet object type.
	typedef typename WidgetTabSheet::ObjectType WidgetTabSheetPtr;

	/// Slider class type.
	typedef WidgetSlider< EventHandlerClass, MessageMapPolicy > WidgetSlider;

	/// Slider object type.
	typedef typename WidgetSlider::ObjectType WidgetSliderPtr;

	/// Spinner class type.
	typedef WidgetSpinner< EventHandlerClass, MessageMapPolicy > WidgetSpinner;

	/// Spinner object type.
	typedef typename WidgetSpinner::ObjectType WidgetSpinnerPtr;

	/// GroupBox class type.
	typedef WidgetGroupBox< EventHandlerClass, MessageMapPolicy > WidgetGroupBox;

	/// GroupBox object type.
	typedef typename WidgetGroupBox::ObjectType WidgetGroupBoxPtr;

	/// RadioButton class type.
	typedef WidgetRadioButton< EventHandlerClass, MessageMapPolicy > WidgetRadioButton;

	/// RadioButton object type.
	typedef typename WidgetRadioButton::ObjectType WidgetRadioButtonPtr;

	/// WidgetChooseFolder class type.
	typedef WidgetChooseFolder< SmartWin::Widget > WidgetChooseFolder;

	/// LoadFileDialog class type.
	typedef WidgetLoadFile< SmartWin::Widget > WidgetLoadFile;

	/// SaveFileDialog class and object type.
	typedef WidgetSaveFile< SmartWin::Widget > WidgetSaveFile;

	/// WidgetChooseColor class and object type.
	typedef WidgetChooseColor< SmartWin::Widget > WidgetChooseColor;

	/// ComboBox class type.
	typedef WidgetComboBox< EventHandlerClass, MessageMapPolicy > WidgetComboBox;

	/// ComboBox object type.
	typedef typename WidgetComboBox::ObjectType WidgetComboBoxPtr;

	/// Static class type.
	typedef WidgetStatic< EventHandlerClass, MessageMapPolicy > WidgetStatic;

	/// Static object type.
	typedef typename WidgetStatic::ObjectType WidgetStaticPtr;

	/// Menu class type.
	typedef WidgetMenu< EventHandlerClass, MessageMapPolicy > WidgetMenu;

	/// Menu object type.
	typedef typename WidgetMenu::ObjectType WidgetMenuPtr;

	/// CheckBox class type.
	typedef WidgetCheckBox< EventHandlerClass, MessageMapPolicy > WidgetCheckBox;

	/// CheckBox object type.
	typedef typename WidgetCheckBox::ObjectType WidgetCheckBoxPtr;

	/// DateTimePicker class type.
	typedef WidgetDateTimePicker< EventHandlerClass, MessageMapPolicy > WidgetDateTimePicker;

	/// DateTimePicker object type.
	typedef typename WidgetDateTimePicker::ObjectType WidgetDateTimePickerPtr;

	//TODO: If we set the policyclass to the MessageMapPolicy (which appears
	//TODO: logicaly) we get bugs when creating widget child widgets within e.g. a
	//TODO: MDIChild or a WidgetDialog since the creational params aren't right...
	//TODO: If we set it like it is now we basically can't create a
	//TODO: WidgetChildWidget and trap events in the container widget which is it's
	//TODO: parent since the type would be wrong...

	/// WidgetChildWindow class type.
	// The following line would work with an appropiate WidgetChildWindow::create
	//typedef SmartWin::WidgetFactory< SmartWin::WidgetWindow, EventHandlerClass, MessageMapPolicy > WidgetChildWindow;
	typedef SmartWin::WidgetFactory< SmartWin::WidgetChildWindow, EventHandlerClass, MessageMapPolicyNormalWidget > WidgetChildWindow;

	/// WidgetChildWindow object type.
	// The following line would work with an appropiate WidgetChildWindow::create
	//typedef SmartWin::WidgetFactory< SmartWin::WidgetWindow, EventHandlerClass, MessageMapPolicy > * WidgetChildWindowPtr;
	typedef SmartWin::WidgetFactory< SmartWin::WidgetChildWindow, EventHandlerClass, MessageMapPolicyNormalWidget > * WidgetChildWindowPtr;

	/// WidgetWindow class type.
	typedef SmartWin::WidgetFactory< SmartWin::WidgetWindow, EventHandlerClass, MessageMapPolicyNormalWidget > WidgetWindow;

	/// WidgetWindow object type.
	typedef SmartWin::WidgetFactory< SmartWin::WidgetWindow, EventHandlerClass, MessageMapPolicyNormalWidget > * WidgetWindowPtr;

	/// ProgressBar class type.
	typedef WidgetProgressBar< EventHandlerClass, MessageMapPolicy > WidgetProgressBar;

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
	/** The returned object is of type boost::shared_ptr< WidgetMenu >, but you should use the typedef WidgetMenuPtr and not < br >
	  * the shared_ptr itself since this may change in future releases.
	  */
	WidgetMenuPtr createMenu();

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

template< template< class, class > class ContainerWidgetType, class EventHandlerClass, class MessageMapPolicy >
WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::WidgetFactory()
		: Widget( 0 )
		, WidgetFactoryPlatformImplementation< ContainerWidgetType, EventHandlerClass, MessageMapPolicy, CurrentPlatform >()
{}

template< template< class, class > class ContainerWidgetType, class EventHandlerClass, class MessageMapPolicy >
WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::WidgetFactory( SmartWin::Widget * parent )
		: Widget( parent )
		, WidgetFactoryPlatformImplementation< ContainerWidgetType, EventHandlerClass, MessageMapPolicy, CurrentPlatform >( parent )
{}

template< template< class, class > class ContainerWidgetType, class EventHandlerClass, class MessageMapPolicy >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::WidgetChooseFolder
WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::createChooseFolder()
{
	WidgetChooseFolder retVal( this );
	return retVal;
}

template< template< class, class > class ContainerWidgetType, class EventHandlerClass, class MessageMapPolicy >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::WidgetLoadFile
WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::createLoadFile()
{
	WidgetLoadFile retVal( this );
	return retVal;
}

template< template< class, class > class ContainerWidgetType, class EventHandlerClass, class MessageMapPolicy >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::WidgetSaveFile
WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::createSaveFile()
{
	WidgetSaveFile retVal( this );
	return retVal;
}

template< template< class, class > class ContainerWidgetType, class EventHandlerClass, class MessageMapPolicy >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::WidgetChooseColor
WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::createChooseColor()
{
	WidgetChooseColor retVal( this );
	return retVal;
}

template< template< class, class > class ContainerWidgetType, class EventHandlerClass, class MessageMapPolicy >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::WidgetMessageBox
WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::createMessageBox()
{
	WidgetMessageBox retVal( this );
	return retVal;
}

template< template< class, class > class ContainerWidgetType, class EventHandlerClass, class MessageMapPolicy >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::WidgetDataGridPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::createDataGrid( const typename WidgetDataGrid::Seed & cs )
{
	return WidgetCreator< WidgetDataGrid >::create( this, cs );
}

template< template< class, class > class ContainerWidgetType, class EventHandlerClass, class MessageMapPolicy >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::WidgetDataGridPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::subclassList( unsigned id )
{
	// If this one fizzles you have tried to call this function from a derived
	// class which not is derived from MessageMapPolicyDialogWidget, like for
	// instance both the MessageMapPolicyNormalWidget and the
	// MessageMapPolicyMDIChildWidget classes cannot logically subclass a dialog
	// item since they're NOT dialog Widgets therefore they will give you a compile
	// error if you try to call this function from Widgets implementing the those
	// classes! Only the MessageMapPolicyDialogWidget can logically call this
	// function and therefore it's the only one which will not give you a compile
	// time error here...
	typename MessageMapPolicy::canSubclassControls checker;
	return WidgetCreator< WidgetDataGrid >::subclass( this, id );
}

template< template< class, class > class ContainerWidgetType, class EventHandlerClass, class MessageMapPolicy >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::WidgetCheckBoxPtr
	WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::createCheckBox( const typename WidgetCheckBox::Seed & cs )
{
	return WidgetCreator< WidgetCheckBox >::create( this, cs );
}

template< template< class, class > class ContainerWidgetType, class EventHandlerClass, class MessageMapPolicy >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::WidgetCheckBoxPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::subclassCheckBox( unsigned id )
{
	// If this one fizzles you have tried to call this function from a derived
	// class which not is derived from MessageMapPolicyDialogWidget, like for
	// instance both the MessageMapPolicyNormalWidget and the
	// MessageMapPolicyMDIChildWidget classes cannot logically subclass a dialog
	// item since they're NOT dialog Widgets therefore they will give you a compile
	// error if you try to call this function from Widgets implementing the those
	// classes! Only the MessageMapPolicyDialogWidget can logically call this
	// function and therefore it's the only one which will not give you a compile
	// time error here...
	typename MessageMapPolicy::canSubclassControls checker;
	return WidgetCreator< WidgetCheckBox >::subclass( this, id );
}

template< template< class, class > class ContainerWidgetType, class EventHandlerClass, class MessageMapPolicy >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::WidgetChildWindowPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::createWidgetChildWindow( const typename WidgetChildWindow::Seed & cs )
{
	WidgetChildWindowPtr retVal = new WidgetChildWindow( this );
	retVal->createWindow( cs );
	return retVal;
}

template< template< class, class > class ContainerWidgetType, class EventHandlerClass, class MessageMapPolicy >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::WidgetTreeViewPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::createTreeView( const typename WidgetTreeView::Seed & cs )
{
	return WidgetCreator< WidgetTreeView >::create( this, cs );
}

template< template< class, class > class ContainerWidgetType, class EventHandlerClass, class MessageMapPolicy >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::WidgetTreeViewPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::subclassTreeView( unsigned id )
{
	// If this one fizzles you have tried to call this function from a derived
	// class which not is derived from MessageMapPolicyDialogWidget, like for
	// instance both the MessageMapPolicyNormalWidget and the
	// MessageMapPolicyMDIChildWidget classes cannot logically subclass a dialog
	// item since they're NOT dialog Widgets therefore they will give you a compile
	// error if you try to call this function from Widgets implementing the those
	// classes! Only the MessageMapPolicyDialogWidget can logically call this
	// function and therefore it's the only one which will not give you a compile
	// time error here...
	typename MessageMapPolicy::canSubclassControls checker;
	return WidgetCreator< WidgetTreeView >::subclass( this, id );
}

template< template< class, class > class ContainerWidgetType, class EventHandlerClass, class MessageMapPolicy >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::WidgetMenuPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::createMenu()
{
	return WidgetCreator< WidgetMenu >::create( this );
}

template< template< class, class > class ContainerWidgetType, class EventHandlerClass, class MessageMapPolicy >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::WidgetTextBoxPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::createTextBox( const typename WidgetTextBox::Seed & cs )
{
	return WidgetCreator< WidgetTextBox >::create( this, cs );
}

template< template< class, class > class ContainerWidgetType, class EventHandlerClass, class MessageMapPolicy >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::WidgetTextBoxPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::subclassTextBox( unsigned id )
{
	// If this one fizzles you have tried to call this function from a derived
	// class which not is derived from MessageMapPolicyDialogWidget, like for
	// instance both the MessageMapPolicyNormalWidget and the
	// MessageMapPolicyMDIChildWidget classes cannot logically subclass a dialog
	// item since they're NOT dialog Widgets therefore they will give you a compile
	// error if you try to call this function from Widgets implementing the those
	// classes! Only the MessageMapPolicyDialogWidget can logically call this
	// function and therefore it's the only one which will not give you a compile
	// time error here...
	typename MessageMapPolicy::canSubclassControls checker;
	return WidgetCreator< WidgetTextBox >::subclass( this, id );
}

template< template< class, class > class ContainerWidgetType, class EventHandlerClass, class MessageMapPolicy >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::WidgetStatusBarPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::createStatusBar( const typename WidgetStatusBar::Seed & cs )
{
	return WidgetCreator< WidgetStatusBar >::create( this, cs );
}

template< template< class, class > class ContainerWidgetType, class EventHandlerClass, class MessageMapPolicy >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::WidgetStatusBarSectionsPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::createStatusBarSections( const typename WidgetStatusBarSections::Seed & cs )
{
	return WidgetCreator< WidgetStatusBarSections >::create( this, cs );
}

template< template< class, class > class ContainerWidgetType, class EventHandlerClass, class MessageMapPolicy >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::WidgetButtonPtr
	WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::createButton( const typename WidgetButton::Seed & cs )
{
	return WidgetCreator< WidgetButton >::create( this, cs );
}

template< template< class, class > class ContainerWidgetType, class EventHandlerClass, class MessageMapPolicy >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::WidgetMDIParentPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::createMDIParent( const typename WidgetMDIParent::Seed & cs )
{
	return WidgetCreator< WidgetMDIParent >::create( this, cs );
}

template< template< class, class > class ContainerWidgetType, class EventHandlerClass, class MessageMapPolicy >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::WidgetTabSheetPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::createTabSheet( const typename WidgetTabSheet::Seed & cs )
{
	return WidgetCreator< WidgetTabSheet >::create( this, cs );
}

template< template< class, class > class ContainerWidgetType, class EventHandlerClass, class MessageMapPolicy >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::WidgetSliderPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::createSlider( const typename WidgetSlider::Seed & cs )
{
	return WidgetCreator< WidgetSlider >::create( this, cs );
}

template< template< class, class > class ContainerWidgetType, class EventHandlerClass, class MessageMapPolicy >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::WidgetSpinnerPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::createSpinner( const typename WidgetSpinner::Seed & cs )
{
	return WidgetCreator< WidgetSpinner >::create( this, cs );
}

template< template< class, class > class ContainerWidgetType, class EventHandlerClass, class MessageMapPolicy >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::WidgetProgressBarPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::createProgressBar( const typename WidgetProgressBar::Seed & cs )
{
	return WidgetCreator< WidgetProgressBar >::create( this, cs );
}

template< template< class, class > class ContainerWidgetType, class EventHandlerClass, class MessageMapPolicy >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::WidgetButtonPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::subclassButton( unsigned id )
{
	// If this one fizzles you have tried to call this function from a derived
	// class which not is derived from MessageMapPolicyDialogWidget, like for
	// instance both the MessageMapPolicyNormalWidget and the
	// MessageMapPolicyMDIChildWidget classes cannot logically subclass a dialog
	// item since they're NOT dialog Widgets therefore they will give you a compile
	// error if you try to call this function from Widgets implementing the those
	// classes! Only the MessageMapPolicyDialogWidget can logically call this
	// function and therefore it's the only one which will not give you a compile
	// time error here...
	typename MessageMapPolicy::canSubclassControls checker;
	return WidgetCreator< WidgetButton >::subclass( this, id );
}


template< template< class, class > class ContainerWidgetType, class EventHandlerClass, class MessageMapPolicy >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::WidgetProgressBarPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::subclassProgressBar( unsigned id )
{
	// If this one fizzles you have tried to call this function from a derived
	// class which not is derived from MessageMapPolicyDialogWidget, like for
	// instance both the MessageMapPolicyNormalWidget and the
	// MessageMapPolicyMDIChildWidget classes cannot logically subclass a dialog
	// item since they're NOT dialog Widgets therefore they will give you a compile
	// error if you try to call this function from Widgets implementing the those
	// classes! Only the MessageMapPolicyDialogWidget can logically call this
	// function and therefore it's the only one which will not give you a compile
	// time error here...
	typename MessageMapPolicy::canSubclassControls checker;
	return WidgetCreator< WidgetProgressBar >::subclass( this, id );
}



template< template< class, class > class ContainerWidgetType, class EventHandlerClass, class MessageMapPolicy >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::WidgetSliderPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::subclassSlider( unsigned id )
{
	// If this one fizzles you have tried to call this function from a derived
	// class which not is derived from MessageMapPolicyDialogWidget, like for
	// instance both the MessageMapPolicyNormalWidget and the
	// MessageMapPolicyMDIChildWidget classes cannot logically subclass a dialog
	// item since they're NOT dialog Widgets therefore they will give you a compile
	// error if you try to call this function from Widgets implementing the those
	// classes! Only the MessageMapPolicyDialogWidget can logically call this
	// function and therefore it's the only one which will not give you a compile
	// time error here...
	typename MessageMapPolicy::canSubclassControls checker;
	return WidgetCreator< WidgetSlider >::subclass( this, id );
}

template< template< class, class > class ContainerWidgetType, class EventHandlerClass, class MessageMapPolicy >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::WidgetSpinnerPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::subclassSpinner( unsigned id )
{
	// If this one fizzles you have tried to call this function from a derived
	// class which not is derived from MessageMapPolicyDialogWidget, like for
	// instance both the MessageMapPolicyNormalWidget and the
	// MessageMapPolicyMDIChildWidget classes cannot logically subclass a dialog
	// item since they're NOT dialog Widgets therefore they will give you a compile
	// error if you try to call this function from Widgets implementing the those
	// classes! Only the MessageMapPolicyDialogWidget can logically call this
	// function and therefore it's the only one which will not give you a compile
	// time error here...
	typename MessageMapPolicy::canSubclassControls checker;
	return WidgetCreator< WidgetSpinner >::subclass( this, id );
}

template< template< class, class > class ContainerWidgetType, class EventHandlerClass, class MessageMapPolicy >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::WidgetGroupBoxPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::createGroupBox( const typename WidgetGroupBox::Seed & cs )
{
	return WidgetCreator< WidgetGroupBox >::create( this, cs );
}

template< template< class, class > class ContainerWidgetType, class EventHandlerClass, class MessageMapPolicy >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::WidgetGroupBoxPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::subclassGroupBox( unsigned id )
{
	// If this one fizzles you have tried to call this function from a derived
	// class which not is derived from MessageMapPolicyDialogWidget, like for
	// instance both the MessageMapPolicyNormalWidget and the
	// MessageMapPolicyMDIChildWidget classes cannot logically subclass a dialog
	// item since they're NOT dialog Widgets therefore they will give you a compile
	// error if you try to call this function from Widgets implementing the those
	// classes! Only the MessageMapPolicyDialogWidget can logically call this
	// function and therefore it's the only one which will not give you a compile
	// time error here...
	typename MessageMapPolicy::canSubclassControls checker;
	return WidgetCreator< WidgetGroupBox >::subclass( this, id );
}

template< template< class, class > class ContainerWidgetType, class EventHandlerClass, class MessageMapPolicy >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::WidgetDateTimePickerPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::createDateTimePicker( const typename WidgetDateTimePicker::Seed & cs )
{
	return WidgetCreator< WidgetDateTimePicker >::create( this, cs );
}

template< template< class, class > class ContainerWidgetType, class EventHandlerClass, class MessageMapPolicy >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::WidgetDateTimePickerPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::subclassDateTimePicker( unsigned id )
{
	// If this one fizzles you have tried to call this function from a derived
	// class which not is derived from MessageMapPolicyDialogWidget, like for
	// instance both the MessageMapPolicyNormalWidget and the
	// MessageMapPolicyMDIChildWidget classes cannot logically subclass a dialog
	// item since they're NOT dialog Widgets therefore they will give you a compile
	// error if you try to call this function from Widgets implementing the those
	// classes! Only the MessageMapPolicyDialogWidget can logically call this
	// function and therefore it's the only one which will not give you a compile
	// time error here...
	typename MessageMapPolicy::canSubclassControls checker;
	return WidgetCreator< WidgetDateTimePicker >::subclass( this, id );
}

template< template< class, class > class ContainerWidgetType, class EventHandlerClass, class MessageMapPolicy >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::WidgetRadioButtonPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::createRadioButton( WidgetGroupBoxPtr parent, const typename WidgetRadioButton::Seed & cs )
{
	WidgetRadioButtonPtr retVal = WidgetCreator< WidgetRadioButton >::create( parent, internal_::getTypedParentOrThrow < EventHandlerClass * >( this ), cs );
	parent->addChild( retVal );
	return retVal;
}

template< template< class, class > class ContainerWidgetType, class EventHandlerClass, class MessageMapPolicy >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::WidgetRadioButtonPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::subclassRadioButton( unsigned id )
{
	// If this one fizzles you have tried to call this function from a derived
	// class which not is derived from MessageMapPolicyDialogWidget, like for
	// instance both the MessageMapPolicyNormalWidget and the
	// MessageMapPolicyMDIChildWidget classes cannot logically subclass a dialog
	// item since they're NOT dialog Widgets therefore they will give you a compile
	// error if you try to call this function from Widgets implementing the those
	// classes! Only the MessageMapPolicyDialogWidget can logically call this
	// function and therefore it's the only one which will not give you a compile
	// time error here...
	typename MessageMapPolicy::canSubclassControls checker;
	WidgetRadioButtonPtr retVal = WidgetCreator< WidgetRadioButton >::subclass( this, id );
	return retVal;
}

template< template< class, class > class ContainerWidgetType, class EventHandlerClass, class MessageMapPolicy >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::WidgetComboBoxPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::createComboBox( const typename WidgetComboBox::Seed & cs )
{
	return WidgetCreator< WidgetComboBox >::create( this, cs );
}

template< template< class, class > class ContainerWidgetType, class EventHandlerClass, class MessageMapPolicy >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::WidgetComboBoxPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::subclassComboBox( unsigned id )
{
	// If this one fizzles you have tried to call this function from a derived
	// class which not is derived from MessageMapPolicyDialogWidget, like for
	// instance both the MessageMapPolicyNormalWidget and the
	// MessageMapPolicyMDIChildWidget classes cannot logically subclass a dialog
	// item since they're NOT dialog Widgets therefore they will give you a compile
	// error if you try to call this function from Widgets implementing the those
	// classes! Only the MessageMapPolicyDialogWidget can logically call this
	// function and therefore it's the only one which will not give you a compile
	// time error here...
	typename MessageMapPolicy::canSubclassControls checker;
	return WidgetCreator< WidgetComboBox >::subclass( this, id );
}

template< template< class, class > class ContainerWidgetType, class EventHandlerClass, class MessageMapPolicy >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::WidgetStaticPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::createStatic( const typename WidgetStatic::Seed & cs )
{
	return WidgetCreator< WidgetStatic >::create( this, cs );
}

template< template< class, class > class ContainerWidgetType, class EventHandlerClass, class MessageMapPolicy >
typename WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::WidgetStaticPtr
WidgetFactory< ContainerWidgetType, EventHandlerClass, MessageMapPolicy >::subclassStatic( unsigned id )
{
	// If this one fizzles you have tried to call this function from a derived
	// class which not is derived from MessageMapPolicyDialogWidget, like for
	// instance both the MessageMapPolicyNormalWidget and the
	// MessageMapPolicyMDIChildWidget classes cannot logically subclass a dialog
	// item since they're NOT dialog Widgets therefore they will give you a compile
	// error if you try to call this function from Widgets implementing the those
	// classes! Only the MessageMapPolicyDialogWidget can logically call this
	// function and therefore it's the only one which will not give you a compile
	// time error here...
	typename MessageMapPolicy::canSubclassControls checker;
	return WidgetCreator< WidgetStatic >::subclass( this, id );
}

// end namespace SmartWin
}

#endif

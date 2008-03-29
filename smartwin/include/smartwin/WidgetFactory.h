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

#include "../SmartUtil.h"
#include "widgets/Button.h"
#include "widgets/ColorDialog.h"
#include "widgets/FolderDialog.h"
#include "widgets/MessageBox.h"
#include "widgets/CheckBox.h"
#include "widgets/ComboBox.h"
#include "widgets/Table.h"
#include "widgets/DateTime.h"
#include "widgets/WidgetDialog.h"
#include "widgets/GroupBox.h"
#include "widgets/LoadDialog.h"
#include "widgets/MDIChild.h"
#include "widgets/MDIFrame.h"
#include "widgets/MDIParent.h"
#include "widgets/ProgressBar.h"
#include "widgets/RadioButton.h"
#include "widgets/SaveDialog.h"
#include "widgets/Slider.h"
#include "widgets/Spinner.h"
#include "widgets/Label.h"
#include "widgets/StatusBar.h"
#include "widgets/WidgetTabView.h"
#include "widgets/TabSheet.h"
#include "widgets/TextBox.h"
#include "widgets/Tree.h"
#include "widgets/ToolTip.h"
#include "widgets/WidgetWindow.h"
#include "widgets/WidgetWindowBase.h"
#include "WidgetFactoryPlatformImplementation.h"
#include "WidgetCreator.h"

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
	
	typedef SmartWin::Button Button;
	typedef typename Button::ObjectType ButtonPtr;

	typedef SmartWin::FolderDialog FolderDialog;

	
	/// MessageBox class and object type.
	typedef SmartWin::MessageBox MessageBox;

	/// DataGrid class type.
	typedef SmartWin::Table Table;

	/// DataGrid object type.
	typedef typename Table::ObjectType TablePtr;

	/// TreeView class type.
	typedef SmartWin::Tree Tree;

	/// TreeView object type.
	typedef typename Tree::ObjectType TreePtr;

	/// TextBox class type.
	typedef SmartWin::TextBox TextBox;

	/// TextBox object type.
	typedef typename TextBox::ObjectType TextBoxPtr;

	/// StatusBar class type.
	typedef SmartWin::StatusBar< > StatusBar;

	/// StatusBar object type.
	typedef typename StatusBar::ObjectType StatusBarPtr;

	/// StatusBarSections class type.
	typedef SmartWin::StatusBar< Section > StatusBarSections;

	/// StatusBarSections object type.
	typedef typename StatusBarSections::ObjectType StatusBarSectionsPtr;

	/// MDIWindow class type.
	typedef SmartWin::MDIParent MDIParent;

	/// MDIWindow object type.
	typedef typename MDIParent::ObjectType MDIParentPtr;

	/// TabView class type.
	typedef SmartWin::WidgetTabView WidgetTabView;

	/// TabView object type.
	typedef typename WidgetTabView::ObjectType WidgetTabViewPtr;

	/// TabSheet class type.
	typedef SmartWin::TabSheet TabSheet;

	/// TabSheet object type.
	typedef typename TabSheet::ObjectType TabSheetPtr;

	/// Slider class type.
	typedef SmartWin::Slider Slider;

	/// Slider object type.
	typedef typename Slider::ObjectType SliderPtr;

	/// Spinner class type.
	typedef SmartWin::Spinner Spinner;

	/// Spinner object type.
	typedef typename Spinner::ObjectType SpinnerPtr;

	/// GroupBox class type.
	typedef SmartWin::GroupBox GroupBox;

	/// GroupBox object type.
	typedef typename GroupBox::ObjectType GroupBoxPtr;

	/// RadioButton class type.
	typedef SmartWin::RadioButton RadioButton;

	/// RadioButton object type.
	typedef typename RadioButton::ObjectType RadioButtonPtr;

	/// LoadFileDialog class type.
	typedef SmartWin::LoadDialog LoadDialog;

	/// SaveFileDialog class and object type.
	typedef SmartWin::SaveDialog SaveDialog;

	/// ColorDialog class and object type.
	typedef SmartWin::ColorDialog ColorDialog;

	/// ComboBox class type.
	typedef SmartWin::ComboBox ComboBox;

	/// ComboBox object type.
	typedef typename ComboBox::ObjectType ComboBoxPtr;

	/// Static class type.
	typedef SmartWin::Label Label;

	/// Static object type.
	typedef typename Label::ObjectType LabelPtr;

	/// CheckBox class type.
	typedef SmartWin::CheckBox CheckBox;

	/// CheckBox object type.
	typedef typename CheckBox::ObjectType CheckBoxPtr;

	/// DateTimePicker class type.
	typedef SmartWin::DateTime DateTime;

	/// DateTimePicker object type.
	typedef typename DateTime::ObjectType DateTimePtr;

	/// WidgetChildWindow class type.
	typedef SmartWin::WidgetChildWindow WidgetChildWindow;

	/// WidgetChildWindow object type.
	typedef typename WidgetChildWindow::ObjectType WidgetChildWindowPtr;

	/// WidgetWindow class type.
	typedef SmartWin::WidgetWindow WidgetWindow;

	/// WidgetWindow object type.
	typedef typename WidgetWindow::ObjectType WidgetWindowPtr;

	/// MDIFrame class type.
	typedef SmartWin::MDIFrame MDIFrame;

	/// WidgetWindow object type.
	typedef typename MDIFrame::ObjectType MDIFramePtr;

	/// ProgressBar class type.
	typedef SmartWin::ProgressBar ProgressBar;

	/// ProgressBar object type.
	typedef typename ProgressBar::ObjectType ProgressBarPtr;

	typedef SmartWin::ToolTip ToolTip;
	
	typedef typename ToolTip::ObjectType ToolTipPtr;
	
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
	LoadDialog createLoadFile();

	/// Creates a SaveFileDialog and returns a pointer to it.
	/** Use this one to construct a ( stack object ) to show a Save File Dialog
	  */
	SaveDialog createSaveFile();

	/// Creates a ColorDialog and returns it.
	/** Usable to let user choose font from the system installed fonts.
	  */
	ColorDialog createColorDialog();

	// TODO: Update, this isn't an automated collected Widget anymore...
	/// Creates a Message Box and returns a pointer to it.
	/** Use this one to construct a ( stack object ) to show a message box
	  */
	MessageBox createMessageBox();

	/// Creates a List View and returns a pointer to it.
	/** DON'T delete the returned pointer!!!
	  */
	TablePtr createTable( const typename Table::Seed & cs = Table::Seed() );

	/// \ingroup SubclassDialog
	/// Subclasses a Check Box from the given resource id.
	/** DON'T delete the returned pointer!!! <br>
	  * Use e.g. the Dialog Designer to design a dialog and attach the controls
	  * with this function.
	  */
	TablePtr attachList( unsigned id );

	/// Creates a Check Box and returns a pointer to it.
	/** DON'T delete the returned pointer!!!
	  */
	CheckBoxPtr createCheckBox( const typename CheckBox::Seed & cs = CheckBox::Seed() );

	/// \ingroup SubclassDialog
	/// Subclasses a Check Box from the given resource id.
	/** DON'T delete the returned pointer!!! <br>
	  * Use e.g. the Dialog Designer to design a dialog and attach the controls
	  * with this function.
	  */
	CheckBoxPtr attachCheckBox( unsigned id );

	/// Creates a child window and returns a pointer to it.
	/** DON'T delete the returned pointer!!!
	  */
	WidgetChildWindowPtr createWidgetChildWindow( const typename WidgetChildWindow::Seed & cs = WidgetChildWindow::Seed() );

	/// Creates a Tree View and returns a pointer to it.
	/** DON'T delete the returned pointer!!!
	  */
	TreePtr createTreeView( const typename Tree::Seed & cs = Tree::Seed() );

	/// \ingroup SubclassDialog
	/// Subclasses a Tree View Control from the given resource id.
	/** DON'T delete the returned pointer!!! <br>
	  * Use e.g. the Dialog Designer to design a dialog and attach the controls
	  * with this function.
	  */
	TreePtr attachTreeView( unsigned id );

	/// Creates a Edit Control and returns a pointer to it.
	/** DON'T delete the returned pointer!!!
	  */
	TextBoxPtr createTextBox( const typename TextBox::Seed & cs = TextBox::Seed() );

	/// \ingroup SubclassDialog
	/// Subclasses a Text Box Control from the given resource id.
	/** DON'T delete the returned pointer!!! <br>
	  * Use e.g. the Dialog Designer to design a dialog and attach the controls
	  * with this function.
	  */
	TextBoxPtr attachTextBox( unsigned id );

	// TODO: Is there any point in attaching a status bar ? ! ?
	/// Creates a Status Bar and returns a pointer to it.
	/** DON'T delete the returned pointer!!!
	  */
	StatusBarPtr createStatusBar( const typename StatusBar::Seed & cs = StatusBar::Seed() );

	/// Creates a Status Bar and returns a pointer to it.
	/** DON'T delete the returned pointer!!!
	  */
	StatusBarSectionsPtr createStatusBarSections( const typename StatusBarSections::Seed & cs = StatusBarSections::Seed() );

	/// Creates a Button Control and returns a pointer to it.
	/** DON'T delete the returned pointer!!!
	  */
	ButtonPtr createButton( const typename Button::Seed & cs = Button::Seed() );

	/// Creates a Button Control and returns a pointer to it.
	/** DON'T delete the returned pointer!!!
	  */
	MDIParentPtr createMDIParent( const typename MDIParent::Seed & cs = MDIParent::Seed() );

	/// Creates a Tab View and returns a pointer to it.
	/** DON'T delete the returned pointer!!!
	  */
	WidgetTabViewPtr createTabView( const typename WidgetTabView::Seed& cs = WidgetTabView::Seed() );
	
	/// Creates a Tab Sheet and returns a pointer to it.
	/** DON'T delete the returned pointer!!!
	  */
	TabSheetPtr createTabSheet( const typename TabSheet::Seed & cs = TabSheet::Seed() );

	/// Creates a Slider Control and returns a pointer to it.
	/** DON'T delete the returned pointer!!!
	  */
	SliderPtr createSlider( const typename Slider::Seed & cs = Slider::Seed() );

	/// Creates a Spinner Control and returns a pointer to it.
	/** DON'T delete the returned pointer!!!
	  */
	SpinnerPtr createSpinner( const typename Spinner::Seed & cs = Spinner::Seed() );

	/// Creates a Progress Bar Control and returns a pointer to it.
	/** DON'T delete the returned pointer!!!
	  */
	ProgressBarPtr createProgressBar( const typename ProgressBar::Seed & cs = ProgressBar::Seed() );

	/// \ingroup SubclassDialog
	/// Subclasses a Button Control from the given resource id.
	/** DON'T delete the returned pointer!!!< br >
	  * Use e.g. the Dialog Designer to design a dialog and attach the controls with this function.
	  */
	ButtonPtr attachButton( unsigned id );

	/// \ingroup SubclassDialog
	/// Subclasses a Progress Bar Control from the given resource id.
	/** DON'T delete the returned pointer!!!< br >
	  * Use e.g. the Dialog Designer to design a dialog and attach the controls with this function.
	  */
	ProgressBarPtr attachProgressBar( unsigned id );

	/// \ingroup SubclassDialog
	/// Subclasses a Slider Control from the given resource id.
	/** DON'T delete the returned pointer!!! <br>
	  * Use e.g. the Dialog Designer to design a dialog and attach the controls
	  * with this function.
	  */
	SliderPtr attachSlider( unsigned id );

	/// \ingroup SubclassDialog
	/// Subclasses a Spinner Control from the given resource id.
	/** DON'T delete the returned pointer!!! <br>
	  * Use e.g. the Dialog Designer to design a dialog and attach the controls
	  * with this function.
	  */
	SpinnerPtr attachSpinner( unsigned id );

	/// Creates a Group Box Control and returns a pointer to it.
	/** DON'T delete the returned pointer!!!
	  */
	GroupBoxPtr createGroupBox( const typename GroupBox::Seed & cs = GroupBox::Seed() );

	/// Subclasses a Group Box Control and returns a pointer to it.
	/** DON'T delete the returned pointer!!!
	  */
	GroupBoxPtr attachGroupBox( unsigned id );

	/// Creates a DateTime Picker Control and returns a pointer to it.
	/** DON'T delete the returned pointer!!!
	  */
	DateTimePtr createDateTimePicker( const typename DateTime::Seed & cs = DateTime::Seed() );

	/// Suvclasses a DateTime Picker Control and returns a pointer to it.
	/** DON'T delete the returned pointer!!!
	  */
	DateTimePtr attachDateTimePicker( unsigned id );

	/// Creates a Radio Button Control and returns a pointer to it.
	/** DON'T delete the returned pointer!!!
	  */
	RadioButtonPtr createRadioButton( GroupBoxPtr parent, const typename RadioButton::Seed & cs = RadioButton::Seed() );

	/// Subclasses a Radio Button Control and returns a pointer to it.
	/** DON'T delete the returned pointer!!!
	  */
	RadioButtonPtr attachRadioButton( unsigned id );

	/// Creates a Comb Box and returns a pointer to it.
	/** DON'T delete the returned pointer!!!
	  */
	ComboBoxPtr createComboBox( const typename ComboBox::Seed & cs = ComboBox::Seed() );

	/// \ingroup SubclassDialog
	/// Subclasses a ComboBox Control from the given resource id.
	/** DON'T delete the returned pointer!!! <br>
	  * Use e.g. the Dialog Designer to design a dialog and attach the controls
	  * with this function.
	  */
	ComboBoxPtr attachComboBox( unsigned id );

	/// Creates a Static Control and returns a pointer to it.
	/** DON'T delete the returned pointer!!!
	  */
	LabelPtr createStatic( const typename Label::Seed & cs = Label::Seed() );

	/// \ingroup SubclassDialog
	/// Subclasses a Static Control from the given resource id.
	/** DON'T delete the returned pointer!!! <br>
	  * Use e.g. the Dialog Designer to design a dialog and attach the controls
	  * with this function.
	  */
	LabelPtr attachStatic( unsigned id );

	ToolTipPtr createToolTip( const typename ToolTip::Seed & cs = ToolTip::Seed() );
protected:
	// Protected to try to avoid stack creation...
	virtual ~WidgetFactory()
	{}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename ContainerWidgetType>
WidgetFactory< ContainerWidgetType >::WidgetFactory( SmartWin::Widget * parent )
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
WidgetFactory< ContainerWidgetType >::createLoadFile()
{
	return LoadDialog ( this );
}

template<typename ContainerWidgetType>
typename WidgetFactory< ContainerWidgetType >::SaveDialog
WidgetFactory< ContainerWidgetType >::createSaveFile()
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

template<typename ContainerWidgetType>
typename WidgetFactory< ContainerWidgetType >::TablePtr
WidgetFactory< ContainerWidgetType >::createTable( const typename Table::Seed & cs )
{
	return WidgetCreator< Table >::create( this, cs );
}

template<typename ContainerWidgetType>
typename WidgetFactory< ContainerWidgetType >::TablePtr
WidgetFactory< ContainerWidgetType >::attachList( unsigned id )
{
	return WidgetCreator< Table >::attach( this, id );
}

template<typename ContainerWidgetType>
typename WidgetFactory< ContainerWidgetType >::CheckBoxPtr
	WidgetFactory< ContainerWidgetType >::createCheckBox( const typename CheckBox::Seed & cs )
{
	return WidgetCreator< CheckBox >::create( this, cs );
}

template<typename ContainerWidgetType>
typename WidgetFactory< ContainerWidgetType >::CheckBoxPtr
WidgetFactory< ContainerWidgetType >::attachCheckBox( unsigned id )
{
	return WidgetCreator< CheckBox >::attach( this, id );
}

template<typename ContainerWidgetType>
typename WidgetFactory< ContainerWidgetType >::WidgetChildWindowPtr
WidgetFactory< ContainerWidgetType >::createWidgetChildWindow( const typename WidgetChildWindow::Seed & cs )
{
	return WidgetCreator< WidgetChildWindow >::createWindow( this, cs );
}

template<typename ContainerWidgetType>
typename WidgetFactory< ContainerWidgetType >::TreePtr
WidgetFactory< ContainerWidgetType >::createTreeView( const typename Tree::Seed & cs )
{
	return WidgetCreator< Tree >::create( this, cs );
}

template<typename ContainerWidgetType>
typename WidgetFactory< ContainerWidgetType >::TreePtr
WidgetFactory< ContainerWidgetType >::attachTreeView( unsigned id )
{
	return WidgetCreator< Tree >::attach( this, id );
}

template<typename ContainerWidgetType>
typename WidgetFactory< ContainerWidgetType >::TextBoxPtr
WidgetFactory< ContainerWidgetType >::createTextBox( const typename TextBox::Seed & cs )
{
	return WidgetCreator< TextBox >::create( this, cs );
}

template<typename ContainerWidgetType>
typename WidgetFactory< ContainerWidgetType >::TextBoxPtr
WidgetFactory< ContainerWidgetType >::attachTextBox( unsigned id )
{
	return WidgetCreator< TextBox >::attach( this, id );
}

template<typename ContainerWidgetType>
typename WidgetFactory< ContainerWidgetType >::StatusBarPtr
WidgetFactory< ContainerWidgetType >::createStatusBar( const typename StatusBar::Seed & cs )
{
	return WidgetCreator< StatusBar >::create( this, cs );
}

template<typename ContainerWidgetType>
typename WidgetFactory< ContainerWidgetType >::StatusBarSectionsPtr
WidgetFactory< ContainerWidgetType >::createStatusBarSections( const typename StatusBarSections::Seed & cs )
{
	return WidgetCreator< StatusBarSections >::create( this, cs );
}

template<typename ContainerWidgetType>
typename WidgetFactory< ContainerWidgetType >::ButtonPtr
	WidgetFactory< ContainerWidgetType >::createButton( const Button::Seed & cs )
{
	return WidgetCreator< Button >::create( this, cs );
}

template<typename ContainerWidgetType>
typename WidgetFactory< ContainerWidgetType >::MDIParentPtr
WidgetFactory< ContainerWidgetType >::createMDIParent( const typename MDIParent::Seed & cs )
{
	return WidgetCreator< MDIParent >::create( this, cs );
}

template<typename ContainerWidgetType>
typename WidgetFactory< ContainerWidgetType >::WidgetTabViewPtr
WidgetFactory< ContainerWidgetType >::createTabView( const typename WidgetTabView::Seed & cs )
{
	return WidgetCreator< WidgetTabView >::create( this, cs );
}

template<typename ContainerWidgetType>
typename WidgetFactory< ContainerWidgetType >::TabSheetPtr
WidgetFactory< ContainerWidgetType >::createTabSheet( const typename TabSheet::Seed & cs )
{
	return WidgetCreator< TabSheet >::create( this, cs );
}

template<typename ContainerWidgetType>
typename WidgetFactory< ContainerWidgetType >::SliderPtr
WidgetFactory< ContainerWidgetType >::createSlider( const typename Slider::Seed & cs )
{
	return WidgetCreator< Slider >::create( this, cs );
}

template<typename ContainerWidgetType>
typename WidgetFactory< ContainerWidgetType >::SpinnerPtr
WidgetFactory< ContainerWidgetType >::createSpinner( const typename Spinner::Seed & cs )
{
	return WidgetCreator< Spinner >::create( this, cs );
}

template<typename ContainerWidgetType>
typename WidgetFactory< ContainerWidgetType >::ProgressBarPtr
WidgetFactory< ContainerWidgetType >::createProgressBar( const typename ProgressBar::Seed & cs )
{
	return WidgetCreator< ProgressBar >::create( this, cs );
}

template<typename ContainerWidgetType>
typename WidgetFactory< ContainerWidgetType >::ButtonPtr
WidgetFactory< ContainerWidgetType >::attachButton( unsigned id )
{
	return WidgetCreator< Button >::attach( this, id );
}

template<typename ContainerWidgetType>
typename WidgetFactory< ContainerWidgetType >::ProgressBarPtr
WidgetFactory< ContainerWidgetType >::attachProgressBar( unsigned id )
{
	return WidgetCreator< ProgressBar >::attach( this, id );
}

template<typename ContainerWidgetType>
typename WidgetFactory< ContainerWidgetType >::SliderPtr
WidgetFactory< ContainerWidgetType >::attachSlider( unsigned id )
{
	return WidgetCreator< Slider >::attach( this, id );
}

template<typename ContainerWidgetType>
typename WidgetFactory< ContainerWidgetType >::SpinnerPtr
WidgetFactory< ContainerWidgetType >::attachSpinner( unsigned id )
{
	return WidgetCreator< Spinner >::attach( this, id );
}

template<typename ContainerWidgetType>
typename WidgetFactory< ContainerWidgetType >::GroupBoxPtr
WidgetFactory< ContainerWidgetType >::createGroupBox( const typename GroupBox::Seed & cs )
{
	return WidgetCreator< GroupBox >::create( this, cs );
}

template<typename ContainerWidgetType>
typename WidgetFactory< ContainerWidgetType >::GroupBoxPtr
WidgetFactory< ContainerWidgetType >::attachGroupBox( unsigned id )
{
	return WidgetCreator< GroupBox >::attach( this, id );
}

template<typename ContainerWidgetType>
typename WidgetFactory< ContainerWidgetType >::DateTimePtr
WidgetFactory< ContainerWidgetType >::createDateTimePicker( const typename DateTime::Seed & cs )
{
	return WidgetCreator< DateTime >::create( this, cs );
}

template<typename ContainerWidgetType>
typename WidgetFactory< ContainerWidgetType >::DateTimePtr
WidgetFactory< ContainerWidgetType >::attachDateTimePicker( unsigned id )
{
	return WidgetCreator< DateTime >::attach( this, id );
}

template<typename ContainerWidgetType>
typename WidgetFactory< ContainerWidgetType >::RadioButtonPtr
WidgetFactory< ContainerWidgetType >::createRadioButton( GroupBoxPtr parent, const typename RadioButton::Seed & cs )
{
#ifdef PORT_ME	
	RadioButtonPtr retVal = WidgetCreator< RadioButton >::create( parent, internal_::getTypedParentOrThrow < EventHandlerClass * >( this ), cs );
	parent->addChild( retVal );
	return retVal;
#endif
}

template<typename ContainerWidgetType>
typename WidgetFactory< ContainerWidgetType >::RadioButtonPtr
WidgetFactory< ContainerWidgetType >::attachRadioButton( unsigned id )
{
	RadioButtonPtr retVal = WidgetCreator< RadioButton >::attach( this, id );
	return retVal;
}

template<typename ContainerWidgetType>
typename WidgetFactory< ContainerWidgetType >::ComboBoxPtr
WidgetFactory< ContainerWidgetType >::createComboBox( const typename ComboBox::Seed & cs )
{
	return WidgetCreator< ComboBox >::create( this, cs );
}

template<typename ContainerWidgetType>
typename WidgetFactory< ContainerWidgetType >::ComboBoxPtr
WidgetFactory< ContainerWidgetType >::attachComboBox( unsigned id )
{
	return WidgetCreator< ComboBox >::attach( this, id );
}

template<typename ContainerWidgetType>
typename WidgetFactory< ContainerWidgetType >::LabelPtr
WidgetFactory< ContainerWidgetType >::createStatic( const typename Label::Seed & cs )
{
	return WidgetCreator< Label >::create( this, cs );
}

template<typename ContainerWidgetType>
typename WidgetFactory< ContainerWidgetType >::LabelPtr
WidgetFactory< ContainerWidgetType >::attachStatic( unsigned id )
{
	return WidgetCreator< Label >::attach( this, id );
}

template<typename ContainerWidgetType>
typename WidgetFactory< ContainerWidgetType >::ToolTipPtr
WidgetFactory< ContainerWidgetType >::createToolTip( const typename ToolTip::Seed & cs )
{
	return WidgetCreator< ToolTip >::create( this, cs );
}

// end namespace SmartWin
}

#endif

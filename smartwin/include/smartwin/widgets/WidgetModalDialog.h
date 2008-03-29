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
#ifndef WidgetModalDialog_h
#define WidgetModalDialog_h

#include "../aspects/AspectDialog.h"
#include "WidgetWindowBase.h"

namespace SmartWin
{
// begin namespace SmartWin

/// Modal Dialog class
/** \ingroup WidgetControls
  * \image html dialog.PNG
  * Class for creating a Modal Dialog based optionally on an embedded resource. <br>
  * Use createDialog( unsigned resourceId ) if you define the dialog in a .rc file, 
  * and use createDialog() if you define the dialog completly in C++ source. <br>
  * Use the createDialog function to actually create a dialog. <br>
  * Class is a public superclass of WidgetWindowBase and therefore can use all 
  * features of WidgetWindowBase. <br>
  * Note! <br>
  * Usually you create a WidgetModalDialog on the stack. <br>
  * This Widget does NOT have selfdestructive semantics and should normally be 
  * constructed on the stack! <br>
  * The createDialog function does NOT return before the Widget is destroyed! <br>
  * Thus, you must declare the "onInitDialog" event handler before calling the 
  * "createDialog()", either in the contructor, or in some intialization routine 
  * called before createDialog();   
  */

class WidgetModalDialog : 
	public WidgetWindowBase< Policies::ModalDialog >,
	public AspectDialog<WidgetModalDialog >
{
	struct Dispatcher
	{
		typedef std::tr1::function<bool ()> F;
		
		Dispatcher(const F& f_) : f(f_) { }

		bool operator()(const MSG& msg, LRESULT& ret) {
			ret = f() ? TRUE : FALSE;
			return true;
		}

		F f;
	};

public:
	typedef WidgetWindowBase< Policies::ModalDialog > BaseType;
	
	/// Class type
	typedef WidgetModalDialog ThisType;

	/// Object type
	/** Note, not a pointer!!!!
	  */
	typedef ThisType ObjectType;

	/// Creates a Modal Dialog Window from a resource id.
	/** This version creates a window from a Dialog Resource ID. <br>
	  * To be called by the invoker of the dialog. <br>
	  * The return comes from the parameter to endDialog() <br>
	  * You must call onInitDialog( &MyDialogWidget::initDialog ); or similar either 
	  * in the constructor of your dialog or right before calling this function. <br>
	  * And in your initDialog, you must call attachXxxx for all of the controls 
	  * you wish to use, and set the event handlers for all controls and events you 
	  * wish to handle. <br>
	  * Example : <br>
	  * LabelPtr prompt = attachStatic( IDC_PROMPT ); <br>
	  * prompt->onClicked( &X::myClickMethod ); <br>
	  * ...etc... 
	  */
	virtual int createDialog( unsigned resourceId );

	/// Creates a Modal Dialog Window defined in C++ alone.
	/** This version creates a dialog window without using a Dialog Resource ID. <br>
	  * To be called by the invoker of the dialog. <br>
	  * The return comes from the parameter to endDialog() <br>
	  * You must call onInitDialog( &MyModalDialogWidget::initDialog ); in the 
	  * constructor of your dialog, <br>
	  * and in your initDialog you create the dialog's Widgets yourself. <br>
	  * Example : <br>
	  * LabelPtr prompt = createStatic(); <br>
	  * prompt->setBounds( 10, 100, 100, 50 ); <br>
	  * prompt->setText( _T("testing") ); 
	  */
	int createDialog();

	/// Ends the Modal Dialog Window started with createDialog().
	/** Pass a return value for createDialog() and close the dialog. <br>
	  * To be called by the dialog class when it should close. <br>
	  * Note that the member variables of the WidgetModalDialog class still exist, 
	  * but not any subwindows or Control Widgets.       
	  */
	void endDialog( int returnValue );

	/// Dialog Init Event Handler setter
	/** This would normally be the event handler where you attach your Widget
	  * controls and do all the initializing etc... <br>
	  * It's important that you declare this event handler BEFORE calling the 
	  * createDialog function since that function doesn't actually return before the 
	  * dialog is destroyed! <br>
	  * Method signature must be bool foo(); <br>
	  * If you return true from your Event Handler the system will NOT mess up the 
	  * initial focus you have chosen, if you return false the system will decide 
	  * which control to initially have focus according to the tab order of the 
	  * controls!       
	  */
	void onInitDialog(const Dispatcher::F& f) {
		addCallback(
			Message( WM_INITDIALOG ), Dispatcher(f)
		);
	}

protected:
	// Protected since this Widget we HAVE to inherit from
	explicit WidgetModalDialog( Widget * parent = 0 );

	virtual ~WidgetModalDialog()
	{}

	/// Specify how a resourceless dialog's window appears.
	/** The derived pure dialog class can control the DLGTEMPLATE parameters used in
	  * createDialog() with this protected call. <br>
	  * The calling layer is prevented from doing so. <br>
	  * See DLGTEMPLATE as used in ::DialogBoxIndirectParam for details. 
	  */
	void setDlgTemplate( DLGTEMPLATE inTemplate );

	/// Called by default when WM_CLOSE is posted to the dialog
	bool defaultClosing() {
		endDialog(IDCANCEL);
		return true;
	}
	
private:
	DLGTEMPLATE itsDefaultDlgTemplate; // For pure modal dialogs without resource files
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline WidgetModalDialog::WidgetModalDialog( Widget * parent )
	: BaseType( parent )
{
	// Default parameters for pure modal dialogs
#ifdef WINCE
		itsDefaultDlgTemplate.style = DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU | DS_CENTER;
#else
		itsDefaultDlgTemplate.style = DS_MODALFRAME | DS_FIXEDSYS | WS_POPUP | WS_CAPTION | WS_SYSMENU | DS_CENTER;
#endif
	itsDefaultDlgTemplate.dwExtendedStyle = 0;
	itsDefaultDlgTemplate.cdit = 0; // No dialog items in the dialog
	itsDefaultDlgTemplate.cx = 280; // 4 horizontal units are the width of one character
	itsDefaultDlgTemplate.cy = 160; // 8 vertical units are the height of one character
	itsDefaultDlgTemplate.x = 0;
	itsDefaultDlgTemplate.y = 0;
	
	onClosing(std::tr1::bind(&ThisType::defaultClosing, this));
}

// The derived pure dialog class can control the DLGTEMPLATE parameters used in
// createDialog() with this protected call. The calling layer is prevented from
// doing so.
//

inline void WidgetModalDialog::setDlgTemplate( DLGTEMPLATE inTemplate )
{
	itsDefaultDlgTemplate = inTemplate;
}

// A Pure dialog created at runtime, without any help from a resource editer. The
// derived dialog class can control the DLGTEMPLATE parameters. instead of the
// calling layer.
//

inline void WidgetModalDialog::endDialog( int retv )
{
	// Causes createDialog() to return with retv.
	//
	::EndDialog( this->handle(), static_cast< INT_PTR >( retv ) );
}

// end namespace SmartWin
}

#endif

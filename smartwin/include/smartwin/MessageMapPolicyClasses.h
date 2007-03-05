// $Revision: 1.14 $
/*
  Copyright (c) 2005, Thomas Hansen
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification,
  are permitted provided that the following conditions are met:

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
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND 
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef MessageMapPolicyClasses_h
#define MessageMapPolicyClasses_h

#include "DestructionClass.h"

namespace SmartWin
{
// begin namespace SmartWin

/// Aspect classes for a MessageMapPolicyDialogWidget
/** Used as the third template argument to WidgetFactory if you're creating a
  * MessageMapPolicyDialogWidget
  */
class MessageMapPolicyDialogWidget
	: protected private_::DestructionClass
{
protected:
	LRESULT kill()
	{
		killChildren();
		killMe();
		return 0;
	}

	// The WidgetDialog CAN subclass existing controls (from the dialog resource)
	// and therefore we add this enum to tell smartwin that all the "subclassXXX"
	// functions are "callable". If you get a compiling error here you are probably
	// trying to call "subclassXXX" on a Normal or MDI Widget type which doesn't
	// work! Think of it as a safety hatch to make it more difficult for you to
	// shoot yourself in the leg!
	enum canSubclassControls
	{ Yup_we_can_do
	};

	// TODO: Protected??
public:
	LRESULT returnFromCloseMsg( HWND hWnd, UINT msg, WPARAM wPar, LPARAM lPar )
	{
		// The dialog internals in windows api ensures we don't HAVE to call
		// DestroyWindow like we have to in other Widget types! But since users of
		// SmartWin may EXPLICITLY call close on Widget we STILL need to call
		// DestroyWindow anyway!
		::DestroyWindow( itsHandle );
		return TRUE;
	}

	LRESULT returnFromHandledWindowProc( HWND hWnd, UINT msg, WPARAM wPar, LPARAM lPar )
	{
		// A dialog Widget should return TRUE to the windows internal dialog
		// message handler procedure to tell windows that it have handled the
		// message
		return TRUE;
	}

	LRESULT returnFromUnhandledWindowProc( HWND hWnd, UINT msg, WPARAM wPar, LPARAM lPar )
	{
		// As opposed to a "normal" Widget the dialog Widget should NOT return
		// ::DefaultMessageProc or something similar since this is done internally
		// INSIDE windows but rather return FALSE to indicate it DID NOT handle the
		// message, exception is WM_INITDIALOG which must return TRUE if unhandled
		// to set focus to default "focusable" control!
		if ( WM_INITDIALOG == msg )
			return TRUE;
		return FALSE;
	}

	// STATIC EXTRACTER/Dialog Message Procedure, extracts the this pointer and
	// dispatches the message to the this Widget, only for subclassed dialog
	// templete Widgets
	static INT_PTR CALLBACK mainWndProc_( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
	{
		Widget * This = reinterpret_cast< Widget * >( ::GetProp( hWnd, _T( "_mainWndProc" ) ) );
		if ( !This )
		{
			if ( uMsg == WM_INITDIALOG )
			{
				// extracting the this pointer and stuffing it into the Window with SetProp
				This = reinterpret_cast< Widget * >( lParam );
				::SetProp( hWnd, _T( "_mainWndProc" ), reinterpret_cast< HANDLE >( This ) );
				private_::setHandle( This, hWnd );
			}
			else
				return FALSE;
		}
		return This->sendWidgetMessage( hWnd, uMsg, wParam, lParam );
	}
};

/// Aspect classes for a MessageMapPolicyModalDialogWidget
/** Used as the third template argument to WidgetFactory if you're creating a
  * MessageMapPolicyModalDialogWidget
  */
class MessageMapPolicyModalDialogWidget
	: protected private_::DestructionClass
{
protected:

/*	Debugging problems with menus and WidgetModalDialogs.

	void snapUserData( char header[], HWND	hw )
	{
		std::stringstream msg;
		msg << header << std::hex << ::GetWindowLong( hw, GWL_USERDATA ) << std::endl;
		_RPT0( _CRT_WARN, msg.str().c_str() );

		if ( ::IsMenu( hw ) ) {
			_RPT0( _CRT_WARN, _T("Aha, it is a menu") );
		}
	}

	void snapChildren()
	{
		snapUserData( "kill called on ! isChild ", this->handle() );

		int sz= (int)itsChildren.size();
		for ( int c=0; c < sz; c++ ) {
			snapUserData( "  children are ", itsChildren[c]->handle() );
		}
	}
*/

	LRESULT kill()
	{
		// Handle either the Modal dialog widget or something created inside it.
		// We can differentiate because "modal dialog boxes cannot have the WS_CHILD style".
		if ( this->isChild ) { 
			// snapUserData( "kill called on isChild ", this->handle() );
			// For a widget in the ModalDialog,
			killMe();	// do as MessageMapPolicyNormalWidget does.
		} else {
			// snapChildren();

			// For the ModalDialog widget itself, just erase me from two lists.
			// ( Since the modal dialog widget is stack based, 
			//   isn't "collected automatically", and thus should not be deleted. )
			killChildren();	// needed for resource based WidgetModalDialogs.
			eraseMeFromParentsChildren();
			eraseFromApplicationWidgets( this );
		}

		return 0;
	}

	// Remove from the Application based list of widgets.
	void eraseFromApplicationWidgets( Widget * inWidget )
	{
		// Explicitly erase it here
		for ( std::list < Widget * >::iterator idx = Application::instance().itsWidgets.begin();
			idx != Application::instance().itsWidgets.end();
			++idx )
		{
			if ( * idx == inWidget )
			{
				Application::instance().itsWidgets.erase( idx );
				break;
			}
		}
	}



	enum canSubclassControls
	{ Yup_we_can_do
	};

	// TODO: Protected??
public:
	LRESULT returnFromCloseMsg( HWND hWnd, UINT msg, WPARAM wPar, LPARAM lPar )
	{
		// make sure we tidy up, re-enable the parent window etc.
		::EndDialog( itsHandle, IDOK );
		return TRUE;
	}

	LRESULT returnFromHandledWindowProc( HWND hWnd, UINT msg, WPARAM wPar, LPARAM lPar )
	{
		// yes, it's been handled.
		return TRUE;
	}

	LRESULT returnFromUnhandledWindowProc( HWND hWnd, UINT msg, WPARAM wPar, LPARAM lPar )
	{
		// see MessageMapPolicyDialogWidget
		if ( WM_INITDIALOG == msg )
			return TRUE;
		return FALSE;
	}

	// STATIC EXTRACTER/Dialog Message Procedure, extracts the this pointer and
	// dispatches the message to the this Widget, only for subclassed dialog
	// templete Widgets
	static INT_PTR CALLBACK mainWndProc_( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
	{
		Widget * This = reinterpret_cast< Widget * >( ::GetProp( hWnd, _T( "_mainWndProc" ) ) );
		if ( !This )
		{
			if ( uMsg == WM_INITDIALOG )
			{
				// extracting the this pointer and stuffing it into the Window with SetProp
				This = reinterpret_cast< Widget * >( lParam );
				::SetProp( hWnd, _T( "_mainWndProc" ), reinterpret_cast< HANDLE >( This ) );
				private_::setHandle( This, hWnd );
			}
			else
				return FALSE;
		}
		return This->sendWidgetMessage( hWnd, uMsg, wParam, lParam );
	}
};

/// Aspect classes for a normal Container Widget
/** Used as the third template argument to WidgetFactory if you're creating a normal
  * Container Widget Note that this one is default so if you don't supply  policy
  * SmartWin will assume this is the one you're after!
  */
class MessageMapPolicyNormalWidget
	: protected private_::DestructionClass
{
protected:
	LRESULT kill()
	{
		killMe();
		return 0;
	}

	// TODO: Protected??
public:
	LRESULT returnFromCloseMsg( HWND hWnd, UINT msg, WPARAM wPar, LPARAM lPar )
	{
		return ::DefWindowProc( hWnd, msg, wPar, lPar );
	}

	LRESULT returnFromHandledWindowProc( HWND hWnd, UINT msg, WPARAM wPar, LPARAM lPar )
	{
		return 0;
	}

	LRESULT returnFromUnhandledWindowProc( HWND hWnd, UINT msg, WPARAM wPar, LPARAM lPar )
	{
		return ::DefWindowProc( hWnd, msg, wPar, lPar );
	}

	// STATIC EXTRACTER/Windows Message Procedure, extracts the this pointer and
	// dispatches the message to the this Widget
	static LRESULT CALLBACK mainWndProc_( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
	{
		Widget * This = reinterpret_cast< Widget * >( ::GetProp( hWnd, _T( "_mainWndProc" ) ) );
		if ( !This )
		{
			if ( uMsg == WM_NCCREATE )
			{
				// extracting the this pointer and stuffing it into the Window with SetProp
				CREATESTRUCT * cs = reinterpret_cast< CREATESTRUCT * >( lParam );
				This = reinterpret_cast< Widget * >( cs->lpCreateParams );
				::SetProp( hWnd, _T( "_mainWndProc" ), reinterpret_cast< HANDLE >( This ) );
				private_::setHandle( This, hWnd );
			}
			// TODO: Should be revised to let WM_CREATE messages be handled
			else if ( uMsg != WM_CREATE )
				return ::DefWindowProc( hWnd, uMsg, wParam, lParam );
		}
		return This->sendWidgetMessage( hWnd, uMsg, wParam, lParam );
	}
};

#ifndef WINCE // MDI Widgets doesn't exist on CE
/// Aspect classes for a MDI Child Container Widget
/** Used as the third template argument to WidgetFactory if you're creating an MDI
  * Child Container Widget
  */
class MessageMapPolicyMDIChildWidget
	: protected private_::DestructionClass
{
protected:
	LRESULT kill()
	{
		killMe();
		return 0;
	}

	// TODO: Protected??
public:
	LRESULT returnFromCloseMsg( HWND hWnd, UINT msg, WPARAM wPar, LPARAM lPar )
	{
		return ::DefMDIChildProc( hWnd, msg, wPar, lPar );
	}

	LRESULT returnFromHandledWindowProc( HWND hWnd, UINT msg, WPARAM wPar, LPARAM lPar )
	{
		switch ( msg )
		{
			case WM_SIZE :
			{
				return ::DefMDIChildProc( hWnd, msg, wPar, lPar );
			}
			default:
				return 0;
		}
	}

	LRESULT returnFromUnhandledWindowProc( HWND hWnd, UINT msg, WPARAM wPar, LPARAM lPar )
	{
		return ::DefMDIChildProc( hWnd, msg, wPar, lPar );
	}

	// STATIC EXTRACTER/Windows Message Procedure, extracts the this pointer and
	// dispatches the message to the this Widget
	static LRESULT CALLBACK mainWndProc_( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
	{
		Widget * This = reinterpret_cast< Widget * >( ::GetProp( hWnd, _T( "_mainWndProc" ) ) );
		if ( !This )
		{
			if ( uMsg == WM_NCCREATE )
			{
				// extracting the this pointer and stuffing it into the Window with SetProp
				CREATESTRUCT * cs = reinterpret_cast< CREATESTRUCT * >( lParam );
				This = reinterpret_cast< Widget * >( ( reinterpret_cast< MDICREATESTRUCT * >
						( cs->lpCreateParams )->lParam ) );

				::SetProp( hWnd, _T( "_mainWndProc" ), reinterpret_cast< HANDLE >( This ) );
				private_::setHandle( This, hWnd );
			}
			// TODO: Should be revised to let WM_CREATE messages be handled
			else if ( uMsg != WM_CREATE )// DefMDIChildProc instead, need own MDI Child class...
				return ::DefMDIChildProc( hWnd, uMsg, wParam, lParam );
		}
		return This->sendWidgetMessage( hWnd, uMsg, wParam, lParam );
	}
};
#endif //! WINCE

// end namespace SmartWin
}

#endif

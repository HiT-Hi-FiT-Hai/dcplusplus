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

#include "MessageMapBase.h"
#include "Application.h"
#include <boost/cast.hpp>

namespace SmartWin
{
// begin namespace SmartWin

template<typename Policy>
class MessageMapPolicy : public Policy {
public:
	MessageMapPolicy() : Widget(0) { }

	static LRESULT CALLBACK wndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		// Check if this is an init type message - a message that will set the window pointer correctly
		Policy::initPolicy(hwnd, uMsg, wParam, lParam);

		// We dispatch certain messages back to the child widget, so that their
		// potential callbacks will die along with the child when the time has come
		HWND handler = getHandler(hwnd, uMsg, wParam, lParam);
		
		Message msgObj( hwnd, uMsg, wParam, lParam, true );
		
		// Try to get the this pointer
		/// @todo Maybe cast this to the actual widget type?
		MessageMapBase* map = reinterpret_cast<MessageMapBase*>(::GetProp( handler, MessageMapBase::propAtom ));
		
		if(!map) {
			if(handler != hwnd) {
				map = reinterpret_cast<Policy*>(::GetProp( hwnd, MessageMapBase::propAtom ));
				if(map) {
					return static_cast<Policy*>(map)->returnUnhandled(hwnd, uMsg, wParam, lParam);
				}
			} 
			return Policy::returnUnknown(hwnd, uMsg, wParam, lParam);
		}
#ifdef WINCE
		if(uMsg == WM_DESTROY) {
#else
		if(uMsg == WM_NCDESTROY) {
#endif
			
			map->kill();
			return Policy::returnDestroyed(hwnd, uMsg, wParam, lParam);
		}

		if((uMsg == WM_COMMAND || uMsg == WM_SYSCOMMAND) && lParam == 0) {
			if(Application::instance().handleCommand(msgObj, handler)) {
				return Policy::returnHandled(0, hwnd, uMsg, wParam, lParam);
			}
		}

		HRESULT hres = 0;
		if(map->tryFire(msgObj, hres)) {
			return Policy::returnHandled(hres, hwnd, uMsg, wParam, lParam);
		}
		
		if(handler != hwnd) {
			map = reinterpret_cast<Policy*>(::GetProp( hwnd, MessageMapBase::propAtom ));
			if(!map) {
				return Policy::returnUnknown(hwnd, uMsg, wParam, lParam);
			}
		} 
		
		Policy* This = reinterpret_cast<Policy*>(map);
		
		return This->returnUnhandled(hwnd, uMsg, wParam, lParam);
	}
private:

	static HWND getHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		HWND handler;
		// Check who should handle the message - parent or child
		switch(uMsg) {
		case WM_CTLCOLORSTATIC :
		case WM_CTLCOLORBTN :
		case WM_CTLCOLOREDIT :
		case WM_CTLCOLORLISTBOX :
		case WM_CTLCOLORSCROLLBAR : {
			handler = reinterpret_cast<HWND>(lParam);
			
		} break;
		case WM_DRAWITEM : {
			/// @todo Not sure who should handle these....
			handler = hwnd;
		} break;
		case WM_NOTIFY : {
			NMHDR* nmhdr = reinterpret_cast<NMHDR*>(lParam);
			handler = nmhdr->hwndFrom;
		} break;
		/// @todo Not sure who should handle these....
		case WM_HSCROLL :
		case WM_VSCROLL :
		case WM_MEASUREITEM :
		case WM_INITMENUPOPUP : {
			handler = hwnd;
		} break;
		case WM_COMMAND: {
			if(lParam != 0) {
				handler = reinterpret_cast<HWND>(lParam);
			} else {
				handler = hwnd;
			}
		} break;
		default: {
			// By default, widgets handle their own messages
			handler = hwnd;
		}
		}
		return handler;
	}
};

namespace Policies {
/// Aspect classes for a MessageMapPolicyDialogWidget
/** Used as the third template argument to WidgetFactory if you're creating a
  * MessageMapPolicyDialogWidget
  */
class Dialog
	: public MessageMapBase
{
public:
	// Note; SmartWin::Widget won't actually be initialized here because of the virtual inheritance
	Dialog() : Widget(0) { }
	
	virtual void kill()
	{
		killChildren();
		killMe();
	}

	static LRESULT returnDestroyed(HWND hWnd, UINT msg, WPARAM wPar, LPARAM lPar) {
		return FALSE;
	}
	
	static LRESULT returnHandled(LRESULT, HWND, UINT, WPARAM, LPARAM) 
	{
		// A dialog Widget should return TRUE to the windows internal dialog
		// message handler procedure to tell windows that it have handled the
		// message
		/// @todo We should SetWindowLong with the actual result in some cases, see DialogProc docs
		return TRUE;
		
	}

	static LRESULT returnUnknown(HWND hWnd, UINT msg, WPARAM wPar, LPARAM lPar) {
		return returnUnhandled(hWnd, msg, wPar, lPar);
	}
	
	static LRESULT returnUnhandled( HWND hWnd, UINT msg, WPARAM wPar, LPARAM lPar )
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
	
	static void initPolicy(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		if ( uMsg == WM_INITDIALOG )
		{
			// extracting the this pointer and stuffing it into the Window with SetProp
			MessageMapBase* This = boost::polymorphic_cast<MessageMapBase*>(reinterpret_cast< Widget * >( lParam ));
			::SetProp( hwnd, MessageMapBase::propAtom, reinterpret_cast< HANDLE >( This ) );
			private_::setHandle( This, hwnd );
		}
	}
};

/// Aspect classes for a MessageMapPolicyModalDialogWidget
/** Used as the third template argument to WidgetFactory if you're creating a
  * MessageMapPolicyModalDialogWidget
  */
class ModalDialog
	: public Dialog
{
public:
	// Note; SmartWin::Widget won't actually be initialized here because of the virtual inheritance
	ModalDialog() : Widget(0) { }

	virtual void kill()
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

};

/// Aspect classes for a normal Container Widget
/** Used as the third template argument to WidgetFactory if you're creating a normal
  * Container Widget Note that this one is default so if you don't supply  policy
  * SmartWin will assume this is the one you're after!
  */
class Normal
	: public MessageMapBase
{
public:
	virtual void kill()
	{
		killMe();
	}

	// Note; SmartWin::Widget won't actually be initialized here because of the virtual inheritance
	Normal() : Widget(0) { }
	
	static LRESULT returnDestroyed(HWND hWnd, UINT msg, WPARAM wPar, LPARAM lPar) {
		return ::DefWindowProc( hWnd, msg, wPar, lPar );
	}
	
	static LRESULT returnHandled( LRESULT hres, HWND hWnd, UINT msg, WPARAM wPar, LPARAM lPar )
	{
		return hres;
	}

	static LRESULT returnUnknown( HWND hWnd, UINT msg, WPARAM wPar, LPARAM lPar ) {
		return ::DefWindowProc( hWnd, msg, wPar, lPar );
	}

	LRESULT returnUnhandled( HWND hWnd, UINT msg, WPARAM wPar, LPARAM lPar )
	{
		return ::DefWindowProc( hWnd, msg, wPar, lPar );
	}

	// STATIC EXTRACTER/Windows Message Procedure, extracts the this pointer and
	// dispatches the message to the this Widget
	static void initPolicy( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
	{
		if ( uMsg == WM_NCCREATE ) {
			// extracting the this pointer and stuffing it into the Window with SetProp
			CREATESTRUCT * cs = reinterpret_cast< CREATESTRUCT * >( lParam );
			MessageMapBase* This = boost::polymorphic_cast<MessageMapBase*>(reinterpret_cast< Widget * >( cs->lpCreateParams ));
			::SetProp( hWnd, MessageMapBase::propAtom, reinterpret_cast< HANDLE >( This ) );
			private_::setHandle( This, hWnd );
		}
	}
};

class Subclassed : public Normal {
public:
	Subclassed() : Widget(0), oldProc(0) { }
	
	LRESULT returnUnhandled(HWND hWnd, UINT msg, WPARAM wPar, LPARAM lPar) {
		if(oldProc) {
			return ::CallWindowProc(oldProc, hWnd, msg, wPar, lPar);
		}
		return Normal::returnUnhandled(hWnd, msg, wPar, lPar);
	}
	
	virtual void subclass( unsigned id ) {
		Normal::subclass(id);
		createMessageMap();
	}

	virtual void create( const SmartWin::Seed & cs) {
		Normal::create(cs);
		createMessageMap();
	}

	/// Call this function from your overridden create() if you add a new Widget to
	/// make the Windows Message Procedure dispatching map right.
	void createMessageMap() {
		::SetProp( handle(), MessageMapBase::propAtom, reinterpret_cast< HANDLE >( static_cast<MessageMapBase*>(this) ) );
		oldProc = reinterpret_cast< WNDPROC >( ::SetWindowLongPtr( handle(), GWL_WNDPROC, ( LONG_PTR ) &MessageMapPolicy<Subclassed>::wndProc ) );
	}
	
private:
	WNDPROC oldProc;	
};

#ifndef WINCE // MDI Widgets doesn't exist on CE
/// Aspect classes for a MDI Child Container Widget
/** Used as the third template argument to WidgetFactory if you're creating an MDI
  * Child Container Widget
  */
class MDIChild
	: public MessageMapBase
{
public:
	virtual void kill()
	{
		killMe();
	}
	
	// Note; SmartWin::Widget won't actually be initialized here because of the virtual inheritance
	MDIChild() : Widget(0) { }
	
	static LRESULT returnDestroyed(HWND hWnd, UINT msg, WPARAM wPar, LPARAM lPar) {
		return ::DefMDIChildProc( hWnd, msg, wPar, lPar );
	}
	
	static LRESULT returnHandled(HRESULT hres, HWND hWnd, UINT msg, WPARAM wPar, LPARAM lPar )
	{
		switch ( msg )
		{
			case WM_SIZE :
			{
				return ::DefMDIChildProc( hWnd, msg, wPar, lPar );
			}
			default:
				return hres;
		}
	}

	static LRESULT returnUnknown( HWND hWnd, UINT msg, WPARAM wPar, LPARAM lPar ) {
		return returnUnhandled(hWnd, msg, wPar, lPar);
	}

	static LRESULT returnUnhandled( HWND hWnd, UINT msg, WPARAM wPar, LPARAM lPar )
	{
		return ::DefMDIChildProc( hWnd, msg, wPar, lPar );
	}

	// STATIC EXTRACTER/Windows Message Procedure, extracts the this pointer and
	// dispatches the message to the this Widget
	static void initPolicy( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
	{
		if ( uMsg == WM_NCCREATE )
		{
			// extracting the this pointer and stuffing it into the Window with SetProp
			CREATESTRUCT * cs = reinterpret_cast< CREATESTRUCT * >( lParam );
			MDICREATESTRUCT * mcs = reinterpret_cast< MDICREATESTRUCT*>(cs->lpCreateParams);
			
			MessageMapBase* This = dynamic_cast<MessageMapBase*>(reinterpret_cast< Widget * >( mcs->lParam ));

			::SetProp( hWnd, MessageMapBase::propAtom, reinterpret_cast< HANDLE >( This ) );
			private_::setHandle( This, hWnd );
		}
	}
};

template<typename WidgetType>
class MDIFrame : public Normal {
public:
	MDIFrame() : Widget(0) { }
	
	LRESULT returnUnhandled( HWND hWnd, UINT msg, WPARAM wPar, LPARAM lPar )
	{
		WidgetType* This = static_cast<WidgetType*>(this);
		if(This->getMDIClient()) {
			return ::DefFrameProc( hWnd, This->getMDIClient()->handle(), msg, wPar, lPar );
		}
		return Normal::returnUnhandled(hWnd, msg, wPar, lPar);
	}
};
#endif //! WINCE

}

// end namespace SmartWin
}

#endif

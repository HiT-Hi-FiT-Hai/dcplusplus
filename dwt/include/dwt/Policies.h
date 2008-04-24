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

#ifndef DWT_Policies_h
#define DWT_Policies_h

#include "Widget.h"

namespace dwt {

namespace Policies {
class Subclassed;
}

template<typename Policy>
class MessageMap : public Policy {
protected:
	typedef MessageMap<Policy> PolicyType;
	
	MessageMap(Widget* parent) : Policy(parent) { }

	static LRESULT CALLBACK wndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		// Check if this is an init type message - a message that will set the window pointer correctly
		Policy::initPolicy(hwnd, uMsg, wParam, lParam);

		// We dispatch certain messages back to the child widget, so that their
		// potential callbacks will die along with the child when the time comes
		HWND handler = getHandler(hwnd, uMsg, wParam, lParam);
		
		MSG msg = { hwnd, uMsg, wParam, lParam };
		
		// Try to get the this pointer
		Widget* w = hwnd_cast<Widget*>(handler);
		
		if(w) {
#ifdef WINCE
			if(uMsg == WM_DESTROY) {
#else
			if(uMsg == WM_NCDESTROY) {
#endif
				w->kill();
				return Policy::returnDestroyed(hwnd, uMsg, wParam, lParam);
			}
		
			LRESULT res = 0;
			if(w->tryFire(msg, res)) {
				return Policy::returnHandled(res, hwnd, uMsg, wParam, lParam);
			}
		}
		PolicyType* p;
		
		if(handler != hwnd) {
			p = hwnd_cast<PolicyType*>(hwnd);
		} else {
			p = dynamic_cast<PolicyType*>(w);
		}
		
		if(!p) {
			return Policy::returnUnknown(hwnd, uMsg, wParam, lParam);
		}
		
		return p->returnUnhandled(hwnd, uMsg, wParam, lParam);
	}
private:
	friend class Policies::Subclassed;
	
	static HWND getHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		HWND handler;
		// Check who should handle the message - parent or child
		switch(uMsg) {
		case WM_CTLCOLORBTN:
		case WM_CTLCOLORDLG:
		case WM_CTLCOLOREDIT:
		case WM_CTLCOLORLISTBOX:
		case WM_CTLCOLORMSGBOX:
		case WM_CTLCOLORSCROLLBAR:
		case WM_CTLCOLORSTATIC: {
			handler = reinterpret_cast<HWND>(lParam);
		} break;
		case WM_NOTIFY : {
			NMHDR* nmhdr = reinterpret_cast<NMHDR*>(lParam);
			handler = nmhdr->hwndFrom;
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
/// Policy for modeless dialogs
class ModelessDialog
	: public Widget
{
protected:
	ModelessDialog(Widget* parent) : Widget(parent) { }
	
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
			ModelessDialog* This = reinterpret_cast<ModelessDialog*>(lParam);
			This->setHandle( hwnd );
		}
	}
};

/// Policy for modal dialogs - these are treated the same as modeless dialogs but are not automagically deleted
class ModalDialog
	: public ModelessDialog
{
protected:
	ModalDialog(Widget* parent) : ModelessDialog(parent) { }
	
	virtual void kill() {
		// Modal dialogs are stack allocated, so no delete
	}
};

/// Aspect classes for a normal Container Widget
/** Used as the third template argument to WidgetFactory if you're creating a normal
  * Container Widget Note that this one is default so if you don't supply  policy
  * SmartWin will assume this is the one you're after!
  */
class Normal
	: public Widget
{
protected:
	Normal(Widget* parent) : Widget(parent) { }
	
	static LRESULT returnDestroyed(HWND hWnd, UINT msg, WPARAM wPar, LPARAM lPar) {
		return ::DefWindowProc( hWnd, msg, wPar, lPar );
	}
	
	static LRESULT returnHandled( LRESULT hres, HWND hWnd, UINT msg, WPARAM wPar, LPARAM lPar ) {
		return hres;
	}

	static LRESULT returnUnknown( HWND hWnd, UINT msg, WPARAM wPar, LPARAM lPar ) {
		return ::DefWindowProc( hWnd, msg, wPar, lPar );
	}

	LRESULT returnUnhandled( HWND hWnd, UINT msg, WPARAM wPar, LPARAM lPar ) {
		return ::DefWindowProc( hWnd, msg, wPar, lPar );
	}

	// STATIC EXTRACTER/Windows Message Procedure, extracts the this pointer and
	// dispatches the message to the this Widget
	static void initPolicy( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) {
		if ( uMsg == WM_NCCREATE ) {
			// extracting the this pointer and stuffing it into the Window with SetProp
			CREATESTRUCT * cs = reinterpret_cast< CREATESTRUCT * >( lParam );
			Normal* This = reinterpret_cast<Normal*>( cs->lpCreateParams );
			This->setHandle( hWnd );
		}
	}
};

class Subclassed : public Normal {
protected:
	Subclassed(Widget* parent) : Normal(parent), oldProc(0) { }
	
	LRESULT returnUnhandled(HWND hWnd, UINT msg, WPARAM wPar, LPARAM lPar) {
		if(oldProc) {
			return ::CallWindowProc(oldProc, hWnd, msg, wPar, lPar);
		}
		return Normal::returnUnhandled(hWnd, msg, wPar, lPar);
	}
	
	HWND create(const Normal::Seed& seed) {
		HWND hWnd = Normal::create(seed);
		setHandle(hWnd);
		return hWnd;
	}
	
	virtual void setHandle(HWND hWnd) {
		Normal::setHandle(hWnd);
		oldProc = reinterpret_cast< WNDPROC >( ::SetWindowLongPtr( hWnd, GWL_WNDPROC, ( LONG_PTR ) &MessageMap<Subclassed>::wndProc ) );
	}
	using Widget::attach;
private:
	WNDPROC oldProc;	
};

#ifndef WINCE // MDI Widgets doesn't exist on CE
/// Aspect classes for a MDI Child Container Widget
class MDIChild
	: public Widget
{
protected:
	MDIChild(Widget* parent) : Widget(parent) { }
	
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
			
			MDIChild* This = reinterpret_cast<MDIChild*>(mcs->lParam);
			This->setHandle(hWnd);
		}
	}
};

template<typename WidgetType>
class MDIFrame : public Normal {
protected:
	MDIFrame(Widget* parent) : Normal(parent) { }
	
	LRESULT returnUnhandled( HWND hWnd, UINT msg, WPARAM wPar, LPARAM lPar ) {
		WidgetType* This = static_cast<WidgetType*>(this);
		if(This->getMDIParent()) {
			return ::DefFrameProc( hWnd, This->getMDIParent()->handle(), msg, wPar, lPar );
		}
		return Normal::returnUnhandled(hWnd, msg, wPar, lPar);
	}
	
};
#endif //! WINCE

}

}

#endif

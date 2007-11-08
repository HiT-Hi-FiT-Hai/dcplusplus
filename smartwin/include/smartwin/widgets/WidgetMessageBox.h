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
#ifndef WidgetMessageBox_h
#define WidgetMessageBox_h

#include "../../SmartUtil.h"
#include "../Widget.h"

namespace SmartWin
{
// begin namespace SmartWin

/// MessageBox class
/** \ingroup WidgetControls
  * \image html messagebox.PNG
  * Class for showing a MessageBox. <br>
  * Either derive from it, call WidgetFactory::createMessageBox or use normal 
  * Constructor. <br>
  * Note! <br>
  * If you wish to use this class with Parent classes other than those from SmartWin 
  * you need to expose a public function called "parent" taking no arguments returning 
  * and HWND in the Parent template parameter. <br>
  * The complete signature of the function will then be "HWND parent()"   
  */
class WidgetMessageBox
{
public:
	/// Class type
	typedef WidgetMessageBox ThisType;

	/// Object type
	/** Note, not a pointer!!!!
	  */
	typedef ThisType ObjectType;

	~WidgetMessageBox()
	{}

	explicit WidgetMessageBox( SmartWin::Widget * parent = 0 );

	// Next three enums are here INTENTIONALLY to abstract away Win32API
	/// Enums for which buttons you want the MessageBox to have.
	enum Buttons
	{
		BOX_OK = MB_OK,
		BOX_OKCANCEL = MB_OKCANCEL,
		BOX_ABORTRETRYIGNORE = MB_ABORTRETRYIGNORE,
		BOX_YESNOCANCEL = MB_YESNOCANCEL,
		BOX_YESNO = MB_YESNO | MB_DEFBUTTON2,	// Default to no
#ifdef MB_CANCELTRYCONTINUE
		BOX_CANCELTRYCONTINUE = MB_CANCELTRYCONTINUE,
#endif
		BOX_RETRYCANCEL = MB_RETRYCANCEL
		};

	/// Enums for which ICON you want the MessageBox to have.
	enum Icon
	{
		BOX_ICONEXCLAMATION = MB_ICONEXCLAMATION,
		BOX_ICONHAND = MB_ICONHAND,
		BOX_ICONQUESTION = MB_ICONQUESTION,
		BOX_ICONASTERISK = MB_ICONASTERISK,
		BOX_ICONINFORMATION = MB_ICONINFORMATION,
		BOX_ICONSTOP = MB_ICONSTOP
		};

	/// Enums for Return Value that the MessageBox::show can return
	enum RetVal
	{
		RETBOX_ABORT = IDABORT,
		RETBOX_CANCEL = IDCANCEL,
#ifdef IDCONTINUE
		RETBOX_CONTINUE = IDCONTINUE,
#endif
		RETBOX_IGNORE = IDIGNORE,
		RETBOX_NO = IDNO,
		RETBOX_OK = IDOK,
		RETBOX_RETRY = IDRETRY,
#ifdef IDTRYAGAIN
		RETBOX_TRYAGAIN = IDTRYAGAIN,
#endif
		RETBOX_YES = IDYES
	};

	/// Shows the actual MessageBox
	/** First parameter is the body shown inside the Message Box. <br>
	  * Second parameter is the HEADER of the Message Box ( the text shown in the
	  * blue line, optional parameter ) <br>
	  * Third parameter is the Buttons you want the Message Box to have ( optional 
	  * parameter ) <br>
	  * Fourth parameter is the Icon you want the Message Box to display ( normally 
	  * in the upper left corner )       
	  */
	RetVal show( const SmartUtil::tstring & body,
		const SmartUtil::tstring & header = _T( "SmartWinMessageBox" ),
		Buttons buttons = BOX_OK,
		Icon icon = BOX_ICONINFORMATION );

private:
	SmartWin::Widget * itsParent;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline WidgetMessageBox::WidgetMessageBox( SmartWin::Widget * parent )
	: itsParent( parent )
{}

inline WidgetMessageBox::RetVal WidgetMessageBox::show( const SmartUtil::tstring & body, const SmartUtil::tstring & header, Buttons buttons, Icon icon )
{
	return static_cast< RetVal >( ::MessageBox( itsParent ? itsParent->handle() : 0, body.c_str(), header.c_str(), buttons | icon ) );
}

// end namespace SmartWin
}

#endif

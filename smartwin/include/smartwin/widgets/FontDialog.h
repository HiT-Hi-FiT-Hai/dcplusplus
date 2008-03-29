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
#ifndef FontDialog_h
#define FontDialog_h

#include "../Widget.h"

namespace SmartWin
{
// begin namespace SmartWin

/// ChooseFontDialog class
/** \ingroup WidgetControls
  * \image html choosefont.PNG
  * Class for showing a common ChooseFontDialog box. <br>
  * Either derive from it or call WidgetFactory::createChooseFont. <br>
  * Note! <br>
  * If you wish to use this class with Parent classes other than those from SmartWin 
  * you need to expose a public function called "parent" taking no arguments returning 
  * an HWND on the template parameter class. <br>
  * the complete signature of the function will then be "HWND parent()"   
  */
class FontDialog
{
public:
	/// Class type
	typedef FontDialog ThisType;

	/// Object type
	/** Note, not a pointer!!!!
	  */
	typedef ThisType ObjectType;

	/// Shows the dialog
	bool open(DWORD dwFlags, LOGFONT& font, DWORD& rgbColors);

	/// Constructor Taking pointer to parent
	explicit FontDialog( Widget* parent = 0 );

	~FontDialog() { }

private:
	Widget* itsParent;

	HWND getParentHandle() { return itsParent ? itsParent->handle() : NULL; }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline FontDialog::FontDialog( Widget * parent )
	: itsParent( parent )
{
}

// end namespace SmartWin
}

#endif
#endif //! WINCE

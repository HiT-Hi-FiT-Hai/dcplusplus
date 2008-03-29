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
#ifndef SaveDialog_h
#define SaveDialog_h

#include "../Widget.h"
#include "../../SmartUtil.h"
#include "../aspects/AspectFileFilter.h"

namespace SmartWin
{
// begin namespace SmartWin

/// SaveFileDialog class
/** \ingroup WidgetControls
  * \image html savefile.PNG
  * Class for showing a Save File Dialog.
  * \sa LoadDialog
  * \sa AspectFileFilter
  */
class SaveDialog
	: public AspectFileFilter
{
public:
	/// Class type
	typedef SaveDialog ThisType;

	/// Object type
	/** Note, not a pointer!!!!
	  */
	typedef ThisType ObjectType;

	/// Shows the dialog
	/** Returns string() or "empty string" if user press cancel. <br>
	  * Returns a "file path" if user presses ok.
	  */
	bool open(SmartUtil::tstring& target);

	/// Constructor Taking pointer to parent
	explicit SaveDialog( Widget * parent = 0 );

	virtual ~SaveDialog()
	{}

private:
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool SaveDialog::open(SmartUtil::tstring& target)
{
	TCHAR szFile[PATH_BUFFER_SIZE + 1]; // buffer for file name
	szFile[0] = '\0';

	OPENFILENAME ofn = { sizeof(OPENFILENAME) }; // common dialog box structure
	fillOFN( ofn, getParentHandle(), 0 );
	ofn.lpstrFile = szFile;

	if ( ::GetSaveFileName( & ofn ) )
	{
		target = ofn.lpstrFile;
		backslashToForwardSlashForUnix( target );
		return true;
	}
	return false;
}

inline SaveDialog::SaveDialog( Widget * parent )
	: AspectFileFilter( parent )
{
}

// end namespace SmartWin
}

#endif

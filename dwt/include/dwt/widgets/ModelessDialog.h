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

#ifndef DWT_ModelessDialog_h
#define DWT_ModelessDialog_h

#include "../aspects/AspectDialog.h"
#include "Frame.h"

namespace dwt {

/// Dialog class
/** \ingroup WidgetControls
  * \WidgetUsageInfo
  * \image html dialog.PNG
  * Class for creating a Modeless Dialog based upon an embedded resource. <br>
  * Use the createDialog function to actually create a dialog. <br>
  * Class is a public superclass of Frame and therefor can use all features 
  * of Frame. 
  */
class ModelessDialog :
	public Frame< Policies::ModelessDialog >,
	public AspectDialog<ModelessDialog >
{
	typedef Frame< Policies::ModelessDialog > BaseType;
public:
	/// Class type
	typedef ModelessDialog ThisType;

	/// Object type
	typedef ThisType * ObjectType;

	/// Creates a Dialog Window
	/** This version creates a window from the given Dialog Resource Id.
	  */
	void createDialog( unsigned resourceId );

protected:
	// Protected since this Widget we HAVE to inherit from
	explicit ModelessDialog( Widget * parent = 0 );

	virtual ~ModelessDialog()
	{}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline ModelessDialog::ModelessDialog( Widget * parent )
	: BaseType( parent )
{}

}

#endif

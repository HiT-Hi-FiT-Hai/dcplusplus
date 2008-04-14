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

#ifndef WindowsHeaders_h
#define WindowsHeaders_h

// This file is supposed to contain (most at least) of the platform specific
// defines and so on

namespace dwt {

// Enum containing the different platforms we're supposed to support,
// SmartWinDesktop means BOTH Wine, MingW and VC++. SmartWinCE means Pocket PC or
// Smart Phone 2003 and onwards!
 enum Platform
{ dwtDesktop = 0, dwtCE = 1
};

}

// Including special GCC Stuff
#ifdef __GNUC__
#include "GCCHeaders.h"
#endif //! __GNUC__

// Including special VC Desktop Platform stuff
#ifndef __GNUC__
#ifndef WINCE
#include "VCDesktopHeaders.h"
#endif
#endif

// Including special VC Pocket PC Platform stuff
#ifndef __GNUC__
#ifdef WINCE
#include "VCPocketPCHeaders.h"
#endif
#endif

#endif // !WindowsHeaders_h

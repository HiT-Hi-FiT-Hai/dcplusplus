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
#ifndef SmartWin_h
#define SmartWin_h

#ifdef _MSC_VER
// We don't want the stupid "pointer trunctation" to 64 bit architecture warning.
// The warnings aren't justified anyway since they are basically a bug in 7.1
// release... E.g. the SetWindowLongPtr is defined as SetWindowLong in 32 bits mode
// but will in 64 bits mode be defined as the 64 bits equivalent version, therefore
// it will give you a 64 bit compile warning when this file is compiled with
// warning level 4 (MSVC)
#pragma warning( disable : 4244 )
#pragma warning( disable : 4312 )
#pragma warning( disable : 4311 )

#endif

#include "WindowsHeaders.h"

#include "../SmartUtil.h"
#include "Anchors.h"
#include "Application.h"
#include "BasicTypes.h"
#include "ClipBoard.h"
#include "Cursor.h"
#include "LibraryLoader.h"
#include "Place.h"
#include "Resource.h"
#include "Threads.h"
#include "WidgetFactory.h"
#include "WindowClass.h"
#include "resources/Accelerator.h"
#include "resources/Bitmap.h"
#include "resources/Brush.h"
#include "resources/Font.h"
#include "resources/Icon.h"
#include "resources/ImageList.h"
#include "resources/Pen.h"
#include "widgets/WidgetModalDialog.h"

namespace sw = SmartWin;

// 2005.05.03 was there 18427 lines of code in SmartWin
// There are considerably less now I hope =) /arnetheduck

#endif

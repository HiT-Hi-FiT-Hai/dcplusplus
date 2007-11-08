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
#include "../include/smartwin/resources/Font.h"

namespace SmartWin
{
// begin namespace SmartWin

	FontPtr createFont( PredefinedFontTypes fontType )
	{
		FontPtr retVal( new Font( fontType ) );
		return retVal;
	}

	FontPtr createFont
		( const SmartUtil::tstring & faceName
		, int height
		, int width
		, int weight
		, DWORD charSet
		, bool italic
		, bool underline
		, bool strikeOut
		, int escapementOrientation
		, DWORD outputPrecision
		, DWORD clipPrecision
		, DWORD quality
		, DWORD pitchAndFamliy
		 )
	{
		FontPtr retVal(
			new Font
				( faceName, height, width, weight, charSet, italic, underline
				, strikeOut, escapementOrientation, outputPrecision, clipPrecision
				, quality, pitchAndFamliy
				)
			);
		return retVal;
	}

// end namespace SmartWin
}

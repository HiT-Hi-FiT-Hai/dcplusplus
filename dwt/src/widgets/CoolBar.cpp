/*
  DC++ Widget Toolkit

  Copyright (c) 2007-2008, Jacek Sieka

  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, 
  are permitted provided that the following conditions are met:

      * Redistributions of source code must retain the above copyright notice, 
        this list of conditions and the following disclaimer.
      * Redistributions in binary form must reproduce the above copyright notice, 
        this list of conditions and the following disclaimer in the documentation 
        and/or other materials provided with the distribution.
      * Neither the name of the DWT nor the names of its contributors 
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

#include <dwt/widgets/CoolBar.h>
#include <dwt/DWTException.h>

namespace dwt {

CoolBar::Seed::Seed() : 
	BaseType::Seed(REBARCLASSNAME, WS_CHILD | WS_VISIBLE | RBS_VARHEIGHT | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | CCS_NODIVIDER)
{
}

void CoolBar::create( const Seed & cs ) {
	BaseType::create(cs);
}

void CoolBar::addChild( Widget * child,
	unsigned width, unsigned height, const tstring & txt
	)
{
	REBARBANDINFO rbBand;
	rbBand.cbSize = sizeof( REBARBANDINFO );
	rbBand.fMask = RBBIM_SIZE | RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_STYLE;
	if ( txt != _T( "" ) )
	{
		rbBand.fMask |= RBBIM_TEXT;
		rbBand.lpText = const_cast < TCHAR * >( txt.c_str() );
	}
	rbBand.hwndChild = child->handle();
	rbBand.cxMinChild = width;
	rbBand.cyMinChild = height;
	rbBand.cx = width;
	rbBand.fStyle = 0; //RBBS_GRIPPERALWAYS;
	if ( sendMessage( RB_INSERTBAND, ( WPARAM ) - 1, ( LPARAM ) & rbBand ) == 0 )
	{
		throw DWTException( "There was a problem when trying to insert a band into your Coolbar object!");
	}
}


}

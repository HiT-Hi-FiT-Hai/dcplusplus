// $Revision: 1.8 $
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
#ifndef WINCE // Doesn't exist in Windows CE based systems

#include "../../include/smartwin/widgets/WidgetSplitter.h"
#include "../../include/smartwin/CanvasClasses.h"
#include "../../include/smartwin/WindowsHeaders.h"
#include "../../include/smartwin/Application.h"

namespace SmartWin
{
// begin namespace SmartWin
	void SplitterThinPaint::paintSplitter( unsigned width, unsigned ySize, HWND handle )
	{
		PaintCanvas canvas( handle );
		{
			// Borders
			Pen pen( canvas, RGB( 160, 160, 160 ) );
			canvas.line( 0, 0, 0, ySize );
			canvas.line( width - 1, 0, width - 1, ySize );
		}
		{
			// Inside
			Pen pen( canvas, RGB( 190, 190, 190 ) );
			for ( unsigned idx = 1;
				idx < width - 1;
				++idx )
			{
				canvas.line( idx, 0, idx, ySize );
			}
		}
	}

	void SplitterThinPaint::paintSplitterMoving( SmartWin::Canvas & canvas, unsigned cursorX, unsigned cursorY, const SmartWin::Rectangle & rect )
	{
		for ( unsigned x = cursorX - 5; x < cursorX + 5; ++x )
			canvas.line( x, rect.pos.y, x, rect.size.y );
	}

	void SplitterCoolPaint::paintSplitter( unsigned width, unsigned ySize, HWND handle )
	{
		PaintCanvas canvas( handle );
		{
			// Borders
			Pen pen( canvas, RGB( 280, 0, 0 ) );
			canvas.line( 0, 0, 0, ySize );
			canvas.line( width - 1, 0, width - 1, ySize );
		}
		{
			// Inside
			Pen pen( canvas, RGB( 255, 0, 0 ) );
			for ( unsigned idx = 1;
				idx < width - 1;
				++idx )
			{
				canvas.line( idx, 0, idx, ySize );
			}
		}
	}

	void SplitterCoolPaint::paintSplitterMoving( Canvas & canvas, unsigned cursorX, unsigned cursorY, const SmartWin::Rectangle & rect )
	{
		canvas.line( cursorX, 0, cursorX, rect.size.y - rect.pos.y );
		canvas.ellipse( cursorX - 20, cursorY - 20, cursorX + 20, cursorY + 20 );
	}

// end namespace SmartWin
}

#endif

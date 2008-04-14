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

#include <dwt/resources/Bitmap.h>
#include <dwt/Point.h>
#include <dwt/Application.h>

namespace dwt {

Bitmap::Bitmap( HBITMAP bitmap, bool own )
	: ResourceType(bitmap, own)
{}

Bitmap::Bitmap( unsigned resourceId, unsigned flags )
	: ResourceType( ( HBITMAP )::LoadImage( Application::instance().getAppHandle(), MAKEINTRESOURCE( resourceId ), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION ) )
{}

Bitmap::Bitmap( const tstring & filePath, unsigned flags )
#ifdef WINCE
	: itsBitmap( ::SHLoadImageFile( filePath.c_str() ) )
#else
	: ResourceType( ( HBITMAP )::LoadImage( Application::instance().getAppHandle(), filePath.c_str(), IMAGE_BITMAP, 0, 0, flags | LR_LOADFROMFILE ) )
#endif
{}

HBITMAP Bitmap::getBitmap() const
{
	return handle();
}

Point Bitmap::getBitmapSize() const
{
	return getBitmapSize(handle());
}

Point Bitmap::getBitmapSize( HBITMAP bitmap )
{
	// init struct for bitmap info
	BITMAP bm = { 0 };

	// get bitmap info
	::GetObject( bitmap, sizeof( BITMAP ), & bm );

	return Point( bm.bmWidth, bm.bmHeight );
}

BitmapPtr Bitmap::resize( const Point & newSize ) const
{
	HDC hdc1 = ::CreateCompatibleDC( 0 );
	HBITMAP hBitmapOld1 = ( HBITMAP )::SelectObject( hdc1, handle() );

	HDC hdc2 = ::CreateCompatibleDC( 0 );
	HBITMAP hBitmapNew = ::CreateCompatibleBitmap( hdc1, newSize.x, newSize.y );
	HBITMAP hBitmapOld2 = ( HBITMAP )::SelectObject( hdc2, hBitmapNew );

	const Point oldSize = getBitmapSize();
	::StretchBlt( hdc2, 0, 0, newSize.x, newSize.y, hdc1, 0, 0, oldSize.x, oldSize.y, SRCCOPY );

	hBitmapNew = ( HBITMAP )::SelectObject( hdc2, hBitmapOld2 );

	// Clean up
	::SelectObject( hdc1, hBitmapOld1 );
	::DeleteDC( hdc2 );
	::DeleteDC( hdc1 );
	return BitmapPtr( new Bitmap( hBitmapNew ) );
}

}

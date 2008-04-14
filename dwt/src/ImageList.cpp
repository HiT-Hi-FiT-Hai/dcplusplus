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

#include <dwt/resources/ImageList.h>
#include <dwt/Application.h>
#include <dwt/xCeption.h>

namespace dwt {

ImageList::ImageList( HIMAGELIST imageList, bool own )
	: ResourceType( imageList, own )
{}

ImageList::ImageList( int width, int height, unsigned flags )
	: ResourceType(ImageList_Create(width, height, flags, 0, 1))
{
	if( handle() == NULL )
	{
		xCeption x( _T( "Couldn't create ImageList" ) );
		throw x;
	}
}

void ImageList::add( const Bitmap & bitmap )
{
	if(ImageList_Add( handle(), bitmap.handle(), NULL ) == -1) {
		throw xCeption(_T("Add failed"));
	}
}

void ImageList::add( const Bitmap & bitmap, const Bitmap & mask )
{
	if(ImageList_Add( handle(), bitmap.handle(), mask.handle() ) == -1) {
		throw xCeption(_T("Add masked failed"));
	}
}

void ImageList::add( const Bitmap& bitmap, COLORREF mask) {
	if(ImageList_AddMasked(handle(), bitmap.handle(), mask) == -1) {
		throw xCeption(_T("Add colormasked failed"));
	}
}

void ImageList::add( const Icon & icon ) {
	if(ImageList_AddIcon( handle(), icon.handle() ) == -1) {
		throw xCeption(_T("Add icon failed"));
	}
}

void ImageList::add(const ImageList& imageList, int image) {
	Icon icon(ImageList_GetIcon(imageList.handle(), image, ILD_TRANSPARENT));
	add(icon);
}

void ImageList::add( const ImageList& imageList )
{
	int initialSize = size();
	try {
		int images = imageList.size();
		for(int i = 0; i < images; ++i) {
			add(imageList, i);
		}
	} catch(...) {
		ImageList_SetImageCount(handle(), initialSize);
		throw;
	}
}

Point ImageList::getImageSize() const
{
	int x, y;
	BOOL success = ImageList_GetIconSize( handle(), & x, & y );
	if ( !success )
	{
		xCeption x( _T( "Couldn't get the ImageList image size" ) );
		throw x;
	}
	return Point( x, y );
}

int ImageList::size() const
{
	return ImageList_GetImageCount( handle() );
}

int ImageList::getImageCount() const {
	return size();
}

void ImageList::setBkColor(COLORREF color) {
	ImageList_SetBkColor(handle(), color);
}

COLORREF ImageList::getBkColor() const {
	return ImageList_GetBkColor(handle());
}

void ImageList::resize( unsigned newSize )
{
	BOOL success = ::ImageList_SetImageCount( handle(), newSize );
	if ( !success )
	{
		xCeption x( _T( "Couldn't resize ImageList" ) );
		throw x;
	}
}

}

// $Revision: 1.7 $
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
#include "../include/smartwin/ImageList.h"
#include "../include/smartwin/Application.h"
#include "../include/smartwin/xCeption.h"

namespace SmartWin
{
// begin namespace SmartWin

ImageList::ImageList( HIMAGELIST imageList, bool own )
	: itsImageList( imageList ),
#ifdef WINCE
		itsFlags( ILC_COLOR | ILC_MASK ),
#else
		itsFlags( ILC_COLOR24 | ILC_MASK ),
#endif
		itsOwnershipFlag( own )
{}

ImageList::ImageList( int width, int height, unsigned flags )
	: itsFlags( flags ),
		itsOwnershipFlag( true )
{
	itsImageList = ::ImageList_Create( width, height, flags, 0, 0 );
	if( itsImageList == NULL )
	{
		xCeption x( _T( "Couldn't create ImageList" ) );
		throw x;
	}
}

ImageList::~ImageList()
{
	if( itsOwnershipFlag )
		::ImageList_Destroy( itsImageList );
}

HIMAGELIST ImageList::getImageList() const
{
	return itsImageList;
}

void ImageList::add( const Bitmap & bitmap )
{
	resize( getImageCount() + 1 );
	ImageList_Replace( itsImageList, getImageCount() - 1, bitmap.getBitmap(), NULL );
}

void ImageList::add( const Bitmap & bitmap, const Bitmap & mask )
{
	resize( getImageCount() + 1 );
	ImageList_Replace( itsImageList, getImageCount() - 1, bitmap.getBitmap(), mask.getBitmap() );
}

unsigned int ImageList::addMultiple( const Bitmap & bitmap )
{
	int count = bitmap.getBitmapSize().x / getImageSize().x + 1;
	return addMultiple( count, bitmap.getBitmap(), (HBITMAP)NULL );
}

unsigned int ImageList::addMultiple( const Bitmap & bitmap, const Bitmap & mask )
{
	int count = bitmap.getBitmapSize().x / getImageSize().x + 1;
	return addMultiple( count, bitmap.getBitmap(), mask.getBitmap() );
}


unsigned int ImageList::addMultiple( const Bitmap & bitmap, COLORREF mask )
{
	int count = bitmap.getBitmapSize().x / getImageSize().x + 1;
	return addMultiple( count, bitmap.getBitmap(), mask );
}

void ImageList::add( const Icon & icon )
{
	resize( getImageCount() + 1 );
	ImageList_ReplaceIcon( itsImageList, getImageCount() - 1, icon.getIcon() );
}

void ImageList::add( const ImageListPtr imageList )
{
	int count = imageList->getImageCount();
	int oldSize = getImageCount();

	resize( oldSize + count );

	for ( int i = 0; i < count; i++ )
	{
		HICON icon = ImageList_ExtractIcon( 0, imageList->getImageList(), i );
		if ( !icon )
		{
			//we need to crop the allocated images that won't be added
			resize( oldSize + i );
			xCeption x( _T( "Couldn't add bitmap to ImageList" ) );
			throw x;
		}
		int success = ImageList_ReplaceIcon( itsImageList, oldSize + i, icon );
		DestroyIcon( icon );
		if ( success == - 1 )
		{
			resize( oldSize + i );
			xCeption x( _T( "Couldn't add bitmap to ImageList" ) );
			throw x;
		}
	}
}

Point ImageList::getImageSize() const
{
	int x, y;
	BOOL success = ImageList_GetIconSize( itsImageList, & x, & y );
	if ( !success )
	{
		xCeption x( _T( "Couldn't get the ImageList image size" ) );
		throw x;
	}
	return Point( x, y );
}

int ImageList::getImageCount() const
{
	return ImageList_GetImageCount( itsImageList );
}

void ImageList::resize( unsigned newSize )
{
	BOOL success = ::ImageList_SetImageCount( itsImageList, newSize );
	if ( !success )
	{
		xCeption x( _T( "Couldn't resize ImageList" ) );
		throw x;
	}
}

unsigned int ImageList::addMultiple( int count, HBITMAP bitmap, HBITMAP mask )
{
	HIMAGELIST list = ::ImageList_Create( getImageSize().x, getImageSize().y, itsFlags, count, 0 );
	if ( !list )
	{
		xCeption x( _T( "Couldn't add bitmap to ImageList" ) );
		throw x;
	}
	ImageListPtr imageList( new ImageList( list ) );
	int success = ImageList_Add( list, bitmap, mask );
	if ( success == - 1 )
	{
		xCeption x( _T( "Couldn't add bitmap to ImageList" ) );
		throw x;
	}
	add( imageList );
	return imageList->getImageCount();
}

unsigned int ImageList::addMultiple( int count, HBITMAP bitmap, COLORREF mask )
{
	HIMAGELIST list = ::ImageList_Create( getImageSize().x, getImageSize().y, itsFlags, count, 0 );
	if ( !list )
	{
		xCeption x( _T( "Couldn't add bitmap to ImageList" ) );
		throw x;
	}
	ImageListPtr imageList( new ImageList( list ) );
	int success = ImageList_AddMasked( list, bitmap, mask );
	if ( success == - 1 )
	{
		xCeption x( _T( "Couldn't add bitmap to ImageList" ) );
		throw x;
	}
	add( imageList );
	return imageList->getImageCount();
}

// end namespace SmartWin
}

// $Revision: 1.6 $
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
#ifndef ImageList_h
#define ImageList_h

#include "WindowsHeaders.h"
#include "boost.h"
#include "SmartUtil.h"
#include "Bitmap.h"
#include "Icon.h"

namespace SmartWin
{
// begin namespace SmartWin

// Forward declaration
class ImageList;

/// \ingroup GlobalStuff
/// ImageList pointer
/** Use this typedef instead to ensure compatibility in future versions of SmartWin!!
  */
typedef boost::shared_ptr< ImageList > ImageListPtr;

/// Class encapsulating an HIMAGELIST and ensuring that the contained HIMAGELIST is
/// freed upon destruction of this object
/** Use this class if you need RAII semantics encapsulating an HIMAGELIST. <br>
  * You can use an ImageList on e.g. a Data Grid.
  */
class ImageList
{
public:
	/// RAII Constructor taking a HIMAGELIST
	/** Note! <br>
	  * The "own" parameter specifies if the class takes "control" of
	  * HIMAGELIST meaning it will automatically free the
	  * contained HIMAGELIST upon destruction
	  */
	explicit ImageList( HIMAGELIST imageList, bool own = true );

	/// RAII Constructor
	/** Creates a new ImageList <br>
	  * The width and height refer to the image size of the image list elements. <br>
	  * The flags parameter specifies the bit depth and if there is a mask for the
	  * images on the list. e.g. (ILC_COLOR32 | ILC_MASK) [32 bits with mask];
	  * (ILC_COLOR16) [16 bits, no mask].
	  */
	ImageList( int width, int height, unsigned flags );

	/// Frees the contained HIMAGELIST
	/** Frees the contained HIMAGELIST meaning it will no longer be accessible or it
	  * will be destroyed
	  */
	~ImageList();

	/// Getter for the underlying HIMAGELIST
	/** Use when you need to access the underlying HIMAGELIST
	  */
	HIMAGELIST getImageList() const;

	/// Add a bitmap to the list
	void add( const Bitmap & bitmap );

	/// Add a masked bitmap to the list
	/** Note, you need to set the flag ILC_MASK on the constructor for this to work.
	  * <br>
	  * The mask is a black and white image; where it's black, the shown image will
	  * be transparent.
	  */
	void add( const Bitmap & bitmap, const Bitmap & mask );

	/// Add multiple images to the list
	/** The bitmap parameter specifies a bitmap with several images.<br>
	  * The number of images is calculated by the bitmap width, e.g. if the
	  * image list is 32x32 and you add a 128x32 bitmap, it will add
	  * four 32x32 bitmaps (note, the height must be the same of the image list)<br>
	  * Returns the number of images added.
	  */
	unsigned int addMultiple( const Bitmap & bitmap );

	/// Add a multiple masked images to the list
	/** Note, you need to set the flag ILC_MASK on the constructor for this to work.
	  * <br>
	  * The mask is a black and white image; where it's black, the shown image will
	  * be transparent. <br>
	  * The bitmap and mask parameters specify bitmaps with several images. The
	  * number of images is calculated by the bitmap width, e.g. if the image list is
	  * 32x32 and you add a 128x32 bitmap, it will add four 32x32 bitmaps (note, the
	  * height must be the same of the image list) Returns the number of images
	  * added.
	  */
	unsigned int addMultiple( const Bitmap & bitmap, const Bitmap & mask );

	/// Add an icon to the list
	/** Note, you need to set the flag ILC_MASK on the constructor to support
	  * transparency. <br>
	  * If the icon has multiple sizes, it will choose the size specified in the
	  * constructor.
	  */
	void add( const Icon & icon );

	/// Add all the images in "imageList" to this image list
	void add( const ImageListPtr imageList );

	/// Returns the image size.
	/** Returns the size of the images in the Image List (all images in the Image
	  * List have the same size)
	  */
	Point getImageSize() const;

	/// Returns the number of images in list.
	int getImageCount() const;

private:

	// Resize underlying image list, helper method just to reuse code
	void resize( unsigned newSize );

	// Add a bitmap with multiple images, helper method
	unsigned int addMultiple( int count, HBITMAP bitmap, HBITMAP mask );

	HIMAGELIST itsImageList;
	unsigned itsFlags;

	// Specifies that the underlying image list is owned, i.e., will be destroyed on the destructor
	bool itsOwnershipFlag;
};

// end namespace SmartWin
}

#endif

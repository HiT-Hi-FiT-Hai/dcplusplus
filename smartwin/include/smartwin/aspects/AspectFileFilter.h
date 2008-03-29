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
#ifndef AspectFileFilter_h
#define AspectFileFilter_h

#include "../WindowsHeaders.h"
#include <vector>
#include "SmartUtil.h"
#include "../xCeption.h"

namespace SmartWin
{
// begin namespace SmartWin

/// Class for adding a filter to e.g. a LoadDialog dialog.
/** \ingroup AspectClasses
  * Class is an Aspect class which should be realized into classes that needs it.
  * <br>
  * Help the LoadDialog and the SaveDialog to add up filters on which file
  * types to look for!
  */
class AspectFileFilter
{
public:
	/// Adds a filter to the object.
	/** filterName is the friendly name of the filter, normally this would be e.g.
	  * "HTML Files" etc. filter is the actual filter to filter in files to show
	  * normally this would be e.g. "*.html".
	  */
	void addFilter( const SmartUtil::tstring & filterName, const SmartUtil::tstring & filter )
	{
		itsFilter.insert( itsFilter.end(), filterName.begin(), filterName.end() );
		itsFilter.push_back( '\0' );
		itsFilter.insert( itsFilter.end(), filter.begin(), filter.end() );
		itsFilter.push_back( '\0' );
	}

	/// Sets the active filter to the specified index
	/** If you have added five filters and set the active filter to 3 then the fourth
	  * filter you added will be the active filter. Active filter means the default
	  * filter used when first showing the dialog.
	  */
	void activeFilter( unsigned filterNo )
	{
		if ( filterNo >= itsFilter.size() )
		{
			xCeption x( _T( "Tried to set active filter to more than number of filters in filter..." ) );
			throw x;
		}
		itsActiveFilter = filterNo;
	}

	/// Returns the active filter of the object
	/** The active filter is the "currently selected" filter of the filter class
	  */
	unsigned getActiveFilter() const
	{
		// Filter index is NOT a zero indexed array...
		return itsActiveFilter + 1;
	}

	/// Sets the starting directory of the LoadDialog or SaveDialog Widget
	/** If given your dialog will try to start in the given directory, otherwise it
	  * will use the working directory of the process.
	  */
	void setStartDirectory( SmartUtil::tstring startDir )
	{
		itsStartDir = startDir;
	}

	/// Ensure filename meets OS expectations for path separators.
	/** We want SaveDialog and LoadDialog to always return a pathname that
	  * meets the OS expectations. <br>
	  * Windows wants: C:\dir\dir\file.ext <br>
	  * and UnixLinux: /dir/dir/file.ext <br>
	  *
	  * Wine produces z:\dir\dir\file.exe from GetLoadFileName(&ofn) )and
	  * GetSaveFileName(&ofn). <br>
	  *
	  * So if we are building for wine, we need to convert from the wine output to
	  * the Linux format. <br>
	  * IE: convert z:\home\awebb\file.txt to /home/awebb/file.txt <br>
	  *
	  * The assumption is that the C++ standard library is Linux native, and thus
	  * needs / pathnames.
	  */
	//TODO: use boost filenames
	void backslashToForwardSlashForUnix( SmartUtil::tstring & filename )
	{
		// wineg++ defines __WINE__  Note we can't use WIN32 because that is also true for wine builds.
#ifdef __WINE__
		if ( 0 != filename.find( "z:" ) ) return; // Not a WINE produced path ?, nothing to do.
		filename.erase( 0, 2 ); // Remove "z:" prefix.

		// Unix file system do not use "\", so convert to "/"
		size_t pos_n;
		while ( std::string::npos != ( pos_n = filename.find( '\\' ) ) )
		{
			filename.replace( pos_n, 1, "/" );
		}
#endif
	}


protected:
	Widget * itsParent;
	HWND getParentHandle() { return itsParent ? itsParent->handle() : NULL; }

	AspectFileFilter(Widget* parent)
		: itsParent(parent), itsActiveFilter( 0 )
	{}

	static const int PATH_BUFFER_SIZE = 32768; //really arbitrary, but 32K sounds reasonable. size in number of TCHARS!

	// Fills out the common members of the OPENFILENAME struct.
	// This is called for both LoadDialog and for SaveDialog Widgets
	void fillOFN( OPENFILENAME & ofn, HWND parent, int flags )
	{
		ofn.hwndOwner = parent;

		ofn.nMaxFile = PATH_BUFFER_SIZE;
		ofn.lpstrFilter = itsFilter.c_str();
		ofn.nFilterIndex = this->getActiveFilter();
		ofn.lpstrInitialDir = itsStartDir.c_str();
		ofn.Flags = flags;
	}


private:
	unsigned int itsActiveFilter;
	SmartUtil::tstring itsStartDir;
	SmartUtil::tstring itsFilter;
};

// end namespace SmartWin
}

#endif

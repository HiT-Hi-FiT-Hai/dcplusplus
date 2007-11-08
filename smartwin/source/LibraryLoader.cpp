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
#include "../include/smartwin/WindowsHeaders.h"
#include "../include/smartwin/LibraryLoader.h"
#include <utility>

namespace SmartWin
{
// begin namespace SmartWin

LibraryLoader::~LibraryLoader()
{
	// Need a lock here since we're accessing the shared static version of its map!
	Utilities::ThreadLock lock( LibraryLoader::itsCs );

	if ( !hasCalledLoad )
		return;

	// Decreasing reference count
	LibraryLoader::itsLibrariesLoaded[itsLibraryName].first -= 1;

	// Checking to see if last instance and if so freeing library and removing map entry
	if ( 0 == LibraryLoader::itsLibrariesLoaded[itsLibraryName].first )
	{
		::FreeLibrary( LibraryLoader::itsLibrariesLoaded[itsLibraryName].second );
		LibraryLoader::itsLibrariesLoaded.erase( itsLibraryName );
	}
}

void LibraryLoader::load( const SmartUtil::tstring & libraryName )
{
	// Need a lock here since we're accessing the shared static version of its map!
	Utilities::ThreadLock lock( LibraryLoader::itsCs );

	// VERY important we DON'T increase refcount or anything like that
	if ( hasCalledLoad )
	{
		xCeption x( _T( "Already called load on LibraryLoader object" ) );
		throw x;
	}

	itsLibraryName = libraryName;

	hasCalledLoad = true;

	std::map< SmartUtil::tstring, std::pair< int, HMODULE > >::const_iterator exists = LibraryLoader::itsLibrariesLoaded.find( libraryName );
	if ( LibraryLoader::itsLibrariesLoaded.end() == exists )
	{
		// Loading library
		itsHMod = ::LoadLibrary( libraryName.c_str() );

		// TODO: Rewrite xAssert to get support for submitting SmartUtil::tstrings (could show library name)
		xAssert( itsHMod != 0, _T( "Error while trying to load library or dll!" ) );

		// SUCCESS!
		itsLibrariesLoaded[libraryName].second = itsHMod;
		itsLibrariesLoaded[libraryName].first = 1;
	}
	else
	{
		itsLibrariesLoaded[libraryName].first += 1;
	}
}

LibraryLoader::LibraryLoader( const SmartUtil::tstring & libraryName )
	: itsLibraryName( libraryName ),
	hasCalledLoad( false )
{
	load( libraryName );
}

LibraryLoader::LibraryLoader()
	: hasCalledLoad( false )
{
}


	// Get procedure address from loaded library by name
	FARPROC LibraryLoader::getProcAddress( const SmartUtil::tstring & procedureName )
	{
		return ::GetProcAddress( itsHMod, SmartUtil::AsciiGuaranteed::doConvert( procedureName, SmartUtil::ConversionCodepage::ANSI ).c_str() );
	}

	// Get procedure address from loaded library by ordinal value
	FARPROC LibraryLoader::getProcAddress( long procedureOrdinal )
	{
		return ::GetProcAddress( itsHMod, (LPCSTR)0 + procedureOrdinal );
	}



// Static members definitions!
Utilities::CriticalSection LibraryLoader::itsCs;
std::map< SmartUtil::tstring, std::pair< int, HMODULE > > LibraryLoader::itsLibrariesLoaded;

// end namespace SmartWin
}

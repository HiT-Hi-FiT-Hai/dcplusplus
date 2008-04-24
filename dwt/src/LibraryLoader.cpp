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

#include <dwt/LibraryLoader.h>
#include <dwt/util/check.h>
#include <dwt/util/StringConversion.h>

namespace dwt {

LibraryLoader::LibraryLoader( const tstring & libraryName ) : 
	itsHMod(NULL)
{
	load(libraryName);
}

LibraryLoader::LibraryLoader() : 
	itsHMod(NULL)
{
}

LibraryLoader::~LibraryLoader() {
	if(itsHMod != NULL) {
		::FreeLibrary(itsHMod);
	}
}

void LibraryLoader::load( const tstring & libraryName )
{
	// VERY important we DON'T increase refcount or anything like that
	if ( itsHMod != NULL ) {
		dwtWin32DebugFail("Already called load on LibraryLoader object");
	}

	// Loading library
	itsHMod = ::LoadLibrary( libraryName.c_str() );

	// TODO: Rewrite xAssert to get support for submitting tstrings (could show library name)
	dwtassert( itsHMod != 0, _T( "Error while trying to load library or dll!" ) );
}

// Get procedure address from loaded library by name
FARPROC LibraryLoader::getProcAddress( const tstring & procedureName ) {
	return ::GetProcAddress( itsHMod, util::AsciiGuaranteed::doConvert( procedureName, util::ConversionCodepage::ANSI ).c_str() );
}

// Get procedure address from loaded library by ordinal value
FARPROC LibraryLoader::getProcAddress( long procedureOrdinal ) {
	return ::GetProcAddress( itsHMod, (LPCSTR)0 + procedureOrdinal );
}

DWORD LibraryLoader::getCommonControlsVersion() {
	static DWORD version = 0;
	if(version == 0) {
		try {
			LibraryLoader lib(_T("comctl32.dll"));
			DLLGETVERSIONPROC pDllGetVersion = (DLLGETVERSIONPROC)lib.getProcAddress(_T("DllGetVersion"));
			if(pDllGetVersion) {
				DLLVERSIONINFO dvi = { sizeof(dvi) };
				if(SUCCEEDED((*pDllGetVersion)(&dvi))) {
					version = PACK_COMCTL_VERSION(dvi.dwMajorVersion, dvi.dwMinorVersion);
				}
			}
		} catch(...) {
			// Ignore loading exceptions...
		}
	}
	return version;
}

}

// $Revision: 1.9 $
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
#ifndef xCeption_h
#define xCeption_h

#include "SmartUtil.h"
#include "WindowsHeaders.h"
#include <stdexcept>
#include "BasicTypes.h"
#include "SmartUtil.h"

namespace SmartWin
{
// begin namespace SmartWin

/// Exception class used in SmartWin
/** All exceptions thrown by SmartWin will be either of this class or derived from
  * this class. <br>
  * You should derive from this class yourself if you throw exceptions inside your
  * event handlers. <br>
  * Otherwise SmartWin may not be able to respond correctly to the Exception.
  */
class xCeption : public std::exception
{
public:
	/// Constructor taking a const char pointer
	/** The TCHAR * will contain the description returned from the virtual member
	  * function called what()
	  */
	xCeption( const TCHAR * err ) throw()
		: std::exception(),
		itsAsciiErrorMsg( SmartUtil::AsciiGuaranteed::doConvert( err, SmartUtil::ConversionCodepage::ANSI ) ),
		itsFormatWndMessage( 0 )
	{
		errId = ::GetLastError();
	}

	/// Overloaded Constructor basically doing the same as the const TCHAR * version
	/** See the const TCHAR * err overloaded version
	  */
	xCeption( const SmartUtil::tstring & err ) throw()
		: std::exception(),
		itsAsciiErrorMsg( SmartUtil::AsciiGuaranteed::doConvert( err, SmartUtil::ConversionCodepage::ANSI ) ),
		itsFormatWndMessage( 0 )
	{
		errId = ::GetLastError();
	}

	/// Returns a descriptive error message explaining what went wrong
	/** Overridden from the std::exception::what()<br>
	  * Related Members<br>
	  * <ul>
	  * <li>getErrorCode()</li>
	  * <li>whatWndMsg()</li>
	  * </ul>
	  * This function will ALWAYS return a char * string since it must be compliant
	  * with std::exception::what() function
	  */
	virtual const char * what() const throw()
	{
		return itsAsciiErrorMsg.c_str();
	}

	/// Returns a DWORD defining what went wrong, basically just a GetErrorCode
	/// call from Windows API called in the Constructor
	/** Returns the Windows Error code which is a DWORD, can be used in combination
	  * with FormatMessage from the <br>
	  * Windows API to get a descriptive Windows Error code explaining what went
	  * wrong (if it was a windows error) <br>
	  * You should in most cases rather use whatWndMsg to get a formatted message
	  * from Windows API. <br>
	  * Related Members <br>
	  * - what()
	  * - whatWndMsg()
	  */
	virtual DWORD getErrorCode() const
	{
		return errId;
	}

	/// Returns a descriptive windows error message explaining what went wrong.
	/** Returns the formatted Windows Error message<br>
	  * Related Members<br>
	  * - what()
	  * - getErrorCode()
	  */
	virtual const TCHAR * whatWndMsg() const
	{
		::FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			errId,
			MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
			( LPTSTR ) & itsFormatWndMessage,
			0,
			NULL );
		return itsFormatWndMessage;
	}

	virtual ~xCeption() throw()
	{
		::LocalFree( itsFormatWndMessage );
	};

private:
	std::string itsAsciiErrorMsg;
	DWORD errId;
	mutable TCHAR * itsFormatWndMessage;
};

/// Contains a wrapper around assert/xception which will toss out an assertion if
/// we're in debug mode but anyway throw an exception
#define xAssert( x, y ) do{ if( !(x) ) { assert(false&&(y)); throw xCeption((y)); } } while(false);

// end namespace SmartWin
}

#endif

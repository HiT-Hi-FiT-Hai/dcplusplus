// $Revision: 1.10 $
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

#include <iosfwd>
#include <sstream>
#include <algorithm>
#include <string>

#ifndef tstring_smartwin_H
#define tstring_smartwin_H

namespace SmartUtil
{
// begin namespace SmartUtil

#if defined UNICODE || defined _UNICODE
#   if defined _STRING_ || defined _CPP_STRING || defined _GLIBCXX_STRING || defined _STLP_STRING
		typedef std::wstring tstring;
#   endif //_STRING_
#   if defined _SSTREAM_ || defined _CPP_SSTREAM || defined _GLIBCXX_SSTREAM || defined _STLP_SSTREAM
		typedef std::wstringstream tstringstream;
		typedef std::wostringstream tostringstream;
		typedef std::wistringstream tistringstream;
#   endif //_SSTREAM_
#   if defined _OSTREAM_ || defined _GLIBCXX_OSTREAM || defined _STLP_OSTREAM
		typedef std::wostream tostream;
#   endif //_OSTREAM_
#   if defined _ISTREAM_ || defined _GLIBCXX_ISTREAM || defined _STLP_ISTREAM
		typedef std::wistream tistream;
#   endif //_ISTREAM_
#   if defined _OSTREAM_ || defined _CPP_OSTREAM || defined _GLIBCXX_OSTREAM || defined _STLP_OSTREAM
#   if defined _ISTREAM_ || defined _CPP_ISTREAM || defined _GLIBCXX_ISTREAM || defined _STLP_ISTREAM
		typedef std::wiostream tiostream;
#   endif //_ISTREAM_
#   endif //_OSTREAM_
#   if defined _IOSFWD_ || defined _FSTREAM_ || defined _CPP_FSTREAM || defined _GLIBCXX_FSTREAM || defined _STLP_FSTREAM
		typedef std::wfilebuf tfilebuf;
		typedef std::wfstream tfstream;
		typedef std::wifstream tifstream;
		typedef std::wofstream tofstream;
#   endif //_FSTREAM_
#else  // UNICODE
#   if defined _STRING_ || defined _CPP_STRING || defined _GLIBCXX_STRING || defined _STLP_STRING
		typedef std::string tstring;
#   endif //_STRING_
#   if defined _SSTREAM_ || defined _CPP_SSTREAM || defined _GLIBCXX_SSTREAM || defined _STLP_SSTREAM
		typedef std::stringstream tstringstream;
		typedef std::ostringstream tostringstream;
		typedef std::istringstream tistringstream;
#   endif //_SSTREAM_
#   if defined _OSTREAM_ || defined _GLIBCXX_OSTREAM || defined _STLP_OSTREAM
		typedef std::ostream tostream;
#   endif //_OSTREAM_
#   if defined _ISTREAM_ || defined _GLIBCXX_ISTREAM || defined _STLP_ISTREAM
		typedef std::istream tistream;
#   endif //_ISTREAM_
#   if defined _OSTREAM_ || defined _CPP_OSTREAM || defined _GLIBCXX_OSTREAM || defined _STLP_OSTREAM
#   if defined _ISTREAM_ || defined _CPP_ISTREAM || defined _GLIBCXX_ISTREAM || defined _STLP_ISTREAM
		typedef std::iostream tiostream;
#   endif //_ISTREAM_
#   endif //_OSTREAM_
#   if defined _IOSFWD_ || defined _FSTREAM_ || defined _CPP_FSTREAM || defined _GLIBCXX_FSTREAM || defined _STLP_FSTREAM
		typedef std::filebuf tfilebuf;
		typedef std::fstream tfstream;
		typedef std::ifstream tifstream;
		typedef std::ofstream tofstream;
#   endif //_FSTREAM_
#endif //UNICODE

	tstring string_replace(const tstring & source, const tstring & whatToReplace, const tstring & whatToReplaceWith);

// end namespace SmartUtil
}

#endif

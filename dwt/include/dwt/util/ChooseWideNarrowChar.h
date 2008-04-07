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

namespace SmartUtil
{
// begin namespace SmartUtil

#if defined UNICODE || defined _UNICODE
        typedef wchar_t tchar; 
#   ifndef _INC_TCHAR
#		define _T(x) L##x
#   endif
#   if defined _STRING_ || defined _GLIBCXX_STRING
        typedef std::wstring tstring;
#   endif //_STRING_
#   if defined _IOSTREAM_ || defined _GLIBCXX_IOSTREAM
        typedef std::wcout tcout;
#   endif //_IOSTREAM_
#   if defined _SSTREAM_ || defined _GLIBCXX_SSTREAM
        typedef std::wstringstream tstringstream;
#   endif //_SSTREAM_
#   if defined _OSTREAM_ || defined _GLIBCXX_OSTREAM
        typedef std::wostream tostream;
#   endif //_OSTREAM_
#   if defined _ISTREAM_ || defined _GLIBCXX_ISTREAM
        typedef std::wistream tistream;
#   endif //_ISTREAM_
#   if defined _FSTREAM_ || defined _GLIBCXX_FSTREAM
        typedef std::wfilebuf tfilebuf;
        typedef std::wfstream tfstream;
        typedef std::wifstream tifstream;
        typedef std::wofstream tofstream;
#   endif //_FSTREAM_
#else  // UNICODE
        typedef char tchar;
#   ifndef _INC_TCHAR
#		define _T(x) x
#   endif
#   if defined _STRING_ || defined _GLIBCXX_STRING
        typedef std::string tstring;
#   endif //_STRING_
#   if defined _IOSTREAM_ || defined _GLIBCXX_IOSTREAM
        typedef std::cout tcout;
#   endif //_IOSTREAM_
#   if defined _SSTREAM_ || defined _GLIBCXX_SSTREAM
        typedef std::stringstream tstringstream;
#   endif //_SSTREAM_
#   if defined _OSTREAM_ || defined _GLIBCXX_OSTREAM
        typedef std::ostream tostream;
#   endif //_OSTREAM_
#   if defined _ISTREAM_ || defined _GLIBCXX_ISTREAM
        typedef std::istream tistream;
#   endif //_ISTREAM_
#   if defined _FSTREAM_ || defined _GLIBCXX_FSTREAM
        typedef std::filebuf tfilebuf;
        typedef std::fstream tfstream;
        typedef std::ifstream tifstream;
        typedef std::ofstream tofstream;
#   endif //_FSTREAM_
#endif //UNICODE

// end namespace SmartUtil
}

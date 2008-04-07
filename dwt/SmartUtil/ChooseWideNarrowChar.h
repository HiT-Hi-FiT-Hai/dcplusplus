// $Revision: 1.1 $

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

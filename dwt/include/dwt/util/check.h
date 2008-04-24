#ifndef CHECK_H_
#define CHECK_H_

namespace dwt { namespace util {

#ifdef _DEBUG

#define dwtDebugFail(m) assert( (false && (m)) )

#else

#define dwtDebugFail(m) 

#endif

#define dwtassert(x, m) assert( (x) && (m) )

#define dwtWin32DebugFail(m) dwtDebugFail(m)

} }
#endif /*CHECK_H_*/

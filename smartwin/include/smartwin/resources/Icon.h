// $Revision: 1.3 $
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
#ifndef Icon_h
#define Icon_h

#include "../WindowsHeaders.h"
#include "../../SmartUtil.h"
#include "Handle.h"

namespace SmartWin
{
// begin namespace SmartWin

// Forward declaration
class Icon;

/// \ingroup GlobalStuff
/// Icon pointer
/** Use this typedef instead to ensure compatibility in future versions of SmartWin!!
  */
typedef boost::intrusive_ptr< Icon > IconPtr;

struct IconPolicy : public NullPolicy<HICON> {
	void release(HICON h) { ::DestroyIcon(h); }
};

/// Class encapsulating an HICON and ensuring that the contained HICON is freed
/// upon destruction of this object
/** Use this class if you need RAII semantics encapsulating an HICON
  */
class Icon : public Handle<IconPolicy>
{
public:
	/// RAII Constructor taking a HICON
	/** Note! <br>
	  * Class takes "control" of HICON meaning it will automatically free the
	  * contained HICON upon destruction
	  */
	explicit Icon( HICON icon, bool own = true );

	/// RAII Constructor loading a icon from a resource ID
	/** Note! <br>
	  * Class takes "control" of HICON meaning it will automatically free the
	  * contained HICON upon destruction
	  */
	explicit Icon( unsigned resourceId );

	/// RAII Constructor loading a icon from a file on disc
	/** Note! <br>
	  * Class takes "control" of HICON meaning it will automatically free the
	  * contained HICON upon destruction
	  */
	explicit Icon( const SmartUtil::tstring & filePath );

	/// Deprecated, use handle()
	HICON getIcon() const;
private:
	friend class Handle<IconPolicy>;
	typedef Handle<IconPolicy> ResourceType;

};

// end namespace SmartWin
}

#endif

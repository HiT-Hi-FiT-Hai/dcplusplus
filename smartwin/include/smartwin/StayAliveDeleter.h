// $Revision: 1.6 $
/*
  Copyright ( c ) 2005, Thomas Hansen
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification,
  are permitted provided that the following conditions are met :

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
  ( INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION ) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  OR TORT ( INCLUDING NEGLIGENCE OR OTHERWISE ) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef StayAliveDeleter_h
#define StayAliveDeleter_h

namespace SmartWin
{
// begin namespace SmartWin

// Basically this is just a helper class to make sure we can actually create a
// boost::shared_ptr which does NOT delete its pointer since we need to pass a
// boost::shared_ptr< WidgetXxx > around to the event handlers for consistency with
// the rest of the widgets we need to make sure we can create a boost::shared_ptr
// which actualy does NOT take control of the pointer it's pointing to!!!!!!!!!!
//
// Sounds weird maybe but since shared_ptr does not have a "release" function this
// is the only way to ensure we can do just that without modifying the shaed_ptr
// implementation which we OBVIOUSLY DON'T want to do!! If you need a
// boost::shared_ptr which does NOT delete the contained pointer use this as the
// SECOND parameter to the CTOR of the shared_ptr!
//
// The template argument for the class is the contained WidgetType!
template< class P >
class StayAliveDeleter
{
public:
	void operator ()( P * p )
	{
	// Here we are SUPPOSED to delete the given pointer since this function will be
	// called when the given shared_ptr is about to delete its contained pointer
	// but since this is a helper class for shared_ptr which is NOT supposed to
	// take ownership of their contained pointer we do NOTHING!! This is kind of
	// like faking a "release()" function on boost::shared_ptr!!!
	}
};

// end namespace SmartWin
}

#endif

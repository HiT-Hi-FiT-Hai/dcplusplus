// $Revision: 1.8 $
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
#ifndef MessageMapBase_h
#define MessageMapBase_h

#include "SignalParams.h"

#include <functional>
#include <map>

namespace SmartWin
{
// begin namespace SmartWin

// Internally used class
// TODO: Move into "private_" namespace ...?!?
class MessageMapBase
{
public:
	typedef std::tr1::function<HRESULT(private_::SignalContent&)> CallbackType;
	
	// We only support one Callback per message, so a map is appropriate
	typedef std::map<Message, CallbackType> CallbackCollectionType;
	
	/// Adds a new Callback into the Callback collection or replaces the existing one
	void setCallback(const Message& msg, const CallbackType& callback );

	virtual ~MessageMapBase()
	{}

protected:
	CallbackCollectionType & getCallbacks() { 
		return itsCallbacks;
	}

	// Returns true if fired, else false
	virtual bool tryFire( const Message & msg, HRESULT & retVal );
private:
	// Contains the list of signals we're (this window) processing
	CallbackCollectionType itsCallbacks;
};

// end namespace SmartWin
}

#endif

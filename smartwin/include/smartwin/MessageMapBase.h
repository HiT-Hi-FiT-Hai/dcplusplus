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

#include "SigSlots.h"

namespace SmartWin
{
// begin namespace SmartWin

// Internally used class
// TODO: Move into "private_" namespace ...?!?
template< class SignalTupleType, class SignalCollectionType >
class MessageMapBase
{
public:
	// Adds a new Signal into the Signal collection!
	void addNewSignal( const SignalTupleType & Signal )
	{
		xAssert( ( Signal.template get< 0 >().FunctionThis == 0 || Signal.template get< 0 >().Function == 0 )
			&& ( Signal.template get< 0 >().FunctionThis != 0 || Signal.template get< 0 >().Function != 0 ),
			_T( "Only ONE of the functors should be defined." ) );

		// First we must check if there already exists a Signal of the given message
		// Since SmartWin doesn't support more than ONE event handler for every event (per control basis) we must DELETE the former
		// event handler if there is one!!
		for ( typename SignalCollectionType::iterator idx = itsSignals.begin();
			idx != itsSignals.end();
			++idx )
		{
			if ( idx->template get< 0 >().Msg == Signal.template get< 0 >().Msg )
			{
				* idx = Signal;
				return;
			}
		}

		// Not found, adding new one
		itsSignals.push_back( Signal );
	}

	virtual ~MessageMapBase()
	{}

protected:
	SignalCollectionType & getSignals()
	{ return itsSignals;
	}

	// Returns true if fired, else false
	virtual bool tryFire( const Message & msg, HRESULT & retVal )
	{
		// First we must create a "comparable" message...
		Message msgComparer( msg.Handle, msg.Msg, msg.WParam, msg.LParam, false );
		for ( typename SignalCollectionType::iterator idx = itsSignals.begin();
			idx != itsSignals.end();
			++idx )
		{
			if ( idx->template get< 0 >().Msg == msgComparer )
			{
				private_::SignalContent params( msg, idx->template get< 0 >().Function, idx->template get< 0 >().FunctionThis, idx->template get< 0 >().This, true );
				retVal = idx->template get< 1 >().fire( params );
				if ( params.RunDefaultHandling )
					return false;
				return true;
			}
		}
		return false;
	}

private:
	// Contains the list of signals we're (this window) processing
	SignalCollectionType itsSignals;
};

// end namespace SmartWin
}

#endif

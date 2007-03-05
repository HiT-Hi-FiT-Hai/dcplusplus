// $Revision: 1.6 $
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
#ifndef WindowObject_h
#define WindowObject_h

namespace SmartWin
{
// begin namespace SmartWin

//TODO: why is this forward declaration needed?
// Forward decleration of friends...
// TODO: Remove
//class Font;
//class WindowObject;

/// Class for objects which windows can obtain
/** Class is a base class for Window Objects which windows can contain, e.g. the Font
  * class is such a class. <br>
  * Class is readied for intrusive reference count (SmartPtr style) so that it may be
  * used with SmartPtr logic with intrusive reference count
  */
class WindowObject
{
public:
	WindowObject() : itsReference( 0 )
	{}

	virtual ~WindowObject()
	{}

private:
	// Intrusive reference count implementation...
	// We HAVE to use intrusive reference counting here since we're converting from
	// derived e.g. SmartPtr<Font,/*...*/> to SmartPtr<WindowObject,/*...*/>
	// therefore we must be able to share the reference count variable from one
	// template class decleration to another...
	void addRef()
	{
		++itsReference;
	}

	unsigned release()
	{
		return --itsReference;
	}

	unsigned itsReference;

	// Never implemented...
	// Denying Copy Constructor and assignment operator
	WindowObject & operator =( const WindowObject & );
	WindowObject( const WindowObject & );
};

// end namespace SmartWin
}

#endif

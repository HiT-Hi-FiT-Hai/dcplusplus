/*
  DC++ Widget Toolkit

  Copyright (c) 2007-2008, Jacek Sieka

  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, 
  are permitted provided that the following conditions are met:

      * Redistributions of source code must retain the above copyright notice, 
        this list of conditions and the following disclaimer.
      * Redistributions in binary form must reproduce the above copyright notice, 
        this list of conditions and the following disclaimer in the documentation 
        and/or other materials provided with the distribution.
      * Neither the name of the DWT nor the names of its contributors 
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

#ifndef ATOM_H_
#define ATOM_H_

#include "WindowsHeaders.h"

namespace dwt {
class AtomBase {
public:
	AtomBase(ATOM atom_) : atom(atom_) { }

	operator LPCTSTR() { return reinterpret_cast<LPCTSTR>(static_cast<size_t>(atom)); }
	operator ATOM() { return atom; }
	
protected:
	ATOM atom;
};

class Atom : public AtomBase {
public:
	Atom(const TCHAR* str) : AtomBase(::AddAtom(str)) {	
	}
	
	~Atom() {
		::DeleteAtom(atom);
	}
};

class GlobalAtom : public AtomBase {
public:
	GlobalAtom(const TCHAR* str) : AtomBase(::GlobalAddAtom(str)) {
		
	}
	~GlobalAtom() {
		::GlobalDeleteAtom(atom);
	}

};
}


#endif /*ATOM_H_*/

#ifndef ATOM_H_
#define ATOM_H_

#include "WindowsHeaders.h"

namespace SmartWin {
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

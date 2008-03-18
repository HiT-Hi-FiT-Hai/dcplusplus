#ifndef DISPATCHERS_H_
#define DISPATCHERS_H_

#include "WindowsHeaders.h"

#include <functional>

namespace SmartWin
{
// begin namespace SmartWin

namespace Dispatchers {

template<LRESULT value = 0, bool handled = true>
struct VoidVoid
{
	typedef std::tr1::function<void ()> F;
	
	VoidVoid(const F& f_) : f(f_) { }

	bool operator()(const MSG& msg, LRESULT& ret) {
		f();
		ret = value;
		return handled;
	}

	F f;
};

// end namespace SmartWin
}

}

#endif /*DISPATCHERS_H_*/

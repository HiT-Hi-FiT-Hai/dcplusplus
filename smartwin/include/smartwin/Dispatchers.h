#ifndef DISPATCHERS_H_
#define DISPATCHERS_H_

#include "WindowsHeaders.h"

#include <functional>

namespace SmartWin {
// begin namespace SmartWin

namespace Dispatchers {

template<typename T>
struct Base {
	typedef std::tr1::function<T> F;

	Base(const F& f_) : f(f_) { }
	F f;
};

template<typename T>
T convert(const MSG& msg) {
	return T(msg);
}

template<typename P, P (*C)(const MSG&) = convert<P>, bool handled = true >
struct ConvertBase : public Base<void(const P&)> {
	typedef Base<void(const P&)> BaseType;
	ConvertBase(const typename BaseType::F& f_) : BaseType(f_) { }
	
	bool operator()(const MSG& msg, LRESULT& ret) {
		f((*C)(msg));
		return handled;
	}
};

template<LRESULT value = 0, bool handled = true>
struct VoidVoid : public Base<void ()> {

	VoidVoid(const F& f_) : Base<void ()>(f_) { }

	bool operator()(const MSG& msg, LRESULT& ret) {
		f();
		ret = value;
		return handled;
	}
};

// end namespace SmartWin
}

}

#endif /*DISPATCHERS_H_*/

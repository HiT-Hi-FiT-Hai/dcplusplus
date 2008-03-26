#ifndef ASPECTHELP_H_
#define ASPECTHELP_H_

#include <functional>

namespace SmartWin {

template<typename WidgetType>
class AspectHelp {
	struct Dispatcher {
		typedef std::tr1::function<void (HWND, unsigned)> F;

		Dispatcher(const F& f_) : f(f_) { }

		bool operator()(const MSG& msg, LRESULT& ret) {
			LPHELPINFO lphi = reinterpret_cast<LPHELPINFO>(msg.lParam);
			if(lphi->iContextType != HELPINFO_WINDOW)
				return false;
			f(reinterpret_cast<HWND>(lphi->hItemHandle), lphi->dwContextId);
			ret = TRUE;
			return true;
		}

		F f;
	};

public:
	unsigned getHelpId() {
		return ::GetWindowContextHelpId(static_cast<WidgetType*>(this)->handle());
	}

	void setHelpId(unsigned id) {
		::SetWindowContextHelpId(static_cast<WidgetType*>(this)->handle(), id);
	}

	void onHelp(const typename Dispatcher::F& f) {
		static_cast<WidgetType*>(this)->addCallback(
			Message( WM_HELP ), Dispatcher(f)
		);
	}
};

}

#endif /*ASPECTHELP_H_*/

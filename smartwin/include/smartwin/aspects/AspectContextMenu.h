#ifndef ASPECTCONTEXTMENU_H_
#define ASPECTCONTEXTMENU_H_

#include "../BasicTypes.h"

namespace SmartWin {

template<typename WidgetType>
class AspectContextMenu {
	struct Dispatcher {
		typedef std::tr1::function<bool (const ScreenCoordinate &)> F;

		Dispatcher(const F& f_) : f(f_) { }

		bool operator()(const MSG& msg, LRESULT& ret) {
			ret = f(ScreenCoordinate(Point(GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam))));
			return ret;
		}

		F f;
	};
	
public:
	void onContextMenu(const typename Dispatcher::F& f) {
		static_cast<WidgetType*>(this)->setCallback(
			Message( WM_CONTEXTMENU ), Dispatcher(f)
		);
	}
};

}

#endif /*ASPECTCONTEXTMENU_H_*/

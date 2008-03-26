#ifndef ASPECTCONTEXTMENU_H_
#define ASPECTCONTEXTMENU_H_

#include "../Point.h"
#include "../Dispatchers.h"

namespace SmartWin {

template<typename WidgetType>
class AspectContextMenu {
	WidgetType& W() { return *static_cast<WidgetType*>(this); }

	struct Dispatcher : Dispatchers::Base<bool (const ScreenCoordinate&)> {
		typedef Dispatchers::Base<bool(const ScreenCoordinate&)> BaseType;
		Dispatcher(const F& f_) : BaseType(f_) { }

		bool operator()(const MSG& msg, LRESULT& ret) {
			bool shown = f(ScreenCoordinate(Point::fromLParam(msg.lParam)));
			ret = shown;
			return shown;
		}
	};
	
public:
	void onContextMenu(const typename Dispatcher::F& f) {
		W().addCallback(Message( WM_CONTEXTMENU ), Dispatcher(f));
	}
};

}

#endif /*ASPECTCONTEXTMENU_H_*/

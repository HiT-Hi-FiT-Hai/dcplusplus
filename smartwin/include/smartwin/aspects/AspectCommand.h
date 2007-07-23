#ifndef ASPECTCOMMAND_
#define ASPECTCOMMAND_

#include "AspectVoidVoidDispatcher.h"

namespace SmartWin {

template<typename WidgetType>
class AspectCommand {
	typedef AspectVoidVoidDispatcher Dispatcher;
public:
	void onCommand(const Dispatcher::F& f, unsigned id) {
		static_cast<WidgetType*>(this)->setCallback(
			Message(WM_COMMAND, id), Dispatcher(f)
		);
	}

};

}

#endif /*ASPECTCOMMAND_*/

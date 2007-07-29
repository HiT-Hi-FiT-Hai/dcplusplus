#ifndef ASPECTCOMMAND_
#define ASPECTCOMMAND_

#include "../Dispatchers.h"

namespace SmartWin {

template<typename WidgetType>
class AspectCommand {
	typedef Dispatchers::VoidVoid<> Dispatcher;
public:
	void onCommand(const Dispatcher::F& f, unsigned id) {
		static_cast<WidgetType*>(this)->setCallback(
			Message(WM_COMMAND, id), Dispatcher(f)
		);
	}
	
	void onSysCommand(const Dispatcher::F& f, unsigned id) {
		static_cast<WidgetType*>(this)->setCallback(
			Message(WM_SYSCOMMAND, id), Dispatcher(f)
		);
	}

};

}

#endif /*ASPECTCOMMAND_*/

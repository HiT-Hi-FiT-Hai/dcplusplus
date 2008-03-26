#ifndef ASPECTCOMMAND_
#define ASPECTCOMMAND_

#include "../Dispatchers.h"

namespace SmartWin {

template<typename WidgetType>
class AspectCommand {
	WidgetType& W() { return *static_cast<WidgetType*>(this); }
	typedef Dispatchers::VoidVoid<> Dispatcher;
public:
	void onCommand(const Dispatcher::F& f, unsigned id) {
		W().addCallback(Message(WM_COMMAND, id), Dispatcher(f));
	}

	void onCommand(const Dispatcher::F& f, unsigned controlId, unsigned code) {
		W().addCallback(Message(WM_COMMAND, MAKEWPARAM(controlId, code)), Dispatcher(f));
	}

	void onSysCommand(const Dispatcher::F& f, unsigned id) {
		W().addCallback(Message(WM_SYSCOMMAND, id), Dispatcher(f));
	}
};

}

#endif /*ASPECTCOMMAND_*/

#ifndef ASPECTHELP_H_
#define ASPECTHELP_H_

namespace SmartWin {

template<typename WidgetType>
class AspectHelp {
	typedef Dispatchers::VoidVoid<TRUE> Dispatcher;

public:
	void onHelp(const typename Dispatcher::F& f) {
		static_cast<WidgetType*>(this)->setCallback(
			Message( WM_HELP ), Dispatcher(f)
		);
	}
};

}

#endif /*ASPECTHELP_H_*/

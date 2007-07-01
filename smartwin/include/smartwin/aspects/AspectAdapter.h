#ifndef AspectAdapter_h_
#define AspectAdapter_h_

#include <functional>

namespace SmartWin {

/**
 * Aspect adapter to support old-style smartwin on* methods.
 */
template<typename Ret, typename EventHandlerClass, bool isControl>
struct AspectAdapter {
};

template<typename Ret, typename EventHandlerClass>
struct AspectAdapter<Ret, EventHandlerClass, true> {

	template<typename WidgetTypePtr, typename FuncType>
	static Ret adapt0(WidgetTypePtr This, FuncType eventHandler) {
		EventHandlerClass * ThisParent = internal_::getTypedParentOrThrow < EventHandlerClass * >( This );
		return std::tr1::bind(eventHandler, ThisParent, This);
	}

	template<typename WidgetTypePtr, typename FuncType>
	static Ret adapt1(WidgetTypePtr This, FuncType eventHandler) {
		EventHandlerClass * ThisParent = internal_::getTypedParentOrThrow < EventHandlerClass * >( This );
		return std::tr1::bind(eventHandler, ThisParent, This, _1);
	}

	template<typename WidgetTypePtr, typename FuncType>
	static Ret adapt2(WidgetTypePtr This, FuncType eventHandler) {
		EventHandlerClass * ThisParent = internal_::getTypedParentOrThrow < EventHandlerClass * >( This );
		return std::tr1::bind(eventHandler, ThisParent, This, _1, _2);
	}
	
};

template<typename Ret, typename EventHandlerClass>
struct AspectAdapter<Ret, EventHandlerClass, false> {
	
	template<typename WidgetTypePtr, typename FuncType>
	static Ret adapt0(WidgetTypePtr This, FuncType eventHandler) {
		EventHandlerClass * ThisParent = internal_::getTypedParentOrThrow < EventHandlerClass * >( This );
		return std::tr1::bind(eventHandler, ThisParent);
	}
	
	template<typename WidgetTypePtr, typename FuncType>
	static Ret adapt1(WidgetTypePtr This, FuncType eventHandler) {
		EventHandlerClass * ThisParent = internal_::getTypedParentOrThrow < EventHandlerClass * >( This );
		return std::tr1::bind(eventHandler, ThisParent, _1);
	}
	
	template<typename WidgetTypePtr, typename FuncType>
	static Ret adapt2(WidgetTypePtr This, FuncType eventHandler) {
		EventHandlerClass * ThisParent = internal_::getTypedParentOrThrow < EventHandlerClass * >( This );
		return std::tr1::bind(eventHandler, ThisParent, _1, _2);
	}
	
};

}
#endif /*ASPECTADAPTER_H_*/

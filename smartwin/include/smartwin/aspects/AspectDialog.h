#ifndef ASPECTDIALOG_H_
#define ASPECTDIALOG_H_

#include "../../SmartUtil.h"

#include <type_traits>

namespace SmartWin {

template<typename WidgetType>
class AspectDialog {
	WidgetType& W() { return *static_cast<WidgetType*>(this); }
	HWND H() { return W().handle(); }

public:
	HWND getItem(int id) {
		return ::GetDlgItem(H(), id);
	}

	void setItemText(int id, const SmartUtil::tstring& text) {
		::SetDlgItemText(H(), id, text.c_str());
	}
	
	template<typename T>
	void attachChild(T& childPtr, int id) {
		childPtr = attachChild<typename std::tr1::remove_pointer<T>::type >(id);
	}
	
	template<typename T>
	typename T::ObjectType attachChild(int id) {
		return WidgetCreator<T>::attach(&W(), id);
	}

};

}

#endif /*ASPECTDIALOG_H_*/

#ifndef ASPECTDIALOG_H_
#define ASPECTDIALOG_H_

#include "../../SmartUtil.h"

namespace SmartWin {

template<typename T>
class AspectDialog {
public:
	void setItemText(int id, const SmartUtil::tstring& text) {
		::SetDlgItemText(static_cast<T*>(this)->handle(), id, text.c_str());
	}
	
};
}
#endif /*ASPECTDIALOG_H_*/

#ifndef WINDOWCLASS_H_
#define WINDOWCLASS_H_

#include "Widget.h"

namespace SmartWin {

class WindowClass : boost::noncopyable {
public:
	WindowClass(const SmartUtil::tstring& className, WNDPROC wndProc, LPCTSTR menu = NULL, HBRUSH background = NULL, HICON icon = NULL, HICON iconsm = NULL, HCURSOR cursor = NULL);
	~WindowClass();
	
	LPCTSTR getClassName() { return reinterpret_cast<LPCTSTR>(static_cast<size_t>(atom)); }

	/// Generate a new unique window class name
	static SmartUtil::tstring getNewClassName(Widget* widget);

private:
	static int itsInstanceNo;
	
	ATOM atom;

};

}
#endif /*WINDOWCLASS_H_*/

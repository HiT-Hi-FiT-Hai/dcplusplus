#ifndef WINDOWCLASS_H_
#define WINDOWCLASS_H_

#include "WindowsHeaders.h"
#include "resources/Icon.h"
#include <boost/noncopyable.hpp>

namespace SmartWin {

class Widget;

class WindowClass : boost::noncopyable {
public:
	WindowClass(const SmartUtil::tstring& className, WNDPROC wndProc, LPCTSTR menu = NULL, HBRUSH background = NULL, IconPtr icon = IconPtr(), IconPtr iconsm = IconPtr(), HCURSOR cursor = NULL);
	~WindowClass();
	
	LPCTSTR getClassName() { return reinterpret_cast<LPCTSTR>(static_cast<size_t>(atom)); }

	/// Generate a new unique window class name
	static SmartUtil::tstring getNewClassName(Widget* widget);

private:
	static int itsInstanceNo;
	
	ATOM atom;
	
	// Keep references
	IconPtr icon;
	IconPtr smallIcon;
};

}
#endif /*WINDOWCLASS_H_*/

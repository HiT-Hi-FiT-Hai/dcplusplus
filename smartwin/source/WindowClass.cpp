#include "../include/smartwin/WindowClass.h"
#include "../include/smartwin/Application.h"
#include "../include/smartwin/xCeption.h"

#include <typeinfo>

namespace SmartWin {

int WindowClass::itsInstanceNo;

WindowClass::WindowClass(const SmartUtil::tstring& className, WNDPROC wndProc, LPCTSTR menu, HBRUSH background, HICON icon, HICON iconsm, HCURSOR cursor) : atom(0) {
	WNDCLASSEX wc = { sizeof(WNDCLASSEX) };
	wc.lpfnWndProc = wndProc;
	wc.lpszMenuName = menu;
	wc.hbrBackground = background;
	wc.hIcon = icon;
	wc.hIconSm = iconsm;
	wc.hCursor = cursor;
	wc.hInstance = Application::instance().getAppHandle();
	wc.lpszClassName = className.c_str();
	
	atom = ::RegisterClassEx(&wc);
	if ( 0 == atom )
	{
		xCeption x( _T( "Could not register class " ) + className );
		throw x;
	}
}

WindowClass::~WindowClass() {
	if(atom != 0) {
		::UnregisterClass(getClassName(), Application::instance().getAppHandle());
	}
}

SmartUtil::tstring WindowClass::getNewClassName(Widget* widget)
{
	std::basic_stringstream< TCHAR > className;
	className << typeid(*widget).name() << ++itsInstanceNo;
	return className.str();
}

}

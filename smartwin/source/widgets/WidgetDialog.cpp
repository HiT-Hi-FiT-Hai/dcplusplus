#include "../../include/smartwin/widgets/WidgetDialog.h"

namespace SmartWin {

void WidgetDialog::createDialog( unsigned resourceId )
{
	HWND wnd = ::CreateDialogParam( Application::instance().getAppHandle(),
		MAKEINTRESOURCE( resourceId ),
		( this->getParent() ? this->getParent()->handle() : 0 ),
		( (DLGPROC)&ThisType::wndProc ),
		reinterpret_cast< LPARAM >( static_cast< Widget * >( this ) ) );

	if ( !wnd ) {
		xCeption x( _T( "CreateDialogParam failed." ) );
		throw x;
	}
	setHandle(wnd);
}

}

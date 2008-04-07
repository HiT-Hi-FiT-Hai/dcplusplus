#include "../../include/smartwin/widgets/ModelessDialog.h"

#include "../../include/smartwin/Application.h"

namespace SmartWin {

void ModelessDialog::createDialog( unsigned resourceId )
{
	HWND wnd = ::CreateDialogParam( Application::instance().getAppHandle(),
		MAKEINTRESOURCE( resourceId ),
		( this->getParent() ? this->getParent()->handle() : 0 ),
		( (DLGPROC)&ThisType::wndProc ),
		reinterpret_cast< LPARAM >( static_cast< Widget * >( this ) ) );

	if ( !wnd ) {
		throw xCeption( _T( "CreateDialogParam failed." ) );
	}
}

}

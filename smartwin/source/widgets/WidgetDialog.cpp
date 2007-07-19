#include "../../include/smartwin/widgets/WidgetDialog.h"

namespace SmartWin {

void WidgetDialog::createDialog( unsigned resourceId )
{
	this->Widget::itsHandle = ::CreateDialogParam( Application::instance().getAppHandle(),
		MAKEINTRESOURCE( resourceId ),
		( this->Widget::itsParent ? this->Widget::itsParent->handle() : 0 ),
		( (DLGPROC)&ThisType::wndProc ),
		reinterpret_cast< LPARAM >( boost::polymorphic_cast< Widget * >( this ) ) );

	if ( !this->Widget::itsHandle ) {
		xCeption x( _T( "CreateDialogParam failed." ) );
		throw x;
	}

	Widget::registerWidget();
}

}

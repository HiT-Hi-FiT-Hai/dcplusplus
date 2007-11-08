#include "../../include/smartwin/widgets/WidgetModalDialog.h"

namespace SmartWin {

int WidgetModalDialog::createDialog( unsigned resourceId )
{
	// this will not return until the dialog is closed by calling endDialog() with
	// a retv
	//
	INT_PTR retv = ::DialogBoxParam
		( ( Application::instance().getAppHandle() )
		, ( MAKEINTRESOURCE( resourceId ) )
		, ( this->getParent() ? this->getParent()->handle() : 0 )
		, ( (DLGPROC)&ThisType::wndProc )
		, ( reinterpret_cast< LPARAM >( dynamic_cast< Widget * >( this ) ) )
		);
	if ( retv == - 1 )
	{
		throw xCeption( _T( "Couldn't create modal dialog" ) );
	}
	return static_cast< int >( retv );
}

int WidgetModalDialog::createDialog()
{
	// Arrange so the DLGTEMPLATE is followed by 0000 for menu, winclass and title.
	unsigned char dlg_menu_winclass_title[ sizeof( DLGTEMPLATE ) + 30 ];
	memset( dlg_menu_winclass_title, 0, sizeof( dlg_menu_winclass_title ) );
	memcpy( dlg_menu_winclass_title, & itsDefaultDlgTemplate, sizeof( DLGTEMPLATE ) );

	// this will not return until the dialog is closed by calling endDialog() with
	// a retv
	//
	INT_PTR retv = ::DialogBoxIndirectParam
		( Application::instance().getAppHandle() // HINSTANCE hInstance
		, ( DLGTEMPLATE * ) dlg_menu_winclass_title // LPCDLGTEMPLATE hDialogTemplate
		, this->getParent() ? this->getParent()->handle() : 0 // HWND hWndParent
		, (DLGPROC)&ThisType::wndProc // DLGPROC lpDialogFunc
		, reinterpret_cast< LPARAM >( dynamic_cast< Widget * >( this ) )
		); // LPARAM dwInitParam

	return static_cast< int >( retv );
}

}

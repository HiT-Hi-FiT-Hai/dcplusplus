#include "../../include/smartwin/widgets/ToolBar.h"

namespace SmartWin {

ToolBar::Seed::Seed() :
	Widget::Seed(TOOLBARCLASSNAME, WS_CHILD | WS_VISIBLE | TBSTYLE_LIST | TBSTYLE_TOOLTIPS)
{
}

void ToolBar::create( const Seed & cs )
{
	ControlType::create(cs);

	this->sendMessage(TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_MIXEDBUTTONS);

	//// Telling the toolbar what the size of TBBUTTON struct is
	this->sendMessage(TB_BUTTONSTRUCTSIZE, ( WPARAM ) sizeof( TBBUTTON ));
}

void ToolBar::appendSeparator()
{
	TBBUTTON tb = { 0 };
	tb.fsStyle = BTNS_SEP;
	if ( this->sendMessage(TB_ADDBUTTONS, 1, reinterpret_cast< LPARAM >( &tb ) ) == FALSE )
	{
		xCeption x( _T( "Error while trying to add a button to toolbar..." ) );
		throw x;
	}
}

void ToolBar::appendItem( int image, const SmartUtil::tstring& toolTip, const Dispatcher::F& f)
{
	int id = -1;
	
	if(f) {
		for(id = 0; id < (int)commands.size(); ++id) {
			if(!commands[id])
				break;
		}
		if(id == (int)commands.size()) {
			commands.push_back(f);
		} else {
			commands[id] = f;
		}
	}
	
	// Adding button
	TBBUTTON tb = { 0 };
	tb.iBitmap = image;
	tb.idCommand = id;
	tb.fsState = TBSTATE_ENABLED;
	tb.fsStyle = BTNS_AUTOSIZE;
	tb.iString = reinterpret_cast<INT_PTR>(toolTip.c_str());
	if ( this->sendMessage(TB_ADDBUTTONS, 1, reinterpret_cast< LPARAM >( &tb ) ) == FALSE )
	{
		xCeption x( _T( "Error while trying to add a button to toolbar..." ) );
		throw x;
	}
}

bool ToolBar::tryFire( const MSG & msg, LRESULT & retVal ) {
	if(msg.message == WM_COMMAND && msg.lParam == reinterpret_cast<LPARAM>(handle())) {
		size_t id = LOWORD(msg.wParam);
		if(id < commands.size() && commands[id]) {
			commands[id]();
			return true;
		}
	}
	return PolicyType::tryFire(msg, retVal);
}

}

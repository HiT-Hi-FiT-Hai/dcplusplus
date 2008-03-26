#include "../../include/smartwin/widgets/WidgetToolbar.h"

namespace SmartWin {

WidgetToolbar::Seed::Seed() :
	Widget::Seed(TOOLBARCLASSNAME, WS_CHILD | WS_VISIBLE | TBSTYLE_LIST | TBSTYLE_TOOLTIPS)
{
}

void WidgetToolbar::create( const Seed & cs )
{
	ControlType::create(cs);

	this->sendMessage(TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_MIXEDBUTTONS);

	//// Telling the toolbar what the size of TBBUTTON struct is
	this->sendMessage(TB_BUTTONSTRUCTSIZE, ( WPARAM ) sizeof( TBBUTTON ));
}

void WidgetToolbar::appendSeparator()
{
	TBBUTTON tb = { 0 };
	tb.fsStyle = BTNS_SEP;
	if ( this->sendMessage(TB_ADDBUTTONS, 1, reinterpret_cast< LPARAM >( &tb ) ) == FALSE )
	{
		xCeption x( _T( "Error while trying to add a button to toolbar..." ) );
		throw x;
	}
}

void WidgetToolbar::appendItem( unsigned int id, int image, const SmartUtil::tstring& toolTip, const Dispatcher::F& f)
{
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

	if(f)
		addCallback(Message(WM_COMMAND, id), Dispatcher(f));
}

bool WidgetToolbar::tryFire( const MSG & msg, LRESULT & retVal )
{
	bool handled = PolicyType::tryFire(msg, retVal);
	
	if(!handled && msg.message == WM_COMMAND) {
		Widget* parent = getParent();
		if(parent != NULL) {
			// Maybe parent knows what to do with the WM_COMMAND (in case of shared menu/toolbar id's)
			handled = parent->tryFire(msg, retVal);
		}
	}
	
	return handled;
}

}

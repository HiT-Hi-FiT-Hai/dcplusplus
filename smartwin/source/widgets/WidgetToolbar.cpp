#include "../../include/smartwin/widgets/WidgetToolbar.h"

namespace SmartWin {

WidgetToolbar::Seed::Seed() :
	Widget::Seed(TOOLBARCLASSNAME, WS_CHILD | WS_VISIBLE | TBSTYLE_TOOLTIPS)
{
}

void WidgetToolbar::create( const Seed & cs )
{
	ControlType::create(cs);

	//// Telling the toolbar what the size of TBBUTTON struct is
	this->sendMessage(TB_BUTTONSTRUCTSIZE, ( WPARAM ) sizeof( TBBUTTON ));
}

void WidgetToolbar::appendSeparator()
{
	TBBUTTON tb = { 0 };
	tb.iBitmap = 1;
	tb.fsState = TBSTATE_ENABLED;
	tb.fsStyle = BTNS_SEP;
	if ( this->sendMessage(TB_ADDBUTTONS, 1, reinterpret_cast< LPARAM >( &tb ) ) == FALSE )
	{
		xCeption x( _T( "Error while trying to add a button to toolbar..." ) );
		throw x;
	}
}

void WidgetToolbar::appendItem( unsigned int id, int image, const SmartUtil::tstring& toolTip, const Dispatcher::F& f)
{
	// Checking if tooltip id exists from before
	if ( itsToolTips.find( id ) != itsToolTips.end() )
	{
		xCeption x( _T( "Tried to add a button with an ID that already exists..." ) );
		throw x;
	}

	// Adding button
	TBBUTTON tb = { 0 };
	tb.iBitmap = image;
	tb.idCommand = id;
	tb.fsState = TBSTATE_ENABLED;
	tb.fsStyle = BTNS_AUTOSIZE;
	if ( this->sendMessage(TB_ADDBUTTONS, 1, reinterpret_cast< LPARAM >( &tb ) ) == FALSE )
	{
		xCeption x( _T( "Error while trying to add a button to toolbar..." ) );
		throw x;
	}
	
	if(!toolTip.empty()) {
		itsToolTips[id] = toolTip;
	}
	
	if(f)
		setCallback(Message(WM_COMMAND, id), Dispatcher(f));
}

bool WidgetToolbar::tryFire( const MSG & msg, LRESULT & retVal )
{
	if(msg.message == WM_NOTIFY) {
		LPNMHDR hdr = reinterpret_cast< LPNMHDR >( msg.lParam );
		if(hdr->code == TTN_GETDISPINFO) {
			LPTOOLTIPTEXT lpttt = reinterpret_cast< LPTOOLTIPTEXT >( msg.lParam );
			if(itsToolTips.find(lpttt->hdr.idFrom) != itsToolTips.end()) {
				lpttt->lpszText = const_cast < TCHAR * >( itsToolTips[lpttt->hdr.idFrom].c_str() );
				return true;
			}
		}
	}
	
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

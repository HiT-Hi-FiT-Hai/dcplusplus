#include "../../include/smartwin/widgets/WidgetToolbar.h"

namespace SmartWin {

const WidgetToolbar::Seed & WidgetToolbar::getDefaultSeed()
{
	static bool d_NeedsInit = true;
	static Seed d_DefaultValues( DontInitializeMe );

	if ( d_NeedsInit )
	{
		d_DefaultValues.className = TOOLBARCLASSNAME;
#ifndef WINCE
		d_DefaultValues.exStyle = TBSTYLE_EX_MIXEDBUTTONS;
#else
		d_DefaultValues.exStyle = 0;
#endif
		d_DefaultValues.style = WS_CHILD | WS_VISIBLE | TBSTYLE_TOOLTIPS;
		d_NeedsInit = false;
	}
	return d_DefaultValues;
}

void WidgetToolbar::create( const Seed & cs )
{
	xAssert((cs.style & WS_CHILD) == WS_CHILD, _T("Widget must have WS_CHILD style"));
	PolicyType::create(cs);

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

void WidgetToolbar::appendItem( unsigned int id, const SmartUtil::tstring& toolTip)
{
	// Checking if tooltip id exists from before
	if ( itsToolTips.find( id ) != itsToolTips.end() )
	{
		xCeption x( _T( "Tried to add a button with an ID that already exists..." ) );
		throw x;
	}

	// Adding button
	TBBUTTON tb = { 0 };
	tb.iBitmap = size();
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
}

}

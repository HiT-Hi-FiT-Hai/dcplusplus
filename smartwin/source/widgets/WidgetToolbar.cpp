#include "../../include/smartwin/widgets/WidgetToolbar.h"

namespace SmartWin {

const WidgetToolbar::Seed & WidgetToolbar::getDefaultSeed()
{
	static bool d_NeedsInit = true;
	static Seed d_DefaultValues( DontInitializeMe );

	if ( d_NeedsInit )
	{
		Application::instance().setSystemClassName( d_DefaultValues, TOOLBARCLASSNAME );
#ifndef WINCE
		d_DefaultValues.exStyle = TBSTYLE_EX_MIXEDBUTTONS;
#else
		d_DefaultValues.exStyle = 0;
#endif
		d_DefaultValues.style = WS_CHILD | WS_VISIBLE | TBSTYLE_TOOLTIPS | TBSTYLE_SEP | CCS_ADJUSTABLE | TBSTYLE_ALTDRAG;
		d_NeedsInit = false;
	}
	return d_DefaultValues;
}

void WidgetToolbar::create( const Seed & cs )
{
	xAssert((cs.style & WS_CHILD) == WS_CHILD, "Widget must have WS_CHILD style");
	PolicyType::create(cs);

	//// Telling the toolbar what the size of TBBUTTON struct is
	::SendMessage( this->Widget::itsHandle, TB_BUTTONSTRUCTSIZE, ( WPARAM ) sizeof( TBBUTTON ), 0 );
	::SendMessage( this->Widget::itsHandle, TB_SETIMAGELIST, 0, 0 );
	////TODO: use CreationalInfo parameters
}

}

#include "../../include/smartwin/widgets/WidgetGroupBox.h"

namespace SmartWin {

const WidgetGroupBox::Seed & WidgetGroupBox::getDefaultSeed()
{
	static bool d_NeedsInit = true;
	static Seed d_DefaultValues( DontInitializeMe );

	if ( d_NeedsInit )
	{
		d_DefaultValues.className = _T( "BUTTON" );
		d_DefaultValues.style = WS_CHILD | WS_VISIBLE | BS_GROUPBOX;
		d_DefaultValues.font = createFont( DefaultGuiFont );
		d_NeedsInit = false;
	}
	return d_DefaultValues;
}

void WidgetGroupBox::create( const Seed & cs )
{
	xAssert((cs.style & WS_CHILD) == WS_CHILD, "Widget must have WS_CHILD style");
	PolicyType::create(cs);
	setFont( cs.font );
}

}

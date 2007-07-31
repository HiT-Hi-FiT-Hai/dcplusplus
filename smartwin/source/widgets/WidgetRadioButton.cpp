#include "../../include/smartwin/widgets/WidgetRadioButton.h"

namespace SmartWin {

const WidgetRadioButton::Seed & WidgetRadioButton::getDefaultSeed()
{
	static bool d_NeedsInit = true;
	static Seed d_DefaultValues( DontInitializeMe );

	if ( d_NeedsInit )
	{
		d_DefaultValues.className = _T( "BUTTON" );
		d_DefaultValues.style = WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON;
		d_DefaultValues.font = createFont( DefaultGuiFont );
		d_NeedsInit = false;
	}
	return d_DefaultValues;
}

void WidgetRadioButton::create( Widget * parent, const Seed & cs )
{
	xAssert((cs.style & WS_CHILD) == WS_CHILD, _T("Widget must have WS_CHILD style"));
	PolicyType::create(cs);
	setFont( cs.font );
}

}

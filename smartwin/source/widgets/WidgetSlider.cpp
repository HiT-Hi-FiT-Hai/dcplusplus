#include "../../include/smartwin/widgets/WidgetSlider.h"

namespace SmartWin {

const WidgetSlider::Seed & WidgetSlider::getDefaultSeed()
{
	static bool d_NeedsInit = true;
	static Seed d_DefaultValues( DontInitializeMe );

	if ( d_NeedsInit )
	{
		d_DefaultValues.className = TRACKBAR_CLASS;
		d_DefaultValues.style = WS_CHILD | WS_VISIBLE;
		//TODO: initialize the values here
		d_NeedsInit = false;
	}
	return d_DefaultValues;
}

void WidgetSlider::create( const Seed & cs )
{
	xAssert((cs.style & WS_CHILD) == WS_CHILD, _T("Widget must have WS_CHILD style"));
	PolicyType::create(cs);
}

}

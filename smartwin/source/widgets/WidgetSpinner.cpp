#include "../../include/smartwin/widgets/WidgetSpinner.h"

namespace SmartWin {

const WidgetSpinner::Seed & WidgetSpinner::getDefaultSeed()
{
	static bool d_NeedsInit = true;
	static Seed d_DefaultValues( DontInitializeMe );

	if ( d_NeedsInit )
	{
		d_DefaultValues.className = UPDOWN_CLASS;
		d_DefaultValues.minValue = 0;
		d_DefaultValues.maxValue = 100;

		d_DefaultValues.style = WS_CHILD | WS_VISIBLE;
		//TODO: initialize the values here
		d_NeedsInit = false;
	}
	return d_DefaultValues;
}

void WidgetSpinner::create( const Seed & cs )
{
	xAssert((cs.style & WS_CHILD) == WS_CHILD, _T("Widget must have WS_CHILD style"));
	PolicyType::create(cs);
	setRange( cs.minValue, cs.maxValue );
	//TODO: use CreationalInfo parameters
}

}

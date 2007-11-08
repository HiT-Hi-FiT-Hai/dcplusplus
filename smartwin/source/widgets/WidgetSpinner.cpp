#include "../../include/smartwin/widgets/WidgetSpinner.h"

namespace SmartWin {

WidgetSpinner::Seed::Seed() :
	Widget::Seed(UPDOWN_CLASS, WS_CHILD | WS_VISIBLE | WS_TABSTOP),
	minValue(0),
	maxValue(100)
{
}

void WidgetSpinner::create( const Seed & cs )
{
	ControlType::create(cs);
	setRange( cs.minValue, cs.maxValue );
}

}

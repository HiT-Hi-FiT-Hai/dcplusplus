#include "../../include/smartwin/widgets/Spinner.h"

namespace SmartWin {

Spinner::Seed::Seed() :
	Widget::Seed(UPDOWN_CLASS, WS_CHILD | WS_VISIBLE | WS_TABSTOP),
	minValue(0),
	maxValue(100)
{
}

void Spinner::create( const Seed & cs )
{
	ControlType::create(cs);
	setRange( cs.minValue, cs.maxValue );
}

}

#include "../../include/smartwin/widgets/WidgetStatic.h"

namespace SmartWin {

const WidgetStatic::Seed & WidgetStatic::getDefaultSeed()
{
	static bool d_NeedsInit = true;
	static Seed d_DefaultValues( DontInitializeMe );

	if ( d_NeedsInit )
	{
		d_DefaultValues.className = _T("Static");
		d_DefaultValues.style = WS_CHILD | WS_VISIBLE | SS_NOTIFY;
		d_DefaultValues.font = createFont( DefaultGuiFont );
		d_NeedsInit = false;
	}
	return d_DefaultValues;
}

void WidgetStatic::create( const Seed & cs )
{
	xAssert((cs.style & WS_CHILD) == WS_CHILD, _T("Widget must have WS_CHILD style"));
	PolicyType::create(cs);
	setFont( cs.font );
}

}

#include "../../include/smartwin/widgets/WidgetTabSheet.h"

namespace SmartWin {

const WidgetTabSheet::Seed & WidgetTabSheet::getDefaultSeed()
{
	static bool d_NeedsInit = true;
	static Seed d_DefaultValues( DontInitializeMe );

	if ( d_NeedsInit )
	{
		d_DefaultValues.className = WC_TABCONTROL;
		d_DefaultValues.style = WS_CHILD | WS_VISIBLE;
		d_DefaultValues.font = createFont( DefaultGuiFont );
		d_NeedsInit = false;
	}
	return d_DefaultValues;
}

void WidgetTabSheet::create( const Seed & cs )
{
	xAssert((cs.style & WS_CHILD) == WS_CHILD, _T("Widget must have WS_CHILD style"));
	PolicyType::create(cs);
	setFont( cs.font );
}

}

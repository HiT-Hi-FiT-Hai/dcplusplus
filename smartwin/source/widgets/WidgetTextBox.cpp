#include "../../include/smartwin/widgets/WidgetTextBox.h"

namespace SmartWin {

const WidgetTextBox::Seed & WidgetTextBox::getDefaultSeed()
{
	static bool d_NeedsInit = true;
	static Seed d_DefaultValues( DontInitializeMe );

	if ( d_NeedsInit )
	{
		d_DefaultValues.className = _T("Edit");
		d_DefaultValues.style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_WANTRETURN;
		d_DefaultValues.exStyle = WS_EX_CLIENTEDGE;
		d_DefaultValues.font = createFont( DefaultGuiFont );
		d_NeedsInit = false;
	}
	return d_DefaultValues;
}

void WidgetTextBox::create( const WidgetTextBox::Seed & cs )
{
	xAssert((cs.style & WS_CHILD) == WS_CHILD, _T("Widget must have WS_CHILD style"));
	PolicyType::create(cs);
	setFont( cs.font );
}

}

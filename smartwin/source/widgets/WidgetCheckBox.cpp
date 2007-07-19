#include "../../include/smartwin/widgets/WidgetCheckBox.h"

namespace SmartWin {

const WidgetCheckBox::Seed & WidgetCheckBox::getDefaultSeed() {
	static bool d_NeedsInit = true;
	static Seed d_DefaultValues( DontInitializeMe );

	if ( d_NeedsInit )
	{
		Application::instance().setSystemClassName( d_DefaultValues, _T("Button") );
		d_DefaultValues.style = WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | BS_CHECKBOX;
		d_DefaultValues.font = createFont( DefaultGuiFont );
		d_NeedsInit = false;
	}
	return d_DefaultValues;
}

void WidgetCheckBox::create( const Seed & cs )
{
	xAssert((cs.style & WS_CHILD) == WS_CHILD, "Widget must have WS_CHILD style");
	PolicyType::create(cs);
	setFont( cs.font );
}

}

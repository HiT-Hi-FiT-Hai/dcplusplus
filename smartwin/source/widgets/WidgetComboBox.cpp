#include "../../include/smartwin/widgets/WidgetComboBox.h"

namespace SmartWin {

const WidgetComboBox::Seed & WidgetComboBox::getDefaultSeed()
{
	static bool d_NeedsInit = true;
	static Seed d_DefaultValues( DontInitializeMe );

	if ( d_NeedsInit )
	{
		Application::instance().setSystemClassName( d_DefaultValues, WC_COMBOBOX );
		d_DefaultValues.style = WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_VSCROLL;
		d_DefaultValues.font = createFont( DefaultGuiFont );
		d_DefaultValues.extended = true;
		d_NeedsInit = false;
	}
	return d_DefaultValues;
}

void WidgetComboBox::create( const Seed & cs )
{
	xAssert((cs.style & WS_CHILD) == WS_CHILD, "Widget must have WS_CHILD style");
	PolicyType::create(cs);
	setFont( cs.font );
	if(cs.extended) {
		::SendMessage(this->handle(), CB_SETEXTENDEDUI, TRUE, 0);
	}
}

}

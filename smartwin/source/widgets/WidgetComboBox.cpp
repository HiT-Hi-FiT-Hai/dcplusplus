#include "../../include/smartwin/widgets/WidgetComboBox.h"
#include "../../include/smartwin/WidgetCreator.h"

namespace SmartWin {

const WidgetComboBox::Seed & WidgetComboBox::getDefaultSeed()
{
	static bool d_NeedsInit = true;
	static Seed d_DefaultValues( DontInitializeMe );

	if ( d_NeedsInit )
	{
		d_DefaultValues.className = WC_COMBOBOX;
		d_DefaultValues.style = WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_VSCROLL;
		d_DefaultValues.font = createFont( DefaultGuiFont );
		d_DefaultValues.extended = true;
		d_NeedsInit = false;
	}
	return d_DefaultValues;
}

void WidgetComboBox::create( const Seed & cs )
{
	xAssert((cs.style & WS_CHILD) == WS_CHILD, _T("Widget must have WS_CHILD style"));
	PolicyType::create(cs);
	setFont( cs.font );
	if(cs.extended) {
		::SendMessage(this->handle(), CB_SETEXTENDEDUI, TRUE, 0);
	}
}

WidgetComboBox::WidgetTextBoxPtr WidgetComboBox::getTextBox() {
	if(!textBox) {
		LONG_PTR style = ::GetWindowLongPtr(handle(), GWL_STYLE);
		if((style & CBS_SIMPLE)  == CBS_SIMPLE || (style & CBS_DROPDOWN) == CBS_DROPDOWN) {
			HWND wnd = ::FindWindowEx(handle(), NULL, _T("EDIT"), NULL);
			if(wnd && wnd != handle())
				textBox = WidgetCreator< WidgetTextBox >::attach(this, wnd);
		}
	}
	return textBox;
}

}

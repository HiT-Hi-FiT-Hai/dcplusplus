#include "../../include/smartwin/widgets/WidgetComboBox.h"
#include "../../include/smartwin/WidgetCreator.h"

namespace SmartWin {

WidgetComboBox::Seed::Seed() : 
	Widget::Seed(WC_COMBOBOX, WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWNLIST | CBS_AUTOHSCROLL),
	font(new Font(DefaultGuiFont)),
	extended(true)
{
}

void WidgetComboBox::create( const Seed & cs )
{
	ControlType::create(cs);
	if(cs.font)
		setFont( cs.font );
	if(cs.extended)
		sendMessage(CB_SETEXTENDEDUI, TRUE);
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

#include "../../include/smartwin/widgets/ComboBox.h"

namespace SmartWin {

ComboBox::Seed::Seed() : 
	Widget::Seed(WC_COMBOBOX, WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWNLIST | CBS_AUTOHSCROLL),
	font(new Font(DefaultGuiFont)),
	extended(true)
{
}

void ComboBox::create( const Seed & cs )
{
	ControlType::create(cs);
	if(cs.font)
		setFont( cs.font );
	if(cs.extended)
		sendMessage(CB_SETEXTENDEDUI, TRUE);
}

}

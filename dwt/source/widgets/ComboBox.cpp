#include "../../include/smartwin/widgets/ComboBox.h"

namespace SmartWin {

ComboBox::Seed::Seed() : 
	BaseType::Seed(WC_COMBOBOX, WS_CHILD | WS_TABSTOP | WS_VSCROLL),
	font(new Font(DefaultGuiFont)),
	extended(true)
{
}

void ComboBox::create( const Seed & cs ) {
	BaseType::create(cs);
	if(cs.font)
		setFont( cs.font );
	if(cs.extended)
		sendMessage(CB_SETEXTENDEDUI, TRUE);
}

SmartUtil::tstring ComboBox::getValue( int index ) {
	// Uses CB_GETLBTEXTLEN and CB_GETLBTEXT
	int txtLength = ComboBox_GetLBTextLen( handle(), index );
	SmartUtil::tstring retVal(txtLength, '\0');
	ComboBox_GetLBText( handle(), index, &retVal[0] );
	return retVal;
}

}

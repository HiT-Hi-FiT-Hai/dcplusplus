#include "../../include/smartwin/widgets/RadioButton.h"

namespace SmartWin {

RadioButton::Seed::Seed(const SmartUtil::tstring& caption) : 
	Widget::Seed(WC_BUTTON, WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTORADIOBUTTON, 0, caption),
	font(new Font(DefaultGuiFont))
{
}

}

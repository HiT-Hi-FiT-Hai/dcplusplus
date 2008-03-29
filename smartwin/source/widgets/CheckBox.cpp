#include "../../include/smartwin/widgets/CheckBox.h"

namespace SmartWin {

CheckBox::Seed::Seed(const SmartUtil::tstring& caption) : 
	Widget::Seed(WC_BUTTON, WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX | BS_CHECKBOX, 0, caption),
	font(new Font(DefaultGuiFont))
{
}

}

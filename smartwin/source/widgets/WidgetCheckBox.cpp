#include "../../include/smartwin/widgets/WidgetCheckBox.h"

namespace SmartWin {

WidgetCheckBox::Seed::Seed(const SmartUtil::tstring& caption) : 
	Widget::Seed(WC_BUTTON, WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX | BS_CHECKBOX, 0, caption),
	font(new Font(DefaultGuiFont))
{
}

}

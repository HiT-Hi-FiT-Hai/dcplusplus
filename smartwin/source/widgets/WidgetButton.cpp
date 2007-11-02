#include "../../include/smartwin/widgets/WidgetButton.h"

namespace SmartWin {

WidgetButton::Seed::Seed(const SmartUtil::tstring& caption) : 
	Widget::Seed(WC_BUTTON, WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, 0, caption),
	font(new Font(DefaultGuiFont))
{
}

}

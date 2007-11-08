#include "../../include/smartwin/widgets/WidgetGroupBox.h"

namespace SmartWin {

WidgetGroupBox::Seed::Seed(const SmartUtil::tstring& caption) :
	Widget::Seed(WC_BUTTON, WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 0, caption),
	font(new Font(DefaultGuiFont))
{
}

}

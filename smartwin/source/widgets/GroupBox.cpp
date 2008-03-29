#include "../../include/smartwin/widgets/GroupBox.h"

namespace SmartWin {

GroupBox::Seed::Seed(const SmartUtil::tstring& caption) :
	Widget::Seed(WC_BUTTON, WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 0, caption),
	font(new Font(DefaultGuiFont))
{
}

}

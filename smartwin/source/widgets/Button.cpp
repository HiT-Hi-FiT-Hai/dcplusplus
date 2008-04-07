#include "../../include/smartwin/widgets/Button.h"

namespace SmartWin {

Button::Seed::Seed(const SmartUtil::tstring& caption, DWORD style) : 
	BaseType::Seed(WC_BUTTON, style | WS_CHILD, 0, caption),
	font(new Font(DefaultGuiFont))
{
}

}

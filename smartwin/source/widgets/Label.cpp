#include "../../include/smartwin/widgets/Label.h"

namespace SmartWin {

Label::Seed::Seed(const SmartUtil::tstring& caption) :
	Widget::Seed(WC_STATIC, WS_CHILD | WS_VISIBLE | SS_NOTIFY, 0, caption),
	font(new Font(DefaultGuiFont))
{

}

void Label::create( const Seed & cs ) {
	ControlType::create(cs);
	if(cs.font)
		setFont( cs.font );
}

}

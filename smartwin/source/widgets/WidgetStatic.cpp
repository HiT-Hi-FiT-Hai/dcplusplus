#include "../../include/smartwin/widgets/WidgetStatic.h"

namespace SmartWin {

WidgetStatic::Seed::Seed(const SmartUtil::tstring& caption) :
	Widget::Seed(WC_STATIC, WS_CHILD | WS_VISIBLE | SS_NOTIFY, 0, caption),
	font(new Font(DefaultGuiFont))
{

}

void WidgetStatic::create( const Seed & cs ) {
	ControlType::create(cs);
	if(cs.font)
		setFont( cs.font );
}

}

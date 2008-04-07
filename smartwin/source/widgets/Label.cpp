#include "../../include/smartwin/widgets/Label.h"

namespace SmartWin {

Label::Seed::Seed(const SmartUtil::tstring& caption) :
	BaseType::Seed(WC_STATIC, WS_CHILD | SS_NOTIFY, 0, caption),
	font(new Font(DefaultGuiFont))
{

}

void Label::create( const Seed & cs ) {
	BaseType::create(cs);
	if(cs.font)
		setFont( cs.font );
}

}

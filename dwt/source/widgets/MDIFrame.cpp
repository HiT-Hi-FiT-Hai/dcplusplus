#include "../../include/smartwin/widgets/MDIFrame.h"
#include "../../include/smartwin/WidgetCreator.h"
#include "../../include/smartwin/widgets/MDIParent.h"

namespace SmartWin {

// TODO Fix caption
MDIFrame::Seed::Seed(const SmartUtil::tstring& caption) :
	BaseType::Seed(caption, 0, 0)
{
}

void MDIFrame::create( const Seed& cs )
{
	BaseType::create(cs);
	
	mdi = WidgetCreator<MDIParent>::create(this);
}

}

#include "../../include/smartwin/widgets/MDIFrame.h"
#include "../../include/smartwin/WidgetCreator.h"
#include "../../include/smartwin/widgets/MDIParent.h"

namespace SmartWin {

MDIFrame::Seed::Seed() :
	BaseType::Seed(0)
{
}

void MDIFrame::create( const Seed& cs )
{
	BaseType::create(cs);
	
	mdi = WidgetCreator<MDIParent>::create(this);
}

}

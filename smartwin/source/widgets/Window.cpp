#include "../../include/smartwin/widgets/Window.h"

namespace SmartWin {

// TODO add caption
Window::Seed::Seed() : BaseType::Seed(SmartUtil::tstring(), 0, 0) {
	
}

void Window::create(const Seed& cs) {
	BaseType::create(cs);
}

}

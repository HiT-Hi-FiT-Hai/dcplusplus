#include "../../include/smartwin/widgets/Window.h"

namespace SmartWin {

Window::Seed::Seed() : BaseType::Seed(0) {
	
}

void Window::create(const Seed& cs) {
	BaseType::create(cs);
}

}
